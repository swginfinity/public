/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef BATTLEFIELDSTATUSCOMMAND_H_
#define BATTLEFIELDSTATUSCOMMAND_H_

#include "server/zone/managers/director/DirectorManager.h"

class BattlefieldStatusCommand : public QueueCommand {
public:

	BattlefieldStatusCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		// Infinity: 10s cooldown to prevent spam
		if (!creature->checkCooldownRecovery("BattlefieldStatusCommand")) {
			const Time* cooldownTime = creature->getCooldownTime("BattlefieldStatusCommand");
			if (cooldownTime != nullptr) {
				float timeLeft = fabs(cooldownTime->miliDifference()) / 1000.f;
				creature->sendSystemMessage("You can use the BattlefieldStatus command again in " + String::format("%.1f", timeLeft) + " second" + ((timeLeft == 1.0f) ? "." : "s."));
				return GENERALERROR;
			} else {
				creature->sendSystemMessage("You can only use the BattlefieldStatus command once every 10 seconds.");
				return GENERALERROR;
			}
		}

		// Infinity: Call instant_pvp screenplay for battlefield status display
		Lua* lua = DirectorManager::instance()->getLuaInstance();
		Reference<LuaFunction*> luaBattlefieldStatusCommand = lua->createFunction("instant_pvp", "battlefieldStatusCommand", 0);

		*luaBattlefieldStatusCommand << creature;
		luaBattlefieldStatusCommand->callFunction();

		creature->updateCooldownTimer("BattlefieldStatusCommand", 10000);

		return SUCCESS;
	}

};

#endif //BATTLEFIELDSTATUSCOMMAND_H_
