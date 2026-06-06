/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef COMPLETEQUESTCOMMAND_H_
#define COMPLETEQUESTCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"

class CompleteQuestCommand : public QueueCommand {
public:

	CompleteQuestCommand(const String& name, ZoneProcessServer* server)
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

		ManagedReference<SceneObject*> obj = nullptr;
		ManagedReference<CreatureObject*> targetCreature = nullptr;
		StringTokenizer args(arguments.toString());

		try { //Infinity:  Allow player name as argument
		
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
					creature->sendSystemMessage("SYNTAX: [playerName] /completeQuest quest <quest>");
					creature->sendSystemMessage("SYNTAX: [playerName] /completeQuest quest <questName> <activeQuestBitmask> <completedQuestBitmask> <questCompleted>");
                    creature->sendSystemMessage("SYNTAX: [playerName] /completeQuest screenplaystate <screenPlayStateName> <state>");
                    creature->sendSystemMessage("SYNTAX: [playerName] /completeQuest deletescreenplaydata <screenplay> <variable>");
                    creature->sendSystemMessage("SYNTAX: [playerName] /completeQuest writescreenplaydata <screenplay> <variable> <data>");
					return INVALIDTARGET;
				}
			}

			String commandType;
			args.getStringToken(commandType);

			Locker targetLock(targetCreature, creature);

			if (commandType.beginsWith("screenplaystate")) {
				String screenPlayName;
				args.getStringToken(screenPlayName);
				uint64 state = args.getLongToken();

				targetCreature->setScreenPlayState(screenPlayName, state | targetCreature->getScreenPlayState(screenPlayName));

				creature->sendSystemMessage(targetCreature->getFirstName() + " " + screenPlayName + " state set to " + String::valueOf(targetCreature->getScreenPlayState(screenPlayName)));

			} else if (commandType.beginsWith("quest")) {
				
				auto ghost = targetCreature->getPlayerObject();

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

				if (!args.hasMoreTokens()) {
					ghost->completeQuest(questCRC);
					creature->sendSystemMessage("Completed quest.");
				}
				else {
					int active = args.getIntToken();
					int completed = args.getIntToken();
					int finished = args.getIntToken();

					PlayerQuestData data;
					data.setOwnerId(ghost->getObjectID());
					data.setActiveStepBitmask(active);
					data.setCompletedStepBitmask(completed);
					data.setCompletedFlag(finished);

					ghost->setPlayerQuestData(questCRC, data, true);
					creature->sendSystemMessage("Updated quest progress.");
				}
			} else if (commandType.beginsWith("writescreenplaydata")) {

				auto ghost = targetCreature->getPlayerObject();

				if (ghost == nullptr)
					return INVALIDTARGET;

                String screenPlayName;
				args.getStringToken(screenPlayName);

                String variableName;
				args.getStringToken(variableName);

                String dataValue;
				args.getStringToken(dataValue);

                if (!screenPlayName.isEmpty() && !variableName.isEmpty() && !dataValue.isEmpty()) {
                    ghost->setScreenPlayData(screenPlayName, variableName, dataValue);
                    creature->sendSystemMessage(targetCreature->getFirstName() + "'s screenplay data for screenplay " + screenPlayName + " and variable " + variableName +  " set to " + dataValue);
                }
                else {
                    creature->sendSystemMessage("SYNTAX: [playerName] /completeQuest writescreenplaydata <screenplay> <variable> <data>");

                    return INVALIDPARAMETERS;                    
                }

            } else if (commandType.beginsWith("deletescreenplaydata")) {

				auto ghost = targetCreature->getPlayerObject();

				if (ghost == nullptr)
					return INVALIDTARGET;

                String screenPlayName;
				args.getStringToken(screenPlayName);

                String variableName;
				args.getStringToken(variableName);

                if (!screenPlayName.isEmpty() && !variableName.isEmpty()) {
                    ghost->deleteScreenPlayData(screenPlayName, variableName);
                    creature->sendSystemMessage(targetCreature->getFirstName() + "'s screenplay data for screenplay " + screenPlayName + " and variable " + variableName + " deleted.");
                }
                else {
                    creature->sendSystemMessage("SYNTAX: [playerName] /completeQuest deletescreenplaydata <screenplay> <variable>");

                    return INVALIDPARAMETERS;                    
                }

            }

			else {
                creature->sendSystemMessage("SYNTAX: [playerName] /completeQuest quest <quest>");
                creature->sendSystemMessage("SYNTAX: [playerName] /completeQuest quest <questName> <activeQuestBitmask> <completedQuestBitmask> <questCompleted>");
                creature->sendSystemMessage("SYNTAX: [playerName] /completeQuest screenplaystate <screenPlayStateName> <state>");
                creature->sendSystemMessage("SYNTAX: [playerName] /completeQuest deletescreenplaydata <screenplay> <variable>");
                creature->sendSystemMessage("SYNTAX: [playerName] /completeQuest writescreenplaydata <screenplay> <variable> <data>");

				return INVALIDPARAMETERS;
			}
		} catch (Exception& e) {
                creature->sendSystemMessage("SYNTAX: [playerName] /completeQuest quest <quest>");
                creature->sendSystemMessage("SYNTAX: [playerName] /completeQuest quest <questName> <activeQuestBitmask> <completedQuestBitmask> <questCompleted>");
                creature->sendSystemMessage("SYNTAX: [playerName] /completeQuest screenplaystate <screenPlayStateName> <state>");
                creature->sendSystemMessage("SYNTAX: [playerName] /completeQuest deletescreenplaydata <screenplay> <variable>");
                creature->sendSystemMessage("SYNTAX: [playerName] /completeQuest writescreenplaydata <screenplay> <variable> <data>");

			return INVALIDPARAMETERS;
		}

		return SUCCESS;
	}

};

#endif //COMPLETEQUESTCOMMAND_H_
