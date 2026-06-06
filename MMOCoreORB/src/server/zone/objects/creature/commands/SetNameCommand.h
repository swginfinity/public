/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef SETNAMECOMMAND_H_
#define SETNAMECOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/objects/player/PlayerObject.h"

class SetNameCommand : public QueueCommand {
public:

	SetNameCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		ZoneServer* zoneServer = server->getZoneServer();

		if (zoneServer == nullptr)
			return GENERALERROR;

		StringTokenizer args(arguments.toString());

		if (!args.hasMoreTokens()) {
			creature->sendSystemMessage("Syntax: /setName objectID NewName  — or target an object and /setName NewName");
			return INVALIDPARAMETERS;
		}

		// OID-first parsing — if the first token resolves to a real object,
		// prefer it over the (possibly stale) command-queue target. Closes
		// the failure mode where a forgotten selection gets renamed when the
		// GM intended to rename by OID. Falls back to the selected target if
		// the first token isn't a valid OID.
		uint64 targetID = 0;
		String firstToken;
		args.getStringToken(firstToken);

		bool firstIsValidOid = false;

		if (!firstToken.isEmpty()) {
			bool allDigits = true;

			for (int i = 0; i < firstToken.length(); ++i) {
				char c = firstToken.charAt(i);
				if (c < '0' || c > '9') {
					allDigits = false;
					break;
				}
			}

			if (allDigits) {
				uint64 candidate = (uint64) Long::valueOf(firstToken);

				if (candidate != 0 && zoneServer->getObject(candidate) != nullptr) {
					firstIsValidOid = true;
					targetID = candidate;
				}
			}
		}

		if (!firstIsValidOid) {
			// First token wasn't an OID — rewind so the name string includes it.
			args = StringTokenizer(arguments.toString());
			targetID = target;
		}

		Reference<SceneObject*> targetObj = zoneServer->getObject(targetID);

		if (targetObj == nullptr) {
			creature->sendSystemMessage("Invalid Target. Syntax: /setName objectID NewName  — or target an object and /setName NewName");
			return INVALIDTARGET;
		}

		if (targetObj->isPlayerCreature()) {
			creature->sendSystemMessage("Player Names cannot be changed with this command. Use /setFirstName and /setLastName.");
			return INVALIDTARGET;
		}

		PlayerObject* ghost = creature->getPlayerObject();
		bool bypassFilter = (ghost != nullptr && ghost->isPrivileged());

		Locker clocker(targetObj, creature);

		String newName = args.getRemainingString();

		if (newName.isEmpty()) {
			creature->sendSystemMessage("You must specify a new name.");
			return GENERALERROR;
		}

		if (!bypassFilter) {
			NameManager* nameManager = server->getNameManager();

			if (nameManager->validateName(newName, -1) != NameManagerResult::ACCEPTED) {
				creature->sendSystemMessage("That name was rejected by the name filter.");
				return GENERALERROR;
			}
		}

		String oldName = targetObj->getDisplayedName();

		targetObj->setCustomObjectName(newName, true);

		creature->sendSystemMessage("OID " + String::valueOf(targetObj->getObjectID()) + " changed from '" + oldName + "' to '" + newName + "'");

		return SUCCESS;
	}

};

#endif //SETNAMECOMMAND_H_
