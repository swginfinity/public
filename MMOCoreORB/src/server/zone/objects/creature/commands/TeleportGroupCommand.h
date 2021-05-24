/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef TELEPORTGROUPCOMMAND_H_
#define TELEPORTGROUPCOMMAND_H_

#include "server/zone/managers/player/PlayerManager.h"

class TeleportGroupCommand : public QueueCommand {
public:

	TeleportGroupCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		StringTokenizer args(arguments.toString());
		String targetName = "";

		if(!args.hasMoreTokens())  {
			creature->sendSystemMessage("SYNTAX: /teleportGroup <playerName>.");
			return INVALIDPARAMETERS;
		}

		args.getStringToken(targetName);
		
		ManagedReference<PlayerManager*> playerManager = server->getPlayerManager();
		ManagedReference<CreatureObject*> targetCreature = playerManager->getPlayer(targetName);

		/* if (targetCreature == nullptr) {     // Save this for now as example, but shouldn't need anymore
			uint64 oid = UnsignedLong::valueOf(targetName);
			if (oid != 0)
				targetCreature = server->getZoneServer()->getObject(oid).castTo<CreatureObject*>();
		} */

		if (targetCreature == nullptr) {
			creature->sendSystemMessage("The specified player does not exist.");
			return INVALIDTARGET;
		}

		if (targetCreature->getZone() == nullptr) {
			creature->sendSystemMessage("The specified player is not in a zone that is currently loaded.");
			return INVALIDTARGET;
		}

		if (!targetCreature->isGrouped()) {
			creature->sendSystemMessage("The specifed player is not in a group.");
			return INVALIDTARGET;
		}

		String zoneName = creature->getZone()->getZoneName();
		float x = creature->getPositionX();
		float y = creature->getPositionY();
		float z = creature->getPositionZ();
		uint64 parentid = creature->getParentID();

		if (targetCreature != creature) { //no point in teleporting to ourselves
			Locker clocker(targetCreature, creature);
			targetCreature->switchZone(zoneName, x, z, y, parentid);    //teleport target player first
			clocker.release();
		}

		ManagedReference<GroupObject*> group = targetCreature->getGroup();

		if (group != nullptr) {    //teleport remainder of group

			for (int i = 0; i < group->getGroupSize(); ++i) {
					Reference<CreatureObject*> groupMember = group->getGroupMember(i);

					if (groupMember == nullptr)
						continue;

					if (groupMember == targetCreature || groupMember == creature || !groupMember->isPlayerCreature())
						continue;
					
					Locker clocker(groupMember,creature);
					groupMember->switchZone(zoneName, x, z, y, parentid);
					clocker.release();
			}

			return SUCCESS;
		}
		else {
			creature->sendSystemMessage("The target must be in a group.");
			return INVALIDTARGET;
		}
	}

};

#endif //TELEPORTGROUPCOMMAND_H_
