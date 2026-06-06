/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.

	Infinity: Death Watch Tor'cabur Command
	Mind Immunity toggle serum - grants immunity to mind damage from DW attacks.
	Using while active toggles the buff OFF and starts cooldown.

	Architecture: C++ handles all validation, cooldowns, and effect logic.
*/

#ifndef DWTORCABURCOMMAND_H_
#define DWTORCABURCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/managers/death_watch/DeathWatchManager.h"

class DwTorcaburCommand : public QueueCommand {
public:

	DwTorcaburCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {
		if (!checkStateMask(creature)) {
			return INVALIDSTATE;
		}

		if (!checkInvalidLocomotions(creature)) {
			return INVALIDLOCOMOTION;
		}

		// Must be a player
		if (!creature->isPlayerCreature()) {
			return GENERALERROR;
		}

		auto zoneServer = server->getZoneServer();
		if (zoneServer == nullptr) {
			return GENERALERROR;
		}

		DeathWatchManager* dwManager = zoneServer->getDeathWatchManager();
		if (dwManager == nullptr) {
			return GENERALERROR;
		}

		// Delegate to DeathWatchManager
		return dwManager->applyTorcabur(creature);
	}

};

#endif //DWTORCABURCOMMAND_H_
