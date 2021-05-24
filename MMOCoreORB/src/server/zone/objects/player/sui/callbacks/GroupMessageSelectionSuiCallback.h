/*
 * GroupMessageSelectionSuiCallback.h
 *
 *  Created on: March 1, 2015
 *      Author: Anakis
 */

#ifndef GROUPMESSAGESELECTIONSUICALLBACK_H_
#define GROUPMESSAGESELECTIONSUICALLBACK_H_

#include "server/zone/objects/player/sui/SuiCallback.h"

class GroupMessageSelectionSuiCallback : public SuiCallback {
public:
	GroupMessageSelectionSuiCallback(ZoneServer* server) : SuiCallback(server) {

	}

	void run(CreatureObject* player, SuiBox* suiBox, uint32 eventIndex, Vector<UnicodeString>* args) {
		bool cancelPressed = (eventIndex == 1);

		//Pre: player is locked
		//Post: player is locked

		if (cancelPressed || !suiBox->isListBox() || player == nullptr || args->size() <= 0)
			return;

		int selection = Integer::valueOf(args->get(0).toString()); //The row number they chose in the list.

		if (selection < 0) //Player made no selection but pressed OK.
			return;

		ManagedReference<PlayerObject*> playerGhost = player->getPlayerObject();

		if (playerGhost == nullptr)	
			return;

		String choice;
		switch (selection) {

			case 3:		choice = "None";
						break;
			case 2:		choice = "Loot Only";
						break;
			case 1:		choice = "Harvest Only";
						break;
			case 0:		choice = "All";
						break;
			default:	choice = "All";
						break;
		}
		
		//Logger::console.error("Selection String = " + choice + " Selection Integer = " + String::valueOf(selection));

		playerGhost->setScreenPlayData("group_message_selection", "typeSelected", choice);

		player->sendSystemMessage("You will now receive the following harvest and loot group system messages:  " + choice);
	}

};


#endif /* GROUPMESSAGESELECTIONSUICALLBACK_H_ */
