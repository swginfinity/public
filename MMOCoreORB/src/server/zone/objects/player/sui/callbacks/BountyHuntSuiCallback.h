//PLAYER

#ifndef PLAYER_BH_SUI_CALLBACK
#define PLAYER_BH_SUI_CALLBACK

#include "server/zone/objects/player/sui/SuiCallback.h"
#include "server/zone/managers/visibility/VisibilityManager.h"
#include "server/zone/objects/player/sui/callbacks/BountyHuntSuiCallback.h"
#include "server/zone/objects/player/sui/inputbox/SuiInputBox.h"
#include "server/zone/managers/mission/MissionManager.h"
#include "server/zone/packets/chat/ChatRoomMessage.h"


class BountyHuntSuiCallback : public SuiCallback {

public:
	BountyHuntSuiCallback(ZoneServer* serv) : SuiCallback(serv) { }

	void run(CreatureObject* creature, SuiBox* suiBox, uint32 eventIndex, Vector<UnicodeString>* args) {
		bool cancelPressed = (eventIndex == 1);
		int newBounty = 0;
		int refund = 0;
		int currentBounty = 0;
		int minBounty = 25000;
		int maxBounty = 2500000;
		int maxTotalBounty = 2500000;
		float guildFee = 0.2f;
		
		if (creature == nullptr || cancelPressed)
			return;

		try {
			int value = Integer::valueOf(args->get(0).toString());

			if (value == 0)
				return;

			if (value <= minBounty)
				value = minBounty;
			else if (value >= maxBounty)
				value = maxBounty;
				
			ManagedReference<SceneObject*> suiObject = suiBox->getUsingObject();
			if (suiObject == nullptr)
				return;

			auto ghost = creature->getPlayerObject();
			if (ghost == nullptr)
				return;

			String killerName = ghost->getScreenPlayData("pvp", "last_killer");

			auto playerManager = server->getPlayerManager();
			if (playerManager == nullptr)
				return;

			ManagedReference<CreatureObject*> killerCreature = playerManager->getPlayer(killerName);
			if (killerCreature == nullptr)
				return;

			auto killerGhost = killerCreature->getPlayerObject();
			if (killerGhost == nullptr)
				return;

			auto zoneServer = creature->getZoneServer();
			if (zoneServer == nullptr)
				return;

			auto missionManager = zoneServer->getMissionManager();
			if (missionManager == nullptr)
				return;

			VisibilityManager* visibilityManager = VisibilityManager::instance();
			if (visibilityManager == nullptr)
				return;

			uint64 oid = killerCreature->getObjectID();
			int currentBounty = missionManager->getPlayerBounty(oid);

			newBounty = currentBounty + (value * (1.0f - guildFee)); // BH guild commission
						
			if (creature->getBankCredits() + creature->getCashCredits() >= value) {
				if (creature->getBankCredits() >= value) {
					creature->subtractBankCredits(value);
				} 
				else {
					creature->subtractCashCredits(value - creature->getBankCredits());
					creature->subtractBankCredits(creature->getBankCredits());
				}

				if (newBounty >= maxTotalBounty) {
					refund = newBounty - maxTotalBounty;
					newBounty = maxTotalBounty;
				}
			
				killerCreature->playEffect("clienteffect/ui_missile_aquiring.cef", "");
				creature->playEffect("clienteffect/holoemote_haunted.cef", "head");
				String playerName = killerCreature->getFirstName();
				creature->sendSystemMessage("Your bounty has been placed successfully!  The bounty for " + playerName + " is now " + String::valueOf(newBounty) + " credits.");	

				if (!missionManager->hasPlayerBountyTargetInList(oid))
					missionManager->addPlayerToBountyList(oid, newBounty);
				else {
					missionManager->updatePlayerBountyReward(oid, newBounty);
				}

				if (killerGhost->isOnline()) { //Infinity:  Set online status in bounty list if online
					missionManager->updatePlayerBountyOnlineStatus(oid, true);	
				}

				Locker locker(killerGhost);
				killerGhost->setVisibility(8000);
				locker.release();

				if (refund > 0){	
					int fullRefund = refund / (1.0f - guildFee);
					creature->sendSystemMessage("Refunding excess bounty of " + String::valueOf(fullRefund) + ".");		
					creature->addBankCredits(fullRefund);
				}

				if (ConfigManager::instance()->isPvpBroadcastChannelEnabled()) {

					auto chatManager = zoneServer->getChatManager();
					if (chatManager != nullptr) {

						StringBuffer zBroadcast;
						zBroadcast << "\r\\#FFA500" << "Bounty Hunter Guild Alert:" << "\r\\#FFFFFF" << "  The Guild has acquired a new target.";

						auto pvpBroadcastRoom = chatManager->getPvpBroadcastRoom();
						if (pvpBroadcastRoom != nullptr) {
							UnicodeString message(zBroadcast.toString());
							UnicodeString formattedMsg(chatManager->formatMessage(message));
							BaseMessage* msg = new ChatRoomMessage("", zoneServer->getGalaxyName(), formattedMsg, pvpBroadcastRoom->getRoomID());
							pvpBroadcastRoom->broadcastMessage(msg);
						}
					}
				}

				Time currentTime;
				uint64 currentTimeMili = currentTime.getMiliTime();
				ghost->setScreenPlayData("pvp", "bounty_placed_timestamp", String::valueOf(currentTimeMili));
			}
			else 
				creature->sendSystemMessage("You have insufficient funds! You need at least 25,000 credits in cash or bank.");
		} 
		
		catch(Exception& e) { }
	}
};

#endif
