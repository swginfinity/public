/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef SABERLEGHIT1COMMAND_H_
#define SABERLEGHIT1COMMAND_H_

#include "JediCombatQueueCommand.h"

class SaberLegHit1Command : public JediCombatQueueCommand {
public:

	SaberLegHit1Command(const String& name, ZoneProcessServer* server)
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

#endif //SABERLEGHIT1COMMAND_H_
