/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef INVITECOMMAND_H_
#define INVITECOMMAND_H_


#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/managers/group/GroupManager.h"
#include "server/zone/ZoneServer.h"

class InviteCommand : public QueueCommand {
public:

	InviteCommand(const String& name, ZoneProcessServer* server) : QueueCommand(name, server) {
	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {
		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		ManagedReference<PlayerObject*> ghost = creature->getPlayerObject();
		bool godMode = false;

		if (ghost != nullptr && ghost->isPrivileged()) {
			godMode = true;
		}

		auto zoneServer = server->getZoneServer();
		if (zoneServer == nullptr)
			return GENERALERROR;

		ManagedReference<SceneObject*> object = zoneServer->getObject(target);

		bool galaxyWide = ConfigManager::instance()->getBool("Core3.PlayerManager.GalaxyWideGrouping", false);
		bool friendsOnly = false;

		if (galaxyWide && (object == nullptr || (!object->isPlayerCreature() && !object->isShipObject()) || object == creature)) {  //Infinity:  Still works if you target yourself
			StringTokenizer args(arguments.toString());
			String firstName;

			if (args.hasMoreTokens()) {
				args.getStringToken(firstName);
			}

			firstName = firstName.toLowerCase();

			if (firstName == "friendsonly") {
				friendsOnly = true;
				if (args.hasMoreTokens()) {
					args.getStringToken(firstName);
				}
			}

			if (zoneServer == nullptr)
				return GENERALERROR;

			auto playerManager = zoneServer->getPlayerManager();
			if (playerManager == nullptr)
				return GENERALERROR;

			object = playerManager->getPlayer(firstName);
		}

		auto groupManager = GroupManager::instance();

		if (object == nullptr || groupManager == nullptr)
			return GENERALERROR;

		if (!object->isPlayerCreature() && !object->isShipObject()) {
			return GENERALERROR;
		}

		CreatureObject* player = nullptr;

		if (object->isShipObject()) {
			auto ship = object->asShipObject();

			if (ship != nullptr) {
				player = ship->getOwner().get();
			}
		} else {
			player = object->asCreatureObject();
		}
		if (player == nullptr)
			return GENERALERROR;

		auto invitedGhost = player->getPlayerObject();
		if (invitedGhost == nullptr)
			return GENERALERROR;

		// Cannot be invited by a player that they ignore, does not apply to privileged players
		if (!godMode && invitedGhost->isIgnoring(creature->getFirstName()))
			return GENERALERROR;

		// If friendsOnly is specified, will only invite players on the group leader's friend list
		if (!invitedGhost->hasGodMode() && friendsOnly && !ghost->hasFriend(player->getFirstName().toLowerCase()))
			return GENERALERROR;

		groupManager->inviteToGroup(creature, player);

		return SUCCESS;
	}

};

#endif //INVITECOMMAND_H_
