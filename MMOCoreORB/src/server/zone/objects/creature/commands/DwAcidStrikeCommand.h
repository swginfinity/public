/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.

	Infinity: Death Watch Acid Strike Command
	Fire acid strike using equipped acid beam + trigger from inventory
*/

#ifndef DWACIDSTRIKECOMMAND_H_
#define DWACIDSTRIKECOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/managers/director/DirectorManager.h"

class DwAcidStrikeCommand : public QueueCommand {
public:

	DwAcidStrikeCommand(const String& name, ZoneProcessServer* server)
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

		// Must have Black Krrsantan Box 2 skill
		if (!creature->hasSkill("combat_death_watch_krr_2")) {
			creature->sendSystemMessage("You must have Black Krrsantan Box 2 to use this command.");
			return GENERALERROR;
		}

		// Call Lua handler for all the logic
		Lua* lua = DirectorManager::instance()->getLuaInstance();
		if (lua == nullptr)
			return GENERALERROR;
		Reference<LuaFunction*> strikeFunc = lua->createFunction("DwAcidStrikeCommand", "handleAcidStrike", 0);

		*strikeFunc << creature;
		*strikeFunc << target;

		strikeFunc->callFunction();

		return SUCCESS;
	}

};

#endif //DWACIDSTRIKECOMMAND_H_
