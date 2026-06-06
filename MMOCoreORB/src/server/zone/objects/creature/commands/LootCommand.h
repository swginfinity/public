/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef LOOTCOMMAND_H_
#define LOOTCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/managers/player/PlayerManager.h"
#include "server/zone/managers/group/GroupLootTask.h"
#include "server/zone/objects/transaction/TransactionLog.h"
#include "server/zone/CloseObjectsVector.h"

class LootCommand : public QueueCommand {

public:
	enum {
		NOPICKUPITEMS = 0,
		ITEMFOROTHER = 1,
		PICKEDANDREMAINING = 2,
		PICKEDANDEMPTY = 3
	};

	LootCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!checkStateMask(creature)) {
			return INVALIDSTATE;
		}

		if (!checkInvalidLocomotions(creature)) {
			return INVALIDLOCOMOTION;
		}

		if (creature == nullptr || creature->isDead()) {
			return GENERALERROR;
		}

		auto zoneServer = server->getZoneServer();
		if (zoneServer == nullptr) {
			return GENERALERROR;
		}

		auto playerManager = zoneServer->getPlayerManager();
		if (playerManager == nullptr) {
			return GENERALERROR;
		}

		bool lootArea = arguments.toString().toLowerCase().beginsWith("area");
		float range = 16.0f;

		if (!lootArea) { //Infinity:  This is standard loot method

			ManagedReference<SceneObject*> targetObject = zoneServer->getObject(target);
			if (targetObject == nullptr || !targetObject->isAiAgent()) {
				return INVALIDTARGET;
			}

			ManagedReference<AiAgent*> agent = targetObject.castTo<AiAgent*>();
			if (agent == nullptr) {
				return INVALIDTARGET;
			}

			if (!agent->isDead()) {
				return GENERALERROR;
			}

			ManagedReference<CreatureObject*> targetCreo = targetObject.castTo<CreatureObject*>();
			if (targetCreo == nullptr) {
				return GENERALERROR;
			}

			ManagedReference<Creature*> targetCreature = targetObject.castTo<Creature*>();
			if (targetCreature != nullptr) {
				if (targetCreature->getDnaState() == CreatureManager::DNADEATH) {
					creature->sendSystemMessage("You cannot loot a creature that has been killed by DNA sampling.");
					return GENERALERROR;
				}
			}

			float adjustedRange = range + Math::min(targetCreo->getTemplateRadius() / 4.0f, 8.0f);

			if (!checkDistance(agent, creature, adjustedRange)) { //Check range plus adjustment
				creature->sendSystemMessage("@error_message:target_out_of_range"); //"Your target is out of range for this action."
				return GENERALERROR;
			}

			Locker locker(agent, creature);

			// Get the corpse's inventory.
			ManagedReference<SceneObject*> lootContainer = agent->getSlottedObject("inventory");
			if (lootContainer == nullptr) {
				return GENERALERROR;
			}

			const ContainerPermissions* permissions = lootContainer->getContainerPermissions();
			if (permissions == nullptr)
				return GENERALERROR;

			// Determine the loot rights.
			uint64 ownerID = permissions->getOwnerID();

			bool lootAll = arguments.toString().beginsWith("all");
			bool looterIsOwner = (ownerID == creature->getObjectID());
			bool groupIsOwner = (ownerID == creature->getGroupID());

			// Allow player to loot the corpse if they own it.
			if (looterIsOwner) {
				if (lootAll) {
					playerManager->lootAll(creature, agent);
				}
				else {
					//Check if the corpse's inventory contains any items.
					if (lootContainer->getContainerObjectsSize() < 1) {
						creature->sendSystemMessage("@error_message:corpse_empty"); //"You find nothing else of value on the selected corpse."
						playerManager->rescheduleCorpseDestruction(creature, agent);
					}
					else {
						agent->notifyObservers(ObserverEventType::LOOTCREATURE, creature, 0);
						lootContainer->openContainerTo(creature);
					}
				}

				return SUCCESS;
			}

			// If player and their group don't own the corpse, pick up any owned items left on corpse due to full inventory, then fail.
			if (!groupIsOwner) {
				int pickupResult = pickupOwnedItems(agent, creature, lootContainer);
				if (pickupResult < 2) { //Player didn't pickup an item nor is one available for them.
					StringIdChatParameter noPermission("error_message","no_corpse_permission"); //"You do not have permission to access this corpse."
					creature->sendSystemMessage(noPermission);
					return GENERALERROR;
				}
				else if (pickupResult == PICKEDANDEMPTY) {
					agent->notifyObservers(ObserverEventType::LOOTCREATURE, creature, 0);
					playerManager->rescheduleCorpseDestruction(creature, agent);
					return SUCCESS;
				}

				return SUCCESS;
			}

			// If looter's group is the owner, attempt to pick up any owned items, then process group loot rule.
			int pickupResult = pickupOwnedItems(agent, creature, lootContainer);

			switch (pickupResult) {
			case NOPICKUPITEMS: //No items available for anyone to pickup.
				break;
			case ITEMFOROTHER: //No items available for looter to pickup, but one is available for someone else.
				agent->notifyObservers(ObserverEventType::LOOTCREATURE, creature, 0);
				lootContainer->openContainerTo(creature);
				return SUCCESS;
			case PICKEDANDREMAINING: //An item was available for the looter, there are items remaining.
				agent->notifyObservers(ObserverEventType::LOOTCREATURE, creature, 0);
				return SUCCESS;
			case PICKEDANDEMPTY: //An item was available for the looter, there are NO items remaining.
				agent->notifyObservers(ObserverEventType::LOOTCREATURE, creature, 0);
				playerManager->rescheduleCorpseDestruction(creature, agent);
				return SUCCESS;
			default:
				break;
			}

			auto group = creature->getGroup();
			if (group == nullptr) {
				return GENERALERROR;
			}

			Reference<GroupLootTask*> task = new GroupLootTask(group, creature, agent, lootAll);
			if (task != nullptr) {
				task->execute();
			}

			return SUCCESS;

		}
		else { // This is /loot area

	
			SortedVector<TreeEntry*> closeObjects;
			CloseObjectsVector* closeObjectsVector = (CloseObjectsVector*) creature->getCloseObjects();
			if (closeObjectsVector == nullptr) {
				auto zone = creature->getZone();
				if (zone == nullptr) {
					return GENERALERROR;
				}
				zone->getInRangeObjects(creature->getWorldPositionX(), creature->getPositionZ(), creature->getWorldPositionY(), 32, &closeObjects, true);
			}
			else {
				closeObjectsVector->safeCopyReceiversTo(closeObjects, CloseObjectsVector::CREOTYPE);
			}

			for (int i = 0; i < closeObjects.size(); ++i) {

				ManagedReference<SceneObject*> targetObject = cast<SceneObject*>(closeObjects.get(i));
				if (targetObject == nullptr) {
					continue;
				}
				
				if (!targetObject->isAiAgent()) {
					continue;
				}

				ManagedReference<AiAgent*> agent = targetObject.castTo<AiAgent*>();
				if (agent == nullptr) {
					continue;
				}

				if (!agent->isDead()) {
					continue;
				}

				ManagedReference<CreatureObject*> targetCreo = targetObject.castTo<CreatureObject*>();
				if (targetCreo == nullptr) {
					continue;
				}

				ManagedReference<Creature*> targetCreature = targetObject.castTo<Creature*>();
				if (targetCreature != nullptr) {// If creature, check for DNA death
					if (targetCreature->getDnaState() == CreatureManager::DNADEATH)
						continue;
				}

				float adjustedRange = range + Math::min(targetCreo->getTemplateRadius() / 4.0f, 8.0f);

				if (!creature->isInRange(agent, adjustedRange)) { //Check range plus adjustment
					continue;
				}

				Locker locker(agent, creature);

				// Get the corpse's inventory.
				ManagedReference<SceneObject*> lootContainer = agent->getSlottedObject("inventory");
				if (lootContainer == nullptr) {
					continue;
				}

				const ContainerPermissions* permissions = lootContainer->getContainerPermissions();
				if (permissions == nullptr) {
					continue;
				}

				// Determine the loot rights.
				uint64 ownerID = permissions->getOwnerID();
				bool looterIsOwner = (ownerID == creature->getObjectID());
				bool groupIsOwner = (ownerID == creature->getGroupID());

				// Allow player to loot the corpse if they own it.
				if (looterIsOwner) {
					playerManager->lootAll(creature, agent);
					continue;
				}

				// If player and their group don't own the corpse, pick up any owned items left on corpse due to full inventory, then fail.
				if (!groupIsOwner) {
					int pickupResult = pickupOwnedItems(agent, creature, lootContainer);
					if (pickupResult < 2) { //Player didn't pickup an item nor is one available for them.
						continue;
					} else if (pickupResult == PICKEDANDEMPTY) {
						agent->notifyObservers(ObserverEventType::LOOTCREATURE, creature, 0);
						playerManager->rescheduleCorpseDestruction(creature, agent);
						continue;
					}
					continue;
				}

				// If looter's group is the owner, attempt to pick up any owned items, then process group loot rule.
				int pickupResult = pickupOwnedItems(agent, creature, lootContainer);

				switch (pickupResult) {
				case NOPICKUPITEMS: //No items available for anyone to pickup.
					break;
				case ITEMFOROTHER: //No items available for looter to pickup, but one is available for someone else.
					agent->notifyObservers(ObserverEventType::LOOTCREATURE, creature, 0);
					lootContainer->openContainerTo(creature);
					continue;
				case PICKEDANDREMAINING: //An item was available for the looter, there are items remaining.
					agent->notifyObservers(ObserverEventType::LOOTCREATURE, creature, 0);
					continue;
				case PICKEDANDEMPTY: //An item was available for the looter, there are NO items remaining.
					agent->notifyObservers(ObserverEventType::LOOTCREATURE, creature, 0);
					playerManager->rescheduleCorpseDestruction(creature, agent);
					continue;
				default:
					break;
				}

				//Infinity: If no loot and no credits, skip and don't do group check
				if (lootContainer->getContainerObjectsSize() < 1 && agent->getCashCredits() == 0) {
					continue;
				}

				ManagedReference<GroupObject*> group = creature->getGroup();
				if (group == nullptr) {
					continue;
				}

				bool lootAll = true;
				
				Reference<GroupLootTask*> task = new GroupLootTask(group, creature, agent, lootAll);
				if (task != nullptr) {
					task->execute();
				}
			}
			
			return SUCCESS;
		}
	}

	int pickupOwnedItems(AiAgent* ai, CreatureObject* creature, SceneObject* lootContainer) const {
		/* Return codes:
		 * NOPICKUPITEMS: No items available for anyone to pickup.
		 * ITEMFOROTHER: No items available for looter to pickup, but one is available for someone else.
		 * PICKEDANDREMAINING: An item was available for the looter, there are items remaining.
		 * PICKEDANDEMPTY: An item was available for the looter, there are NO items remaining.
		 */

		bool attemptedPickup = false;
		bool pickupAvailableOther = false;

		if (lootContainer->getContainerObjectsSize() < 1) return NOPICKUPITEMS;

		ContainerPermissions* contPerms = lootContainer->getContainerPermissionsForUpdate();
		if (contPerms == nullptr) {
			return NOPICKUPITEMS;
		}

		SceneObject* playerInventory = creature->getSlottedObject("inventory");
		if (playerInventory == nullptr) {
			return NOPICKUPITEMS;
		}

		// Check each loot item to see if the player owns it.
		for (int i = lootContainer->getContainerObjectsSize() - 1; i >= 0; --i) {

			ManagedReference<SceneObject*> object = lootContainer->getContainerObject(i,"LootCommand:340");
			if (object == nullptr) {
				continue;
			}

			ContainerPermissions* itemPerms = object->getContainerPermissionsForUpdate();
			if (itemPerms == nullptr) {
				continue;
			} 

			//Check if player owns the loot item.
			uint64 itemOwnerID = itemPerms->getOwnerID();
			if (itemOwnerID == creature->getObjectID()) {

				// Attempt to transfer the item to the player.
				attemptedPickup = true;
				if (playerInventory->isContainerFullRecursive()) {
					StringIdChatParameter full("group", "you_are_full"); //"Your Inventory is full."
					creature->sendSystemMessage(full);
					return PICKEDANDREMAINING;
				}

				uint64 originalOwner = contPerms->getOwnerID();
				contPerms->setOwner(creature->getObjectID());
				TransactionLog trx(ai, creature, object, TrxCode::NPCLOOTCLAIM);

				if (creature->getZoneServer()->getObjectController()->transferObject(object, playerInventory, -1, true)) {
					itemPerms->clearDenyPermission("player", ContainerPermissions::OPEN);
					itemPerms->clearDenyPermission("player", ContainerPermissions::MOVECONTAINER);
					trx.commit();
				}
				else {
					trx.abort() << "Failed to transferObject to player";
				}

				contPerms->setOwner(originalOwner);

			}
			else if (itemOwnerID != 0) {
				pickupAvailableOther = true;
			}
		}

		//Determine which result code to return.
		if (attemptedPickup) {
			if (lootContainer->getContainerObjectsSize() > 0)
				return PICKEDANDREMAINING;
			else
				return PICKEDANDEMPTY;
		}

		if (pickupAvailableOther) {
			return ITEMFOROTHER;
		}

		return NOPICKUPITEMS;
	}
};
#endif //LOOTCOMMAND_H_