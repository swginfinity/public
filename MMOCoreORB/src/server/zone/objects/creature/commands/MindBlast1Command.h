/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef MINDBLAST1COMMAND_H_
#define MINDBLAST1COMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "ForcePowersQueueCommand.h"

class MindBlast1Command : public ForcePowersQueueCommand {
public:

	MindBlast1Command(const String& name, ZoneProcessServer* server)
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

		ManagedReference<SceneObject*> targetObject = server->getZoneServer()->getObject(target);

		if (targetObject == nullptr || !targetObject->isCreatureObject()) {
			return INVALIDTARGET;
		}
		
        ManagedReference<CreatureObject*> creatureTarget = targetObject.castTo<CreatureObject*>();
        if (creatureTarget == nullptr){
            return INVALIDTARGET;
        }
		
		if (!creature->checkCooldownRecovery("mindblast")){
           creature->sendSystemMessage("You cannot use mindblast again yet.");
           return GENERALERROR;
		}
		int res;
		res = doCombatAction(creature, target);
		if (res == SUCCESS){
			creature->updateCooldownTimer("mindblast", 3000); 
		}
		return res;
	}

};

#endif //MINDBLAST1COMMAND_H_
