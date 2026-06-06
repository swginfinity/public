/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef ADDFRIENDCOMMAND_H_
#define ADDFRIENDCOMMAND_H_

#include "server/zone/objects/player/PlayerObject.h"
#include "server/chat/StringIdChatParameter.h"
#include "server/zone/managers/player/PlayerManager.h"
#include "server/zone/objects/creature/CreatureObject.h"
// Infinity: Custom includes
#include "server/chat/ChatManager.h"
#include "server/zone/packets/chat/ChatFriendsListUpdate.h"
#include "server/zone/objects/player/variables/FriendList.h"

class AddFriendCommand : public QueueCommand {
public:

	AddFriendCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		if (!creature->isPlayerCreature())
			return GENERALERROR;

		String name = arguments.toString();

		ManagedReference<PlayerObject*> ghost = creature->getSlottedObject("ghost").castTo<PlayerObject*>();
		if (ghost == nullptr) {
			creature->sendSystemMessage("Could not find player object.");
			return GENERALERROR;
		}

		if (name == "appearoffline") {
			// Infinity: Allow players to appear offline to friends
			auto chatManager = server->getChatManager();
			if (chatManager == nullptr) {
				return GENERALERROR;
			}

			bool appearOffline = ghost->getScreenPlayData("addfriend_appear_offline", "status") == "true";

			if (appearOffline) {
				ghost->setScreenPlayData("addfriend_appear_offline", "status", "false");
				creature->sendSystemMessage("You will appear online to friends after your next login.");
			} else {
				ghost->setScreenPlayData("addfriend_appear_offline", "status", "true");
				ghost->notifyOffline();
				creature->sendSystemMessage("You will now appear offline to all friends except for staff.");
			}
			return SUCCESS;
		}

		if (ghost->isIgnoring(name)) {
			StringIdChatParameter param("cmnty", "friend_fail_is_ignored");
			param.setTT(name);
			creature->sendSystemMessage(param);

			return GENERALERROR;
		}

		if (ghost->hasFriend(name)) {
			StringIdChatParameter param("cmnty", "friend_duplicate");
			param.setTT(name);
			creature->sendSystemMessage(param);

			return GENERALERROR;
		}

		if (ghost->getFriendListSize() >= 1000) {
			creature->sendSystemMessage("You have too many friends!");

			return GENERALERROR;
		}

		auto creo = server->getPlayerManager()->getPlayer(name);
		if (creo != nullptr) {
			PlayerObject* player = creo->getPlayerObject();
			if (player != nullptr && !ghost->hasGodMode()) {
				if (player->hasGodMode()) {
					creature->sendSystemMessage("You may not add that player to your friend list.");

					return GENERALERROR;
				}
			}
		}
		auto playerManager = server->getPlayerManager();

		bool validName = playerManager->existsName(name);

		if (!validName) {
			StringIdChatParameter param("cmnty", "friend_not_found");
			param.setTT(name);
			creature->sendSystemMessage(param);

			return GENERALERROR;
		}

		ghost->addFriend(name);

		return SUCCESS;
	}

};

#endif //ADDFRIENDCOMMAND_H_
