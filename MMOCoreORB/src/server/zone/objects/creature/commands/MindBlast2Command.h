/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef MINDBLAST2COMMAND_H_
#define MINDBLAST2COMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "ForcePowersQueueCommand.h"

class MindBlast2Command : public ForcePowersQueueCommand {
public:

	MindBlast2Command(const String& name, ZoneProcessServer* server)
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

		auto zoneServer = server->getZoneServer();
		if (zoneServer == nullptr) {
			return GENERALERROR;
		}

		ManagedReference<SceneObject*> targetObject = zoneServer->getObject(target);

		if (targetObject == nullptr || !targetObject->isCreatureObject()) {
			return INVALIDTARGET;
		}

        ManagedReference<CreatureObject*> creatureTarget = targetObject.castTo<CreatureObject*>();
        if (creatureTarget == nullptr) {
            return INVALIDTARGET;
        }
		
		if (!creature->checkCooldownRecovery("mindblast")){
           creature->sendSystemMessage("You cannot use mindblast again yet.");
           return GENERALERROR;
		}

		int res = doCombatAction(creature, target);

		if (res == SUCCESS){
			creature->updateCooldownTimer("mindblast", 3 * 1000); //Infinity: 3 second cooldown
		}

		return res;
	}

};

#endif //MINDBLAST2COMMAND_H_
