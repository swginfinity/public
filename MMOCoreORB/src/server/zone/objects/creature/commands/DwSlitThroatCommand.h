/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.

	Infinity: Death Watch Black Sun Slit Throat Command
	A brutal ambush attack that cuts the target's Action HAM in half
	Requires: Black Sun Box 3 skill, Shadow Protocol active, Death Watch Dagger equipped
*/

#ifndef DWSLITTHROATCOMMAND_H_
#define DWSLITTHROATCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/managers/director/DirectorManager.h"
#include "server/zone/managers/death_watch/DeathWatchManager.h"

class DwSlitThroatCommand : public QueueCommand {
public:

	DwSlitThroatCommand(const String& name, ZoneProcessServer* server)
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

		// Must have Death Watch Novice skill
		if (!creature->hasSkill("combat_death_watch_novice")) {
			creature->sendSystemMessage("\\#FF6666You are not Death Watch.");
			return GENERALERROR;
		}

		// Must have Black Sun Box 3 skill
		if (!creature->hasSkill("combat_death_watch_sun_3")) {
			creature->sendSystemMessage("\\#FF6666Only Black Sun operatives can use this technique.");
			return GENERALERROR;
		}

		// Get Death Watch Manager
		ZoneServer* zoneServer = server->getZoneServer();
		if (zoneServer == nullptr) {
			return GENERALERROR;
		}

		DeathWatchManager* dwManager = zoneServer->getDeathWatchManager();
		if (dwManager == nullptr) {
			return GENERALERROR;
		}

		// Check if Shadow Protocol is active - REQUIRED for Slit Throat
		// NOTE: We only CHECK here, don't deactivate - Lua handles deactivation
		// using the same code path as normal Shadow Protocol expiration
		if (!dwManager->isShadowProtocolActive(creature)) {
			creature->sendSystemMessage("\\#FF6666Slit Throat requires Shadow Protocol to be active.");
			return GENERALERROR;
		}

		// Get target - use passed target or fall back to player's current target
		uint64 actualTarget = target;
		if (actualTarget == 0) {
			actualTarget = creature->getTargetID();
		}

		if (actualTarget == 0) {
			creature->sendSystemMessage("\\#FF6666You must have a valid target selected.");
			return GENERALERROR;
		}

		// Call Lua handler - it will deactivate Shadow Protocol using the same
		// code path as normal expiration, then deal damage
		Lua* lua = DirectorManager::instance()->getLuaInstance();
		if (lua == nullptr)
			return GENERALERROR;
		Reference<LuaFunction*> abilityFunc = lua->createFunction("dw_faction_ability", "executeSlitThroat", 0);

		*abilityFunc << creature;
		*abilityFunc << actualTarget;

		abilityFunc->callFunction();

		return SUCCESS;
	}

};

#endif //DWSLITTHROATCOMMAND_H_
