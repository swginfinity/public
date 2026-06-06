/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef OBJECTCOMMAND_H_
#define OBJECTCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/managers/loot/LootManager.h"
#include "server/zone/managers/crafting/CraftingManager.h"
#include "server/zone/managers/crafting/ComponentMap.h"
#include "server/zone/objects/tangible/terminal/characterbuilder/CharacterBuilderTerminal.h"
//Infinity:  Custom includes
#include "server/zone/objects/region/CityRegion.h"
#include "server/zone/objects/factorycrate/FactoryCrate.h"
#include "server/zone/managers/stringid/StringIdManager.h"
#include "server/zone/objects/building/BuildingObject.h"
#include "server/zone/managers/structure/tasks/DestroyStructureTask.h"
#include "server/zone/managers/gcw/GCWManager.h"
#include "templates/intangible/DraftSchematicObjectTemplate.h"
#include "server/zone/managers/resource/ResourceManager.h"
#include "server/zone/objects/resource/ResourceSpawn.h"
#include "server/zone/managers/loot/LootGroupMap.h"
#include "templates/LootItemTemplate.h"


class ObjectCommand : public QueueCommand {
public:

	ObjectCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

        auto zoneServer = server->getZoneServer();
        if (zoneServer == nullptr)
            return GENERALERROR;

		StringTokenizer args(arguments.toString());

		try {
			String commandType;
			args.getStringToken(commandType);

			if (commandType.beginsWith("createitem")) {
				String objectTemplate;
				args.getStringToken(objectTemplate);

				ManagedReference<CraftingManager*> craftingManager = zoneServer->getCraftingManager();
				if(craftingManager == nullptr) {
					return GENERALERROR;
				}

				Reference<SharedObjectTemplate*> shot = TemplateManager::instance()->getTemplate(objectTemplate.hashCode());

				if (shot == nullptr || !shot->isSharedTangibleObjectTemplate()) {
					creature->sendSystemMessage("Templates must be tangible objects, or descendants of tangible objects, only.");
					return INVALIDPARAMETERS;
				}

				ManagedReference<SceneObject*> inventory = creature->getSlottedObject("inventory");

				if (inventory == nullptr || inventory->isContainerFullRecursive()) {
					creature->sendSystemMessage("Your inventory is full, so the item could not be created.");
					return INVALIDPARAMETERS;
				}

				ManagedReference<TangibleObject*> object = (zoneServer->createObject(shot->getServerObjectCRC(), 1)).castTo<TangibleObject*>();

				if (object == nullptr) {
					creature->sendSystemMessage("The object '" + commandType + "' could not be created because the template could not be found.");
					return INVALIDPARAMETERS;
				}

				Locker locker(object);

				object->createChildObjects();

				// Set Crafter name and generate serial number
				String name = "Generated with Object Command";
				object->setCraftersName(name);

				StringBuffer customName;
				customName << object->getDisplayedName() <<  " (System Generated)";

				object->setCustomObjectName(customName.toString(), false);

				String serial = craftingManager->generateSerial();
				object->setSerialNumber(serial);

				int quantity = 1;

				if (args.hasMoreTokens())
					quantity = args.getIntToken();

				if(quantity > 1 && quantity <= 100)
					object->setUseCount(quantity);

				// load visible components
				while (args.hasMoreTokens()) {
					String visName;
					args.getStringToken(visName);

					uint32 visId = visName.hashCode();
					if (ComponentMap::instance()->getFromID(visId).getId() == 0)
						continue;

					object->addVisibleComponent(visId, false);
				}

				if (inventory->transferObject(object, -1, true)) {
					inventory->broadcastObject(object, true);
					creature->info(true) << "/object createitem " << objectTemplate << " created oid: " << object->getObjectID() << " \"" << object->getDisplayedName() << "\"";
				} else {
					object->destroyObjectFromDatabase(true);
					creature->sendSystemMessage("Error transferring object to inventory.");
				}
			}
			else if (commandType.beginsWith("createloot")) {
				String lootGroup;
				args.getStringToken(lootGroup);

				int level = 1;

				if (args.hasMoreTokens())
					level = args.getIntToken();

				ManagedReference<SceneObject*> inventory = creature->getSlottedObject("inventory");

				if (inventory == nullptr || inventory->isContainerFullRecursive()) {
					creature->sendSystemMessage("Your inventory is full, so the item could not be created.");
					return INVALIDPARAMETERS;
				}

				ManagedReference<LootManager*> lootManager = zoneServer->getLootManager();

				if (lootManager == nullptr)
					return INVALIDPARAMETERS;

				TransactionLog trx(TrxCode::ADMINCOMMAND, creature);
				trx.addState("commandType", commandType);
				if (lootManager->createLoot(trx, inventory, lootGroup, level) > 0) {
					creature->info(true) << "/object createloot " << lootGroup << " trxId: " << trx.getTrxID();
					trx.commit(true);
				} else {
					trx.abort() << "createLoot failed for lootGroup " << lootGroup << " level " << level;
				}
			}
			else if (commandType.beginsWith("createresource")) {
				String resourceName;
				args.getStringToken(resourceName);

				int quantity = 100000;

				if (args.hasMoreTokens())
					quantity = args.getIntToken();

				ManagedReference<ResourceManager*> resourceManager = zoneServer->getResourceManager();
				resourceManager->givePlayerResource(creature, resourceName, quantity);
			}
			// Infinity: createingredients - spawn all ingredients for a draft schematic
			else if (commandType.beginsWith("createingredients")) {
				String schematicTemplate;
				args.getStringToken(schematicTemplate);

				int quantity = 1;
				if (args.hasMoreTokens())
					quantity = args.getIntToken();

				if (quantity < 1 || quantity > 100) {
					creature->sendSystemMessage("Quantity must be between 1 and 100.");
					return INVALIDPARAMETERS;
				}

				// Look up the draft schematic template
				Reference<SharedObjectTemplate*> shot = TemplateManager::instance()->getTemplate(schematicTemplate.hashCode());

				if (shot == nullptr) {
					creature->sendSystemMessage("Template not found: " + schematicTemplate);
					return INVALIDPARAMETERS;
				}

				DraftSchematicObjectTemplate* draftTemplate = dynamic_cast<DraftSchematicObjectTemplate*>(shot.get());

				if (draftTemplate == nullptr) {
					creature->sendSystemMessage("Template is not a draft schematic: " + schematicTemplate);
					return INVALIDPARAMETERS;
				}

				ManagedReference<SceneObject*> inventory = creature->getSlottedObject("inventory");

				if (inventory == nullptr || inventory->isContainerFullRecursive()) {
					creature->sendSystemMessage("Your inventory is full.");
					return INVALIDPARAMETERS;
				}

				const Vector<short>* slotTypes = draftTemplate->getIngredientSlotType();
				const Vector<String>* resTypes = draftTemplate->getResourceTypes();
				const Vector<int>* resQuantities = draftTemplate->getResourceQuantities();
				const Vector<String>* titleNames = draftTemplate->getIngredientTitleNames();

				if (slotTypes == nullptr || resTypes == nullptr || resQuantities == nullptr) {
					creature->sendSystemMessage("Schematic has no ingredient data.");
					return GENERALERROR;
				}

				int slotCount = slotTypes->size();
				int resourcesCreated = 0;
				int componentsCreated = 0;
				int slotsFailed = 0;

				ManagedReference<ResourceManager*> resourceManager = zoneServer->getResourceManager();
				ManagedReference<CraftingManager*> craftingManager = zoneServer->getCraftingManager();
				ManagedReference<LootManager*> lootManager = zoneServer->getLootManager();

				Zone* zone = creature->getZone();
				String zoneName = (zone != nullptr) ? zone->getZoneName() : "naboo";

				creature->sendSystemMessage("Creating ingredients for: " + schematicTemplate + " (x" + String::valueOf(quantity) + ")");

				for (int i = 0; i < slotCount; ++i) {
					short slotType = slotTypes->get(i);
					const String& resType = resTypes->get(i);
					int resQty = resQuantities->get(i) * quantity;
					String slotName = (titleNames != nullptr && i < titleNames->size()) ? titleNames->get(i) : "slot " + String::valueOf(i);

					if (slotType == 0) {
						// Resource slot — find an active spawn matching this resource class
						ManagedReference<ResourceSpawn*> spawn = resourceManager->getCurrentSpawn(resType, zoneName);

						if (spawn == nullptr) {
							// Try all zones to find any active spawn for this resource class
							for (int z = 0; z < zoneServer->getZoneCount() && spawn == nullptr; ++z) {
								Zone* searchZone = zoneServer->getZone(z);
								if (searchZone != nullptr) {
									spawn = resourceManager->getCurrentSpawn(resType, searchZone->getZoneName());
								}
							}
						}

						if (spawn != nullptr) {
							Locker slocker(spawn);

							Reference<ResourceContainer*> container = spawn->createResource(resQty);

							if (container != nullptr) {
								spawn->extractResource(zoneName, resQty);

								Locker rlocker(container);

								if (inventory->transferObject(container, -1, true)) {
									inventory->broadcastObject(container, true);
									resourcesCreated++;
									creature->sendSystemMessage("  [Resource] " + slotName + ": " + spawn->getName() + " x" + String::valueOf(resQty));
								} else {
									container->destroyObjectFromDatabase(true);
									slotsFailed++;
									creature->sendSystemMessage("  [FAILED] " + slotName + ": inventory transfer failed");
								}
							} else {
								slotsFailed++;
								creature->sendSystemMessage("  [FAILED] " + slotName + ": could not create resource container");
							}
						} else {
							slotsFailed++;
							creature->sendSystemMessage("  [FAILED] " + slotName + ": no active spawn for '" + resType + "'");
						}
					} else {
						// Component slot (type 1-4)
						// resType is a shared_ template path like "object/tangible/component/weapon/shared_blaster_power_handler.iff"
						// Convert to server template path by stripping "shared_" from filename
						String serverTemplatePath = resType;
						String filename;
						int lastSlash = serverTemplatePath.lastIndexOf('/');
						if (lastSlash != -1) {
							String dir = serverTemplatePath.subString(0, lastSlash + 1);
							filename = serverTemplatePath.subString(lastSlash + 1);
							if (filename.beginsWith("shared_")) {
								filename = filename.subString(7); // strip "shared_"
							}
							serverTemplatePath = dir + filename;
						} else {
							filename = serverTemplatePath;
						}

						// Derive loot item template name from filename (strip .iff extension)
						String lootItemName = filename;
						int dotPos = lootItemName.lastIndexOf('.');
						if (dotPos != -1) {
							lootItemName = lootItemName.subString(0, dotPos);
						}

						uint32 serverCRC = serverTemplatePath.hashCode();

						// Try loot system first — direct lookup by derived name
						Reference<const LootItemTemplate*> matchedLootItem = nullptr;

						if (lootManager != nullptr) {
							LootGroupMap* lootGroupMap = LootGroupMap::instance();
							matchedLootItem = lootGroupMap->getLootItemTemplate(lootItemName);
						}

						int componentCount = resQty;
						bool slotOk = true;

						// Create one component and set useCount = componentCount so all copies
						// share the same serial (factory crate behavior). This fixes the bug
						// where looping createLootObject() assigned a unique serial per call,
						// making N "identical" components unusable together in crafting assembly.
						ManagedReference<TangibleObject*> component = nullptr;

						if (matchedLootItem != nullptr && lootManager != nullptr) {
							// Create via loot system — gives proper crafting stats
							TransactionLog trx(TrxCode::ADMINCOMMAND, creature);
							trx.addState("commandType", "createingredients");
							component = lootManager->createLootObject(trx, matchedLootItem, 300, false);

							if (component != nullptr) {
								trx.commit(true);
							} else {
								trx.abort() << "createLootObject failed for " << serverTemplatePath;
							}
						}

						if (component == nullptr) {
							// Fallback — create plain object using server template path
							component = (zoneServer->createObject(serverCRC, 1)).castTo<TangibleObject*>();
						}

						if (component == nullptr) {
							slotsFailed++;
							creature->sendSystemMessage("  [FAILED] " + slotName + ": could not create '" + serverTemplatePath + "'");
							slotOk = false;
						} else {
							Locker clocker(component);

							if (matchedLootItem == nullptr) {
								// Only do manual setup for non-loot objects (loot system handles its own)
								component->createChildObjects();

								if (craftingManager != nullptr) {
									String serial = craftingManager->generateSerial();
									component->setSerialNumber(serial);
								}

								String crafterLabel = "Generated with Object Command";
								component->setCraftersName(crafterLabel);
							}

							// Stack all copies under one serial — same as /object createitem quantity behavior
							if (componentCount > 1) {
								component->setUseCount(componentCount);
							}

							if (inventory->transferObject(component, -1, true)) {
								inventory->broadcastObject(component, true);
								componentsCreated += componentCount;
							} else {
								component->destroyObjectFromDatabase(true);
								slotsFailed++;
								creature->sendSystemMessage("  [FAILED] " + slotName + ": inventory full");
								slotOk = false;
							}
						}

						if (slotOk) {
							String method = (matchedLootItem != nullptr) ? "Loot" : "Object";
							creature->sendSystemMessage("  [" + method + "] " + slotName + ": " + serverTemplatePath + " x" + String::valueOf(componentCount));
						}
					}
				}

				creature->sendSystemMessage("Done: " + String::valueOf(resourcesCreated) + " resources, " + String::valueOf(componentsCreated) + " components created. " + String::valueOf(slotsFailed) + " failed.");
				creature->info(true) << "/object createingredients " << schematicTemplate << " quantity=" << quantity << " resources=" << resourcesCreated << " components=" << componentsCreated << " failed=" << slotsFailed;
			}
			else if (commandType.beginsWith("createarealoot")) {
				String lootGroup;
				args.getStringToken(lootGroup);

				int range = 32;
				if (args.hasMoreTokens())
					range = args.getIntToken();

				if( range < 0 )
					range = 32;

				if( range > 128 )
					range = 128;

				int level = 1;
				if (args.hasMoreTokens())
					level = args.getIntToken();

				ManagedReference<LootManager*> lootManager = zoneServer->getLootManager();
				if (lootManager == nullptr)
					return INVALIDPARAMETERS;

				Zone* zone = creature->getZone();
				if (zone == nullptr)
					return GENERALERROR;

				// Find all objects in range
				SortedVector<TreeEntry*> closeObjects;
				CloseObjectsVector* closeObjectsVector = (CloseObjectsVector*) creature->getCloseObjects();
				if (closeObjectsVector == nullptr) {
					zone->getInRangeObjects(creature->getPositionX(), 0, creature->getPositionY(), range, &closeObjects, true);
				} else {
					closeObjectsVector->safeCopyTo(closeObjects);
				}

				// Award loot group to all players in range
				for (int i = 0; i < closeObjects.size(); i++) {
					SceneObject* targetObject = static_cast<SceneObject*>(closeObjects.get(i));

					if (targetObject->isPlayerCreature() && creature->isInRange(targetObject, range)) {

						CreatureObject* targetPlayer = cast<CreatureObject*>(targetObject);
						Locker tlock( targetPlayer, creature);

						ManagedReference<SceneObject*> inventory = targetPlayer->getSlottedObject("inventory");
						if (inventory != nullptr) {
							TransactionLog trx(creature, targetPlayer, nullptr, TrxCode::ADMINCOMMAND);
							trx.addState("commandType", commandType);
							if (lootManager->createLoot(trx, inventory, lootGroup, level) > 0) {
								creature->info(true) << "/object createlootarea " << lootGroup << " trxId: " << trx.getTrxID();
								trx.commit(true);
								targetPlayer->sendSystemMessage( "You have received a loot item!");
							} else {
								trx.abort() << "createLoot failed for lootGroup " << lootGroup << " level " << level;
							}
						}
					}
				}
			}
			else if (commandType.beginsWith("checklooted")) {
				ManagedReference<LootManager*> lootManager = zoneServer->getLootManager();
				if (lootManager == nullptr)
					return INVALIDPARAMETERS;

				creature->sendSystemMessage("Number of Legendaries Looted: " + String::valueOf(lootManager->getLegendaryLooted()));
				creature->sendSystemMessage("Number of Exceptionals Looted: " + String::valueOf(lootManager->getExceptionalLooted()));
				creature->sendSystemMessage("Number of Magical Looted: " + String::valueOf(lootManager->getYellowLooted()));

			}
			else if (commandType.beginsWith("characterbuilder")) {
				if (!ConfigManager::instance()->getBool("Core3.CharacterBuilderEnabled", true)) {
					creature->sendSystemMessage("characterbuilder is not enabled on this server.");
					return GENERALERROR;
				}

				String blueFrogTemplate = "object/tangible/terminal/terminal_character_builder.iff";
				ManagedReference<CharacterBuilderTerminal*> blueFrog = ( zoneServer->createObject(blueFrogTemplate.hashCode(), 0)).castTo<CharacterBuilderTerminal*>();

				if (blueFrog == nullptr)
					return GENERALERROR;

				Locker clocker(blueFrog, creature);

				float x = creature->getPositionX();
				float y = creature->getPositionY();
				float z = creature->getPositionZ();

				ManagedReference<SceneObject*> parent = creature->getParent().get();

				blueFrog->initializePosition(x, z, y);
				blueFrog->setDirection(creature->getDirectionW(), creature->getDirectionX(), creature->getDirectionY(), creature->getDirectionZ());

				if (parent != nullptr && parent->isCellObject())
					parent->transferObject(blueFrog, -1);
				else
					creature->getZone()->transferObject(blueFrog, -1, true);

				creature->info(true) << "/object characterbuilder " << " created oid: " << blueFrog->getObjectID() << " \"" << blueFrog->getDisplayedName() << "\" as " << creature->getWorldPosition() << " on " << creature->getZone()->getZoneName();
			}
			else if (commandType.beginsWith("createattachment")) {
				//Syntax exmaple /object createattachment ca mindblast_accuracy 25
				String attachmentType;
				String skillMod;
				String force;

				if (args.hasMoreTokens()) {
					args.getStringToken(attachmentType);
					attachmentType = attachmentType.toUpperCase();
				}
				else {
					creature->sendSystemMessage("You must specify AA or CA.");
					return INVALIDPARAMETERS;
				}
	
				if (attachmentType != "AA" && attachmentType != "CA"){
					creature->sendSystemMessage("You must specify AA or CA.");
					return INVALIDPARAMETERS;
				}

				if (args.hasMoreTokens()) {
					args.getStringToken(skillMod);
					skillMod = skillMod.toLowerCase();
				}

				ManagedReference<LootManager*> lootManager = zoneServer->getLootManager();	
				if (lootManager == nullptr)
					return INVALIDPARAMETERS;
	
				int skillModValue = 1;
				if (args.hasMoreTokens())
					skillModValue = args.getIntToken();

				else {
					creature->sendSystemMessage("You must specify a skillmod value");
					return INVALIDPARAMETERS;
				}

				if (args.hasMoreTokens()) {
					args.getStringToken(force);
					force = force.toLowerCase();
				}

				bool ignoreChecks = force.beginsWith("f");
				bool validAttachment = lootManager->isValidAttachment(attachmentType, skillMod);

				ManagedReference<SceneObject*> inventory = creature->getSlottedObject("inventory");
				if (inventory == nullptr || inventory->isContainerFullRecursive()) {
					creature->sendSystemMessage("Your inventory is full, so the item could not be created.");
					return INVALIDPARAMETERS;
				}
				
				Reference<const LootItemTemplate*> itemTemplate = nullptr;
				LootGroupMap* lootGroupMap = LootGroupMap::instance();
				if (attachmentType == "AA")
					itemTemplate = lootGroupMap->getLootItemTemplate("attachment_armor");
				else
					itemTemplate = lootGroupMap->getLootItemTemplate("attachment_clothing");
							
				ManagedReference<SceneObject*> SEA = lootManager->createLootAttachment(itemTemplate, skillMod, skillModValue, ignoreChecks); 

				if (SEA != nullptr){
					Locker locker(SEA);
					if (inventory->transferObject(SEA, -1, true, true)) { //Transfer tape to player inventory
						inventory->broadcastObject(SEA, true);
						if (validAttachment) {
							creature->sendSystemMessage("Created SEA:  " + SEA->getCustomObjectName().toString());
						}
						else {
							creature->sendSystemMessage("Created SEA:  " + skillMod + " " + String::valueOf(skillModValue));
						}
					} else {
						SEA->destroyObjectFromWorld(true);
						SEA->destroyObjectFromDatabase(true);
						creature->sendSystemMessage("Unable to place SEA in player's inventory!");
						return INVALIDPARAMETERS;
					}								
				}
				else
					creature->sendSystemMessage("Unable to create SEA with skillmod \"" + skillMod + "\"");
			}
			else if (commandType.beginsWith("destroyobject")) {

				uint64 oid = 0;

				if (target == 0) {
					oid = args.getLongToken();
					if (oid == 0) {
						creature->sendSystemMessage("INVALID OBJECT.  Please specify a valid object id");
					}
				}
				else {
					oid = target;
				}

				if (oid == 0 ) {
					creature->sendSystemMessage("Invalid Object.  Please specify a valid object id");
				}

				ManagedReference<SceneObject*> object = zoneServer->getObject(oid, false);

				if (object == nullptr) {
					creature->sendSystemMessage("Error getting object - nullptr " + String::valueOf(oid));
				} else {

                    String force;  //Let's allow for force deletion of structures with argument
                    if (args.hasMoreTokens()) {
                        args.getStringToken(force);
                        force = force.toUpperCase();
                    }

					if (object->isPlayerCreature() || object->isMissionTerminal()) {
						creature->sendSystemMessage("Do not use the /object destroyobject command on players or mission terminals.");
						return INVALIDPARAMETERS;
					}

					if ((object->isStructureObject() || object->isCreatureObject()) && force != "FORCE") {
						creature->sendSystemMessage("Do not use the /object destroyobject command on creatures or structures without FORCE argument. Usage: /object destroyobject <oid> force");
						return INVALIDPARAMETERS;
					}

					ManagedReference<CityRegion*>city = object->getCityRegion().get();

					Locker olocker(object, creature);

					if (city != nullptr && city->isCityDecoration(object)) {
						Locker clocker(city, creature);
						city->removeDecoration(object);
						creature->sendSystemMessage("@city/city:mt_removed"); // The object has been removed from the city.
					}

					String objectName = object->getDisplayedName();
					object->destroyObjectFromWorld(true);
					object->destroyObjectFromDatabase();
					creature->sendSystemMessage("The object, " + objectName + ", has been successfully destroyed from the database.");
				}
			}
			else if (commandType.beginsWith("destroypet")) {

				uint64 oid = 0;

				if (target == 0) {
					oid = args.getLongToken();
					if (oid == 0) {
						creature->sendSystemMessage("INVALID OBJECT.  Please specify a valid object id");
					}
				}
				else {
					oid = target;
				}

				if (oid == 0 ) {
					creature->sendSystemMessage("Invalid Object.  Please specify a valid object id");
				}

				ManagedReference<SceneObject*> object = zoneServer->getObject(oid, false);

				if (object == nullptr) {
					creature->sendSystemMessage("Error getting object - nullptr " + String::valueOf(oid));
				}
				else {

					if (!object->isCreatureObject() && !object->isPet()) {
						creature->sendSystemMessage("The /object destroypet command can only be used on pets.");
						return INVALIDPARAMETERS;
					}

					ManagedReference<AiAgent*> pet = object.castTo<AiAgent*>();

					if (pet == nullptr) {
						creature->sendSystemMessage("Error casting to AiAgent.");
						return GENERALERROR;
					}

					ManagedReference<CreatureObject*> owner = pet->getLinkedCreature().get();

					if (owner == nullptr) {
						creature->sendSystemMessage("Error getting owner creature object");
						return GENERALERROR;
					}

					ManagedReference<PetControlDevice*> petControlDevice = pet->getControlDevice().get().castTo<PetControlDevice*>();

					if (petControlDevice != nullptr) {
						creature->sendSystemMessage("Pet control device exists, delete pet from datapad.");
						return GENERALERROR;
					}

					//We appear to have an orphaned pet, let's remove from owner's active pets list

					ManagedReference<PlayerObject*> ownerGhost = owner->getPlayerObject();

					if (ownerGhost != nullptr && ownerGhost->hasActivePet(pet)) {
						Locker oLocker (ownerGhost, creature);
						ownerGhost->removeFromActivePets(pet);
					}

					//Now let's destroy the pet from world and database

					Locker plocker(object, creature);

					String objectName = object->getDisplayedName();
					object->destroyObjectFromWorld(true);
					object->destroyObjectFromDatabase();
					creature->sendSystemMessage("The pet, " + objectName + ", has been successfully destroyed from the database.");
				}
			}
			else if (commandType.beginsWith("setarmorpiercing")) {

				uint64 oid = 0;

				if (target == 0) {
					oid = args.getLongToken();
					if (oid == 0) {
						creature->sendSystemMessage("INVALID OBJECT.  Please specify a valid object id");
					}
				}
				else {
					oid = target;
				}

				ManagedReference<SceneObject*>object = zoneServer->getObject(oid, false);
				
				if (object == nullptr) {
					creature->sendSystemMessage("Error getting object - nullptr " + String::valueOf(oid));
					return GENERALERROR; 
				} 

				if (!object->isWeaponObject()) {
					creature->sendSystemMessage("/object setarmorpiercing can only be used on weapon objects.");
					return INVALIDPARAMETERS;
				}

				ManagedReference<WeaponObject*> weapon = cast<WeaponObject*>(object.get());

				if (weapon == nullptr) {
					creature->sendSystemMessage("/object setarmorpiercing can only be used on weapon objects.");
					return INVALIDPARAMETERS;
				}

				int oldAP = weapon->getArmorPiercing();
				int newAP = 0;

				if (args.hasMoreTokens())
					newAP = args.getIntToken();

				if (newAP < 0 || newAP > 3) {
					creature->sendSystemMessage("Valid values for armorpiercing are 0-3 (None, Light, Medium, Heavy).");
					return INVALIDPARAMETERS;
				}

				if (newAP == oldAP) {
					creature->sendSystemMessage("No change made to armor piercing of weapon.");
					return GENERALERROR;
				}
				else {
					Locker clocker(weapon, creature);
					weapon->setArmorPiercing(newAP);
					creature->sendSystemMessage("Armor piercing of " + weapon->getDisplayedName() + " changed from " + String::valueOf(oldAP) + " to " + String::valueOf(newAP));
				}				
			}
			else if (commandType.beginsWith("setconditiondamage")) {

				uint64 oid = 0;

				if (target == 0) {
					oid = args.getLongToken();
					if (oid == 0) {
						creature->sendSystemMessage("INVALID OBJECT.  Please specify a valid object id");
					}
				}
				else {
					oid = target;
				}

				ManagedReference<SceneObject*>object = zoneServer->getObject(oid, false);
				
				if (object == nullptr) {
					creature->sendSystemMessage("Error getting object - nullptr " + String::valueOf(oid));
					return GENERALERROR; 
				} 

				if (object->isCreatureObject() || object->isPlayerCreature()) {
					creature->sendSystemMessage("Do not use the /object setcondition command on creatures or players.");
					return INVALIDPARAMETERS;
				}

				int conditionDamage = 0;
				if (args.hasMoreTokens())
					conditionDamage = args.getIntToken();

				if (conditionDamage < 0)
					conditionDamage = 0;

				TangibleObject* tano = nullptr;
				if (object->isTangibleObject())
					tano = object->asTangibleObject();
				if (tano == nullptr) {
					creature->sendSystemMessage("Invalid object.  Please specify a tangible object");
					return INVALIDPARAMETERS;
				}
				else {
					int oldDamage = tano->getConditionDamage();
					int maxCondition = tano->getMaxCondition();
					if (conditionDamage > maxCondition)
						conditionDamage = maxCondition;
					Locker clocker(tano, creature);
					tano->setConditionDamage(conditionDamage,true);
					creature->sendSystemMessage("Condition damage of " + tano->getDisplayedName() + " changed from " + String::valueOf(oldDamage) + " to " + String::valueOf(conditionDamage));
				}
				
			}
			else if (commandType.beginsWith("setcraftersname")) {

				uint64 oid = 0;

				if (target == 0) {
					oid = args.getLongToken();
					if (oid == 0) {
						creature->sendSystemMessage("INVALID OBJECT.  Please specify a valid object id");
					}
				}
				else {
					oid = target;
				}

				ManagedReference<SceneObject*>object = zoneServer->getObject(oid, false);
				
				if (object == nullptr) {
					creature->sendSystemMessage("Error getting object - nullptr " + String::valueOf(oid));
					return GENERALERROR; 
				} 

				if (object->isCreatureObject() || object->isPlayerCreature()) {
					creature->sendSystemMessage("Do not use the /object setcraftersname command on creatures or players.");
					return INVALIDPARAMETERS;
				}

				String craftersName = "";
				if (args.hasMoreTokens())
					args.getStringToken(craftersName);

				if (craftersName == "")  {
					creature->sendSystemMessage("Please enter a valid player first name.");
					return INVALIDPARAMETERS;
				}

				craftersName = craftersName.trim();

				ManagedReference<CreatureObject*> crafterCreo = nullptr;
				try {
					crafterCreo = server->getPlayerManager()->getPlayer(craftersName);
				}
				catch (ArrayIndexOutOfBoundsException& ex) {
					creature->sendSystemMessage("Please enter a valid player first name as crafter's name.");
					return INVALIDPARAMETERS;
				}

				if (crafterCreo == nullptr) {
					creature->sendSystemMessage("Please enter a valid player first name as crafter's name.");
					return INVALIDPARAMETERS;
				}

				ManagedReference<TangibleObject*> tano = object->asTangibleObject();
				if (tano == nullptr) {
					creature->sendSystemMessage("Invalid object.  Please specify a tangible object");
					return INVALIDPARAMETERS;
				}
				else {
					Locker clocker(tano, creature);
					String oldName = tano->getCraftersName();
					tano->setCraftersName(craftersName);
					creature->sendSystemMessage("Crafter's name of " + tano->getDisplayedName() + " changed from " + oldName + " to " + craftersName);
				}
			}
			else if (commandType.beginsWith("setcrateitemcount")) {

				uint64 oid = 0;

				if (target == 0) {
					oid = args.getLongToken();
					if (oid == 0) {
						creature->sendSystemMessage("INVALID OBJECT.  Please specify a valid object id");
					}
				}
				else {
					oid = target;
				}

				ManagedReference<SceneObject*>object = zoneServer->getObject(oid, false);
				
				if (object == nullptr) {
					creature->sendSystemMessage("Error getting object - nullptr " + String::valueOf(oid));
					return GENERALERROR; 
				} 

				if (!object->isFactoryCrate()) {
					creature->sendSystemMessage("/object setcrateitemcount can only be used on factory crates.");
					return INVALIDPARAMETERS;
				}

				FactoryCrate* crate = cast<FactoryCrate*>( object.get());

				if (crate == nullptr) {
					creature->sendSystemMessage("/object setcrateitemcount can only be used on factory crates.");
					return INVALIDPARAMETERS;
				}

				int oldCount = crate->getUseCount();
				int newCount = 0;

				if (args.hasMoreTokens())
					newCount = args.getIntToken();

				if (newCount <= 0) {
					creature->sendSystemMessage("Factory crate item count must be an integer greater than 0.");
					return INVALIDPARAMETERS;
				}

				if (newCount == oldCount) {
					creature->sendSystemMessage("No change made to factory crate item count.");
					return GENERALERROR;
				}
				else {
					Locker clocker(crate, creature);
					crate->setUseCount(newCount, true);
					creature->sendSystemMessage("Item count of " + crate->getDisplayedName() + " changed from " + String::valueOf(oldCount) + " to " + String::valueOf(newCount));
				}				
			}

			else if (commandType.beginsWith("setmaxcondition")) {

				uint64 oid = 0;
				
				if (target == 0) {
					oid = args.getLongToken();
					if (oid == 0) {
						creature->sendSystemMessage("INVALID OBJECT.  Please specify a valid object id");
					}
				}
				else {
					oid = target;
				}

				ManagedReference<SceneObject*>object = zoneServer->getObject(oid, false);
				
				if (object == nullptr) {
					creature->sendSystemMessage("Error getting object - nullptr " + String::valueOf(oid));
					return GENERALERROR; 
				} 

				if (object->isCreatureObject() || object->isPlayerCreature()) {
					creature->sendSystemMessage("Do not use the /object setmaxcondition command on creatures or players.");
					return INVALIDPARAMETERS;
				}

				int maxCondition = 1000;
				if (args.hasMoreTokens())
					maxCondition = args.getIntToken();

				if (maxCondition < 0)
					maxCondition = 1000;

				TangibleObject* tano = nullptr;
				if (object->isTangibleObject())
					tano = object->asTangibleObject();
				if (tano == nullptr) {
					creature->sendSystemMessage("Invalid object.  Please specify a tangible object.");
					return INVALIDPARAMETERS;
				}
				else {
					int oldMaxCondition = tano->getMaxCondition();
					int oldConditionDamage = tano->getConditionDamage();
					Locker clocker(tano, creature);
					tano->setMaxCondition(maxCondition,true);
					creature->sendSystemMessage("Max condition of " + tano->getDisplayedName() + " changed from " + String::valueOf(oldMaxCondition) + " to " + String::valueOf(maxCondition));
					if (oldConditionDamage > maxCondition) {
						tano->setConditionDamage(maxCondition,true);
					}
				}
			}	
			else if (commandType.beginsWith("setmaxdamage")) {

				uint64 oid = 0;

				if (target == 0) {
					oid = args.getLongToken();
					if (oid == 0) {
						creature->sendSystemMessage("INVALID OBJECT.  Please specify a valid object id");
					}
				}
				else {
					oid = target;
				}

				ManagedReference<SceneObject*>object = zoneServer->getObject(oid, false);
				
				if (object == nullptr) {
					creature->sendSystemMessage("Error getting object - nullptr " + String::valueOf(oid));
					return GENERALERROR; 
				} 

				if (!object->isWeaponObject()) {
					creature->sendSystemMessage("/object setmaxdamage can only be used on weapon objects.");
					return INVALIDPARAMETERS;
				}

				ManagedReference<WeaponObject*> weapon = cast<WeaponObject*>(object.get());

				if (weapon == nullptr) {
					creature->sendSystemMessage("/object setmaxdamage can only be used on weapon objects.");
					return INVALIDPARAMETERS;
				}

				if (weapon->hasPowerup()) {
					creature->sendSystemMessage("Remove powerup before using /object setmaxdamage.");
					return INVALIDPARAMETERS;
				}

				int oldMaxDamage = weapon->getMaxDamage();
				int newMaxDamage = 0;

				if (args.hasMoreTokens())
					newMaxDamage = args.getIntToken();

				if (newMaxDamage < 0) {
					creature->sendSystemMessage("Maximum damage must be an integer greater than 0.");
					return INVALIDPARAMETERS;
				}

				if (newMaxDamage == oldMaxDamage) {
					creature->sendSystemMessage("No change made to maximum damage of weapon.");
					return GENERALERROR;
				}
				else {
					Locker clocker(weapon, creature);
					weapon->setMaxDamage(newMaxDamage);
					creature->sendSystemMessage("Maximum damage of " + weapon->getDisplayedName() + " changed from " + String::valueOf(oldMaxDamage) + " to " + String::valueOf(newMaxDamage));
				}				
			}
			else if (commandType.beginsWith("setmindamage")) {

				uint64 oid = 0;

				if (target == 0) {
					oid = args.getLongToken();
					if (oid == 0) {
						creature->sendSystemMessage("INVALID OBJECT.  Please specify a valid object id");
					}
				}
				else {
					oid = target;
				}

				ManagedReference<SceneObject*>object = zoneServer->getObject(oid, false);
				
				if (object == nullptr) {
					creature->sendSystemMessage("Error getting object - nullptr " + String::valueOf(oid));
					return GENERALERROR; 
				} 

				if (!object->isWeaponObject()) {
					creature->sendSystemMessage("/object setmindamage can only be used on weapon objects.");
					return INVALIDPARAMETERS;
				}

				ManagedReference<WeaponObject*> weapon = cast<WeaponObject*>(object.get());

				if (weapon == nullptr) {
					creature->sendSystemMessage("/object setmindamage can only be used on weapon objects.");
					return INVALIDPARAMETERS;
				}

				if (weapon->hasPowerup()) {
					creature->sendSystemMessage("Remove powerup before using /object setmindamage.");
					return INVALIDPARAMETERS;
				}

				int oldMinDamage = weapon->getMinDamage();
				int newMinDamage = 0;

				if (args.hasMoreTokens())
					newMinDamage = args.getIntToken();

				if (newMinDamage < 0) {
					creature->sendSystemMessage("Minimum damage must be an integer greater than 0.");
					return INVALIDPARAMETERS;
				}

				if (newMinDamage == oldMinDamage) {
					creature->sendSystemMessage("No change made to minimum damage of weapon.");
					return GENERALERROR;
				}
				else {
					Locker clocker(weapon, creature);
					weapon->setMinDamage(newMinDamage);
					creature->sendSystemMessage("Minimum damage of " + weapon->getDisplayedName() + " changed from " + String::valueOf(oldMinDamage) + " to " + String::valueOf(newMinDamage));
				}				
			}
			else if (commandType.beginsWith("setforcecost")) {

				uint64 oid = 0;

				if (target == 0) {
					oid = args.getLongToken();
					if (oid == 0) {
						creature->sendSystemMessage("INVALID OBJECT.  Please specify a valid object id");
					}
				}
				else {
					oid = target;
				}

				ManagedReference<SceneObject*>object = zoneServer->getObject(oid, false);
				
				if (object == nullptr) {
					creature->sendSystemMessage("Error getting object - nullptr " + String::valueOf(oid));
					return GENERALERROR; 
				} 

				if (!object->isWeaponObject()) {
					creature->sendSystemMessage("/object forcecost can only be used on weapon objects.");
					return INVALIDPARAMETERS;
				}

				ManagedReference<WeaponObject*> weapon = cast<WeaponObject*>(object.get());

				if (weapon == nullptr) {
					creature->sendSystemMessage("/object forcecost can only be used on weapon objects.");
					return INVALIDPARAMETERS;
				}

				if (!weapon->isJediWeapon()) {
					creature->sendSystemMessage("/object forcecost can only be used on Jedi weapons.");
					return INVALIDPARAMETERS;
				}

				float oldForceCost = weapon->getForceCost();
				float newForceCost = 0;

				if (args.hasMoreTokens())
					newForceCost = args.getFloatToken();

				if (newForceCost == oldForceCost) {
					creature->sendSystemMessage("No change made to force cost of weapon.");
					return GENERALERROR;
				}
				else {
					Locker clocker(weapon, creature);
					weapon->setForceCost(newForceCost);
					creature->sendSystemMessage("Force cost of " + weapon->getDisplayedName() + " changed from " + String::valueOf(oldForceCost) + " to " + String::valueOf(newForceCost));
				}				
			}
			else if (commandType.beginsWith("setcraftersid")) {

				uint64 oid = 0;

				if (target == 0) {
					oid = args.getLongToken();
					if (oid == 0) {
						creature->sendSystemMessage("INVALID OBJECT.  Please specify a valid object id");
					}
				}
				else {
					oid = target;
				}

				ManagedReference<SceneObject*> object = zoneServer->getObject(oid, false);
				
				if (object == nullptr) {
					creature->sendSystemMessage("Error getting object - nullptr " + String::valueOf(oid));
					return GENERALERROR; 
				} 

				if (object->isCreatureObject() || object->isPlayerCreature()) {
					creature->sendSystemMessage("Do not use the /object setcraftersid command on creatures or players.");
					return INVALIDPARAMETERS;
				}

				uint64 newCraftersID = 0;
  				if (args.hasMoreTokens())
					newCraftersID = args.getLongToken();
                else {
					creature->sendSystemMessage("No crafter ID specified");
					return INVALIDPARAMETERS;
                }

                auto crafterSceno = zoneServer->getObject(newCraftersID, false);

                if (crafterSceno == nullptr) {
					creature->sendSystemMessage("Invalid crafter object ID specified.");
					return INVALIDPARAMETERS;
                }

				ManagedReference<TangibleObject*> tano = object->asTangibleObject();
				if (tano == nullptr) {
					creature->sendSystemMessage("Invalid object.  Please specify a valid tangible object");
					return INVALIDPARAMETERS;
				}
				else {
					Locker clocker(tano, creature);
					uint64 oldID = tano->getCraftersID();
					tano->setCraftersID(newCraftersID);
					creature->sendSystemMessage("Crafter's ID of " + tano->getDisplayedName() + " changed from " + String::valueOf(oldID) + " to " + String::valueOf(newCraftersID));
				}
			}			

			else if (commandType.beginsWith("despawnaiagent")) {

				uint64 oid = 0;

				if (target == 0) {
					oid = args.getLongToken();
					if (oid == 0) {
						creature->sendSystemMessage("INVALID OBJECT.  Please specify a valid object id");
					}
				}
				else {
					oid = target;
				}

				if (oid == 0 ) {
					creature->sendSystemMessage("Invalid Object.  Please specify a valid object id");
				}

				ManagedReference<SceneObject*> object = zoneServer->getObject(oid, false);

				if (object == nullptr) {
					creature->sendSystemMessage("Error getting object - nullptr " + String::valueOf(oid));
				}
				else {

					if (!object->isAiAgent() || object->isPet()) {
						creature->sendSystemMessage("The /object despawnaiagent command can only be used on non-pet AI agents.");
						return INVALIDPARAMETERS;
					}

					ManagedReference<AiAgent*> agent = object.castTo<AiAgent*>();

					if (agent == nullptr) {
						creature->sendSystemMessage("Error casting to AiAgent.");
						return GENERALERROR;
					}

					auto zone = agent->getZone();
					if (zone == nullptr) {
						creature->sendSystemMessage("Agent zone = nullptr.");
						return GENERALERROR;
					}

					//Now let's destroy the agent from the world
					creature->sendSystemMessage("The AI agent, " + agent->getDisplayedName() + ", has been successfully destroyed from the world and will not respawn");
					
					Locker plocker(agent, creature);
					agent->destroyObjectFromWorld(true);
					agent->notifyDespawn(zone, true);
				}
			}

		else if (commandType.beginsWith("destroystructureforce")) {

				ManagedReference<SceneObject*> parent = creature->getRootParent();

				if (parent == nullptr) {

					SortedVector<ManagedReference<TreeEntry*> > nearbyObjects;
					CloseObjectsVector* closeObjectsVector = (CloseObjectsVector*) creature->getCloseObjects();
					if (closeObjectsVector == nullptr) {
						creature->sendSystemMessage("You must be in building or within 5 meters of the center to use this command.");
						return GENERALERROR;
					}
					else {
						closeObjectsVector->safeCopyReceiversTo(nearbyObjects, CloseObjectsVector::STRUCTURETYPE);
					}	

					for(int i = 0; i < nearbyObjects.size(); ++i) {
						ManagedReference<SceneObject*> scno = cast<SceneObject*>(nearbyObjects.get(i).get());
						if (scno != nullptr && scno->isBuildingObject()) {
							ManagedReference<BuildingObject*> building = cast<BuildingObject*>(scno.get());
							if (building != nullptr && building->isInRange(creature, 5.f)) {
								parent = building;
								break;
							}
						}
					}				
				}
				else if (!parent->isBuildingObject()) {
					creature->sendSystemMessage("Invalid building type for destruction.");
					return GENERALERROR;
				}

				ManagedReference<BuildingObject*> building = cast<BuildingObject*>(parent.get());
				if (building == nullptr || building->getOwnerObjectID() == 0) {
					creature->sendSystemMessage("Invalid building type for destruction.");
					return GENERALERROR;
				}

				if (building->isGCWBase()) {
					auto zone = creature->getZone();
					if (zone != nullptr) {
						GCWManager* gcwMgr = zone->getGCWManager();
						if (gcwMgr != nullptr) {
							gcwMgr->doBaseDestruction(building);
						}
					}
				}
				else {				
					Reference<DestroyStructureTask*> task = new DestroyStructureTask(building, false, true);
					task->execute();
				}
			}
			else if (commandType.beginsWith("setattackspeed")) {

				uint64 oid = 0;

				if (target == 0) {
					oid = args.getLongToken();
					if (oid == 0) {
						creature->sendSystemMessage("INVALID OBJECT.  Please specify a valid object id");
					}
				}
				else {
					oid = target;
				}

				ManagedReference<SceneObject*>object = zoneServer->getObject(oid, false);
				
				if (object == nullptr) {
					creature->sendSystemMessage("Error getting object - nullptr " + String::valueOf(oid));
					return GENERALERROR; 
				} 

				if (!object->isWeaponObject()) {
					creature->sendSystemMessage("/object setattackspeed can only be used on weapon objects.");
					return INVALIDPARAMETERS;
				}

				ManagedReference<WeaponObject*> weapon = cast<WeaponObject*>(object.get());

				if (weapon == nullptr) {
					creature->sendSystemMessage("/object setattackspeed can only be used on weapon objects.");
					return INVALIDPARAMETERS;
				}

				if (weapon->hasPowerup()) {
					creature->sendSystemMessage("Remove powerup before using /object setattackspeed.");
					return INVALIDPARAMETERS;
				}

				float oldAttackSpeed = weapon->getAttackSpeed(false);  //Don't include powerup
				float newAttackSpeed = 0;

				if (args.hasMoreTokens())
					newAttackSpeed = args.getFloatToken();

				if (newAttackSpeed < 0.1) {
					creature->sendSystemMessage("Attack speed must be a float value greater than 0.1.");
					return INVALIDPARAMETERS;
				}

				if (newAttackSpeed == oldAttackSpeed) {
					creature->sendSystemMessage("No change made to attack speed of weapon.");
					return GENERALERROR;
				}
				else {
					Locker clocker(weapon, creature);
					weapon->setAttackSpeed(newAttackSpeed);
					creature->sendSystemMessage("Attack speed of " + weapon->getDisplayedName() + " changed from " + String::valueOf(oldAttackSpeed) + " to " + String::valueOf(newAttackSpeed));
				}				
			}			
			else {  // Didn't match any known commands, give them the options
				creature->sendSystemMessage("SYNTAX: /object characterbuilder");
				creature->sendSystemMessage("SYNTAX: /object createitem <objectTemplatePath> [<quantity>]");
				creature->sendSystemMessage("SYNTAX: /object createloot <loottemplate> [<level>]");
				creature->sendSystemMessage("SYNTAX: /object createarealoot <loottemplate> [<range>] [<level>]");
				creature->sendSystemMessage("SYNTAX: /object createattachment <AA/CA> <skillMod> <skillModValue> [force]");
				creature->sendSystemMessage("SYNTAX: /object createresource <resourceName> [<quantity>]");
				creature->sendSystemMessage("SYNTAX: /object createingredients <schematicTemplate> [<quantity>]");
				creature->sendSystemMessage("SYNTAX: /object checklooted");
				creature->sendSystemMessage("SYNTAX: /object despawnaiagent <objectID>");
				creature->sendSystemMessage("SYNTAX: /object destroyobject [<objectID>]");
				creature->sendSystemMessage("SYNTAX: /object destroypet <objectID>");
				creature->sendSystemMessage("SYNTAX: /object destroystructureforce");
				creature->sendSystemMessage("SYNTAX: /object setarmorpiercing [<objectID>] [<level>]");
				creature->sendSystemMessage("SYNTAX: /object setattackspeed [<objectID>] <speed>");
				creature->sendSystemMessage("SYNTAX: /object setconditiondamage [<objectID>] <conditionDamage>");
                creature->sendSystemMessage("SYNTAX: /object setcraftersid [<objectID>] <crafterID>");
				creature->sendSystemMessage("SYNTAX: /object setcraftersname [<objectID>] <craftersName>");
				creature->sendSystemMessage("SYNTAX: /object setcrateitemcount [<objectID>] <item count>");
				creature->sendSystemMessage("SYNTAX: /object setmaxcondition [<objectID>] <maxCondition>");
				creature->sendSystemMessage("SYNTAX: /object setmaxdamage [<objectID>] <maxDamage>");
				creature->sendSystemMessage("SYNTAX: /object setmindamage [<objectID>] <minDamage>");
				creature->sendSystemMessage("SYNTAX: /object setforcecost [<objectID>] <forceCost>");
				return INVALIDPARAMETERS;				
			}

		} catch (Exception& e) {
			creature->sendSystemMessage("SYNTAX: /object characterbuilder");
			creature->sendSystemMessage("SYNTAX: /object createitem <objectTemplatePath> [<quantity>]");
			creature->sendSystemMessage("SYNTAX: /object createloot <loottemplate> [<level>]");
			creature->sendSystemMessage("SYNTAX: /object createarealoot <loottemplate> [<range>] [<level>]");
			creature->sendSystemMessage("SYNTAX: /object createattachment <AA/CA> <skillMod> <skillModValue> [force]");
			creature->sendSystemMessage("SYNTAX: /object createresource <resourceName> [<quantity>]");
			creature->sendSystemMessage("SYNTAX: /object createingredients <schematicTemplate> [<quantity>]");
			creature->sendSystemMessage("SYNTAX: /object checklooted");
			creature->sendSystemMessage("SYNTAX: /object despawnaiagent <objectID>");
			creature->sendSystemMessage("SYNTAX: /object destroyobject [<objectID>]");
			creature->sendSystemMessage("SYNTAX: /object destroypet <objectID>");
			creature->sendSystemMessage("SYNTAX: /object destroystructureforce");
			creature->sendSystemMessage("SYNTAX: /object setarmorpiercing [<objectID>] [<level>]");
			creature->sendSystemMessage("SYNTAX: /object setattackspeed [<objectID>] <speed>");
			creature->sendSystemMessage("SYNTAX: /object setconditiondamage [<objectID>] <conditionDamage>");
            creature->sendSystemMessage("SYNTAX: /object setcraftersid [<objectID>] <crafterID>");
			creature->sendSystemMessage("SYNTAX: /object setcraftersname [<objectID>] <craftersName>");
			creature->sendSystemMessage("SYNTAX: /object setcrateitemcount [<objectID>] <item count>");
			creature->sendSystemMessage("SYNTAX: /object setmaxcondition [<objectID>] <maxCondition>");
			creature->sendSystemMessage("SYNTAX: /object setmaxdamage [<objectID>] <maxDamage>");
			creature->sendSystemMessage("SYNTAX: /object setmindamage [<objectID>] <minDamage>");
			creature->sendSystemMessage("SYNTAX: /object setforcecost [<objectID>] <forceCost>");
			return INVALIDPARAMETERS;                 
		}

		return SUCCESS;
	}

};

#endif //OBJECTCOMMAND_H_
