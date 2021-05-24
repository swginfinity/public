/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef CREATEEFFECTCOMMAND_H_
#define CREATEEFFECTCOMMAND_H_

#include "server/zone/managers/player/PlayerManager.h"

class CreateEffectCommand : public QueueCommand {
public:

	CreateEffectCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;


		StringTokenizer args(arguments.toString());
		
		String targetName;
		String effectName;
		try {
			args.getStringToken(targetName);

		} catch (Exception& e) {
			creature->sendSystemMessage("SYNTAX: /createEffect <targetName> <effectName>");
			return INVALIDPARAMETERS;
		}

		ManagedReference<PlayerManager*> playerManager = server->getPlayerManager();
		ManagedReference<CreatureObject*> targetCreature = playerManager->getPlayer(targetName);

		if (targetCreature == NULL) {
			creature->sendSystemMessage("The specified player does not exist: " + targetName);
			return INVALIDTARGET;
		}

		if (targetCreature->getZone() == NULL) {
			creature->sendSystemMessage("The specified player is not in a zone that is currently loaded.");
			return INVALIDTARGET;
		}


		if (args.hasMoreTokens()){
			args.getStringToken(effectName);
		}
		else{
			creature->sendSystemMessage("SYNTAX: /createEffect <targetName> <effectName>");
			creature->sendSystemMessage("targetName: " + targetName + " Effect Name: " + effectName);
			return INVALIDPARAMETERS;
		}


		targetCreature->playEffect(effectName, "head");


		return SUCCESS;
	}

};

#endif //CREATEEFFECTCOMMAND_H_
