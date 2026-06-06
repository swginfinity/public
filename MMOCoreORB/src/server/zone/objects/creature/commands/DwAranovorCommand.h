/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.

	Infinity: Death Watch Ara'novor Command
	Targeting Boost serum - grants accuracy bonus for missile targeting.
	Accuracy bonus = power / 5 (range: +10 to +40)

	Architecture: C++ handles all validation, cooldowns, and effect logic.
*/

#ifndef DWARANOVORCOMMAND_H_
#define DWARANOVORCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/managers/death_watch/DeathWatchManager.h"

class DwAranovorCommand : public QueueCommand {
public:

	DwAranovorCommand(const String& name, ZoneProcessServer* server)
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
		return dwManager->applyAranovor(creature);
	}

};

#endif //DWARANOVORCOMMAND_H_
