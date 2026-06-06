/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.

	Infinity: Death Watch Fortify Command
	Defensive stance ability for Xonti Brothers Box 3
	Reduced movement speed, increased defense

	Architecture: C++ handles validation + cooldowns, Lua handles effect logic
*/

#ifndef DWFORTIFYCOMMAND_H_
#define DWFORTIFYCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/managers/director/DirectorManager.h"
#include "server/zone/managers/death_watch/DeathWatchManager.h"

class DwFortifyCommand : public QueueCommand {
public:

	DwFortifyCommand(const String& name, ZoneProcessServer* server)
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
		if (!creature->hasSkill("combat_death_watch_xon_3")) {
			creature->sendSystemMessage("You must have Xonti Brothers Box 3 to use this command.");
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
		if (!dwManager->checkAndApplyCooldown(creature, "dwFortify")) {
			return GENERALERROR;
		}

		// Call Lua handler for effect logic only
		Lua* lua = DirectorManager::instance()->getLuaInstance();
		if (lua == nullptr)
			return GENERALERROR;
		Reference<LuaFunction*> fortifyFunc = lua->createFunction("DwFortifyCommand", "applyEffect", 0);

		*fortifyFunc << creature;

		fortifyFunc->callFunction();

		return SUCCESS;
	}

};

#endif //DWFORTIFYCOMMAND_H_
