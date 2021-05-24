/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef FORCEBREACHCOMMAND_H_
#define FORCEBREACHCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "ForcePowersQueueCommand.h"


class ForceBreachCommand : public ForcePowersQueueCommand {
public:

	ForceBreachCommand(const String& name, ZoneProcessServer* server)
		: ForcePowersQueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		if (isWearingArmor(creature)) {
			return NOJEDIARMOR;
		}

		ManagedReference<SceneObject*> targetObject = server->getZoneServer()->getObject(target);

		if (targetObject == NULL || !targetObject->isCreatureObject()) {
			return INVALIDTARGET;
		}

		CreatureObject *tarCreo = targetObject->asCreatureObject();

		int res = doCombatAction(creature, target);
		
		if (!tarCreo->checkCooldownRecovery("breach")){
		creature->sendSystemMessage("This targets defenses have been breached too recently!");
		return GENERALERROR;
		}
	
		if(res == SUCCESS) {

			ManagedReference<CreatureObject*> creatureTarget = targetObject.castTo<CreatureObject*>();

			if (creatureTarget != NULL) {
				Locker clocker(creatureTarget, creature);
				ManagedReference<Buff*> buff = NULL;
				int duration = 10;
				duration *= creature->getFrsMod("control");
				if (!creatureTarget->isPlayerCreature())
					duration *= 2;
				buff = new Buff(creatureTarget, BuffCRC::JEDI_FORCEBREACH, duration, BuffType::JEDI);

				Locker locker(buff);

				if (creatureTarget->hasBuff(BuffCRC::JEDI_FORCEBREACH)){
					creature->sendSystemMessage("Their defeneses are already breached!");
					return res;
				}

				if (creatureTarget->getSkillMod("dizzy_defense") > 14)
					buff->setSkillModifier("dizzy_defense",-15);
				if (creatureTarget->getSkillMod("stun_defense") > 14)
					buff->setSkillModifier("stun_defense",-15);
				if (creatureTarget->getSkillMod("blind_defense") > 14)
					buff->setSkillModifier("blind_defense",-15);
				if (creatureTarget->getSkillMod("knockdown_defense") > 14)
					buff->setSkillModifier("knockdown_defense",-15);
				if (creatureTarget->getSkillMod("jedi_state_defense") > 14)
					buff->setSkillModifier("jedi_state_defense",-15);
				int fDefense = creatureTarget->getSkillMod("force_defense");
					fDefense *= .45;
					fDefense *= -1;
				buff->setSkillModifier("force_defense",fDefense); //now breaches 45% force defense 

				creatureTarget->addBuff(buff);
				creatureTarget->playEffect("clienteffect/pl_force_resist_bleeding_self.cef", "");
				creatureTarget->playEffect("clienteffect/general_greivous_gutsack.cef", "");
				CombatManager::instance()->broadcastCombatSpam(creature, creatureTarget, NULL, 0, "cbt_spam", combatSpam + "_hit", 1);
				tarCreo->updateCooldownTimer("breach", 45000); // 45 second cooldown.
			}
		}

		return res;
	}
	float getCommandDuration(CreatureObject* object, const UnicodeString& arguments) const {
		return defaultTime * 2;
	}

};

#endif //FORCEBREACHCOMMAND_H_
