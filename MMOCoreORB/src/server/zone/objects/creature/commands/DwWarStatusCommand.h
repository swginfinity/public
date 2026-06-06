/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.

	Infinity: Death Watch War Status Command
	Quick display of current war kill counts
*/

#ifndef DWWARSTATUSCOMMAND_H_
#define DWWARSTATUSCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/managers/director/DirectorManager.h"

class DwWarStatusCommand : public QueueCommand {
public:

	DwWarStatusCommand(const String& name, ZoneProcessServer* server)
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

		// Call Lua handler for all the logic
		Lua* lua = DirectorManager::instance()->getLuaInstance();
		if (lua == nullptr)
			return GENERALERROR;
		Reference<LuaFunction*> statusFunc = lua->createFunction("DwWarStatusCommand", "handleCommand", 0);

		*statusFunc << creature;

		statusFunc->callFunction();

		return SUCCESS;
	}

};

#endif //DWWARSTATUSCOMMAND_H_
