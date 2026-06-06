/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.

	Infinity: Death Watch Orbital Emergency Strike Command
	Fire emergency orbital strike at player's position using beacon from inventory
*/

#ifndef DWORBITALEMERGENCYCOMMAND_H_
#define DWORBITALEMERGENCYCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/managers/director/DirectorManager.h"

class DwOrbitalEmergencyCommand : public QueueCommand {
public:

	DwOrbitalEmergencyCommand(const String& name, ZoneProcessServer* server)
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
		Reference<LuaFunction*> strikeFunc = lua->createFunction("DwOrbitalStrikeCommand", "handleOrbitalEmergency", 0);

		*strikeFunc << creature;

		strikeFunc->callFunction();

		return SUCCESS;
	}

};

#endif //DWORBITALEMERGENCYCOMMAND_H_
