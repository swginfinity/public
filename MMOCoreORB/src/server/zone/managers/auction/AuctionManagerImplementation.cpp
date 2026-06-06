/*
 * AuctionManagerImplementation.cpp
 *
 *  Created on: 13/03/2010
 *      Author: victor
 */

#include "server/zone/managers/auction/AuctionManager.h"
#include "server/zone/managers/auction/AuctionsMap.h"
#include "server/zone/managers/object/ObjectManager.h"
#include "templates/manager/TemplateManager.h"
#include "server/zone/managers/player/PlayerManager.h"
#include "server/zone/objects/auction/AuctionItem.h"
#include "server/zone/packets/auction/ItemSoldMessage.h"
#include "server/zone/packets/auction/CancelLiveAuctionResponseMessage.h"
#include "server/zone/packets/auction/AuctionQueryHeadersResponseMessage.h"
#include "server/zone/packets/auction/RetrieveAuctionItemResponseMessage.h"
#include "server/zone/packets/auction/BidAuctionResponseMessage.h"
#include "server/zone/packets/scene/AttributeListMessage.h"
#include "server/chat/StringIdChatParameter.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/objects/player/PlayerObject.h"
#include "server/zone/objects/region/CityRegion.h"
#include "server/zone/objects/building/BuildingObject.h"
#include "server/zone/objects/waypoint/WaypointObject.h"
#include "server/zone/Zone.h"
#include "server/zone/ZoneServer.h"
#include "server/chat/ChatManager.h"
#include "CheckAuctionsTask.h"
#include "ExpireAuctionTask.h"
#include "server/zone/managers/vendor/VendorManager.h"
#include "server/zone/objects/tangible/components/vendor/VendorDataComponent.h"
#include "server/zone/objects/tangible/components/vendor/AuctionTerminalDataComponent.h"
#include "server/zone/objects/intangible/VendorControlDevice.h"
#include "server/zone/objects/cell/CellObject.h"
#include "conf/ConfigManager.h"
#include "server/zone/objects/player/sessions/TradeSession.h"
#include "AuctionSearchTask.h"
#include "server/zone/objects/factorycrate/FactoryCrate.h"
#include "server/zone/objects/transaction/TransactionLog.h"
// Infinity:  Custom includes
#include "server/zone/objects/resource/ResourceContainer.h"

const int AuctionManagerImplementation::MAXVENDORPRICE; // Infinity:  Added for clang compatibility

namespace {
	// Vendor Pack-Up (Phase 1): boot-sweep cache for auction items belonging
	// to packed vendors. AuctionsMap::addItem rejects vendors with
	// getZone() == nullptr, and packed vendors live inside a VCD inside a
	// player datapad (no zone). We can't load these items into auctionsMap
	// at boot, so we hold strong refs here and re-add them when the vendor
	// deploys (postUnpackVendor). File-scope avoids an IDL change for a
	// transient runtime cache.
	Mutex packedVendorPendingItemsMutex;
	VectorMap<uint64, Vector<ManagedReference<AuctionItem*>>> packedVendorPendingItems;
}

void AuctionManagerImplementation::initialize() {
	Locker locker(_this.getReferenceUnsafeStaticCast());

	auto logLevel = ConfigManager::instance()->getInt("Core3.AuctionManager.LogLevel", (int)-1);

	if (logLevel > -1) {
		setGlobalLogging(false);
		setFileLogger("log/auctions.log", true, ConfigManager::instance()->getRotateLogAtStart());
		setLogSynchronized(true);
		setRotateLogSizeMB(ConfigManager::instance()->getInt("Core3.AuctionManager.RotateLogSizeMB", ConfigManager::instance()->getRotateLogSizeMB()));
		setLogToConsole(false);
		info(true) << "AuctionManager initializing.";
		setLogLevel(static_cast<Logger::LogLevel>(logLevel));
	}

	Core::getTaskManager()->initializeCustomQueue("AuctionSearch", ConfigManager::instance()->getMaxAuctionSearchJobs(), true);

	auctionMap = new AuctionsMap();

	ObjectDatabase* auctionDatabase = ObjectDatabaseManager::instance()->loadObjectDatabase("auctionitems", true);
	ObjectDatabaseManager::instance()->commitLocalTransaction();

	ObjectDatabaseIterator iterator(auctionDatabase);

	Time startTime;
	Time progressTime;
	int countDatabaseItems = 0;
	int countDuplicates = 0;
	uint64 objectID = 0;
	int expireSecondsOffset = 300;

	Vector<Reference<AuctionItem*>> itemsToDelete;
	Vector<ManagedReference<AuctionItem*>> orphanedBazaarItems;
	ManagedReference<SceneObject*> defaultBazaar = nullptr;
	ManagedReference<PlayerManager*> playerManager = zoneServer->getPlayerManager();

	while (iterator.getNextKey(objectID)) {
		countDatabaseItems++;
		Reference<AuctionItem*> auctionItem = Core::getObjectBroker()->lookUp(objectID).castTo<AuctionItem*>();
		ObjectDatabaseManager::instance()->commitLocalTransaction();

		if (auctionItem == nullptr) {
			error() << "Unable to load auction item: " << objectID;
			continue;
		}

		Locker lock(auctionItem);

		/* Infinity:  Save this as example in case we need to fix any other auction item names
		if (auctionItem->getItemType() == 4194315 && auctionItem->getItemName().beginsWith("Lokian  Animal Bones")) {
			auctionItem->setItemName("Lokian " + auctionItem->getItemName().subString(8));
		}
		*/

		if (progressTime.miliDifference() > 5000) {
			progressTime.updateToCurrentTime();
			info(true) << "Scanned " << countDatabaseItems << " auctionitems db object(s) and loaded " << auctionMap->getTotalItemCount() << " object(s).";
		}

		if (auctionItem->getStatus() == AuctionItem::RETRIEVED || (auctionItem->getStatus() == AuctionItem::EXPIRED && auctionItem->getExpireTime() <= time(0))) {
			itemsToDelete.add(auctionItem);
			continue;
		}

		ManagedReference<SceneObject*> vendor = zoneServer->getObject(auctionItem->getVendorID());

		if (vendor == nullptr || vendor->getZone() == nullptr) {
			if (auctionItem->isOnBazaar()) {
				orphanedBazaarItems.add(auctionItem);
				continue;
			}

			// Vendor Pack-Up (Phase 1): a packed vendor lives inside a VCD
			// inside a player's datapad and has getZone() == nullptr - but
			// it is NOT orphaned, just dormant. Canonical "is packed?" test
			// is structural: vendor's root parent is a creature object.
			// (suppressUIDRebuild on AuctionTerminalDataComponent is also
			// set during pack but is a UID-rebuild gate, not the source of
			// truth for packed-state.) Items are stashed in
			// packedVendorPendingItems and re-added to auctionsMap by
			// postUnpackVendor when the vendor deploys back into a zone.
			if (vendor != nullptr) {
				auto rootParent = vendor->getRootParent();
				if (rootParent != nullptr && rootParent->isCreatureObject()) {
					{
						Locker pendLock(&packedVendorPendingItemsMutex);
						uint64 vid = vendor->getObjectID();
						if (!packedVendorPendingItems.contains(vid)) {
							packedVendorPendingItems.put(vid, Vector<ManagedReference<AuctionItem*>>());
						}
						packedVendorPendingItems.get(vid).add(auctionItem);
					}
					info(true) << "Vendor " << vendor->getObjectID()
					           << " is packed (rootParent OID=" << rootParent->getObjectID()
					           << " is creature); deferred load of auction item "
					           << auctionItem->getAuctionedItemObjectID()
					           << " until vendor deploys.";
					continue;
				}

				error() << "Vendor with no zone, deleting vendorObject: " << *vendor;
				vendor->destroyObjectFromWorld(true);
				vendor->destroyObjectFromDatabase();
			}

			warning() << "Auction Item's vendor is gone, deleting auctionItem: " << *auctionItem;
			itemsToDelete.add(auctionItem);
			continue;
		}

		String ownerName = playerManager->getPlayerName(auctionItem->getOwnerID());

		if (ownerName.isEmpty()) {
			error() << "Auction with invalid owner, deleting auctionItem: " << *auctionItem;
			itemsToDelete.add(auctionItem);
			continue;
		}

		uint64 vendorExpire = time(0) + ConfigManager::instance()->getVendorExpirePeriod();
		uint64 commodityExpire = time(0) + ConfigManager::instance()->getCommodityExpirePeriod();

		if (auctionItem->getStatus() == AuctionItem::FORSALE && auctionItem->getExpireTime() > vendorExpire) {
			auto oldExpire = auctionItem->getExpireTime();
			auctionItem->setExpireTime(vendorExpire);
			error() << "Invalid expiration time for vendor auction, was: " << oldExpire << " auctionItem: " << *auctionItem;
		}

		if (auctionItem->getStatus() == AuctionItem::OFFERED && auctionItem->getExpireTime() > commodityExpire) {
			auto oldExpire = auctionItem->getExpireTime();
			auctionItem->setExpireTime(commodityExpire);
			error() << "Invalid expiration time for auction, was: " << oldExpire << " auctionItem: " << *auctionItem;
		}

		if (vendor->isBazaarTerminal() && defaultBazaar == nullptr)
			defaultBazaar = vendor;

		int result = auctionMap->addItem(nullptr, vendor, auctionItem);

		// On duplicate item, pick the highest auctionItem oid and arrange to delete the rest
		if (result == ItemSoldMessage::ALREADYFORSALE) {
			countDuplicates++;
			Reference<AuctionItem*> currentItem = auctionMap->getItem(auctionItem->getAuctionedItemObjectID());

			if (auctionItem->getObjectID() > currentItem->getObjectID()) {
				Reference<SceneObject*> currentVendor = zoneServer->getObject(currentItem->getVendorID());

				auctionMap->removeItem(currentVendor, currentItem);
				itemsToDelete.add(currentItem);

				// Try again, result is checked below
				result = auctionMap->addItem(nullptr, vendor, auctionItem);
			} else {
				itemsToDelete.add(auctionItem);
				continue;
			}
		}

		if (result != ItemSoldMessage::SUCCESS) {
			auto msg = error();

			msg << "Failed to addItem to AuctionsMap " << ItemSoldMessage::statusToString(result) << ", auctionItem: " << *auctionItem;

			if (result == ItemSoldMessage::ALREADYFORSALE) {
				Reference<AuctionItem*> otherItem = auctionMap->getItem(auctionItem->getAuctionedItemObjectID());

				if (otherItem != nullptr) {
					msg << "; otherAuctionItem: " << *otherItem;
				}
			}

			msg.flush();
			continue;
		}

		if (auctionItem->isOnBazaar() || auctionItem->getStatus() == AuctionItem::OFFERED)
			auctionMap->addToCommodityLimit(auctionItem);

		if (auctionItem->isAuction()) {
			Reference<Task*> newTask = new ExpireAuctionTask(_this.getReferenceUnsafeStaticCast(), auctionItem);
			int expireSeconds = auctionItem->getExpireTime() - time(0);

			if (expireSeconds < 0) {
				expireSeconds = (expireSecondsOffset += 5);
				error() << "Item past expireTime, scheduling expire in " << expireSeconds << " seconds, auctionItem: " << *auctionItem;
			}

			newTask->schedule(expireSeconds * 1000);

			Locker locker(&auctionEvents);
			auctionEvents.put(auctionItem->getAuctionedItemObjectID(), newTask);
		}
	}

	for (int i = 0; i < itemsToDelete.size(); ++i) {
		auto auctionItem = itemsToDelete.get(i);

		if (auctionItem == nullptr) {
			continue;
		}

		Locker lock(auctionItem);

		// By default delete auctioned object on Expired auctions.
		auto deleteAuctionedObject = auctionItem->getStatus() == AuctionItem::EXPIRED;

		auto msg = info(true);

		msg << "Deleting " << auctionItem->getStatusString() << " item";

		if (auctionMap->containsItem(auctionItem->getAuctionedItemObjectID())) {
			msg << " (Duplicate Listing)";

			deleteAuctionedObject = false; // Other auctionItem holds the object
		}

		Reference<SceneObject*> vendor = zoneServer->getObject(auctionItem->getVendorID());

		if (vendor == nullptr) {
			msg << " (Vendor missing)";
		}

		msg << ", auctionItem: " << *auctionItem;

		if (!deleteAuctionedObject) {
			// If we're not deleting it clear the auctioned item
			auctionItem->setAuctionedItemObjectID(0);
		}

		msg.flush();

		auctionItem->destroyAuctionItemFromDatabase(false, deleteAuctionedObject);
	}

	/// This is in case a bazaar is removed, it could move and item
	/// to a difference city, but at least it doesn't poof
	if (defaultBazaar != nullptr) {
		for (int i = 0; i < orphanedBazaarItems.size(); ++i) {
			ManagedReference<AuctionItem*> auctionItem = orphanedBazaarItems.get(i);

			Locker lock(auctionItem);

			String vuid = getVendorUID(defaultBazaar);
			auctionMap->addItem(nullptr, defaultBazaar, auctionItem);

			Locker alocker(auctionItem);
			auctionItem->setVendorID(defaultBazaar->getObjectID());

			if (auctionItem->isAuction()) {
				Reference<Task*> newTask = new ExpireAuctionTask(_this.getReferenceUnsafeStaticCast(), auctionItem);
				newTask->schedule((auctionItem->getExpireTime() - time(0)) * 1000);

				Locker locker(&auctionEvents);
				auctionEvents.put(auctionItem->getAuctionedItemObjectID(), newTask);
			}
		}
	}

	for (int i = 0; i < pendingUIDUpdates.size(); ++i) {
		ManagedReference<SceneObject*> vendor = pendingUIDUpdates.elementAt(i).getKey();
		String uid = pendingUIDUpdates.get(vendor);
		String oldUID = pendingOldUIDUpdates.get(vendor);

		auctionMap->updateUID(vendor, oldUID, uid);
	}

	locker.release();

	Core::getTaskManager()->executeTask(
		[=]() {
			checkAuctions(true);
			checkVendorItems(true);
		},
		"StartupAuctionManagerCheck", "slowQueue");

	auto elapsed = startTime.miliDifference() / 1000.0;
	int ps = elapsed > 0 ? countDatabaseItems / elapsed : countDatabaseItems;
	int skipped = countDatabaseItems - auctionMap->getTotalItemCount();

	info(true) << "Scanned " << countDatabaseItems << " auctionitem db object(s) "
			   << "in " << elapsed << " second(s), (" << ps << "/s), "
			   << "skipped " << skipped << " (" << countDuplicates << " duplicate listings), "
			   << "loaded " << auctionMap->getTotalItemCount() << " object(s) into auctionsMap.";
}

void AuctionManagerImplementation::checkVendorItems(bool startupTask) {
	if (startupTask)
		info("checkVendorItems initial startup task", true);

	Timer timer(Time::MONOTONIC_TIME);

	timer.start();
	TerminalListVector items = auctionMap->getVendorTerminalData("", "", 0);

	info("Checking " + String::valueOf(items.size()) + " vendor terminals", true);

	doAuctionMaint(&items, "vendor", startupTask);

	auto elapsed = timer.stopMs();

	info("Vendor terminal checks completed in " + String::valueOf(elapsed) + "ms", true);

	if (startupTask) {
		info(true) << "Market enabled";

		marketEnabled = true;
	}
}

void AuctionManagerImplementation::checkAuctions(bool startupTask) {
	if (startupTask)
		info("checkAuctions initial startup task", true);

	Reference<CheckAuctionsTask*> task = new CheckAuctionsTask(_this.getReferenceUnsafeStaticCast());
	task->schedule(CHECKEVERY * 60 * 1000);

	Timer timer(Time::MONOTONIC_TIME);
	timer.start();
	TerminalListVector items = auctionMap->getBazaarTerminalData("", "", 0);

	info("Checking " + String::valueOf(items.size()) + " bazaar terminals", true);

	doAuctionMaint(&items, "bazaar", startupTask);

	auto elapsed = timer.stopMs();

	info("Bazaar terminal checks completed in " + String::valueOf(elapsed) + "ms", true);
}

void AuctionManagerImplementation::doAuctionMaint(TerminalListVector* items, const String& logTag, bool startupTask) {
	Time expireTime;
	Time progressTime;
	uint64 currentTime = expireTime.getMiliTime() / 1000;

	int countTotal = 0;
	int countUpdated = 0;
	int countInvalid = 0;

	for (int i = 0; i < items->size(); ++i) {
		Reference<TerminalItemList*>& terminalList = items->get(i);

		if (terminalList == nullptr || terminalList->size() == 0)
			continue;

		// Get a copy of this terminal's list because the loop deletes some objects as it runs
		Reference<TerminalItemList*> list = new TerminalItemList(*terminalList);

		for (int j = 0; j < list->size(); ++j) {
			ManagedReference<AuctionItem*> item = list->get(j);

			if (item == nullptr)
				continue;

			Locker locker(item);

			countTotal++;

			if (progressTime.miliDifference() > 5000) {
				progressTime.updateToCurrentTime();
				info(true) << logTag << ": Checked " << countTotal << " auctions in " << i << " of " << items->size() << " " << logTag << "s.";
			}

			if (item->getStatus() == AuctionItem::DELETED) {
				error() << "Skipping deleted auctionItem: " << *item;
				continue;
			}

			ManagedReference<SceneObject*> vendor = zoneServer->getObject(item->getVendorID());
			ManagedReference<PlayerManager*> playerManager = zoneServer->getPlayerManager();
			String ownerName = playerManager->getPlayerName(item->getOwnerID());

			if (vendor == nullptr || vendor->getZone() == nullptr || ownerName.isEmpty()) {
				// Vendor Pack-Up (Phase 1): same packed-vendor protection as
				// the boot sweep. If vendor is dormant inside a VCD inside a
				// player datapad (root parent is a creature), do NOT delete
				// its items - they are in legitimate hibernation, not
				// orphaned. The periodic sweep would otherwise destroy a
				// player's listings any time the parent-chain resolution
				// produced a null zone (offline owner, partial load, etc).
				if (vendor != nullptr) {
					auto rootParent = vendor->getRootParent();
					if (rootParent != nullptr && rootParent->isCreatureObject()) {
						continue;
					}
				}

				StringBuffer errMsg;

				if (vendor == nullptr) {
					errMsg << "missing vendor, ";
				}

				if (vendor != nullptr && vendor->getZone() == nullptr) {
					errMsg << "vendor missing zone, ";
				}

				if (ownerName.isEmpty()) {
					errMsg << "missing owner, ";
				}

				error() << "Auction Item failed validation: " << errMsg.toString() << "deleting auctionItem: " << *item;

				uint64 sellingId = item->getAuctionedItemObjectID();
				auctionMap->deleteItem(vendor, item, true);
				continue;
			}

			uint64 vendorExpire = time(0) + ConfigManager::instance()->getVendorExpirePeriod();
			uint64 commodityExpire = time(0) + ConfigManager::instance()->getCommodityExpirePeriod();
			bool updatedExpire = false;
			uint64 oldExpire = 0;

			if (item->getStatus() == AuctionItem::FORSALE && item->getExpireTime() > vendorExpire) {
				oldExpire = item->getExpireTime();
				item->setExpireTime(vendorExpire);
				updatedExpire = true;
			}

			if (item->getStatus() == AuctionItem::OFFERED && item->getExpireTime() > commodityExpire) {
				oldExpire = item->getExpireTime();
				item->setExpireTime(commodityExpire);
				updatedExpire = true;
			}

			if (updatedExpire) {
				if (item->isAuction() && auctionEvents.contains(item->getAuctionedItemObjectID())) {
					Reference<Task*> newTask = auctionEvents.get(item->getAuctionedItemObjectID());

					if (newTask != nullptr)
						newTask->reschedule((item->getExpireTime() - time(0)) * 1000);
				}

				error() << "Auction Item had invalid expiration time. Old: " << oldExpire << ", new: " << item->getExpireTime() << ", auctionItem: " << *item;
			}

			if (item->getExpireTime() <= currentTime) {
				if (item->getStatus() == AuctionItem::EXPIRED) {
					expireSale(item);
					continue;
				}

				// Infinity: Expire offered items that have passed their expiration time.
				// Without this, OFFERED items only expire lazily when someone browses
				// the vendor. If no one visits, the item stays OFFERED indefinitely and
				// the offerer can never retrieve it from their stockroom.
				if (item->getStatus() == AuctionItem::OFFERED) {
					expireSale(item);
					continue;
				}
			}

			if (item->getStatus() == AuctionItem::RETRIEVED) {
				error() << "Found RETRIEVED item in maintenance, auctionItem: " << *item;
				auctionMap->deleteItem(vendor, item);
				continue;
			}

			if (startupTask && ConfigManager::instance()->getBool("Core3.AuctionManager.Startup.ExpireInvalid", false)) {
				String validationError;
				auto sellingId = item->getAuctionedItemObjectID();
				auto sellingItem = zoneServer->getObject(sellingId);

				if (sellingItem == nullptr) {
					validationError = "has null item, deleting";
					auctionMap->deleteItem(vendor, item, true);
				} else if (sellingItem->isNoTrade() || sellingItem->containsNoTradeObjectRecursive()) {
					validationError = "isNoTrade or contains NoTrade items, expiring";
					expireSale(item);
				}

				if (!validationError.isEmpty()) {
					countInvalid++;
					error() << logTag << ": Invalid auction for item " << sellingId << " " << validationError << " auctionItem: " << *item;
					continue;
				}
			}

			if (startupTask && !item->isUpdated()) {
				bool updatedItemCrate = false;
				uint64 sellingId = item->getAuctionedItemObjectID();
				ManagedReference<SceneObject*> sellingItem = zoneServer->getObject(sellingId);

				if (sellingItem != nullptr) {
					if (sellingItem->isFactoryCrate()) {
						Locker clocker(sellingItem, item);
						Reference<FactoryCrate*> crate = sellingItem.castTo<FactoryCrate*>();

						if (crate != nullptr) {
							ManagedReference<TangibleObject*> prototype = crate->getPrototype();

							if (prototype != nullptr) {
								item->setFactoryCrate(true);
								// Infinity Custom: Shriek Hawk Missile object type override
								if (prototype->getObjectTemplate() != nullptr && (prototype->getObjectTemplate()->getTemplateFileName().contains("wpn_missile_mandalorian"))) {
									item->setCratedItemType(SceneObjectType::SHIPMISSILE);
								} else {
									item->setCratedItemType(prototype->getClientGameObjectType());
								}
								updatedItemCrate = true;
							}
						}
					} else {
						if (item->isFactoryCrate()) {
							item->setFactoryCrate(false);
							updatedItemCrate = true;
						}

						if (item->getCratedItemType() != 0) {
							item->setCratedItemType(0);
							updatedItemCrate = true;
						}
					}
				}

				item->setUpdated(true);

				if (updatedItemCrate) {
					countUpdated++;
				}
			}
		}
	}

	auto elapsed = expireTime.miliDifference() / 1000.0;
	int ps = elapsed > 0 ? countTotal / elapsed : countTotal;

	auto msg = info(true);

	msg << logTag << ": Checked " << countTotal << " auction item(s),"
		<< " updated " << countUpdated << " item(s)";

	if (startupTask && ConfigManager::instance()->getBool("Core3.AuctionManager.Startup.ExpireInvalid", false)) {
		msg << " and found " << countInvalid << " invalid item(s),";
	}

	msg << " in " << int(elapsed) << " second(s) (" << ps << "/s)";
	msg.flush();
}

int AuctionManagerImplementation::addSaleItem(CreatureObject* player, uint64 objectid, SceneObject* vendor, const UnicodeString& description, int price, uint32 duration, bool auction, bool premium) {
	if (player == nullptr || player->isDead() || player->isIncapacitated()) {
		ItemSoldMessage* soldMessage = new ItemSoldMessage(objectid, ItemSoldMessage::UNKNOWNERROR);

		if (player != nullptr && soldMessage != nullptr) {
			player->sendMessage(soldMessage);
		}

		return ItemSoldMessage::UNKNOWNERROR;
	}
	if (vendor == nullptr || (!vendor->isVendor() && !vendor->isBazaarTerminal())) {
		error() << "addSaleItem(player=" << player->getObjectID() << ", objectid=" << objectid << "): Not valid vendor object.";

		ItemSoldMessage* soldMessage = new ItemSoldMessage(objectid, ItemSoldMessage::VENDORNOTWORKING);

		if (soldMessage != nullptr) {
			player->sendMessage(soldMessage);
		}

		return ItemSoldMessage::VENDORNOTWORKING;
	}

	auto zone = vendor->getZone();

	if (zone == nullptr) {
		error() << "addSaleItem(plyer=" << player->getObjectID() << ", objectid=" << objectid << "): Vendor has null zone.";

		ItemSoldMessage* soldMessage = new ItemSoldMessage(objectid, ItemSoldMessage::UNKNOWNERROR);

		if (soldMessage != nullptr) {
			player->sendMessage(soldMessage);
		}

		return ItemSoldMessage::UNKNOWNERROR;
	}

	ManagedReference<TradeSession*> tradeContainer = player->getActiveSession(SessionFacadeType::TRADE).castTo<TradeSession*>();

	if (tradeContainer != nullptr) {
		auto playerManager = zoneServer->getPlayerManager();

		if (playerManager != nullptr) {
			playerManager->handleAbortTradeMessage(player);
		}
	}

	bool stockroomSale = false;

	ManagedReference<SceneObject*> objectToSell = zoneServer->getObject(objectid);

	if (objectToSell == nullptr || objectToSell->isNoTrade() || objectToSell->containsNoTradeObjectRecursive()) {
		ItemSoldMessage* soldMessage = new ItemSoldMessage(objectid, ItemSoldMessage::INVALIDITEM);

		if (soldMessage != nullptr) {
			player->sendMessage(soldMessage);
		}

		if (objectToSell != nullptr) {
			player->sendSystemMessage("@container_error_message:container26"); // This item could not be transferred.
		}

		return ItemSoldMessage::INVALIDITEM;
	}

	ManagedReference<AuctionItem*> oldItem = auctionMap->getItem(objectid);

	if (oldItem == nullptr) {
		if (objectToSell == nullptr || !objectToSell->isASubChildOf(player)) {
			if (objectToSell != nullptr) {
				error() << "addSaleItem(plyer=" << player->getObjectID() << ", objectid=" << objectid << "): Selling invalid object.";
			}

			ItemSoldMessage* soldMessage = new ItemSoldMessage(objectid, ItemSoldMessage::INVALIDITEM);

			if (soldMessage != nullptr) {
				player->sendMessage(soldMessage);
			}

			return ItemSoldMessage::INVALIDITEM;
		}
	} else {
		if (oldItem->getStatus() == AuctionItem::DELETED) {
			error() << "addSaleItem(plyer=" << player->getObjectID() << ", objectid=" << objectid << "): Attempt to sell a DELETED item, auctionItem: " << *oldItem;

			ItemSoldMessage* soldMessage = new ItemSoldMessage(objectid, ItemSoldMessage::INVALIDITEM);

			if (soldMessage != nullptr) {
				player->sendMessage(soldMessage);
			}

			return ItemSoldMessage::INVALIDITEM;
		}

		if (oldItem->getStatus() == AuctionItem::RETRIEVED) {
			error() << "addSaleItem(plyer=" << player->getObjectID() << ", objectid=" << objectid << "): Attempt to sell a RETRIEVED item, auctionItem: " << *oldItem;

			ItemSoldMessage* soldMessage = new ItemSoldMessage(objectid, ItemSoldMessage::INVALIDITEM);

			if (soldMessage != nullptr) {
				player->sendMessage(soldMessage);
			}

			return ItemSoldMessage::INVALIDITEM;
		}

		if (oldItem->getStatus() == AuctionItem::FORSALE) {
			ItemSoldMessage* soldMessage = new ItemSoldMessage(objectid, ItemSoldMessage::ALREADYFORSALE);

			if (soldMessage != nullptr) {
				player->sendMessage(soldMessage);
			}

			return ItemSoldMessage::ALREADYFORSALE;
		}

		/// Is it being sold from the stockroom
		if (oldItem->getOwnerID() != player->getObjectID()) {
			error() << "addSaleItem(plyer=" << player->getObjectID() << ", objectid=" << objectid << "): Selling object owned by another player: " << oldItem->getOwnerID();

			ItemSoldMessage* soldMessage = new ItemSoldMessage(objectid, ItemSoldMessage::INVALIDITEM);

			if (soldMessage != nullptr) {
				player->sendMessage(soldMessage);
			}

			return ItemSoldMessage::INVALIDITEM;
		}

		stockroomSale = true;

		ManagedReference<SceneObject*> oldVendor = zoneServer->getObject(oldItem->getVendorID());

		if (oldVendor != nullptr && oldVendor->isVendor()) {
			vendor = oldVendor;
		}
	}

	int result = checkSaleItem(player, objectToSell, vendor, price, premium, stockroomSale);

	if (result == ItemSoldMessage::SUCCESS) {
		// Infinity:  Custom code to make sure items are only sold from player inventory or wearable container
		auto containerPlayerParent = objectToSell->getParentRecursively(SceneObjectType::PLAYERCREATURE);

		if (containerPlayerParent != nullptr) {
			auto inventory = containerPlayerParent->getSlottedObject("inventory");
			auto backpack = containerPlayerParent->getSlottedObject("back");
			bool validLocation = (inventory != nullptr and objectToSell->isASubChildOf(inventory)) || (backpack != nullptr && backpack->isWearableContainerObject() && objectToSell->isASubChildOf(backpack));
			if (!validLocation) {
				result = ItemSoldMessage::INVALIDITEM;
				player->sendSystemMessage("@ui_auc:err_create_sale_invalid_item_location"); // You can only sell items that are in your inventory or backpack
			}
		}
	}

	if (result != ItemSoldMessage::SUCCESS) {
		ItemSoldMessage* soldMessage = new ItemSoldMessage(objectid, result);

		if (soldMessage != nullptr) {
			player->sendMessage(soldMessage);
		}

		return result;
	}

	// Old item is on the auction map already, remove it first
	if (oldItem != nullptr) {
		auctionMap->deleteItem(vendor, oldItem);
	}

	// Check to ensure item was removed from the auction map
	if (auctionMap->containsItem(objectToSell->getObjectID())) {
		ItemSoldMessage* soldMessage = new ItemSoldMessage(objectid, ItemSoldMessage::ALREADYFORSALE);

		if (soldMessage != nullptr) {
			player->sendMessage(soldMessage);
		}

		return ItemSoldMessage::ALREADYFORSALE;
	}

	// add city tax to the price
	ManagedReference<CityRegion*> city = vendor->getCityRegion().get();

	if (city != nullptr) {
		int salesTax = city->getSalesTax();

		if (salesTax > 0) {
			price *= (1.0f + (salesTax / 100.0f));
		}
	}

	ManagedReference<AuctionItem*> item = createVendorItem(player, objectToSell.get(), vendor, description, price, duration, auction, premium);

	if (item == nullptr) {
		error() << "addSaleItem(plyer=" << player->getObjectID() << ", objectid=" << objectid << "): createVendorItem failed.";

		ItemSoldMessage* soldMessage = new ItemSoldMessage(objectid, ItemSoldMessage::UNKNOWNERROR);

		if (soldMessage != nullptr) {
			player->sendMessage(soldMessage);
		}

		return ItemSoldMessage::UNKNOWNERROR;
	}

	TransactionLog trx(player, vendor, objectToSell, TrxCode::AUCTIONADDSALE);
	trx.setAutoCommit(false);
	trx.addRelatedObject(item->getAuctionedItemObjectID(), true);
	trx.setExportRelatedObjects(true);

	Locker locker(item);

	result = auctionMap->addItem(player, vendor, item);

	if (result != ItemSoldMessage::SUCCESS) {
		trx.abort() << "failed to add to auctionMap, result=" << ItemSoldMessage::statusToString(result);

		info() << "addSaleItem(plyer=" << player->getObjectID() << ", objectid=" << objectid << "): " << trx.getErrorMessage() << ", auctionItem: " << *item;

		ItemSoldMessage* soldMessage = new ItemSoldMessage(objectid, result);

		if (soldMessage != nullptr) {
			player->sendMessage(soldMessage);
		}

		auctionMap->removeFromCommodityLimit(item);

		return result;
	}

	Locker objectToSellLocker(objectToSell);

	objectToSell->destroyObjectFromWorld(true);

	objectToSellLocker.release();

	trx.commit();

	if (vendor->isBazaarTerminal()) {
		StringIdChatParameter str("@base_player:sale_fee"); // The fee for your listing is %DI credits.

		float costReduction = 1;

		if (player->hasSkill("crafting_merchant_sales_01")) {
			costReduction = .80f;
		}

		if (player->hasSkill("crafting_merchant_sales_03")) {
			costReduction = .60f;
		}

		if (item->isPremiumAuction()) {
			TransactionLog trx(player, TrxCode::BAZAARSYSTEM, costReduction * (SALESFEE * 5), false);
			player->subtractBankCredits(costReduction * (SALESFEE * 5));
			str.setDI(costReduction * (SALESFEE * 5));

		} else {
			TransactionLog trx(player, TrxCode::BAZAARSYSTEM, costReduction * SALESFEE, false);
			player->subtractBankCredits(costReduction * SALESFEE);
			str.setDI(costReduction * SALESFEE);
		}

		player->sendSystemMessage(str);
	}

	if (item->getStatus() == AuctionItem::OFFERED) {
		VendorDataComponent* vendorData = nullptr;
		DataObjectComponentReference* data = vendor->getDataObjectComponent();

		if (data != nullptr && data->get() != nullptr && data->get()->isVendorData()) {
			vendorData = cast<VendorDataComponent*>(data->get());
		}

		if (vendorData != nullptr) {
			auto ownerSceno = zoneServer->getObject(vendorData->getOwnerId());

			if (ownerSceno != nullptr && ownerSceno->isPlayerCreature()) {
				auto ownerCreo = cast<CreatureObject*>(ownerSceno.get());

				if (ownerCreo != nullptr) {
					if (ownerCreo->isOnline()) {
						ownerCreo->sendSystemMessage(player->getDisplayedName() + " has offered an item to " + vendor->getDisplayedName());
					}

					// Infinity: Thanks to Tarkin's Revenge: Email player about offer to vendor
					auto chatManager = zoneServer->getChatManager();
					if (chatManager != nullptr) {
						UnicodeString subject("@auction:vendor_offer_subject"); // An item has been offered to your vendor

						StringIdChatParameter offerBody("@auction:vendor_offer_body"); // %TU has been offered %TO by %TT for %DI credits.
						offerBody.setTU(vendor->getDisplayedName());
						offerBody.setTO(item->getItemName());
						offerBody.setTT(player->getDisplayedName());
						offerBody.setDI(item->getPrice());

						float waypointX = vendor->getWorldPositionX();
						float waypointY = vendor->getWorldPositionY();

						auto waypointObject = zoneServer->createObject(STRING_HASHCODE("object/waypoint/world_waypoint_blue.iff"), 1).castTo<WaypointObject*>();

						if (waypointObject != nullptr) {
							Locker locker(waypointObject);
							waypointObject->setCustomObjectName(vendor->getDisplayedName(), false);
							waypointObject->setActive(false);
							waypointObject->setPosition(waypointX, 0, waypointY);
							waypointObject->setPlanetCRC(vendor->getPlanetCRC());
						}

						chatManager->sendMail(vendor->getDisplayedName(), subject, offerBody, ownerCreo->getFirstName(), waypointObject); // ChatManager will handle nullptr waypoint
					}
				}
			}
		}
	}

	item->setPersistent(1);

	if (item->isAuction()) {
		Reference<Task*> newTask = new ExpireAuctionTask(_this.getReferenceUnsafeStaticCast(), item);
		newTask->schedule((item->getExpireTime() - time(0)) * 1000);

		Locker locker(&auctionEvents);
		auctionEvents.put(item->getAuctionedItemObjectID(), newTask);
	}

	BaseMessage* msg = new ItemSoldMessage(objectid, ItemSoldMessage::SUCCESS);
	player->sendMessage(msg);

	info() << "addSaleItem(plyer=" << player->getObjectID() << ", objectid=" << objectid << "): SUCCESS, auctionItem: " << *item;

	return ItemSoldMessage::SUCCESS;
}

String AuctionManagerImplementation::getVendorUID(SceneObject* vendor) {
	if (vendor->getZone() == nullptr) {
		error() << "Vendor missing zone: " << *vendor;
		return "nozone.nozone.sadpandavendor." + String::valueOf(vendor->getObjectID()) + "#0,0";
	}

	String uid = "error.error.errorvendor." + String::valueOf(vendor->getObjectID()) + "#0,0";

	AuctionTerminalDataComponent* terminalData = nullptr;
	DataObjectComponentReference* data = vendor->getDataObjectComponent();
	if (data != nullptr && data->get() != nullptr && data->get()->isAuctionTerminalData())
		terminalData = cast<AuctionTerminalDataComponent*>(data->get());

	if (terminalData != nullptr)
		uid = terminalData->getUID();

	return uid;
}

int AuctionManagerImplementation::checkSaleItem(CreatureObject* player, SceneObject* object, SceneObject* vendor, int price, bool premium, bool stockroomSale) {
	if (vendor == nullptr) {
		error() << "checkSaleItem(player=" << player->getObjectID() << ", object=" << (object != nullptr ? String::valueOf(object->getObjectID()) : "nullptr") << ", vendor=nullptr): Vendor is null";
		return ItemSoldMessage::UNKNOWNERROR;
	}

	if (object == nullptr) {
		error() << "checkSaleItem(player=" << player->getObjectID() << ", object=nullptr, vendor=" << vendor->getObjectID();
		return ItemSoldMessage::UNKNOWNERROR;
	}

	if (price < 1) {
		return ItemSoldMessage::INVALIDSALEPRICE;
	}

	if (player->getPlayerObject()->getVendorCount() > player->getSkillMod("manage_vendor"))
		return ItemSoldMessage::TOOMANYITEMS;

	auto itemParent = object->getParent().get();

	if (itemParent == nullptr && !stockroomSale) {
		error() << "checkSaleItem(player=" << player->getObjectID() << ", object=" << object->getObjectID() << ", vendor=" << vendor->getObjectID() << "): itemParent is null; stockroomSale=" << stockroomSale;
		return ItemSoldMessage::UNKNOWNERROR;
	}

	if (itemParent != nullptr && !itemParent->checkContainerPermission(player, ContainerPermissions::MOVEOUT)) {
		error() << "checkSaleItem(player=" << player->getObjectID() << ", object=" << object->getObjectID() << ", vendor=" << vendor->getObjectID() << "): No MOVEOUT permission from " << itemParent->getObjectID() << " (" << itemParent->getObjectName()->getFullPath() << ") stockroomSale=" << stockroomSale;
		return ItemSoldMessage::INVALIDITEM;
	}

	if (vendor->isVendor()) {
		VendorDataComponent* vendorData = cast<VendorDataComponent*>(vendor->getDataObjectComponent()->get());

		if (vendorData == nullptr) {
			return ItemSoldMessage::UNKNOWNERROR;
		}

		if (player->getObjectID() == vendorData->getOwnerId()) {
			if (stockroomSale) {
				if (auctionMap->getPlayerItemCount(player) > player->getSkillMod("vendor_item_limit"))
					return ItemSoldMessage::TOOMANYITEMS;
			} else {
				if ((auctionMap->getPlayerItemCount(player) + object->getSizeOnVendorRecursive()) > player->getSkillMod("vendor_item_limit"))
					return ItemSoldMessage::TOOMANYITEMS;
			}
		} else {
			if (auctionMap->getCommodityCount(player) >= MAXSALES)
				return ItemSoldMessage::TOOMANYITEMS;
		}

		if (price > MAXVENDORPRICE)
			return ItemSoldMessage::INVALIDSALEPRICE;
	}

	if (vendor->isBazaarTerminal()) {
		if (auctionMap->getCommodityCount(player) >= MAXSALES)
			return ItemSoldMessage::TOOMANYITEMS;

		if (price > MAXBAZAARPRICE)
			return ItemSoldMessage::INVALIDSALEPRICE;

		if (player->getBankCredits() < SALESFEE)
			return ItemSoldMessage::NOTENOUGHCREDITS;

		if (premium && player->getBankCredits() < SALESFEE * 5)
			return ItemSoldMessage::NOTENOUGHCREDITS;
	}

	if (object->isIntangibleObject() && !object->isManufactureSchematic())
		return ItemSoldMessage::INVALIDITEM;

	for (int i = 0; i < object->getArrangementDescriptorSize(); ++i) {
		const Vector<String>* descriptors = object->getArrangementDescriptor(i);

		for (int j = 0; j < descriptors->size(); ++j) {
			const String& descriptor = descriptors->get(j);

			if (descriptor == "inventory" || descriptor == "datapad" || descriptor == "default_weapon" || descriptor == "mission_bag" || descriptor == "ghost" || descriptor == "bank" || descriptor == "hair")
				return ItemSoldMessage::INVALIDITEM;
		}
	}

	return ItemSoldMessage::SUCCESS;
}

AuctionItem* AuctionManagerImplementation::createVendorItem(CreatureObject* player, SceneObject* objectToSell, SceneObject* vendor, const UnicodeString& description, int price, unsigned int duration, bool auction, bool premium) {
	Zone* zone = vendor->getZone();

	if (zone == nullptr)
		return nullptr;

	uint64 vendorExpire = time(0) + ConfigManager::instance()->getVendorExpirePeriod();
	uint64 commodityExpire = time(0) + ConfigManager::instance()->getCommodityExpirePeriod();
	uint64 auctionExpire = time(0) + ConfigManager::instance()->getAuctionExpirePeriod();
	uint64 vendorOfferExpire = time(0) + ConfigManager::instance()->getVendorOfferExpirePeriod();

	String playername = player->getFirstName().toLowerCase();
	String descr = description.toString();
	String planetStr = zone->getZoneName();

	AuctionItem* item = new AuctionItem(objectToSell->getObjectID());

	ManagedReference<CityRegion*> cityRegion = vendor->getCityRegion().get();
	String region = "@planet_n:" + planetStr;

	if (cityRegion != nullptr)
		region = cityRegion->getCityRegionName();

	String name = objectToSell->getDisplayedName();

	Locker locker(item);

	item->setVendorUID(getVendorUID(vendor));
	item->setOnBazaar(vendor->isBazaarTerminal());

	if (premium)
		item->setAuctionPremium();

	item->setVendorID(vendor->getObjectID());

	if (objectToSell->isResourceContainer()) { // Infinity custom resource listings

		ManagedReference<ResourceContainer*> resourceContainer = cast<ResourceContainer*>(objectToSell);
		if (resourceContainer != nullptr) {
			int quantity = resourceContainer->getQuantity();
			float quantityK = quantity / 1000.0f;
			bool whole = quantityK == floor(quantityK); // Integer, no decimal points
			float cpu = price / (float)quantity;
			String quantityString;

			if (quantity < 10000) { // 0-9999
				quantityString = String::valueOf(quantity, 0);
			} else if (whole || quantity >= 100000) { // Whole number or 100k+
				quantityString = String::valueOf(quantity / 1000.0f, 0) + "k";
				;
			} else { // Fractional and between 10K and 99999K
				quantityString = String::valueOf(quantity / 1000.0f, 1) + "k";
				;
			}

			name += " <" + quantityString + " @ " + String::valueOf(cpu, 2) + " cpu>";
		}
	}

	item->setItemName(name);
	item->setItemDescription(description.toString());

	if (objectToSell->isFactoryCrate()) {
		ManagedReference<FactoryCrate*> crate = cast<FactoryCrate*>(objectToSell);

		if (crate != nullptr) {
			ManagedReference<TangibleObject*> prototype = crate->getPrototype();

			if (prototype != nullptr) {
				item->setFactoryCrate(true);
				// Infinity Custom: Shriek Hawk Missile object type override
				if (prototype->getObjectTemplate() != nullptr && (prototype->getObjectTemplate()->getTemplateFileName().contains("wpn_missile_mandalorian"))) {
					item->setCratedItemType(SceneObjectType::SHIPMISSILE);
				} else {
					item->setCratedItemType(prototype->getClientGameObjectType());
				}
				item->setUpdated(true);
			}
		}
	}

	item->setItemType(objectToSell->getClientGameObjectType());
	item->setPrice(price);
	item->setAuction(auction);
	item->setStatus(AuctionItem::FORSALE);
	item->setBuyerID(0);
	item->setBidderName("");
	item->setSize(objectToSell->getSizeOnVendorRecursive());

	// Infinity Custom: Shriek Hawk Missile object type override
	if (objectToSell->getObjectTemplate() != nullptr && (objectToSell->getObjectTemplate()->getTemplateFileName().contains("wpn_missile_mandalorian"))) {
		item->setItemType(SceneObjectType::SHIPMISSILE);
	}

	VendorDataComponent* vendorData = nullptr;
	DataObjectComponentReference* data = vendor->getDataObjectComponent();
	if (data != nullptr && data->get() != nullptr && data->get()->isVendorData())
		vendorData = cast<VendorDataComponent*>(data->get());

	if (!vendor->isBazaarTerminal()) {
		if (vendorData == nullptr)
			return nullptr;

		// Someone else's Vendor (making this sell item an offer)
		if (vendorData->getOwnershipRightsOf(player) == 1) {
			item->setStatus(AuctionItem::OFFERED);
			item->setOfferToID(vendorData->getOwnerId());
			item->setExpireTime(vendorOfferExpire); // Infinity:  7 days
		} else {
			item->setExpireTime(vendorExpire);

			if (auctionMap->getVendorItemCount(vendor, true) == 0)
				sendVendorUpdateMail(vendor, false);
		}
	} else {
		if (auction) {
			if (duration < ConfigManager::instance()->getAuctionExpirePeriod()) { // Infinity:  Created custom auction duration of 7 days
				item->setExpireTime(time(0) + duration);
			} else {
				item->setExpireTime(auctionExpire);
			}
		} else {
			item->setExpireTime(commodityExpire); // Instant Sale
		}
	}

	ObjectManager::instance()->persistObject(item, 0, "auctionitems");
	updateAuctionOwner(item, player);

	return item;
}

int AuctionManagerImplementation::checkBidAuction(CreatureObject* player, AuctionItem* item, int price1, int price2) {
	if ((price1 > MAXBAZAARPRICE || price2 > MAXBAZAARPRICE) && item->isOnBazaar()) {
		return BidAuctionResponseMessage::PRICEOVERFLOW;
	}

	if (price1 < 1 || price2 < 1 || price1 < item->getPrice()) {
		return BidAuctionResponseMessage::INVALIDPRICE;
	}

	if (player->getBankCredits() < price1) { // Credit Check
		return BidAuctionResponseMessage::NOTENOUGHCREDITS;
	}

	return 0;
}

void AuctionManagerImplementation::doInstantBuy(CreatureObject* player, AuctionItem* item) {
	if (zoneServer == nullptr) {
		return;
	}

	ManagedReference<SceneObject*> vendor = zoneServer->getObject(item->getVendorID());

	if (vendor == nullptr) {
		return;
	}

	int tax = 0;
	ManagedReference<CityRegion*> city = nullptr;
	String vendorPlanetName("@planet_n:" + vendor->getZone()->getZoneName());
	String vendorRegionName = vendorPlanetName;

	city = vendor->getCityRegion().get();

	if (city != nullptr) {
		tax = item->getPrice() - (int)ceil(item->getPrice() / (1.0f + (city->getSalesTax() / 100.f))); // Infinity:  Added ceil for accuracy
		vendorRegionName = city->getCityRegionName();
	}

	String playername = player->getFirstName().toLowerCase();

	auto playerManager = zoneServer->getPlayerManager();
	auto seller = playerManager != nullptr ? playerManager->getPlayer(item->getOwnerName()) : nullptr;

	String sender = "auctioner";
	String sellerName = item->getOwnerName();

	Time expireTime;
	uint64 currentTime = expireTime.getMiliTime() / 1000;
	uint64 availableTime = 0;

	Locker locker(item);

	if (item->isOnBazaar() || item->getStatus() == AuctionItem::OFFERED)
		availableTime = currentTime + ConfigManager::instance()->getCommodityExpirePeriod();
	else
		availableTime = currentTime + ConfigManager::instance()->getVendorExpirePeriod();

	updateAuctionOwner(item, player);

	item->setStatus(AuctionItem::SOLD);
	item->setExpireTime(availableTime);
	item->setBuyerID(player->getObjectID());
	item->setBidderName(playername);
	item->clearAuctionWithdraw();

	TransactionLog trx(player, seller, TrxCode::INSTANTBUY, item->getPrice(), false);
	trx.setAutoCommit(false);
	trx.addRelatedObject(item->getAuctionedItemObjectID(), true);
	trx.setExportRelatedObjects(true);
	player->subtractBankCredits(item->getPrice());

	BaseMessage* msg = new BidAuctionResponseMessage(item->getAuctionedItemObjectID(), 0);
	player->sendMessage(msg);

	// Waypoint to Vendor / bazaar
	float waypointX = vendor->getWorldPositionX();
	float waypointY = vendor->getWorldPositionY();

	WaypointChatParameter waypointParam;
	waypointParam.set(vendor->getDisplayedName(), waypointX, 0, waypointY, vendor->getPlanetCRC());

	String itemName = removeColorCodes(item->getItemName());






			for (int i = 0; i < object->getContainerObjectsSize(); ++i) {

	}

	auto chatManager = zoneServer->getChatManager();

	if (chatManager != nullptr) {
		if (!item->isOnBazaar()) {
			// Setup the mail to the vendor owner

			/*ManagedReference<CreatureObject*> seller = playerManager->getPlayer(item->getOwnerName());

			Locker _locker(seller);*/

			StringIdChatParameterVector sellerBodyVector;
			WaypointChatParameterVector sellerWaypointVector;

			UnicodeString sellerSubject("@auction:subject_vendor_seller");			// Vendor Sale Complete
			StringIdChatParameter sellerBodySale("@auction:seller_success_vendor"); // %TU has sold %TO to %TT for %DI credits.
			sellerBodySale.setTU(vendor->getDisplayedName());
			sellerBodySale.setTO(itemName);
			sellerBodySale.setTT(item->getBidderName());
			sellerBodySale.setDI(item->getPrice());

			StringIdChatParameter sellerBodyLoc("@auction:seller_success_location"); // The sale took place at %TT, on %TO.
			sellerBodyLoc.setTO(vendorPlanetName);
			sellerBodyLoc.setTT(vendorRegionName);

			sellerBodyVector.add(sellerBodySale);
			sellerBodyVector.add(sellerBodyLoc);

			sellerWaypointVector.add(waypointParam);

			// Setup the mail to the buyer
			/*ManagedReference<CreatureObject*> buyer = playerManager->getPlayer(item->getBidderName());

			Locker _locker2(buyer);*/

			StringIdChatParameterVector buyerBodyVector;
			WaypointChatParameterVector buyerWaypointVector;

			UnicodeString buyerSubject("@auction:subject_vendor_buyer");   // Vendor Item Purchased
			StringIdChatParameter buyerBodySale("@auction:buyer_success"); // You have won the auction of "%TO" from "%TT" for %DI credits. See the attached waypoint for location.
			buyerBodySale.setTO(itemName);
			buyerBodySale.setTT(sellerName);
			buyerBodySale.setDI(item->getPrice());

			StringIdChatParameter buyerBodyLoc("@auction:buyer_success_location"); // The sale took place at %TT, on %TO.
			buyerBodyLoc.setTO(vendorPlanetName);
			buyerBodyLoc.setTT(vendorRegionName);

			buyerBodyVector.add(buyerBodySale);
			buyerBodyVector.add(buyerBodyLoc);

			buyerWaypointVector.add(waypointParam);

			// Send the Mail
			UnicodeString blankBody;
			chatManager->sendMail(sender, sellerSubject, blankBody, sellerName, &sellerBodyVector, &sellerWaypointVector);
			chatManager->sendMail(sender, buyerSubject, blankBody, item->getBidderName(), &buyerBodyVector, &buyerWaypointVector);

			if (auctionMap->getVendorItemCount(vendor, true) == 0) {
				sendVendorUpdateMail(vendor, true);
			}

		} else {
			StringIdChatParameterVector sellerBodyVector;
			WaypointChatParameterVector sellerWaypointVector;

			// Setup the mail to the seller
			UnicodeString sellerSubject("@auction:subject_instant_seller");	 // Instant Sale Complete
			StringIdChatParameter sellerBodySale("@auction:seller_success"); // Your auction of %TO has been sold to %TT for %DI credits
			sellerBodySale.setTO(itemName);
			sellerBodySale.setTT(item->getBidderName());
			sellerBodySale.setDI(item->getPrice());

			StringIdChatParameter sellerBodyLoc("@auction:seller_success_location"); // The sale took place at %TT, on %TO.
			sellerBodyLoc.setTO(vendorPlanetName);
			sellerBodyLoc.setTT(vendorRegionName);

			sellerBodyVector.add(sellerBodySale);
			sellerBodyVector.add(sellerBodyLoc);

			sellerWaypointVector.add(waypointParam);

			// Setup the mail to the buyer
			StringIdChatParameterVector buyerBodyVector;
			WaypointChatParameterVector buyerWaypointVector;

			UnicodeString buyerSubject("@auction:subject_instant_buyer");  // Instant Sale Item Purchased
			StringIdChatParameter buyerBodySale("@auction:buyer_success"); // You have won the auction of "%TO" from "%TT" for %DI credits. See the attached waypoint for location.
			buyerBodySale.setTO(itemName);
			buyerBodySale.setTT(sellerName);
			buyerBodySale.setDI(item->getPrice());

			StringIdChatParameter buyerBodyLoc("@auction:buyer_success_location"); // The sale took place at %TT, on %TO.
			buyerBodyLoc.setTO(vendorPlanetName);
			buyerBodyLoc.setTT(vendorRegionName);

			buyerBodyVector.add(buyerBodySale);
			buyerBodyVector.add(buyerBodyLoc);

			buyerWaypointVector.add(waypointParam);

			// Send the Mail
			UnicodeString blankBody;
			chatManager->sendMail(sender, sellerSubject, blankBody, sellerName, &sellerBodyVector, &sellerWaypointVector);
			chatManager->sendMail(sender, buyerSubject, blankBody, item->getBidderName(), &buyerBodyVector, &buyerWaypointVector);
		}
	}

	if (seller == nullptr) {
		// doInstantBuy(CreatureObject* player, AuctionItem* item)
		trx.errorMessage() << "Null Seller: " + item->getOwnerName();
		trx.commit();
		error("seller null for name " + item->getOwnerName());

		error() << "doInstantBuy(player=" << player->getObjectID() << ", item=" << item->getObjectID() << "): Seller not found [" << item->getOwnerName() << "], auctionItem: " << *item;
		return;
	}

	locker.release();

	Locker slocker(seller);
	seller->addBankCredits(item->getPrice());
	trx.commit();

	if (city != nullptr && tax > 0) {
		TransactionLog trxFee(seller, TrxCode::CITYSALESTAX, tax, false);
		trxFee.groupWith(trx);
		trxFee.addState("cityRegionID", city->getObjectID());
		seller->subtractBankCredits(tax);
	}
	slocker.release();

	if (city != nullptr && !city->isClientRegion() && tax) {
		Locker clock(city);
		city->addToCityTreasury(tax);
	}
}

void AuctionManagerImplementation::doAuctionBid(CreatureObject* player, AuctionItem* item, int price1, int proxyBid) {
	if (price1 <= item->getPrice() || proxyBid <= price1) {
		BaseMessage* msg = new BidAuctionResponseMessage(item->getAuctionedItemObjectID(), 1);
		player->sendMessage(msg);
		return;
	}

	String playername = player->getFirstName().toLowerCase();

	auto playerManager = zoneServer->getPlayerManager();
	// don't allow owner bid on the item.  don't allow old auction info
	// send auctioner invalid message
	if (playername == item->getOwnerName() || price1 <= item->getPrice() || proxyBid <= price1) {
		BaseMessage* msg = new BidAuctionResponseMessage(item->getAuctionedItemObjectID(), 1);
		player->sendMessage(msg);
		return;
	}

	ManagedReference<CreatureObject*> priorBidder = playerManager != nullptr ? playerManager->getPlayer(item->getBidderName()) : nullptr;

	/// Use previous proxy
	if (priorBidder != nullptr && proxyBid < item->getProxy()) {
		Locker locker(item);
		Locker plocker(priorBidder);

		int increase = price1 - item->getPrice();

		int fullPrice = proxyBid + increase - item->getPrice();

		// TODO: prior didnt have enough money -> assert.. fix properly
		if (priorBidder->getBankCredits() < fullPrice) {
			BaseMessage* msg = new BidAuctionResponseMessage(item->getAuctionedItemObjectID(), BidAuctionResponseMessage::NOTENOUGHCREDITS);
			player->sendMessage(msg);

			return;
		}

		TransactionLog trx(priorBidder, TrxCode::AUCTIONBID, fullPrice, false);
		priorBidder->subtractBankCredits(fullPrice);
		item->setPrice(proxyBid + increase);
		BaseMessage* msg = new BidAuctionResponseMessage(item->getAuctionedItemObjectID(), BidAuctionResponseMessage::SUCCEDED);
		player->sendMessage(msg);

		if (priorBidder != player) {
			StringIdChatParameter body("@auction:bidder_outbid");
			body.setTO(item->getItemName());
			priorBidder->sendSystemMessage(body);
		}

		return;
	}

	Locker locker(item);
	Locker plocker(player);

	if (player->getBankCredits() < price1 || player->getBankCredits() < item->getPrice()) {
		BaseMessage* msg = new BidAuctionResponseMessage(item->getAuctionedItemObjectID(), BidAuctionResponseMessage::NOTENOUGHCREDITS);
		player->sendMessage(msg);

		return;
	}

	item->setProxy(proxyBid);

	// send prior bidder their money back
	if (item->getBidderName().length() > 0) {
		String itemName = removeColorCodes(item->getItemName());

		StringIdChatParameter bidderBody("@auction:bidder_outbid"); // You have been outbid on the "%TO" that you were bidding on.
		bidderBody.setTO(itemName);

		// mail prior bidder with outcome
		UnicodeString bidderSubject("@auction:subject_auction_outbid"); // Auction Outbid

		item->setPrice(price1);
		item->setBuyerID(player->getObjectID());
		item->setBidderName(playername);

		// take money from high bidder
		TransactionLog trx(player, TrxCode::AUCTIONBID, item->getPrice(), false);
		player->subtractBankCredits(item->getPrice());

		if (priorBidder != nullptr) {
			Locker clocker(priorBidder, player);

			if (priorBidder != player)
				priorBidder->sendSystemMessage(bidderBody);

			TransactionLog trx(TrxCode::AUCTIONBID, priorBidder, item->getPrice(), false);
			priorBidder->addBankCredits(item->getPrice());
		}

		plocker.release();
		locker.release();

		auto chatManager = zoneServer->getChatManager();
		if (chatManager != nullptr) {
			String sender = "auctioner";
			chatManager->sendMail(sender, bidderSubject, bidderBody, item->getBidderName());
		}

		// no prior bidder, just take the money
	} else {
		item->setPrice(price1);
		item->setBuyerID(player->getObjectID());
		item->setBidderName(playername);

		TransactionLog trx(player, TrxCode::AUCTIONBID, item->getPrice(), false);
		player->subtractBankCredits(item->getPrice());

		trx.addRelatedObject(item->getAuctionedItemObjectID());
		trx.addState("itemName", removeColorCodes(item->getItemName()));
		trx.addState("itemType", item->getItemType());
		trx.addState("sellerName", item->getOwnerName());
	}

	BaseMessage* msg = new BidAuctionResponseMessage(item->getAuctionedItemObjectID(), 0);
	player->sendMessage(msg);
}

void AuctionManagerImplementation::buyItem(CreatureObject* player, uint64 objectid, int price1, int price2) {
	ManagedReference<AuctionItem*> item = auctionMap->getItem(objectid);

	if (item == nullptr) {
		BaseMessage* msg = new BidAuctionResponseMessage(objectid, BidAuctionResponseMessage::INVALIDITEM);
		player->sendMessage(msg);
		return;
	}

	ManagedReference<SceneObject*> vendor = zoneServer->getObject(item->getVendorID());

	if (vendor == nullptr || item->getStatus() == AuctionItem::SOLD) {
		BaseMessage* msg = new BidAuctionResponseMessage(objectid, BidAuctionResponseMessage::INVALIDITEM);
		player->sendMessage(msg);
		return;
	}

	if (item->getOwnerID() == player->getObjectID()) {
		BaseMessage* msg = new BidAuctionResponseMessage(objectid, BidAuctionResponseMessage::PURCHASEFAILED);
		player->sendMessage(msg);
		return;
	}

	ManagedReference<CityRegion*> city = vendor->getCityRegion().get();

	int totalPrice = item->getPrice();

	int res = checkBidAuction(player, item, totalPrice, price2);

	if (res != 0) {
		BaseMessage* msg = new BidAuctionResponseMessage(objectid, res);
		player->sendMessage(msg);
		return;
	}

	if (!item->isAuction()) { // Instant buy
		doInstantBuy(player, item);
	} else { // For Auction Bids
		if (price1 < 1) {
			BaseMessage* msg = new BidAuctionResponseMessage(objectid, BidAuctionResponseMessage::INVALIDPRICE);
			player->sendMessage(msg);

			return;
		}

		doAuctionBid(player, item, price1, price2);
	}
}

int AuctionManagerImplementation::checkRetrieve(CreatureObject* player, uint64 objectIdToRetrieve, SceneObject* vendor) {
	// Check both Bazaar and Vendors

	if (!auctionMap->containsItem(objectIdToRetrieve))
		return RetrieveAuctionItemResponseMessage::NOTALLOWED;

	ManagedReference<SceneObject*> saleItem = zoneServer->getObject(objectIdToRetrieve);

	if (saleItem == nullptr) {
		return RetrieveAuctionItemResponseMessage::NOTALLOWED;
	}

	ManagedReference<AuctionItem*> item = auctionMap->getItem(objectIdToRetrieve);
	if (item == nullptr || item->getStatus() == AuctionItem::RETRIEVED) {
		return RetrieveAuctionItemResponseMessage::NOTALLOWED;
	}

	if (item->isAuction() && item->getStatus() == AuctionItem::FORSALE)
		return RetrieveAuctionItemResponseMessage::NOTALLOWED;

	int size = item->getSize();
	if (saleItem->isIntangibleObject()) {
		ManagedReference<SceneObject*> datapad = player->getSlottedObject("datapad");
		if (datapad->getCountableObjectsRecursive() + size > datapad->getContainerVolumeLimit())
			return RetrieveAuctionItemResponseMessage::FULLINVENTORY;
	} else {
		ManagedReference<SceneObject*> inventory = player->getSlottedObject("inventory");

		if (inventory->getCountableObjectsRecursive() + size > inventory->getContainerVolumeLimit())
			return RetrieveAuctionItemResponseMessage::FULLINVENTORY;
	}

	/*
	if(item->getStatus() == AuctionItem::SOLD && player->getObjectID() == item->getOwnerID()) {
		item->setStatus(AuctionItem::EXPIRED);
		return RetrieveAuctionItemResponseMessage::DONTRETRIEVE;
	}
	*/

	String playername = player->getFirstName();

	// only the owner can yank his own auction off the vendor
	if (item->getStatus() != AuctionItem::SOLD && (player->getObjectID() != item->getOwnerID()))
		return RetrieveAuctionItemResponseMessage::NOTALLOWED;

	// the bidder is the only one who can get his auction after expiration
	if (item->getStatus() == AuctionItem::SOLD && item->getBuyerID() != player->getObjectID()) {
		player->sendSystemMessage(item->getBidderName() + " bought this, " + player->getFirstName() + " trying to retrieve");
		error(item->getBidderName() + " bought this, " + player->getFirstName() + " trying to retrieve");
		return RetrieveAuctionItemResponseMessage::NOTALLOWED;
	}

	if (vendor->isVendor() && !vendor->isInRange(player, 8.0f))
		return RetrieveAuctionItemResponseMessage::TOOFAR;

	if (vendor->isBazaarTerminal()) {
		ManagedReference<CityRegion*> cityRegion = vendor->getCityRegion().get();

		String location = vendor->getZone()->getZoneName() + ".";

		if (cityRegion != nullptr) {
			location += cityRegion->getCityRegionName();
			// String region = terminal->getBazaarRegion();

			if (!item->getVendorUID().beginsWith(location)) {
				return RetrieveAuctionItemResponseMessage::TOOFAR;
			}
		} else {
			StringBuffer msg;
			msg << "null area for bazaar terminal at" << vendor->getPositionX() << " " << vendor->getPositionY() << " zone " << vendor->getZone()->getZoneName();
			error(msg);
		}

		if (!item->getVendorUID().beginsWith(location)) {
			return RetrieveAuctionItemResponseMessage::TOOFAR;
		}
	}

	return 0;
}

void AuctionManagerImplementation::refundAuction(AuctionItem* item) {
	auto playerManager = zoneServer->getPlayerManager();
	ManagedReference<CreatureObject*> bidder = playerManager != nullptr ? playerManager->getPlayer(item->getBidderName()) : nullptr;

	String itemName = removeColorCodes(item->getItemName());

	// send the player a mail and system message
	UnicodeString buyerSubject("@auction:subject_auction_cancelled");									  // Auction Cancelled
	Reference<StringIdChatParameter*> buyerBody = new StringIdChatParameter("auction", "buyer_canceled"); // The auction of "%TO" that you were bidding on has been canceled by %TT.
	buyerBody->setTO(itemName);
	buyerBody->setTT(item->getOwnerName());

	if (bidder != nullptr) {
		int itemPrice = item->getPrice();

		Core::getTaskManager()->executeTask(
			[=]() {
				Locker locker(bidder);

				TransactionLog trx(TrxCode::AUCTIONBID, bidder, itemPrice, false);
				bidder->addBankCredits(itemPrice);
				bidder->sendSystemMessage(*(buyerBody.get()));
			},
			"RefundAuctionLambda");
	}

	auto chatManager = zoneServer->getChatManager();
	if (chatManager != nullptr) {
		String sender = "auctioner";
		chatManager->sendMail(sender, buyerSubject, *(buyerBody.get()), item->getBidderName());
	}
}

void AuctionManagerImplementation::retrieveItem(CreatureObject* player, uint64 objectid, uint64 vendorID) {
	ManagedReference<SceneObject*> vendor = zoneServer->getObject(vendorID);
	RetrieveAuctionItemResponseMessage* msg = nullptr;

	// check for valid vendor terminal
	if (vendor == nullptr || (!vendor->isVendor() && !vendor->isBazaarTerminal())) {
		msg = new RetrieveAuctionItemResponseMessage(objectid, RetrieveAuctionItemResponseMessage::NOTALLOWED);
		player->sendMessage(msg);
		return;
	}

	int res = checkRetrieve(player, objectid, vendor);

	if (res != 0) {
		if (res == RetrieveAuctionItemResponseMessage::DONTRETRIEVE)
			res = 0;
		msg = new RetrieveAuctionItemResponseMessage(objectid, res);
		player->sendMessage(msg);
		return;
	}

	ManagedReference<AuctionItem*> item = auctionMap->getItem(objectid);
	if (item == nullptr) {
		error() << "retrieveItem(player=" << player->getObjectID() << ", objectid=" << objectid << ", vendorID=" << vendorID << "): Item not found in auctionMap";
		msg = new RetrieveAuctionItemResponseMessage(objectid, RetrieveAuctionItemResponseMessage::NOTALLOWED);
		player->sendMessage(msg);
		return;
	}

	// INFINITY-CLIFFY-DEBUG: log retrieve attempts
	if (player->getObjectID() == 281476564742784ULL) {
		ManagedReference<SceneObject*> debugObj = zoneServer->getObject(objectid);
		info(true) << "CLIFFY-RETRIEVE: item=" << objectid << " name=[" << item->getItemName() << "]"
				   << " status=" << item->getStatusString() << " sceneObject=" << (debugObj != nullptr ? "exists" : "NULL") << " vendor=" << vendorID;
	}

	ManagedReference<SceneObject*> objectToRetrieve = zoneServer->getObject(objectid);

	if (objectToRetrieve == nullptr) {
		error() << "retrieveItem(player=" << player->getObjectID() << ", objectid=" << objectid << ", vendorID=" << vendorID << "): invalid object.";
		msg = new RetrieveAuctionItemResponseMessage(objectid, 0);
		player->sendMessage(msg);
		return;
	}

	Locker locker(item);
	Locker plocker(player);

	ManagedReference<SceneObject*> destination = nullptr;

	if (objectToRetrieve->isIntangibleObject())
		destination = player->getSlottedObject("datapad");
	else
		destination = player->getSlottedObject("inventory");

	TransactionLog trx(vendor, player, objectToRetrieve, TrxCode::AUCTIONRETRIEVE);
	trx.addRelatedObject(objectid, true);
	trx.setExportRelatedObjects(true);

	if (destination->transferObject(objectToRetrieve, -1, false)) {
		trx.commit();
		destination->broadcastObject(objectToRetrieve, true);

		item->setStatus(AuctionItem::RETRIEVED);

		auctionMap->deleteItem(vendor, item);
		auctionMap->removeFromCommodityLimit(item);

		item->setAuctionedItemObjectID(0);

		msg = new RetrieveAuctionItemResponseMessage(objectid, 0);
		player->sendMessage(msg);
	} else {
		trx.abort() << "transferObject failed";
		msg = new RetrieveAuctionItemResponseMessage(objectid, RetrieveAuctionItemResponseMessage::NOTALLOWED);
		player->sendMessage(msg);
	}
}

bool AuctionManagerImplementation::checkItemCategory(int category, AuctionItem* item) {
	if (item == nullptr)
		return false;

	int itemType = item->getItemType();
	bool isCrate = item->isFactoryCrate();
	int cratedItemType = item->getCratedItemType();

	if (category & 255) { // Searching a sub category
		if (itemType == category || (isCrate && cratedItemType > 0 && cratedItemType == category)) {
			return true;
		}
	} else if ((itemType & category) || (isCrate && cratedItemType > 0 && (cratedItemType & category))) { // Searching main category
		return true;
	} else if ((category == 8192) && (itemType < 256 || (isCrate && cratedItemType < 256))) {
		return true;
	} else if (category == 0) { // Searching all items
		return true;
	}

	return false;
}
AuctionQueryHeadersResponseMessage* AuctionManagerImplementation::fillAuctionQueryHeadersResponseMessage(CreatureObject* player, SceneObject* vendor, TerminalListVector* terminalList, int searchType, uint32 itemCategory, const UnicodeString& filterText, int minPrice, int maxPrice, bool includeEntranceFee, int clientCounter, int offset) {
	AuctionQueryHeadersResponseMessage* reply = new AuctionQueryHeadersResponseMessage(searchType, clientCounter, player);

#ifdef DEBUG_AUCTION_SEARCH
	Logger::console.info(true) << __FUNCTION__ << "()"
							   << " player=" << player->getObjectID() << "; vendor=" << vendor->getObjectID() << "; terminalListSize=" << terminalList->size() << "; searchType=" << searchType << "; itemCategory=" << itemCategory << "; filterText=" << filterText.toString() << "; minPrice=" << minPrice
							   << "; maxPrice=" << maxPrice << "; includeEntranceFee=" << includeEntranceFee << "; clientCounter=" << clientCounter << "; offset=" << offset;
#endif // DEBUG_AUCTION_SEARCH

	String pname = player->getFirstName().toLowerCase();
	uint32 now = time(0);
	int displaying = 0;

	if (!isMarketEnabled()) {
		player->sendSystemMessage("@ui_auc:err_vendor_terminal_error"); // This market is unavailable.
		reply->createMessage(offset, true);
		return reply;
	}

	for (int j = 0; (j < terminalList->size()) && (displaying < (offset + 100)); ++j) {
		auto& items = terminalList->get(j);

		if (items == nullptr) {
			continue;
		}

		/// Exclude non-searchable vendor Items
		if (vendor->isBazaarTerminal() && searchType == ST_VENDOR_SELLING && !items->isSearchable()) {
			continue;
		}

		try {
			items->rlock();

			for (int i = 0; (i < items->size()) && (displaying < (offset + 100)); i++) {
				ManagedReference<AuctionItem*>& item = items->get(i);

				if (item == nullptr) {
					continue;
				}

				if (item->getStatus() == AuctionItem::DELETED || item->getStatus() == AuctionItem::RETRIEVED) {
					info() << "fillAuctionQueryHeadersResponseMessage(): Skipping " << item->getStatusString() << " auctionItem: " << *item;
					continue;
				}

				if (!item->isAuction() && item->getExpireTime() <= now) {
					Core::getTaskManager()->executeTask([=]() { expireSale(item); }, "ExpireSaleLambda");

					continue;
				}

				switch (searchType) {
					case ST_VENDOR_SELLING: { // Vendor search Bazaar && Vendor

						if (vendor->isVendor() && item->getVendorID() != vendor->getObjectID()) {
							if (item->getOwnerID() != player->getObjectID())
								continue;
						}
					}
					case ST_ALL: { // All Auctions (Bazaar)
						if (item->getStatus() == AuctionItem::FORSALE) {
							if (!checkItemCategory(itemCategory, item)) {
								continue;
							}

							if (minPrice != 0 || maxPrice != 0) {
								int itemPrice = item->getPrice();

								if (includeEntranceFee) {
									ManagedReference<SceneObject*> itemVendor = player->getZoneServer()->getObject(item->getVendorID());

									if (itemVendor != nullptr && itemVendor->isVendor()) {
										int accessFee = 0;
										ManagedReference<SceneObject*> parent = itemVendor->getRootParent();

										if (parent != nullptr && parent->isBuildingObject()) {
											BuildingObject* building = cast<BuildingObject*>(parent.get());

											if (building != nullptr)
												accessFee = building->getAccessFee();
										}

										itemPrice += accessFee;
									}
								}

								if ((minPrice != 0 && itemPrice < minPrice) || (maxPrice != 0 && itemPrice > maxPrice))
									continue;
							}

							if (!filterText.isEmpty()) {
								String lowerFilter = filterText.toString().toLowerCase();
								String itemName = item->getItemName().toLowerCase();

								if (itemName.indexOf(lowerFilter) == -1)
									continue;
							}

							if (displaying >= offset) {
								reply->addItemToList(item);
							}

							displaying++;
						}
						break;
					}
					case ST_PLAYER_SALES: { // My auctions/sales
						if (item->getStatus() == AuctionItem::FORSALE && (item->getOwnerID() == player->getObjectID())) {
							if (checkItemCategory(itemCategory, item)) {
								if (displaying >= offset) {
									reply->addItemToList(item);
								}

								displaying++;
							}
						}
						break;
					}
					case ST_PLAYER_BIDS: { // My Bids
						if (item->isAuction() && item->getStatus() == AuctionItem::FORSALE && (item->getBidderName() == pname)) {
							reply->addItemToList(item);
						}
						break;
					}
					case ST_PLAYER_STOCKROOM: { // Retrieve items screen
						if ((item->getStatus() == AuctionItem::SOLD && item->getBuyerID() == player->getObjectID()) || (item->getStatus() == AuctionItem::EXPIRED && item->getOwnerID() == player->getObjectID())) {
							// INFINITY-CLIFFY-DEBUG: targeted logging for ghost stockroom items
							if (player->getObjectID() == 281476564742784ULL) {
								info(true) << "CLIFFY-STOCKROOM: playerStockroom item=" << item->getAuctionedItemObjectID() << " name=[" << item->getItemName() << "]"
										   << " status=" << item->getStatusString() << " owner=" << item->getOwnerID() << " buyer=" << item->getBuyerID() << " vendor=" << item->getVendorID() << " price=" << item->getPrice() << " expired=" << item->getExpireTime();
							}
							reply->addItemToList(item);
						}
						break;
					}
					case ST_VENDOR_OFFERS: { // Offers to Vendor (vendor owner)
						if (item->getStatus() == AuctionItem::OFFERED && item->getOfferToID() == player->getObjectID()) {
							if (checkItemCategory(itemCategory, item)) {
								if (displaying >= offset) {
									reply->addItemToList(item);
								}

								displaying++;
							}
						}
						break;
					}
					case ST_VENDOR_STOCKROOM: { // Stockroom
						if ((item->getStatus() == AuctionItem::EXPIRED && item->getOwnerID() == player->getObjectID()) || (item->getStatus() == AuctionItem::SOLD && item->getBuyerID() == player->getObjectID())) {
							if (checkItemCategory(itemCategory, item)) {
								// INFINITY-CLIFFY-DEBUG: targeted logging for ghost stockroom items
								if (player->getObjectID() == 281476564742784ULL) {
									info(true) << "CLIFFY-STOCKROOM: vendorStockroom item=" << item->getAuctionedItemObjectID() << " name=[" << item->getItemName() << "]"
											   << " status=" << item->getStatusString() << " owner=" << item->getOwnerID() << " buyer=" << item->getBuyerID() << " vendor=" << item->getVendorID() << " price=" << item->getPrice() << " expired=" << item->getExpireTime();
								}
								if (displaying >= offset) {
									reply->addItemToList(item);
								}

								displaying++;
							}
						}
						break;
					}
					case ST_PLAYER_OFFERS_TO_VENDOR: { // Offers to vendor (browsing player)
						if (item->getStatus() == AuctionItem::OFFERED && item->getOwnerID() == player->getObjectID()) {
							if (checkItemCategory(itemCategory, item)) {
								if (displaying >= offset) {
									reply->addItemToList(item);
								}

								displaying++;
							}
						}
						break;
					}
				}
			}
			items->runlock();
		} catch (Exception& e) {
			error(e.getMessage());
			items->runlock();
		}
	}

	if (displaying == (offset + 100)) {
		reply->createMessage(offset, true);
	} else {
		reply->createMessage(offset);
	}

	return reply;
}

void AuctionManagerImplementation::getData(CreatureObject* player, int locationType, uint64 vendorObjectID, int searchType, unsigned int itemCategory, const UnicodeString& filterText, int minPrice, int maxPrice, bool includeEntranceFee, int clientCounter, int offset) {
	auto errorMessage = [=]() -> LoggerHelper {
		auto msg = error();

		msg << "getData(player=" << player->getObjectID() << ", locationType=" << locationType << ", vendor=" << vendorObjectID << ", searchType=" << searchType << ", itemCategory=" << itemCategory << ", filterText=[" << filterText << "]"
			<< ", minPrice=" << minPrice << ", maxPrice=" << maxPrice << ", includeEntranceFee=" << includeEntranceFee << ", clientCounter=" << clientCounter << ", offset=" << offset << "): ";

		return msg;
	};

	// Infinity: Cancel any pending search task instead of silently dropping the
	// new request. The client sends rapid queries when switching tabs/filters,
	// and silently dropping them leaves the client with no response, causing
	// offers to flash then disappear or become permanently invisible.
	Reference<AuctionSearchTask*> pendingTask = player->getAuctionSearchTask().get();
	if (pendingTask != nullptr) {
		pendingTask->cancel();
		player->setAuctionSearchTask(nullptr);
	}

	ManagedReference<TangibleObject*> vendorInUse = (zoneServer->getObject(vendorObjectID)).castTo<TangibleObject*>();

	if (vendorInUse == nullptr || (!vendorInUse->isVendor() && !vendorInUse->isBazaarTerminal())) {
		errorMessage() << "Vendor not found.";
		return;
	}

	ManagedReference<SceneObject*> parent = vendorInUse->getRootParent();

	if (parent != nullptr && parent != player->getRootParent())
		return;

	if (player->getZone() == nullptr) {
		errorMessage() << "Player not in a zone.";
		return;
	}

	ManagedReference<BuildingObject*> rootParent = cast<BuildingObject*>(parent.get());
	if (rootParent != nullptr && !rootParent->isAllowedEntry(player))
		return;

	// Handle Merchant XP for players using other players vendors...
	if (!vendorInUse->isBazaarTerminal()) {
		DataObjectComponentReference* data = vendorInUse->getDataObjectComponent();
		if (data == nullptr || data->get() == nullptr || !data->get()->isVendorData()) {
			errorMessage() << "Vendor has no data component.";
			return;
		}

		VendorDataComponent* vendorData = cast<VendorDataComponent*>(data->get());
		if (vendorData == nullptr) {
			errorMessage() << "Vendor has wrong data component.";
			return;
		}

		vendorData->awardUseXP();
	}

	String planet = "";
	String region = "";
	ManagedReference<SceneObject*> vendor = nullptr;
	ManagedReference<CityRegion*> city = nullptr;

	// Infinity: The fall-through here is intentional — each case narrows the
	// filter by adding variables: galaxy (nothing) → planet → region → vendor.
	switch (locationType) {
		case LT_MARKET:
			vendor = vendorInUse;
		case LT_REGION:
			city = player->getCityRegion().get();
			if (city != nullptr) {
				region = city->getCityRegionName();
			} else {
				// Infinity: When not in a city, use planet name as region fallback.
				// Do NOT set vendor here — "This Region" must query all vendors in
				// the region, not just the one being used. Setting vendor caused
				// offers to disappear when switching location filters (Core3 bug).
				region = "@planet_n:" + player->getZone()->getZoneName();
			}
		case LT_PLANET:
			planet = player->getZone()->getZoneName();
		default:
			break;
	}

	AuctionSearchTask* task = new AuctionSearchTask(_this.getReferenceUnsafeStaticCast(), player, vendorInUse, planet, region, vendor, searchType, itemCategory, filterText, minPrice, maxPrice, includeEntranceFee, clientCounter, offset);
	player->setAuctionSearchTask(task);
	task->schedule(100);
}

void AuctionManagerImplementation::getAuctionData(CreatureObject* player, SceneObject* usedVendor, const String& planet, const String& region, SceneObject* vendor, int searchType, uint32 itemCategory, const UnicodeString& filterText, int minPrice, int maxPrice, bool includeEntranceFee, int clientCounter, int offset) {
	TerminalListVector terminalList;

	if (usedVendor->isVendor()) {
		switch (searchType) {
			case ST_PLAYER_STOCKROOM:
			case ST_VENDOR_SELLING:
			case ST_PLAYER_OFFERS_TO_VENDOR:
				break;

			case ST_VENDOR_OFFERS:
			case ST_VENDOR_STOCKROOM: {
				auto data = usedVendor->getDataObjectComponent();

				if (data == nullptr) {
					error() << "Vendor " << usedVendor->getObjectID() << " has no data component";
					return;
				}

				auto vendorData = cast<VendorDataComponent*>(data->get());

				if (vendorData == nullptr) {
					error() << "Vendor " << usedVendor->getObjectID() << " has wrong data component";
					return;
				}

				if (vendorData->getOwnerId() == player->getObjectID()) {
					break;
				}
			}

			default:
				error() << "Player " << player->getObjectID() << " tried search type " << searchType << " on vendor " << usedVendor->getObjectID();

				StringIdChatParameter err("@cmd_err:target_range_prose"); // Your target is too far away to %TO.
				err.setTO("search");
				player->sendSystemMessage(err);

				return;
		}
	}

	if (usedVendor->isBazaarTerminal() && searchType != ST_VENDOR_SELLING) { // This is to prevent bazaar items from showing on Vendor Search
		terminalList = auctionMap->getBazaarTerminalData(planet, region, vendor);
	} else {
		terminalList = auctionMap->getVendorTerminalData(planet, region, vendor);
	}

	AuctionQueryHeadersResponseMessage* msg = fillAuctionQueryHeadersResponseMessage(player, usedVendor, &terminalList, searchType, itemCategory, filterText, minPrice, maxPrice, includeEntranceFee, clientCounter, offset);

	if (msg != nullptr) {
		player->sendMessage(msg);
	}
}

void AuctionManagerImplementation::getItemAttributes(CreatureObject* player, uint64 objectid) {
	Reference<AuctionItem*> auctionItem = auctionMap->getItem(objectid);

	if (auctionItem == nullptr)
		return;

	Reference<SceneObject*> object = zoneServer->getObject(auctionItem->getAuctionedItemObjectID());

	if (object == nullptr) {
		error() << "getItemAttributes(player=" << player->getObjectID() << ", objectid=" << objectid << ") getObject failed.";
		return;
	}

	UnicodeString description(auctionItem->getItemDescription());
	AttributeListMessage* msg = new AttributeListMessage(objectid, description);

	// For objects that don't fill the attribute list normally...
	if (object->getAttributeListComponent() != nullptr) {
		object->getAttributeListComponent()->fillAttributeList(msg, player, object);
	} else
		object->fillAttributeList(msg, player);

	PlayerObject* ghost = player->getPlayerObject();

	if (ghost != nullptr && ghost->isPrivileged()) {
		msg->insertAttribute("auctionitem_oid", String::valueOf(auctionItem->getObjectID()));
		msg->insertAttribute("item_type", auctionItem->getItemType());
		bool isCrate = auctionItem->isFactoryCrate();
		msg->insertAttribute("is_factory_crate", (isCrate ? "Yes" : "No"));

		if (isCrate) {
			msg->insertAttribute("crated_item_type", auctionItem->getCratedItemType());
		}
	}

	// msg->insertInt(0);
	String templateFile = TemplateManager::instance()->getTemplateFile(object->getClientObjectCRC());
	msg->insertAscii(templateFile);

	String cust = "";
	if (object->isTangibleObject()) {
		ManagedReference<TangibleObject*> tano = cast<TangibleObject*>(object.get());
		if (tano != nullptr)
			tano->getCustomizationString(cust);
	}

	msg->insertAscii(cust);

	player->sendMessage(msg);
}

void AuctionManagerImplementation::cancelItem(CreatureObject* player, uint64 objectID) {
	ManagedReference<AuctionItem*> item = auctionMap->getItem(objectID);

	// Item wasnt found. it doesn't exist
	if (item == nullptr) {
		error() << "cancelItem(player=" << player->getObjectID() << ", objectID=" << objectID << "): Item not found in auctionMap.";
		BaseMessage* msg = new CancelLiveAuctionResponseMessage(objectID, CancelLiveAuctionResponseMessage::INVALIDITEM);
		player->sendMessage(msg);
		return;
	}

	Time expireTime;
	uint64 currentTime = expireTime.getMiliTime() / 1000;
	uint64 availableTime = 0;
	bool forSaleOnVendor = false;

	if (item->getStatus() == AuctionItem::FORSALE) {
		if (item->getOwnerID() != player->getObjectID()) {
			error() << "cancelItem(player=" << player->getObjectID() << ", objectID=" << objectID << "): Not owner of item, owner: " << item->getOwnerID();
			BaseMessage* msg = new CancelLiveAuctionResponseMessage(objectID, CancelLiveAuctionResponseMessage::NOTOWNER);
			player->sendMessage(msg);
			return;
		}

		if (item->isOnBazaar())
			availableTime = currentTime + ConfigManager::instance()->getCommodityExpirePeriod();
		else {
			availableTime = currentTime + ConfigManager::instance()->getVendorExpirePeriod();
			forSaleOnVendor = true;
		}

	} else if (item->getStatus() == AuctionItem::OFFERED) {
		if (item->getOfferToID() != player->getObjectID() && item->getOwnerID() != player->getObjectID()) {
			error() << "cancelItem(player=" << player->getObjectID() << ", objectID=" << objectID << "): Not the target of offer, auctionItem: " << *item;
			BaseMessage* msg = new CancelLiveAuctionResponseMessage(objectID, CancelLiveAuctionResponseMessage::INVALIDITEM);
			player->sendMessage(msg);
			return;
		}
		/// 7 Days
		availableTime = currentTime + ConfigManager::instance()->getCommodityExpirePeriod();

	} else {
		BaseMessage* msg = new CancelLiveAuctionResponseMessage(objectID, CancelLiveAuctionResponseMessage::ALREADYCOMPLETED);
		player->sendMessage(msg);
		return;
	}

	if (item->getExpireTime() <= 0) {
		BaseMessage* msg = new CancelLiveAuctionResponseMessage(objectID, CancelLiveAuctionResponseMessage::ALREADYCOMPLETED);
		player->sendMessage(msg);
		return;
	}

	// refund auction money
	if (item->isAuction()) {
		refundAuction(item);
		Locker locker(&auctionEvents);

		if (auctionEvents.contains(item->getAuctionedItemObjectID())) {
			Reference<Task*> newTask = auctionEvents.get(item->getAuctionedItemObjectID());

			if (newTask != nullptr)
				newTask->cancel();

			auctionEvents.drop(item->getAuctionedItemObjectID());
		}
	}

	/// If the offeree cancels
	if (item->getStatus() == AuctionItem::OFFERED && item->getOfferToID() == player->getObjectID()) {
		ManagedReference<SceneObject*> vendor = zoneServer->getObject(item->getVendorID());
		auto chatManager = zoneServer->getChatManager();

		if (vendor != nullptr && chatManager != nullptr) {
			String sender = "auctioner";

			// Waypoint to Vendor / bazaar
			float waypointX = vendor->getWorldPositionX();
			float waypointY = vendor->getWorldPositionY();

			ManagedReference<WaypointObject*> waypoint = zoneServer->createObject(0xc456e788, 0).castTo<WaypointObject*>();
			Locker waypointLocker(waypoint);
			waypoint->setPlanetCRC(vendor->getPlanetCRC());
			waypoint->setPosition(waypointX, 0, waypointY);

			waypoint->setCustomObjectName(vendor->getDisplayedName(), false);

			String itemName = removeColorCodes(item->getItemName());

			UnicodeString sellerSubject("@auction:subject_auction_unsuccessful"); // Auction Unsuccessful
			StringIdChatParameter sellerBody("@auction:seller_fail");			  // Your auction of %TO has been completed and has not been purchased.
			sellerBody.setTO(itemName);

			// Send the Mail
			chatManager->sendMail(sender, sellerSubject, sellerBody, item->getOwnerName(), waypoint);
		}
	}

	Locker locker(item);

	item->setStatus(AuctionItem::EXPIRED);
	item->setExpireTime(availableTime);
	item->clearAuctionWithdraw();

	BaseMessage* msg = new CancelLiveAuctionResponseMessage(objectID, 0);
	player->sendMessage(msg);

	if (forSaleOnVendor) {
		ManagedReference<SceneObject*> vendor = zoneServer->getObject(item->getVendorID());

		if (vendor != nullptr && auctionMap->getVendorItemCount(vendor, true) == 0)
			sendVendorUpdateMail(vendor, true);
	}
}

void AuctionManagerImplementation::expireSale(AuctionItem* item) {
	Locker locker(item);

	if (item->getStatus() == AuctionItem::EXPIRED) {
		deleteExpiredSale(item);
		return;
	}

	auto chatManager = zoneServer->getChatManager();

	if (chatManager != nullptr) {
		String itemName = removeColorCodes(item->getItemName());

		// Send the mail to the vendor owner
		String sender = "auctioner";

		UnicodeString sellerSubject("@auction:subject_auction_unsuccessful"); // Auction Unsuccessful

		StringIdChatParameter sellerBody("@auction:seller_fail"); // Your auction of %TO has been completed and has not been purchased.
		sellerBody.setTO(itemName);

		Time expireTime;
		uint64 currentTime = expireTime.getMiliTime() / 1000;
		uint64 availableTime = 0;

		if (item->isOnBazaar()) {
			availableTime = currentTime + ConfigManager::instance()->getCommodityExpirePeriod();
		} else {
			availableTime = currentTime + ConfigManager::instance()->getVendorExpirePeriod();
		}

		item->setStatus(AuctionItem::EXPIRED);
		item->setExpireTime(availableTime);
		item->clearAuctionWithdraw();

		locker.release();

		chatManager->sendMail(sender, sellerSubject, sellerBody, item->getOwnerName());
	}

	if (!item->isOnBazaar()) {
		ManagedReference<SceneObject*> vendor = zoneServer->getObject(item->getVendorID());

		if (vendor != nullptr && auctionMap->getVendorItemCount(vendor, true) == 0) {
			sendVendorUpdateMail(vendor, true);
		}
	}
}

void AuctionManagerImplementation::expireBidAuction(AuctionItem* item) {
	ManagedReference<ChatManager*> chatManager = zoneServer->getChatManager();

	if (chatManager != nullptr) {
		Locker locker(item);

		String itemName = removeColorCodes(item->getItemName());

		// Send the mail to the vendor owner
		String sender = "auctioner";

		UnicodeString sellerSubject("@auction:subject_auction_item_expired"); // Auction Item Expired

		StringIdChatParameter sellerBody("@auction:seller_fail"); // Your auction of %TO has been completed and has not been purchased.
		sellerBody.setTO(itemName);

		Time expireTime;
		uint64 currentTime = expireTime.getMiliTime() / 1000;
		uint64 availableTime = 0;

		if (item->isOnBazaar()) {
			availableTime = currentTime + ConfigManager::instance()->getCommodityExpirePeriod();
		} else {
			availableTime = currentTime + ConfigManager::instance()->getVendorExpirePeriod();
		}

		item->setStatus(AuctionItem::EXPIRED);
		item->setExpireTime(availableTime);
		item->clearAuctionWithdraw();

		locker.release();

		chatManager->sendMail(sender, sellerSubject, sellerBody, item->getOwnerName());
	}
}

void AuctionManagerImplementation::expireAuction(AuctionItem* item) {
	ManagedReference<SceneObject*> vendor = zoneServer->getObject(item->getVendorID());

	if (vendor == nullptr) {
		return;
	}

	String playername = item->getBidderName();
	String sellerName = item->getOwnerName();

	auto playerManager = zoneServer->getPlayerManager();

	Zone* zone = vendor->getZone();
	String vendorPlanetName;

	if (zone != nullptr) {
		vendorPlanetName = "@planet_n:" + zone->getZoneName();
	}

	String vendorRegionName = vendorPlanetName;

	ManagedReference<CityRegion*> city = vendor->getCityRegion().get();
	if (city != nullptr) {
		vendorRegionName = city->getCityRegionName();
	}

	Time expireTime;
	uint64 currentTime = expireTime.getMiliTime() / 1000;
	uint64 availableTime = currentTime + ConfigManager::instance()->getCommodityExpirePeriod();

	Locker locker(item);
	item->setExpireTime(availableTime);
	item->clearAuctionWithdraw();

	if (playername.isEmpty()) {
		locker.release();
		expireBidAuction(item);

	} else {
		// Someone won the auction
		ManagedReference<CreatureObject*> buyer = playerManager != nullptr ? playerManager->getPlayer(item->getBidderName()) : nullptr;

		if (buyer == nullptr) {
			locker.release();
			expireBidAuction(item);
			return;
		}

		item->setStatus(AuctionItem::SOLD);
		updateAuctionOwner(item, buyer);

		auto chatManager = zoneServer->getChatManager();

		if (chatManager != nullptr) {
			// Waypoint to Vendor / bazaar
			float waypointX = vendor->getWorldPositionX();
			float waypointY = vendor->getWorldPositionY();

			WaypointChatParameter waypointParam;
			waypointParam.set(vendor->getDisplayedName(), waypointX, 0, waypointY, vendor->getPlanetCRC());

			String sender = "auctioner";

			StringIdChatParameterVector sellerBodyVector;
			WaypointChatParameterVector sellerWaypointVector;

			StringIdChatParameter sellerBodySale;
			UnicodeString sellerSubject;

			String itemName = removeColorCodes(item->getItemName());

			if (!item->isOnBazaar()) {
				sellerSubject = "@auction:subject_vendor_seller";			  // Vendor Sale Complete
				sellerBodySale.setStringId("@auction:seller_success_vendor"); // %TU has sold %TO to %TT for %DI credits.
				sellerBodySale.setTU(vendor->getDisplayedName());
			} else {
				sellerSubject = "@auction:subject_auction_seller";	   // Auction Complete
				sellerBodySale.setStringId("@auction:seller_success"); // Your auction of %TO has been sold to %TT for %DI credits
			}
			sellerBodySale.setTO(itemName);
			sellerBodySale.setTT(item->getBidderName());
			sellerBodySale.setDI(item->getPrice());

			StringIdChatParameter sellerBodyLoc("@auction:seller_success_location"); // The sale took place at %TT, on %TO.
			sellerBodyLoc.setTO(vendorPlanetName);
			sellerBodyLoc.setTT(vendorRegionName);

			sellerBodyVector.add(sellerBodySale);
			sellerBodyVector.add(sellerBodyLoc);

			sellerWaypointVector.add(waypointParam);

			StringIdChatParameterVector buyerBodyVector;
			WaypointChatParameterVector buyerWaypointVector;

			UnicodeString buyerSubject;
			if (!item->isOnBazaar()) {
				buyerSubject = "@auction:subject_vendor_buyer"; // Vendor Item Purchased
			} else {
				buyerSubject = "@auction:subject_auction_buyer"; // Auction Won
			}
			StringIdChatParameter buyerBodySale("@auction:buyer_success"); // You have won the auction of "%TO" from "%TT" for %DI credits. See the attached waypoint for location.
			buyerBodySale.setTO(itemName);
			buyerBodySale.setTT(sellerName);
			buyerBodySale.setDI(item->getPrice());

			StringIdChatParameter buyerBodyLoc("@auction:buyer_success_location"); // The sale took place at %TT, on %TO.
			buyerBodyLoc.setTO(vendorPlanetName);
			buyerBodyLoc.setTT(vendorRegionName);

			buyerBodyVector.add(buyerBodySale);
			buyerBodyVector.add(buyerBodyLoc);

			buyerWaypointVector.add(waypointParam);

			// Send the Mail
			locker.release();

			UnicodeString blankBody;
			chatManager->sendMail(sender, sellerSubject, blankBody, sellerName, &sellerBodyVector, &sellerWaypointVector);
			chatManager->sendMail(sender, buyerSubject, blankBody, item->getBidderName(), &buyerBodyVector, &buyerWaypointVector);
		}
	}
}

void AuctionManagerImplementation::deleteExpiredSale(AuctionItem* item, bool sendMail) {
	Locker locker(item);

	Reference<SceneObject*> vendor = zoneServer->getObject(item->getVendorID());
	auto chatManager = zoneServer->getChatManager();

	if (vendor != nullptr && sendMail && chatManager != nullptr) {
		String sender = "auctioner";

		// Waypoint to Vendor / bazaar
		float waypointX = vendor->getWorldPositionX();
		float waypointY = vendor->getWorldPositionY();

		Reference<WaypointObject*> waypoint = zoneServer->createObject(0xc456e788, 0).castTo<WaypointObject*>();

		Locker lockerWaypoint(waypoint);

		waypoint->setPlanetCRC(vendor->getPlanetCRC());
		waypoint->setPosition(waypointX, 0, waypointY);

		waypoint->setCustomObjectName(vendor->getDisplayedName(), false);

		lockerWaypoint.release();

		String itemName = removeColorCodes(item->getItemName());

		UnicodeString sellerSubject("@auction:subject_auction_unsuccessful"); // Auction Unsuccessful
		StringIdChatParameter sellerBody("@auction:item_expired");			  // Because you failed to pick it up in time, the "%TO" that you were auctioning has expired.
		sellerBody.setTO(itemName);

		// Send the Mail
		locker.release();

		chatManager->sendMail(sender, sellerSubject, sellerBody, item->getOwnerName(), waypoint);
	}

	TransactionLog trx(vendor, TrxCode::AUCTIONEXPIRED, zoneServer->getObject(item->getAuctionedItemObjectID()));
	trx.addRelatedObject(item->getAuctionedItemObjectID(), true);
	auctionMap->deleteItem(vendor, item, true);
}

void AuctionManagerImplementation::displayPlayerStockroom(CreatureObject* player, const String& targetName) {
	if (player == nullptr) {
		return;
	}

	auto playerManager = zoneServer->getPlayerManager();
	if (playerManager == nullptr) {
		return;
	}

	ManagedReference<CreatureObject*> target = playerManager->getPlayer(targetName);
	if (target == nullptr) {
		player->sendSystemMessage("Player '" + targetName + "' not found.");
		return;
	}

	uint64 targetOID = target->getObjectID();

	ManagedReference<SuiListBox*> list = new SuiListBox(player, SuiWindowType::MARKET_INFO, 0x00);
	list->setPromptTitle("Stockroom: " + targetName);
	list->setPromptText("Stockroom items for " + targetName + " (OID: " + String::valueOf(targetOID) + ")");

	int count = 0;
	int totalItems = auctionMap->getTotalItemCount();

	for (int i = 0; i < totalItems; i++) {
		ManagedReference<AuctionItem*> item = auctionMap->getItemByIndex(i);

		if (item == nullptr) {
			continue;
		}

		// Only stockroom items: EXPIRED (owner) or SOLD (buyer)
		bool expiredOwner = (item->getStatus() == AuctionItem::EXPIRED && item->getOwnerID() == targetOID);
		bool soldBuyer = (item->getStatus() == AuctionItem::SOLD && item->getBuyerID() == targetOID);

		if (!expiredOwner && !soldBuyer) {
			continue;
		}

		ManagedReference<SceneObject*> sceneObj = zoneServer->getObject(item->getAuctionedItemObjectID());
		ManagedReference<SceneObject*> vendorObj = zoneServer->getObject(item->getVendorID());
		String vendorName = (vendorObj != nullptr) ? vendorObj->getDisplayedName() : "unknown";
		String vendorZone = "?";
		String vendorCoords = "?,?";

		if (vendorObj != nullptr) {
			Zone* vZone = vendorObj->getZone();
			if (vZone != nullptr) {
				vendorZone = vZone->getZoneName();
				vendorCoords = String::valueOf((int)vendorObj->getWorldPositionX()) + ", " + String::valueOf((int)vendorObj->getWorldPositionY());
			}
		}

		StringBuffer line;
		line << item->getItemName() << " | item:" << item->getAuctionedItemObjectID() << " | " << vendorName << " (" << item->getVendorID() << ")"
			 << " | " << vendorZone << " " << vendorCoords
			 << " | " << item->getStatusString() << ((sceneObj == nullptr) ? " [GHOST]" : "");

		list->addMenuItem(line.toString());
		count++;

		info(true) << "ADMIN-STOCKROOM: player=" << targetName << " item=" << item->getAuctionedItemObjectID() << " name=[" << item->getItemName() << "]"
				   << " status=" << item->getStatusString() << " vendor=" << item->getVendorID() << " vendorName=" << vendorName << " sceneObject=" << ((sceneObj != nullptr) ? "exists" : "NULL");
	}

	list->addMenuItem("---");
	list->addMenuItem("Total items: " + String::valueOf(count));

	player->sendMessage(list->generateMessage());

	// Send results as in-game mail for easy OID copy/paste
	ManagedReference<ChatManager*> chatManager = zoneServer->getChatManager();
	if (chatManager != nullptr) {
		StringBuffer mailBody;
		mailBody << "Stockroom report for " << targetName << " (OID: " << targetOID << ")\n";
		mailBody << "Total items: " << count << "\n\n";

		int totalItems2 = auctionMap->getTotalItemCount();
		for (int i = 0; i < totalItems2; i++) {
			ManagedReference<AuctionItem*> mailItem = auctionMap->getItemByIndex(i);

			if (mailItem == nullptr) {
				continue;
			}

			bool expOwner = (mailItem->getStatus() == AuctionItem::EXPIRED && mailItem->getOwnerID() == targetOID);
			bool sldBuyer = (mailItem->getStatus() == AuctionItem::SOLD && mailItem->getBuyerID() == targetOID);

			if (!expOwner && !sldBuyer) {
				continue;
			}

			ManagedReference<SceneObject*> obj = zoneServer->getObject(mailItem->getAuctionedItemObjectID());
			ManagedReference<SceneObject*> mailVendor = zoneServer->getObject(mailItem->getVendorID());
			String mailVendorName = (mailVendor != nullptr) ? mailVendor->getDisplayedName() : "unknown";
			String mZone = "?";
			String mCoords = "?,?";

			if (mailVendor != nullptr) {
				Zone* mz = mailVendor->getZone();
				if (mz != nullptr) {
					mZone = mz->getZoneName();
					mCoords = String::valueOf((int)mailVendor->getWorldPositionX()) + ", " + String::valueOf((int)mailVendor->getWorldPositionY());
				}
			}

			mailBody << mailItem->getItemName() << " | item:" << mailItem->getAuctionedItemObjectID() << " | " << mailVendorName << " (" << mailItem->getVendorID() << ")"
					 << " | " << mZone << " " << mCoords
					 << " | " << mailItem->getStatusString() << ((obj == nullptr) ? " [GHOST]" : "") << "\n";
		}

		UnicodeString subject = "Stockroom: " + targetName;
		UnicodeString body = mailBody.toString();
		chatManager->sendMail("System", subject, body, player->getFirstName());
	}

	player->sendSystemMessage("Found " + String::valueOf(count) + " items for " + targetName + ". Results mailed and logged.");
}

void AuctionManagerImplementation::displayInfo(CreatureObject* player) {
	ManagedReference<SuiListBox*> list = new SuiListBox(player, SuiWindowType::MARKET_INFO, 0x00);
	list->setPromptTitle("Market Info");
	list->setPromptText("Information about the commodities market");

	list->addMenuItem("Terminals with items");
	list->addMenuItem("\tBazaars: " + String::valueOf(auctionMap->getBazaarCount()));
	list->addMenuItem("\tVendors: " + String::valueOf(auctionMap->getVendorCount()));
	list->addMenuItem("Total Items: " + String::valueOf(auctionMap->getTotalItemCount()));

	player->sendMessage(list->generateMessage());
}

void AuctionManagerImplementation::updateAuctionOwner(AuctionItem* item, CreatureObject* player) {
	Locker locker(item);

	auto playerManager = zoneServer->getPlayerManager();
	ManagedReference<CreatureObject*> previousOwner = playerManager != nullptr ? playerManager->getPlayer(item->getOwnerName()) : nullptr;

	if (player == previousOwner) {
		return;
	}

	auctionMap->removeFromCommodityLimit(item);

	item->setOwnerID(player->getObjectID());
	item->setOwnerName(player->getFirstName());

	if (!item->isOnBazaar() && item->getStatus() != AuctionItem::OFFERED) {
		return;
	}

	auctionMap->addToCommodityLimit(item);
}

void AuctionManagerImplementation::sendVendorUpdateMail(SceneObject* vendor, bool isEmpty) {
	// Send the mail to the vendor owner
	if (vendor == nullptr || !vendor->isVendor()) {
		return;
	}

	VendorDataComponent* vendorData = nullptr;
	DataObjectComponentReference* data = vendor->getDataObjectComponent();
	if (data != nullptr && data->get() != nullptr && data->get()->isVendorData()) {
		vendorData = cast<VendorDataComponent*>(data->get());
	}

	if (vendorData == nullptr) {
		return;
	}

	auto chatManager = zoneServer->getChatManager();
	ManagedReference<CreatureObject*> owner = vendor->getZoneServer()->getObject(vendorData->getOwnerId()).castTo<CreatureObject*>();

	if (chatManager == nullptr || owner == nullptr) {
		return;
	}

	String sender = vendor->getDisplayedName();
	UnicodeString subject("@auction:vendor_status_subject");

	if (isEmpty) {
		StringIdChatParameter body("@auction:vendor_status_empty");
		body.setTO(vendor->getDisplayedName());
		chatManager->sendMail(sender, subject, body, owner->getFirstName());
		vendorData->setEmpty();

		if (vendorData->isRegistered())
			VendorManager::instance()->handleUnregisterVendor(owner, cast<TangibleObject*>(vendor));
	} else {
		StringIdChatParameter body("@auction:vendor_status_normal");
		body.setTO(vendor->getDisplayedName());
		chatManager->sendMail(sender, subject, body, owner->getFirstName());
	}
}

String AuctionManagerImplementation::removeColorCodes(const String& name) {
	String itemName = name;

	while (itemName.contains("\\#")) {
		int index = itemName.indexOf("\\#");
		String sub = "\\" + itemName.subString(index, index + 2);
		itemName = itemName.replaceFirst(sub, "");
	}

	return itemName;
}

void AuctionManagerImplementation::restockVendor(CreatureObject* creature, SceneObject* vendor) {
	if (creature == nullptr || vendor == nullptr) {
		return;
	}

	if (!isMarketEnabled()) {
		creature->sendSystemMessage("@ui_auc:err_vendor_terminal_error"); // This market is unavailable.
		return;
	}

	auto zone = vendor->getZone();

	if (zone == nullptr) {
		return;
	}

	String planet = zone->getZoneName();

	String region = "@planet_n:" + planet;
	ManagedReference<CityRegion*> cityRegion = vendor->getCityRegion().get();

	if (cityRegion != nullptr) {
		region = cityRegion->getCityRegionName();
	}

	auto terminalList = auctionMap->getVendorTerminalData(planet, region, vendor);

	// Infinity: Get sales tax once — it won't change during restock
	int salesTax = 0;

	if (cityRegion != nullptr) {
		salesTax = cityRegion->getSalesTax();
	}

	int itemsRestocked = 0;

	for (int j = 0; (j < terminalList.size()); ++j) {
		Reference<TerminalItemList*>& terminalListi = terminalList.get(j);

		if (terminalListi == nullptr || terminalListi->size() == 0) {
			continue;
		}

		// Get a copy of this terminal's list because the loop deletes some objects as it runs
		Reference<TerminalItemList*> list = new TerminalItemList(*terminalListi);

		try {
			list->rlock();

			for (int i = 0; (i < list->size()); i++) {
				ManagedReference<AuctionItem*>& item = list->get(i);

				if (item == nullptr) {
					continue;
				}

				if (vendor->isVendor() && item->getVendorID() != vendor->getObjectID()) {
					continue;
				}

				if (item->getOwnerID() != creature->getObjectID()) {
					continue;
				}

				if (item->getStatus() == AuctionItem::DELETED || item->getStatus() == AuctionItem::RETRIEVED) {
					continue;
				}

				if ((item->getStatus() == AuctionItem::SOLD && item->getBuyerID() == creature->getObjectID()) || (item->getStatus() == AuctionItem::EXPIRED && item->getOwnerID() == creature->getObjectID())) {
					int itemPrice = item->getPrice();
					int basePrice = Math::min(itemPrice, MAXVENDORPRICE);

					if (salesTax > 0) {
						basePrice = Math::min((int)ceil(itemPrice / (1.0f + (salesTax / 100.f))), MAXVENDORPRICE);
					}

					int result = addSaleItem(creature, item->getAuctionedItemObjectID(), vendor, item->getItemDescription(), basePrice, ConfigManager::instance()->getVendorExpirePeriod(), item->isAuction(), item->isPremiumAuction());

					if (result == ItemSoldMessage::SUCCESS) {
						itemsRestocked++;
					}
				}
			}
		} catch (Exception& e) {
			warning() << "Exception in restockVendor: " << e.what();
		}

		list->runlock(); // Need to unlock regardless of exception
	}
	if (itemsRestocked > 0) {
		creature->sendSystemMessage(String::valueOf(itemsRestocked) + " vendor item" + (itemsRestocked > 1 ? "s were" : " was") + " restocked.");
	} else {
		creature->sendSystemMessage("No vendor items were eligible to restock.");
	}
}

Logger* AuctionManagerImplementation::getLogger() {
	return dynamic_cast<Logger*>(this);
}

// ============================================================
// Vendor Pack-Up (Phase 1) primitives
// ============================================================

void AuctionManagerImplementation::prePackVendor(SceneObject* vendor) {
	if (vendor == nullptr) {
		return;
	}

	AuctionTerminalDataComponent* terminalData = nullptr;
	DataObjectComponentReference* data = vendor->getDataObjectComponent();
	if (data != nullptr && data->get() != nullptr && data->get()->isAuctionTerminalData()) {
		terminalData = cast<AuctionTerminalDataComponent*>(data->get());
	}

	if (terminalData != nullptr) {
		terminalData->setSuppressUIDRebuild(true);
	} else {
		warning() << "prePackVendor: vendor " << vendor->getObjectID()
		          << " has no AuctionTerminalDataComponent";
	}

	if (auctionMap != nullptr) {
		auctionMap->updateVendorSearch(vendor, false);
	}

	// NH6 (partial): cancel pending vendorreturn task. This task fires
	// from the vendor at-mailing path (VendorDataComponent line 458);
	// without cancellation it could fire on a packed vendor whose zone
	// has resolved up the player chain, causing getNearbyPlayers calls
	// against bogus world coords. The defensive packed-state guard in
	// runVendorUpdate (NH2) already skips the task scheduling for
	// packed vendors, but if a task was queued before pack, this
	// removes it.
	//
	// Note: full listing-cancellation + bid-refund (NH6 main scope) is
	// deferred — captured as a separate Phase 1 follow-up. Active bids
	// at the time of pack remain in DB; on unpack they resume. On decay-
	// destroy of packed vendor (NH1 path), the existing
	// auctionsMap->deleteTerminalItems destroys the auction items
	// without explicit refund. Worth fixing before live but acceptable
	// for TC validation.
	vendor->removePendingTask("vendorreturn");

	info(true) << "prePackVendor: vendor " << vendor->getObjectID()
	           << " suppressed + hidden from search; vendorreturn task removed";
}

void AuctionManagerImplementation::postUnpackVendor(SceneObject* vendor) {
	if (vendor == nullptr) {
		return;
	}

	AuctionTerminalDataComponent* terminalData = nullptr;
	DataObjectComponentReference* data = vendor->getDataObjectComponent();
	if (data != nullptr && data->get() != nullptr && data->get()->isAuctionTerminalData()) {
		terminalData = cast<AuctionTerminalDataComponent*>(data->get());
	}

	if (terminalData != nullptr) {
		terminalData->setSuppressUIDRebuild(false);
		// Force UID recomputation now that the vendor is back in a real
		// zone via its parent chain.
		terminalData->updateUID();
	} else {
		warning() << "postUnpackVendor: vendor " << vendor->getObjectID()
		          << " has no AuctionTerminalDataComponent";
	}

	if (auctionMap != nullptr) {
		auctionMap->updateVendorSearch(vendor, true);
	}

	// Drain any auction items that were stashed by the boot sweep because
	// this vendor was packed at server start. They survived in DB but never
	// got into auctionsMap (AuctionsMap::addItem rejects vendors with
	// getZone() == nullptr). Now that the vendor has a real zone again,
	// re-add each one. Without this, the deployed vendor shows an empty
	// stockroom across server-restart-while-packed cycles.
	if (auctionMap != nullptr) {
		Vector<ManagedReference<AuctionItem*>> drained;
		{
			Locker pendLock(&packedVendorPendingItemsMutex);
			uint64 vid = vendor->getObjectID();
			if (packedVendorPendingItems.contains(vid)) {
				drained = packedVendorPendingItems.get(vid);
				packedVendorPendingItems.drop(vid);
			}
		}
		int reloaded = 0;
		int failed = 0;
		int expireTasksScheduled = 0;
		for (int i = 0; i < drained.size(); ++i) {
			ManagedReference<AuctionItem*> item = drained.get(i);
			if (item == nullptr) {
				continue;
			}
			Locker itemLock(item);
			int result = auctionMap->addItem(nullptr, vendor, item);
			if (result == ItemSoldMessage::SUCCESS) {
				reloaded++;
				// Live auctions need their per-item expire task scheduled.
				// Boot init() does this inline for normal-path items; the
				// deferred-load path skipped that, so do it here. Sale items
				// (FORSALE/OFFERED) expire via the periodic doAuctionMaint
				// sweep instead - no per-item task needed.
				if (item->isAuction() && !auctionEvents.contains(item->getAuctionedItemObjectID())) {
					Reference<Task*> newTask = new ExpireAuctionTask(_this.getReferenceUnsafeStaticCast(), item);
					int expireSeconds = item->getExpireTime() - time(0);
					if (expireSeconds < 0) {
						expireSeconds = 5;
					}
					newTask->schedule(expireSeconds * 1000);
					Locker eventsLock(&auctionEvents);
					auctionEvents.put(item->getAuctionedItemObjectID(), newTask);
					expireTasksScheduled++;
				}
			} else {
				failed++;
				warning() << "postUnpackVendor: addItem failed for auction item "
				          << item->getAuctionedItemObjectID()
				          << " (vendor " << vendor->getObjectID()
				          << ") result=" << ItemSoldMessage::statusToString(result);
			}
		}
		if (drained.size() > 0) {
			info(true) << "postUnpackVendor: re-loaded " << reloaded << "/" << drained.size()
			           << " stashed auction items into auctionsMap for vendor "
			           << vendor->getObjectID() << " (failed=" << failed
			           << ", expireTasksScheduled=" << expireTasksScheduled << ")";
		}
	}

	info(true) << "postUnpackVendor: vendor " << vendor->getObjectID()
	           << " unsuppressed + visible on search";
}

bool AuctionManagerImplementation::retryVendorReinit(SceneObject* vendor) {
	if (vendor == nullptr) {
		return false;
	}

	info(true) << "retryVendorReinit: re-running postUnpackVendor for vendor "
	           << vendor->getObjectID();
	postUnpackVendor(vendor);
	return true;
}

bool AuctionManagerImplementation::packVendor(CreatureObject* player, SceneObject* vendor) {
	if (player == nullptr || vendor == nullptr) {
		return false;
	}

	if (!ConfigManager::instance()->isVendorPackupEnabled()) {
		player->sendSystemMessage("Vendor pack-up is currently disabled on this server.");
		return false;
	}

	if (!vendor->isVendor()) {
		player->sendSystemMessage("Target is not a vendor.");
		return false;
	}

	// Race-protect the entire pack flow against double-fire from the radial
	// menu. Without this, two clicks ~2ms apart both pass the cell-parent
	// check before either's transferObject commits, producing two VCDs each
	// claiming the vendor (one ends up as a stale-weak-ref orphan in the
	// datapad). Confirmed live 2026-05-01: vendor 281475289368616 stored in
	// VCDs 281475289368729 + 281475289369732 within the same millisecond on
	// the same thread, requiring the duplicate-VCD orphan detection in
	// VCD::initializeTransientMembers to clean up. The lock is reentrant
	// for the inner Locker(vendor, player) blocks below.
	Locker vendorRaceLock(vendor, player);

	// Already-packed guard: if vendor's root parent is a creature, it is
	// already inside a VCD inside someone's datapad. Reject.
	auto vendorRoot = vendor->getRootParent();
	if (vendorRoot != nullptr && vendorRoot->isCreatureObject()) {
		player->sendSystemMessage("Vendor is already packed.");
		return false;
	}

	// Ownership check via VendorDataComponent
	auto vendorData = vendor->getDataObjectComponent();
	VendorDataComponent* vData = nullptr;
	if (vendorData != nullptr && vendorData->get() != nullptr && vendorData->get()->isVendorData()) {
		vData = cast<VendorDataComponent*>(vendorData->get());
	}

	if (vData == nullptr) {
		player->sendSystemMessage("Vendor is missing data component.");
		return false;
	}

	auto ghost = player->getPlayerObject();
	bool isAdmin = (ghost != nullptr && ghost->isAdmin());

	if (vData->getOwnerId() != player->getObjectID() && !isAdmin) {
		player->sendSystemMessage("You must own the vendor or be an admin to pack it up.");
		return false;
	}

	// Locate parent cell + building from current parent chain
	auto cellSceno = vendor->getParent().get();
	if (cellSceno == nullptr) {
		player->sendSystemMessage("Vendor must be deployed in a building cell to be packed up.");
		return false;
	}

	auto cell = cellSceno.castTo<CellObject*>();
	if (cell == nullptr) {
		player->sendSystemMessage("Vendor parent is not a cell — packing only works for in-building vendors.");
		return false;
	}

	auto buildingSceno = cell->getParent().get();
	BuildingObject* building = nullptr;
	if (buildingSceno != nullptr) {
		building = buildingSceno.castTo<BuildingObject*>();
	}

	if (building == nullptr) {
		player->sendSystemMessage("Could not resolve parent building for vendor.");
		return false;
	}

	auto zoneRef = building->getZone();
	String planet = (zoneRef != nullptr) ? zoneRef->getZoneName() : String("");

	// Snapshot vendor metadata BEFORE pre-pack (which doesn't change anything
	// the snapshot reads, but defensive ordering).
	float vx = vendor->getPositionX();
	float vy = vendor->getPositionY();
	float vz = vendor->getPositionZ();
	String vendorName = vendor->getDisplayedName();
	uint64 buildingOID = building->getObjectID();
	uint64 cellOID = cell->getObjectID();
	uint64 packerOID = player->getObjectID();
	uint64 ownerOID = vData->getOwnerId();

	// Capture player-facing toggle state BEFORE deregistering — restored
	// at deploy by VCD::deployVendorAtCell. Mirrors the
	// VendorDataComponent::destroy path's deregister sequence.
	bool wasSearchEnabledAtPack = vData->isVendorSearchEnabled();
	bool wasRegisteredAtPack = vData->isRegistered();

	// Allocate the Vendor Control Device
	auto vcdSceno = zoneServer->createObject(STRING_HASHCODE("object/intangible/vendor/generic_vendor_control_device.iff"), 1);
	auto vcd = vcdSceno.castTo<VendorControlDevice*>();

	if (vcd == nullptr) {
		player->sendSystemMessage("Failed to allocate Vendor Control Device.");
		return false;
	}

	// Snapshot
	uint64 nowEpoch = (uint64)(System::getMiliTime() / 1000);
	vcd->setSnapshot(planet, buildingOID, cellOID, vx, vy, vz, packerOID, ownerOID, vendorName, /*listingCount placeholder*/ 0, nowEpoch);
	vcd->setControlledObject(vendor->asTangibleObject());

	// Custom name so datapad list shows which packed vendor this is. Without
	// this, every VCD shows the generic "Vendor Control Device" label and the
	// player can't distinguish multiple packed vendors. notifyClient=false
	// because the VCD will be sent to client via the upcoming datapad
	// transferObject's notify path.
	vcd->setCustomObjectName(UnicodeString(vendorName), false);

	// Move VCD into player's datapad
	auto datapad = player->getSlottedObject("datapad");
	if (datapad == nullptr) {
		// Cleanup orphan VCD
		vcd->destroyObjectFromWorld(true);
		vcd->destroyObjectFromDatabase(true);
		player->sendSystemMessage("Failed to access player datapad.");
		return false;
	}

	{
		Locker datapadLock(datapad, player);
		if (!datapad->transferObject(vcd, -1, true)) {
			Locker vcdLock(vcd);
			vcd->destroyObjectFromWorld(true);
			vcd->destroyObjectFromDatabase(true);
			player->sendSystemMessage("Failed to transfer Vendor Control Device into datapad.");
			return false;
		}
	}

	// transferObject's notifyClient sends only the link/container update for
	// the new child; for a freshly-created object the player has never seen,
	// the SceneCreateObject + baselines also need to go out. Explicit sendTo
	// forces the full create+baselines sequence so the VCD shows in datapad
	// immediately instead of only after a soft-relog. Confirmed live
	// 2026-05-01: pre-fix, the VCD was server-side correct but the client
	// only learned about it after logout/login refreshed the datapad stream.
	vcd->sendTo(player, true, false);

	// Deregister from planetary map + clear search-enabled flag BEFORE the
	// move. handleUnregisterVendor needs vendor->getZone() to be non-null,
	// which it still is at this point (vendor is still in cell). After
	// transferObject the vendor's parent chain resolves up through the
	// player creature, so getZone() would return the player's zone — wrong.
	if (wasRegisteredAtPack) {
		VendorManager::instance()->handleUnregisterVendor(player, vendor->asTangibleObject());
	}
	if (wasSearchEnabledAtPack) {
		Locker vendorSearchLock(vendor);
		vData->setVendorSearchEnabled(false);
	}

	// Suppress UID rebuilds + hide from search BEFORE the move
	prePackVendor(vendor);

	// Broadcast destroy to nearby players BEFORE the move. ContainerComponent::
	// transferObject's removeObject path passes notifyClient=false because the
	// assumption is "object is moving to another visible parent". Here the
	// destination (VCD in datapad) is invisible to cell observers, so without
	// this explicit broadcast, clients in the cell keep the vendor in their
	// stale object cache and don't realize it's gone. Headless harness
	// confirmed this 2026-05-01 — pack succeeded server-side but vendor
	// stayed in client's perception until zone re-stream.
	vendor->broadcastDestroy(vendor->asTangibleObject(), true);

	// COMMIT POINT — physically move the vendor from its parent cell into
	// the VCD's slotted container. After this transferObject succeeds, the
	// vendor's parentID in BDB is the VCD; the cell no longer references
	// it. On next zone init the cell will not respawn the vendor, so
	// duplicate-vendor (one in cell + one in datapad) is prevented.
	{
		Locker vendorLock(vendor, player);
		Locker vcdLock(vcd, vendor);

		if (!vcd->transferObject(vendor->asTangibleObject(), -1, true)) {
			// Rollback per "abort" recovery policy. ContainerComponent::
			// transferObject removes the object from its old parent BEFORE
			// running the destination's volume-limit / arrangement check
			// (ContainerComponent.cpp:241-264 then 306). When the dest
			// check rejects, the vendor is left ORPHANED — no parent, not
			// in any zone — even though transferObject returned false.
			// Confirmed live 2026-05-01: a failed pack-up loop left
			// "Packup" (OID 281475289118599) with parent=null, invisible
			// to clients on next boot.
			//
			// So the rollback must (1) detect orphan, (2) put vendor back
			// in its original cell at original position. Then undo the
			// prePackVendor commodities-market state and destroy the empty
			// VCD as before.
			warning() << "packVendor: vcd->transferObject failed; rolling back vendor "
			          << vendor->getObjectID();

			// Step 1: re-insert vendor to original cell if orphaned.
			auto currentParent = vendor->getParent().get();
			if (currentParent == nullptr) {
				warning() << "packVendor: vendor " << vendor->getObjectID()
				          << " was orphaned by failed transferObject; re-inserting into cell "
				          << cell->getObjectID();

				Locker cellLocker(cell, vendor);

				if (cell->transferObject(vendor->asTangibleObject(), -1, true)) {
					vendor->initializePosition(vx, vz, vy);
					info(true) << "packVendor: vendor " << vendor->getObjectID()
					           << " re-parented to cell " << cell->getObjectID()
					           << " at (" << vx << "," << vy << "," << vz << ")";
				} else {
					error() << "packVendor: CRITICAL — could not re-insert orphaned vendor "
					        << vendor->getObjectID() << " into cell " << cell->getObjectID()
					        << ". Vendor exists in DB with no parent. Manual recovery needed.";
				}
			} else if (currentParent.get() != cell) {
				// Vendor's parent isn't the cell AND isn't the VCD — somewhere unexpected.
				warning() << "packVendor: vendor " << vendor->getObjectID()
				          << " parent unexpectedly " << currentParent->getObjectID()
				          << " (expected cell " << cell->getObjectID() << " or null)";
			}

			// Step 2: undo prePackVendor commodities-market state.
			auto data = vendor->getDataObjectComponent();
			if (data != nullptr && data->get() != nullptr && data->get()->isAuctionTerminalData()) {
				auto td = cast<AuctionTerminalDataComponent*>(data->get());
				td->setSuppressUIDRebuild(false);
				td->updateUID();
			}
			if (auctionMap != nullptr) {
				auctionMap->updateVendorSearch(vendor, true);
			}

			// Step 3: destroy the empty VCD.
			vcd->destroyObjectFromWorld(true);
			vcd->destroyObjectFromDatabase(true);

			player->sendSystemMessage("Pack-up failed at the final step. Vendor restored.");
			return false;
		}
	}

	info(true) << "packVendor: vendor " << vendor->getObjectID()
	           << " packed into VCD " << vcd->getObjectID()
	           << " in datapad of player " << player->getObjectID();

	{
		TransactionLog trx(player, vendor, TrxCode::VENDORPACKUP);
		trx.addState("subjectVendorName", vendorName);
		trx.addState("subjectPlanet", planet);
		trx.addState("subjectBuildingOID", buildingOID);
		trx.addState("subjectCellOID", cellOID);
		trx.addState("subjectX", vx);
		trx.addState("subjectY", vy);
		trx.addState("subjectZ", vz);
		trx.addState("subjectVCDObjectID", vcd->getObjectID());
	}

	player->sendSystemMessage("Vendor packed. Find the Vendor Control Device in your datapad.");
	return true;
}

bool AuctionManagerImplementation::adminPackVendorForOwner(CreatureObject* admin, SceneObject* vendor) {
	if (admin == nullptr || vendor == nullptr) {
		return false;
	}

	// Hard gate: full admin only.
	auto adminGhost = admin->getPlayerObject();
	if (adminGhost == nullptr || adminGhost->getAdminLevel() < 15) {
		admin->sendSystemMessage("Admin pack-vendor requires admin level 15.");
		return false;
	}

	if (!vendor->isVendor()) {
		admin->sendSystemMessage("Target is not a vendor.");
		return false;
	}

	// Resolve owner from VendorDataComponent
	auto vendorDataRef = vendor->getDataObjectComponent();
	VendorDataComponent* vData = nullptr;
	if (vendorDataRef != nullptr && vendorDataRef->get() != nullptr && vendorDataRef->get()->isVendorData()) {
		vData = cast<VendorDataComponent*>(vendorDataRef->get());
	}

	if (vData == nullptr) {
		admin->sendSystemMessage("Vendor is missing data component.");
		return false;
	}

	uint64 ownerOID = vData->getOwnerId();
	if (ownerOID == 0) {
		admin->sendSystemMessage("Vendor has no owner. Manual cleanup required.");
		return false;
	}

	if (ownerOID == admin->getObjectID()) {
		// Admin is the owner — use the regular pack flow instead.
		admin->sendSystemMessage("You own this vendor — use the regular Pack Up Vendor option.");
		return false;
	}

	auto ownerSceno = zoneServer->getObject(ownerOID);
	if (ownerSceno == nullptr) {
		admin->sendSystemMessage("Owner OID " + String::valueOf(ownerOID) + " not resolvable. Manual cleanup required.");
		return false;
	}

	auto owner = ownerSceno.castTo<CreatureObject*>();
	if (owner == nullptr) {
		admin->sendSystemMessage("Owner is not a CreatureObject.");
		return false;
	}

	// Locate building/cell from vendor's parent chain
	auto cellSceno = vendor->getParent().get();
	if (cellSceno == nullptr) {
		admin->sendSystemMessage("Vendor must be deployed in a building cell.");
		return false;
	}

	auto cell = cellSceno.castTo<CellObject*>();
	if (cell == nullptr) {
		admin->sendSystemMessage("Vendor parent is not a cell.");
		return false;
	}

	auto buildingSceno = cell->getParent().get();
	BuildingObject* building = buildingSceno != nullptr ? buildingSceno.castTo<BuildingObject*>() : nullptr;
	if (building == nullptr) {
		admin->sendSystemMessage("Could not resolve parent building.");
		return false;
	}

	auto zoneRef = building->getZone();
	String planet = (zoneRef != nullptr) ? zoneRef->getZoneName() : String("");

	// Snapshot
	float vx = vendor->getPositionX();
	float vy = vendor->getPositionY();
	float vz = vendor->getPositionZ();
	String vendorName = vendor->getDisplayedName();
	uint64 buildingOID = building->getObjectID();
	uint64 cellOID = cell->getObjectID();

	// Capture player-facing toggle state pre-deregister (same as packVendor).
	bool wasSearchEnabledAtPack = vData->isVendorSearchEnabled();
	bool wasRegisteredAtPack = vData->isRegistered();

	// Resolve owner's datapad (with bank fallback per deep-review C5)
	auto datapad = owner->getSlottedObject("datapad");
	if (datapad == nullptr) {
		admin->sendSystemMessage("Could not access owner's datapad.");
		return false;
	}

	SceneObject* recipient = datapad;
	String recipientLabel = "datapad";

	if (datapad->isContainerFullRecursive()) {
		auto bank = owner->getSlottedObject("bank");
		if (bank != nullptr && !bank->isContainerFullRecursive()) {
			recipient = bank;
			recipientLabel = "bank";
		} else {
			admin->sendSystemMessage("Owner's datapad and bank are both full. Pack rejected.");
			return false;
		}
	}

	// Allocate VCD
	auto vcdSceno = zoneServer->createObject(STRING_HASHCODE("object/intangible/vendor/generic_vendor_control_device.iff"), 1);
	auto vcd = vcdSceno.castTo<VendorControlDevice*>();

	if (vcd == nullptr) {
		admin->sendSystemMessage("Failed to allocate Vendor Control Device.");
		return false;
	}

	uint64 nowEpoch = (uint64)(System::getMiliTime() / 1000);
	vcd->setSnapshot(planet, buildingOID, cellOID, vx, vy, vz, admin->getObjectID(), ownerOID, vendorName, 0, nowEpoch);
	vcd->setControlledObject(vendor->asTangibleObject());

	// Custom name so datapad list shows which packed vendor this is.
	vcd->setCustomObjectName(UnicodeString(vendorName), false);

	// Transfer VCD into recipient (owner's datapad or bank)
	{
		Locker recipientLock(recipient, admin);
		if (!recipient->transferObject(vcd, -1, true)) {
			Locker vcdLock(vcd);
			vcd->destroyObjectFromWorld(true);
			vcd->destroyObjectFromDatabase(true);
			admin->sendSystemMessage("Failed to transfer Vendor Control Device into owner's " + recipientLabel + ".");
			return false;
		}
	}

	// Send VCD create+baselines to the OWNER (whose datapad/bank now holds
	// it) if they're online. See packVendor for rationale.
	if (owner->isOnline()) {
		vcd->sendTo(owner, true, false);
	}

	// Deregister + clear search BEFORE the move (must run while vendor still
	// has its zone). Note: handleUnregisterVendor takes a CreatureObject for
	// the system-message recipient — pass owner here so the courtesy mail
	// path uses the right player; if they're offline the message is dropped
	// silently which is acceptable for the admin path.
	if (wasRegisteredAtPack) {
		VendorManager::instance()->handleUnregisterVendor(owner, vendor->asTangibleObject());
	}
	if (wasSearchEnabledAtPack) {
		Locker vendorSearchLock(vendor);
		vData->setVendorSearchEnabled(false);
	}

	// Standard pack actions on the vendor
	prePackVendor(vendor);

	// Broadcast destroy to cell observers — see packVendor for rationale.
	vendor->broadcastDestroy(vendor->asTangibleObject(), true);

	// COMMIT POINT — physically move vendor into VCD's slotted container.
	// Same atomicity argument as packVendor: parent change is BDB-logged,
	// preventing zone-reload duplicate.
	{
		Locker vendorLock(vendor, admin);
		Locker vcdLock(vcd, vendor);

		if (!vcd->transferObject(vendor->asTangibleObject(), -1, true)) {
			// See packVendor rollback comment — same orphan-on-failure
			// recovery applies here.
			warning() << "adminPackVendorForOwner: vcd->transferObject failed; rolling back vendor "
			          << vendor->getObjectID();

			// Step 1: re-insert vendor to original cell if orphaned.
			auto currentParent = vendor->getParent().get();
			if (currentParent == nullptr) {
				warning() << "adminPackVendorForOwner: vendor " << vendor->getObjectID()
				          << " was orphaned by failed transferObject; re-inserting into cell "
				          << cell->getObjectID();

				Locker cellLocker(cell, vendor);

				if (cell->transferObject(vendor->asTangibleObject(), -1, true)) {
					vendor->initializePosition(vx, vz, vy);
					info(true) << "adminPackVendorForOwner: vendor " << vendor->getObjectID()
					           << " re-parented to cell " << cell->getObjectID()
					           << " at (" << vx << "," << vy << "," << vz << ")";
				} else {
					error() << "adminPackVendorForOwner: CRITICAL — could not re-insert orphaned vendor "
					        << vendor->getObjectID() << " into cell " << cell->getObjectID()
					        << ". Vendor exists in DB with no parent. Manual recovery needed.";
				}
			}

			// Step 2: undo prePackVendor commodities-market state.
			auto data = vendor->getDataObjectComponent();
			if (data != nullptr && data->get() != nullptr && data->get()->isAuctionTerminalData()) {
				auto td = cast<AuctionTerminalDataComponent*>(data->get());
				td->setSuppressUIDRebuild(false);
				td->updateUID();
			}
			if (auctionMap != nullptr) {
				auctionMap->updateVendorSearch(vendor, true);
			}

			// Step 3: destroy the empty VCD.
			vcd->destroyObjectFromWorld(true);
			vcd->destroyObjectFromDatabase(true);

			admin->sendSystemMessage("Admin pack-up failed at the final step. Vendor restored.");
			return false;
		}
	}

	info(true) << "adminPackVendorForOwner: admin=" << admin->getObjectID()
	           << " owner=" << ownerOID
	           << " vendor=" << vendor->getObjectID()
	           << " vcd=" << vcd->getObjectID()
	           << " recipient=" << recipientLabel;

	{
		TransactionLog trx(admin, vendor, TrxCode::ADMINVENDORPACK);
		trx.addState("adminObjectID", admin->getObjectID());
		trx.addState("ownerObjectID", ownerOID);
		trx.addState("ownerName", owner->getDisplayedName());
		trx.addState("ownerOnline", owner->isOnline());
		trx.addState("recipientSlot", recipientLabel);
		trx.addState("subjectVendorName", vendorName);
		trx.addState("subjectPlanet", planet);
		trx.addState("subjectBuildingOID", buildingOID);
		trx.addState("subjectCellOID", cellOID);
		trx.addState("subjectVCDObjectID", vcd->getObjectID());
	}

	admin->sendSystemMessage("Admin packed vendor for " + owner->getDisplayedName() + ". VCD placed in their " + recipientLabel + ".");

	// Notify owner. If online, sysmsg pops immediately. If offline, queue
	// in-game mail via ChatManager so they see it on next login.
	const String mailBody = "An administrator has packed up your vendor.\n\n"
		"Vendor: " + vendorName + "\n"
		"Former location: " + planet + "\n\n"
		"You can find the Vendor Control Device in your " + recipientLabel + ". "
		"Right-click the device and choose a Deploy option to place the vendor at a new location.";

	if (owner->isOnline()) {
		owner->sendSystemMessage("An administrator has packed up your vendor (formerly at " + planet + ", " + vendorName + "). Find the Vendor Control Device in your " + recipientLabel + ".");
	}

	auto chatManager = zoneServer->getChatManager();
	if (chatManager != nullptr) {
		UnicodeString subject = "Vendor Packed by Administrator";
		UnicodeString body = mailBody;
		chatManager->sendMail(
			"system",                          // from
			subject,                           // subject (UnicodeString)
			body,                              // body (UnicodeString)
			owner->getFirstName()              // recipient name
		);
	}

	return true;
}