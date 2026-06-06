/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.

	Infinity: Death Watch Faction Status Command
	Displays player's DW faction, standing, XP, and current war state
*/

#ifndef DWFACTIONSTATUSCOMMAND_H_
#define DWFACTIONSTATUSCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/managers/director/DirectorManager.h"

class DwFactionStatusCommand : public QueueCommand {
public:

	DwFactionStatusCommand(const String& name, ZoneProcessServer* server)
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

		// Call Lua handler for all the formatting and display
		Lua* lua = DirectorManager::instance()->getLuaInstance();
		if (lua == nullptr)
			return GENERALERROR;
		Reference<LuaFunction*> statusFunc = lua->createFunction("DwFactionStatusCommand", "handleCommand", 0);

		*statusFunc << creature;

		statusFunc->callFunction();

		return SUCCESS;
	}

};

#endif //DWFACTIONSTATUSCOMMAND_H_
