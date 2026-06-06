/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef FORCELIGHTNINGCONE1COMMAND_H_
#define FORCELIGHTNINGCONE1COMMAND_H_

#include "ForcePowersQueueCommand.h"

class ForceLightningCone1Command : public ForcePowersQueueCommand {
public:

	ForceLightningCone1Command(const String& name, ZoneProcessServer* server)
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
           creature->sendSystemMessage("You are still recovering from your last Force Lightning.");
           return GENERALERROR;
		}

		int res = doCombatAction(creature, target);
		if (res == SUCCESS) {
			float cloakOfPain = creature->getSkillMod("cloakofpain");
			float lightningCD = 3750.0f / (1 + (cloakOfPain/50.0f));
			creature->updateCooldownTimer("force_lightning", lightningCD); // 4.5 second, scales down to 3 with master powers
		}

		return res;
	}

};

#endif //FORCELIGHTNINGCONE1COMMAND_H_
