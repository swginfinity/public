/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.

	Infinity: Death Watch Gotal'din Command
	Missile Reflector serum - grants chance to reflect Whistling Bird missiles.
	Stores reflect power in skill mod for missile hit processing.

	Architecture: C++ handles all validation, cooldowns, and effect logic.
*/

#ifndef DWGOTALDINCOMMAND_H_
#define DWGOTALDINCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/managers/death_watch/DeathWatchManager.h"

class DwGotaldinCommand : public QueueCommand {
public:

	DwGotaldinCommand(const String& name, ZoneProcessServer* server)
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
		return dwManager->applyGotaldin(creature);
	}

};

#endif //DWGOTALDINCOMMAND_H_
