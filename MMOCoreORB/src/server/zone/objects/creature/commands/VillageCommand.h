/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef VILLAGECOMMAND_H_
#define VILLAGECOMMAND_H_

#include "server/zone/managers/director/DirectorManager.h"
#include "server/zone/managers/jedi/JediManager.h"

class VillageCommand : public QueueCommand {
public:

	VillageCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {
		if (!checkStateMask(creature)) {
			return INVALIDSTATE;
		}

		if (!checkInvalidLocomotions(creature)) {
			return INVALIDLOCOMOTION;
		}

		if (JediManager::instance()->getJediProgressionType() != JediManager::VILLAGEJEDIPROGRESSION) {
			return GENERALERROR;
		}

		if (!creature->checkCooldownRecovery("VillageCommand"))  {
			const Time* cooldownTime = creature->getCooldownTime("VillageCommand");
			if (cooldownTime != nullptr) {
				float timeLeft = fabs(cooldownTime->miliDifference()) / 1000.f;
				creature->sendSystemMessage("You can use the village command again in " + String::format("%.1f", timeLeft) + " second" + ((timeLeft == 1.0f) ? "." : "s."));
				return GENERALERROR;
			} 
			else {
				creature->sendSystemMessage("You can only use the village command once every 10 seconds.");
                return GENERALERROR;
			}
		}

		Lua* lua = DirectorManager::instance()->getLuaInstance();
		Reference<LuaFunction*> luaVillageCommand = lua->createFunction("VillageCommand", "villageCommand", 0);

		*luaVillageCommand << creature;
		luaVillageCommand->callFunction();

        bool liveServer = true;
		auto zoneServer = creature->getZoneServer();
		if (zoneServer != nullptr) {
			liveServer = zoneServer->getGalaxyName() == "SWG Infinity";
		}

        if (liveServer) {
		    creature->updateCooldownTimer("VillageCommand", 10 * 1000);   // 10 Second cooldown on live server only
        }

		return SUCCESS;
	}

};

#endif //VILLAGECOMMAND_H_
