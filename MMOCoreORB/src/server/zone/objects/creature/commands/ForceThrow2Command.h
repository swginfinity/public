/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef FORCETHROW2COMMAND_H_
#define FORCETHROW2COMMAND_H_

#include "ForcePowersQueueCommand.h"

class ForceThrow2Command : public ForcePowersQueueCommand {
public:

	ForceThrow2Command(const String& name, ZoneProcessServer* server)
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
		if (zoneServer == nullptr)
			return GENERALERROR;

		ManagedReference<SceneObject*> object = zoneServer->getObject(target);

		if (object == nullptr)
			return INVALIDTARGET;

		UnicodeString args = "healthDamageMultiplier=0.33f;actionDamageMultiplier=0.33f;mindDamageMultiplier=0.33f;";   // Infinity:  Split the damage over 3 pools
		int res = doCombatAction(creature, target, args);

		ManagedReference<CreatureObject*> targetCreature = cast<CreatureObject*>(object.get());
		if (targetCreature != nullptr && res == SUCCESS) {
			targetCreature->playEffect("clienteffect/mustafar/dark_jedi_rock_attack_1.cef", "");  //Infinity:  Special animation
		}

		return res;

	}

};

#endif //FORCETHROW2COMMAND_H_
