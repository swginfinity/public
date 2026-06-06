/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef FORCEKNOCKDOWN2COMMAND_H_
#define FORCEKNOCKDOWN2COMMAND_H_

#include "server/zone/objects/tangible/TangibleObject.h"
#include "ForcePowersQueueCommand.h"

class ForceKnockdown2Command : public ForcePowersQueueCommand {
public:

	ForceKnockdown2Command(const String& name, ZoneProcessServer* server)
		: ForcePowersQueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!checkStateMask(creature)) {
			return INVALIDSTATE;
		}

		if (!checkInvalidLocomotions(creature)) {
			return INVALIDLOCOMOTION;
		}

		if (isWearingArmor(creature)) {
			return NOJEDIARMOR;
		}

		// Infinity: Allow targeting TangibleObjects (turrets, destructibles) not just creatures
		ManagedReference<SceneObject*> targetSceno = server->getZoneServer()->getObject(target);

		if (targetSceno == nullptr || !targetSceno->isTangibleObject()) {
			return INVALIDTARGET;
		}

		TangibleObject* targetTano = dynamic_cast<TangibleObject*>(targetSceno.get());

		if (!(targetTano != nullptr && targetTano->isAttackableBy(creature))) {
			return INVALIDTARGET;
		}

		return doCombatAction(creature, target);
	}

};

#endif //FORCEKNOCKDOWN2COMMAND_H_
