/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.

	Infinity: Death Watch Hettir Command
	AoE Bacta Denial - 32m radius, applies debuff that blocks stimpack use.
	Duration: 45 seconds fixed

	Architecture: C++ handles all validation, cooldowns, and effect logic.
*/

#ifndef DWHETTIRCOMMAND_H_
#define DWHETTIRCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/managers/death_watch/DeathWatchManager.h"

class DwHettirCommand : public QueueCommand {
public:

	DwHettirCommand(const String& name, ZoneProcessServer* server)
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
		return dwManager->applyHettir(creature);
	}

};

#endif //DWHETTIRCOMMAND_H_
