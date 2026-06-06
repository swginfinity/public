/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.

	Infinity: Death Watch Cloak Command
	Stealth/disruption ability for Black Sun Box 3
	Speed boost, fog cover, disables enemy HUDs

	Architecture: C++ handles validation + cooldowns, Lua handles effect logic
*/

#ifndef DWCLOAKCOMMAND_H_
#define DWCLOAKCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/managers/director/DirectorManager.h"
#include "server/zone/managers/death_watch/DeathWatchManager.h"

class DwCloakCommand : public QueueCommand {
public:

	DwCloakCommand(const String& name, ZoneProcessServer* server)
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

		// Skill check - C++
		if (!creature->hasSkill("combat_death_watch_sun_3")) {
			creature->sendSystemMessage("You must have Black Sun Box 3 to use this command.");
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

		// Cooldown check/set - C++ (duration from Lua config)
		if (!dwManager->checkAndApplyCooldown(creature, "dwCloak")) {
			return GENERALERROR;
		}

		// Call Lua handler for effect logic only
		Lua* lua = DirectorManager::instance()->getLuaInstance();
		Reference<LuaFunction*> cloakFunc = lua->createFunction("DwCloakCommand", "applyEffect", 0);

		*cloakFunc << creature;

		cloakFunc->callFunction();

		return SUCCESS;
	}

};

#endif //DWCLOAKCOMMAND_H_
