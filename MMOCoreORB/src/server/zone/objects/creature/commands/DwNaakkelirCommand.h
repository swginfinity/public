/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.

	Infinity: Death Watch Naak'kelir Command
	Purge serum - clears negative states in priority order.
	States cleared = 1 + floor(power/125) + floor(dw_recovery/25)

	Architecture: C++ handles all validation, cooldowns, and effect logic.
*/

#ifndef DWNAAKKELIRCOMMAND_H_
#define DWNAAKKELIRCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/managers/death_watch/DeathWatchManager.h"

class DwNaakkelirCommand : public QueueCommand {
public:

	DwNaakkelirCommand(const String& name, ZoneProcessServer* server)
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
		return dwManager->applyNaakkelir(creature);
	}

};

#endif //DWNAAKKELIRCOMMAND_H_
