/*
 * TangibleObjectMenuComponent.cpp
 *
 *  Created on: 26/05/2011
 *      Author: victor
 */

#include "TangibleObjectMenuComponent.h"
#include "server/zone/objects/player/sessions/SlicingSession.h"
#include "server/zone/packets/object/ObjectMenuResponse.h"
#include "server/zone/objects/player/PlayerObject.h"
// Infinity: Custom includes for SEA extraction
#include "server/zone/managers/loot/LootManager.h"
#include "server/zone/managers/loot/LootGroupMap.h"
#include "server/zone/objects/tangible/wearables/WearableObject.h"
#include "server/zone/objects/tangible/attachment/Attachment.h"
#include "templates/LootItemTemplate.h"

void TangibleObjectMenuComponent::fillObjectMenuResponse(SceneObject* sceneObject, ObjectMenuResponse* menuResponse, CreatureObject* player) const {
	ObjectMenuComponent::fillObjectMenuResponse(sceneObject, menuResponse, player);

	uint32 gameObjectType = sceneObject->getGameObjectType();

	if (!sceneObject->isTangibleObject())
		return;

	TangibleObject* tano = cast<TangibleObject*>( sceneObject);

	// Infinity: Stability check
	if (tano == nullptr || player == nullptr)
		return;

	// Figure out what the object is and if its able to be Sliced.
	if (tano->isSliceable() && !tano->isSecurityTerminal()) { // Check to see if the player has the correct skill level

		bool hasSkill = true;
		ManagedReference<SceneObject*> inventory = player->getSlottedObject("inventory");

		if (gameObjectType == SceneObjectType::PLAYERLOOTCRATE && player->getSkillMod("slicing") < 5)  // Infinity: Skillmod check instead of skill box
			hasSkill = false;
		else if (sceneObject->isContainerObject())
			hasSkill = false; // Let the container handle our slice menu
		else if (sceneObject->isMissionTerminal() && player->getSkillMod("slicing") < 15)    // Infinity: Skillmod check instead of skill box
			hasSkill = false;
		else if (sceneObject->isWeaponObject() && (!inventory->hasObjectInContainer(sceneObject->getObjectID()) || player->getSkillMod("slicing") < 25))    // Infinity: Skillmod check instead of skill box
			hasSkill = false;
		else if (sceneObject->isArmorObject() && (!inventory->hasObjectInContainer(sceneObject->getObjectID()) || player->getSkillMod("slicing") < 35))    // Infinity: Skillmod check instead of skill box
			hasSkill = false;

		if (hasSkill)
			menuResponse->addRadialMenuItem(69, 3, "@slicing/slicing:slice"); // Slice
	}

	if (player->getPlayerObject() != nullptr && player->getPlayerObject()->isPrivileged()) {
		/// Viewing components used to craft item, for admins
		ManagedReference<SceneObject*> container = tano->getSlottedObject("crafted_components");

		if (container != nullptr && container->getContainerObjectsSize() > 0) {
			SceneObject* satchel = container->getContainerObject(0);

			if (satchel != nullptr && satchel->getContainerObjectsSize() > 0) {
				menuResponse->addRadialMenuItem(79, 3, "@ui_radial:ship_manage_components"); // View Components
			}
		}
	}

	// Infinity: SEA Removal Tool support
	if (tano->isWearableObject()) {
		WearableObject* wearable = cast<WearableObject*>(tano);
		if (wearable != nullptr && hasSeaRemovalTool(player)) {
			VectorMap<String, int>* mods = wearable->getWearableSkillMods();
			if (mods->size() > 0) {
				menuResponse->addRadialMenuItem(89, 3, "Extract SEAs");
			}
		}
	}

	ManagedReference<SceneObject*> parent = tano->getParent().get();
	if (parent != nullptr && parent->getGameObjectType() == SceneObjectType::STATICLOOTCONTAINER) {
		menuResponse->addRadialMenuItem(10, 3, "@ui_radial:item_pickup"); //Pick up
	}
}

int TangibleObjectMenuComponent::handleObjectMenuSelect(SceneObject* sceneObject, CreatureObject* player, byte selectedID) const {
	if (!sceneObject->isTangibleObject())
		return 0;

	TangibleObject* tano = cast<TangibleObject*>( sceneObject);

	// Infinity: Stability check
	if (tano == nullptr)
		return 0;

	if (selectedID == 69 && player->getSkillMod("slicing") >= 5) { // Slice [PlayerLootCrate]  // Infinity: Skillmod check instead of skill box
		if (player->containsActiveSession(SessionFacadeType::SLICING)) {
			player->sendSystemMessage("@slicing/slicing:already_slicing");
			return 0;
		}

		//Create Session
		ManagedReference<SlicingSession*> session = new SlicingSession(player);
		session->initalizeSlicingMenu(player, tano);

		return 0;
	} else if (selectedID == 79) { // See components (admin)
		if(player->getPlayerObject() != nullptr && player->getPlayerObject()->isPrivileged()) {

			SceneObject* container = tano->getSlottedObject("crafted_components");
			if(container != nullptr) {

				if(container->getContainerObjectsSize() > 0) {

					SceneObject* satchel = container->getContainerObject(0);

					if(satchel != nullptr) {

						satchel->sendWithoutContainerObjectsTo(player);
						satchel->openContainerTo(player);

					} else {
						player->sendSystemMessage("There is no satchel this container");
					}
				} else {
					player->sendSystemMessage("There are no items in this container");
				}
			} else {
				player->sendSystemMessage("There is no component container in this object");
			}
		}

		return 0;
	} else if (selectedID == 89) { // Infinity: Remove SEA Mods from wearable
		WearableObject* wearable = cast<WearableObject*>(tano);

		if (wearable == nullptr) {
			return 0;
		}

		ManagedReference<SceneObject*> sea = nullptr;
		bool convertedMods = false;
		ManagedReference<SceneObject*> inventory = player->getSlottedObject("inventory");

		if (inventory == nullptr) {
			return 0;
		}

		if (!hasSeaRemovalTool(player)) {
			player->sendSystemMessage("This action requires a specialized SEA Removal tool uncrated in inventory.");
			return 0;
		}

		if (wearable->isWearableObject() || wearable->isArmorObject()) {
			if (wearable->isEquipped()) {
				player->sendSystemMessage("You must first un-equip the item to remove SEAs.");
				return 0;
			}

			VectorMap<String, int>* mods = wearable->getWearableSkillMods();

			if (mods->size() > 0) {
				auto zoneServer = player->getZoneServer();
				if (zoneServer == nullptr) {
					return 0;
				}

				auto lootManager = zoneServer->getLootManager();
				if (lootManager == nullptr) {
					return 0;
				}

				LootGroupMap* lootGroupMap = LootGroupMap::instance();
				if (lootGroupMap == nullptr) {
					error("Invalid loot template");
					return 0;
				}

				Reference<const LootItemTemplate*> itemTemplate = nullptr;
				String objectTemplate = wearable->getObjectTemplate()->getFullTemplateString();

				if (wearable->isArmorObject() ||
					objectTemplate == "object/tangible/wearables/armor/padded/armor_padded_s01_belt.iff" ||
					objectTemplate == "object/tangible/wearables/armor/zam/armor_zam_wesell_belt.iff") {
					itemTemplate = lootGroupMap->getLootItemTemplate("attachment_armor");
				} else {
					itemTemplate = lootGroupMap->getLootItemTemplate("attachment_clothing");
				}

				int roll = System::random(100);
				int extractionFailed = roll < 10;

				if (!extractionFailed) {
					for (int i = 0; i < mods->size(); i++) {
						String modKey = mods->elementAt(i).getKey();

						if (modKey == "onehandlightsaber_accuracy" || modKey == "twohandlightsaber_accuracy" || modKey == "polearmlightsaber_accuracy") {
							modKey = "lightsaber_accuracy";
							convertedMods = true;
						}

						if (modKey == "onehandlightsaber_speed" || modKey == "twohandlightsaber_speed" || modKey == "polearmlightsaber_speed") {
							modKey = "lightsaber_speed";
							convertedMods = true;
						}

						if (modKey == "forceintimidate_accuracy") {
							modKey = "intimidate";
							convertedMods = true;
						}

						// Infinity: Use LootManager to create proper attachment with correct naming
						sea = lootManager->createLootAttachment(itemTemplate, modKey, mods->elementAt(i).getValue());

						if (sea != nullptr) {
							Attachment* attachment = cast<Attachment*>(sea.get());

							if (attachment != nullptr) {
								Locker objLocker(attachment);
								if (inventory->transferObject(sea, -1, true, true)) {
									inventory->broadcastObject(sea, true);
								} else {
									sea->destroyObjectFromDatabase(true);
									error("Unable to place SEA in player's inventory!");
									return false;
								}
								objLocker.release();
							}
						}
					}
				}

				if (hasSeaRemovalTool(player, true)) {
					player->sendSystemMessage("Your SEA Removal Tool has been consumed in the process.");
				}

				Locker wlocker(wearable);
				wearable->destroyObjectFromWorld(true);
				wearable->destroyObjectFromDatabase(true);
				wlocker.release();

				if (extractionFailed) {
					player->sendSystemMessage("The extraction process failed and the item was destroyed.");
				} else {
					player->sendSystemMessage("The extraction process was successful and the SEAs were removed.");
					if (convertedMods) {
						player->sendSystemMessage("Old skill mods were converted to new skill mods.");
					}
				}
			}
		}

		return 0;
	} else {
		return ObjectMenuComponent::handleObjectMenuSelect(sceneObject, player, selectedID);
	}
}

bool TangibleObjectMenuComponent::hasSeaRemovalTool(CreatureObject* player, bool removeItem) const {
	if (player == nullptr) {
		return false;
	}

	ManagedReference<SceneObject*> inventory = player->getSlottedObject("inventory");

	if (inventory == nullptr) {
		return false;
	}

	uint32 toolCRC = STRING_HASHCODE("object/tangible/item/clothing_sea_removal_tool.iff");

	for (int i = 0; i < inventory->getContainerObjectsSize(); ++i) {
		ManagedReference<SceneObject*> sceno = inventory->getContainerObject(i);

		if (sceno == nullptr) {
			continue;
		}

		if (sceno->getServerObjectCRC() == toolCRC) {
			if (removeItem) {
				Locker locker(sceno);
				sceno->destroyObjectFromWorld(true);
				sceno->destroyObjectFromDatabase(true);
				locker.release();
			}
			return true;
		}
	}

	return false;
}

