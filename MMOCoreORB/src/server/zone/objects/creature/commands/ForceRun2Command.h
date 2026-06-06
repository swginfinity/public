/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions. */

#ifndef FORCERUN2COMMAND_H_
#define FORCERUN2COMMAND_H_

#include "server/zone/objects/creature/buffs/PrivateSkillMultiplierBuff.h"
#include "JediQueueCommand.h"

class ForceRun2Command : public JediQueueCommand {
public:
	ForceRun2Command(const String& name, ZoneProcessServer* server) : JediQueueCommand(name, server) {

		// BuffCRC's, first one is used.
		buffCRC = BuffCRC::JEDI_FORCE_RUN_2;

		blockingCRCs.add(STRING_HASHCODE("luaSetSpeed"));

		skillMods.put("force_run", 2);
		skillMods.put("slope_move", 66);
	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (creature->hasBuff(STRING_HASHCODE("luaSetSpeed"))) {
			creature->sendSystemMessage("You cannot use Force Run 2 while your speed is being controlled by an unknown entity.");
			return GENERALERROR;
		}

		if (creature->hasBuff(buffCRC)) {  //Infinity:  Toggle force run off if active
			creature->removeBuff(buffCRC);
			return SUCCESS;
		}

   		if (!creature->checkCooldownRecovery("forcerun2")) {
			creature->sendSystemMessage("You cannot use Force Run 2 again so soon.");
			return GENERALERROR;
		}

		int res = doJediSelfBuffCommand(creature);

		// Return if something is in error.
		if (res != SUCCESS) {
			return res;
		}

		creature->updateCooldownTimer("forcerun2", 150000);		//Infinity:  Cooldown is 150 seconds from start of buff

		// Infinity:   Cancel all other speed buffs  
		if (creature->hasBuff(BuffCRC::JEDI_FORCE_RUN_1)) {		//Cancel Force Run 1 if Force Run 2 activated
			creature->removeBuff(BuffCRC::JEDI_FORCE_RUN_1);
		}

		if (creature->hasBuff(BuffCRC::JEDI_FORCE_RUN_3)) {		//Cancel Force Run 3 if Force Run 2 activated
			creature->removeBuff(BuffCRC::JEDI_FORCE_RUN_3);
		}

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

		creature->notifyObservers(ObserverEventType::ABILITYUSED, nullptr, STRING_HASHCODE("forcerun2"));

		// Return.
		return SUCCESS;
	}

};

#endif //FORCERUN2COMMAND_H_
