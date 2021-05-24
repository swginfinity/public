/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef AREATAUNTCOMMAND_H_
#define AREATAUNTCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/objects/tangible/threat/ThreatStates.h"

class AreaTauntCommand : public CombatQueueCommand {
public:

	AreaTauntCommand(const String& name, ZoneProcessServer* server)
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
			
		if (!creature->checkCooldownRecovery("areaTaunt"))  {
			Time* cooldownTime = creature->getCooldownTime("areaTaunt");
			if (cooldownTime != nullptr) {
				float timeLeft = round(fabs(cooldownTime->miliDifference() / 100.f)) / 10.f;
				creature->sendSystemMessage("You can use the areaTaunt command again in " + String::valueOf(timeLeft) + " second" + ((timeLeft == 1.0f) ? "." : "s."));
				return GENERALERROR;
			}
			else {
				creature->sendSystemMessage("You can only use the areaTaunt command once every 10 seconds.");
                return GENERALERROR;
			}
		}	

		int tauntSkill = creature->getSkillMod("taunt");
		int cooldown = 10000;  // 10 seconds
		creature->updateCooldownTimer("areaTaunt", cooldown); 		

		Locker clocker(targetCreature, creature);
		ThreatMap* threatMap = targetCreature->getThreatMap();
				threatMap->addAggro(creature, tauntSkill, 0);
				threatMap->setThreatState(creature, ThreatStates::TAUNTED, (uint64)ceil(tauntSkill / 5.0  * 1000));  // No cooldown

			SortedVector<QuadTreeEntry*> closeObjects;
			CloseObjectsVector* vec = (CloseObjectsVector*) targetCreature->getCloseObjects();
			vec->safeCopyTo(closeObjects);
			if (creature->isPlayerCreature())
				creature->sendSystemMessage("@cbt_spam:taunt_success_single");
			
		if(closeObjects.size() != 0) {
			creature->doAnimation("taunt");
			int res = doCombatAction(creature, target);
			for (int i = 0;i<closeObjects.size();i++ ){
				SceneObject* object = static_cast<SceneObject*>( closeObjects.get(i));
				
				if (!object->isCreatureObject())
					continue;

				if (object == creature || object == targetCreature)
					continue;

				if (creature->getWorldPosition().distanceTo(object->getWorldPosition()) - object->getTemplateRadius() > 30)
					continue;
					
				CreatureObject* targetNext = cast<CreatureObject*>( object);		

				if (targetNext == nullptr)
					continue;

				if (!targetNext->isAttackableBy(creature))
					continue;
					
				Locker clocker(targetNext, creature);
					
				ThreatMap* threatMap = targetNext->getThreatMap();
				threatMap->addAggro(creature, tauntSkill, 0);
				targetCreature->getThreatMap()->setThreatState(creature, ThreatStates::TAUNTED, (uint64)ceil(tauntSkill / 5.0  * 1000));  // No cooldown
			}
		}
		return SUCCESS;
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

#endif //AREATAUNTCOMMAND_H_
