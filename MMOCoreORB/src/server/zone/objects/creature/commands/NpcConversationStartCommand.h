/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef NPCCONVERSATIONSTARTCOMMAND_H_
#define NPCCONVERSATIONSTARTCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/objects/ship/ai/SpaceStationObject.h"
#include "server/zone/managers/collision/CollisionManager.h"
#include "server/chat/StringIdChatParameter.h"

class NpcConversationStartCommand : public QueueCommand {
public:
	NpcConversationStartCommand(const String& name, ZoneProcessServer* server) : QueueCommand(name, server) {
	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {
		if (!checkStateMask(creature)) {
			return INVALIDSTATE;
		}

		if (!checkInvalidLocomotions(creature)) {
			return INVALIDLOCOMOTION;
		}

		if (!creature->isPlayerCreature()) {
			return GENERALERROR;
		}

		auto ghost = creature->getPlayerObject();

		if (ghost == nullptr) {
			return GENERALERROR;
		}

		auto zoneServer = creature->getZoneServer();

		if (zoneServer == nullptr) {
			return GENERALERROR;
		}

		ManagedReference<SceneObject*> object = zoneServer->getObject(target);

		if (object == nullptr)
			return GENERALERROR;

		if (object->isAiAgent()) {
			AiAgent* agent = object->asAiAgent();

			if (agent == nullptr)
				return INVALIDTARGET;

			try {
				Locker clocker(agent, creature);

				ValidatedPosition* validPosition = ghost->getLastValidatedPosition();

				if (validPosition == nullptr)
					return GENERALERROR;

				Vector3 playerPosition = validPosition->getPosition();
				uint64 playerParentID = validPosition->getParent();

				Vector3 agentPosition = agent->getPosition();
				uint64 agentParentID = agent->getParentID();

				// Infinity: Cell-aware distance check — no conversing from different cells
				if (playerParentID != agentParentID) {
					StringIdChatParameter params;
					params.setStringId("@ui:sui_different_cell_start");
					params.setTT(agent->getDisplayedName());
					creature->sendSystemMessage(params);
					return TOOFAR;
				}

				// Infinity: Outdoor NPCs use 2D distance (ignore Z/terrain height). LoS also checked below
				if (agentParentID == 0) {
					float distanceSq = playerPosition.squaredDistanceTo2d(agentPosition);

					if (distanceSq > 25) {
						return TOOFAR;
					}
				} else if (playerPosition.squaredDistanceTo(agentPosition) > 25) {
					return TOOFAR;
				}

				// Infinity: No conversing without line of sight
				if (!CollisionManager::checkLineOfSight(agent, creature)) {
					// Surface a message so the player understands why nothing
					// happened, and record the same LOS fail hint the combat
					// commands do — that way /unstickTarget can recognize
					// "you can't talk to it either" as a valid stuck signal.
					creature->sendSystemMessage("@cbt_spam:los_fail"); // "You lost sight of your target."
					StringBuffer losKeyBuf;
					losKeyBuf << "los_fail_" << agent->getObjectID();
					creature->addCooldown(losKeyBuf.toString(), 60 * 1000);
					return GENERALERROR;
				}

				ghost->setConversatingObject(agent);

				if (agent->sendConversationStartTo(creature)) {
					agent->notifyObservers(ObserverEventType::STARTCONVERSATION, creature);
				}
			} catch (Exception& e) {
				e.printStackTrace();
				creature->error() << "parseNpcStartConversation exception - NPC: " << agent->getDisplayedName() << " OID: " << agent->getObjectID() << " Template: " << agent->getObjectTemplate()->getTemplateFileName() << " Zone: " << (agent->getZone() != nullptr ? agent->getZone()->getZoneName() : "null") << " Position: " << agent->getWorldPosition().toString();
			}
		} else if (object->isShipAiAgent()) {
			try {
				auto shipAgent = object->asShipAiAgent();

				if (shipAgent == nullptr) {
					return INVALIDTARGET;
				}

				Locker lock(shipAgent, creature);

				ghost->setConversatingObject(shipAgent);

				if (shipAgent->sendConversationStartTo(creature)) {
					shipAgent->notifyObservers(ObserverEventType::STARTCONVERSATION, creature);
				}
			} catch (Exception& e) {
				e.printStackTrace();
			}
		}

		return SUCCESS;
	}
};

#endif // NPCCONVERSATIONSTARTCOMMAND_H_
