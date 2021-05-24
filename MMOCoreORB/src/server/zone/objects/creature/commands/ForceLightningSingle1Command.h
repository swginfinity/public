/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef FORCELIGHTNINGSINGLE1COMMAND_H_
#define FORCELIGHTNINGSINGLE1COMMAND_H_

#include "ForcePowersQueueCommand.h"

class ForceLightningSingle1Command : public ForcePowersQueueCommand {
public:

	ForceLightningSingle1Command(const String& name, ZoneProcessServer* server)
		: ForcePowersQueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		if (isWearingArmor(creature)) {
			return NOJEDIARMOR;
		}
		if (!creature->checkCooldownRecovery("force_lightning")){
           creature->sendSystemMessage("You are still recovering from your last force lightning.");
           return GENERALERROR;
		}
		int res;
		res = doCombatAction(creature, target);
		if (res == SUCCESS){
                        float cop = 0.f;
                        cop = creature->getSkillMod("cloakofpain");
                        float lightningCD = 3750/(1+(cop/50));
                        creature->updateCooldownTimer("force_lightning", lightningCD); // 4.5 second, scales down to 3 with master powers

		}
		return res;
	}

};

#endif //FORCELIGHTNINGSINGLE1COMMAND_H_
