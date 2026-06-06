/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef ATTACKCOMMAND_H_
#define ATTACKCOMMAND_H_

#include "CombatQueueCommand.h"

class AttackCommand : public CombatQueueCommand {
public:
	AttackCommand(const String& name, ZoneProcessServer* server) : CombatQueueCommand(name, server) {
	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {
		bool ridingMount = creature->isRidingMount();

		if ((ridingMount && !creature->isSwimming() && !checkStateMask(creature)) || (!ridingMount && !checkStateMask(creature))) {
			return INVALIDSTATE;
		}

		if (!checkInvalidLocomotions(creature)) {
			return INVALIDLOCOMOTION;
		}

		// Death Watch jetpack: silent no-op - use custom DW combat commands instead
		if (ridingMount) {
			ManagedReference<SceneObject*> parent = creature->getParent().get();
			if (parent != nullptr && parent->getServerObjectCRC() == 0x8A24BE7D) {
				return SUCCESS;
			}
		}

		return doCombatAction(creature, target);
	}
};

#endif // ATTACKCOMMAND_H_
