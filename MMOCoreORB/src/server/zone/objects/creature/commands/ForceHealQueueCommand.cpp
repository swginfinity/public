/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#include "ForceHealQueueCommand.h"
#include "server/zone/managers/combat/CombatManager.h"
#include "templates/params/creature/CreatureAttribute.h"
#include "server/zone/managers/stringid/StringIdManager.h"
#include "server/zone/managers/collision/CollisionManager.h"
#include "server/zone/managers/frs/FrsManager.h"
#include "server/zone/objects/player/FactionStatus.h"
//Infinity: Custom includes
#include "server/zone/Zone.h"

ForceHealQueueCommand::ForceHealQueueCommand(const String& name, ZoneProcessServer* server) : JediQueueCommand(name, server) {
	speed = 1;
	allowedTarget = TARGET_AUTO;

	forceCost = 0;
	forceCostMultiplier = 0;

	statesToHeal = 0;
	healStateCost = 0;

	healBleedingCost = 0;
	healPoisonCost = 0;
	healDiseaseCost = 0;
	healFireCost = 0;

	attributesToHeal = 0;
	woundAttributesToHeal = 0;

	healBattleFatigue = 0;
	healAmount = 0;
	healWoundAmount = 0;

	bleedHealIterations = 1;
	poisonHealIterations = 1;
	diseaseHealIterations = 1;
	fireHealIterations = 1;

	visMod = 10;

	range = 0;

	clientEffect = "clienteffect/pl_force_heal_self.cef";
	animationCRC = STRING_HASHCODE("force_healing_1");
}

int ForceHealQueueCommand::runCommand(CreatureObject* creature, CreatureObject* targetCreature) const {

	if (creature == nullptr || targetCreature == nullptr)
		return GENERALERROR;

	auto playerObject = creature->getPlayerObject();

	if (playerObject == nullptr)
		return GENERALERROR;

	int currentForce = playerObject->getForcePower();
	int totalCost = forceCost;
	bool healPerformed = false;

	//Infinity:  Custom variables
	bool HAMhealPerformed = false;
	float frsManipulationBonus = creature->getFrsMod("manipulation");
	float frsPowerBonus = creature->getFrsMod("power");
	float frsControlBonus = creature->getFrsMod("control");
	CombatManager* combatManager = CombatManager::instance();

	if (combatManager == nullptr) {
		creature->sendSystemMessage("Combat Manager could not be found.");
		return GENERALERROR;
	}

	// Attribute Wound Healing
	for (int i = 0; i < 3; i++) {
		// Attrib Values: Health = 1, Action = 2, Mind = 4
		if (woundAttributesToHeal & (1 << i)) {
			for (int j = 0; j < 3; j++) {
				if (totalCost < currentForce) {
					uint8 attrib = (i * 3) + j;
					int woundAmount = targetCreature->getWounds(attrib);

					if ((healWoundAmount * frsControlBonus) > 0 && woundAmount > (healWoundAmount * frsControlBonus))
						woundAmount = healWoundAmount * frsControlBonus;

					totalCost += woundAmount * forceCostMultiplier / frsManipulationBonus;

					if (totalCost > currentForce) {
						int forceDiff = totalCost - currentForce;
						totalCost -= forceDiff;
						woundAmount -= forceDiff / forceCostMultiplier;
					}

					if (woundAmount > 0) {
						targetCreature->healWound(creature, attrib, woundAmount, true);
						healPerformed = true;
						sendHealMessage(creature, targetCreature, HEAL_WOUNDS, attrib, woundAmount);
					}
				}
			}
		}
	}

	int healingWithFrs = -1;
	if (healAmount != -1)
		healingWithFrs = healAmount * frsControlBonus;

	// HAM Attribute Healing
	for (int i = 0; i < 3; i++) {
		// Attrib Values: Health = 1, Action = 2, Mind = 4
		if (attributesToHeal & (1 << i)) {
			if (totalCost < currentForce) {
				uint8 attrib = i * 3;
				int curHam = targetCreature->getHAM(attrib);
				int maxHam = targetCreature->getMaxHAM(attrib) - targetCreature->getWounds(attrib);
				int amtToHeal = maxHam - curHam;

				if (healingWithFrs > 0 && amtToHeal >= healingWithFrs)
					amtToHeal = healingWithFrs;

				totalCost += amtToHeal * forceCostMultiplier / frsManipulationBonus;

				if (totalCost > currentForce) {
					int forceDiff = totalCost - currentForce;
					totalCost -= forceDiff;
					amtToHeal -= forceDiff / forceCostMultiplier;
				}

				if (amtToHeal > 0) {
					targetCreature->healDamage(creature, attrib, amtToHeal, true);
					healPerformed = true;
					HAMhealPerformed = true;
					sendHealMessage(creature, targetCreature, HEAL_DAMAGE, attrib, amtToHeal);
				}
			}
		}
	}

	auto zone = targetCreature->getZone();

	if (zone != nullptr && HAMhealPerformed && creature->hasSkill("force_discipline_healing_master") && (creature->hasSkill("force_rank_light_novice") || creature->hasSkill("force_rank_dark_novice"))) {

		CloseObjectsVector* closeObjectsVector = targetCreature->getCloseObjects();
		SortedVector<ManagedReference<TreeEntry*> > closeObjects;

		if (closeObjectsVector != nullptr) {
			closeObjectsVector->safeCopyReceiversTo(closeObjects, CloseObjectsVector::CREOTYPE);
		} else {
			zone->getInRangeObjects(targetCreature->getWorldPositionX(), targetCreature->getWorldPositionZ(), targetCreature->getWorldPositionY(), 32, &closeObjects, true);
		}

		int iterations = 0;
		int chainHealing = 0;		// Chain heal is lesser of amount healed or 750 plus FRS bonus
		if (healAmount == -1)		// TotalHealSelf and TotalHealOther = -1
			chainHealing = 750 * frsControlBonus;
		else
			chainHealing = Math::min(750, healAmount) * frsControlBonus;

		for (int i = 0; i < closeObjects.size(); i++) {

			if (iterations > 3)  //Will heal up to 4 nearby allies (0-3)
				break;

			ManagedReference<SceneObject*> object = cast<SceneObject*>(closeObjects.get(i).get());

			if (object == nullptr)
				continue;

			if (object == targetCreature) 	//Can't chain to the heal target
				continue;

			if (!(object->isPlayerCreature() || object->isPet()))  //Needs to be player or pet
				continue;

			ManagedReference<CreatureObject*> chainCreatureTarget = cast<CreatureObject*>(object.get());

			if (chainCreatureTarget == nullptr)
				continue;

			if (targetCreature->isDead() || chainCreatureTarget->isDead() || creature->isDead())   // If healer, target, or nearby creature is dead, no chain heal
				continue;

			if (chainCreatureTarget->isWalkerSpecies() || chainCreatureTarget->isDroidSpecies())
				continue;

			if (chainCreatureTarget->isAttackableBy(creature) || !chainCreatureTarget->isHealableBy(creature))
				continue;

			if (closeObjectsVector != nullptr && !chainCreatureTarget->isInRange(targetCreature, 32.0f)) //Must be within 32 meters of heal target, only check if using COV
				continue;

			if (!chainCreatureTarget->isInRange(creature, 40.0f))   //Must be within 40 meters for the healer
				continue;

			if (!CollisionManager::checkLineOfSight(targetCreature, chainCreatureTarget))   //Must be in LOS
				continue;

			if (healAmount != 0) {  //make sure this is actually a heal skill

				bool chainHealed = false;

				for (int j = 0; j < 3; j++) {
					// Attrib Values: Health = 0, Action = 3, Mind = 6
					if (attributesToHeal & (1 << j)) {
						uint8 attrib = j * 3;
						int curHam = chainCreatureTarget->getHAM(attrib);
						int maxHam = chainCreatureTarget->getMaxHAM(attrib) - chainCreatureTarget->getWounds(attrib);
						int amtToHeal = maxHam - curHam;

						if (amtToHeal > chainHealing)
							amtToHeal = chainHealing;

						if (amtToHeal > 0) {
							healPerformed = true;
							chainHealed = true;
							sendHealMessage(creature, chainCreatureTarget, HEAL_DAMAGE, attrib, amtToHeal);

							Reference<CreatureObject*> chainCreatureTargetRef = chainCreatureTarget;
							Reference<CreatureObject*> creatureRef = creature;

							Core::getTaskManager()->executeTask([chainCreatureTargetRef, creatureRef, attrib, amtToHeal] () {

								if (chainCreatureTargetRef == nullptr || creatureRef == nullptr) {
									return;
								}

								Locker locker(chainCreatureTargetRef);
								chainCreatureTargetRef->healDamage(creatureRef, attrib, amtToHeal, true);
							}, "ChainHealLambdaTask");
						}
					}
				}

				if (chainHealed) { //If creature was healed, increase the iteration count
					iterations++;
					if (creature == targetCreature) { //Self-heal
						if (chainCreatureTarget->isPlayerCreature()) {
							chainCreatureTarget->sendSystemMessage("You have been chain healed by " + creature->getFirstName() + ".");
						}
						creature->sendSystemMessage("You have chain healed " + chainCreatureTarget->getFirstName());
					} else {
						if (creature != chainCreatureTarget) {
							if (chainCreatureTarget->isPlayerCreature()) {
								chainCreatureTarget->sendSystemMessage("You have been chain healed by " + creature->getFirstName() + ".");
							}
							creature->sendSystemMessage("You have chain healed " + chainCreatureTarget->getFirstName());
						} else {
							creature->sendSystemMessage("You have chain healed yourself.");
						}
					}
					targetCreature->doCombatAnimation(chainCreatureTarget, animationCRC, 0, 0xFF);
					checkForTef(creature, chainCreatureTarget);
				}
			}
		}
	}

	// Battle fatigue
	if (totalCost < currentForce && healBattleFatigue != 0) {
		int battleFatigue = targetCreature->getShockWounds();

		if (healBattleFatigue > 0 && battleFatigue > healBattleFatigue)
			battleFatigue = healBattleFatigue;

		totalCost += battleFatigue * forceCostMultiplier;

		if (totalCost > currentForce) {
			int forceDiff = totalCost - currentForce;
			totalCost -= forceDiff;
			battleFatigue -= forceDiff / forceCostMultiplier;
		}

		if (battleFatigue > 0) {
			targetCreature->addShockWounds(-battleFatigue, true, false);
			sendHealMessage(creature, targetCreature, HEAL_FATIGUE, 0, battleFatigue);
			healPerformed = true;
		}
	}

	// States - Stun, Blind, Dizzy, Intim
	if (totalCost < currentForce) {
		int totalStates = 0;
		int healedStates = 0;
		for (int i = 12; i <= 15; i++) {
			int state = (1 << i);

			if ((statesToHeal & state) && targetCreature->hasState(state)) {
				totalStates++;
				int newTotal = totalCost + healStateCost;

				if (newTotal < currentForce) {
					targetCreature->removeStateBuff(state);
					targetCreature->notifyObservers(ObserverEventType::HEALINGRECEIVED, creature, 0);
					totalCost = newTotal;
					healPerformed = true;
					healedStates++;
				}
			}
		}

		if (healedStates > 0 && healedStates == totalStates)
			sendHealMessage(creature, targetCreature, HEAL_STATES, 0, 0);
	}

	// Bleeding
	if (targetCreature->isBleeding() && healBleedingCost > 0 && totalCost + healBleedingCost < currentForce) {
		bool result = false;
		int iteration = 1;

		while (!result && (totalCost + healBleedingCost < currentForce) && (bleedHealIterations == -1 || iteration <= bleedHealIterations * frsControlBonus)) {
			result = targetCreature->healDot(CreatureState::BLEEDING, 500 * frsPowerBonus, false);
			totalCost += healBleedingCost / frsManipulationBonus;
			iteration++;
		}

		if (result) {
			sendHealMessage(creature, targetCreature, HEAL_BLEEDING, 0, 1);
		} else {
			sendHealMessage(creature, targetCreature, HEAL_BLEEDING, 0, 0);
		}

		healPerformed = true;
	}

	// Poison

	if (!creature->checkCooldownRecovery("forceCurePoison") && targetCreature->isPoisoned() && healPoisonCost > 0 && poisonHealIterations != -1) {

		const Time* cooldownTime = creature->getCooldownTime("forceCurePoison");
		if (cooldownTime != nullptr) {
			float timeLeft = fabs(cooldownTime->miliDifference()) / 1000.0f;
			creature->sendSystemMessage("You can use the forceCurePoison command again in " + String::format("%.1f", timeLeft) + " second" + ((timeLeft == 1.0f) ? "." : "s."));
			return GENERALERROR;
		}
		else {
			creature->sendSystemMessage("You are not ready to use forceCurePoison again.");
			return GENERALERROR;
		}
	}
	else if (targetCreature->isPoisoned() && healPoisonCost > 0 && totalCost + healPoisonCost < currentForce) {

		bool result = false;
		int iteration = 1;

		while (!result && (totalCost + healPoisonCost < currentForce) && (poisonHealIterations == -1 || iteration <= poisonHealIterations * frsControlBonus)) {
			result = targetCreature->healDot(CreatureState::POISONED, 500 * frsPowerBonus, false);
			totalCost += healPoisonCost / frsManipulationBonus;
			iteration++;
		}

		if (result) {
			sendHealMessage(creature, targetCreature, HEAL_POISON, 0, 1);
		} else {
			sendHealMessage(creature, targetCreature, HEAL_POISON, 0, 0);
		}

		if (poisonHealIterations != -1) {
			creature->updateCooldownTimer("forceCurePoison", 3000); // 3 seconds if not a totalHeal
		}

		healPerformed = true;
		targetCreature->notifyObservers(ObserverEventType::HEALINGRECEIVED, creature, 0);
	}

	// Disease
	if (!creature->checkCooldownRecovery("forceCureDisease") && targetCreature->isDiseased() && healDiseaseCost > 0 && diseaseHealIterations != -1) {

		const Time* cooldownTime = creature->getCooldownTime("forceCureDisease");
		if (cooldownTime != nullptr) {
			float timeLeft = fabs(cooldownTime->miliDifference()) / 1000.0f;
			creature->sendSystemMessage("You can use the forceCureDisease command again in " + String::format("%.1f", timeLeft) + " second" + ((timeLeft == 1.0f) ? "." : "s."));
			return GENERALERROR;
		} else {
			creature->sendSystemMessage("You are not ready to use forceCureDisease again.");
			return GENERALERROR;
		}
	}
	else if (targetCreature->isDiseased() && healDiseaseCost > 0 && totalCost + healDiseaseCost < currentForce) {

		bool result = false;
		int iteration = 1;

		while (!result && (totalCost + healDiseaseCost < currentForce) && (diseaseHealIterations == -1 || iteration <= diseaseHealIterations*frsControlBonus)) {
			result = targetCreature->healDot(CreatureState::DISEASED, 500 * frsPowerBonus, false);
			totalCost += healDiseaseCost / frsManipulationBonus;
			iteration++;
		}

		if (result) {
			sendHealMessage(creature, targetCreature, HEAL_DISEASE, 0, 1);
		} else {
			sendHealMessage(creature, targetCreature, HEAL_DISEASE, 0, 0);
		}
		if (diseaseHealIterations != -1) {
			creature->updateCooldownTimer("forceCureDisease", 3000); // 3 seconds if not a totalHeal
		}
		healPerformed = true;
		targetCreature->notifyObservers(ObserverEventType::HEALINGRECEIVED, creature, 0);
	}

	// Fire
	if (targetCreature->isOnFire() && healFireCost > 0 && totalCost + healFireCost < currentForce) {
		bool result = false;
		int iteration = 1;

		while (!result && (totalCost + healFireCost < currentForce) && (fireHealIterations == -1 || iteration <= fireHealIterations)) {
			result = targetCreature->healDot(CreatureState::ONFIRE, 500 * frsPowerBonus, false);
			totalCost += healFireCost;
			iteration++;
		}

		if (result) {
			sendHealMessage(creature, targetCreature, HEAL_FIRE, 0, 1);
		} else {
			sendHealMessage(creature, targetCreature, HEAL_FIRE, 0, 0);
		}

		healPerformed = true;
		targetCreature->notifyObservers(ObserverEventType::HEALINGRECEIVED, creature, 0);
	}

	bool selfHeal = creature == targetCreature;

	if (selfHeal && creature->hasSkill("force_discipline_healing_master") && healAmount == -1) { //Infinity:  Master healer performing totalHealSelf removes force debuffs

		//bool removedBreach = false;
		bool removedWeaken = false;

		/* Removed for balance
		if (creature->hasBuff(BuffCRC::JEDI_FORCEBREACH)) {
			creature->removeBuff(BuffCRC::JEDI_FORCEBREACH);
			removedBreach = true;
		}
		*/

		if (creature->hasBuff(BuffCRC::JEDI_FORCEWEAKEN1)) {
			creature->removeBuff(BuffCRC::JEDI_FORCEWEAKEN1);
			removedWeaken = true;
		}

		if (creature->hasBuff(BuffCRC::JEDI_FORCEWEAKEN2)) {
			creature->removeBuff(BuffCRC::JEDI_FORCEWEAKEN2);
			removedWeaken = true;
		}

		if (creature->hasBuff(BuffCRC::JEDI_FORCEWEAKEN3)) {
			creature->removeBuff(BuffCRC::JEDI_FORCEWEAKEN3);
			removedWeaken = true;
		}

		if (removedWeaken) {
			creature->healDamage(creature, CreatureAttribute::HEALTH, creature->getMaxHAM(CreatureAttribute::HEALTH) * 1.25, true);
			creature->healDamage(creature, CreatureAttribute::ACTION, creature->getMaxHAM(CreatureAttribute::ACTION) * 1.25, true);
			creature->healDamage(creature, CreatureAttribute::MIND, creature->getMaxHAM(CreatureAttribute::MIND) * 1.25, true);
			creature->sendSystemMessage("You have removed all Force Weaken debuffs affecting your character.");
			healPerformed = true;
		}

		/*
		if (removedBreach || removedWeaken) {
			creature->sendSystemMessage("You have removed all Force debuffs affecting your character.");
			healPerformed = true;
		}
		*/
	}

	if (healPerformed) {
		if (selfHeal)
			creature->playEffect(clientEffect, "");
		else {
			creature->doCombatAnimation(targetCreature, animationCRC, 0, 0xFF);
			checkForTef(creature, targetCreature);
			if (healAmount == -1) {
				combatManager->broadcastCombatSpam(creature, targetCreature, nullptr, 0, "jedi_spam", "totalhealother", 0);
			}
		}

		if (currentForce < totalCost) {
			playerObject->setForcePower(0);
			creature->error("Did not have enough force to pay for the healing he did. Total cost of command: " + String::valueOf(totalCost) + ", player's current force: " + String::valueOf(currentForce));
		} else {
			playerObject->setForcePower(currentForce - totalCost);
		}

		VisibilityManager::instance()->increaseVisibility(creature, visMod);

		return SUCCESS;
	} else {
		if (selfHeal) {
			creature->sendSystemMessage("@jedi_spam:no_damage_heal_self");
		} else {
			creature->sendSystemMessage("@jedi_spam:no_damage_heal_other");
		}

		return GENERALERROR;
	}
}

void ForceHealQueueCommand::sendHealMessage(CreatureObject* creature, CreatureObject* target, int healType, int healSpec, int amount) const {
	if (creature == nullptr || target == nullptr || amount < 0)
		return;

	uint64 playerID = creature->getObjectID();
	uint64 targetID = target->getObjectID();
	const bool selfHeal = playerID == targetID;

	if (healType == HEAL_DAMAGE || healType == HEAL_WOUNDS || healType == HEAL_FATIGUE) {
		String strVal = "@jedi_spam:";

		switch (healType) {
		case HEAL_DAMAGE: strVal = strVal + CreatureAttribute::getName(healSpec) + "_damage"; break;
		case HEAL_WOUNDS: strVal = strVal + CreatureAttribute::getName(healSpec) + "_wounds"; break;
		default: strVal = strVal + "battle_fatigue";
		}

		String statStr = StringIdManager::instance()->getStringId(strVal.hashCode()).toString();

		if (selfHeal) {
			StringIdChatParameter message("jedi_spam", "heal_self");
			message.setTO(statStr);
			message.setDI(amount);
			creature->sendSystemMessage(message);
		} else {
			StringIdChatParameter message("jedi_spam", "heal_other_self");
			message.setTO(statStr);
			message.setDI(amount);
			message.setTT(targetID);
			creature->sendSystemMessage(message);

			if (target->isPlayerCreature()) {
				StringIdChatParameter message("jedi_spam", "heal_other_other");
				message.setTO(statStr);
				message.setDI(amount);
				message.setTT(playerID);
				target->sendSystemMessage(message);
			}
		}
	} else if (healType == HEAL_STATES && amount == 0 && !selfHeal) {
		StringIdChatParameter message("jedi_spam", "stop_states_other");
		message.setTT(targetID);
		creature->sendSystemMessage(message);
	} else if (healType == HEAL_POISON && !selfHeal) {
		if (amount == 1) {
			StringIdChatParameter message("jedi_spam", "stop_poison_other");
			message.setTT(targetID);
			creature->sendSystemMessage(message);
		} else {
			StringIdChatParameter message("jedi_spam", "staunch_poison_other");
			message.setTT(targetID);
			creature->sendSystemMessage(message);
		}
	} else if (healType == HEAL_DISEASE && !selfHeal) {
		if (amount == 1) {
			StringIdChatParameter message("jedi_spam", "stop_disease_other");
			message.setTT(targetID);
			creature->sendSystemMessage(message);
		} else {
			StringIdChatParameter message("jedi_spam", "staunch_disease_other");
			message.setTT(targetID);
			creature->sendSystemMessage(message);
		}
	} else if (healType == HEAL_BLEEDING && !selfHeal) {
		if (amount == 1) {
			StringIdChatParameter message("jedi_spam", "stop_bleeding_other");
			message.setTT(targetID);
			creature->sendSystemMessage(message);
		} else {
			StringIdChatParameter message("jedi_spam", "staunch_bleeding_other");
			message.setTT(targetID);
			creature->sendSystemMessage(message);
		}
	}

	if (amount == 0) {
		if (healType == HEAL_POISON) {
			target->getDamageOverTimeList()->sendDecreaseMessage(target, CreatureState::POISONED);
		} else if (healType == HEAL_DISEASE) {
			target->getDamageOverTimeList()->sendDecreaseMessage(target, CreatureState::DISEASED);
		} else if (healType == HEAL_BLEEDING) {
			target->getDamageOverTimeList()->sendDecreaseMessage(target, CreatureState::BLEEDING);
		} else if (healType == HEAL_FIRE) {
			target->getDamageOverTimeList()->sendDecreaseMessage(target, CreatureState::ONFIRE);
		}
	}
}

int ForceHealQueueCommand::runCommandWithTarget(CreatureObject* creature, CreatureObject* targetCreature) const {
	if (creature == nullptr || targetCreature == nullptr)
		return GENERALERROR;

	if (creature->getObjectID() == targetCreature->getObjectID()) // no self healing
		return GENERALERROR;

	if ((!targetCreature->isPlayerCreature() && !targetCreature->isPet()) || targetCreature->isDroidObject() || targetCreature->isWalkerSpecies())
		return INVALIDTARGET;

	if (targetCreature->isDead() || targetCreature->isAttackableBy(creature))
		return GENERALERROR;

	Locker crossLocker(targetCreature, creature);

	if (creature->isKnockedDown())
		return GENERALERROR;

	if(!checkDistance(creature, targetCreature, range))
		return TOOFAR;

	if (checkForArenaDuel(targetCreature)) {
		creature->sendSystemMessage("@jedi_spam:no_help_target"); // You are not permitted to help that target.
		return GENERALERROR;
	}

	if (!targetCreature->isHealableBy(creature)) {
		creature->sendSystemMessage("@healing:pvp_no_help"); // It would be unwise to help such a patient.
		return GENERALERROR;
	}

	if (!CollisionManager::checkLineOfSight(creature, targetCreature)) {
		creature->sendSystemMessage("@healing:no_line_of_sight"); // You cannot see your target.
		return GENERALERROR;
	}

	if (!playerEntryCheck(creature, targetCreature)) {
		return GENERALERROR;
	}

	return runCommand(creature, targetCreature);
}

int ForceHealQueueCommand::doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {
	if (creature == nullptr || !creature->isPlayerCreature())
		return GENERALERROR;

	if (!checkInvalidLocomotions(creature))
		return INVALIDLOCOMOTION;

	int comResult = doCommonJediSelfChecks(creature);

	if (comResult != SUCCESS)
		return comResult;

	ManagedReference<CreatureObject*> targetCreature = nullptr;
	bool isRemoteHeal = range > 0 && ((allowedTarget == TARGET_AUTO) || (allowedTarget & TARGET_OTHER));

	if (isRemoteHeal && target != 0 && target != creature->getObjectID()) {
		ManagedReference<SceneObject*> sceno = server->getZoneServer()->getObject(target);

		if (sceno != nullptr && sceno->isCreatureObject()) {
			targetCreature = sceno.castTo<CreatureObject*>();
		}
	}

	const bool selfHealingAllowed = (allowedTarget & TARGET_SELF) || !isRemoteHeal;
	if (!isRemoteHeal || (targetCreature == nullptr && selfHealingAllowed) || (targetCreature != nullptr && (!targetCreature->isHealableBy(creature)) && selfHealingAllowed)) {
		isRemoteHeal = false;
		targetCreature = creature;
	}

	if (targetCreature == nullptr)
		return GENERALERROR;

	int retval = GENERALERROR;

	if (isRemoteHeal)
		retval = runCommandWithTarget(creature, targetCreature);
	else
		retval = runCommand(creature, targetCreature);

	return retval;
}
