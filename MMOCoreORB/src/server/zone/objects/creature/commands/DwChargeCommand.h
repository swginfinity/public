/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.

	Infinity: Death Watch Charge Command
	Berserk mode ability for Krrsantan Box 3
	Increased damage at the cost of reduced defense

	Architecture: C++ handles validation + cooldowns, Lua handles effect logic
*/

#ifndef DWCHARGECOMMAND_H_
#define DWCHARGECOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/managers/director/DirectorManager.h"
#include "server/zone/managers/death_watch/DeathWatchManager.h"

class DwChargeCommand : public QueueCommand {
public:

	DwChargeCommand(const String& name, ZoneProcessServer* server)
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
		if (!creature->hasSkill("combat_death_watch_krr_3")) {
			creature->sendSystemMessage("You must have Black Krrsantan Box 3 to use this command.");
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
		if (!dwManager->checkAndApplyCooldown(creature, "dwCharge")) {
			return GENERALERROR;
		}

		// Call Lua handler for effect logic only
		Lua* lua = DirectorManager::instance()->getLuaInstance();
		Reference<LuaFunction*> chargeFunc = lua->createFunction("DwChargeCommand", "applyEffect", 0);

		*chargeFunc << creature;

		chargeFunc->callFunction();

		return SUCCESS;
	}

};

#endif //DWCHARGECOMMAND_H_
