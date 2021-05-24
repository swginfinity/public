/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions. */

#ifndef FORCERUN1COMMAND_H_
#define FORCERUN1COMMAND_H_

#include "JediQueueCommand.h"
#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/objects/group/GroupObject.h"

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
		ManagedReference<GroupObject*> group = creature->getGroup();

		if (0) {  // creature->hasSkill("force_discipline_enhancements_master") && group != NULL && group->getGroupSize() > 1){
			creature->sendSystemMessage("Your mastery of the enhancement discipline allows you to extend this power to others near you");
			int res = doJediGroupBuffCommand(creature);
		}

		if (creature->hasBuff(buffCRC)) {
			creature->removeBuff(buffCRC);
			return SUCCESS;
		}


                int res = doJediSelfBuffCommand(creature);

		// Return if something is in error.
		if (res != SUCCESS) {
			return res;
		}

		// SPECIAL - For Force Run.
		if (creature->hasBuff(STRING_HASHCODE("burstrun")) || creature->hasBuff(STRING_HASHCODE("retreat"))) {
			creature->removeBuff(STRING_HASHCODE("burstrun"));
			creature->removeBuff(STRING_HASHCODE("retreat"));
		}

		// Return.
		return SUCCESS;
	}

};

#endif //FORCERUN1COMMAND_H_
