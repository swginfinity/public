/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.

	Infinity: Death Watch Biometric Sampling Command
*/

#ifndef DWSAMPLEDNACOMMAND_H_
#define DWSAMPLEDNACOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/managers/director/DirectorManager.h"

class DwSampleDNACommand : public QueueCommand {
public:

	DwSampleDNACommand(const String& name, ZoneProcessServer* server)
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
			creature->sendSystemMessage("You must be Death Watch to use this command.");
			return GENERALERROR;
		}

		// Cannot be in combat
		if (creature->isInCombat()) {
			creature->sendSystemMessage("You cannot sample DNA while in combat.");
			return GENERALERROR;
		}

		// Call Lua handler - it handles sampler search, target validation, etc.
		// Lua uses player's current target via getTargetID() for consistency with radial menu
		Lua* lua = DirectorManager::instance()->getLuaInstance();
		Reference<LuaFunction*> sampleFunc = lua->createFunction("DwSampleDNACommand", "handleCommand", 0);

		*sampleFunc << creature;

		sampleFunc->callFunction();

		return SUCCESS;
	}

};

#endif //DWSAMPLEDNACOMMAND_H_
