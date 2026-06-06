/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef TAUNTCOMMAND_H_
#define TAUNTCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/objects/tangible/threat/ThreatMap.h"

class TauntCommand : public CombatQueueCommand {
public:
	TauntCommand(const String& name, ZoneProcessServer* server) : CombatQueueCommand(name, server) {
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

		AiAgent* agent = targetCreature->asAiAgent();

		if (agent == nullptr)
			return INVALIDTARGET;

		if (!agent->isTauntable()) {
			if (creature->isPlayerCreature()) {
				creature->sendSystemMessage("@cbt_spam:taunt_fail_single");
			}
			return INVALIDTARGET;
		}

		if (!creature->checkCooldownRecovery("taunt")) {
			const Time* cooldownTime = creature->getCooldownTime("taunt");
			if (cooldownTime != nullptr) {
				float timeLeft = round(fabs(cooldownTime->miliDifference() / 100.f)) / 10.f;
				creature->sendSystemMessage("You can use the taunt command again in " + String::valueOf(timeLeft) + " second" + ((timeLeft == 1.0f) ? "." : "s."));
				return GENERALERROR;
			} else {
				creature->sendSystemMessage("You can only use the taunt command once every 3 seconds.");
				return GENERALERROR;
			}
		}

		int res = doCombatAction(creature, target);

		if (res == SUCCESS) {

			if (creature->isPlayerCreature()) {
				creature->updateCooldownTimer("taunt", 3 * 1000);
			}

			Locker clocker(targetCreature, creature);

			ThreatMap* threatMap = targetCreature->getThreatMap();

			if (threatMap != nullptr) {
				int tauntSkill = creature->getSkillMod("taunt");

				// Success chance: skill-based with CL bonus for weaker mobs
				// Master+tape (115) vs CL500: 90%   Master (90) vs CL500: 70%
				// Master+tape (115) vs CL85:  95%   Master (90) vs CL85:  81%
				// 3s cooldown allows retry on failure
				int mobLevel = targetCreature->getLevel();
				int baseChance = (int)(tauntSkill * 0.783f);  // 90% at skill 115 (master+tape), 70% at 90
				int clScale = Math::clamp(300 - mobLevel, 0, 300);
				int clBonus = (int)(clScale * tauntSkill / 1800.0f);
				int successChance = Math::clamp(baseChance + clBonus, 5, 95);

				if (System::random(99) < successChance) {
					threatMap->addAggro(creature, tauntSkill, 0);
					threatMap->setThreatState(creature, ThreatStates::TAUNTED, (uint64)ceil(tauntSkill / 5.0 * 1000));
					creature->doAnimation("taunt");

					if (creature->isPlayerCreature())
						creature->sendSystemMessage("@cbt_spam:taunt_success_single");
				} else {
					if (creature->isPlayerCreature())
						creature->sendSystemMessage("@cbt_spam:taunt_fail_single");
				}
			}
		} else {
			if (creature->isPlayerCreature())
				creature->sendSystemMessage("@cbt_spam:taunt_fail_single");
		}

		return res;
	}
};

#endif // TAUNTCOMMAND_H_
