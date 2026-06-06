/*
 * OfflineTools.h
 *
 * Offline database maintenance tools that run at startup before
 * the server fully initializes. Server must be offline (BDB does
 * not support concurrent access from multiple processes).
 *
 * Usage:
 *   ./core3 removeadk                            - remove all AntiDecayKit objects from sceneobjects
 *   ./core3 purgeinvalidcrc                     - scan and report objects with invalid template CRCs
 *   ./core3 purgeinvalidcrc delete              - scan and DELETE objects with invalid template CRCs
 *   ./core3 purgeinvalidcrc deletesingle <OID>  - delete a single object by OID from all databases
 *   ./core3 purgeships                          - scan and report all ship/JTL objects in the database
 *   ./core3 purgeships delete                   - scan and DELETE all ship/JTL objects from the database
 *   ./core3 purgeability <name>                 - scan and report players with a specific stale ability
 *   ./core3 purgeability <name> delete          - scan and REMOVE the ability from all player ability lists
 *   ./core3 reconcileaccountids                 - scan and report PlayerObjects with stale accountID vs MySQL
 *   ./core3 reconcileaccountids fix             - rewrite stale accountID fields in the BDB
 *   ./core3 reconcileaccountids resetorphans    - queue BDB orphans for full-tree purge: UPDATE db_deleted=0 on
 *                                                 existing deleted_characters rows, INSERT db_deleted=0 rows for
 *                                                 strays (orphans with no MySQL row at all). DeleteCharactersTask
 *                                                 purges them on the next server run.
 */

#ifndef OFFLINETOOLS_H_
#define OFFLINETOOLS_H_

#include "engine/engine.h"
#include "conf/ConfigManager.h"
#include "templates/manager/TemplateManager.h"
#include "server/zone/objects/player/variables/AbilityList.h"
#include "server/db/ServerDatabase.h"

class OfflineTools : public Logger {
private:
	static constexpr uint32 HASH_ADK_FIELD = 0x2e41d02d; // TangibleObject.antiDecayKitObject
	static constexpr uint32 HASH_SERVER_CRC = STRING_HASHCODE("SceneObject.serverObjectCRC");
	static constexpr uint32 HASH_CLASSNAME = STRING_HASHCODE("_className");
	static constexpr uint32 HASH_ABILITYLIST = STRING_HASHCODE("PlayerObject.abilityList");
	static constexpr uint32 HASH_ACCOUNTID = STRING_HASHCODE("PlayerObject.accountID");
	static constexpr uint32 HASH_SLOTTEDOBJECTS = STRING_HASHCODE("SceneObject.slottedObjects");

	// Inline helper: find the offset of a serialized variable within an object stream.
	// Returns the byte offset where the variable's data begins, or -1 if not found.
	static int getVariableDataOffset(const uint32 variableHashCode, ObjectInputStream* stream) {
		uint16 dataSize = stream->readShort();

		for (int i = 0; i < dataSize; ++i) {
			uint32 nameHash = stream->readInt();
			uint32 varSize = stream->readInt();
			int offset = stream->getOffset();

			if (nameHash != variableHashCode)
				stream->shiftOffset(varSize);
			else {
				stream->reset();
				return offset;
			}
		}

		stream->reset();
		return -1;
	}

	// Inline helper: replace a variable's data within a serialized object stream.
	// Returns a new stream with the variable replaced, or nullptr if variable not found.
	static ObjectOutputStream* changeVariableData(const uint32 variableHashCode, ObjectInputStream* object, Stream* newVariableData) {
		int offset = getVariableDataOffset(variableHashCode, object);

		if (offset == -1)
			return nullptr;

		auto newData = new ObjectOutputStream(object->size());
		object->copy(newData);

		object->reset();
		newData->reset();
		object->shiftOffset(offset - 4);
		uint32 dataSize = object->readInt();

		newData->shiftOffset(offset);

		if (dataSize > 0)
			newData->removeRange(offset, offset + dataSize);

		newData->writeInt(offset - 4, newVariableData->size());
		newData->insertStream(newVariableData, newVariableData->size(), offset);

		object->reset();

		return newData;
	}

public:

	// Infinity: Remove all AntiDecayKit objects from sceneobjects database.
	// Deletes standalone ADK objects directly from the raw DB — no object
	// broker, no locking, no ZoneServer needed.
	static int removeADK() {
		Logger logger("RemoveADK");

		logger.info(true) << "=== AntiDecayKit Removal Tool ===";
		logger.info(true) << "Initializing...";

		ConfigManager::instance()->loadConfigData();

		auto dbManager = ObjectDatabaseManager::instance();

		if (dbManager == nullptr) {
			logger.error("Failed to get ObjectDatabaseManager instance.");
			return 1;
		}

		dbManager->loadDatabases(false);

		ObjectDatabase* database = dbManager->loadObjectDatabase("sceneobjects", true);

		if (database == nullptr) {
			logger.error("Failed to load sceneobjects database.");
			return 1;
		}

		logger.info(true) << "Scanning sceneobjects database for AntiDecayKit objects...";

		// Phase 1: Collect ADK OIDs and items that reference them
		HashSet<uint64> adkOIDSet;
		VectorMap<uint64, uint64> appliedADKMap; // parentItemOID -> adkOID
		HashTable<String, int> classCountMap(200, 0.75f); // class name -> count
		int totalScanned = 0;

		{
			ObjectDatabaseIterator iterator(database);
			ObjectInputStream objectData(2000);
			uint64 objectID = 0;

			while (iterator.getNextKeyAndValue(objectID, &objectData)) {
				totalScanned++;

				if (totalScanned % 500000 == 0) {
					logger.info(true) << "  Progress: " << totalScanned << " scanned, "
						<< adkOIDSet.size() << " ADKs, "
						<< appliedADKMap.size() << " items with applied ADKs, "
						<< classCountMap.size() << " unique classes...";
				}

				try {
					String className;
					uint64 adkOID = 0;

					if (Serializable::getVariable<String>(STRING_HASHCODE("_className"), &className, &objectData)) {
						// Track class counts
						int count = classCountMap.get(className);
						classCountMap.put(className, count + 1);

						if (className == "AntiDecayKit") {
							adkOIDSet.add(objectID);
							logger.info(true) << "  Found ADK: OID=" << objectID
								<< " dataSize=" << objectData.size();
						} else if (Serializable::getVariable<uint64>(HASH_ADK_FIELD, &adkOID, &objectData)) {
							// Infinity: Check for applied ADK reference on TangibleObjects
							// TangibleObject.antiDecayKitObject serializes as a uint64 OID
							if (adkOID != 0) {
								appliedADKMap.put(objectID, adkOID);
							}
						}
					}
				} catch (...) {
				}

				objectData.clear();
			}
		}

		logger.info(true) << "Scan complete. Scanned " << totalScanned << " objects.";
		logger.info(true) << "  Unique classes found: " << classCountMap.size();

		// Log all classes and counts
		auto classIterator = classCountMap.iterator();

		while (classIterator.hasNext()) {
			String key;
			int value = 0;

			classIterator.getNextKeyAndValue(key, value);
			logger.info(true) << "  class=" << key << " count=" << value;
		}

		logger.info(true) << "  Standalone ADK objects: " << adkOIDSet.size();
		logger.info(true) << "  Items with applied ADKs: " << appliedADKMap.size();

		if (adkOIDSet.size() == 0 && appliedADKMap.size() == 0) {
			logger.info(true) << "Nothing to remove.";
			return 0;
		}

		int deleted = 0;

		// Phase 2: Log applied ADKs (which item has which ADK)
		for (int i = 0; i < appliedADKMap.size(); i++) {
			uint64 parentOID = appliedADKMap.elementAt(i).getKey();
			uint64 adkOID = appliedADKMap.elementAt(i).getValue();

			logger.info(true) << "  Applied ADK " << adkOID << " on item " << parentOID;

			// Make sure this ADK is in the delete set
			adkOIDSet.add(adkOID);
		}

		// Phase 3: Delete all ADK entries from the database
		auto adkIterator = adkOIDSet.iterator();

		while (adkIterator.hasNext()) {
			uint64 oid = adkIterator.next();

			logger.info(true) << "  Deleting ADK OID=" << oid;

			database->deleteData(oid);
			deleted++;

			// Commit in batches to avoid huge transaction logs
			if (deleted % 100 == 0) {
				ObjectDatabaseManager::instance()->commitLocalTransaction();
			}
		}

		// Final commit
		ObjectDatabaseManager::instance()->commitLocalTransaction();

		logger.info(true) << "=== Removal Complete ===";
		logger.info(true) << "  Total scanned: " << totalScanned;
		logger.info(true) << "  ADKs deleted:  " << deleted;
		logger.info(true) << "  (Applied ADK references on " << appliedADKMap.size() << " items will resolve to null on next server load)";

		return 0;
	}

	// Infinity: Scan object databases for entries whose serverObjectCRC
	// no longer maps to a valid template. Default is dry-run (report only).
	// Pass doDelete=true to actually remove invalid entries.
	static int purgeInvalidCRC(bool doDelete) {
		Logger logger("PurgeInvalidCRC");

		logger.info(true) << "=== Purge Invalid CRC Tool ===";

		if (doDelete) {
			logger.info(true) << "*** DELETE MODE - invalid entries WILL be removed ***";
			logger.info(true) << "*** Make sure you have a database backup! ***";
		} else {
			logger.info(true) << "DRY RUN mode - no entries will be deleted. Use './core3 purgeinvalidcrc delete' to delete.";
		}

		logger.info(true) << "Initializing...";

		ConfigManager::instance()->loadConfigData();

		// Initialize TemplateManager (loads TRE archive, slots, etc.)
		auto templateManager = TemplateManager::instance();

		if (templateManager == nullptr) {
			logger.error("Failed to initialize TemplateManager.");
			return 1;
		}

		// Load all object templates (CRC -> template mappings)
		logger.info(true) << "Loading object templates...";
		templateManager->loadLuaTemplates();

		auto dbManager = ObjectDatabaseManager::instance();

		if (dbManager == nullptr) {
			logger.error("Failed to get ObjectDatabaseManager instance.");
			return 1;
		}

		dbManager->loadDatabases(false);

		const char* dbNames[] = {
			"sceneobjects",
			"playerstructures",
			"draftschematics",
			"cityregions",
			"guilds",
			"resourcespawns",
			nullptr
		};

		int grandTotalScanned = 0;
		int grandTotalInvalid = 0;
		int grandTotalDeleted = 0;

		for (int d = 0; dbNames[d] != nullptr; d++) {
			String dbName = dbNames[d];

			ObjectDatabase* database = dbManager->loadObjectDatabase(dbName, true);

			if (database == nullptr) {
				logger.warning() << "Could not load database: " << dbName << " - skipping.";
				continue;
			}

			logger.info(true) << "Scanning database: " << dbName;

			int totalScanned = 0;
			int totalInvalid = 0;
			int totalDeleted = 0;

			ObjectDatabaseIterator iterator(database);
			ObjectInputStream objectData(2000);
			uint64 objectID = 0;

			while (iterator.getNextKeyAndValue(objectID, &objectData)) {
				totalScanned++;

				if (totalScanned % 500000 == 0) {
					logger.info(true) << "  Progress: " << totalScanned << " scanned, " << totalInvalid << " invalid so far...";
				}

				try {
					uint32 serverObjectCRC = 0;

					if (Serializable::getVariable<uint32>(HASH_SERVER_CRC, &serverObjectCRC, &objectData)) {
						// CRC 0 means no template set (e.g. ManagedObject entries) — skip
						if (serverObjectCRC != 0 && !templateManager->existsTemplate(serverObjectCRC)) {
							totalInvalid++;

							// Read className for logging
							String className;
							Serializable::getVariable<String>(STRING_HASHCODE("_className"), &className, &objectData);

							logger.info(true) << "  INVALID: OID=" << objectID
								<< " CRC=0x" << hex << serverObjectCRC
								<< " class=" << className
								<< " db=" << dbName;

							if (doDelete) {
								database->deleteData(objectID);
								totalDeleted++;

								// Commit in batches
								if (totalDeleted % 1000 == 0) {
									ObjectDatabaseManager::instance()->commitLocalTransaction();
								}
							}
						}
					}
				} catch (...) {
				}

				objectData.clear();
			}

			// Commit remaining deletes for this database
			if (doDelete && totalDeleted > 0) {
				ObjectDatabaseManager::instance()->commitLocalTransaction();
			}

			logger.info(true) << "  " << dbName << ": " << totalScanned << " scanned, " << totalInvalid << " invalid"
				<< (doDelete ? String(", ") + String::valueOf(totalDeleted) + " deleted" : String(""));

			grandTotalScanned += totalScanned;
			grandTotalInvalid += totalInvalid;
			grandTotalDeleted += totalDeleted;
		}

		logger.info(true) << "";
		logger.info(true) << "=== Summary ===";
		logger.info(true) << "  Total scanned: " << grandTotalScanned;
		logger.info(true) << "  Total invalid: " << grandTotalInvalid;

		if (doDelete) {
			logger.info(true) << "  Total deleted: " << grandTotalDeleted;
		} else {
			logger.info(true) << "  Total deleted: 0 (dry run)";
		}

		return 0;
	}

	// Infinity: Delete a single object by OID from whichever database contains it.
	// Searches all known databases for the OID, logs what it finds, then deletes.
	static int deleteSingleObject(uint64 targetOID) {
		Logger logger("DeleteSingleObject");

		logger.info(true) << "=== Delete Single Object Tool ===";
		logger.info(true) << "Target OID: " << targetOID;
		logger.info(true) << "Initializing...";

		ConfigManager::instance()->loadConfigData();

		auto dbManager = ObjectDatabaseManager::instance();

		if (dbManager == nullptr) {
			logger.error("Failed to get ObjectDatabaseManager instance.");
			return 1;
		}

		dbManager->loadDatabases(false);

		const char* dbNames[] = {
			"sceneobjects",
			"playerstructures",
			"draftschematics",
			"cityregions",
			"guilds",
			"resourcespawns",
			nullptr
		};

		for (int d = 0; dbNames[d] != nullptr; d++) {
			String dbName = dbNames[d];

			ObjectDatabase* database = dbManager->loadObjectDatabase(dbName, true);

			if (database == nullptr) {
				continue;
			}

			ObjectInputStream objectData(2000);

			if (database->getData(targetOID, &objectData) == 0) {
				// Found it — log details before deleting
				String className = "(unknown)";
				uint32 serverObjectCRC = 0;

				Serializable::getVariable<String>(STRING_HASHCODE("_className"), &className, &objectData);
				Serializable::getVariable<uint32>(HASH_SERVER_CRC, &serverObjectCRC, &objectData);

				logger.info(true) << "Found OID " << targetOID << " in database: " << dbName;
				logger.info(true) << "  Class: " << className;
				logger.info(true) << "  CRC: 0x" << hex << serverObjectCRC;

				logger.info(true) << "Deleting OID " << targetOID << " from " << dbName << "...";
				database->deleteData(targetOID);
				ObjectDatabaseManager::instance()->commitLocalTransaction();

				logger.info(true) << "Done. Object deleted.";
				return 0;
			}
		}

		logger.info(true) << "OID " << targetOID << " not found in any database.";
		return 1;
	}
	// Infinity: Purge all ship/JTL-related objects from the database.
	// Use when JTL is disabled and ship objects are causing deadlocks
	// during character login (removeAllPlayersFromShip in notifyLoadFromDatabase).
	// Default is dry-run (report only). Pass doDelete=true to actually remove.
	static int purgeShips(bool doDelete) {
		Logger logger("PurgeShips");

		logger.info(true) << "=== Purge Ship/JTL Objects Tool ===";

		if (doDelete) {
			logger.info(true) << "*** DELETE MODE - ship objects WILL be removed ***";
			logger.info(true) << "*** Make sure you have a database backup! ***";
		} else {
			logger.info(true) << "DRY RUN mode - no entries will be deleted. Use './core3 purgeships delete' to delete.";
		}

		logger.info(true) << "Initializing...";

		ConfigManager::instance()->loadConfigData();

		auto dbManager = ObjectDatabaseManager::instance();

		if (dbManager == nullptr) {
			logger.error("Failed to get ObjectDatabaseManager instance.");
			return 1;
		}

		dbManager->loadDatabases(false);

		ObjectDatabase* database = dbManager->loadObjectDatabase("sceneobjects", true);

		if (database == nullptr) {
			logger.error("Failed to load sceneobjects database.");
			return 1;
		}

		// All ship/JTL class names that can exist in the database
		HashSet<String> shipClasses;
		shipClasses.add("ShipObject");
		shipClasses.add("FighterShipObject");
		shipClasses.add("MultiPassengerShipObject");
		shipClasses.add("PobShipObject");
		shipClasses.add("ShipControlDevice");
		shipClasses.add("ShipDeed");
		shipClasses.add("ShipComponent");
		shipClasses.add("ShipArmorComponent");
		shipClasses.add("ShipBoosterComponent");
		shipClasses.add("ShipCapacitorComponent");
		shipClasses.add("ShipChassisComponent");
		shipClasses.add("ShipCounterMeasureComponent");
		shipClasses.add("ShipDroidInterfaceComponent");
		shipClasses.add("ShipEngineComponent");
		shipClasses.add("ShipMissileComponent");
		shipClasses.add("ShipReactorComponent");
		shipClasses.add("ShipShieldComponent");
		shipClasses.add("ShipSubComponent");
		shipClasses.add("ShipWeaponComponent");
		shipClasses.add("ShipPaintKit");
		shipClasses.add("ShipComponentRepairKit");
		shipClasses.add("ShipPlasmaConduit");
		shipClasses.add("ShipInteriorComponent");
		shipClasses.add("ShipAiAgent");
		shipClasses.add("CapitalShipObject");
		shipClasses.add("SpaceStationObject");

		logger.info(true) << "Scanning sceneobjects database for " << shipClasses.size() << " ship/JTL class types...";

		HashTable<String, int> classCounts(50, 0.75f);
		Vector<uint64> deleteOIDs;
		int totalScanned = 0;

		{
			ObjectDatabaseIterator iterator(database);
			ObjectInputStream objectData(2000);
			uint64 objectID = 0;

			while (iterator.getNextKeyAndValue(objectID, &objectData)) {
				totalScanned++;

				if (totalScanned % 500000 == 0) {
					logger.info(true) << "  Progress: " << totalScanned << " scanned, "
						<< deleteOIDs.size() << " ship objects found...";
				}

				try {
					String className;

					if (Serializable::getVariable<String>(STRING_HASHCODE("_className"), &className, &objectData)) {
						if (shipClasses.contains(className)) {
							deleteOIDs.add(objectID);

							int count = classCounts.get(className);
							classCounts.put(className, count + 1);
						}
					}
				} catch (...) {
				}

				objectData.clear();
			}
		}

		logger.info(true) << "Scan complete. Scanned " << totalScanned << " objects.";
		logger.info(true) << "Found " << deleteOIDs.size() << " ship/JTL objects:";

		auto classIterator = classCounts.iterator();

		while (classIterator.hasNext()) {
			String key;
			int value = 0;

			classIterator.getNextKeyAndValue(key, value);
			logger.info(true) << "  " << key << ": " << value;
		}

		if (deleteOIDs.size() == 0) {
			logger.info(true) << "No ship objects found. Nothing to do.";
			return 0;
		}

		if (!doDelete) {
			logger.info(true) << "DRY RUN complete. " << deleteOIDs.size() << " objects would be deleted.";
			logger.info(true) << "Run './core3 purgeships delete' to actually remove them.";
			return 0;
		}

		logger.info(true) << "Deleting " << deleteOIDs.size() << " ship objects...";

		int deleted = 0;

		for (int i = 0; i < deleteOIDs.size(); i++) {
			database->deleteData(deleteOIDs.get(i));
			deleted++;

			if (deleted % 1000 == 0) {
				ObjectDatabaseManager::instance()->commitLocalTransaction();
				logger.info(true) << "  Deleted " << deleted << " / " << deleteOIDs.size() << "...";
			}
		}

		ObjectDatabaseManager::instance()->commitLocalTransaction();

		logger.info(true) << "=== Purge Complete ===";
		logger.info(true) << "  Total scanned:  " << totalScanned;
		logger.info(true) << "  Ship objects deleted: " << deleted;

		return 0;
	}

	// Infinity: Show object details from the database.
	// Usage: ./core3 showobject <OID> [<OID2> ...]
	// Reads the raw serialized data and dumps class, template, parent, and
	// crafting attribute values (Component.attributeMap).
	static int showObject(int argc, char* argv[]) {
		Logger logger("ShowObject");

		logger.info(true) << "=== Show Object Tool ===";

		ConfigManager::instance()->loadConfigData();

		auto templateManager = TemplateManager::instance();
		if (templateManager != nullptr) {
			templateManager->loadLuaTemplates();
		}

		auto dbManager = ObjectDatabaseManager::instance();
		if (dbManager == nullptr) {
			logger.error("Failed to get ObjectDatabaseManager instance.");
			return 1;
		}

		dbManager->loadDatabases(false);

		ObjectDatabase* database = dbManager->loadObjectDatabase("sceneobjects", true);
		if (database == nullptr) {
			logger.error("Failed to load sceneobjects database.");
			return 1;
		}

		for (int i = 1; i < argc; i++) {
			String arg = argv[i];
			if (arg == "showobject")
				continue;

			uint64 targetOID = 0;
			try {
				targetOID = UnsignedLong::valueOf(arg);
			} catch (...) {
				continue;
			}

			if (targetOID == 0)
				continue;

			ObjectInputStream objectData(4000);

			if (database->getData(targetOID, &objectData) != 0) {
				logger.info(true) << "OID " << targetOID << ": NOT FOUND";
				continue;
			}

			logger.info(true) << "========================================";
			logger.info(true) << "OID: " << targetOID;

			// Class name
			String className;
			if (Serializable::getVariable<String>(STRING_HASHCODE("_className"), &className, &objectData)) {
				logger.info(true) << "  Class: " << className;
			}

			// Server CRC -> template path
			uint32 serverObjectCRC = 0;
			if (Serializable::getVariable<uint32>(HASH_SERVER_CRC, &serverObjectCRC, &objectData)) {
				logger.info(true) << "  CRC: 0x" << hex << serverObjectCRC;
				if (templateManager != nullptr) {
					auto tmpl = templateManager->getTemplate(serverObjectCRC);
					if (tmpl != nullptr) {
						logger.info(true) << "  Template: " << tmpl->getFullTemplateString();
					}
				}
			}

			// Parent OID
			uint64 parentOID = 0;
			if (Serializable::getVariable<uint64>(STRING_HASHCODE("SceneObject.parent"), &parentOID, &objectData)) {
				logger.info(true) << "  Parent OID: " << parentOID;
			}

			// Custom name (unicode)
			UnicodeString customName;
			if (Serializable::getVariable<UnicodeString>(STRING_HASHCODE("TangibleObject.customName"), &customName, &objectData)) {
				if (!customName.isEmpty()) {
					logger.info(true) << "  Custom Name: " << customName.toString();
				}
			}

			// Serial number
			String serialNumber;
			if (Serializable::getVariable<String>(STRING_HASHCODE("TangibleObject.craftersSerial"), &serialNumber, &objectData)) {
				if (!serialNumber.isEmpty()) {
					logger.info(true) << "  Serial: " << serialNumber;
				}
			}

			// Component attributeMap (VectorMap<String, float>)
			VectorMap<String, float> attributeMap;
			if (Serializable::getVariable<VectorMap<String, float>>(STRING_HASHCODE("Component.attributeMap"), &attributeMap, &objectData)) {
				if (attributeMap.size() > 0) {
					logger.info(true) << "  --- Crafting Attributes ---";
					for (int j = 0; j < attributeMap.size(); j++) {
						auto& key = attributeMap.elementAt(j).getKey();
						float val = attributeMap.elementAt(j).getValue();
						logger.info(true) << "    " << key << " = " << val;
					}
				}
			}

			// Also try ArmorComponent effectiveness values (stored differently)
			// LightsaberCrystalComponent values
			// These use different variable names depending on class

			logger.info(true) << "  Data size: " << objectData.size() << " bytes";
		}

		return 0;
	}

	// Infinity: Purge a specific ability from all PlayerObject ability lists.
	// Default is dry-run (report only). Pass doDelete=true to actually modify entries.
	// Usage: ./core3 purgeability <name>          (dry run)
	//        ./core3 purgeability <name> delete   (modify database)
	static int purgeAbility(const String& abilityName, bool doDelete) {
		Logger logger("PurgeAbility");

		logger.info(true) << "=== Purge Ability Tool ===";
		logger.info(true) << "Target ability: " << abilityName;

		if (doDelete) {
			logger.info(true) << "*** DELETE MODE - ability entries WILL be removed from player data ***";
			logger.info(true) << "*** Make sure you have a database backup! ***";
		} else {
			logger.info(true) << "DRY RUN mode - no entries will be modified. Use './core3 purgeability " << abilityName << " delete' to remove.";
		}

		logger.info(true) << "Initializing...";

		ConfigManager::instance()->loadConfigData();

		auto dbManager = ObjectDatabaseManager::instance();

		if (dbManager == nullptr) {
			logger.error("Failed to get ObjectDatabaseManager instance.");
			return 1;
		}

		dbManager->loadDatabases(false);

		ObjectDatabase* database = dbManager->loadObjectDatabase("sceneobjects", true);

		if (database == nullptr) {
			logger.error("Failed to load sceneobjects database.");
			return 1;
		}

		logger.info(true) << "Scanning sceneobjects database for PlayerObjects with ability '" << abilityName << "'...";

		int totalScanned = 0;
		int totalPlayerObjects = 0;
		int totalMatched = 0;
		int totalModified = 0;

		{
			ObjectDatabaseIterator iterator(database);
			ObjectInputStream objectData(2000);
			uint64 objectID = 0;

			while (iterator.getNextKeyAndValue(objectID, &objectData)) {
				totalScanned++;

				if (totalScanned % 500000 == 0) {
					logger.info(true) << "  Progress: " << totalScanned << " scanned, "
						<< totalPlayerObjects << " PlayerObjects, "
						<< totalMatched << " with target ability...";
				}

				try {
					String className;

					if (!Serializable::getVariable<String>(HASH_CLASSNAME, &className, &objectData)) {
						objectData.clear();
						continue;
					}

					if (className != "PlayerObject") {
						objectData.clear();
						continue;
					}

					totalPlayerObjects++;

					AbilityListMigrator abilityList;

					if (!Serializable::getVariable<AbilityListMigrator>(HASH_ABILITYLIST, &abilityList, &objectData)) {
						objectData.clear();
						continue;
					}

					// Check if this player has the target ability
					bool found = false;

					for (int i = 0; i < abilityList.names.size(); i++) {
						if (abilityList.names.get(i) == abilityName) {
							found = true;
							break;
						}
					}

					if (!found) {
						objectData.clear();
						continue;
					}

					totalMatched++;

					logger.info(true) << "  Found: PlayerObject OID=" << objectID
						<< " abilityCount=" << abilityList.names.size();

					if (doDelete) {
						// Remove all instances of the target ability
						for (int i = abilityList.names.size() - 1; i >= 0; i--) {
							if (abilityList.names.get(i) == abilityName) {
								abilityList.names.remove(i);
							}
						}

						// Serialize the modified ability list
						ObjectOutputStream data;
						TypeInfo<uint32>::toBinaryStream(&abilityList.updateCounter, &data);
						abilityList.names.toBinaryStream(&data);

						// Replace the variable in the object stream
						ObjectOutputStream* newStream = changeVariableData(HASH_ABILITYLIST, &objectData, &data);

						if (newStream != nullptr) {
							newStream->reset();
							database->putData(objectID, newStream, nullptr);
							// Do NOT delete newStream: putData only stores the pointer in the local
							// transaction; the stream is compressed+written later at
							// commitLocalTransaction. Deleting now is a use-after-free that segfaults
							// at commit (released when the process exits — offline one-shot).
							totalModified++;

							// Commit in batches
							if (totalModified % 1000 == 0) {
								ObjectDatabaseManager::instance()->commitLocalTransaction();
							}
						} else {
							logger.error() << "  Failed to replace ability list for OID=" << objectID;
						}
					}
				} catch (...) {
				}

				objectData.clear();
			}
		}

		// Final commit
		if (doDelete && totalModified > 0) {
			ObjectDatabaseManager::instance()->commitLocalTransaction();
		}

		logger.info(true) << "";
		logger.info(true) << "=== Summary ===";
		logger.info(true) << "  Total scanned:        " << totalScanned;
		logger.info(true) << "  Total PlayerObjects:  " << totalPlayerObjects;
		logger.info(true) << "  With '" << abilityName << "': " << totalMatched;

		if (doDelete) {
			logger.info(true) << "  Modified:             " << totalModified;
		} else {
			logger.info(true) << "  Modified:             0 (dry run)";
		}

		return 0;
	}

	// Infinity: Reconcile persisted PlayerObject.accountID against the authoritative
	// MySQL characters.account_id. When a character is manually moved between accounts
	// (UPDATE characters SET account_id=... ) the persisted ghost keeps the OLD accountID,
	// which causes FRS wipes at maintenance. This scans every ghost, reports mismatches,
	// and in fix mode rewrites the stale accountID in the BDB.
	//
	// Default is dry-run (report only). Pass doFix=true to actually rewrite.
	//
	// Mapping: ghost(PlayerObject).accountID is keyed by the ghost OID, but MySQL is
	// keyed by character_oid = the owning CreatureObject OID. We recover the link from
	// each CreatureObject's slottedObjects["ghost"] (the authoritative persisted container
	// link; see SceneObjectImplementation::notifyLoadFromDatabase).
	static int reconcileAccountIDs(bool doFix, bool doResetOrphans) {
		Logger logger("ReconcileAccountIDs");

		logger.info(true) << "=== Reconcile Account IDs Tool ===";

		if (doFix) {
			logger.info(true) << "*** FIX MODE - stale accountID fields WILL be rewritten in the BDB ***";
			logger.info(true) << "*** Make sure you have a database backup! ***";
		} else {
			logger.info(true) << "DRY RUN mode - no accountID fields will be modified. Use './core3 reconcileaccountids fix' to apply.";
		}

		if (doResetOrphans) {
			logger.info(true) << "*** RESET-ORPHANS MODE - orphaned deleted-characters WILL be set db_deleted=0 (re-queued for purge) ***";
		} else {
			logger.info(true) << "Orphan-reset OFF. Add 'resetorphans' to re-queue orphaned deleted-characters for purge.";
		}

		logger.info(true) << "Initializing...";

		ConfigManager::instance()->loadConfigData();

		// --- Phase 0: load authoritative characters table from MySQL ---
		VectorMap<uint64, uint32> mysqlAccountByChar; // character_oid -> account_id
		mysqlAccountByChar.setNullValue(0);

		ServerDatabase* serverDatabase = nullptr;

		try {
			serverDatabase = new ServerDatabase(ConfigManager::instance());
		} catch (const Exception& e) {
			logger.error() << "Failed to initialize ServerDatabase (MySQL): " << e.getMessage();
			return 1;
		}

		int galaxyID = ConfigManager::instance()->getZoneGalaxyID();

		try {
			String query = "SELECT character_oid, account_id FROM characters WHERE galaxy_id = "
				+ String::valueOf(galaxyID);

			UniqueReference<ResultSet*> res(ServerDatabase::instance()->executeQuery(query));

			while (res != nullptr && res->next()) {
				uint64 charOID = res->getUnsignedLong(0);
				uint32 accountID = (uint32) res->getInt(1);

				mysqlAccountByChar.put(charOID, accountID);
			}
		} catch (const Exception& e) {
			logger.error() << "Failed to query characters table: " << e.getMessage();
			delete serverDatabase;
			return 1;
		}

		logger.info(true) << "Loaded " << mysqlAccountByChar.size()
			<< " characters from MySQL (galaxy_id=" << galaxyID << ").";

		if (mysqlAccountByChar.size() == 0) {
			logger.warning() << "No characters loaded from MySQL - aborting (wrong galaxy_id or empty DB?).";
			delete serverDatabase;
			return 1;
		}

		// --- Phase 0b: load ALL deleted_characters rows (any db_deleted), to classify/re-queue orphans ---
		// We load every row (not just db_deleted=1) so a BDB orphan is recognised as "legitimately
		// deleted, objects not yet purged" regardless of its flag. This keeps reruns sane: after
		// resetorphans flips a row to db_deleted=0, a verify pass still classifies it as resettable
		// (not as a stray needing manual review).
		VectorMap<uint64, int> deletedChars; // character_oid -> 1
		deletedChars.setNullValue(0);

		try {
			String dquery = "SELECT character_oid FROM deleted_characters WHERE galaxy_id = "
				+ String::valueOf(galaxyID);

			UniqueReference<ResultSet*> dres(ServerDatabase::instance()->executeQuery(dquery));

			while (dres != nullptr && dres->next()) {
				deletedChars.put(dres->getUnsignedLong(0), 1);
			}
		} catch (const Exception& e) {
			logger.error() << "Failed to query deleted_characters table: " << e.getMessage();
			delete serverDatabase;
			return 1;
		}

		logger.info(true) << "Loaded " << deletedChars.size()
			<< " deleted_characters rows (galaxy_id=" << galaxyID << ").";

		// --- Phase 1: single BDB pass; build ghostOID->creatureOID and ghostOID->accountID ---
		auto dbManager = ObjectDatabaseManager::instance();

		if (dbManager == nullptr) {
			logger.error("Failed to get ObjectDatabaseManager instance.");
			delete serverDatabase;
			return 1;
		}

		dbManager->loadDatabases(false);

		ObjectDatabase* database = dbManager->loadObjectDatabase("sceneobjects", true);

		if (database == nullptr) {
			logger.error("Failed to load sceneobjects database.");
			delete serverDatabase;
			return 1;
		}

		logger.info(true) << "Scanning sceneobjects database...";

		VectorMap<uint64, uint64> creatureByGhost;   // ghost OID -> creature OID
		VectorMap<uint64, uint32> accountByGhost;     // ghost OID -> persisted accountID
		creatureByGhost.setNullValue(0);
		accountByGhost.setNullValue(0);

		int totalScanned = 0;
		int totalCreatures = 0;
		int totalGhosts = 0;

		{
			ObjectDatabaseIterator iterator(database);
			ObjectInputStream objectData(2000);
			uint64 objectID = 0;

			while (iterator.getNextKeyAndValue(objectID, &objectData)) {
				totalScanned++;

				if (totalScanned % 500000 == 0) {
					logger.info(true) << "  Progress: " << totalScanned << " scanned, "
						<< totalCreatures << " creatures, " << totalGhosts << " ghosts...";
				}

				try {
					String className;

					if (!Serializable::getVariable<String>(HASH_CLASSNAME, &className, &objectData)) {
						objectData.clear();
						continue;
					}

					if (className == "PlayerObject") {
						uint32 accountID = 0;

						if (Serializable::getVariable<uint32>(HASH_ACCOUNTID, &accountID, &objectData)) {
							totalGhosts++;
							accountByGhost.put(objectID, accountID);
						}
					} else if (className == "CreatureObject") {
						// Read slottedObjects; a "ghost" slot marks a player creature.
						// ManagedReference<SceneObject*> serializes as a bare uint64 OID,
						// so VectorMap<String,uint64> is binary-compatible.
						VectorMap<String, uint64> slotted;

						if (Serializable::getVariable<VectorMap<String, uint64>>(HASH_SLOTTEDOBJECTS, &slotted, &objectData)) {
							int idx = slotted.find(String("ghost"));

							if (idx != -1) {
								uint64 ghostOID = slotted.get(idx);

								if (ghostOID != 0) {
									totalCreatures++;
									creatureByGhost.put(ghostOID, objectID);
								}
							}
						}
					}
				} catch (...) {
				}

				objectData.clear();
			}
		}

		logger.info(true) << "Scan complete. Scanned " << totalScanned << " objects, "
			<< totalCreatures << " player creatures, " << totalGhosts << " ghosts.";

		// --- Phase 2: join + compare + (optional) fix ---
		int mismatches = 0;
		int fixed = 0;
		int ghostNoCreature = 0;   // ghost with no owning creature found in BDB
		int charNotInMysql = 0;    // creature exists in BDB but not in MySQL characters
		int skippedZeroGhost = 0;  // ghost accountID == 0 (never logged in / placeholder)
		int resettableOrphans = 0; // orphan that IS a pending deleted_characters row (re-queueable)
		int strayOrphans = 0;      // orphan in BDB but in neither characters nor deleted_characters
		int orphansReset = 0;      // deleted_characters rows set db_deleted=0
		int straysInserted = 0;    // deleted_characters rows INSERTed for strays
		Vector<uint64> orphanResetOIDs;
		Vector<uint64> strayInsertOIDs;
		Vector<uint32> strayInsertAccts; // parallel to strayInsertOIDs

		for (int i = 0; i < accountByGhost.size(); i++) {
			uint64 ghostOID = accountByGhost.elementAt(i).getKey();
			uint32 ghostAccountID = accountByGhost.elementAt(i).getValue();

			if (ghostAccountID == 0) {
				skippedZeroGhost++;
				continue;
			}

			int cIdx = creatureByGhost.find(ghostOID);

			if (cIdx == -1) {
				ghostNoCreature++;
				continue;
			}

			uint64 creatureOID = creatureByGhost.get(cIdx);

			int mIdx = mysqlAccountByChar.find(creatureOID);

			if (mIdx == -1) {
				// Character in BDB but not in MySQL `characters` — it was deleted but its objects
				// still exist (the orphaned-deletion bug). If it's a pending deleted_characters row,
				// re-queue it (db_deleted=0) so the fixed DeleteCharactersTask purges it full-tree
				// on the next server run.
				charNotInMysql++;

				if (deletedChars.find(creatureOID) != -1) {
					resettableOrphans++;
					orphanResetOIDs.add(creatureOID);
					logger.info(true) << "  ORPHAN (deleted, objects still in BDB): charOID=" << creatureOID
						<< " ghostOID=" << ghostOID
						<< (doResetOrphans ? " -> set db_deleted=0 (re-queue for purge)" : " (resettable)");
				} else {
					// In BDB but in neither `characters` nor `deleted_characters` — a failed
					// character creation (the BDB object tree was persisted before the MySQL row
					// was written; the creation then aborted by crash/kill, e.g. during a MySQL
					// lock, bypassing the RAII orphan guard). No MySQL identity at all, never on
					// the character-select list. INSERT a db_deleted=0 row so the fixed
					// DeleteCharactersTask purges the full tree (creature+ghost+contents) online.
					strayOrphans++;
					strayInsertOIDs.add(creatureOID);
					strayInsertAccts.add(ghostAccountID);
					logger.info(true) << "  STRAY (in BDB, not in characters or deleted_characters): charOID="
						<< creatureOID << " ghostOID=" << ghostOID << " ghostAccountID=" << ghostAccountID
						<< (doResetOrphans ? " -> INSERT db_deleted=0 (queue full-tree purge)" : " (would queue purge w/ resetorphans)");
				}

				continue;
			}

			uint32 mysqlAccountID = mysqlAccountByChar.get(mIdx);

			if (mysqlAccountID == 0) {
				// MySQL account_id 0/null — treat as untrustworthy, report but do not fix.
				logger.info(true) << "  MYSQL ACCOUNT 0/NULL: charOID=" << creatureOID
					<< " ghostOID=" << ghostOID << " ghostAccountID=" << ghostAccountID
					<< " (skipped)";
				continue;
			}

			if (ghostAccountID != mysqlAccountID) {
				mismatches++;

				logger.info(true) << "  MISMATCH: charOID=" << creatureOID
					<< " ghostOID=" << ghostOID
					<< " ghostAccountID=" << ghostAccountID
					<< " -> mysqlAccountID=" << mysqlAccountID;

				if (doFix) {
					ObjectInputStream ghostData(2000);

					if (database->getData(ghostOID, &ghostData) != 0) {
						logger.error() << "    Could not re-read ghost OID=" << ghostOID << " for fix.";
						continue;
					}

					// Serialize replacement uint32 accountID exactly as originally written.
					ObjectOutputStream data;
					TypeInfo<uint32>::toBinaryStream(&mysqlAccountID, &data);

					ObjectOutputStream* newStream = changeVariableData(HASH_ACCOUNTID, &ghostData, &data);

					if (newStream != nullptr) {
						newStream->reset();
						database->putData(ghostOID, newStream, nullptr);
						// Do NOT delete newStream here: putData only stores the pointer in the
						// local transaction (addUpdateObject); the stream is compressed+written
						// later at commitLocalTransaction. Deleting now is a use-after-free that
						// segfaults at commit. The stream is released when the process exits
						// (offline one-shot; bounded by the per-batch commit count).
						fixed++;

						if (fixed % 1000 == 0) {
							ObjectDatabaseManager::instance()->commitLocalTransaction();
						}
					} else {
						logger.error() << "    Failed to replace accountID for ghost OID=" << ghostOID
							<< " (PlayerObject.accountID field not found in stream).";
					}
				}
			}
		}

		if (doFix && fixed > 0) {
			ObjectDatabaseManager::instance()->commitLocalTransaction();
		}

		// Re-queue orphaned deleted-characters: set db_deleted=0 in batches so the fixed
		// DeleteCharactersTask purges them full-tree on the next server run.
		if (doResetOrphans && orphanResetOIDs.size() > 0) {
			for (int start = 0; start < orphanResetOIDs.size(); start += 500) {
				int end = start + 500;

				if (end > orphanResetOIDs.size())
					end = orphanResetOIDs.size();

				StringBuffer in;

				for (int i = start; i < end; i++) {
					if (i > start)
						in << ",";

					in << orphanResetOIDs.get(i);
				}

				String uq = "UPDATE deleted_characters SET db_deleted = 0 WHERE galaxy_id = "
					+ String::valueOf(galaxyID) + " AND character_oid IN (" + in.toString() + ")";

				try {
					// Use executeQuery (direct synchronous mysql_query), NOT executeStatement:
					// executeStatement routes through a MysqlTask on a custom task queue, which
					// is unsafe in this offline one-shot (no Core task scheduler running) and
					// segfaults at teardown. executeQuery handles INSERT/UPDATE (null result set).
					UniqueReference<ResultSet*> ur(ServerDatabase::instance()->executeQuery(uq));
					orphansReset += (end - start);
				} catch (const Exception& e) {
					logger.error() << "Failed to reset db_deleted for a batch: " << e.getMessage();
				}
			}
		}

		// Queue stray orphans for purge: INSERT a db_deleted=0 deleted_characters row for each
		// (no row exists to UPDATE). The fixed DeleteCharactersTask then loads the creature and
		// calls destroyPlayerCreatureFromDatabase(true), which recursively deletes the full tree
		// (ghost, inventory, bank, datapad, all contents) on the next server run. INSERT IGNORE
		// keeps reruns idempotent (a re-scan would reclassify the row as resettable anyway).
		if (doResetOrphans && strayInsertOIDs.size() > 0) {
			for (int start = 0; start < strayInsertOIDs.size(); start += 500) {
				int end = start + 500;

				if (end > strayInsertOIDs.size())
					end = strayInsertOIDs.size();

				StringBuffer values;

				for (int i = start; i < end; i++) {
					if (i > start)
						values << ",";

					values << "(" << strayInsertOIDs.get(i) << "," << strayInsertAccts.get(i)
						<< "," << galaxyID << ",'orphan-purge','orphan-purge',0)";
				}

				String iq = "INSERT IGNORE INTO deleted_characters "
					"(character_oid, account_id, galaxy_id, firstname, template, db_deleted) VALUES "
					+ values.toString();

				try {
					// executeQuery (direct synchronous mysql_query), not executeStatement — see
					// the orphan-reset UPDATE above for why (task-queue path segfaults offline).
					UniqueReference<ResultSet*> ir(ServerDatabase::instance()->executeQuery(iq));
					straysInserted += (end - start);
				} catch (const Exception& e) {
					logger.error() << "Failed to INSERT stray orphan batch: " << e.getMessage();
				}
			}
		}

		logger.info(true) << "";
		logger.info(true) << "=== Summary ===";
		logger.info(true) << "  Objects scanned:            " << totalScanned;
		logger.info(true) << "  Player creatures:           " << totalCreatures;
		logger.info(true) << "  Ghosts (PlayerObjects):     " << totalGhosts;
		logger.info(true) << "  MySQL characters loaded:    " << mysqlAccountByChar.size();
		logger.info(true) << "  Mismatches found:           " << mismatches;
		logger.info(true) << "  Skipped (ghost acct 0):     " << skippedZeroGhost;
		logger.info(true) << "  Ghosts w/o owning creature: " << ghostNoCreature;
		logger.info(true) << "  In BDB, not in MySQL:       " << charNotInMysql;
		logger.info(true) << "    - resettable (deleted):   " << resettableOrphans;
		logger.info(true) << "    - stray (manual review):  " << strayOrphans;
		logger.info(true) << "  Orphans re-queued (UPDATE db_deleted=0): " << orphansReset << (doResetOrphans ? "" : " (resetorphans off)");
		logger.info(true) << "  Strays queued (INSERT db_deleted=0):     " << straysInserted << (doResetOrphans ? "" : " (resetorphans off)");

		if (doFix) {
			logger.info(true) << "  Fixed (rewritten):          " << fixed;
		} else {
			logger.info(true) << "  Fixed (rewritten):          0 (dry run)";
		}

		delete serverDatabase;

		return 0;
	}

};

#endif /* OFFLINETOOLS_H_ */
