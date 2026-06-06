/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef SABERBODYHIT1COMMAND_H_
#define SABERBODYHIT1COMMAND_H_

#include "JediCombatQueueCommand.h"

class SaberBodyHit1Command : public JediCombatQueueCommand {
public:

	SaberBodyHit1Command(const String& name, ZoneProcessServer* server)
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

#endif //SABERBODYHIT1COMMAND_H_
