/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.

	Infinity: Death Watch Escape Command
	Emergency escape ability for Zann Consortium Box 3
	Destroys jetpack for high-speed getaway with fog cover

	Architecture: C++ handles validation + cooldowns, Lua handles effect logic
*/

#ifndef DWESCAPECOMMAND_H_
#define DWESCAPECOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/managers/director/DirectorManager.h"
#include "server/zone/managers/death_watch/DeathWatchManager.h"

class DwEscapeCommand : public QueueCommand {
public:

	DwEscapeCommand(const String& name, ZoneProcessServer* server)
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
		if (!creature->hasSkill("combat_death_watch_zan_3")) {
			creature->sendSystemMessage("You must have Zann Consortium Box 3 to use this command.");
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
		if (!dwManager->checkAndApplyCooldown(creature, "dwEscape")) {
			return GENERALERROR;
		}

		// Call Lua handler for effect logic only
		Lua* lua = DirectorManager::instance()->getLuaInstance();
		if (lua == nullptr)
			return GENERALERROR;
		Reference<LuaFunction*> escapeFunc = lua->createFunction("DwEscapeCommand", "applyEffect", 0);

		*escapeFunc << creature;

		escapeFunc->callFunction();

		return SUCCESS;
	}

};

#endif //DWESCAPECOMMAND_H_
