//PLAYER

#ifndef PLAYER_BH_SUI_CALLBACK
#define PLAYER_BH_SUI_CALLBACK

#include "server/zone/objects/player/sui/SuiCallback.h"
#include "server/zone/managers/visibility/VisibilityManager.h"
#include "server/zone/objects/player/sui/callbacks/BountyHuntSuiCallback.h"
#include "server/zone/objects/player/sui/inputbox/SuiInputBox.h"
#include "server/zone/managers/mission/MissionManager.h"


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
			CreatureObject* player = cast<CreatureObject*>(suiObject.get());
			if (player == nullptr)
				return;
			ManagedReference<PlayerObject*> playerGhost = player->getPlayerObject();
			if (playerGhost == nullptr)
				return;
			MissionManager* missionManager = creature->getZoneServer()->getMissionManager();
			if (missionManager == nullptr)
				return;

			uint64 id = player->getObjectID();
			int currentBounty = missionManager->getPlayerBounty(id);

			newBounty = currentBounty + (value * (1.0f - guildFee)); // BH guild commission
						
			if (creature->getBankCredits() + creature->getCashCredits() >= value) {
				if (creature->getBankCredits() > value) {
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
			
				player->playEffect("clienteffect/ui_missile_aquiring.cef", "");
				creature->playEffect("clienteffect/holoemote_haunted.cef", "head");
				String playerName = player->getFirstName();
				creature->sendSystemMessage("Your bounty has been placed successfully!  The bounty for " + playerName + " is now " + String::valueOf(newBounty) + " credits.");	

				if (!(playerGhost->getVisibility() >= VisibilityManager::instance()->getTerminalVisThreshold()) || currentBounty == 0) { // Only want to add to bounty list if not already there
					VisibilityManager::instance()->addPlayerToBountyList(player, newBounty);
				}
				missionManager->updatePlayerBountyReward(id, newBounty);
				VisibilityManager::instance()->setVisibility(player, 8000);
			
				if (refund > 0){	
					int fullRefund = refund / (1.0f - guildFee);
					creature->sendSystemMessage("Refunding excess bounty of " + String::valueOf(fullRefund) + ".");		
					creature->addBankCredits(fullRefund);
				}

				//Broadcast to Server
				StringBuffer zBroadcast;
				zBroadcast << "\\#66B3FF" << "[Bounty Hunter Guild Alert] " << "\\#FFFFFF" << "The Guild has acquired a new target.";
				creature->getZoneServer()->getChatManager()->broadcastGalaxy(nullptr, zBroadcast.toString());
			}
			else 
				creature->sendSystemMessage("You have insufficient funds! You need at least 25,000 credits in cash or bank.");
		} 
		
		catch(Exception& e) { }
	}
};

#endif
