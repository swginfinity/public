/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.

	Infinity: Death Watch Identify Command
	Scan another player to gather intel based on subterfuge skill
*/

#ifndef DWIDENTIFYCOMMAND_H_
#define DWIDENTIFYCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/managers/director/DirectorManager.h"

class DwIdentifyCommand : public QueueCommand {
public:

	DwIdentifyCommand(const String& name, ZoneProcessServer* server)
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
			creature->sendSystemMessage("You cannot identify targets while in combat.");
			return GENERALERROR;
		}

		// Must have a target
		if (target == 0) {
			creature->sendSystemMessage("You must have a target to identify.");
			return INVALIDTARGET;
		}

		// Call Lua handler for all the logic
		Lua* lua = DirectorManager::instance()->getLuaInstance();
		if (lua == nullptr)
			return GENERALERROR;
		Reference<LuaFunction*> identifyFunc = lua->createFunction("DwIdentifyCommand", "handleCommand", 0);

		*identifyFunc << creature;
		*identifyFunc << target;

		identifyFunc->callFunction();

		return SUCCESS;
	}

};

#endif //DWIDENTIFYCOMMAND_H_
