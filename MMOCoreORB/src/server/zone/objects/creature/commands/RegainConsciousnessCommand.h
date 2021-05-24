/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef REGAINCONSCIOUSNESSCOMMAND_H_
#define REGAINCONSCIOUSNESSCOMMAND_H_

#include "server/zone/ZoneServer.h"
#include "server/zone/managers/player/PlayerManager.h"
#include "server/zone/objects/creature/buffs/PrivateBuff.h"
#include "server/zone/objects/creature/buffs/PrivateSkillMultiplierBuff.h"

#include "JediQueueCommand.h"

class RegainConsciousnessCommand : public JediQueueCommand {
public:

	RegainConsciousnessCommand(const String& name, ZoneProcessServer* server)
	: JediQueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {
		int buffAmount = 2500;
		int duration = 1800;
		int cooldown = 3600000; //60 minutes 
		// They should be dead...
		if (creature->isDead()){

			int res = doCommonJediSelfChecks(creature);
			if(res != SUCCESS)
				return res;

			ManagedReference<PlayerObject*> playerObject = creature->getPlayerObject();

			if (playerObject == nullptr)
				return false;

			if (!creature->checkCooldownRecovery("regain"))  {
				Time* cooldownTime = creature->getCooldownTime("regain");
				if (cooldownTime != nullptr) {
					float timeLeft = round(fabs(cooldownTime->miliDifference() / 1000.f / 60.0f));
					creature->sendSystemMessage("You can use the regainConsciousness command again in " + String::valueOf(timeLeft) + " minute" + ((timeLeft == 1.0f) ? "." : "s."));
					return GENERALERROR;
				}
				else {
					creature->sendSystemMessage("You can only use the regainConsciousness command once per hour.");
                	return GENERALERROR;
				}
			}

			//close clone window
			playerObject->removeSuiBoxType(SuiWindowType::CLONE_REQUEST);

			// Revive user by setting posture to standing.

			creature->removeFeignedDeath();

			creature->setPosture(CreaturePosture::UPRIGHT);

			if(creature->getHAM(CreatureAttribute::HEALTH) <= 0) {
				creature->setHAM(CreatureAttribute::HEALTH, 500, true);
			}

			if(creature->getHAM(CreatureAttribute::ACTION) <= 0) {
				creature->setHAM(CreatureAttribute::ACTION, 500, true);
			}

			if(creature->getHAM(CreatureAttribute::MIND) <= 0) {
				creature->setHAM(CreatureAttribute::MIND, 500, true);
			}

			PlayerManager* pm = server->getZoneServer()->getPlayerManager();
			pm->regainEnhanceCharacter(creature,buffAmount,duration);
	
			
			int maxForce = playerObject->getForcePowerMax();
			int currentForce = playerObject->getForcePower();
			int lowForce = maxForce * 0.25;

			if (currentForce < lowForce)	
				playerObject->setForcePower(lowForce);


			creature->updateCooldownTimer("regain", cooldown);
			return SUCCESS;
		}

		return GENERALERROR;

	}

};

#endif //REGAINCONSCIOUSNESSCOMMAND_H_
