/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef CLEARCOMPLETEDQUESTCOMMAND_H_
#define CLEARCOMPLETEDQUESTCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"

class ClearCompletedQuestCommand : public QueueCommand {
public:

	ClearCompletedQuestCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		auto zoneServer = server->getZoneServer();
		if (zoneServer == nullptr)
			return GENERALERROR;

		//Infinity:  Command heavily modified to support player name argument
		ManagedReference<SceneObject*> obj = nullptr;
		ManagedReference<CreatureObject*> targetCreature = nullptr;
		StringTokenizer args(arguments.toString());

		try {
			if (target != 0) {
				obj = zoneServer->getObject(target);
				if (obj == nullptr || !obj->isPlayerCreature()) {
					return INVALIDTARGET;
				}
				else {
					targetCreature = cast<CreatureObject*>(obj.get());
				}
				if (targetCreature == nullptr) {
					return INVALIDTARGET;
				}
			}
			else {
				String playerName;
				args.getStringToken(playerName);

				auto playerManager = zoneServer->getPlayerManager();
				if (playerManager == nullptr)
					return GENERALERROR;

				targetCreature = playerManager->getPlayer(playerName);
				if (targetCreature == nullptr) {
					creature->sendSystemMessage("SYNTAX: /clearCompletedQuest [player] screenplaystate <screenPlayStateName> <state>");
					creature->sendSystemMessage("SYNTAX: /clearCompletedQuest [player] quest <questName> <activeQuestBitmask> <completedQuestBitmask>");
					return INVALIDTARGET;
				}
			}

			String commandType;
			args.getStringToken(commandType);

			Locker targetLock(targetCreature, creature);

			if (commandType.beginsWith("screenplaystate")) {

				String screenPlayName = args.getStringToken();

				uint64 state = args.getLongToken();
				state = ~state; // invert state to only clear the requested state.

				targetCreature->setScreenPlayState(screenPlayName, state & targetCreature->getScreenPlayState(screenPlayName));

				creature->sendSystemMessage(targetCreature->getFirstName() + " " + screenPlayName + " set to " + String::valueOf(targetCreature->getScreenPlayState(screenPlayName)));

			}
			else if (commandType.beginsWith("quest")) {
				
				ManagedReference<PlayerObject*> ghost = targetCreature->getPlayerObject();

				if (ghost == nullptr)
					return INVALIDTARGET;

				String questName = args.getStringToken();
				int questCRC;

				if (questName.contains("quest")) {
					questCRC = questName.hashCode();
				}
				else {
					questCRC = Integer::valueOf(questName);
				}

				int active = 0;
				int completed = 0;

				if (args.hasMoreTokens()) {
					active = args.getIntToken();
					completed = args.getIntToken();
				}
				PlayerQuestData data;
				data.setOwnerId(ghost->getObjectID());
				data.setActiveStepBitmask(active);
				data.setCompletedStepBitmask(completed);
				data.setCompletedFlag(0);

				ghost->setPlayerQuestData(questCRC, data, true);
				if (active == 0 && completed == 0) {
					creature->sendSystemMessage("Quest progress reset.");
				}
				else {
					creature->sendSystemMessage("Updated quest progress.");
				}
			} else {
				creature->sendSystemMessage("SYNTAX: /clearCompletedQuest [player] screenplaystate <screenPlayStateName> <state>");
				creature->sendSystemMessage("SYNTAX: /clearCompletedQuest [player] quest <questName> <activeQuestBitmask> <completedQuestBitmask>");
				return INVALIDPARAMETERS;
			}
		} catch (Exception& e) {
			creature->sendSystemMessage("SYNTAX: /clearCompletedQuest [player] screenplaystate <screenPlayStateName> <state>");
			creature->sendSystemMessage("SYNTAX: /clearCompletedQuest [player] quest <questName> <activeQuestBitmask> <completedQuestBitmask>");
			return INVALIDPARAMETERS;
		}

		return SUCCESS;
	}

};

#endif //CLEARCOMPLETEDQUESTCOMMAND_H_
