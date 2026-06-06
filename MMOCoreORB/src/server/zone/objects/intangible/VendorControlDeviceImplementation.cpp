/*
				Copyright <Infinity>
		See file COPYING for copying conditions.*/

#include "server/zone/objects/intangible/VendorControlDevice.h"
#include "server/zone/objects/intangible/IntangibleObject.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/objects/player/PlayerObject.h"
#include "server/zone/objects/tangible/TangibleObject.h"
#include "server/zone/objects/tangible/components/vendor/AuctionTerminalDataComponent.h"
#include "server/zone/objects/cell/CellObject.h"
#include "server/zone/objects/building/BuildingObject.h"
#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/objects/transaction/TransactionLog.h"
#include "server/zone/managers/auction/AuctionManager.h"
#include "server/zone/managers/vendor/VendorManager.h"
#include "server/zone/ZoneProcessServer.h"
#include "server/zone/packets/object/ObjectMenuResponse.h"
#include "server/zone/Zone.h"
#include "server/zone/ZoneServer.h"

void VendorControlDeviceImplementation::initializeTransientMembers() {
	IntangibleObjectImplementation::initializeTransientMembers();

	// Lazy orphan scan: detect VCDs that lost their contained vendor due to
	// a deserialized-but-stale state (decay-destroyed vendor not properly
	// marked). Per the abort recovery policy: mark as husk so the player
	// can discard via the existing radial.
	//
	// CRITICAL distinction: getSavedObjectID() != 0 gates this check so
	// fresh VCDs (savedObjectID==0, weak-ref never bound to anything yet)
	// are NOT husk-marked. Without this gate, every freshly-created VCD
	// from packVendor -> createObject -> initializeTransientMembers gets
	// husk-flagged BEFORE setControlledObject runs, and the player ends
	// up with a "Discard Empty Device" radial that destroys the
	// successfully-packed vendor on click. (Deep-review NC1.)
	//
	// Two orphan shapes handled:
	//   (a) controlled == nullptr -> weak-ref unresolvable (vendor destroyed)
	//   (b) controlled != nullptr but vendor's parent is some OTHER VCD
	//       -> duplicate VCD case. A double-pack race produced two VCDs
	//       claiming the same vendor; only one truly owns it (the one the
	//       vendor's parent points to). The other is a husk.
	uint64 savedOID = controlledObject.getSavedObjectID();
	auto controlled = controlledObject.get();
	if (savedOID != 0 && !vendorDestroyedByDecay) {
		bool isOrphan = false;
		String reason;
		if (controlled == nullptr) {
			isOrphan = true;
			reason = "weak-ref resolves null";
		} else {
			Reference<VendorControlDevice*> thisRef = _this.getReferenceUnsafeStaticCast();
			auto vendorParent = controlled->getParent().get();
			if (vendorParent != nullptr && vendorParent.get() != thisRef.get()) {
				isOrphan = true;
				reason = "vendor's parent is OID " + String::valueOf(vendorParent->getObjectID()) + " (not this VCD)";
			}
		}
		if (isOrphan) {
			warning() << "VCD::initializeTransientMembers: orphan VCD detected (" << reason
			          << "); marking as empty husk for owner discard. "
			          << "VCD OID=" << getObjectID() << " savedOID=" << savedOID
			          << " ownerSnap=" << ownerObjectIDSnap;
			vendorDestroyedByDecay = true;
		}
	}
}

void VendorControlDeviceImplementation::storeObject(CreatureObject* player, bool force) {
	if (player == nullptr) {
		return;
	}

	auto controlled = controlledObject.get();
	if (controlled == nullptr) {
		warning() << "VCD::storeObject: controlledObject is null";
		return;
	}

	auto vendor = controlled.castTo<TangibleObject*>();
	if (vendor == nullptr) {
		warning() << "VCD::storeObject: controlledObject is not a TangibleObject";
		return;
	}

	auto zoneServer = getZoneServer();
	if (zoneServer == nullptr) {
		return;
	}

	auto auctionManager = zoneServer->getAuctionManager();
	if (auctionManager == nullptr) {
		warning() << "VCD::storeObject: no AuctionManager";
		return;
	}

	// AuctionManager::prePackVendor (suppress UID + hide from search)
	// is called by StructureManager::packVendor BEFORE this method runs,
	// so by this point the vendor's commodities-market state is already
	// frozen. Caller has also already moved the vendor into our slotted
	// container chain. This method is a hook for any per-device bookkeeping
	// we want to do at finalize time — currently a no-op.

	info(true) << "VCD::storeObject: vendor " << vendor->getObjectID()
	           << " stored in VCD " << getObjectID();
}

void VendorControlDeviceImplementation::deployVendorAtCell(CreatureObject* player, CellObject* targetCell, float x, float y, float z) {
	if (player == nullptr || targetCell == nullptr) {
		return;
	}

	auto controlled = controlledObject.get();
	if (controlled == nullptr) {
		player->sendSystemMessage("This control device is empty.");
		return;
	}

	auto vendor = controlled.castTo<TangibleObject*>();
	if (vendor == nullptr) {
		warning() << "VCD::deployVendorAtCell: controlledObject is not a TangibleObject";
		return;
	}

	// Stale-VCD exploit guard (deep-review NH4): if the server crashed
	// between the datapad-transfer and the vendor-transfer in packVendor,
	// the VCD persists with a weak-ref to a vendor that's still deployed
	// in the original cell. Without this check, the player could "Deploy"
	// to teleport that still-functioning vendor to their current location.
	// Validate that the vendor's current parent is THIS VCD before
	// proceeding — that's the only state where deploy is legitimate.
	Reference<VendorControlDevice*> thisRef = _this.getReferenceUnsafeStaticCast();
	auto vendorParent = vendor->getParent().get();
	if (vendorParent == nullptr || vendorParent.get() != thisRef.get()) {
		// TODO: replace with @player_vendor:deploy_stale_state once STF entry added.
		player->sendSystemMessage("This Vendor Control Device is in an inconsistent state - its vendor is not actually contained. Discard and retry pack-up.");
		warning() << "VCD::deployVendorAtCell: stale weak-ref detected. VCD OID=" << getObjectID()
		          << " vendor OID=" << vendor->getObjectID()
		          << " vendor's current parent OID=" << (vendorParent != nullptr ? String::valueOf(vendorParent->getObjectID()) : String("null"));
		return;
	}

	auto zoneServer = getZoneServer();
	if (zoneServer == nullptr) {
		return;
	}

	auto auctionManager = zoneServer->getAuctionManager();
	if (auctionManager == nullptr) {
		warning() << "VCD::deployVendorAtCell: no AuctionManager";
		return;
	}

	// Validate target cell parent is a building the player can deploy in.
	// Full permission gate is in StructureManager::deployPackedVendor —
	// this is a defensive re-check.
	auto cellParent = targetCell->getParent().get();
	if (cellParent == nullptr) {
		warning() << "VCD::deployVendorAtCell: targetCell has no parent";
		return;
	}

	auto building = cellParent.castTo<BuildingObject*>();
	if (building == nullptr) {
		warning() << "VCD::deployVendorAtCell: target cell parent is not a BuildingObject";
		return;
	}

	// Move vendor from VCD container into target cell.
	{
		Locker vendorLock(vendor, thisRef);
		Locker cellLock(targetCell, vendor);

		if (!targetCell->transferObject(vendor, -1, true)) {
			warning() << "VCD::deployVendorAtCell: failed to transfer vendor "
			          << vendor->getObjectID() << " into cell "
			          << targetCell->getObjectID();
			return;
		}

		vendor->initializePosition(x, z, y);
	}

	// Restore vendor's bazaar registration at the new location.
	auctionManager->postUnpackVendor(vendor);

	// Note: vendor search + planetary-map registration are NOT restored on
	// deploy. Pack-up always disables both (in AuctionManagerImpl::packVendor),
	// and the player must re-enable them via the standard vendor radials
	// after deploy. Per MrO: simpler model, fewer state surfaces, and gives
	// the player a fresh choice at the new location.

	// Clear husk flag if it was somehow set (defensive — destroyed vendor
	// shouldn't have made it this far, but if a reattached vendor recovered
	// we want a clean state).
	vendorDestroyedByDecay = false;

	info(true) << "VCD::deployVendorAtCell: vendor " << vendor->getObjectID()
	           << " deployed in cell " << targetCell->getObjectID()
	           << " at (" << x << ", " << y << ", " << z << ")";

	{
		TransactionLog trx(player, vendor, TrxCode::VENDORUNPACK);
		trx.addState("subjectCellOID", targetCell->getObjectID());
		trx.addState("subjectX", x);
		trx.addState("subjectY", y);
		trx.addState("subjectZ", z);
		trx.addState("previousPlanet", lastPlanet);
		trx.addState("previousX", lastX);
		trx.addState("previousY", lastY);
		trx.addState("previousZ", lastZ);
		trx.addState("subjectVCDObjectID", getObjectID());
	}

	if (player != nullptr) {
		player->sendSystemMessage("Vendor deployed at the chosen location.");
	}

	// Destroy the now-empty VCD. The vendor has been transferred back into
	// a cell, so this VCD has nothing left to control. Without this the
	// player accumulates orphan VCDs in the datapad — one per pack/deploy
	// cycle. Confirmed live 2026-05-01 (MrO observed two VCD messages on
	// shutdown after pack/deploy/pack/deploy/pack sequence — old VCDs
	// kept their stale weak refs and fired storeObject on save).
	//
	// Player can immediately re-pack the same vendor; AuctionManager::
	// packVendor will allocate a fresh VCD. (thisRef already declared
	// above for the deploy-time stale-VCD-exploit guard.)
	{
		Locker selfLocker(thisRef);
		thisRef->destroyObjectFromWorld(true);
		thisRef->destroyObjectFromDatabase(true);
	}
}

void VendorControlDeviceImplementation::generateObject(CreatureObject* player) {
	// Phase 1 — deploy always goes to the player's current cell + position.
	// The ControlDevice base contract requires this method; handle the
	// permission + cell validation inline and delegate to deployVendorAtCell.
	if (player == nullptr) {
		return;
	}

	if (vendorDestroyedByDecay) {
		player->sendSystemMessage("This Vendor Control Device is empty (vendor was destroyed by decay).");
		return;
	}

	auto parent = player->getParent().get();
	if (parent == nullptr) {
		player->sendSystemMessage("You must be inside a building cell to deploy a vendor.");
		return;
	}

	auto cell = parent.castTo<CellObject*>();
	if (cell == nullptr) {
		player->sendSystemMessage("You must be inside a building cell to deploy a vendor.");
		return;
	}

	auto buildingSceno = cell->getParent().get();
	auto building = (buildingSceno != nullptr) ? buildingSceno.castTo<BuildingObject*>() : nullptr;
	if (building == nullptr) {
		player->sendSystemMessage("Could not resolve parent building.");
		return;
	}

	auto ghost = player->getPlayerObject();
	bool isAdmin = (ghost != nullptr && ghost->isAdmin());
	bool isOwner = (building->getOwnerObjectID() == player->getObjectID());
	bool isOnAdmin = building->isOnAdminList(player);

	if (!isOwner && !isAdmin && !isOnAdmin) {
		player->sendSystemMessage("You don't have permission to deploy a vendor in this building.");
		return;
	}

	deployVendorAtCell(player, cell, player->getPositionX(), player->getPositionY(), player->getPositionZ());
}

void VendorControlDeviceImplementation::fillObjectMenuResponse(ObjectMenuResponse* menuResponse, CreatureObject* player) {
	if (menuResponse == nullptr || player == nullptr) {
		return;
	}

	// Standard intangible base options
	IntangibleObjectImplementation::fillObjectMenuResponse(menuResponse, player);

	// Owner only (or admin)
	auto ghost = player->getPlayerObject();
	bool isAdmin = (ghost != nullptr && ghost->isAdmin());
	bool isOwner = (ownerObjectIDSnap == player->getObjectID());

	if (!isOwner && !isAdmin) {
		return;
	}

	if (vendorDestroyedByDecay) {
		menuResponse->addRadialMenuItem(82, 3, "Discard Empty Device");
	} else {
		// Phase 1 — single deploy option. Player must be standing in
		// the cell where they want the vendor (any building they own
		// or have admin access to).
		menuResponse->addRadialMenuItem(81, 3, "Deploy Vendor");
	}
}

int VendorControlDeviceImplementation::canBeDestroyed(CreatureObject* player) {
	// VCDs are never player-destroyable. The packed vendor inside (and any
	// auction items / stockroom listings still bound to it) must not be
	// orphaned by a casual right-click Destroy. Even empty husks have the
	// dedicated radial 82 "Discard Empty Device" path which uses our
	// internal destroyObjectFromWorld call and bypasses this hook.
	if (player != nullptr) {
		// TODO: replace with @player_vendor:destroy_blocked once STF added.
		player->sendSystemMessage("Vendor Control Devices cannot be destroyed. Deploy the vendor first, or use the radial menu to discard an empty device.");
	}
	return 1;
}

int VendorControlDeviceImplementation::handleObjectMenuSelect(CreatureObject* player, byte selectedID) {
	if (player == nullptr) {
		return 0;
	}

	if (selectedID == 81) {
		// Deploy at player's current cell + position.
		// generateObject() does the permission + cell validation +
		// delegates to deployVendorAtCell.
		Locker deviceLocker(_this.getReferenceUnsafeStaticCast(), player);
		generateObject(player);
		return 0;
	}

	if (selectedID == 82 && vendorDestroyedByDecay) {
		// Discard empty husk
		Locker deviceLocker(_this.getReferenceUnsafeStaticCast(), player);
		destroyObjectFromWorld(true);
		destroyObjectFromDatabase(true);
		player->sendSystemMessage("Empty Vendor Control Device discarded.");
		return 0;
	}

	return IntangibleObjectImplementation::handleObjectMenuSelect(player, selectedID);
}

void VendorControlDeviceImplementation::destroyObjectFromDatabase(bool destroyContainedObjects) {
	// Cascade-clean the packed vendor + auction items if this VCD is going away
	// via a path that bypassed our canBeDestroyed guard (most notably:
	// character deletion cascading through inventory -> datapad -> VCD).
	//
	// Without this, auction items on a packed vendor outlive their VCD because
	// their suppressUIDRebuild=true flag makes the boot-time orphan sweep skip
	// them.  Mirrors the pattern in PetControlDeviceImplementation.
	//
	// Three states to handle:
	//   1. Husk (vendorDestroyedByDecay=true): controlled may resolve to null or
	//      a husk vendor; either way the underlying vendor + listings were already
	//      cleared by the decay path.  Just pass through to super.
	//   2. Packed (controlled resolves, vendor's parent is this VCD, vendor has
	//      no zone): real cleanup needed.  Clear suppressUIDRebuild so the normal
	//      vendor-destroy path handles the auction items, unregister the vendor,
	//      then destroy the vendor object.
	//   3. Deployed-but-VCD-being-destroyed (rare, shouldn't normally happen):
	//      same cleanup as packed.  destroyObjectFromDatabase on the vendor will
	//      pull it off whatever cell it's on.

	if (!vendorDestroyedByDecay) {
		ManagedReference<TangibleObject*> vendor = controlledObject.get();
		if (vendor != nullptr) {
			// Confirm the vendor still claims this VCD as parent — guards against
			// the duplicate-VCD shape called out in initializeTransientMembers().
			Reference<VendorControlDevice*> thisRef = _this.getReferenceUnsafeStaticCast();
			auto vendorParent = vendor->getParent().get();
			bool weAreTheOwner = (vendorParent != nullptr && vendorParent.get() == thisRef.get());

			if (weAreTheOwner) {
				Locker vendorLocker(vendor);

				// Step 1: clear suppressUIDRebuild so the auction-manager teardown
				// treats the items as live, not packed-and-protected.
				DataObjectComponentReference* dataComp = vendor->getDataObjectComponent();
				if (dataComp != nullptr) {
					AuctionTerminalDataComponent* atd = cast<AuctionTerminalDataComponent*>(dataComp->get());
					if (atd != nullptr && atd->isSuppressUIDRebuild()) {
						atd->setSuppressUIDRebuild(false);
					}
				}

				// Step 2: VendorManager teardown (vendor map, bazaar listings, etc).
				VendorManager::instance()->destroyVendor(vendor);

				// Step 3: destroy the vendor object itself (and its contents,
				// including any AuctionItem children).
				vendor->destroyObjectFromDatabase(true);
			}
		}
	}

	IntangibleObjectImplementation::destroyObjectFromDatabase(destroyContainedObjects);
}
