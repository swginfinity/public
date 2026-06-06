/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.

	Infinity: Death Watch Beskar'gam Command
	Damage Shield serum - grants damage mitigation via private_mitigation_divisor.
	Mitigation = 1.5 base + (power/100) + (dw_healing * 0.05), max 3.0

	Architecture: C++ handles all validation, cooldowns, and effect logic.
*/

#ifndef DWBESKARGAMCOMMAND_H_
#define DWBESKARGAMCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/managers/death_watch/DeathWatchManager.h"

class DwBeskargamCommand : public QueueCommand {
public:

	DwBeskargamCommand(const String& name, ZoneProcessServer* server)
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
		return dwManager->applyBeskargam(creature);
	}

};

#endif //DWBESKARGAMCOMMAND_H_
