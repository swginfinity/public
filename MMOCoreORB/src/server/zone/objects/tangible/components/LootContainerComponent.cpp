/*
 * LootContainerComponent.cpp
 *
 *  Created on: Jan 24, 2012
 *      Author: xyborn
 */

#include "LootContainerComponent.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/objects/group/GroupObject.h"
#include "server/zone/objects/player/PlayerObject.h"

int LootContainerComponent::notifyObjectRemoved(SceneObject* sceneObject, SceneObject* object, SceneObject* destination) const {
	if (destination != nullptr) {
		ManagedReference<SceneObject*> rootParent = destination->getParent().get();

		if (rootParent != nullptr && rootParent->isCreatureObject()) {
			CreatureObject* creature = cast<CreatureObject*>(rootParent.get());

			if (creature != nullptr) {
				ManagedReference<GroupObject*> group = creature->getGroup();

				if (group != nullptr) {
					StringIdChatParameter params("group", "notify_single_loot"); //[GROUP] %TU looted %TO from %TT.
					params.setTO(object->getDisplayedName());
					params.setTU(creature->getDisplayedName());
					params.setTT(sceneObject->getParent().get()->getDisplayedName() );
					group->sendSystemMessage(params,true,2);  //sendtoself = true, messageType 1 = Harvest, 2 = Loot

					// Need to generate loot message if group loot messages are turned off
					ManagedReference<PlayerObject*> creatureGhost = creature->getPlayerObject();
					if (creatureGhost != nullptr) {
						String typeSelected = creatureGhost->getScreenPlayData("group_message_selection", "typeSelected");
						if (typeSelected == "Harvest Only" || typeSelected == "None")  { //Group loot messages turned off?
							StringIdChatParameter params("base_player", "prose_item_looted_self_detail"); // You looted %TO from %TT
							params.setTO(object->getDisplayedName());
							params.setTT(sceneObject->getParent().get()->getDisplayedName());
							creature->sendSystemMessage(params);
						}
					}

				} else {
					StringIdChatParameter params("base_player", "prose_item_looted_self_detail"); // You looted %TO from %TT
					params.setTO(object->getDisplayedName());
					params.setTT(sceneObject->getParent().get()->getDisplayedName());
					creature->sendSystemMessage(params);
				}
			}
		}
	}

	return 0;
}


bool LootContainerComponent::checkContainerPermission(SceneObject* sceneObject, CreatureObject* creature, uint16 permission) const {
	ContainerPermissions* permissions = sceneObject->getContainerPermissions();
	if(permission == ContainerPermissions::MOVEIN)
		return false;
	else if (permission == ContainerPermissions::MOVEOUT ){
		return (permissions->getOwnerID() == creature->getObjectID() || permissions->getOwnerID() == creature->getGroupID());
	}

	return false;
}

int LootContainerComponent::canAddObject(SceneObject* sceneObject, SceneObject* object, int containmentType, String& errorDescription) const {

	if(sceneObject->getContainerObjectsSize() >= 0){
		errorDescription = "@error_message:remove_only_corpse"; //You cannot place items into a corpse.
		 return TransferErrorCode::INVALIDTYPE;
	}
	
	return 1;
}

