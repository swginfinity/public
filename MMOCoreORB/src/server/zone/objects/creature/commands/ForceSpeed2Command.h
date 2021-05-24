/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions. */

#ifndef FORCESPEED2COMMAND_H_
#define FORCESPEED2COMMAND_H_

class ForceSpeed2Command : public JediQueueCommand {
public:

	ForceSpeed2Command(const String& name, ZoneProcessServer* server)
	: JediQueueCommand(name, server) {

		buffCRC = BuffCRC::JEDI_FORCE_SPEED_2;
		overrideableCRCs.add(BuffCRC::JEDI_FORCE_SPEED_1);
		skillMods.put("combat_haste", 25);
	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {
		ManagedReference<GroupObject*> group = creature->getGroup();

		if (0) {  // (creature->hasSkill("force_discipline_enhancements_master") && group != NULL && group->getGroupSize() > 1){
			creature->sendSystemMessage("Your mastery of the enhancement discipline allows you to extend this power to others near you");
			int groupres = doJediGroupBuffCommand(creature);

		}

		return doJediSelfBuffCommand(creature);
	}

};

#endif //FORCESPEED2COMMAND_H_
