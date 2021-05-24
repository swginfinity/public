/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef TAUNTCOMMAND_H_
#define TAUNTCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"

class TauntCommand : public CombatQueueCommand {
public:

	TauntCommand(const String& name, ZoneProcessServer* server)
		: CombatQueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		ManagedReference<SceneObject*> targetObject = creature->getZoneServer()->getObject(target);

		if (targetObject == nullptr || !targetObject->isCreatureObject() || targetObject->isPlayerCreature())
			return INVALIDTARGET;

		CreatureObject* targetCreature = cast<CreatureObject*>(targetObject.get());

		if (targetCreature == nullptr)
			return INVALIDTARGET;

		if (!targetCreature->isAttackableBy(creature))
			return INVALIDTARGET;

		if (!creature->checkCooldownRecovery("taunt"))  {
			Time* cooldownTime = creature->getCooldownTime("taunt");
			if (cooldownTime != nullptr) {
				float timeLeft = round(fabs(cooldownTime->miliDifference() / 100.f)) / 10.f;
				creature->sendSystemMessage("You can use the taunt command again in " + String::valueOf(timeLeft) + " second" + ((timeLeft == 1.0f) ? "." : "s."));
				return GENERALERROR;
			} 
			else {
				creature->sendSystemMessage("You can only use the taunt command once every 3 seconds.");
                return GENERALERROR;
			}
		}			

		int tauntSkill = creature->getSkillMod("taunt");
		int cooldown = 3000;
		creature->updateCooldownTimer("taunt", cooldown); // 3 seconds			

		int res = doCombatAction(creature, target);

		if (res == SUCCESS) {
			Locker clocker(targetCreature, creature);

			targetCreature->getThreatMap()->addAggro(creature, tauntSkill, 0);
			targetCreature->getThreatMap()->setThreatState(creature, ThreatStates::TAUNTED, (uint64)ceil(tauntSkill / 5.0  * 1000));   // No cooldown
			creature->doAnimation("taunt");

			if (creature->isPlayerCreature())
				creature->sendSystemMessage("@cbt_spam:taunt_success_single");

		} else {

			if (creature->isPlayerCreature())
				creature->sendSystemMessage("@cbt_spam:taunt_fail_single");
		}

		return res;
	}

	void sendAttackCombatSpam(TangibleObject* attacker, TangibleObject* defender, int attackResult, int damage, const CreatureAttackData& data) const {
		if (attacker == nullptr)
			return;

		Zone* zone = attacker->getZone();
		if (zone == nullptr)
			return;

		String stringName = data.getCombatSpam();
		byte color = 1;

		switch (attackResult) {
		case CombatManager::HIT:
			stringName += "_success";
			break;
		case CombatManager::MISS:
		case CombatManager::DODGE:
		case CombatManager::COUNTER:
		case CombatManager::BLOCK:
		case CombatManager::RICOCHET:
			stringName += "_fail";
			color = 0;
			break;
		default:
			break;
		}

		CombatManager::instance()->broadcastCombatSpam(attacker, NULL, NULL, damage, "cbt_spam", stringName, color);

	}

};

#endif //TAUNTCOMMAND_H_
