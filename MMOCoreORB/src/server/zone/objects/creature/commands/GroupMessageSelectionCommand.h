/*
 * GroupMessageSelectionCommand.h
 *
 *  Modified on: March 2, 2015
 *      Author: MrO
 */

#ifndef GROUPMESSAGESELECTIONCOMMAND_H_
#define GROUPMESSAGESELECTIONCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/objects/player/sui/listbox/SuiListBox.h"
#include "server/zone/objects/player/sui/callbacks/GroupMessageSelectionSuiCallback.h"

class GroupMessageSelectionCommand : public QueueCommand {
public:

	GroupMessageSelectionCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		ManagedReference<PlayerObject*> ghost = creature->getPlayerObject();
		if (ghost == nullptr)
			return GENERALERROR;

		//Close SUI box if already open.
		ghost->closeSuiWindowType(SuiWindowType::GROUP_MESSAGE_SELECTION);

		//Create the Group Loot Selection SUI box.
		ManagedReference<SuiListBox*> sui = new SuiListBox(creature, SuiWindowType::GROUP_MESSAGE_SELECTION);
		sui->setPromptTitle("Group System Message Selection");
		sui->setPromptText("Choose the harvest and loot group system message types that you wish to receive:"); //"Choose which group system messages to receive:"
		sui->setCancelButton(true, "@ui:cancel");
		sui->setOkButton(true, "@ui:ok");
		sui->setCallback(new GroupMessageSelectionSuiCallback(creature->getZoneServer()));

		sui->addMenuItem("All"); 
		sui->addMenuItem("Harvest Only"); 
		sui->addMenuItem("Loot Only"); 
		sui->addMenuItem("None"); 

		//Send group leader the SUI box.
		ghost->addSuiBox(sui);
		creature->sendMessage(sui->generateMessage());


		return SUCCESS;

	}

};

#endif /* GROUPMESSAGESELECTIONCOMMAND_H_ */
