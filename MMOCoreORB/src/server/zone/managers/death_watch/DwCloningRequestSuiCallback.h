/*
 * DwCloningRequestSuiCallback.h
 *
 * Death Watch Safe Clone UI Callback
 * Handles player selection from the DW safe house clone menu
 */

#ifndef DWCLONINGREQUESTSUICALLBACK_H_
#define DWCLONINGREQUESTSUICALLBACK_H_

#include "server/zone/objects/player/sui/SuiCallback.h"
#include "server/zone/objects/player/sui/listbox/SuiListBox.h"
#include "server/zone/managers/death_watch/DeathWatchManager.h"

class DwCloningRequestSuiCallback : public SuiCallback {
	int typeofdeath;

public:
	DwCloningRequestSuiCallback(ZoneServer* server, int tod)
		: SuiCallback(server) {
		typeofdeath = tod;
	}

	void run(CreatureObject* player, SuiBox* suiBox, uint32 eventIndex, Vector<UnicodeString>* args) {
		bool cancelPressed = (eventIndex == 1);

		if (!suiBox->isListBox() || player == nullptr || args->size() <= 0)
			return;

		SuiListBox* listbox = cast<SuiListBox*>(suiBox);

		int index = Integer::valueOf(args->get(0).toString());

		// Player must still be dead
		if (!player->isDead())
			return;

		// Handle cancel or invalid selection
		if (index < 0 || index >= listbox->getMenuSize() || cancelPressed) {
			player->sendSystemMessage("You will remain dead until you choose a location to clone or you are revived. Type /activateClone to restore the cloning window.");
			return;
		}

		uint64 clonerID = listbox->getMenuObjectID(index);
		if (clonerID == 0) {
			player->sendSystemMessage("Invalid clone location selected.");
			return;
		}

		// Check if this is their HOME (pre-designated) facility
		PlayerObject* ghost = player->getPlayerObject();
		bool isPreDesignated = false;
		if (ghost != nullptr) {
			isPreDesignated = (ghost->getCloningFacility() == clonerID);
		}

		// Send to DW cloner
		ManagedReference<DeathWatchManager*> dwManager = server->getDeathWatchManager();
		if (dwManager != nullptr) {
			dwManager->sendDeathWatchPlayerToCloner(player, clonerID, typeofdeath, isPreDesignated);
		} else {
			player->sendSystemMessage("Death Watch cloning system unavailable.");
		}
	}
};

#endif /* DWCLONINGREQUESTSUICALLBACK_H_ */
