/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef GRANTBADGECOMMAND_H_
#define GRANTBADGECOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
// Infinity #1620-class follow-up: -player <name> lookup for offline-target CSR use
#include "server/chat/ChatManager.h"

class GrantBadgeCommand : public QueueCommand {
public:

	GrantBadgeCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		if (arguments.isEmpty() && target == 0) {
			creature->sendSystemMessage("Syntax: /grantBadge [badge id]");
			creature->sendSystemMessage("Syntax: /grantBadge [-area [range]] [badge id]");
			creature->sendSystemMessage("Syntax: /grantBadge [-multiple] [start badge] [end badge]");
			creature->sendSystemMessage("Syntax: /grantBadge -player <name> [badge id]");
			creature->sendSystemMessage("Syntax: /grantBadge -player <name> -multiple [start badge] [end badge]");
			return GENERALERROR;
		}

		auto zoneServer = server->getZoneServer();
		if (zoneServer == nullptr)
			return GENERALERROR;

		ManagedReference<SceneObject*> targetObject = zoneServer->getObject(target);

		bool area = false;
		bool multiple = false;
		int range = 32;  //Infinity: Default range to 32 meters
		int badgeId = 0;
		int endBadge = 0;
		String playerName = ""; // Infinity: -player option captures the name here

		try {

			StringTokenizer args(arguments.toString());

			while (args.hasMoreTokens()) {

				String arg;
				args.getStringToken(arg);

				if (arg.charAt(0) != '-') {
					// Positional bare badge id — end of flags.
					badgeId = Integer::valueOf(arg);
					break;
				}

				String lower = arg.toLowerCase();

				if (lower == "-help" || lower == "-h") {
					creature->sendSystemMessage("Syntax: /grantBadge [badge id]");
					creature->sendSystemMessage("Syntax: /grantBadge [-area [range]] [badge id]");
					creature->sendSystemMessage("Syntax: /grantBadge [-multiple] [start badge] [end badge]");
					creature->sendSystemMessage("Syntax: /grantBadge -player <name> [badge id]");
					creature->sendSystemMessage("Syntax: /grantBadge -player <name> -multiple [start badge] [end badge]");
					return GENERALERROR;
				}
				else if (lower == "-player" || lower == "-p") {
					// Infinity: name-based target lookup. Eats next token as the
					// player first name. Resolves later via ChatManager so the
					// target doesn't need to be in-game-targeted.
					if (!args.hasMoreTokens()) {
						creature->sendSystemMessage("Syntax: /grantBadge -player <name> [badge id]");
						return INVALIDPARAMETERS;
					}
					args.getStringToken(playerName);
					// continue loop to parse remaining flags (e.g. -multiple) or
					// a positional badge id
				}
				else if (lower == "-area" || lower == "-a") {
					area = true;

					if (!args.hasMoreTokens()) {
						creature->sendSystemMessage("Syntax: /grantBadge [-area [range]] [badge id]");
						return GENERALERROR;
					}
					badgeId = args.getIntToken();

					if (args.hasMoreTokens()) {
						range = badgeId;
						badgeId = args.getIntToken();

						if (range <= 0 || range > 192) {
							creature->sendSystemMessage("Invalid range, must be between 1 and 192.");
							return INVALIDPARAMETERS;
						}
					}
					// -area consumes all remaining tokens — break.
					break;
				}
				else if (lower == "-multiple" || lower == "-m") {
					multiple = true;

					if (!args.hasMoreTokens()) {
						creature->sendSystemMessage("Syntax: /grantBadge [-multiple] [start badge] [end badge]");
						return GENERALERROR;
					}
					badgeId = args.getIntToken();

					if (!args.hasMoreTokens()) {
						creature->sendSystemMessage("Syntax: /grantBadge [-multiple] [start badge] [end badge]");
						return GENERALERROR;
					}
					endBadge = args.getIntToken();

					if (endBadge <= 0 || endBadge < badgeId) {
						creature->sendSystemMessage("End badge must be greater than 0 and greater than the start badge.");
						return INVALIDPARAMETERS;
					}
					// -multiple consumes tokens — break.
					break;
				}
				else {
					creature->sendSystemMessage("Invalid option " + arg);
					return INVALIDPARAMETERS;
				}
			}
		}
 		catch (Exception& e) {
				creature->sendSystemMessage("Syntax: /grantBadge [badge id]");
				creature->sendSystemMessage("Syntax: /grantBadge [-area [range]] [badge id]");
				creature->sendSystemMessage("Syntax: /grantBadge [-multiple] [start badge] [end badge]");
				creature->sendSystemMessage("Syntax: /grantBadge -player <name> [badge id]");
				creature->sendSystemMessage(e.getMessage());
				return GENERALERROR;
		}

		// Infinity: -player and -area are mutually exclusive. -area sweeps the
		// caller's range, -player resolves a single named target. They have
		// nothing useful to do together.
		if (!playerName.isEmpty() && area) {
			creature->sendSystemMessage("Cannot combine -player with -area.");
			return INVALIDPARAMETERS;
		}

		// Infinity: if -player was supplied, resolve the named player via
		// ChatManager (online-only lookup). Replaces the in-game-target
		// requirement for the single-target paths below.
		if (!playerName.isEmpty()) {
			ManagedReference<ChatManager*> chatMgr = zoneServer->getChatManager();
			if (chatMgr == nullptr) {
				creature->sendSystemMessage("Unable to look up player (chat manager unavailable).");
				return GENERALERROR;
			}
			ManagedReference<CreatureObject*> named = chatMgr->getPlayer(playerName);
			if (named == nullptr) {
				creature->sendSystemMessage("Player '" + playerName + "' not found or not online.");
				return INVALIDTARGET;
			}
			targetObject = named.get();
		}

		if (area) {
			SortedVector<TreeEntry*> closeObjects;
			Zone* zone = creature->getZone();
			if (zone == nullptr)
				return GENERALERROR;

			if (creature->getCloseObjects() == nullptr) {
#ifdef COV_DEBUG
				creature->info("Null closeobjects vector in GrantBadgeCommand::doQueueCommand", true);
#endif
				zone->getInRangeObjects(creature->getPositionX(), 0, creature->getPositionY(), range, &closeObjects, true);
			}
			else {
				CloseObjectsVector* closeVector = (CloseObjectsVector*) creature->getCloseObjects();
				closeVector->safeCopyReceiversTo(closeObjects, CloseObjectsVector::PLAYERTYPE);
			}

			int numGranted = 0;

			for (int i = 0; i < closeObjects.size(); i++) {
	
				ManagedReference<SceneObject*> sceno = cast<SceneObject*>(closeObjects.get(i));
				if (sceno == nullptr || !sceno->isPlayerCreature())
					continue;

				ManagedReference<CreatureObject*> player = sceno->asCreatureObject();
				if (player == nullptr || player == creature)
					continue;

				ManagedReference<PlayerObject*> ghost = player->getPlayerObject();
				if (ghost == nullptr)
					continue;

				Locker crossLocker(ghost, creature);
				ghost->awardBadge(badgeId);
				numGranted++;
			}
			creature->sendSystemMessage("Granted badge #" + String::valueOf(badgeId) + " to " + String::valueOf(numGranted) + " players found within " + String::valueOf(range) + "m.");
			return SUCCESS;

		} else {
			if (targetObject == nullptr || !targetObject->isPlayerCreature()) {
					creature->sendSystemMessage("Invalid target.");
					return INVALIDTARGET;
			}
			else {
				ManagedReference<CreatureObject*> player = targetObject->asCreatureObject();
				if (player == nullptr)
					return GENERALERROR;

				ManagedReference<PlayerObject*> ghost = player->getPlayerObject();
				if (ghost == nullptr)
					return GENERALERROR;				

				Locker crossLocker(ghost, creature);

				if (multiple) {
					for (int i = (int)badgeId; i <= endBadge; i++) {
						ghost->awardBadge(i);
					}
					creature->sendSystemMessage("Granted badges from #" + String::valueOf(badgeId) + " to #" + String::valueOf(endBadge) + " to " + player->getDisplayedName() + ".");
				} else {
					ghost->awardBadge(badgeId);
					creature->sendSystemMessage("Granted badge #" + String::valueOf(badgeId) + " to " + player->getDisplayedName() + ".");
				}			
			}
		}

		return SUCCESS;
	}

};

#endif //GRANTBADGECOMMAND_H_
