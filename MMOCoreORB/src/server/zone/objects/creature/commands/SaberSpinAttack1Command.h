/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef SABERSPINATTACK1COMMAND_H_
#define SABERSPINATTACK1COMMAND_H_

#include "JediCombatQueueCommand.h"

class SaberSpinAttack1Command : public JediCombatQueueCommand {
public:

	SaberSpinAttack1Command(const String& name, ZoneProcessServer* server)
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

#endif //SABERSPINATTACK1COMMAND_H_
