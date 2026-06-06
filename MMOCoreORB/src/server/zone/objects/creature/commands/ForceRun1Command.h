/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions. */

#ifndef FORCERUN1COMMAND_H_
#define FORCERUN1COMMAND_H_

#include "JediQueueCommand.h"

class ForceRun1Command : public JediQueueCommand {
public:

	ForceRun1Command(const String& name, ZoneProcessServer* server)
	: JediQueueCommand(name, server) {

		// BuffCRC's, first one is used.
		buffCRC = BuffCRC::JEDI_FORCE_RUN_1;

        // If these are active they will block buff use
		blockingCRCs.add(BuffCRC::JEDI_FORCE_RUN_2);
		blockingCRCs.add(BuffCRC::JEDI_FORCE_RUN_3);


		// Skill mods.
		skillMods.put("force_run", 1);
		skillMods.put("slope_move", 33);
	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		// Infinity: Block if speed is being controlled by Lua (e.g. screenplay)
		if (creature->hasBuff(STRING_HASHCODE("luaSetSpeed"))) {
			creature->sendSystemMessage("You cannot use Force Run 1 while your speed is being controlled by an unknown entity.");
			return GENERALERROR;
		}

		// Infinity: Toggle force run off if already active
		if (creature->hasBuff(buffCRC)) {
			creature->removeBuff(buffCRC);
			return SUCCESS;
		}

		int res = doJediSelfBuffCommand(creature);

		// Return if something is in error.
		if (res != SUCCESS) {
			return res;
		}

		// Cancel all other speed buffs
		if (creature->hasBuff(STRING_HASHCODE("burstrun"))) {
			creature->removeBuff(STRING_HASHCODE("burstrun"));
		}

		if (creature->hasBuff(STRING_HASHCODE("gallop"))) {
			creature->removeBuff(STRING_HASHCODE("gallop"));
		}

		if (creature->hasBuff(STRING_HASHCODE("retreat"))) {
			creature->removeBuff(STRING_HASHCODE("retreat"));
		}

		if (creature->hasBuff(STRING_HASHCODE("dash"))) {
			creature->removeBuff(STRING_HASHCODE("dash"));
		}

		if (creature->hasBuff(STRING_HASHCODE("rocketboots"))) {
			creature->removeBuff(STRING_HASHCODE("rocketboots"));
		}

		// Return.
		return SUCCESS;
	}

};

#endif //FORCERUN1COMMAND_H_
