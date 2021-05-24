/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef KNOCKDOWNFIRECOMMAND_H_
#define KNOCKDOWNFIRECOMMAND_H_

#include "CombatQueueCommand.h"

class KnockdownFireCommand : public CombatQueueCommand {
public:

	KnockdownFireCommand(const String& name, ZoneProcessServer* server)
		: CombatQueueCommand(name, server) {
	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		ManagedReference<WeaponObject*> weapon = creature->getWeapon();
		
		if (weapon == NULL)
			return GENERALERROR;

		String type = weapon->getWeaponType();
		
		if (type != "pistol" && type != "carbine"){
			creature->sendSystemMessage("You must be using a pistol or carbine to use this ability");
			return GENERALERROR;
		}

		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		return doCombatAction(creature, target);
	}

};

#endif //KNOCKDOWNFIRECOMMAND_H_
