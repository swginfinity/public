/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef SABERSPINATTACK3COMMAND_H_
#define SABERSPINATTACK3COMMAND_H_

#include "JediCombatQueueCommand.h"

class SaberSpinAttack3Command : public JediCombatQueueCommand {
public:

	SaberSpinAttack3Command(const String& name, ZoneProcessServer* server)
		: JediCombatQueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		if (isWearingArmor(creature)) {
			return NOJEDIARMOR;
		}

		return doCombatAction(creature, target);
	}

};

#endif //SABERSPINATTACK3COMMAND_H_
