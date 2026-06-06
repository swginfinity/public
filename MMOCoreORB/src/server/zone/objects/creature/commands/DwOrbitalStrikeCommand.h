/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.

	Infinity: Death Watch Orbital Strike Command
	Fire orbital strike using equipped lightning beam + trigger from inventory
*/

#ifndef DWORBITALSTRIKECOMMAND_H_
#define DWORBITALSTRIKECOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/managers/director/DirectorManager.h"

class DwOrbitalStrikeCommand : public QueueCommand {
public:

	DwOrbitalStrikeCommand(const String& name, ZoneProcessServer* server)
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

		// Must have Black Sun Box 2 skill
		if (!creature->hasSkill("combat_death_watch_sun_2")) {
			creature->sendSystemMessage("You must have Black Sun Box 2 to use this command.");
			return GENERALERROR;
		}

		// Call Lua handler for all the logic
		Lua* lua = DirectorManager::instance()->getLuaInstance();
		Reference<LuaFunction*> strikeFunc = lua->createFunction("DwOrbitalStrikeCommand", "handleOrbitalStrike", 0);

		*strikeFunc << creature;
		*strikeFunc << target;

		strikeFunc->callFunction();

		return SUCCESS;
	}

};

#endif //DWORBITALSTRIKECOMMAND_H_
