/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.

	Infinity: Death Watch Whistling Bird Command
	Fire whistling bird missiles (Mk1-6) via command instead of radial menu
*/

#ifndef DWWHISTLINGBIRDCOMMAND_H_
#define DWWHISTLINGBIRDCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/managers/director/DirectorManager.h"

class DwWhistlingBirdCommand : public QueueCommand {
public:

	DwWhistlingBirdCommand(const String& name, ZoneProcessServer* server)
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

		// Parse the mark number from arguments
		String args = arguments.toString().trim();
		if (args.isEmpty()) {
			creature->sendSystemMessage("Usage: /dwWhistlingBird <1-6>");
			return INVALIDPARAMETERS;
		}

		int markNumber = 0;
		try {
			markNumber = Integer::valueOf(args);
		} catch (...) {
			creature->sendSystemMessage("Invalid mark number. Usage: /dwWhistlingBird <1-6>");
			return INVALIDPARAMETERS;
		}

		if (markNumber < 1 || markNumber > 6) {
			creature->sendSystemMessage("Invalid mark number. Must be 1-6.");
			return INVALIDPARAMETERS;
		}

		// Call Lua handler for all the logic
		Lua* lua = DirectorManager::instance()->getLuaInstance();
		if (lua == nullptr)
			return GENERALERROR;
		Reference<LuaFunction*> birdFunc = lua->createFunction("DwWhistlingBirdCommand", "handleCommand", 0);

		*birdFunc << creature;
		*birdFunc << markNumber;
		*birdFunc << target;

		birdFunc->callFunction();

		return SUCCESS;
	}

};

#endif //DWWHISTLINGBIRDCOMMAND_H_
