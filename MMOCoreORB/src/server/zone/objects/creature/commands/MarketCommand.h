/*
 * VendorInfoCommand.h
 *
 *  Created on: Apr 6, 2011
 *      Author: polonel
 */

#ifndef MARKETCOMMAND_H_
#define MARKETCOMMAND_H_

#include "engine/engine.h"
#include "server/zone/managers/auction/AuctionManager.h"

class MarketCommand {
public:
	static int executeCommand(CreatureObject* creature, uint64 target, const UnicodeString& arguments) {
		if (creature == nullptr || !creature->isPlayerCreature())
			return 1;

		ManagedReference<PlayerObject*> ghost = creature->getPlayerObject();

		if (ghost == nullptr || ghost->getAdminLevel() < 15) {
			return 1;
		}

		ManagedReference<AuctionManager*> auctionManager = creature->getZoneServer()->getAuctionManager();
		// Parse the weather command.

		StringTokenizer tokenizer(arguments.toString());
		if (!tokenizer.hasMoreTokens()) {
			sendSyntax(creature);
			return 1;
		}

		String command;
		tokenizer.getStringToken(command);
		command = command.toLowerCase();

		if (command == "enable") {
			auctionManager->setMarketEnabled(true);
			creature->sendSystemMessage("Market Enabled");
		} else if (command == "disable") {
			auctionManager->setMarketEnabled(false);
			creature->sendSystemMessage("Market Disabled");
		} else if (command == "info") {
			auctionManager->displayInfo(creature);
		} else if (command == "stockroom") {
			if (!tokenizer.hasMoreTokens()) {
				creature->sendSystemMessage("Syntax: /server market stockroom <playerName>");
				return 1;
			}
			String targetName;
			tokenizer.getStringToken(targetName);
			auctionManager->displayPlayerStockroom(creature, targetName);
		}

		return 0;
	}

	static void sendSyntax(CreatureObject* player) {
		if (player != nullptr)
			player->sendSystemMessage("Syntax: /server market [enable/disable/info/stockroom <playerName>]");
	}
};

#endif /* MARKETCOMMAND_H_ */
