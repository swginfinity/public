/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef SETBOUNTYCOMMAND_H_
#define SETBOUNTYCOMMAND_H_


#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/ZoneServer.h"
#include "server/zone/managers/mission/MissionManager.h"

class SetBountyCommand : public QueueCommand {
public:

	SetBountyCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		PlayerObject* playerObject = creature->getPlayerObject();

		GroupManager* groupManager = GroupManager::instance();

		ManagedReference<SceneObject*> object = NULL;
		MissionManager* missionManager = creature->getZoneServer()->getMissionManager();
		ManagedReference<PlayerObject*> ghost = playerCreature->getPlayerObject();
		if (target != 0 && target != creature->getObjectID())
			object = server->getZoneServer()->getObject(target);
		else if (!arguments.isEmpty()) {
			StringTokenizer tokenizer(arguments.toString());
			if (tokenizer.hasMoreTokens()) {
				String name;
				tokenizer.getStringToken(name);
				name = name.toLowerCase();
				if (name != "self" && name != "this") {
					try {
						object = server->getPlayerManager()->getPlayer(name);
						//missionManager->updatePlayerBountyReward(object->getObjectID(), ghost->calculateBhReward() + 25000);
						missionManager->updatePlayerBountyReward(object, VisibilityManager::instance()->calculateReward(ghost) + 25000);
					} catch (ArrayIndexOutOfBoundsException& ex) {
						// this happens if the player wasn't found
					}
				}
			}
		}
		
		if (object == NULL) {
			return GENERALERROR;
		}
		
		return SUCCESS;
	}

};

#endif //SETBOUNTYCOMMAND_H_
