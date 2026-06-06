/*
 * CombatManager.cpp
 *
 *  Created on: 24/02/2010
 *      Author: victor
 */

#include "CombatManager.h"
#include "CreatureAttackData.h"
#include "DefenderHitList.h"
#include "server/zone/objects/scene/variables/DeltaVector.h"
#include "server/zone/objects/building/BuildingObject.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/objects/player/PlayerObject.h"
#include "templates/params/creature/CreatureState.h"
#include "server/zone/objects/creature/commands/CombatQueueCommand.h"
#include "templates/params/creature/CreatureAttribute.h"
#include "server/zone/packets/object/CombatSpam.h"
#include "server/zone/packets/object/CombatAction.h"
#include "server/zone/packets/tangible/UpdatePVPStatusMessage.h"
#include "server/zone/Zone.h"
#include "server/zone/managers/collision/CollisionManager.h"
#include "server/zone/managers/visibility/VisibilityManager.h"
#include "server/zone/managers/creature/LairObserver.h"
#include "server/zone/managers/reaction/ReactionManager.h"
#include "server/zone/managers/player/PlayerManager.h"
#include "server/zone/objects/installation/components/TurretDataComponent.h"
#include "server/zone/objects/creature/ai/AiAgent.h"
#include "server/zone/objects/installation/InstallationObject.h"
#include "server/zone/packets/object/ShowFlyText.h"
#include "server/zone/managers/frs/FrsManager.h"
#include "server/zone/objects/intangible/PetControlDevice.h"
#include "server/zone/objects/installation/TurretObject.h"
//Infinity:  Custom includes
#include "server/zone/managers/skill/SkillManager.h"
#include "server/zone/objects/tangible/component/lightsaber/LightsaberCrystalComponent.h"
#include "server/zone/managers/creature/PetManager.h"
#include "server/zone/managers/death_watch/DeathWatchManager.h"

#define COMBAT_SPAM_RANGE 85 // Range at which players will see Combat Log Info

//#define DEBUG_APPLYDOTS

/*
* Notes:
* Every player that uses an attack ability, including peace, is considered the attacker in a CombatManager instance.
*
* Each Attacker has their Defender List stored on their Object.
* Peace will clear a Object's defender list, but they will not be able to exit Combat if checks are not passed.
*
*/

// Sets attackers mainDefender and puts both in combat
bool CombatManager::startCombat(CreatureObject* attacker, TangibleObject* defender, bool lockDefender, bool allowIncapTarget) const {
	if (attacker == defender) {
		return false;
	}

	if (attacker->getZone() == nullptr || defender->getZone() == nullptr) {
		return false;
	}

	if (attacker->isRidingMount()) {
		ManagedReference<CreatureObject*> parent = attacker->getParent().get().castTo<CreatureObject*>();

		if (parent == nullptr || !parent->isMount()) {
			return false;
		}

		if (parent->hasBuff(STRING_HASHCODE("gallop"))) {
			return false;
		}
	}

	if (attacker->hasRidingCreature()) {
		return false;
	}

	if (!defender->isAttackableBy(attacker)) {
		return false;
	}

	if (attacker->isPlayerCreature() && attacker->getPlayerObject()->isAFK()) {
		return false;
	}

	CreatureObject* creo = defender->asCreatureObject();
	if (creo != nullptr && creo->isIncapacitated() && creo->isFeigningDeath() == false) {
		if (allowIncapTarget) {
			attacker->clearState(CreatureState::PEACE);
			return true;
		}

		return false;
	}

	attacker->clearState(CreatureState::PEACE);

	if (attacker->isPlayerCreature() && !attacker->hasDefender(defender)) {
		ManagedReference<WeaponObject*> weapon = attacker->getWeapon();

		if (weapon != nullptr && weapon->isJediWeapon()) {
			VisibilityManager::instance()->increaseVisibility(attacker, 25);
		}
	}

	Locker clocker(defender, attacker);

	if (creo != nullptr && creo->isPlayerCreature() && !creo->hasDefender(attacker)) {
		ManagedReference<WeaponObject*> weapon = creo->getWeapon();

		if (weapon != nullptr && weapon->isJediWeapon()) {
			VisibilityManager::instance()->increaseVisibility(creo, 25);
		}
	}

	// Infinity: Cancel Force Run 3 if combat starts
	if (creo != nullptr && (creo->isPlayerCreature() || creo->isPet()) && attacker != nullptr && (attacker->isPlayerCreature() || attacker->isPet())) {

		if (attacker->hasBuff(BuffCRC::JEDI_FORCE_RUN_3))
			attacker->removeBuff(BuffCRC::JEDI_FORCE_RUN_3);

		if (creo->hasBuff(BuffCRC::JEDI_FORCE_RUN_3))
			creo->removeBuff(BuffCRC::JEDI_FORCE_RUN_3);
	}

	attacker->setCombatState();
	defender->setCombatState();

	attacker->setDefender(defender);

	if (defender->isCreatureObject()) {
		ThreatMap* defenderThreatMap = defender->getThreatMap();

		if (defenderThreatMap != nullptr) {
			defenderThreatMap->addDamage(attacker, 0);
		}
	}

	return true;
}

// Called when creature attempts to peace out of combat -- Creature is locked pre, Defender List is cleared
bool CombatManager::attemptPeace(CreatureObject* creature) const {
	if (creature == nullptr)
		return false;

	creature->removeDefenders();
	creature->setState(CreatureState::PEACE);

	ThreatMap* threatMap = creature->getThreatMap();

	if (threatMap != nullptr) {
		if (threatMap->size() == 0) {
			creature->clearCombatState(false);
			return true;
		}

		uint64 creatureID = creature->getObjectID();

		for (int i = 0; i < threatMap->size(); i++) {
			TangibleObject* threatTano = threatMap->elementAt(i).getKey();

			if (threatTano == nullptr || threatTano->getObjectID() == creatureID) {
				continue;
			}

			SceneObject* mainDefender = threatTano->getMainDefender();

			// If the defender is in range and is the maind defender of the creature, fail to peace
			if (creature->isInRange(threatTano, 128.f) && mainDefender != nullptr && mainDefender->getObjectID() == creatureID) {
				return false;
			}
		}

		creature->clearCombatState(false);
	}

	return true;
}

// Called for AiAgents to break their combat state
void CombatManager::forcePeace(CreatureObject* attacker) const {
	Reference<CreatureObject*> attackerRef = attacker;

	Core::getTaskManager()->scheduleTask([attackerRef] () {
		Locker lock(attackerRef);

		const DeltaVector<ManagedReference<SceneObject*>>* defenderList = attackerRef->getDefenderList();

		for (int i = defenderList->size() - 1; i >= 0; --i) {
			ManagedReference<SceneObject*> object = defenderList->getSafe(i);

			if (object == nullptr || !object->isTangibleObject())
				continue;

			TangibleObject* defender = cast<TangibleObject*>(object.get());

			Locker clocker(defender, attackerRef);

			if (defender->hasDefender(attackerRef)) {
				attackerRef->removeDefender(defender);
				defender->removeDefender(attackerRef);
			} else {
				attackerRef->removeDefender(defender);
			}

			clocker.release();
		}

		attackerRef->clearCombatState(false);
		attackerRef->setState(CreatureState::PEACE);

	}, "ForcePeaceLambda", 250);
}

/*
*
*	Start Combat Action
*
*/

/*
	CreO attacker -- doCombatAction

*/

int CombatManager::doCombatAction(CreatureObject* attacker, WeaponObject* weapon, TangibleObject* defenderObject, const CreatureAttackData& data) const {
	debug("entering doCombat action with data");

	if (data.getCommand() == nullptr)
		return -3;

	if (!startCombat(attacker, defenderObject, true, data.getHitIncapTarget()))
		return -1;

	// Infinity: DW melee pets cannot attack DW players on jetpacks
	if (attacker->isPet() && weapon != nullptr && weapon->isMeleeWeapon()) {
		ManagedReference<CreatureObject*> petOwner = attacker->getLinkedCreature();
		if (petOwner != nullptr && petOwner->hasSkill("combat_death_watch_novice")) {
			CreatureObject* defenderCreo = defenderObject->asCreatureObject();
			if (defenderCreo != nullptr && defenderCreo->hasSkill("combat_death_watch_novice")) {
				ZoneServer* zs = defenderCreo->getZoneServer();
				if (zs != nullptr) {
					DeathWatchManager* dwMgr = zs->getDeathWatchManager();
					if (dwMgr != nullptr && dwMgr->isOnDeathWatchJetpack(defenderCreo)) {
						petOwner->sendSystemMessage("Your pet cannot reach the airborne target.");
						return -1;  // Melee pet can't reach jetpack rider
					}
				}
			}
		}
	}

	debug("past start combat");

	if (!applySpecialAttackCost(attacker, weapon, data)) {
		return -2;
	}

	debug("past special attack cost");

	SortedVector<DefenderHitList*> targetDefenders;

	int damage = 0;
	bool shouldGcwCrackdownTef = false, shouldGcwTef = false, shouldBhTef = false;

	damage = doTargetCombatAction(attacker, weapon, defenderObject, &targetDefenders, data, &shouldGcwCrackdownTef, &shouldGcwTef, &shouldBhTef);

	if (data.getCommand()->isAreaAction() || data.getCommand()->isConeAction()) {
		Reference<SortedVector<ManagedReference<TangibleObject*>>*> areaDefenders = getAreaTargets(attacker, weapon, defenderObject, data);

		while (areaDefenders->size() > 0) {
			int areaDam = 0;

			for (int i = areaDefenders->size() - 1; i >= 0; i--) {
				TangibleObject* tano = areaDefenders->get(i);
				if (tano == attacker || tano == nullptr) {
					areaDefenders->remove(i);
					continue;
				}

				if (!tano->tryWLock()) {
					continue;
				}

				areaDam += doTargetCombatAction(attacker, weapon, areaDefenders->get(i), &targetDefenders, data, &shouldGcwCrackdownTef, &shouldGcwTef, &shouldBhTef);
				areaDefenders->remove(i);

				tano->unlock();
			}

			if (areaDam > 0)
				damage += areaDam;

			attacker->unlock();
			Thread::yield();
			attacker->wlock(true);
		}
	}

	if (damage > 0) {
		attacker->updateLastSuccessfulCombatAction();

		if (attacker->isPlayerCreature() && data.getCommandCRC() != STRING_HASHCODE("attack")) {
			weapon->decay(attacker);
		}

		// Decreases the powerup once per successful attack
		if (!data.isForceAttack()) {
			weapon->decreasePowerupUses(attacker);
		}
	}

	// Broadcast CombatSpam and CombatAction packets now that the attack is complete
	if (damage >= 0) {
		finalCombatSpam(attacker, weapon, targetDefenders, data);
		broadcastCombatAction(attacker, weapon, targetDefenders, data);
	}

	int defenderSize = targetDefenders.size();

	for (int i = defenderSize - 1; i >= 0; i--) {
		DefenderHitList* list = targetDefenders.get(i);

		delete list;
	}

	// Update PvP TEF Duration   // Infinity:   Test to make sure BH/Mando TEF is being broadcast
	if (shouldGcwCrackdownTef || shouldGcwTef || shouldBhTef) {
		ManagedReference<CreatureObject*> attackingCreature = nullptr;

		if (attacker->isPet()) {
			ManagedReference<PetControlDevice*> controlDevice = attacker->getControlDevice().get().castTo<PetControlDevice*>();

			if (controlDevice != nullptr) {
				ManagedReference<SceneObject*> lastCommander = controlDevice->getLastCommander().get();

				if (lastCommander != nullptr && lastCommander->isCreatureObject()) {
					attackingCreature = lastCommander->asCreatureObject();
				} else {
					attackingCreature = attacker->getLinkedCreature();
				}
			}
		} else {
			attackingCreature = attacker;
		}

		if (attackingCreature != nullptr) {
			PlayerObject* ghost = attackingCreature->getPlayerObject();

			if (ghost != nullptr) {
				Locker olocker(attackingCreature, attacker);
				ghost->updateLastCombatActionTimestamp(shouldGcwCrackdownTef, shouldGcwTef, shouldBhTef);
			}
		}
	}

	if (attacker->isPlayerCreature() && defenderObject->isPlayerCreature()) {
		PlayerObject* ghost = attacker->getPlayerObject();

		if (ghost != nullptr && ghost->isInPvpArea(true)) {
			ghost->updateLastPvpAreaCombatActionTimestamp();
		}
	}

	return damage;
}

/*

	CreO attacker -- doTargetCombatAction

*/

int CombatManager::doTargetCombatAction(CreatureObject* attacker, WeaponObject* weapon, TangibleObject* tano, SortedVector<DefenderHitList*>* targetDefenders, const CreatureAttackData& data, bool* shouldGcwCrackdownTef, bool* shouldGcwTef, bool* shouldBhTef) const {
	int damage = 0;

	Locker clocker(tano, attacker);

	if (!tano->isAttackableBy(attacker)) {
		return -1;
	}

	if (targetDefenders == nullptr) {
		return -1;
	}

	DefenderHitList* hitList = new DefenderHitList();

	if (hitList == nullptr) {
		return -1;
	}

	// Add DefenderHitList to the targetDefenders Vector and set the defender to that list
	hitList->setDefender(tano);
	targetDefenders->add(hitList);

	if (tano->isCreatureObject()) {
		CreatureObject* defender = tano->asCreatureObject();

		if (defender->getWeapon() == nullptr) {
			return -1;
		}

		damage = creoTargetCombatAction(attacker, weapon, defender, hitList, data, shouldGcwCrackdownTef, shouldGcwTef, shouldBhTef);
	} else {
		int poolsToDamage = calculatePoolsToDamage(data.getPoolsToDamage());

		damage = applyDamage(attacker, weapon, tano, hitList, poolsToDamage, data);

		// No Accuracy / Defense Calculation for TanO defender. setHit to HIT value.
		hitList->setHit(HIT);

		bool covertOvert = ConfigManager::instance()->useCovertOvertSystem();
		uint32 tanoFaction = tano->getFaction();

		if (covertOvert && attacker->isPlayerCreature() && tanoFaction > 0 && attacker->getFaction() != tanoFaction && attacker->getFactionStatus() >= FactionStatus::COVERT) {
			PlayerObject* ghost = attacker->getPlayerObject();

			if (ghost != nullptr) {
				ghost->updateLastCombatActionTimestamp(false, true, false);
			}
		}
	}

	if (damage > 0 && tano->isAiAgent()) {
		AiAgent* aiAgent = cast<AiAgent*>(tano);
		bool help = false;

		for (int i = 0; i < 9; i += 3) {
			if (aiAgent->getHAM(i) < (aiAgent->getMaxHAM(i) / 2)) {
				help = true;
				break;
			}
		}

		if (help)
			aiAgent->sendReactionChat(attacker, ReactionManager::HELP);
		else
			aiAgent->sendReactionChat(attacker, ReactionManager::HIT);
	}

	if (damage > -1 && attacker->isAiAgent()) {
		AiAgent* aiAgent = cast<AiAgent*>(attacker);
		aiAgent->sendReactionChat(tano, ReactionManager::HITTARGET);
	}

	return damage;
}

/*

	C+reO attacker && CreO Defender -- doTargetCombatAction

*/

int CombatManager::creoTargetCombatAction(CreatureObject* attacker, WeaponObject* weapon, CreatureObject* defender, DefenderHitList* targetHitList, const CreatureAttackData& data, bool* shouldGcwCrackdownTef, bool* shouldGcwTef, bool* shouldBhTef) const {
	if (targetHitList == nullptr) {
		return 0;
	}

	if (defender->isEntertaining()) {
		defender->stopEntertaining();
	}

	int hitVal = HIT;
	uint8 hitLocation = 0;

	float damageMultiplier = data.getDamageMultiplier();

	int damage = 0;

	if (damageMultiplier != 0) {
		damage = calculateDamage(attacker, weapon, defender, data) * damageMultiplier;
		targetHitList->setInitialDamage(damage);

#ifdef DEBUG_CALCULATEDAMAGE
		StringBuffer debugCalculateDamage;
		debugCalculateDamage << " ========= creoTargetCombatAction (creo vs. creo) Debug ============\n";
		debugCalculateDamage << " Final damage after command damage multiplier: " << damage << ".  Multiplier:"  << damageMultiplier  << ".\n";
		debugCalculateDamage << " ========= END creoTargetCombatAction (creo vs. creo) Debug ========";

		if (attacker->isPlayerCreature()) {
			attacker->sendSystemMessage(debugCalculateDamage.toString());
		}
		if (defender->isPlayerCreature()) {
			defender->sendSystemMessage(debugCalculateDamage.toString());
		}
#endif // DEBUG_CALCULATEDAMAGE
	}

	/* Infinity:  Logging for pet ranged attack
	StringBuffer msg;
	msg << "===== Debug creoTargetCombatAction weapon and damage =====" << endl;
	msg << "Attacker: " << attacker->getDisplayedName() << endl;
	msg << "Defender: " << defender->getDisplayedName() << endl;
	msg << "Attack: " << data.getCommandName() << endl;
	if (weapon != nullptr) {
		msg << "Weapon: " << weapon->getObjectTemplate()->getTemplateFileName() << endl;
		String attackType = "";
		if (weapon->getAttackType() == SharedWeaponObjectTemplate::MELEEATTACK)
			attackType = "Melee Attack";
		else (weapon->getAttackType() == SharedWeaponObjectTemplate::RANGEDATTACK)
			attackType = "Ranged Attack";
		msg << "Attack Type: " << attackType << endl;
	}
	msg << "Damage: " << damage << endl;

	if (attacker != nullptr) {

		if (attacker->isPlayerCreature())
			attacker->sendSystemMessage(msg.toString());
		else if (attacker->isPet()) {
			ManagedReference<CreatureObject*> owner = attacker->getLinkedCreature();
			if (owner != nullptr)
				owner->sendSystemMessage(msg.toString());
		}
	}

	if (defender != nullptr) {

		if (defender->isPlayerCreature())
			defender->sendSystemMessage(msg.toString());
		else if (defender->isPet()) {
			ManagedReference<CreatureObject*> owner = defender->getLinkedCreature();
			if (owner != nullptr)
				owner->sendSystemMessage(msg.toString());
		}

	}

	*/

	damageMultiplier = 1.0f;

	if (!data.isStateOnlyAttack()) {
		hitVal = getHitChance(attacker, defender, weapon, data, damage, data.getAccuracyBonus());
	}

	switch (hitVal) {
	case MISS:  // Infinity:  May need checkForTefs here MrObvious ****
		doMiss(attacker, weapon, defender, data, damage);
		damageMultiplier = 0.0f;
		break;
	case BLOCK:
		doBlock(attacker, weapon, defender, damage);
		damageMultiplier = 0.0f;  // Infinity:   Block will avoid 100% of damage
		break;
	case DODGE:
		doDodge(attacker, weapon, defender, damage);
		damageMultiplier = 0.0f;
		break;
	case COUNTER: {
		doCounterAttack(attacker, weapon, defender, damage);
		if (!defender->hasState(CreatureState::PEACE))
			defender->executeObjectControllerAction(STRING_HASHCODE("attack"), attacker->getObjectID(), "");
		damageMultiplier = 0.0f;
		break;
	}
	case RICOCHET:
		damageMultiplier = 0.0f;
		break;
	default:
		break;
	}

	// If it's a state only attack (intimidate, warcry, wookiee roar) don't apply dots or break combat delays
	if (!data.isStateOnlyAttack() && hitVal != MISS) {  
		if (defender->hasAttackDelay()) {
			defender->removeAttackDelay();
		}

		if (damageMultiplier != 0 && damage != 0) {
			int poolsToDamage = calculatePoolsToDamage(data.getPoolsToDamage());
			// TODO: animations are probably determined by which pools are damaged (high, mid, low, combos, etc)
			int unmitDamage = damage;

			damage = applyDamage(attacker, weapon, defender, targetHitList, damage, damageMultiplier, poolsToDamage, hitLocation, data);

			if (defender->isDead() || defender->isIncapacitated()) {
				// This broadcasts the dead or incapacitated postures immediately
				defender->updatePostures(true);
			} else {
				applyDots(attacker, defender, data, damage, unmitDamage, poolsToDamage);

				// Prevents weapon dots from applying with offensive Force powers
				if (!data.isForceAttack()) {
					applyWeaponDots(attacker, defender, weapon);
				}
			}
		}
	}

	if (hitVal != MISS) {
		checkForTefs(attacker, defender, shouldGcwCrackdownTef, shouldGcwTef, shouldBhTef);
	}

	// Set DefenderHitList values to be used for CombatSpam and CombatAction broadcasts
	targetHitList->setHit(hitVal);
	targetHitList->setDamageMultiplier(damageMultiplier);
	targetHitList->setHitLocation(hitLocation);

	return damage;
}

/*
	TanO attacker -- doCombatAction

*/

int CombatManager::doCombatAction(TangibleObject* attacker, WeaponObject* weapon, TangibleObject* defender, const CombatQueueCommand* command) const {
	if (command == nullptr)
		return -3;

	const CreatureAttackData data = CreatureAttackData("", command, defender->getObjectID());
	int damage = 0;

	SortedVector<DefenderHitList*> targetDefenders;

	if (weapon != nullptr) {
		damage = doTargetCombatAction(attacker, weapon, defender, &targetDefenders, data);

		if (data.getCommand()->isAreaAction() || data.getCommand()->isConeAction()) {
			Reference<SortedVector<ManagedReference<TangibleObject*>>*> areaDefenders = getAreaTargets(attacker, weapon, defender, data);

			for (int i = 0; i < areaDefenders->size(); i++) {
				damage += doTargetCombatAction(attacker, weapon, areaDefenders->get(i), &targetDefenders, data);
			}
		}
	}

	// Send out CombatSpam broadcast now that attack is complete. TanO attackers CombatAction packets are sent out in tanoTargetCombatAction
	finalCombatSpam(attacker, weapon, targetDefenders, data);

	int defenderSize = targetDefenders.size();

	for (int i = defenderSize - 1; i >= 0; i--) {
		DefenderHitList* list = targetDefenders.get(i);

		delete list;
	}

	return damage;
}

/*

	TanO attacker -- doTargetCombatAction

*/

int CombatManager::doTargetCombatAction(TangibleObject* attacker, WeaponObject* weapon, TangibleObject* tano, SortedVector<DefenderHitList*>* targetDefenders, const CreatureAttackData& data) const {
	int damage = 0;

	Locker clocker(tano, attacker);

	DefenderHitList* hitList = new DefenderHitList();

	if (hitList == nullptr) {
		return 0;
	}

	// Create DefenderHitList for the defender and add it to targetDefenders Vector
	targetDefenders->add(hitList);
	hitList->setDefender(tano);

	if (tano->isCreatureObject()) {
		CreatureObject* defenderObject = tano->asCreatureObject();

		if (defenderObject->getWeapon() != nullptr) {
			damage = tanoTargetCombatAction(attacker, weapon, defenderObject, hitList, data);
		}
	} else {
		// TODO: Implement TanO vs TanO damage -- This currently has no use
	}

	return damage;
}

/*

	TanO attacker && CreO Defender -- doTargetCombatAction

*/

int CombatManager::tanoTargetCombatAction(TangibleObject* attacker, WeaponObject* weapon, CreatureObject* defenderObject, DefenderHitList* targetHitList, const CreatureAttackData& data) const {
	if (defenderObject == nullptr || !defenderObject->isAttackableBy(attacker))
		return 0;

	if (defenderObject->isEntertaining()) {
		defenderObject->stopEntertaining();
	}

	float damageMultiplier = data.getDamageMultiplier();

	int damage = 0;

	if (damageMultiplier != 0) {
		damage = calculateDamage(attacker, weapon, defenderObject, data) * damageMultiplier;
	}

	damageMultiplier = 1.0f;
	int hitVal = getHitChance(attacker, defenderObject, weapon, data, damage, data.getAccuracyBonus());

	uint8 hitLocation = 0;

	switch (hitVal) {
	case MISS:
		doMiss(attacker, weapon, defenderObject, data, damage);
		damageMultiplier = 0.0f;
		break;
	case HIT:
		break;
	case BLOCK:
		doBlock(attacker, weapon, defenderObject, damage);
		damageMultiplier = 0.0f;  // Infinity:   Block will avoid 100% of damage
		break;
	case DODGE:
		doDodge(attacker, weapon, defenderObject, damage);
		damageMultiplier = 0.0f;
		break;
	case COUNTER:
		doCounterAttack(attacker, weapon, defenderObject, damage);
		if (!defenderObject->hasState(CreatureState::PEACE)) {
			defenderObject->executeObjectControllerAction(STRING_HASHCODE("attack"), attacker->getObjectID(), "");
		}
		damageMultiplier = 0.0f;
		break;
	case RICOCHET:
		damageMultiplier = 0.0f;
		break;
	default:
		break;
	}

	if (hitVal != MISS) {
		int poolsToDamage = calculatePoolsToDamage(data.getPoolsToDamage());

		if (poolsToDamage == 0) {
			return 0;
		}

		if (defenderObject->hasAttackDelay()) {
			defenderObject->removeAttackDelay();
		}

		if (damageMultiplier != 0 && damage != 0) {
			damage = applyDamage(attacker, weapon, defenderObject, targetHitList, damage, damageMultiplier, poolsToDamage, hitLocation, data);
		}

		if (defenderObject->isDead() || defenderObject->isIncapacitated()) {
			// This broadcasts the dead or incapacitated postures immediately
			defenderObject->updatePostures(true);
		}
	} else {
		damage = 0;
	}

	defenderObject->updatePostures(false);

	// Set DefenderHitList values to be used for CombatSpam broadcasts
	targetHitList->setHit(hitVal);
	targetHitList->setInitialDamage(damage);
	targetHitList->setDamageMultiplier(damageMultiplier);
	targetHitList->setHitLocation(hitLocation);

	uint32 animationCRC = data.getCommand()->getAnimation(attacker, defenderObject, weapon, hitLocation, damage).hashCode();

	CombatAction* combatAction = new CombatAction(attacker, defenderObject, animationCRC, hitVal, CombatManager::DEFAULTTRAIL);
	attacker->broadcastMessage(combatAction, true);

	return damage;
}

/*
	Broadcast CombatAction Packets
	-- Handles Animations

*/

void CombatManager::broadcastCombatAction(CreatureObject* attacker, WeaponObject* weapon, SortedVector<DefenderHitList*> targetDefenders, const CreatureAttackData& data) const {
	if (attacker == nullptr) {
		return;
	}

	String effect = "";
	DefenderHitList* hitList = targetDefenders.get(0);

	if (hitList != nullptr && weapon != nullptr) {
		TangibleObject* defenderObject = hitList->getDefender();

		if (defenderObject != nullptr) {
			uint32 animationCRC = 0;
			auto combatCommand = data.getCommand();

			if (combatCommand != nullptr) {
				const String& animation = combatCommand->getAnimation(attacker, defenderObject, weapon, hitList->getHitLocation(), hitList->getInitialDamage());

				if (!animation.isEmpty()) {
					animationCRC = animation.hashCode();
				}

				effect = combatCommand->getEffectString();
			}

			if (animationCRC > 0) {
				uint64 weaponID = weapon->getObjectID();

				auto combatAction = new CombatAction(attacker, targetDefenders, animationCRC, data.getTrails(), weaponID);

				if (combatAction != nullptr) {
					attacker->broadcastMessage(combatAction, true);
				}
			} else {
				attacker->error() << "animationCRC is 0 for " << data.getCommandName();
			}
		}
	}

	if (data.changesAttackerPosture()) {
		attacker->updatePostures(false);
	}

	if (!effect.isEmpty()) {
		attacker->playEffect(effect);
	}
}

void CombatManager::finalCombatSpam(TangibleObject* attacker, WeaponObject* weapon, SortedVector<DefenderHitList*> targetDefenders, const CreatureAttackData& data) const {
	if (attacker == nullptr) {
		return;
	}

	Zone* zone = attacker->getZone();

	if (zone == nullptr) {
		return;
	}


	for (int i = 0; i < targetDefenders.size(); ++i) {
		DefenderHitList* hitList = targetDefenders.get(i);

		if (hitList == nullptr) {
			continue;
		}

		TangibleObject* defender = hitList->getDefender();

		// Combat Spam
		if (defender != nullptr) {
			Locker defLock(defender, attacker);

			int jediMitigation = hitList->getJediMitigation();
			int foodMit = hitList->getFoodMitigation();
			int broadcastDamage = hitList->getInitialDamage() - jediMitigation - foodMit;
			int hit = hitList->getHit();

			// Initial attack combat spam
			if (!data.isStateOnlyAttack()) {
				if (hit == RICOCHET || hit == DODGE) {
					data.getCommand()->sendAttackCombatSpam(attacker, defender, hit, hitList->getInitialDamage(), data);
				} else {
					data.getCommand()->sendAttackCombatSpam(attacker, defender, hit, broadcastDamage, data);
				}
			}

			if (defender->isCreatureObject()) {
				CreatureObject* defenderCreo = defender->asCreatureObject();

				// Mitigation Combat spam
				if (defenderCreo != nullptr) {
					int feedbackDmg = hitList->getForceFeedback();
					int forceAbsorb = hitList->getForceAbsorb();
					int psgDmgAbsorbed = hitList->getPsgMitigation();
					int armorDmgAbsorbed = hitList->getArmorMitigation();
					int copDmgAbsorbed = hitList->getCopMitigation();

					if (jediMitigation > 0 && !data.isForceAttack()) {
						sendMitigationCombatSpam(defenderCreo, nullptr, jediMitigation, FORCEARMOR);
					} else if (jediMitigation > 0) {
						sendMitigationCombatSpam(defenderCreo, nullptr, jediMitigation, FORCESHIELD);
					}

					if (copDmgAbsorbed > 0) {
						sendMitigationCombatSpam(defenderCreo, nullptr, copDmgAbsorbed, CLOAKOFPAIN);
					}

					if (feedbackDmg > 0) {
						broadcastCombatSpam(defender, attacker, nullptr, feedbackDmg, "cbt_spam", "forcefeedback_hit", 1);
					}

					if (forceAbsorb > 0) {
						sendMitigationCombatSpam(defenderCreo, nullptr, forceAbsorb, FORCEABSORB);
					}

					ManagedReference<ArmorObject*> psg = getPSGArmor(defenderCreo);

					if (psgDmgAbsorbed > 0) {
						sendMitigationCombatSpam(defenderCreo, psg, psgDmgAbsorbed, PSG);
					}

					ManagedReference<ArmorObject*> armor = nullptr;
					armor = getArmorObject(defenderCreo, hitList->getHitLocation());

					if (armor != nullptr && armorDmgAbsorbed > 0) {
						sendMitigationCombatSpam(defenderCreo, armor, armorDmgAbsorbed, ARMOR);
					}

					if (!defenderCreo->isIncapacitated() && !defenderCreo->isDead()) {
						if (attacker->isCreatureObject() && hit == HIT) {
							applyStates(attacker->asCreatureObject(), defenderCreo, hitList, data);
						}

						woundCreatureTarget(defenderCreo, weapon, hitList->getPoolsToWound());
					}

					if (foodMit > 0) {
						sendMitigationCombatSpam(defenderCreo, weapon, foodMit, FOOD);
					}
				}
			}
		}
	}
}

// broadcast CombatSpam packets
void CombatManager::broadcastCombatSpam(TangibleObject* attacker, TangibleObject* defender, TangibleObject* item, int damage, const String& file, const String& stringName, byte color) const {
	if (attacker == nullptr)
		return;

	Zone* zone = attacker->getZone();

	if (zone == nullptr)
		return;

	CloseObjectsVector* vec = (CloseObjectsVector*)attacker->getCloseObjects();
	SortedVector<TreeEntry*> closeObjects;

	if (vec != nullptr) {
		closeObjects.removeAll(vec->size(), 10);
		vec->safeCopyReceiversTo(closeObjects, CloseObjectsVector::PLAYERTYPE);
	} else {
#ifdef COV_DEBUG
		info("Null closeobjects vector in CombatManager::broadcastCombatSpam", true);
#endif
		zone->getInRangeObjects(attacker->getWorldPositionX(), attacker->getWorldPositionZ(), attacker->getWorldPositionY(), COMBAT_SPAM_RANGE, &closeObjects, true);
	}

	for (int i = 0; i < closeObjects.size(); ++i) {
		SceneObject* object = static_cast<SceneObject*>(closeObjects.get(i));

		if (object->isPlayerCreature() && attacker->isInRange(object, COMBAT_SPAM_RANGE)) {
			CreatureObject* receiver = static_cast<CreatureObject*>(object);
			CombatSpam* spam = new CombatSpam(attacker, defender, receiver, item, damage, file, stringName, color);
			receiver->sendMessage(spam);
		}
	}
}

void CombatManager::sendMitigationCombatSpam(CreatureObject* defender, TangibleObject* item, uint32 damage, int type) const {
	if (defender == nullptr || !defender->isPlayerCreature())
		return;

	int color = 0; // text color
	String file = "";
	String stringName = "";

	switch (type) {
	case PSG:
		color = 1; // green, confirmed
		file = "cbt_spam";
		stringName = "shield_damaged";
		break;
	case FORCESHIELD:
		color = 1; // green, unconfirmed
		file = "cbt_spam";
		stringName = "forceshield_hit";
		item = nullptr;
		break;
	case FORCEFEEDBACK:
		color = 2; // red, confirmed
		file = "cbt_spam";
		stringName = "forcefeedback_hit";
		item = nullptr;
		break;
	case FORCEABSORB:
		color = 0; // white, unconfirmed
		file = "cbt_spam";
		stringName = "forceabsorb_hit";
		item = nullptr;
		break;
	case FORCEARMOR:
		color = 1; // green, confirmed
		file = "cbt_spam";
		stringName = "forcearmor_hit";
		item = nullptr;
		break;
	case ARMOR:
		color = 1; // green, confirmed
		file = "cbt_spam";
		stringName = "armor_damaged";
		break;
	case FOOD:
		color = 0; // white, confirmed
		file = "combat_effects";
		stringName = "mitigate_damage";
		item = nullptr;
		break;
	case CLOAKOFPAIN:
		color = 1; // green
		file = "cbt_spam";
		stringName = "cloakofpain_hit";
		item = nullptr;
		break;
	default:
		break;
	}

	CombatSpam* spam = new CombatSpam(defender, nullptr, defender, item, damage, file, stringName, color);
	defender->sendMessage(spam);
}

/*
*
*	Other Combat Functions Below
*
*/

// Get Attackers Area Targets
Reference<SortedVector<ManagedReference<TangibleObject*>>*> CombatManager::getAreaTargets(TangibleObject* attacker, WeaponObject* weapon, TangibleObject* defenderObject, const CreatureAttackData& data) const {
	Vector3 attackerPos = attacker->getPosition();
	Vector3 defenderPos = defenderObject->getPosition();

	float dx = defenderPos.getX() - attackerPos.getX();
	float dy = defenderPos.getY() - attackerPos.getY();

	Reference<SortedVector<ManagedReference<TangibleObject*>>*> defenders = new SortedVector<ManagedReference<TangibleObject*>>();

	Zone* zone = attacker->getZone();

	if (zone == nullptr)
		return defenders;

	PlayerManager* playerManager = zone->getZoneServer()->getPlayerManager();

	int damage = 0;

	int areaRange = data.getAreaRange();
	int range = areaRange;

	if (data.getCommand()->isConeAction()) {
		int coneRange = data.getConeRange();

		if (coneRange > -1) {
			range = coneRange;
		} else {
			range = data.getRange();
		}
	}

	bool thrownWeapon = false;
	bool heavyWeapon = false;

	if (weapon != nullptr) {
		thrownWeapon = weapon->isThrownWeapon();
		heavyWeapon = weapon->isHeavyWeapon();

		if (range < 0) {
			range = weapon->getMaxRange();
		}

		if (data.isSplashDamage())
			range += data.getRange();

		if (thrownWeapon || heavyWeapon)
			range = weapon->getMaxRange() + areaRange;
	}

	if (range < 0)
		return defenders;

	try {
		// zone->rlock();

		CloseObjectsVector* vec = (CloseObjectsVector*)attacker->getCloseObjects();

		SortedVector<TreeEntry*> closeObjects;

		if (vec != nullptr) {
			closeObjects.removeAll(vec->size(), 10);
			vec->safeCopyTo(closeObjects);
		} else {
#ifdef COV_DEBUG
			attacker->info("Null closeobjects vector in CombatManager::getAreaTargets", true);
#endif
			zone->getInRangeObjects(attackerPos.getX(), 0, attackerPos.getY(), 128, &closeObjects, true);
		}

		for (int i = 0; i < closeObjects.size(); ++i) {
			SceneObject* object = static_cast<SceneObject*>(closeObjects.get(i));

			TangibleObject* tano = object->asTangibleObject();

			if (tano == nullptr) {
				continue;
			}

			if (tano == attacker || tano == defenderObject) {
				// error("object is attacker");
				continue;
			}

			if (!tano->isAttackableBy(attacker)) {
				// error("object is not attackable");
				continue;
			}

			uint64 tarParentID = object->getParentID();

			if (attacker->isPlayerCreature() && tarParentID != 0 && attacker->getParentID() != tarParentID) {
				Reference<CellObject*> targetCell = tano->getParent().get().castTo<CellObject*>();
				CreatureObject* attackerCreO = attacker->asCreatureObject();

				if (attackerCreO != nullptr && targetCell != nullptr) {
					ManagedReference<SceneObject*> parentSceneObject = targetCell->getParent().get();

					if (parentSceneObject != nullptr) {
						BuildingObject* building = parentSceneObject->asBuildingObject();

						if (building != nullptr && !building->isAllowedEntry(attackerCreO)) {
							continue;
						}
					}

					const ContainerPermissions* perms = targetCell->getContainerPermissions();

					// This portion of the check is specific for locked dungeons doors since they do not inherit perms from parent
					if (!perms->hasInheritPermissionsFromParent() && (attacker->getRootParent() == tano->getRootParent())) {
						if (!targetCell->checkContainerPermission(attackerCreO, ContainerPermissions::WALKIN)) {
							continue;
						}
					}
				}
			}

			float attackerRadiusSq = attacker->getTemplateRadius() * attacker->getTemplateRadius();
			float tanoRadiusSq = tano->getTemplateRadius() * tano->getTemplateRadius();

			if ((attacker->getWorldPosition().squaredDistanceTo(tano->getWorldPosition()) - attackerRadiusSq - tanoRadiusSq) > (range * range)) {
				// error("not in range " + String::valueOf(range));
				continue;
			}

			if ((data.isSplashDamage() || thrownWeapon || heavyWeapon) && (defenderObject->getWorldPosition().squaredDistanceTo(tano->getWorldPosition()) - tanoRadiusSq > (areaRange * areaRange)))
				continue;

			CreatureObject* creo = tano->asCreatureObject();

			if (creo != nullptr && creo->isFeigningDeath() == false && creo->isIncapacitated()) {
				// error("object is incapacitated");
				continue;
			}

			if (data.getCommand()->isConeAction() && !checkConeAngle(tano, data.getConeAngle(), attackerPos.getX(), attackerPos.getY(), dx, dy)) {
				// error("object is not in cone angle");
				continue;
			}

			// zone->runlock();

			try {
				if (!thrownWeapon && !data.isSplashDamage() && !heavyWeapon) {
					if (CollisionManager::checkLineOfSight(object, attacker)) {
						defenders->put(tano);
						attacker->addDefender(tano);
					}
				} else {
					if (CollisionManager::checkLineOfSight(object, defenderObject)) {
						defenders->put(tano);
						attacker->addDefender(tano);
					}
				}
			} catch (Exception& e) {
				error(e.getMessage());
			} catch (...) {
				// zone->rlock();

				throw;
			}

			//			zone->rlock();
		}

		//		zone->runlock();
	} catch (...) {
		//		zone->runlock();

		throw;
	}

	return defenders;
}

/*
	Damage
*/

// Calculate Damage - CreO attacker & CreO defender
float CombatManager::calculateDamage(CreatureObject* attacker, WeaponObject* weapon, CreatureObject* defender, const CreatureAttackData& data) const {

	if (attacker == nullptr || defender == nullptr) {
		return 0;
	}

	float damage = 0;
	int diff = 0;

	String commandName = data.getCommandName();

	if (data.getMinDamage() > 0 && data.getMaxDamage() > 0) { // this is a special attack (force, etc)
		float minDmg = data.getMinDamage();
		float maxDmg = data.getMaxDamage();

		float mod = attacker->isAiAgent() ? cast<AiAgent*>(attacker)->getSpecialDamageMult() : 1.f;
		damage = minDmg * mod;
		diff = (maxDmg * mod) - damage;
	}
	else {
		diff = calculateDamageRange(attacker, defender, weapon);
		float minDamage = weapon->getMinDamage();

		if (attacker->isPlayerCreature() && !weapon->isCertifiedFor(attacker))
			minDamage = 5;

		damage = minDamage;
	}

	if (diff > 0)
		damage += System::random(diff);

#ifdef DEBUG_CALCULATEDAMAGE
	StringBuffer debugCalculateDamage;
	debugCalculateDamage << " ========= calculateDamage (creo vs. creo) Debug ====++++====\n";
		debugCalculateDamage << " Attacker: " << attacker->getFirstName() << ".  Defender: " << defender->getFirstName() << ".\n";
		debugCalculateDamage << " Command: " << commandName << ".  Force attack: ";
	if (data.isForceAttack()) {
		debugCalculateDamage << "Yes. \n";
	}
	else {
		debugCalculateDamage << "No. \n";
	}
	debugCalculateDamage << " Damage including weapon and special attack damage: " << damage << "\n";
#endif // DEBUG_CALCULATEDAMAGE

	damage = applyDamageModifiers(attacker, weapon, damage, data);

#ifdef DEBUG_CALCULATEDAMAGE
	debugCalculateDamage << " Damage after damage modifiers (buffs, damage mods): " << damage << "\n";
#endif // DEBUG_CALCULATEDAMAGE

	float damageMitigationDivisor = defender->getSkillMod("private_mitigation_divisor");  	            //Infinity: New mod, standard integer divisor
	if (damageMitigationDivisor != 0)
		damage /= damageMitigationDivisor;

	damageMitigationDivisor = defender->getSkillMod("private_mitigation_divisor_x100") / 100.f;  	    //Infinity: New mod, doing X100 to get two decimal points for
	if (damageMitigationDivisor != 0)
		damage /= damageMitigationDivisor;

#ifdef DEBUG_CALCULATEDAMAGE
	debugCalculateDamage << " Damage after private_mitigation_divisors: " << damage << "\n";
#endif // DEBUG_CALCULATEDAMAGE

	damage += defender->getSkillMod("private_damage_susceptibility");

#ifdef DEBUG_CALCULATEDAMAGE
	debugCalculateDamage << " Damage after private_damage_susceptibility and before FRS mods: " << damage << "\n";
#endif // DEBUG_CALCULATEDAMAGE

	if (attacker->isPlayerCreature()) {// Infinity:  FRS damage bonuses here and standard player 1.5x damage bonus

		if (data.isForceAttack()) {
			damage *= attacker->getFrsMod("power",2);  //Infinity - FRS Bonus based on power mod
#ifdef DEBUG_CALCULATEDAMAGE
			debugCalculateDamage << " Damage after FRS power modifier on Force attack: " << damage << ".  FRS Multiplier:" << attacker->getFrsMod("power",2)  << ".\n";
#endif // DEBUG_CALCULATEDAMAGE
		}
		else if (weapon->getDamageType() == SharedWeaponObjectTemplate::LIGHTSABER && weapon->getAttackType() == SharedWeaponObjectTemplate::MELEEATTACK) {
			damage *= attacker->getFrsMod("manipulation",2);        // Infinity: Lightsaber damage scales with FRS manipulation mod in PVE/PVP like powers	
#ifdef DEBUG_CALCULATEDAMAGE
			debugCalculateDamage << " Damage after FRS manipulation modifier on non-Force attack: " << damage << ".  FRS Multiplier:" << attacker->getFrsMod("manipulation",2)  << ".\n";
#endif // DEBUG_CALCULATEDAMAGE
		}

		// Infinity: Removing vanilla random damage increase on powers against NPCs.  All players get 1.5x damage bonus
		damage *= 1.5;

#ifdef DEBUG_CALCULATEDAMAGE
		debugCalculateDamage << " Damage after 1.5x player damage bonus: " << damage << "\n";
#endif // DEBUG_CALCULATEDAMAGE
	}

	if (attacker->isPet()) { // Infinity: All pets receive a damage boost
		if (attacker->isWalkerSpecies() || attacker->isDroidSpecies()) { // Infinity: Want to limit AT-ST and Droideka damage
			String templateName = attacker->getObjectTemplate()->getTemplateFileName();
			if (templateName == "atst" || templateName == "rebel_droideka")
				damage *= 1.5;
		}
		else {
			damage *= 2.0;
		}
#ifdef DEBUG_CALCULATEDAMAGE
		debugCalculateDamage << " Damage after (1.5/2.0) pet damage bonus: " << damage << "\n";
#endif // DEBUG_CALCULATEDAMAGE
	}	

	if (!data.isForceAttack() && weapon->getAttackType() == SharedWeaponObjectTemplate::MELEEATTACK) {
		damage *= 1.25;
#ifdef DEBUG_CALCULATEDAMAGE
		debugCalculateDamage << " Damage after 1.25x Melee Damage Bonus: " << damage << "\n";
#endif // DEBUG_CALCULATEDAMAGE
	}

	// Infinity: Let's setup booleans for simplicity
	bool attackerPlayerOrPet = attacker->isPlayerCreature() || attacker->isPet();
	bool defenderPlayerOrPet = defender->isPlayerCreature() || defender->isPet();

	if (defender->isKnockedDown()) { 		// Infinity:  KD logic

		if (defender->isPlayerCreature() && !attackerPlayerOrPet)
			damage *= 1.2f;
		else 
			damage *= 1.5f;

#ifdef DEBUG_CALCULATEDAMAGE
		debugCalculateDamage << " Damage after 1.2x/1.5x KD multiplier: " << damage << "\n";
#endif // DEBUG_CALCULATEDAMAGE
	}

#ifdef DEBUG_CALCULATEDAMAGE
	float saveDamage = damage;
#endif // DEBUG_CALCULATEDAMAGE

   if (commandName.beginsWith("flame"))								//Infinity: 25% damage reduction on flamethrower attack in PVP
		if (attacker->isPlayerCreature() && defenderPlayerOrPet)
			damage *= .75; 

	if (weapon != nullptr && weapon->isHeavyWeapon()){
		if (!attacker->isPlayerCreature() && defenderPlayerOrPet)
			damage *= .4; 											// Infinity:  Reduce NPC heavy weapon damage vs players by 60%
	}

	if (weapon != nullptr && weapon->isSpecialHeavyWeapon()){
		if (!attacker->isPlayerCreature() && defenderPlayerOrPet)	// Infinity:  reduce npc special heavy weapon damage vs players by 25%
			damage *= .75;
	}

#ifdef DEBUG_CALCULATEDAMAGE
	if (damage < saveDamage) {
		debugCalculateDamage << " Damage after flamethrower/heavyweapon/special heavy weapon reduction: " << damage << "\n";
	}
#endif // DEBUG_CALCULATEDAMAGE

	// Infinity:  Get player objects for Infinity custom skills
	ManagedReference<PlayerObject*> attackerGhost = attacker->getPlayerObject();
	ManagedReference<PlayerObject*> defenderGhost = defender->getPlayerObject();

	// Save defender weapon booleans
	bool defenderJediWeapon = false;
	bool defenderMeleeWeapon = false;
	if (defender->isPlayerCreature()) {
		ManagedReference<WeaponObject*> defenderWeapon = defender->getWeapon();
		if (defenderWeapon != nullptr) {
			if (defenderWeapon->isJediWeapon())
				defenderJediWeapon = true;

			if (defenderWeapon->isMeleeWeapon())
				defenderMeleeWeapon = true;
		}
	}

	// Infinity:  Mark of the Hunter buff
	if (attacker->hasSkill("combat_bountyhunter_novice") && attackerGhost != nullptr && defenderGhost != nullptr && defenderJediWeapon) { //Mark of the Hunter only works vs for bounty hunters vs. jedi
		if (attackerGhost->hasBhTef() && defenderGhost->hasBhTef()){
			if (!attacker->hasBuff(BuffCRC::SKILL_BUFF_HUNTER)){
				ManagedReference<Buff*> hunterBuff = new Buff(attacker, BuffCRC::SKILL_BUFF_HUNTER, 30, BuffType::SKILL);
				Locker locker(hunterBuff);
				attacker->addBuff(hunterBuff);
				locker.release();
				attacker->showFlyText("combat_effects", "hunter", 0, 255, 0);

				if (!defender->hasBuff(BuffCRC::SKILL_BUFF_HUNTED1)){
					ManagedReference<Buff*> huntedBuff = new Buff(defender, BuffCRC::SKILL_BUFF_HUNTED1, 30, BuffType::SKILL);
					Locker hlocker(huntedBuff);
					defender->addBuff(huntedBuff);
					hlocker.release();
					defender->showFlyText("combat_effects", "hunted1", 0, 255, 0);
				}
				else if (!defender->hasBuff(BuffCRC::SKILL_BUFF_HUNTED2)){
					ManagedReference<Buff*> huntedBuff = new Buff(defender, BuffCRC::SKILL_BUFF_HUNTED2, 30, BuffType::SKILL);
					Locker hlocker(huntedBuff);
					defender->addBuff(huntedBuff);
					hlocker.release();
					defender->showFlyText("combat_effects", "hunted2", 0, 255, 0);
				}
				else if (!defender->hasBuff(BuffCRC::SKILL_BUFF_HUNTED3)){    
					ManagedReference<Buff*> huntedBuff = new Buff(defender, BuffCRC::SKILL_BUFF_HUNTED3, 30, BuffType::SKILL);
					Locker hlocker(huntedBuff);
					defender->addBuff(huntedBuff);
					hlocker.release();
					defender->showFlyText("combat_effects", "hunted3", 0, 255, 0);
				} 
			}
		}
	}

	// Infinity:  Now check Gyroscopic Stabilizers
	if (defender->isPlayerCreature() && defender->isKnockedDown() && defender->isDizzied() && defenderGhost->hasAbility("gyroStabilizers") && 
			defender->checkCooldownRecovery("gyroStabilizers") && !defender->isDead() && !defender->isIncapacitated() && !defender->isFeigningDeath()) {
		defender->setPosture(CreaturePosture::UPRIGHT);
		defender->removeStateBuff(CreatureState::DIZZY);
		if (defender->hasAttackDelay())
			defender->removeAttackDelay();
		defender->updateCooldownTimer("gyroStabilizers", 1000 * 60 * 3);  //3 Minute cooldown
		defender->sendSystemMessage("Your Gyroscopic Stabilizers have activated!");
	}

	// Infinity:  Jedi Fortify
	bool isDelayed = false;
	if (defender->hasAttackDelay() || defender->hasPostureChangeDelay())
		isDelayed = true;

   if (!isDelayed && defenderGhost != nullptr && defender->hasBuff(BuffCRC::JEDI_FORTIFY) && !attacker->isIntimidated() &&
		!attacker->isDroidSpecies() && !attacker->isVehicleObject() && !attacker->isWalkerSpecies() && 
		attacker->checkCooldownRecovery("fortifyintimidate") && attacker->checkCooldownRecovery("forceintimidatesingle") && attacker->isInRange(defender, 25.f)) {

		int forceCost = attackerPlayerOrPet ? 12 : 8;  // 12 in PVP, 8 in PVE

	  	int mdefForcePower = defenderGhost->getForcePower();
		if (mdefForcePower >= forceCost) {  // Check for enough force first
			defenderGhost->setForcePower(mdefForcePower - forceCost);  
  
			//defender has fortify and attacker is not intimidated, attempt to intimidate them.
			int playerLevel = 0;
			if (attacker->isPlayerCreature()) {
				ZoneServer* server = attacker->getZoneServer();
				if (server != nullptr) {
					PlayerManager* pManager = server->getPlayerManager();
					if (pManager != nullptr) 
						playerLevel = pManager->calculatePlayerLevel(attacker) - 5;
				}
			}

			int intimidateDefense = (attacker->getSkillMod("intimidate_defense") / 1.5 + playerLevel) * attacker->getFrsMod("manipulation",1.25);
			int intimidateAccuracy = (100 + defender->getSkillMod("intimidate")) * defender->getFrsMod("manipulation",1.25);;

			int roll = System::random(100);

			// Infinity: Check intimidate new calcs MrObvious *******

			if (attackerPlayerOrPet) {
				attacker->updateCooldownTimer("fortifyintimidate", 8000);   // 8 second cooldown
			}
			if (roll <= intimidateAccuracy - intimidateDefense) { //This is a custom roll for accuracy
				data.getCommand()->applyEffect(defender, attacker, CommandEffect::INTIMIDATE, 0, 10);   //10 Seconds before defense reduction
				broadcastCombatSpam(defender, attacker, nullptr, 0, "cbt_spam", "fortify_hit", 1);
				damage *= 0.5;	//Need to reduce damage on initial attack that triggers Fortify
#ifdef DEBUG_CALCULATEDAMAGE
				debugCalculateDamage << " Damage after fortify (should only reduce on first hit): " << damage << "\n";
#endif // DEBUG_CALCULATEDAMAGE	
			}
			else {
					attacker->showFlyText("combat_effects", "intimidated_miss", 0xFF, 0, 0 );
			}
		}
    }

	// Infinity:  All other Infinity special force powers stuff is here
	if (data.isForceAttack()) {

		//Force Lightning adjustments
		if (defenderPlayerOrPet) {
			if (commandName.beginsWith("forcelightning")) {
				if (attacker->isPlayerCreature()) {
					damage *= 0.50;  // Reduce PVP lightning by 50% in PVP
				}
				else {
					damage *= .30; //70% damage reduction for NPCs using lightning abilities (after lightning "multiplier" moved to lua)
				}
#ifdef DEBUG_CALCULATEDAMAGE
				debugCalculateDamage << " Damage after Force lightning (50% PVP, 80% from NPC): " << damage << "\n";
#endif // DEBUG_CALCULATEDAMAGE	
			}
		}

		//Force Throw against player targets
		if (defenderPlayerOrPet) {
			if (commandName.beginsWith("forcethrow")) {
				if (attacker->isPlayerCreature()) {
					damage *= 0.50;  // Reduce PVP Force Throw by 50% in PVP
				}
				else {
					damage *= .30; //70% damage reduction for NPCs using Force Throw)
				}
#ifdef DEBUG_CALCULATEDAMAGE
				debugCalculateDamage << " Damage after Force throw reduction (50% PVP): " << damage << "\n";
#endif // DEBUG_CALCULATEDAMAGE	
			}
		}

		if (attacker->isPlayerCreature() && defenderPlayerOrPet && commandName.beginsWith("mindblast")) { 	//15% PVP reduction for mindblast
			damage *= .85;
#ifdef DEBUG_CALCULATEDAMAGE
			debugCalculateDamage << " Damage after PVP mindblast2 reduction: " << damage << "\n";
#endif // DEBUG_CALCULATEDAMAGE	
		}

		if (!attacker->isPlayerCreature() && defenderPlayerOrPet) {

			if (commandName.beginsWith("mindblast")) { // Reduce mindblast damage from NPCs against players by 60%
				damage *= .40;
	#ifdef DEBUG_CALCULATEDAMAGE
				debugCalculateDamage << " Damage after NPC vs. player mindblast reduction: " << damage << "\n";
	#endif // DEBUG_CALCULATEDAMAGE	
			}
			else if (commandName == "forcechoke") { // Reduce forceChoke damage from NPCs against players by 20%
				damage *= .80;
	#ifdef DEBUG_CALCULATEDAMAGE
				debugCalculateDamage << " Damage after NPC vs. player mindblast reduction: " << damage << "\n";
	#endif // DEBUG_CALCULATEDAMAGE	
			}
		}

		SkillManager* skillManager = SkillManager::instance();
		if (attacker->isPlayerCreature() && skillManager != nullptr){ //player is using force powers, apply cloak of pain buff
			uint32 buffCRC = BuffCRC::JEDI_CLOAKOFPAIN;
			Buff* buff = attacker->getBuff(buffCRC);
			int cloakOfPain = attacker->getSkillMod("cloakofpain");
			if (buff != nullptr && cloakOfPain <= 45)  //normal buff currently active, extend
				attacker->renewBuff(buffCRC, 8, true);
			else{ //New buff
				if (buff != nullptr && cloakOfPain > 45)
					attacker->removeBuff(buffCRC);
				ManagedReference<Buff*> cloakBuff = new Buff(attacker, buffCRC, 8, BuffType::JEDI);
				int cloakStrength = 7;
				cloakStrength += 2*skillManager->getSpecificSkillCount(attacker,"force_discipline_powers"); //Cloak strength scales with boxes purchased in powers
				if (cloakStrength == 43)  // Give 2 points of bonus at master box
					cloakStrength = 45;
				Locker locker(cloakBuff);
				cloakBuff->setSkillModifier("cloakofpain", cloakStrength);
				attacker->addBuff(cloakBuff);
				locker.release();
				attacker->showFlyText("combat_effects", "cloakofpain", 0, 255, 0); // +Cloak of Pain+
			}
		}

		float crystalBonus = 0.0;
		ManagedReference<WeaponObject*> wielded = attacker->getWeapon();

		if (attackerGhost != nullptr && wielded != nullptr && wielded->isJediWeapon()){
			ManagedReference<SceneObject*> saberInv = wielded->getSlottedObject("saber_inv");
			float damageAverage = 0.0;
			int i = 0;
			if (saberInv != nullptr) {
				for (i = 0; i < saberInv->getContainerObjectsSize(); i++) {
					ManagedReference<LightsaberCrystalComponent*> crystal = saberInv->getContainerObject(i).castTo<LightsaberCrystalComponent*>();
					if (crystal != nullptr) {
						if (crystal->getColor() == 31)
							damageAverage += crystal->getDamage();
					}
				}
			}

			i--; //reduce average by 1 to account for color crystal
			if (i < 1)
				i = 1;
			
			damageAverage /= i;

			if (damageAverage <= 50.0f) {
					crystalBonus = 0.60f + (damageAverage/ 100.0f);  // average 50 crystals will yield 10% bonus damage
			}
			else {
					crystalBonus = 1.10f + ((damageAverage - 50.0f) / 4.0f) / 100.0f; 
			}
		
			if (crystalBonus < 0.65) {
				crystalBonus = 0.65;  //up to a 35% penalty for poor crystals
			}

			damage *= Math::min(crystalBonus, 1.15f);  // Max bonus is always 15%

		}
		else {
			if (attackerGhost != nullptr)
				damage *= 0.65;   //35% penalty for no saber
		}

#ifdef DEBUG_CALCULATEDAMAGE
		debugCalculateDamage << " Damage after lightsaber crystal damage bonus for powers: " << damage << ".  Crystal bonus: "  << Math::min(crystalBonus, 1.15f) <<  ".\n";
#endif // DEBUG_CALCULATEDAMAGE

		// Infinity:  here we also return a portion of the pre-mitigation damage dealt by the attacker's damaging force attack
		if (attackerGhost != nullptr && attacker->getSkillMod("avoid_incapacitation") < 1) { // No Force return while using Avoid incap
			float cloakOfPain = attacker->getSkillMod("cloakofpain");						 // this skill mod scales with investment in force power, convenient to use
			int maxForce = attackerGhost->getForcePowerMax();
			int currentForce = attackerGhost->getForcePower();
			float siphon = damage * 0.0115f * (1 + (cloakOfPain / 4.0f) / 100); // This will be the amount of force returned

			/* Infinity:  Debug Siphon calcs
			StringBuffer msg;
			msg << "Force Siphon calcs" << endl;
			msg << " damage = " << damage << endl;
			msg << " initial siphon = " << siphon << endl;
			msg << " cloak of pain = " << cloakOfPain << endl;

			//*/

			if (wielded != nullptr) { // Reduced force power returned based on saber Force cost
				if (wielded->isJediWeapon()) {
					float fc = wielded->getForceCost(); // If you are Mpowers doing a 10k hit you get back 161 force after the -1 penalty for force cost
					if (fc < 1)
						fc = 1;
					if (fc > 20) // caps at 20 force penalty for very bad saber
						fc = 20;
					siphon -= fc; // This will deduct from the original cost.
				} else {
					siphon *= 0.80; // 20% penalty for not holding a lightsaber
				}
			} else {
				siphon *= 0.80; // 20% penalty for not holding a weapon
			}

			if (!((!defender->isPlayerCreature() && !defender->isPet()) && (attackerGhost->hasBhTef() || (attackerGhost->hasPvpTef() && attacker->getFactionStatus() == FactionStatus::OVERT)))) { // No siphon if target is not player creature and Teffed

				if (data.getCommand()->isAreaAction() || data.getCommand()->isConeAction()) { // AOE or Cone Attack
                    // Want to reduce siphon from Force Throw2 in all cases (PVE and PVP) except Powers Jedi being hunted by BH
					if (commandName == "forcethrow2" && !(defender->isPlayerCreature() && defender->hasSkill("combat_bountyhunter_investigation_03") && defender->hasBountyMissionFor(attacker))) {	
						siphon *= 0.5;
					}
                    if (!defender->isPlayerCreature() && !defender->isPet()) {
						siphon *= 1.0; // PVE
					} else {
						siphon *= 1.0; // PVP
					}
				} else { // Single target attack
					if (!defender->isPlayerCreature() && !defender->isPet()) {
						siphon *= 2.5; // PVE
					} else {
						siphon *= 2.0; // PVP
					}
				}

				siphon = ceil(siphon);
				if (currentForce + siphon < maxForce) {
					// msg << " Force siphon increasing force by " << siphon << ".";
					attackerGhost->setForcePower(currentForce + siphon);
				} else {
					// msg << " Force siphon setting force to max after attempting to add " << siphon << ".";
					attackerGhost->setForcePower(maxForce);
				}
			}

			// attacker->sendSystemMessage(msg.toString());  //Logging
		}
	}

	// Toughness reduction
	if (data.isForceAttack())
		damage = getDefenderToughnessModifier(defender, SharedWeaponObjectTemplate::FORCEATTACK, data.getDamageType(), damage);
	else
		damage = getDefenderToughnessModifier(defender, weapon->getAttackType(), weapon->getDamageType(), damage);

#ifdef DEBUG_CALCULATEDAMAGE
	debugCalculateDamage << " Damage after toughness reduction: " << damage << "\n";
#endif // DEBUG_CALCULATEDAMAGE

	// Infinity:  Modifying vanilla code here Force Defense skillmod damage reduction
	if (data.isForceAttack()) {
		int forceDefense = Math::min(defender->getSkillMod("force_defense"), 125);  				// Infinity:  125 Cap before FRS
		if (forceDefense > 0) {
			float adjustedForceDefense = (forceDefense/125.0f) * defender->getFrsMod("manipulation");
			float forceDefenseMitigation = damage * 0.45f * adjustedForceDefense;
			damage -= forceDefenseMitigation; 														// Infinity: force defense now protects vs force attacks directly and scales with manipulation
#ifdef DEBUG_CALCULATEDAMAGE
			debugCalculateDamage << " Damage after Force Defense: " << damage << ".  Force Defense = " << forceDefense << ".  FRS Manipulation mod = " << defender->getFrsMod("manipulation");
			debugCalculateDamage <<  ".  FRS-adjusted Force defense: " << adjustedForceDefense <<  ".  " << "forceDefenseMitgation:  " << forceDefenseMitigation << ".\n";
#endif // DEBUG_CALCULATEDAMAGE
		}
	}

	// Infinity:  FRS Mods on Intimidation
	if (attacker->isIntimidated() && attacker->isPlayerCreature()) {
		damage *= attacker->getFrsMod("control",2); // Infinity: Up to a 12.5% damage bypass for intimidate based on control score
#ifdef DEBUG_CALCULATEDAMAGE
		debugCalculateDamage << " Damage after attacker FRS intimidate bypass: " << damage << "\n";
#endif // DEBUG_CALCULATEDAMAGE	
	}

	if (attacker->isIntimidated() && defender->isPlayerCreature()) {
		damage /= defender->getFrsMod("power",2); //Infinity: Up to a 12.5% damage reduction for intimidate based on power score
#ifdef DEBUG_CALCULATEDAMAGE
		debugCalculateDamage << " Damage after defender FRS intimidate bonus: " << damage << "\n";
#endif // DEBUG_CALCULATEDAMAGE	
	}

	// Infinity:  Modified vanilla code to 75% PVP Damage Reduction for players and pets
	if (attackerPlayerOrPet && defenderPlayerOrPet) {
		if (defender->isPlayerCreature()) {
			damage *= 0.25;
		}
		else { // Defender is pet
			ManagedReference<PetControlDevice*> controlDevice = defender->getControlDevice().get().castTo<PetControlDevice*>();
			if (controlDevice != nullptr) {
				int petType = controlDevice->getPetType();
				if (petType == PetManager::CREATUREPET) {
					int petLevel = defender->getLevel();
					switch (petLevel) {
						case 80 ... 125:
							damage *= 0.50;	
							break;
						case 60 ... 79:
							damage *= 0.40;	
							break;
						case 30 ... 59:
							damage *= 0.35;	
							break;
						case 11 ... 29:
							damage *= 0.25;	
							break;
						default:
							damage *= 0.25;											
							break;
					}
				}
			}
		}
	}

#ifdef DEBUG_CALCULATEDAMAGE
	debugCalculateDamage << " Damage after PVP damage reduction: " << damage << "\n";
#endif // DEBUG_CALCULATEDAMAGE	

	//Infinity:  //Mark of Mandalore only works vs Jedi, Jedi is attacker here
	if (!data.isForceAttack() && defender->isPlayerCreature() && attacker->isPlayerCreature() && weapon != nullptr && weapon->isJediWeapon() & defenderMeleeWeapon & !defenderJediWeapon) { 
		int huntedLevel = attacker->getHuntedLevel();
		if (huntedLevel > 0 && defender->hasBuff(BuffCRC::SKILL_BUFF_HUNTER)) {
			damage *= .80f;   //Reduce damage of Jedi attacker by a flat 20% for ranged BH
#ifdef DEBUG_CALCULATEDAMAGE
			debugCalculateDamage << " Damage after Mark of Mandalore damage reduction (Jedi attacker vs. Melee BH): " << damage << "\n\n";
#endif // DEBUG_CALCULATEDAMAGE	
		}
	}

	if (damage < 1)
		damage = 1;

	debug() << "damage to be dealt is " << damage;

#ifdef DEBUG_CALCULATEDAMAGE
		debugCalculateDamage << " FINAL DAMAGE: " << damage << "\n";
		debugCalculateDamage << " ========= END Calculatedamage (creo vs. creo) Debug ========";
	if (attacker->isPlayerCreature()) {
		attacker->sendSystemMessage(debugCalculateDamage.toString());
	}
	if (defender->isPlayerCreature()) {
		defender->sendSystemMessage(debugCalculateDamage.toString());
	}
#endif // DEBUG_CALCULATEDAMAGE	

	return damage;
}

// Calculate Damage - Creo Attacker & Tano Defender
float CombatManager::calculateDamage(CreatureObject* attacker, WeaponObject* weapon, TangibleObject* defender, const CreatureAttackData& data) const {
	float damage = 0;
	int diff = 0;

	if (data.getMinDamage() > 0 && data.getMaxDamage() > 0) { // this is a special attack (force, etc)
		float minDmg = data.getMinDamage();
		float maxDmg = data.getMaxDamage();

		/* Infinity using our FRS Mods here
		if (data.isForceAttack() && attacker->isPlayerCreature())
			getFrsModifiedForceAttackDamage(attacker, minDmg, maxDmg, data);
		*/

		float mod = attacker->isAiAgent() ? cast<AiAgent*>(attacker)->getSpecialDamageMult() : 1.f;
		damage = minDmg * mod;
		diff = (maxDmg * mod) - damage;
	} else {
		float minDamage = weapon->getMinDamage(), maxDamage = weapon->getMaxDamage();

		if (attacker->isPlayerCreature() && !weapon->isCertifiedFor(attacker)) {
			minDamage = 5.f;
			maxDamage = 10.f;
		}

		damage = minDamage;
		diff = maxDamage - minDamage;
	}

	if (diff > 0)
		damage += System::random(diff);

	damage = applyDamageModifiers(attacker, weapon, damage, data);

	if (attacker->isPlayerCreature()) {// Infinity:  FRS damage bonuses here and standard player 1.5x damage bonus

		if (data.isForceAttack())
			damage *= attacker->getFrsMod("power",2);  //Infinity - FRS Bonus based on power mod
		else if (weapon->getDamageType() == SharedWeaponObjectTemplate::LIGHTSABER && weapon->getAttackType() == SharedWeaponObjectTemplate::MELEEATTACK)
			damage *= attacker->getFrsMod("manipulation",2);      // Infinity: Lightsaber damage scales with FRS manipulation mod in PVE/PVP like powers	

		// Infinity: Removing vanilla random damage increase on powers against NPCs.  All players get 1.5x damage bonus
		damage *= 1.5;
	}

	if (attacker->isPet()) { // Infinity: All pets receive a damage boost
		if (attacker->isWalkerSpecies() || attacker->isDroidSpecies()) { //Want to limit AT-ST and Droideka damage, also affects all droid pets
			damage *= 1.5;
		}
		else {
			damage *= 2.0;
		}
	}

	if (!data.isForceAttack() && weapon->getAttackType() == SharedWeaponObjectTemplate::MELEEATTACK)
		damage *= 1.25;

	debug() << "damage to be dealt is " << damage;

	ManagedReference<LairObserver*> lairObserver = nullptr;
	SortedVector<ManagedReference<Observer*>> observers = defender->getObservers(ObserverEventType::OBJECTDESTRUCTION);

	for (int i = 0; i < observers.size(); i++) {
		lairObserver = cast<LairObserver*>(observers.get(i).get());
		if (lairObserver != nullptr)
			break;
	}

	if (lairObserver && lairObserver->getSpawnNumber() > 2)
		damage *= 3.5;

	return damage;
}

// Calculate Damage - TanO attacker & CreO defender
float CombatManager::calculateDamage(TangibleObject* attacker, WeaponObject* weapon, CreatureObject* defender, const CreatureAttackData& data) const {
	float damage = 0;

	int diff = calculateDamageRange(attacker, defender, weapon);
	float minDamage = weapon->getMinDamage();

	if (diff > 0)
		damage = System::random(diff) + (int)minDamage;

	damage += defender->getSkillMod("private_damage_susceptibility");

	if (defender->isKnockedDown()) {// Infinity: Adjust damage to knocked down players as in creo to creo function

        //defender->sendSystemMessage("TC - Creature " + defender->getFirstName() + " is knocked down and receiving bonus damage.");

		if (defender->isPlayerCreature())
			damage *= 1.2f;
		else
			damage *= 1.5f;
	}

	// Toughness reduction
	damage = getDefenderToughnessModifier(defender, weapon->getAttackType(), weapon->getDamageType(), damage);

	return damage;
}

int CombatManager::calculateDamageRange(TangibleObject* attacker, CreatureObject* defender, WeaponObject* weapon) const {
	int attackType = weapon->getAttackType();
	int damageMitigation = 0;
	float minDamage = weapon->getMinDamage(), maxDamage = weapon->getMaxDamage();

	// restrict damage if a player is not certified (don't worry about mobs)
	if (attacker->isPlayerCreature() && !weapon->isCertifiedFor(cast<CreatureObject*>(attacker))) {
		minDamage = 5;
		maxDamage = 10;
	}

	debug() << "attacker base damage is " << minDamage << "-" << maxDamage;

	auto defenderGhost = defender->getPlayerObject();

	// this is for damage mitigation
	if (defenderGhost != nullptr) {
		String mitString = "none";
		bool melee = false;
		switch (attackType) {
		case SharedWeaponObjectTemplate::MELEEATTACK:
			mitString = "melee_damage_mitigation_";
			melee = true;
			break;
		case SharedWeaponObjectTemplate::RANGEDATTACK:
			mitString = "ranged_damage_mitigation_";
			break;
		default:
			break;
		}

		if (mitString != "none") {

			for (int i = 3; i > 0; i--) {

				if (defenderGhost->hasAbility(mitString + i)) {

					if (i == 1) {

						bool defenderJediWeapon = false;
						ManagedReference<WeaponObject*> defenderWeapon = defender->getWeapon();
						if (defenderWeapon != nullptr) {
							if (defenderWeapon->isJediWeapon()) {
								defenderJediWeapon = true;
							}
						}

						if (melee) {
							if (!defenderJediWeapon && !defender->hasSkill("combat_unarmed_speed_01") && !defender->hasSkill("combat_1hsword_ability_01") && !defender->hasSkill("combat_2hsword_ability_01") &&
									!defender->hasSkill("combat_polearm_ability_01") && !defender->hasSkill("combat_smuggler_combat_03")) {							
								continue;
							}
						}
						else {
							if (!defenderJediWeapon && !defender->hasSkill("combat_rifleman_speed_01") && !defender->hasSkill("combat_pistol_ability_01") && !defender->hasSkill("combat_carbine_ability_01") &&
									!defender->hasSkill("combat_smuggler_combat_01") && !defender->hasSkill("combat_bountyhunter_investigation_01") && !defender->hasSkill("combat_commando_heavyweapon_accuracy_01")) {
								continue;
							}
						}
					}

					damageMitigation = i;
					break;
				}
			}

			if (damageMitigation > 0) {
				maxDamage = minDamage + (maxDamage - minDamage) * (1 - (0.2 * damageMitigation));
			}
		}
	}

	float range = maxDamage - minDamage;

	debug() << "attacker weapon damage mod is " << maxDamage;

	return range < 0 ? 0 : (int)range;
}

float CombatManager::applyDamageModifiers(CreatureObject* attacker, WeaponObject* weapon, float damage, const CreatureAttackData& data) const {
	if (!data.isForceAttack()) {
		const auto weaponDamageMods = weapon->getDamageModifiers();

		for (int i = 0; i < weaponDamageMods->size(); ++i) {
			damage += attacker->getSkillMod(weaponDamageMods->get(i));
		}

		int attackType = weapon->getAttackType();

		if (attackType == SharedWeaponObjectTemplate::MELEEATTACK) // Berserk Bonus
			damage += attacker->getSkillMod("private_melee_damage_bonus");
		if (attackType == SharedWeaponObjectTemplate::RANGEDATTACK)
			damage += attacker->getSkillMod("private_ranged_damage_bonus");
	}

	damage += attacker->getSkillMod("private_damage_bonus");

	float damageMultiplier = attacker->getSkillMod("private_damage_multiplier");  	            //Infinity: Leave standard multiplier for other uses
	if (damageMultiplier != 0)
		damage *= damageMultiplier;

	damageMultiplier = attacker->getSkillMod("private_damage_multiplier_x100") / 100.f;  	    //Infinity: Doing X100 to get two decimal points for Feign Death Damage Buff
	if (damageMultiplier != 0)
		damage *= damageMultiplier;

	float damageDivisor = attacker->getSkillMod("private_damage_divisor_intimidate");  			//Infinity:  Intimidated State
	if (damageDivisor != 0)
		damage /= damageDivisor;

	damageDivisor = attacker->getSkillMod("private_damage_divisor_rocketboots");  				//Infinity:  Rocket Boots
	if (damageDivisor != 0)
		damage /= damageDivisor;

	damageDivisor = attacker->getSkillMod("private_damage_divisor_stun_x100") / 100.f;			//Infinity:  Stunned State
	if (damageDivisor != 0)
		damage /= damageDivisor;

	damageDivisor = attacker->getSkillMod("private_damage_divisor_forcerun3");					//Infinity:  Force Run 3
	if (damageDivisor != 0)
		damage /= damageDivisor;

	// States Damage Reduction
	float intimidateMod = attacker->getSkillMod("private_damage_divisor_intimidate");
	float stunMod = attacker->getSkillMod("private_damage_divisor_stun");
	float preDamage = damage;

#ifdef DEBUG_STATE_REDUCTION
	info(true) << "\n\nTotal Before Damage: " <<  damage << " Intimidate Mod: " << intimidateMod << " Stun Mod: " << stunMod;
#endif // DEBUG_STATE_REDUCTION

	if (intimidateMod > 0) {
		damage -= (preDamage * (intimidateMod / 100.f));
	}

#ifdef DEBUG_STATE_REDUCTION
	StringBuffer intimMsg;
	intimMsg << (((preDamage * (intimidateMod / 100.f)) / preDamage) * 100.f);

	info(true) << "Damage After Intimidate: " << damage << " Intimidate Reduction: " << ((intimidateMod > 0) ? intimMsg.toString() : "0") << "%";
#endif // DEBUG_STATE_REDUCTION

	if (stunMod > 0) {
		damage -= (preDamage * (stunMod / 100.f));
	}

#ifdef DEBUG_STATE_REDUCTION
	StringBuffer stunMsg;
	stunMsg << (((preDamage * (stunMod / 100.f)) / preDamage) * 100.f);

	info(true) << "Damage After Stun: " << damage << " Stun Reduction: " << ((stunMod > 0) ? stunMsg.toString() : "0") << "%\n\n";
#endif // DEBUG_STATE_REDUCTION

	return damage;
}

/*
void CombatManager::getFrsModifiedForceAttackDamage(CreatureObject* attacker, float& minDmg, float& maxDmg, const CreatureAttackData& data) const {
	ManagedReference<PlayerObject*> ghost = attacker->getPlayerObject();

	if (ghost == nullptr)
		return;

	FrsData* playerData = ghost->getFrsData();
	int councilType = playerData->getCouncilType();

	float minMod = 0, maxMod = 0;
	int powerModifier = 0;

	if (councilType == FrsManager::COUNCIL_LIGHT) {
		powerModifier = attacker->getSkillMod("force_power_light");
		minMod = data.getFrsLightMinDamageModifier();
		maxMod = data.getFrsLightMaxDamageModifier();
	} else if (councilType == FrsManager::COUNCIL_DARK) {
		powerModifier = attacker->getSkillMod("force_power_dark");
		minMod = data.getFrsDarkMinDamageModifier();
		maxMod = data.getFrsDarkMaxDamageModifier();
	}

	if (powerModifier > 0) {
		if (minMod > 0)
			minDmg += (int)((powerModifier * minMod) + 0.5);

		if (maxMod > 0)
			maxDmg += (int)((powerModifier * maxMod) + 0.5);
	}
}
*/

int CombatManager::calculatePoolsToDamage(int poolsToDamage) const {
	if (poolsToDamage & RANDOM) {
		int rand = System::random(100);

		if (rand <= 60) {
			poolsToDamage = HEALTH;
		} else if (rand <= 95) {
			poolsToDamage = ACTION;
		} else {
			poolsToDamage = MIND;
		}
	}

	return poolsToDamage;
}

int CombatManager::applyDamage(TangibleObject* attacker, WeaponObject* weapon, CreatureObject* defender, DefenderHitList* defenderHitList, int damage, float damageMultiplier, int poolsToDamage, uint8& hitLocation, const CreatureAttackData& data) const {
	if (poolsToDamage == 0 || damageMultiplier == 0 || defenderHitList == nullptr || weapon == nullptr) {
		return 0;
	}

	float ratio = weapon->getWoundsRatio();
	float healthDamage = 0.f, actionDamage = 0.f, mindDamage = 0.f;

	if (defender->isInvulnerable()) {
		return 0;
	}

	// Infinity: Death Watch pet damage transfer
	// DW players with CL95+ pets transfer some damage to their pet
	if (defender->isPlayerCreature() && defender->hasSkill("combat_death_watch_novice")) {
		ZoneServer* zs = defender->getZoneServer();
		if (zs != nullptr) {
			DeathWatchManager* dwMgr = zs->getDeathWatchManager();
			if (dwMgr != nullptr) {
				damage = dwMgr->applyPetDamageTransfer(defender, attacker, damage);
			}
		}
	}

	// Infinity: Death Watch mind immunity check (computed once, used for both direct and spill-over)
	// Only applies when BOTH attacker and defender are Death Watch players at war
	// and defender has the dw_mind_immunity buff active
	bool dwMindImmune = false;
	if (attacker != nullptr && attacker->isPlayerCreature() && defender->isPlayerCreature()) {
		CreatureObject* attackerCreature = attacker->asCreatureObject();
		if (attackerCreature != nullptr && defender->getSkillMod("dw_mind_immunity") > 0) {
			ZoneServer* zoneServer = defender->getZoneServer();
			if (zoneServer != nullptr) {
				DeathWatchManager* dwManager = zoneServer->getDeathWatchManager();
				if (dwManager != nullptr && dwManager->canPlayersAttack(attackerCreature, defender)) {
					// Both are DW and at war, and defender has immunity - block mind damage
					dwMindImmune = true;
				}
			}
		}
	}

	String xpType;
	if (data.isForceAttack()) {
		xpType = "jedi_general";
	} else if (attacker->isPet()) {
		xpType = "creaturehandler";
	} else {
		xpType = weapon->getXpType();
	}

	bool healthDamaged = (!!(poolsToDamage & HEALTH) && data.getHealthDamageMultiplier() > 0.0f);
	bool actionDamaged = (!!(poolsToDamage & ACTION) && data.getActionDamageMultiplier() > 0.0f);
	bool mindDamaged = (!!(poolsToDamage & MIND) && data.getMindDamageMultiplier() > 0.0f);

	if (data.getCommandName() == "forcechoke") //Infinity:  Special case for Force Choke which hits lowest HAM pool
	{
		int targetHamHighest = defender->getHAM(CreatureAttribute::HEALTH);
		int targetHAM = CreatureAttribute::HEALTH;
			healthDamaged = true; actionDamaged = false; mindDamaged = false;

		if (defender->getHAM(CreatureAttribute::ACTION) > targetHamHighest){
			targetHAM = CreatureAttribute::ACTION;
			targetHamHighest = defender->getHAM(CreatureAttribute::ACTION);
			healthDamaged = false; actionDamaged = true; mindDamaged = false;
		}
		if (defender->getHAM(CreatureAttribute::MIND) > targetHamHighest){
			healthDamaged = false; actionDamaged = false; mindDamaged = true;
			targetHAM = CreatureAttribute::MIND;
		}
	}

	int numberOfPoolsDamaged = (healthDamaged ? 1 : 0) + (actionDamaged ? 1 : 0) + (mindDamaged ? 1 : 0);
	Vector<int> poolsToWound;

#ifdef DEBUG_SPILL_DAMAGE
	StringBuffer spillOverDebug;
	spillOverDebug << " ========== Spill Over Debug ==========\n";
#endif

	int numSpillOverPools = 3 - numberOfPoolsDamaged;

	float spillMultPerPool = (0.0834f * numSpillOverPools) / Math::max(numberOfPoolsDamaged, 1);
	int totalSpillOver = 0; // Accumulate our total spill damage

#ifdef DEBUG_SPILL_DAMAGE
	spillOverDebug << " Number of Spill Over Pools: " << numSpillOverPools << "\n";
	spillOverDebug << " Spill Over Multiplier: " << spillMultPerPool << "\n";
#endif

	// from screenshots, it appears that food mitigation and armor mitigation were independently calculated
	// and then added together.
	int foodBonus = defender->getSkillMod("mitigate_damage");
	foodBonus > 100 ? foodBonus = 100 : foodBonus;

	int totalFoodMit = 0;
	float logDamage = 0.f;

	if (healthDamaged) {
		static const uint8 bodyLocations[] = {HIT_BODY, HIT_BODY, HIT_LARM, HIT_RARM};
		hitLocation = bodyLocations[System::random(3)];

		float damageMultiplied = damage * data.getHealthDamageMultiplier();

		logDamage += damageMultiplied;
		healthDamage = getArmorReduction(attacker, weapon, defender, defenderHitList, damageMultiplied, hitLocation, data) * damageMultiplier;

		int foodMitigation = 0;

		if (foodBonus > 0) {
			foodMitigation = (int)(healthDamage * foodBonus / 100.f);
		}

		healthDamage -= foodMitigation;
		totalFoodMit += foodMitigation;

#ifdef DEBUG_SPILL_DAMAGE
		spillOverDebug << " Non-Spill Health Damaged: " << healthDamage << "\n";
#endif

		int spilledDamage = (int)(healthDamage * spillMultPerPool); // Cut our damage by the spill percentage
		healthDamage -= spilledDamage;								// subtract spill damage from total damage
		totalSpillOver += spilledDamage;							// accumulate spill damage

#ifdef DEBUG_SPILL_DAMAGE
		spillOverDebug << " Health Spill Over Amount: " << spilledDamage << "\n";
#endif

		defender->inflictDamage(attacker, CreatureAttribute::HEALTH, (int)healthDamage, true, xpType, true, true);

		poolsToWound.add(CreatureAttribute::HEALTH);
	}

	if (actionDamaged) {
		static const uint8 legLocations[] = {HIT_LLEG, HIT_RLEG};
		hitLocation = legLocations[System::random(1)];

		float damageMultiplied = damage * data.getActionDamageMultiplier();

		logDamage += damageMultiplied;
		actionDamage = getArmorReduction(attacker, weapon, defender, defenderHitList, damageMultiplied, hitLocation, data) * damageMultiplier;

		int foodMitigation = 0;

		if (foodBonus > 0) {
			foodMitigation = (int)(actionDamage * foodBonus / 100.f);
		}

		actionDamage -= foodMitigation;
		totalFoodMit += foodMitigation;

#ifdef DEBUG_SPILL_DAMAGE
		spillOverDebug << " Non-Spill Action Damaged: " << actionDamage << "\n";
#endif

		int spilledDamage = (int)(actionDamage * spillMultPerPool);
		actionDamage -= spilledDamage;
		totalSpillOver += spilledDamage;

#ifdef DEBUG_SPILL_DAMAGE
		spillOverDebug << " Action Spill Over Amount: " << spilledDamage << "\n";
#endif

		defender->inflictDamage(attacker, CreatureAttribute::ACTION, (int)actionDamage, true, xpType, true, true);

		poolsToWound.add(CreatureAttribute::ACTION);
	}

	if (mindDamaged) {
		hitLocation = HIT_HEAD;

		// Infinity: DW mind immunity - skip all mind damage if defender has immunity buff
		// (dwMindImmune was computed at function start to be reused for spill-over)
		if (dwMindImmune) {
			mindDamage = 0;
			defender->sendSystemMessage("Your mental fortitude blocks the mind attack.");
			// TODO: Play client effect when one is created
			// defender->playEffect("clienteffect/dw_mind_block.cef", "");
		} else {
			float damageMultiplied = damage * data.getMindDamageMultiplier();

			logDamage += damageMultiplied;
			mindDamage = getArmorReduction(attacker, weapon, defender, defenderHitList, damageMultiplied, hitLocation, data) * damageMultiplier;

			int foodMitigation = 0;

			if (foodBonus > 0) {
				foodMitigation = (int)(mindDamage * foodBonus / 100.f);
			}

			mindDamage -= foodMitigation;
			totalFoodMit += foodMitigation;

#ifdef DEBUG_SPILL_DAMAGE
			spillOverDebug << " Non-Spill Mind Damaged: " << mindDamage << "\n";
#endif

			int spilledDamage = (int)(mindDamage * spillMultPerPool);
			mindDamage -= spilledDamage;
			totalSpillOver += spilledDamage;

#ifdef DEBUG_SPILL_DAMAGE
			spillOverDebug << " Mind Spill Over Amount: " << spilledDamage << "\n";
#endif

			defender->inflictDamage(attacker, CreatureAttribute::MIND, (int)mindDamage, true, xpType, true, true);

			poolsToWound.add(CreatureAttribute::MIND);
		}
	}

	if (numSpillOverPools > 0) {
#ifdef DEBUG_SPILL_DAMAGE
		spillOverDebug << " Total Spill Over Damage: " << totalSpillOver << "\n";
#endif

		int spillDamagePerPool = (int)(totalSpillOver / numSpillOverPools); // Split the spill over damage between the pools damaged
		int spillOverRemainder = (totalSpillOver % numSpillOverPools) + spillDamagePerPool;
		int spillToApply = (numSpillOverPools-- > 1 ? spillDamagePerPool : spillOverRemainder);

		if ((poolsToDamage ^ 0x7) & HEALTH) {
#ifdef DEBUG_SPILL_DAMAGE
			spillOverDebug << " Health Spill Over Damage: " << spillToApply << "\n";
#endif
			defender->inflictDamage(attacker, CreatureAttribute::HEALTH, spillToApply, true, xpType, true, true);
		}

		if ((poolsToDamage ^ 0x7) & ACTION) {
#ifdef DEBUG_SPILL_DAMAGE
			spillOverDebug << " Action Spill Over Damage: " << spillToApply << "\n";
#endif
			defender->inflictDamage(attacker, CreatureAttribute::ACTION, spillToApply, true, xpType, true, true);
		}

		// Infinity: DW mind immunity also blocks spill-over mind damage
		if (((poolsToDamage ^ 0x7) & MIND) && !dwMindImmune) {
#ifdef DEBUG_SPILL_DAMAGE
			spillOverDebug << " Mind Spill Over Damage: " << spillToApply << "\n";
#endif
			defender->inflictDamage(attacker, CreatureAttribute::MIND, spillToApply, true, xpType, true, true);
		}
	}

	int totalDamage = (int)(healthDamage + actionDamage + mindDamage);
	defender->notifyObservers(ObserverEventType::DAMAGERECEIVED, attacker, totalDamage);

	if (attacker->isPlayerCreature()) {
		showHitLocationFlyText(attacker->asCreatureObject(), defender, hitLocation);
	}

	defenderHitList->setInitialDamage(logDamage);
	defenderHitList->setHitLocation(hitLocation);
	defenderHitList->setFoodMitigation(totalFoodMit);
	defenderHitList->setPoolsToWound(poolsToWound);

#ifdef DEBUG_SPILL_DAMAGE
	spillOverDebug << " ========== END Spill Over Debug ==========\n";
	attacker->info(true) << spillOverDebug.toString();
#endif

	return totalDamage;
}

int CombatManager::applyDamage(CreatureObject* attacker, WeaponObject* weapon, TangibleObject* defender, DefenderHitList* defenderHitList, int poolsToDamage, const CreatureAttackData& data) const {
	if (defender == nullptr || defenderHitList == nullptr || poolsToDamage == 0) {
		return 0;
	}

	if (defender->getPvpStatusBitmask() == ObjectFlag::NONE) {
		return 0;
	}

	int damage = calculateDamage(attacker, weapon, defender, data);

	float damageMultiplier = data.getDamageMultiplier();

	if (damageMultiplier != 0)
		damage *= damageMultiplier;

	defenderHitList->setInitialDamage(damage);

	String xpType;
	if (data.isForceAttack())
		xpType = "jedi_general";
	else if (attacker->isPet())
		xpType = "creaturehandler";
	else
		xpType = weapon->getXpType();

	if (defender->isTurret()) {
		int damageType = 0, armorPiercing = 1;

		if (!data.isForceAttack()) {
			damageType = weapon->getDamageType();
			armorPiercing = weapon->getArmorPiercing();

			if (weapon->isBroken())
				armorPiercing = 0;
		} else {
			damageType = data.getDamageType();
		}

		int armorReduction = getArmorTurretReduction(attacker, defender, damageType);

		if (armorReduction >= 0)
			damage *= getArmorPiercing(defender, armorPiercing);

		if (armorReduction > 0)
			damage *= (1.f - (armorReduction / 100.f));
	}

	defender->inflictDamage(attacker, 0, damage, true, xpType, true, true);

	defender->notifyObservers(ObserverEventType::DAMAGERECEIVED, attacker, damage);

	return damage;
}

void CombatManager::woundCreatureTarget(CreatureObject* defender, WeaponObject* weapon, Vector<int> poolsToWound) const {
	if (defender == nullptr || poolsToWound.size() <= 0) {
		return;
	}

	Locker wlock(defender);

	float ratio = weapon->getWoundsRatio();

	if (System::random(100) < ratio) {
		int poolToWound = poolsToWound.get(System::random(poolsToWound.size() - 1));
		defender->addWounds(poolToWound, 1, true);
		defender->addWounds(poolToWound + 1, 1, true);
		defender->addWounds(poolToWound + 2, 1, true);
	}
}

/*
	Damage Over Time
*/

void CombatManager::applyDots(CreatureObject* attacker, CreatureObject* defender, const CreatureAttackData& data, int appliedDamage, int unmitDamage, int poolsToDamage) const {

	const Vector<DotEffect>* dotEffects = data.getDotEffects();

	if (attacker == nullptr || defender == nullptr)
		return;

	if (defender->isInvulnerable())
		return;

	if (defender->isAiAgent()) {
		AiAgent* defAgent = defender->asAiAgent();

		if (defAgent != nullptr && (defAgent->getCreatureBitmask() & ObjectFlag::NODOT))  // Infinity:  Really don't need this as it should be caught in hasDotImmunity
			return;
	}

	for (int i = 0; i < dotEffects->size(); i++) {
		const DotEffect& effect = dotEffects->get(i);

		if (defender->hasDotImmunity(effect.getDotType()) || effect.getDotDuration() == 0 || System::random(100) > effect.getDotChance())
			continue;

		const Vector<String>& defenseMods = effect.getDefenderStateDefenseModifiers();
		int resist = 0;

		for (int j = 0; j < defenseMods.size(); j++)
			resist += defender->getSkillMod(defenseMods.get(j));

		int damageToApply = appliedDamage;
		uint32 dotType = effect.getDotType();

		// Infinity:  Special creature checks
		bool creaturePet = false;
		if (attacker->isPet()) {
			ManagedReference<PetControlDevice*> controlDevice = attacker->getControlDevice().get().castTo<PetControlDevice*>();
			if (controlDevice != nullptr) {
				if (controlDevice->getPetType() == PetManager::CREATUREPET) {
					creaturePet = true;
				}
			}
		}

		if (effect.isDotDamageofHit()) {
			// determine if we should use unmitigated damage
			if (dotType != CreatureState::BLEEDING) { // Infinity:  Use pre-armor unmitigated damage for all DOTs except bleeds
				damageToApply = unmitDamage;
				// Infinity:  Special creature checks
				if (creaturePet && (dotType == CreatureState::POISONED || dotType == CreatureState::DISEASED))   //Infinity: Want to limit pet DOTs to something reasonable
					damageToApply *= 0.70f;  //30% reduction for now
			}
		}

		int potency = effect.getDotPotency();

		// Infinity:  Special creature checks
		if (potency == 0) {	// Default 150 potency

			if (attacker->isAiAgent()) {
				if (attacker->isPet())
					if (creaturePet)  // Only applies to creature pets
						potency = Math::max(50,attacker->getLevel() + 30);
					else
						potency = 150;
				else
					potency = Math::min(350, Math::max(100,attacker->getLevel()));  // DOT Potency from 100-350
			}
			else  // Players
				potency = 150;
		}

		uint8 pool = effect.getDotPool();

		if (pool == CreatureAttribute::UNKNOWN) {
			pool = getPoolForDot(dotType, poolsToDamage);
		}

		debug() << "entering addDotState with dotType:" << dotType;

		float damMod = attacker->isAiAgent() ? cast<AiAgent*>(attacker)->getSpecialDamageMult() : 1.f;

		int dotStrength = effect.getDotStrength() * damMod;  // Infinity:  Notice this value already includes effect DOT strength times damage mod

		String commandName = data.getCommandName();

		if (!attacker->isPlayerCreature() && commandName.beginsWith("forcelightning")) // Infinity:  No lightning fire DOTs for npcs
			continue;
	
		// Infinity:  Reduce dotStrength and damageToApply by FRS Manipulation mod
		dotStrength /= defender->getFrsMod("manipulation");
		damageToApply /= defender->getFrsMod("manipulation");

		float primaryPercent = effect.getPrimaryPercent();
		float secondaryPercent = effect.getSecondaryPercent();

		//Infinity: Buff to flamethrower DOTs in PVE only
		if (attacker->isPlayerCreature() && !(defender->isPlayerCreature() || defender->isPet()) && commandName.beginsWith("flame") ) { //Infinity:  Let's double DOT strength for flamethrower attacks in PVE
			primaryPercent *= 2.0f;
			secondaryPercent *= 2.0f;
		}

#ifdef DEBUG_APPLYDOTS

		// DOT Logging Begins Here

		StringBuffer msg;
		msg << endl << "DOT Logging from CombatManager::applyDots()" << endl;
		msg << "  Command: " << commandName << "  effect.isDotDamageofHit(): " << effect.isDotDamageofHit() << "  effect.getDotStrength(): " << effect.getDotStrength() << endl;
		msg << "  Unmitigated Damage: " << unmitDamage << "  Applied Damage: " << appliedDamage << "  Damage To Apply: " << damageToApply << endl;
		msg << "  effect.primaryPercent: " << effect.getPrimaryPercent() << "  Adjusted PrimaryPercent " << primaryPercent;
		msg << "  effect.secondaryPercent: " << effect.getSecondaryPercent() << "  Adjusted SecondaryPercent " << secondaryPercent << endl;

		String poolName;
		switch (pool) {
			case 0:
				poolName = "HEALTH";
				break;
			case 1:
				poolName = "STRENGTH";
				break;
			case 2:
				poolName = "CONSTITUTION";
				break;
			case 3:
				poolName = "ACTION";
				break;
			case 4:
				poolName = "QUICKNESS";
				break;
			case 5:
				poolName = "STAMINA";
				break;
			case 6:
				poolName = "MIND";
				break;
			case 7:
				poolName = "FOCUS";
				break;
			case 8:
				poolName = "WILLPOWER";
				break;
			default:
				poolName = "UNKNOWN";
				break;
		}

		String dotName;
		switch (dotType) {
			case CreatureState::POISONED: 
				dotName = "POISONED"; 
				break;
			case CreatureState::DISEASED: 
				dotName = "DISEASED";
				break;
			case CreatureState::BLEEDING: 
				dotName = "BLEEDING";
				break;
			case CreatureState::ONFIRE: 
				dotName = "ONFIRE";
				break;
			default:
				dotName = "UNKNOWN";
				break;
		}

		msg << "  DOT Type = " << dotName << ", Pool = " << poolName << ", Command CRC = " << data.getCommand()->getNameCRC();
		msg << ", Primary DOT = " << (effect.isDotDamageofHit() ? damageToApply * primaryPercent / 100.0f : dotStrength);
		msg << ", Secondary DOT = " << (effect.isDotDamageofHit() ? damageToApply * secondaryPercent / 100.0f : dotStrength);
		msg << ", potency = " << potency << ", resist = " << resist;

		if (attacker->isPlayerCreature())
			attacker->sendSystemMessage(msg.toString());
		error(msg.toString());

		//DOT Logging Ends Here //
#endif
		defender->addDotState(attacker, dotType, data.getCommand()->getNameCRC(), effect.isDotDamageofHit() ? damageToApply * primaryPercent / 100.0f : dotStrength, pool, effect.getDotDuration(), potency, resist,
							  effect.isDotDamageofHit() ? damageToApply * secondaryPercent / 100.0f : dotStrength);
	}
}

void CombatManager::applyWeaponDots(CreatureObject* attacker, CreatureObject* defender, WeaponObject* weapon) const {
	if (defender->isInvulnerable())
		return;

	if (!weapon->isCertifiedFor(attacker))
		return;

	if (defender->isAiAgent()) {
		AiAgent* defAgent = defender->asAiAgent();

		if (defAgent != nullptr && (defAgent->getCreatureBitmask() & ObjectFlag::NODOT))
			return;
	}

	for (int i = 0; i < weapon->getNumberOfDots(); i++) {
		if (weapon->getDotUses(i) <= 0)
			continue;

		int type = 0;
		int resist = 0;
		// utilizing this switch-block for easier *functionality* , present & future
		// SOE strings only provide this ONE specific type of mod (combat_bleeding_defense) and
		// there's no evidence (yet) of other 3 WEAPON dot versions also being resistable.
		switch (weapon->getDotType(i)) {
		case 1: // POISON
			type = CreatureState::POISONED;
			resist = defender->getSkillMod("resistance_poison") + defender->getSkillMod("poison_disease_resist");
			break;
		case 2: // DISEASE
			type = CreatureState::DISEASED;
			resist = defender->getSkillMod("resistance_disease") + defender->getSkillMod("poison_disease_resist");
			break;
		case 3: // FIRE
			type = CreatureState::ONFIRE;
			resist = defender->getSkillMod("resistance_fire") + defender->getSkillMod("fire_resist");
			break;
		case 4: // BLEED
			type = CreatureState::BLEEDING;
			resist = defender->getSkillMod("combat_bleeding_defense") + defender->getSkillMod("bleed_resist");
			break;
		default:
			break;
		}

		if (defender->hasDotImmunity(type))
			continue;

		uint64 attributeTypeMask = (weapon->getDotAttribute(i) & 0b1111) | ((type & 0b1111) << 4); //Infinity:  Limit to one DOT of each type per pool, rather than per weapon

		if (defender->addDotState(attacker, type, attributeTypeMask, weapon->getDotStrength(i), weapon->getDotAttribute(i), weapon->getDotDuration(i), weapon->getDotPotency(i), resist, (int)(weapon->getDotStrength(i) / 5.f)) > 0) {
			if (weapon->getDotUses(i) > 0)
				weapon->setDotUses(weapon->getDotUses(i) - 1, i); // Unresisted despite mod, reduce use count.
		}
	}
}

uint8 CombatManager::getPoolForDot(uint64 dotType, int poolsToDamage) const {
	uint8 pool = 0;

	switch (dotType) {
	case CreatureState::POISONED:
	case CreatureState::ONFIRE:
	case CreatureState::BLEEDING:
		if (poolsToDamage & HEALTH) {
			pool = CreatureAttribute::HEALTH;
		} else if (poolsToDamage & ACTION) {
			pool = CreatureAttribute::ACTION;
		} else if (poolsToDamage & MIND) {
			pool = CreatureAttribute::MIND;
		}
		break;
	case CreatureState::DISEASED:
		if (poolsToDamage & HEALTH) {
			pool = CreatureAttribute::HEALTH + System::random(2);
		} else if (poolsToDamage & ACTION) {
			pool = CreatureAttribute::ACTION + System::random(2);
		} else if (poolsToDamage & MIND) {
			pool = CreatureAttribute::MIND + System::random(2);
		}
		break;
	default:
		break;
	}

	return pool;
}

/*
	Accuracy and Modifiers
*/

float CombatManager::getWeaponRangeModifier(float currentRange, WeaponObject* weapon) const {
	float minRange = 0;
	float idealRange = 2;
	float maxRange = 5;

	float smallMod = 7;
	float bigMod = 7;

	minRange = (float)weapon->getPointBlankRange();
	idealRange = (float)weapon->getIdealRange();
	maxRange = (float)weapon->getMaxRange();

	smallMod = (float)weapon->getPointBlankAccuracy();
	bigMod = (float)weapon->getIdealAccuracy();

	if (currentRange >= maxRange)
		return (float)weapon->getMaxRangeAccuracy();

	if (currentRange <= minRange)
		return smallMod;

	// this assumes that we are attacking somewhere between point blank and ideal range
	float smallRange = minRange;
	float bigRange = idealRange;

	// check that assumption and correct if it's not true
	if (currentRange > idealRange) {
		smallMod = (float)weapon->getIdealAccuracy();
		bigMod = (float)weapon->getMaxRangeAccuracy();

		smallRange = idealRange;
		bigRange = maxRange;
	}

	if (bigRange == smallRange) // if they are equal, we know at least one is ideal, so just return the ideal accuracy mod
		return weapon->getIdealAccuracy();

	return smallMod + ((currentRange - smallRange) / (bigRange - smallRange) * (bigMod - smallMod));
}

int CombatManager::calculatePostureModifier(CreatureObject* creature, WeaponObject* weapon) const {

	if (creature == nullptr || weapon == nullptr) {
		return 0;
	}

	int accuracyWeapon = 0;
	int accuracyPosture = 0;

	uint8 posture = creature->getPosture();
	uint8 locomotion = creature->getLocomotion();

	uint32 attackType = weapon->getAttackType();
	uint32 weaponMask = weapon->getWeaponBitmask();

	CreaturePosture* postureLookup = CreaturePosture::instance();

	if (postureLookup == nullptr) {
		return 0;
	}

	if (!weapon->isMeleeWeapon()) {
		String weaponName = "";

		if (weaponMask == WeaponType::PISTOLWEAPON) {
			weaponName = "pistol";
		} else if (weaponMask == WeaponType::CARBINEWEAPON) {
			weaponName = "carbine";
		} else if (weaponMask == WeaponType::RIFLEWEAPON) {
			weaponName = "rifle";
		}

		if (weaponName != "") {
			uint8 speed = postureLookup->getSpeed(posture, locomotion);

			if (speed == CreatureLocomotion::SLOW || speed == CreatureLocomotion::FAST) {
				accuracyWeapon += creature->getSkillMod(weaponName + "_hit_while_moving");
			}

			if (posture == CreaturePosture::UPRIGHT) {
				accuracyWeapon += creature->getSkillMod(weaponName + "_accuracy_while_standing");
			}
		}

		accuracyPosture = postureLookup->getRangedAttackMod(locomotion);
	} else {
		accuracyPosture = postureLookup->getMeleeAttackMod(locomotion);

		if (creature->isAiAgent()) { //Infinity:  Give melee AI a bonus to hit while running to match Core3 code

			switch(locomotion) {

				case CreatureLocomotion::WALKING:			
					accuracyPosture = 10;
					break;

				case CreatureLocomotion::RUNNING:				
					accuracyPosture = 25;
					break;

				default:
					break;
			}
		}
	}

	int accuracyModifier = accuracyPosture * getWeaponPostureModifier(weaponMask);
	accuracyModifier += accuracyWeapon;

	if (accuracyModifier > 0 && accuracyPosture < 0) {
		accuracyModifier = 0;
	}

	return accuracyModifier;
}

int CombatManager::calculateTargetPostureModifier(WeaponObject* weapon, CreatureObject* targetCreature) const {
	CreaturePosture* postureLookup = CreaturePosture::instance();

	uint8 locomotion = targetCreature->getLocomotion();

	if (!weapon->isMeleeWeapon())
		return postureLookup->getRangedDefenseMod(locomotion);
	else
		return postureLookup->getMeleeDefenseMod(locomotion);
}

int CombatManager::getAttackerAccuracyModifier(TangibleObject* attacker, CreatureObject* defender, WeaponObject* weapon) const {

	if (attacker->isAiAgent()) {

		AiAgent* agent = cast<AiAgent*>(attacker);
		if (agent == nullptr) {
			error("AiAgent is nullptr.");
			return 0;
		}

		if (!attacker->isPet()) { //Infinity:  Not Pet
			return agent->getChanceHit() * 100.0f;
		}
		else { //Infinity:  aiAgent is some type of pet
			float petAccuracy = agent->getChanceHit() * 100.0f;
			if (!defender->isPlayerCreature() && !defender->isPet()) {  //Infinity:  Going to bump accuracy on low CL pets in PVE Only
				petAccuracy = Math::max(petAccuracy, 75.0f);   // Infinity:  Pets will have minimum 75 accuracy in PVE only
			}
			return petAccuracy;
		}
	}
	else if (attacker->isInstallationObject()) {//Check 
		return cast<InstallationObject*>(attacker)->getHitChance() * 100;
	}

	if (!attacker->isCreatureObject()) {
		return 0;
	}

	CreatureObject* creoAttacker = cast<CreatureObject*>(attacker);

	if (creoAttacker == nullptr) {
		return 0;
	}

	int attackerAccuracy = 0;

	const auto creatureAccMods = weapon->getCreatureAccuracyModifiers();

	for (int i = 0; i < creatureAccMods->size(); ++i) {
		const String& mod = creatureAccMods->get(i);
		attackerAccuracy += creoAttacker->getSkillMod(mod);
		attackerAccuracy += creoAttacker->getSkillMod("private_" + mod);
	}

	if (attackerAccuracy == 0)
		attackerAccuracy = -15; // unskilled penalty, TODO: this might be -50 or -125, do research

	// Infinity:  FS skill mods and account for buff stims
	if (weapon->getAttackType() == SharedWeaponObjectTemplate::MELEEATTACK) {
		attackerAccuracy += creoAttacker->getSkillModOfType("melee_accuracy",SkillModManager::SKILLBOX);
	}
	else if (weapon->getAttackType() == SharedWeaponObjectTemplate::RANGEDATTACK) {
		attackerAccuracy += creoAttacker->getSkillModOfType("ranged_accuracy",SkillModManager::SKILLBOX);
	}

	if (weapon != nullptr && weapon->getAttackType() == SharedWeaponObjectTemplate::MELEEATTACK && weapon->isJediWeapon() && creoAttacker->isPlayerCreature()) {

		float frsBonus = creoAttacker->getFrsMod("manipulation",0.62);
		int lightsaberAccuracyCap = 142;   // 125 + 12 + 5 = 142 for non-MLS / FRS.  Padawan can hit max of 125 + 12 = 137

		if (frsBonus == 1.0) { // Padawan MLS bonus
			if (creoAttacker->hasSkill("force_discipline_light_saber_master")) {  // MLS bonus accuracy to 147
					attackerAccuracy += 10;
			}
		}
		else if (frsBonus > 1.0) {   // Only applies to FRS jedi
			if (creoAttacker->hasSkill("force_discipline_light_saber_master"))
				lightsaberAccuracyCap = 147;

			/* Not needed, no Mando Jedi missions
			ManagedReference<PlayerObject*> defenderGhost = defender->getPlayerObject();
			if (defenderGhost != nullptr && defenderGhost->hasBhTef() && defender->hasSkill("mandalorian_master") && 
							!defender->hasSkill("combat_bountyhunter_investigation_03") &&
							(defender->hasSkill("combat_unarmed_master") || defender->hasSkill("combat_1hsword_master") || 
							defender->hasSkill("combat_2hsword_master") || defender->hasSkill("combat_polearm_master"))) {
				lightsaberAccuracyCap += 1 + (frsBonus - 1) * 14.88;   //This should give a max bonus of +5 at rank 11 against Mando hunter only
			}
			*/

			attackerAccuracy = round(attackerAccuracy * frsBonus); //bonus Accuracy for FRS manipulation

			if (!creoAttacker->isRallied()) {
				if (attackerAccuracy > lightsaberAccuracyCap)
					attackerAccuracy = lightsaberAccuracyCap; // Infinity:  cap pre food/other buffs for base skill+tapes+FS Accuracy
			}
			else  { // Infinity:  Player is rallied, +50 to accuracy so cap is higher by 50
				if (attackerAccuracy > (lightsaberAccuracyCap + 50))
					attackerAccuracy = lightsaberAccuracyCap + 50; // cap pre food/other buffs + 50 Rally bonus
			}
		}
	}

	// Infinity:  Buff stims
	if (weapon->getAttackType() == SharedWeaponObjectTemplate::MELEEATTACK) {
		attackerAccuracy += creoAttacker->getSkillModFromBuffs("melee_accuracy");
	}
	else if (weapon->getAttackType() == SharedWeaponObjectTemplate::RANGEDATTACK) {
		attackerAccuracy += creoAttacker->getSkillModFromBuffs("ranged_accuracy");
	}

	//Food + Dead Eye Prototype Bonus
	attackerAccuracy += creoAttacker->getSkillMod("attack_accuracy") + creoAttacker->getSkillModFromBuffs("dead_eye");

	return attackerAccuracy;
}

int CombatManager::getAttackerAccuracyBonus(CreatureObject* attacker, WeaponObject* weapon) const {
	int bonus = 0;

	bonus += attacker->getSkillMod("private_attack_accuracy");
	bonus += attacker->getSkillMod("private_accuracy_bonus");

	if (weapon != nullptr) {
		if (weapon->getAttackType() == SharedWeaponObjectTemplate::MELEEATTACK)
			bonus += attacker->getSkillMod("private_melee_accuracy_bonus");
		if (weapon->getAttackType() == SharedWeaponObjectTemplate::RANGEDATTACK)
			bonus += attacker->getSkillMod("private_ranged_accuracy_bonus");
	}

	return bonus;
}

/*
	Defenses
*/

int CombatManager::getDefenderDefenseModifier(CreatureObject* defender, WeaponObject* weapon, TangibleObject* attacker) const {
	int targetDefense = defender->getLevel();
	int buffDefense = 0;
	int cop = 0;

	if (defender->isPlayerCreature()) {
		cop = defender->getSkillMod("cloakofpain");
	}

	const auto defenseAccMods = weapon->getDefenderDefenseModifiers();

	for (int i = 0; i < defenseAccMods->size(); ++i) {
		const String& mod = defenseAccMods->get(i);
		targetDefense += defender->getSkillMod(mod);
		targetDefense += defender->getSkillMod("private_" + mod);
	}

	debug() << "Base target defense is " << targetDefense;

	if (cop > 0) {
		int checkForceArmor = defender->getSkillMod("force_armor");
		if (checkForceArmor > 0) {
			if (cop == 45)
				cop = 25;	//Minimum 25 at master powers
			else
				cop /=2;  // Reduced by 50% due to force armor
		}

		float copPenalty = (int)((float)cop/100 * Math::min(125,targetDefense));

		targetDefense -= copPenalty;
	}

	// defense hardcap
	if (targetDefense > 125)
		targetDefense = 125;

	if (attacker->isPlayerCreature())
		targetDefense += defender->getSkillMod("private_defense");

	if (targetDefense > 135)
		targetDefense = 135; // Infinity: Stronghold bonus only increases cap by 10, still benefits non capped players fully

	// SL bonuses go on top of hardcap
	for (int i = 0; i < defenseAccMods->size(); ++i) {
		const String& mod = defenseAccMods->get(i);
		targetDefense += defender->getSkillMod("private_group_" + mod);
	}

	// food bonus goes on top as well
	targetDefense += defender->getSkillMod("dodge_attack");
	targetDefense += defender->getSkillMod("private_dodge_attack");

	debug() << "Target defense after state affects and cap is " << targetDefense;

	return targetDefense;
}

int CombatManager::getDefenderSecondaryDefenseModifier(CreatureObject* defender) const {

	// Infinity:  Not going to totally negate secondary defense
	if (defender->isBerserked() || defender->isVehicleObject() || (defender->isIntimidated() && !defender->isPlayerCreature()))
		return 0;

	int targetDefense = defender->getLevel();
	ManagedReference<WeaponObject*> weapon = defender->getWeapon();

	const auto defenseAccMods = weapon->getDefenderSecondaryDefenseModifiers();

	for (int i = 0; i < defenseAccMods->size(); ++i) {
		const String& mod = defenseAccMods->get(i);
		targetDefense += defender->getSkillMod(mod);
		targetDefense += defender->getSkillMod("private_" + mod);
	}

	if (defender->isIntimidated())
			targetDefense *= 0.55f;  //Infinity:  Reduce secondary defenses by 55% when intimidated, should be players only at this point

	if (targetDefense > 125)
		targetDefense = 125;

	return targetDefense;
}

/*
	Hit Chance
*/

int CombatManager::getHitChance(TangibleObject* attacker, CreatureObject* creoDefender, WeaponObject* weapon, const CreatureAttackData& data, int damage, int bonusAccuracy) const {
	// AttackData
	float accuracyWeapon = 0.f;
	int accuracySkill = 0;
	int accuracyPosture = 0;
	int accuracyBonus = 0;
	int defenseSkill = 0;
	int defensePosture = 0;

	// DefendData
	int evadeSkill = 0;
	int evadeCenter = 0;

	// HitResult
	float accuracyTotal = 0.f;
	float defenseTotal = 0.f;
	float evadeTotal = 0.f;
	float toHitChance = 0.f;

	int hitResult = HitStatus::MISS;

	ManagedReference<CreatureObject*> creoAttacker = nullptr;
	if (attacker->isCreatureObject()) {
		creoAttacker = attacker->asCreatureObject();
	}

	// set AttackData
	if (data.isForceAttack()) {

		if (creoAttacker != nullptr) {
			//accuracySkill = creoAttacker->getSkillMod(data.getCommand()->getAccuracySkillMod());  //Infinity:  Not used on Infinity

			if (creoAttacker->isPlayerCreature()) {

				int lightsaberAccuracy = creoAttacker->getSkillMod("lightsaber_accuracy");
				if (lightsaberAccuracy < 110) {//If the accuracy is below 110, we are going to grant them the difference and adjust it with FRS mod.  We will store it in accuracyWeapon.
					accuracyWeapon = round((110.f - lightsaberAccuracy) * creoAttacker->getFrsMod("manipulation",0.62));
				}
			}

			if (weapon != nullptr) {
				accuracySkill += getAttackerAccuracyModifier(attacker, creoDefender, weapon);
			}
		}

		if (weapon != nullptr) {
			defenseSkill = getDefenderDefenseModifier(creoDefender, weapon, attacker);
			defensePosture = calculateTargetPostureModifier(weapon, creoDefender);
		}
		else {
			if (creoAttacker != nullptr) {

				StringBuffer msg;

				msg << "Nullptr weapon for Force Powers in getHitChance" << endl;
				msg << "Attacker Name:  " << creoAttacker->getFirstName() << endl;
				msg << "Attacker OID:  " << creoAttacker->getObjectID() << endl;

				if (creoAttacker->isAiAgent()) {

					ManagedReference<AiAgent*> agent = creoAttacker->asAiAgent();

					if (agent != nullptr) {
						const CreatureTemplate* creatureTemplate = agent->getCreatureTemplate();
						if (creatureTemplate != nullptr) {
							msg << "AI template:  " << creatureTemplate->getTemplateName();
						}
					}
				}
				else if (creoAttacker->isPlayerCreature()) {
					msg << "Player:  Yes";
				}

				error(msg.toString());

			}
			defenseSkill = creoDefender->getSkillMod("melee_defense");
		}
	}
	else {
		const Vector3& attackPosition = attacker->getWorldPosition();
		const Vector3& defendPosition = creoDefender->getWorldPosition();

		float distance = attackPosition.distanceTo(defendPosition);
		distance -= (attacker->getTemplateRadius() + creoDefender->getTemplateRadius());

		accuracyWeapon = getWeaponRangeModifier(distance, weapon);
		accuracySkill = getAttackerAccuracyModifier(attacker, creoDefender, weapon);

		if (creoAttacker != nullptr) {
			accuracyBonus = getAttackerAccuracyBonus(creoAttacker, weapon);
			accuracyBonus += bonusAccuracy;

			accuracyPosture = calculatePostureModifier(creoAttacker, weapon);

			if (weapon->getAttackType() == SharedWeaponObjectTemplate::RANGEDATTACK) {
				accuracyWeapon += creoAttacker->getSkillMod("private_aim");
			}

			if (creoDefender->isCreature()) {
				accuracyBonus += creoAttacker->getSkillMod("creature_hit_bonus");
			}
		}

		defenseSkill = getDefenderDefenseModifier(creoDefender, weapon, attacker);
		defensePosture = calculateTargetPostureModifier(weapon, creoDefender);
	}

	// set AttackResult
	accuracyTotal = accuracySkill + accuracyWeapon + accuracyPosture + accuracyBonus;
	defenseTotal = defenseSkill + defensePosture;

	toHitChance = hitChanceEquation(accuracyTotal, defenseTotal);

	if (System::random(100) <= toHitChance) {
		hitResult = HitStatus::HIT;
	}

	// set DefendData and DefendResult
	if (hitResult == HitStatus::HIT && !data.isForceAttack() && !data.isStateOnlyAttack()) {
		ManagedReference<WeaponObject*> defendWeapon = creoDefender->getWeapon().get();
		int defendMask = SharedWeaponObjectTemplate::UNARMEDWEAPON;

		if (defendWeapon != nullptr) {
			defendMask = defendWeapon->getWeaponBitmask();
		}

		int defendResult = getWeaponDefendResult(defendMask);

		if (defendResult == HitStatus::RICOCHET) {
			int attackMask = weapon->getWeaponBitmask();
			int attackType = weapon->getAttackType();

			// Infinity:   Saberblock will not work with armor equipped
			if (attackType == SharedWeaponObjectTemplate::RANGEDATTACK && !attacker->isTurret() && !weapon->isThrownWeapon() && defendWeapon != nullptr && defendWeapon->isJediWeapon() && !creoDefender->isWearingArmor()) {

                // Infinity:   This is all Infinity custom stuff for saberblock FRS mods
                int saberBlock = creoDefender->getSkillMod("saber_block");
                float saberFRSMod = creoDefender->getFrsMod("manipulation",0.33);  // Max 80 manipulation yields 1.505 value maximum
                int baseSaberBlock = saberBlock;
                //info("Saberblock skillmod = " + String::valueOf(saberBlock), true);

                saberBlock = ceil(saberBlock * saberFRSMod);   // Saber block adjusted for FRS for PVE

                if (saberBlock > 85) //Saber block maximum in PVE remains 85%, with 85% being the hard cap..
                    saberBlock = 85;

                if (saberBlock < 0)
                    saberBlock = 0;


                if (attacker->isPlayerCreature() && saberBlock > 50) { //saber block maximums in PVP.   80 Saberblock hard cap in PVP

                    saberFRSMod = creoDefender->getFrsMod("manipulation",0.50);	// Recalcuate FRS scaling for PVP
                    switch (baseSaberBlock) {
                        case 90:
                            saberBlock = ceil(Math::min(65 * saberFRSMod, 80.0f));	// MLS Rank 8 to 10
                            break;
                        case 85:
                            saberBlock = ceil(Math::min(60 * saberFRSMod, 80.0f));	// MLS Rank 7 and Below 
                            break;	
                        case 65:
                            saberBlock = ceil(Math::min(55 * saberFRSMod, 80.0f));	// non-MLS Rank 8 to 10
                            break;	
                        case 60:
                            saberBlock = ceil(Math::min(50 * saberFRSMod, 80.0f));	// non-MLS below Jedi Guardian up to 70 before MoM Debuff
                            break;
                        default:
                            saberBlock = ceil(Math::min((baseSaberBlock - 10) * saberFRSMod, 80.0f));	// PVP default is -10 from PVE / skill tree values												
                            break;
                    }

                    // Infinity:   This is all Infinity custom stuff for Mark of the Hunter
                    int huntedLevel = creoDefender->getHuntedLevel();

                    switch (huntedLevel) {
                        case 3:
                            saberBlock = ceil(saberBlock * 0.88f); 	// 12% reduction
                            break;
                        case 2:
                            saberBlock = ceil(saberBlock * 0.89f);	// 11% reduction
                            break;
                        case 1:
                            saberBlock = ceil(saberBlock * 0.90f);	// 10% reduction
                            break;
                        case 0:										// no debuff
                            break;
                        default:									// no debuff
                            break;
                    }
                }
                evadeTotal = evadeSkill = saberBlock;		

				if (evadeTotal > 0 && System::random(100) <= evadeTotal) {
					hitResult = HitStatus::RICOCHET;
				}
			}
		} else { // HitStatus::BLOCK, HitStatus::COUNTER, HitStatus::DODGE
			evadeSkill = getDefenderSecondaryDefenseModifier(creoDefender);

			if (evadeSkill != 0) {
				int attackRoll = System::random(499) + 1;
				int defendRoll = System::random(199) + 1;

				evadeCenter = creoDefender->getSkillMod("private_center_of_being");
				evadeTotal = evadeSkill + evadeCenter + defensePosture;

				if (accuracyTotal + attackRoll <= evadeTotal + defendRoll) {
					hitResult = defendResult;
				}
			}
		}
	}

#ifdef DEBUG_TOHIT
	float evadeChance = evadeTotal;

	if (evadeSkill != 0 && hitResult != HitStatus::RICOCHET) {
		evadeChance = ((evadeTotal + 100) / (accuracyTotal + 250.f)) * 0.5f;

		if (accuracyTotal > evadeTotal) {
			float rate = (accuracyTotal - evadeTotal) / 150.f;
			evadeChance *= 1.f - (rate > 1.f ? 1.f : rate);
		}
	}

	String r = "\\#882222\\";
	String g = "\\#228822\\";
	String b = "\\#222288\\";
	String a = "\\#444444\\";
	String h = hitResult == HitStatus::HIT ? g : hitResult == HitStatus::MISS ? r : b;

	StringBuffer msg;
	msg << "ToHitDebug                " << data.getCommandName() << endl
		<< a << "--------------------------------" << endl
		<< g << "  Attacker           " << attacker->getDisplayedName() << endl
		<< g << "    accuracyWeapon   " << accuracyWeapon << endl
		<< g << "    accuracySkill    " << accuracySkill << endl
		<< g << "    accuracyPosture  " << accuracyPosture << endl
		<< g << "    accuracyBonus    " << accuracyBonus << endl
		<< g << "  accuracyTotal      " << accuracyTotal << endl
		<< a << "--------------------------------" << endl
		<< r << "  Defender           " << creoDefender->getDisplayedName() << endl
		<< r << "    defenseSkill     " << defenseSkill << endl
		<< r << "    defensePosture   " << defensePosture << endl
		<< r << "  defenseTotal       " << defenseTotal << endl
		<< a << "--------------------------------" << endl
		<< b << "  Evasion            " << endl
		<< b << "    evadeSkill       " << evadeSkill << endl
		<< b << "    evadeCenter      " << evadeCenter << endl
		<< b << "  evadeTotal         " << evadeTotal << endl
		<< a << "--------------------------------" << endl
		<< h << "    toHitChance      " << int(toHitChance) << endl
		<< h << "    evadeChance      " << int(evadeChance * 100) << endl
		<< h << "  hitResult          " << (hitResult == HIT ? "HIT" : hitResult == MISS ? "MISS" : "EVADE") << endl
		<< a << "--------------------------------";

	if (attacker->isPlayerCreature()) {
		attacker->asCreatureObject()->sendSystemMessage(msg.toString());
	}

	if (creoDefender->isPlayerCreature()) {
		creoDefender->sendSystemMessage(msg.toString());
	}
#endif // DEBUG_TOHIT

	return hitResult;
}

float CombatManager::hitChanceEquation(float attackerAccuracy, float targetDefense) const {
	
	float roll = (attackerAccuracy - targetDefense) / toHitScale;
	float sign = (roll > 0.f) - (roll < 0.f);
	float toHit = toHitBase;

	for (int i = 1; i <= toHitStepMax; i++) {
		if ((roll * sign) > i) {
			toHit += sign * toHitStep;
			roll -= sign * i;
		} else {
			toHit += (roll / i) * toHitStep;
			break;
		}
	}

	if (toHit > toHitMax) {
		toHit = toHitMax;
	} else if (toHit < toHitMin) {
		toHit = toHitMin;
	}

	return toHit;
}

int CombatManager::getSpeedModifier(CreatureObject* attacker, WeaponObject* weapon) const {
	int speedMods = 0;

	if (weapon != nullptr) {
		const auto weaponSpeedMods = weapon->getSpeedModifiers();

		for (int i = 0; i < weaponSpeedMods->size(); ++i) {
			speedMods += attacker->getSkillMod(weaponSpeedMods->get(i));
		}
	}

	speedMods += attacker->getSkillMod("private_speed_bonus");

	if (weapon->getAttackType() == SharedWeaponObjectTemplate::MELEEATTACK) {
		speedMods += attacker->getSkillMod("private_melee_speed_bonus");
		speedMods += attacker->getSkillMod("melee_speed");
	} else if (weapon->getAttackType() == SharedWeaponObjectTemplate::RANGEDATTACK) {
		speedMods += attacker->getSkillMod("private_ranged_speed_bonus");
		speedMods += attacker->getSkillMod("ranged_speed");
	}

	return speedMods;
}

// Toughness Mitigation

float CombatManager::getDefenderToughnessModifier(CreatureObject* defender, int attackType, int damType, float damage) const {

	if (defender == nullptr) {
		return damage;
	}

	ManagedReference<WeaponObject*> weapon = defender->getWeapon();
	if (weapon == nullptr) {
		return damage;
	}

	const auto defenseToughMods = weapon->getDefenderToughnessModifiers();

	bool isWearingArmor = defender->isPlayerCreature() && defender->isWearingArmor();  // Infinity: Armor Check

	if (attackType == weapon->getAttackType()) {
		for (int i = 0; i < defenseToughMods->size(); ++i) {
			// Infinity check for armor to disable lightsaber toughness
			String toughModString = defenseToughMods->get(i);
			int toughMod = defender->getSkillMod(toughModString);
			if (toughModString == "lightsaber_toughness" && isWearingArmor)
				continue;
			if (toughMod > 0)
				damage *= 1.f - (toughMod / 100.f);
		}
	}

	int jediToughness = defender->getSkillMod("jedi_toughness");
	if (jediToughness > 0 && !isWearingArmor && damType != SharedWeaponObjectTemplate::LIGHTSABER && attackType != SharedWeaponObjectTemplate::FORCEATTACK)
		damage *= 1.f - (jediToughness / 100.f);

	return damage < 0 ? 0 : damage;
}

/*

	Armor Reduction and Calculations - Player, Ai, Turret, Vehcile

*/

ArmorObject* CombatManager::getArmorObject(CreatureObject* defender, uint8 hitLocation) const {
	Vector<ManagedReference<ArmorObject*>> armor = defender->getWearablesDeltaVector()->getArmorAtHitLocation(hitLocation);

	if (armor.isEmpty())
		return nullptr;

	return armor.get(System::random(armor.size() - 1));
}

ArmorObject* CombatManager::getPSGArmor(CreatureObject* defender) const {
	SceneObject* psg = defender->getSlottedObject("utility_belt");

	if (psg != nullptr && psg->isPsgArmorObject())
		return cast<ArmorObject*>(psg);

	return nullptr;
}

int CombatManager::getArmorObjectReduction(ArmorObject* armor, int damageType) const {
	float resist = 0;

	switch (damageType) {
	case SharedWeaponObjectTemplate::KINETIC:
		resist = armor->getKinetic();
		break;
	case SharedWeaponObjectTemplate::ENERGY:
		resist = armor->getEnergy();
		break;
	case SharedWeaponObjectTemplate::ELECTRICITY:
		resist = armor->getElectricity();
		break;
	case SharedWeaponObjectTemplate::STUN:
		resist = armor->getStun();
		break;
	case SharedWeaponObjectTemplate::BLAST:
		resist = armor->getBlast();
		break;
	case SharedWeaponObjectTemplate::HEAT:
		resist = armor->getHeat();
		break;
	case SharedWeaponObjectTemplate::COLD:
		resist = armor->getCold();
		break;
	case SharedWeaponObjectTemplate::ACID:
		resist = armor->getAcid();
		break;
	case SharedWeaponObjectTemplate::LIGHTSABER:
		resist = armor->getLightSaber();
		break;
	}

	return Math::max(0, (int)resist);
}

int CombatManager::getArmorNpcReduction(AiAgent* defender, int damageType) const {
	float resist = 0;

	switch (damageType) {
	case SharedWeaponObjectTemplate::KINETIC:
		resist = defender->getKinetic();
		break;
	case SharedWeaponObjectTemplate::ENERGY:
		resist = defender->getEnergy();
		break;
	case SharedWeaponObjectTemplate::ELECTRICITY:
		resist = defender->getElectricity();
		break;
	case SharedWeaponObjectTemplate::STUN:
		resist = defender->getStun();
		break;
	case SharedWeaponObjectTemplate::BLAST:
		resist = defender->getBlast();
		break;
	case SharedWeaponObjectTemplate::HEAT:
		resist = defender->getHeat();
		break;
	case SharedWeaponObjectTemplate::COLD:
		resist = defender->getCold();
		break;
	case SharedWeaponObjectTemplate::ACID:
		resist = defender->getAcid();
		break;
	case SharedWeaponObjectTemplate::LIGHTSABER:
		resist = defender->getLightSaber();
		break;
	}

	return (int)resist;
}

int CombatManager::getArmorVehicleReduction(VehicleObject* defender, int damageType) const {
	float resist = 0;

	switch (damageType) {
	case SharedWeaponObjectTemplate::KINETIC:
		resist = defender->getKinetic();
		break;
	case SharedWeaponObjectTemplate::ENERGY:
		resist = defender->getEnergy();
		break;
	case SharedWeaponObjectTemplate::ELECTRICITY:
		resist = defender->getElectricity();
		break;
	case SharedWeaponObjectTemplate::STUN:
		resist = defender->getStun();
		break;
	case SharedWeaponObjectTemplate::BLAST:
		resist = defender->getBlast();
		break;
	case SharedWeaponObjectTemplate::HEAT:
		resist = defender->getHeat();
		break;
	case SharedWeaponObjectTemplate::COLD:
		resist = defender->getCold();
		break;
	case SharedWeaponObjectTemplate::ACID:
		resist = defender->getAcid();
		break;
	case SharedWeaponObjectTemplate::LIGHTSABER:
		resist = defender->getLightSaber();
		break;
	}

	return (int)resist;
}

// Armor mitigation

int CombatManager::getArmorReduction(TangibleObject* attacker, WeaponObject* weapon, CreatureObject* defender, DefenderHitList* hitList, float damage, int hitLocation, const CreatureAttackData& data) const {
	int damageType = 0, armorPiercing = 1;

	if (hitList == nullptr) {
		return 0;
	}

	if (!data.isForceAttack()) {
		damageType = weapon->getDamageType();
		armorPiercing = weapon->getArmorPiercing();

		if (weapon->isBroken())
			armorPiercing = 0;
	} else {
		damageType = data.getDamageType();
	}

	bool defenderPlayerOrPet = defender->isPlayerCreature() || defender->isPet();
    bool forceBreached = defender->hasBuff(BuffCRC::JEDI_FORCEBREACH);

	if (attacker->isPlayerCreature() && !defenderPlayerOrPet && data.getCommandName().beginsWith("forcelightning"))    //Infinity:  Lightning is LS Damage in PVE (not player/pet PVP)
		damageType = SharedWeaponObjectTemplate::LIGHTSABER;

	// Infinity: DW pets inherit owner's weapon damage type against DW targets
	if (attacker->isPet() && defender->hasSkill("combat_death_watch_novice")) {
		CreatureObject* petCreature = attacker->asCreatureObject();
		if (petCreature != nullptr) {
			ManagedReference<CreatureObject*> petOwner = petCreature->getLinkedCreature();
			if (petOwner != nullptr && petOwner->hasSkill("combat_death_watch_novice")) {
				WeaponObject* ownerWeapon = petOwner->getWeapon();
				if (ownerWeapon != nullptr) {
					damageType = ownerWeapon->getDamageType();
				}
			}
		}
	}

	if (defender->isAiAgent()) {
		float armorReduction = getArmorNpcReduction(cast<AiAgent*>(defender), damageType);

		if (armorReduction >= 0) {
			//Infinity:  Reduce armor piercing by one level from NPC's against pets when AP > 1 and pet has light armor (AR1)
			if (armorPiercing > 1 && defender->isPet() && attacker->isAiAgent() && !attacker->isPet() && cast<AiAgent*>(defender)->getArmor() == 1) {  
				armorPiercing--;
			}

            if (forceBreached && !defender->isPet()) { //Force Breach increases AP by 1 against PVE targets
                armorPiercing++;
            }

            if (attacker->isPlayerCreature() && defender->isPet() && data.getCommandName().beginsWith("flame")) {
                armorPiercing++;		// Infinity:  increase AP by 1 to compensate for 25% damage nerf, equals less damage for unarmored targets, but same for armored PVP targets
            }

            damage *= getArmorPiercing(cast<AiAgent*>(defender), armorPiercing);
		}

		if (armorReduction > 0) {
			damage *= (1.f - (armorReduction / 100.f));
        }

		return damage;

	} else if (defender->isVehicleObject()) {
		float armorReduction = getArmorVehicleReduction(cast<VehicleObject*>(defender), damageType);

		if (armorReduction >= 0)
			damage *= getArmorPiercing(cast<VehicleObject*>(defender), armorPiercing);

		if (armorReduction > 0)
			damage *= (1.f - (armorReduction / 100.f));

		return damage;
	}

	float jediMit = hitList->getJediMitigation();

	bool wearingArmor = defender->isPlayerCreature() && defender->isWearingArmor();
	ManagedReference<PlayerObject*> defenderGhost = defender->getPlayerObject();

	if (!data.isForceAttack()) {
		// Force Armor
		float rawDamage = damage;

        //Infinity:  FRS Mods
		float forceArmor = defender->getSkillMod("force_armor") * defender->getFrsMod("manipulation");  //Infinity:  Force Armor scales with manipulation mod

		if (forceArmor > 0 && !wearingArmor) {  // Infinity:  Force armor does not function with armor equipped
			float dmgAbsorbed = rawDamage - (damage *= 1.f - (forceArmor / 100.f));
			defender->notifyObservers(ObserverEventType::FORCEARMOR, attacker, dmgAbsorbed);

			jediMit += dmgAbsorbed;
			hitList->setJediMitigation(jediMit);
		}
	} else {
		float jediBuffDamage = 0;
		float rawDamage = damage;

		// Force Shield  Infinity updates for FRS
		float forceShield = defender->getSkillMod("force_shield") * defender->getFrsMod("control");   //Infinity:  Force Shield scales with manipulation mod

		if (forceShield > 0 && !wearingArmor) { // Infinity:  Shield does not function with armor equipped
			jediBuffDamage = rawDamage - (damage *= 1.f - (forceShield / 100.f));
			defender->notifyObservers(ObserverEventType::FORCESHIELD, attacker, jediBuffDamage);

			jediMit += jediBuffDamage;
			hitList->setJediMitigation(jediMit);
		}

		// Force Feedback
 	    float forceFeedback = defender->getSkillMod("force_feedback") * defender->getFrsMod("power");  //Infinity:  Force Feedback scales with power mod

		if (forceFeedback > 0 && (defender->hasBuff(BuffCRC::JEDI_FORCE_FEEDBACK_1) || defender->hasBuff(BuffCRC::JEDI_FORCE_FEEDBACK_2)) && !wearingArmor) { // Infinity:  Won't work with armor equipped
			float feedbackDmg = rawDamage * (forceFeedback / 100.f);

			if (attacker->isPlayerCreature() && defender->isPlayerCreature()) {    // PVP
				feedbackDmg *= .50;   // Infinity:  In PVP, reduce Force Feedback damage by 50%
				if (forceBreached) {       // Infinity:  Force Breach Reduces force feedback damage by 60%
					feedbackDmg *= .4;
				}
			}

			ManagedReference<CreatureObject*> creoAttacker = attacker->asCreatureObject();
			if (creoAttacker != nullptr && creoAttacker->isPlayerCreature()) {

				//Infinity:  Attacker Force Shield reduction of feedback damage
				float attackerShield = creoAttacker->getSkillMod("force_shield");
				if (attackerShield > 0)
					feedbackDmg *= 1.0f - (attackerShield / 100.f * creoAttacker->getFrsMod("control"));

				//Infinity:  Attacker Force Defense reduction of feedback damage
				float attackerForceDefense = creoAttacker->getSkillMod("force_defense");
				if (attackerForceDefense > 0)
					feedbackDmg *= 1.0f - 0.45f * (attackerForceDefense / 125.0f) * creoAttacker->getFrsMod("manipulation");
			}

			float splitDmg = feedbackDmg / 3;

			attacker->inflictDamage(defender, CreatureAttribute::HEALTH, splitDmg, true, true, true);
			attacker->inflictDamage(defender, CreatureAttribute::ACTION, splitDmg, true, true, true);
			attacker->inflictDamage(defender, CreatureAttribute::MIND, splitDmg, true, true, true);
			defender->notifyObservers(ObserverEventType::FORCEFEEDBACK, attacker, feedbackDmg);
			defender->playEffect("clienteffect/pl_force_feedback_block.cef", "");
			hitList->setForceFeedback(feedbackDmg);
		}

		// Force Absorb
		int absorbSkill = defender->getSkillMod("force_absorb");
		float absorbDamage = rawDamage;
		if (absorbSkill > 0 && absorbDamage > 0 && defender->isPlayerCreature() && !forceBreached && defender->getSkillMod("avoid_incapacitation") < 1 && !wearingArmor) {
			if (attacker->isPlayerCreature()) {
				bool isPowersHealer = defender->hasSkill("force_discipline_healing_master") && defender->hasSkill("force_discipline_powers_master");
				if (!isPowersHealer || (isPowersHealer && defender->checkCooldownRecovery("pvp_force_absorb"))) {
					absorbDamage *= 0.20f; // Infinity:  nerf absorbed force vs PC by 80%
					if (absorbSkill == 2 && absorbDamage > 85) {
						absorbDamage = 85; // Infinity: PVP hardcap
					} else if (absorbSkill == 1 && absorbDamage > 40) {
						absorbDamage = 40; // Infinity: PVP hardcap
					}
					defender->updateCooldownTimer("pvp_force_absorb", 333); // Cooldown on Master Powers/Master Healer to prevent unlimited force (3 times per second)
				} else {
					absorbDamage = 0;
				}
			} else {
				if (absorbSkill == 2) {
					absorbDamage *= 0.6f; // Infinity:  Reduction of 40% from NPCs with Force Absorb 2
				} else if (absorbSkill == 1) {
					absorbDamage *= 0.4f; // Infinity:  Reduction of 60% from NPCs with Force Absorb 1
				}
			}

			if (absorbDamage > 0) {
				defender->notifyObservers(ObserverEventType::FORCEABSORB, attacker, absorbDamage);
				hitList->setForceAbsorb(absorbDamage);
			}
		}
	}

	// Infinity:  Cloak of Pain
	if (defender->isPlayerCreature() && !wearingArmor) {    // Calculating Damage Reduction from Cloak of Pain Buff
		float cloakOfPain = defender->getSkillMod("cloakofpain");
		if (cloakOfPain > 0){ //defender has cloak of pain

			//Cloak of pain is less effective with force shield and force armor
			if (data.isForceAttack() && defender->getSkillMod("force_shield") > 0) {
				if (cloakOfPain == 45)
					cloakOfPain = 25;   //Minimum 25 at master powers
				else
					cloakOfPain /= 2.0;
			}
			if (!data.isForceAttack() && defender->getSkillMod("force_armor") > 0) {
				if (cloakOfPain == 45)
					cloakOfPain = 25;	//Minimum 25 at master powers
				else
					cloakOfPain /= 2.0;
			}
			
			if (forceBreached) {	//Infinity:   Breach reduces Cloak of Pain damage reduction by 25%

				float debuffPercentage = 0.20f;   //Infinity:   Cloak of Pain reduced by 20% if breached

                /*  Removed on May 19, 2025
				if (defender->hasSkill("force_discipline_healing_master")) { //Master healers get 50% reduction in breach
					debuffPercentage *= 0.50;
				}
                */

				cloakOfPain *= (1.0f - debuffPercentage);              
			}

			cloakOfPain *= defender->getFrsMod("manipulation");  //Infinity:  Cloak of pain scales with manipulation mod
			float dmgAbsorbed = damage;

			damage *= 1.f - (cloakOfPain / 100.f);
			dmgAbsorbed -= damage;
			int copMit = hitList->getCopMitigation();
			copMit += dmgAbsorbed;
			hitList->setCopMitigation(copMit);
		}
	}

	// PSG
	ManagedReference<ArmorObject*> psg = getPSGArmor(defender);

	if (psg != nullptr && !psg->isVulnerable(damageType)) {
		float armorReduction = getArmorObjectReduction(psg, damageType);

		if (armorReduction > 46) {
			armorReduction = 46; //Infinity:  Hardcap for legacy PSGS
        }

		if (forceBreached) {
		    armorReduction *= .75; // Infinity:  Force breach bypasses 25% of your PSG
        }

		float dmgAbsorbed = damage;
		float initialDamage = damage;

		// damage *= getArmorPiercing(psg, armorPiercing);  // Infinity:  Armor piercing will not work against PSG's)

		if (armorReduction > 0) {
			damage *= 1.f - (armorReduction / 100.f);
        }

		dmgAbsorbed -= damage;
		if (dmgAbsorbed > 0) {  // Infinity:  Custom code to reduce PSG damage
			if (defender->checkCooldownRecovery("psg_damaged")) {
				float psgMultiplier = 0;
				if (attacker->isPlayerCreature()) {
					if (defenderGhost != nullptr && defenderGhost->hasBhTef() && (defender->hasSkill("combat_bountyhunter_investigation_03") || defender->hasSkill("mandalorian_master")) && (weapon->isJediWeapon() || data.isForceAttack())) {
						psgMultiplier = 1.25 / 100.f;		//BH with BH Tef being attacked by Jedi, 1.25% of initial damage in BH vs. Jedi Bounty PVP
					}
					else {  
						psgMultiplier = 0.45 / 100.f;		//All PVP not Jedi attacking BH with Bounty, .45% of initial damage
					}
				}
				else {	
					psgMultiplier = 0.125 / 100.f;			// 0.125% of initial damage in PVE		
				}

				float psgDamage = initialDamage * psgMultiplier;
				Locker plocker(psg);
				psg->inflictDamage(psg, 0, psgDamage, true, true);
				plocker.release();

				defender->updateCooldownTimer("psg_damaged", 1000);	//anti burst mechanic for PSGs, wont take damage more often than once per second
			}

			int psgMit = hitList->getPsgMitigation();
			psgMit += dmgAbsorbed;
			hitList->setPsgMitigation(psgMit);
		}
	}

	// Standard Armor
	ManagedReference<ArmorObject*> armor = nullptr;

	armor = getArmorObject(defender, hitLocation);

	if (armor != nullptr && !armor->isVulnerable(damageType)) {
		float armorReduction = getArmorObjectReduction(armor, damageType);
		float dmgAbsorbed = damage;
		float preArmorDamage = damage;

		if (attacker->isPlayerCreature() && data.getCommandName().beginsWith("forcelightning")) //Infinity:  Force Lightning now has inherent AP2 for players only
			armorPiercing = 2;	

		if (data.getCommandName() == "forcechoke" && attacker->isPlayerCreature())	//Infinity:  Force Choke now has inherent AP2 for players only
			armorPiercing = 2;

		if (attacker->isPlayerCreature() && defender->isPlayerCreature() && data.getCommandName().beginsWith("flame")) {
			armorPiercing++;		// Infinity:  increase AP by 1 to compensate for 25% damage nerf, equals less damage for unarmored targets, but same for armored PVP targets
        }

		if (data.isForceAttack() && attacker->isPlayerCreature()) {  //Cap on resists versus player force powers in PVP
			if (defender->isPlayerCreature() && armorReduction > 80) { //80% resist cap vs force powers in PVP
				armorReduction = 80;
			}

			ManagedReference<CreatureObject*> attackerCreature = attacker->asCreatureObject();
			if (attackerCreature != nullptr && defender->isPlayerCreature() && (defender->hasSkill("combat_bountyhunter_investigation_03") || defender->hasSkill("mandalorian_master")) && attackerCreature->hasSkill("force_discipline_powers_master")) {

				if (armorReduction > 70 && data.getCommandName().beginsWith("forcethrow")) {
					armorReduction = 70;
				}
				else if (armorReduction > 60) {
					armorReduction = 60;
				}
			}
		}
		
		if (damageType == SharedWeaponObjectTemplate::LIGHTSABER && defender->isPlayerCreature() && armorReduction > 39.0f) { //39% hardcap for Mando LS resist
			armorReduction = 39.0f;
		}

		// use only the damage applied to the armor for piercing (after the PSG takes some off)
		damage *= getArmorPiercing(armor, armorPiercing);

		if (armorReduction > 0) {
			damage *= (1.f - (armorReduction / 100.f));

			if (preArmorDamage < damage && defender->isPlayerCreature()) { // Infinity:  players cannot take more damage after armorpiercing and resists than you would have unarmored.
				damage = preArmorDamage;
			}
			dmgAbsorbed -= damage;

			int armorMit = hitList->getArmorMitigation();
			armorMit += dmgAbsorbed;
			hitList->setArmorMitigation(armorMit);
		}

		// inflict condition damage
		Locker alocker(armor);

		// Infinity:  Damage multiplier reduces from 0.2 to 0.1
		armor->inflictDamage(armor, 0, damage * 0.1, true, true);
	}

	return damage;
}

int CombatManager::getArmorTurretReduction(CreatureObject* attacker, TangibleObject* defender, int damageType) const {
	int resist = 0;

	if (defender == nullptr || !defender->isTurret()) {
		return resist;
	}

	auto turretDefender = cast<TurretObject*>(defender);

	if (turretDefender == nullptr) {
		return resist;
	}

	switch (damageType) {
		case SharedWeaponObjectTemplate::KINETIC:
			resist = turretDefender->getKinetic();
			break;
		case SharedWeaponObjectTemplate::ENERGY:
			resist = turretDefender->getEnergy();
			break;
		case SharedWeaponObjectTemplate::ELECTRICITY:
			resist = turretDefender->getElectricity();
			break;
		case SharedWeaponObjectTemplate::STUN:
			resist = turretDefender->getStun();
			break;
		case SharedWeaponObjectTemplate::BLAST:
			resist = turretDefender->getBlast();
			break;
		case SharedWeaponObjectTemplate::HEAT:
			resist = turretDefender->getHeat();
			break;
		case SharedWeaponObjectTemplate::COLD:
			resist = turretDefender->getCold();
			break;
		case SharedWeaponObjectTemplate::ACID:
			resist = turretDefender->getAcid();
			break;
		case SharedWeaponObjectTemplate::LIGHTSABER:
			resist = turretDefender->getLightSaber();
			break;
		default:
			break;
	}

	return resist;
}

float CombatManager::getArmorPiercing(TangibleObject* defender, int armorPiercing) const {
	int armorReduction = 0;

	if (defender->isAiAgent()) {
		AiAgent* aiDefender = cast<AiAgent*>(defender);
		armorReduction = aiDefender->getArmor();
	} else if (defender->isArmorObject()) {
		ArmorObject* armorDefender = cast<ArmorObject*>(defender);

		if (armorDefender != nullptr && !armorDefender->isBroken())
			armorReduction = armorDefender->getRating();
	} else if (defender->isVehicleObject()) {
		VehicleObject* vehicleDefender = cast<VehicleObject*>(defender);
		armorReduction = vehicleDefender->getArmor();
	} else if (defender->isTurret()) {
		auto turret = cast<TurretObject*>(defender);

		if (turret != nullptr) {
			armorReduction = turret->getArmorRating();
		}
	}

	if (armorPiercing > armorReduction)
		return pow(1.25, armorPiercing - armorReduction);
	else
		return pow(0.50, armorReduction - armorPiercing);
}

// Mine & Bomb Droid Detonation
float CombatManager::doObjectDetonation(TangibleObject* attackerTanO, CreatureObject* defender, float damage, WeaponObject* weapon) const {
	if (attackerTanO == nullptr || defender == nullptr) {
		return 0;
	}

	if (defender->isInvulnerable()) {
		return 0;
	}

	int armorPiercing = 0;

	if (weapon != nullptr) {
		armorPiercing = weapon->getArmorPiercing();
	}

	// need to check armor reduction with just defender, blast and their AR + resists
	if (defender->isVehicleObject()) {
		auto defenderVehicle = cast<VehicleObject*>(defender);

		if (defenderVehicle != nullptr) {
			int armorResist = defenderVehicle->getBlast();

			if (armorResist > 0) {
				damage *= getArmorPiercing(defenderVehicle, armorPiercing);

				damage *= (1.f - (armorResist / 100.f));
			}
		}
	} else {
		// Pick a random HAM pool
		int pool = calculatePoolsToDamage(RANDOM);

		uint8 hitLocation = 0;
		uint8 attribute = CreatureAttribute::HEALTH;

		// Determine hitLocation
		switch (pool) {
			case HEALTH: {
				static const uint8 bodyLocations[] = {HIT_BODY, HIT_BODY, HIT_LARM, HIT_RARM};
				hitLocation = bodyLocations[System::random(3)];
				break;
			}
			case ACTION: {
				static const uint8 legLocations[] = {HIT_LLEG, HIT_RLEG};
				hitLocation = legLocations[System::random(1)];
				attribute = CreatureAttribute::ACTION;
				break;
			}
			case MIND: {
				hitLocation = HIT_HEAD;
				attribute = CreatureAttribute::MIND;
				break;
			}
			default:
				break;
		}

		// Calculate agent armor reduction
		if (defender->isAiAgent()) {
			auto agent = defender->asAiAgent();

			if (agent != nullptr) {
				int armorResist = agent->getBlast();

				if (armorResist > 0) {
					damage *= getArmorPiercing(agent, armorPiercing);

					damage *= (1.f - (armorResist / 100.f));
				}
			}
		// Calculate player armor reduction
		} else {
			// PvP Damage Reduction
			if (attackerTanO->isDroidObject() && defender->isPlayerCreature()) {
				damage *= 0.25;
			}

			ArmorObject* psgArmor = getPSGArmor(defender);

			// PSG Reduction
			if (psgArmor != nullptr && !psgArmor->isVulnerable(SharedWeaponObjectTemplate::BLAST)) {
				float armorReduction = psgArmor->getBlast();

				damage *= getArmorPiercing(psgArmor, armorPiercing);

				if (armorReduction > 0) {
					damage *= (1.f - (armorReduction / 100.f));
				}

				Locker plocker(psgArmor, attackerTanO);

				psgArmor->inflictDamage(psgArmor, 0, damage * 0.2, true, true);
			}

			ManagedReference<ArmorObject*> armor = getArmorObject(defender, hitLocation);

			// Armor Reduction
			if (armor != nullptr && !armor->isVulnerable(SharedWeaponObjectTemplate::BLAST)) {
				float armorReduction = getArmorObjectReduction(armor, SharedWeaponObjectTemplate::BLAST);

				// use only the damage applied to the armor for piercing (after the PSG takes some off)
				damage *= getArmorPiercing(armor, armorPiercing);

				if (armorReduction > 0) {
					damage *= (1.f - (armorReduction / 100.f));
				}

				// inflict condition damage
				Locker alocker(armor, attackerTanO);

				armor->inflictDamage(armor, 0, damage * 0.2, true, true);
			}
		}

		// Handle spill over damage for all pools
		if (defender->isCreatureObject() && !defender->isVehicleObject()) {
			// Calculate Spill over
			int numSpillOverPools = 2;
			float spillMultPerPool = (0.0834f * numSpillOverPools);

			// Calculate spill damage
			int spilledDamage = (int)(damage * spillMultPerPool);

			// subtract spill damage from total damage
			damage -= spilledDamage;

			// Split the spill over damage between the pools damaged
			int spillDamagePerPool = (int)(spilledDamage / numSpillOverPools);
			int spillOverRemainder = (spilledDamage % numSpillOverPools) + spillDamagePerPool;
			int spillToApply = (numSpillOverPools-- > 1 ? spillDamagePerPool : spillOverRemainder);

			if ((pool ^ 0x7) & HEALTH) {
				defender->inflictDamage(attackerTanO, CreatureAttribute::HEALTH, spillToApply, true, true, false);
			}

			if ((pool ^ 0x7) & ACTION) {
				defender->inflictDamage(attackerTanO, CreatureAttribute::ACTION, spillToApply, true, true, false);
			}

			if ((pool ^ 0x7) & MIND) {
				defender->inflictDamage(attackerTanO, CreatureAttribute::MIND, spillToApply, true, true, false);
			}
		}

		// Apply the damage to the defender agent or player
		defender->inflictDamage(attackerTanO, attribute, (int)damage, true, true, false);
	}

	return damage;
}

// Calculate Weapon Speed

float CombatManager::calculateWeaponAttackSpeed(CreatureObject* attacker, WeaponObject* weapon, float skillSpeedRatio) const {
	if (weapon == nullptr) {
		return 4.0f;
	}

	int speedMod = getSpeedModifier(attacker, weapon);
	float jediSpeed = attacker->getSkillMod("combat_haste") / 100.0f;

	float attackSpeed = (1.0f - ((float)speedMod / 100.0f)) * skillSpeedRatio * weapon->getAttackSpeed();

	if (jediSpeed > 0)
		attackSpeed = attackSpeed - (attackSpeed * jediSpeed);

	return Math::max(attackSpeed, 1.0f);
}

// Fly Text - Miss, Counterattack, Block, Hit Location
void CombatManager::doMiss(TangibleObject* attacker, WeaponObject* weapon, CreatureObject* defender, const CreatureAttackData& data, int damage) const {
	if (defender == nullptr)
		return;

	defender->showFlyText("combat_effects", "miss", 0xFF, 0xFF, 0xFF);

	if (data.getCommandCRC() == STRING_HASHCODE("concealshot") && attacker != nullptr && attacker->isPlayerCreature() && defender->isAiAgent()) {
		AiAgent* agent = defender->asAiAgent();

		if (agent != nullptr) {
			VectorMap<uint64, int>* targetMissCount = agent->getTargetMissCount();
			if (targetMissCount != nullptr) {
				uint64 attackerID = attacker->getObjectID();

				Locker agentLock(agent);

				if (targetMissCount->contains(attackerID)) {
					for (int i = 0; i < targetMissCount->size(); i++){
						uint64 listTarget = targetMissCount->elementAt(i).getKey();

						if (listTarget == attackerID) {
							int missCount = targetMissCount->elementAt(i).getValue();

							agent->setTargetMissCount(attackerID, missCount + 1);
							break;
						}
					}
				} else {
					agent->addTargetMissCount(attackerID, 1);
				}
			}
		}
	}
}

void CombatManager::doCounterAttack(TangibleObject* attacker, WeaponObject* weapon, CreatureObject* defender, int damage) const {
	defender->showFlyText("combat_effects", "counterattack", 0, 0xFF, 0);
}

void CombatManager::doBlock(TangibleObject* attacker, WeaponObject* weapon, CreatureObject* defender, int damage) const {
	defender->showFlyText("combat_effects", "block", 0, 0xFF, 0);
}

void CombatManager::doDodge(TangibleObject* attacker, WeaponObject* weapon, CreatureObject* defender, int damage) const {
	defender->doCombatAnimation(STRING_HASHCODE("dodge"));
	defender->showFlyText("combat_effects", "dodge", 0, 0xFF, 0);
}

void CombatManager::showHitLocationFlyText(CreatureObject* attacker, CreatureObject* defender, uint8 location) const {
	if (defender->isVehicleObject())
		return;

	ShowFlyText* fly = nullptr;
	switch (location) {
	case HIT_HEAD:
		fly = new ShowFlyText(defender, "combat_effects", "hit_head", 0, 0, 0xFF, 1.0f);
		break;
	case HIT_BODY:
		fly = new ShowFlyText(defender, "combat_effects", "hit_body", 0xFF, 0, 0, 1.0f);
		break;
	case HIT_LARM:
		fly = new ShowFlyText(defender, "combat_effects", "hit_larm", 0xFF, 0, 0, 1.0f);
		break;
	case HIT_RARM:
		fly = new ShowFlyText(defender, "combat_effects", "hit_rarm", 0xFF, 0, 0, 1.0f);
		break;
	case HIT_LLEG:
		fly = new ShowFlyText(defender, "combat_effects", "hit_lleg", 0, 0xFF, 0, 1.0f);
		break;
	case HIT_RLEG:
		fly = new ShowFlyText(defender, "combat_effects", "hit_rleg", 0, 0xFF, 0, 1.0f);
		break;
	}

	if (fly != nullptr)
		attacker->sendMessage(fly);
}

// Special Attack Cost

bool CombatManager::applySpecialAttackCost(CreatureObject* attacker, WeaponObject* weapon, const CreatureAttackData& data) const {
	if (attacker == nullptr || attacker->isAiAgent() || data.isForceAttack() || weapon == nullptr)
		return true;

	PlayerObject* attackerGhost = attacker->getPlayerObject();
	float weaponForceCost = weapon->getForceCost();

	// Infinity:  Minimum saber costs in PVP
	if (attackerGhost != nullptr && weapon->isJediWeapon()) {
		if (weaponForceCost < 1.0)   
			weaponForceCost = 1.0f;  // Minimum saber force cost of 1 in PVE
		if (((attackerGhost->hasPvpTef() && attacker->getFactionStatus() == FactionStatus::OVERT) || attackerGhost->hasBhTef() || attackerGhost->getDuelListSize() != 0) && weaponForceCost < 5)    
			weaponForceCost = 5.0f;  // Minimum saber force cost of 5 in PVP
		if (attackerGhost->hasBhTef())   // Make sure attacker has BH Tef
			weaponForceCost += (attacker->getHuntedLevel() * 1.5); //Infinity:  1.5 additional base cost to saber per bounty hunter
	}

	// Infinity:  Min force cost
	float force = weaponForceCost * data.getForceCostMultiplier();

	if (force > 0) { // Need Force check first otherwise it can be spammed.
		ManagedReference<PlayerObject*> playerObject = attacker->getPlayerObject();
		if (playerObject != nullptr) {
			if (playerObject->getForcePower() <= force) {
				attacker->sendSystemMessage("@jedi_spam:no_force_power");
				return false;
			} else {
				playerObject->setForcePower(playerObject->getForcePower() - force);
				VisibilityManager::instance()->increaseVisibility(attacker, data.getCommand()->getVisMod()); // Give visibility
			}
		}
	}

	float health = weapon->getHealthAttackCost() * data.getHealthCostMultiplier();
	float action = weapon->getActionAttackCost() * data.getActionCostMultiplier();
	float mind = weapon->getMindAttackCost() * data.getMindCostMultiplier();

	health = attacker->calculateCostAdjustment(CreatureAttribute::STRENGTH, health);
	action = attacker->calculateCostAdjustment(CreatureAttribute::QUICKNESS, action);
	mind = attacker->calculateCostAdjustment(CreatureAttribute::FOCUS, mind);

	if (attacker->getHAM(CreatureAttribute::HEALTH) <= health)
		return false;

	if (attacker->getHAM(CreatureAttribute::ACTION) <= action)
		return false;

	if (attacker->getHAM(CreatureAttribute::MIND) <= mind)
		return false;

	if (health > 0)
		attacker->inflictDamage(attacker, CreatureAttribute::HEALTH, health, true, true, true);

	if (action > 0)
		attacker->inflictDamage(attacker, CreatureAttribute::ACTION, action, true, true, true);

	if (mind > 0)
		attacker->inflictDamage(attacker, CreatureAttribute::MIND, mind, true, true, true);

	return true;
}

// Apply States
void CombatManager::applyStates(CreatureObject* creature, CreatureObject* targetCreature, DefenderHitList* hitList, const CreatureAttackData& data) const {
	const VectorMap<uint8, StateEffect>* stateEffects = data.getStateEffects();
	int stateAccuracyBonus = data.getStateAccuracyBonus();

	if (creature == nullptr || targetCreature == nullptr) {
		return;
	}

	if (targetCreature->isInvulnerable()) {
		return;
	}

	Locker statelock(targetCreature, creature);

	int playerLevel = 0;

	if (targetCreature->isPlayerCreature()) {
		ZoneServer* server = targetCreature->getZoneServer();

		if (server != nullptr) {
			PlayerManager* pManager = server->getPlayerManager();

			if (pManager != nullptr) {
				playerLevel = pManager->calculatePlayerLevel(targetCreature) - 5;
			}
		}
	}

#ifdef DEBUG_STATES
	StringBuffer stateDebug;
	stateDebug << "---------- Starting state application Calculation --------- Player Level: " << playerLevel << "\n";
    stateDebug << "- Attacker: " << creature->getDisplayedName() << "   Defender: " << targetCreature->getDisplayedName() << "\n";
#endif

	// loop through all the states in the command
	for (int i = 0; i < stateEffects->size(); i++) {
		const StateEffect& effect = stateEffects->get(i);
		bool failed = false;
		uint8 effectType = effect.getEffectType();

#ifdef DEBUG_STATES


        String effectString = "INVALID";
        switch (effectType) {
            case CommandEffect::BLIND:
                effectString = "BLIND";
                break;
            case CommandEffect::DIZZY:
                effectString = "DIZZY";
                break;
            case CommandEffect::INTIMIDATE:
                effectString = "INTIMIDATE";
                break;
            case CommandEffect::STUN:
                effectString = "STUN";
                break;
            case CommandEffect::KNOCKDOWN:
                effectString = "KNOCKDOWN";
                break;
            case CommandEffect::POSTUREUP:
                effectString = "POSTUREUP";
                break;
            case CommandEffect::POSTUREDOWN:
                effectString = "POSTUREDOWN";
                break;
            case CommandEffect::NEXTATTACKDELAY:
                effectString = "NEXTATTACKDELAY";
                break;
            default:
                break;
        }

		stateDebug << "- Type: " << effectString << " (" << effectType << ")" << "\n";
#endif

		float accuracyMod = effect.getStateChance() + stateAccuracyBonus;
		if (data.isStateOnlyAttack()) {
			accuracyMod += creature->getSkillMod(data.getCommand()->getAccuracySkillMod());
		}

		// Infinity:  Let's get managed references we will use throughout the function
		ManagedReference<WeaponObject*> creatureWeapon = creature->getWeapon();
		ManagedReference<WeaponObject*> targetWeapon = targetCreature->getWeapon();

		//Infinity:  Reduce lightsaber attack state accuracy in PVP
		if (effectType == CommandEffect::STUN || effectType == CommandEffect::DIZZY || effectType == CommandEffect::BLIND) {
			if (creature->isPlayerCreature() && targetCreature->isPlayerCreature()) {
				if (creature->hasSkill("force_title_jedi_rank_03") || (creatureWeapon != nullptr && creatureWeapon->isJediWeapon())) {
					if (targetCreature->hasSkill("force_title_jedi_rank_03") || (targetWeapon != nullptr && targetWeapon->isJediWeapon())) {
						if (data.isForceAttack() || data.getCommandName().beginsWith("saber")) { // Infinity:  Reduce state accuracy in Jedi vs.Jedi PVP by 15 for skills that were increased
							if (accuracyMod >= 80) {
#ifdef DEBUG_STATES
								stateDebug << "Reducing state accuracy mod for Jedi PVP from " << accuracyMod << " to " << (accuracyMod-15) <<  "\n";
#endif

								accuracyMod -= 15;
							}
						}
					}
				}
			}
		}

#ifdef DEBUG_STATES
		stateDebug << "- Accuracy Mod after Jedi PVP adjustment: " << accuracyMod << "\n";
#endif
		// Infinity:  Increase state accuracy based on FRS mod
		accuracyMod *= creature->getFrsMod("manipulation",1.25);

#ifdef DEBUG_STATES
		stateDebug << "- Accuracy Mod after FRS mod: " << accuracyMod << "\n";
#endif
		// Check for state immunity.
		if (targetCreature->hasEffectImmunity(effectType)) {
			failed = true;
#ifdef DEBUG_STATES
			stateDebug << " -- FAILED DUE TO IMMUNITY -- \n";
#endif
		}

		if (!failed) {
			const Vector<String>& exclusionTimers = effect.getDefenderExclusionTimers();

			// loop through any exclusion timers
			for (int j = 0; j < exclusionTimers.size(); j++) {
				if (!targetCreature->checkCooldownRecovery(exclusionTimers.get(j))) {
					failed = true;
				}
			}
		}

		float targetDefense = 0.f;

		// if recovery timer conditions aren't satisfied, it won't matter
		if (!failed) {
			const Vector<String>& defenseMods = effect.getDefenderStateDefenseModifiers();
			// add up all defenses against the state the target has
			for (int j = 0; j < defenseMods.size(); j++) {
				targetDefense += targetCreature->getSkillMod(defenseMods.get(j));
			}

#ifdef DEBUG_STATES
			stateDebug << " - Target Defense Base = " << targetDefense << "\n";
#endif

			targetDefense /= 1.5;

#ifdef DEBUG_STATES
			stateDebug << " - Target Defense /= 1.5 = " << targetDefense << "\n";
#endif

			targetDefense += playerLevel;

#ifdef DEBUG_STATES
			stateDebug << " - Target Defense + playerLevel = " << targetDefense << "\n";
#endif

			// Infinity:  Adjust state defense based on FRS manipulation mod
			targetDefense *= targetCreature->getFrsMod("manipulation",1.25);

#ifdef DEBUG_STATES
			stateDebug << " - Target Defense after FRS mod = " << targetDefense << "\n";
#endif

			// Run roll to check against
			int roll = System::random(100);

#ifdef DEBUG_STATES
			stateDebug << " - Roll = " << roll << "\n";
#endif

			PlayerObject* targetGhost = targetCreature->getPlayerObject();
			if (targetCreature->isPlayerCreature() && creature->isPlayerCreature() && creatureWeapon != nullptr && creatureWeapon->isJediWeapon() && targetWeapon != nullptr &&
					targetWeapon->isMeleeWeapon() && targetGhost != nullptr && targetGhost->hasBhTef()) { // Infinity:  Melee mando state defense cap of 100
				bool validState = effectType == CommandEffect::BLIND || effectType == CommandEffect::DIZZY || effectType == CommandEffect::INTIMIDATE || effectType == CommandEffect::STUN;
				if (validState && targetDefense > 86.67f) {
					targetDefense = Math::min(86.67f, targetDefense);
#ifdef DEBUG_STATES
					stateDebug << "- Target Defense after melee BH cap = " << targetDefense << "\n";
#endif
				}
			}

			// Players are able to be state immune.
			int calc = (int)(accuracyMod - targetDefense);

#ifdef DEBUG_STATES
			stateDebug << " - Calc = " << calc << "\n";
#endif

			if (roll > calc) {
				failed = true;

#ifdef DEBUG_STATES
				stateDebug << " -- Roll > calc, failed = true -- \n";
#endif
			}
			else {

#ifdef DEBUG_STATES
				stateDebug << " -- Roll <= calc, failed = false -- \n";
				stateDebug << " -- Now Checking Jedi State Defenses -- \n";
#endif
			}

			// no reason to apply jedi defenses if primary defense was successful
			// and only perform extra rolls if the character is a Jedi
			// Infinity:  Doesn't work with armor equipped

			if (!failed && targetCreature->isPlayerCreature() && targetGhost != nullptr && targetGhost->isJedi() && !targetCreature->isWearingArmor()) {

				float frsBonus = targetCreature->getFrsMod("manipulation",2.083);	// Infinity:  Max bonus x 1.08

				const Vector<String>& jediMods = effect.getDefenderJediStateDefenseModifiers();
				// second chance for jedi, roll against their special defenses jedi_state_defense & resistance_states if applicable
				for (int j = 0; j < jediMods.size(); j++) {
#ifdef DEBUG_STATES
			stateDebug << " - Checking Jedi skillmod = " << jediMods.get(j) << "\n";
#endif					
					targetDefense = targetCreature->getSkillMod(jediMods.get(j));

					if (!targetCreature->hasSkill("force_title_jedi_rank_03"))	//Infinity:  90 cap for non-Jedi Knight, 95 for Jedi Knight
						targetDefense = Math::min(90.0f,targetDefense);
					else
						targetDefense = Math::min(95.0f,targetDefense);

#ifdef DEBUG_STATES
			stateDebug << " - Target Jedi State Defense after FRS mod = " << targetDefense << "\n";
#endif

					targetDefense /= 1.5;
					targetDefense += playerLevel;

					if (targetCreature->hasSkill("force_title_jedi_rank_03")) {  // Infinity:  Jedi Knight
						targetDefense *= frsBonus;
						targetDefense = Math::min(90.0f,targetDefense); 		// Infinity:  90 Hard Cap = 105 Jedi State Defense
					}

#ifdef DEBUG_STATES
			stateDebug << " - Target Defense from Jedi State Defense after FRS mod = " << targetDefense << "\n";
#endif

					calc = (int)(accuracyMod - targetDefense);

					if (roll > calc) {
						failed = true;
#ifdef DEBUG_STATES
					stateDebug << " -- Jedi Defense check:  Roll > calc, failed = true -- \n";
#endif
						break;
					}
					else {

#ifdef DEBUG_STATES
						stateDebug << " -- Jedi Defense check:  Roll <= calc, failed = false -- \n";
#endif
					}
				}
			}
		}

		// Infinity:  BH Special attacks that can apply intimidate
		String commandName = data.getCommandName();
		if (effectType == CommandEffect::INTIMIDATE && (commandName == "fastblast" || commandName == "firelightningcone2" || commandName == "firelightningsingle2")) { 
			if (!creature->isPlayerCreature()) {
				failed = true;
			}
			else {
				ManagedReference<PlayerObject*> creatureGhost = creature->getPlayerObject();
				if (creatureGhost == nullptr) { // Infinity:  Only works for players, this is null playerObject
					failed = true;
				}
				else if (!creatureGhost->hasBhTef()) { //Infinity:  fastblast/LLC only applies intimidate when the bounty hunter is actively engaged in a hunt
					failed = true;
				}
			}
		}

		if (!failed) {
			if (effectType == CommandEffect::NEXTATTACKDELAY) {
				StringIdChatParameter stringId("combat_effects", "delay_applied_other");
				stringId.setTT(targetCreature->getObjectID());
				stringId.setDI(effect.getStateLength());
				creature->sendSystemMessage(stringId);
			}

#ifdef DEBUG_STATES
			stateDebug << " -- ROLL SUCCESS -- Effect applied with a strength of: " << effect.getStateStrength() + stateAccuracyBonus << "\n";
#endif

			data.getCommand()->applyEffect(creature, targetCreature, effectType, effect.getStateStrength() + stateAccuracyBonus);

			if (data.changesDefenderPosture()) {
				targetCreature->updatePostures(true);
			}
		}
		else {

#ifdef DEBUG_STATES
			stateDebug << " -- ROLL FAILED -- Effect NOT applied" "\n";
#endif
			switch (effectType) {
				case CommandEffect::KNOCKDOWN:
					if (!targetCreature->checkKnockdownRecovery() && targetCreature->getPosture() != CreaturePosture::UPRIGHT) {
						targetCreature->setPosture(CreaturePosture::UPRIGHT);
						if (targetCreature->isPlayerCreature()) {
							String name = creature->getDisplayedName();
							name[0] = toupper(name[0]);
							targetCreature->sendSystemMessage(name + "'s knockdown attack somehow lifts you to your feet.");
						}
					}
					creature->sendSystemMessage("@cbt_spam:knockdown_fail");
					break;
				case CommandEffect::POSTUREDOWN:
					if (!targetCreature->checkPostureDownRecovery() && targetCreature->getPosture() != CreaturePosture::UPRIGHT) {
						targetCreature->setPosture(CreaturePosture::UPRIGHT);
						if (targetCreature->isPlayerCreature()) {
							String name = creature->getDisplayedName();
							name[0] = toupper(name[0]);
							targetCreature->sendSystemMessage(name + "'s posture down attack somehow lifts you to your feet.");
						}
					}
					creature->sendSystemMessage("@cbt_spam:posture_change_fail");
					break;
				case CommandEffect::POSTUREUP:
					if (!targetCreature->checkPostureUpRecovery() && targetCreature->getPosture() != CreaturePosture::UPRIGHT) {
						targetCreature->setPosture(CreaturePosture::UPRIGHT);
						if (targetCreature->isPlayerCreature()) {
							String name = creature->getDisplayedName();
							name[0] = toupper(name[0]);
							targetCreature->sendSystemMessage(name + "'s posture up attack somehow lifts you to your feet.");
						}
					}
					creature->sendSystemMessage("@cbt_spam:posture_change_fail");
					break;
				case CommandEffect::NEXTATTACKDELAY:
					if (data.getCommand()->getNameCRC() != STRING_HASHCODE("panicshot"))
						targetCreature->showFlyText("combat_effects", "warcry_miss", 0xFF, 0, 0);

					creature->sendSystemMessage("@combat_effects:combat_delay_resist");
					break;
				case CommandEffect::INTIMIDATE:
					targetCreature->showFlyText("combat_effects", "intimidated_miss", 0xFF, 0, 0);
					break;
				default:
					break;
			}
		}

		// now check combat equilibrium
		if (!failed && targetCreature->isPlayerCreature() && (effectType == CommandEffect::KNOCKDOWN || effectType == CommandEffect::POSTUREDOWN || effectType == CommandEffect::POSTUREUP)) {
			int combatEquil = targetCreature->getSkillMod("combat_equillibrium");

			if (combatEquil > 100) {
				combatEquil = 100;
			}

			int frsEquil = targetCreature->getFrsManipulation();

			if (frsEquil >combatEquil)
				combatEquil = frsEquil;

			// Infinity:  Removed intimidate check on combat equilibrium.   This compares combatEquil / 2 to random(100).
			if ((combatEquil >> 1) > (int)System::random(100) && !targetCreature->isDead() && !targetCreature->isIncapacitated()) {
				targetCreature->setPosture(CreaturePosture::UPRIGHT, true, true);

				String attackType = "";
				switch (effectType) {
					case CommandEffect::KNOCKDOWN:
						attackType = "knockdown attack.";
						break;
					case CommandEffect::POSTUREDOWN:
						attackType = "posture down attack.";
						break;
					case CommandEffect::POSTUREUP:
						attackType = "posture up attack.";
						break;
					default:
						break;
				}

				targetCreature->sendSystemMessage("Your combat equilibrium allows you to stand up immediately from the " + attackType);
			}
		}

		// Send Combat Spam for state-only attacks.
		if (data.isStateOnlyAttack()) {
			if (failed) {
				data.getCommand()->sendAttackCombatSpam(creature, targetCreature, MISS, 0, data);
			} else {
				data.getCommand()->sendAttackCombatSpam(creature, targetCreature, HIT, 0, data);
			}
		}

#ifdef DEBUG_STATES
		stateDebug << "---------- End of calc --------- \n";
		info(true) << stateDebug.toString();

        if (creature->isPlayerCreature()) {
            creature->sendSystemMessage(stateDebug.toString());
        }

        if (targetCreature->isPlayerCreature()) {
            targetCreature->sendSystemMessage(stateDebug.toString());
        }

#endif
	}
}

// Cone AoE attack angle
bool CombatManager::checkConeAngle(SceneObject* target, float angle, float creatureVectorX, float creatureVectorY, float directionVectorX, float directionVectorY) const {
	float Target1 = target->getPositionX() - creatureVectorX;
	float Target2 = target->getPositionY() - creatureVectorY;

	float resAngle = atan2(Target2, Target1) - atan2(directionVectorY, directionVectorX);
	float degrees = resAngle * 180 / M_PI;

	float coneAngle = angle / 2;

	if (degrees > coneAngle || degrees < -coneAngle) {
		return false;
	}

	return true;
}

/*

	Player Duels

*/

void CombatManager::requestDuel(CreatureObject* player, CreatureObject* targetPlayer) const {
	/* Pre: player != targetPlayer and not nullptr; player is locked
	 * Post: player requests duel to targetPlayer
	 */

	Locker clocker(targetPlayer, player);

	PlayerObject* ghost = player->getPlayerObject();
	PlayerObject* targetGhost = targetPlayer->getPlayerObject();

	if (ghost->requestedDuelTo(targetPlayer)) {
		StringIdChatParameter stringId("duel", "already_challenged");
		stringId.setTT(targetPlayer->getObjectID());
		player->sendSystemMessage(stringId);

		return;
	}

	player->debug() << "requesting duel with " << targetPlayer->getObjectID();

	ghost->addToDuelList(targetPlayer);

	if (targetGhost->requestedDuelTo(player)) {
		BaseMessage* pvpstat = new UpdatePVPStatusMessage(targetPlayer, player, targetPlayer->getPvpStatusBitmask() | ObjectFlag::ATTACKABLE | ObjectFlag::AGGRESSIVE);
		player->sendMessage(pvpstat);

		for (int i = 0; i < targetGhost->getActivePetsSize(); i++) {
			ManagedReference<AiAgent*> pet = targetGhost->getActivePet(i);

			if (pet != nullptr) {
				BaseMessage* petpvpstat = new UpdatePVPStatusMessage(pet, player, pet->getPvpStatusBitmask() | ObjectFlag::ATTACKABLE | ObjectFlag::AGGRESSIVE);
				player->sendMessage(petpvpstat);
			}
		}

		StringIdChatParameter stringId("duel", "accept_self");
		stringId.setTT(targetPlayer->getObjectID());
		player->sendSystemMessage(stringId);

		BaseMessage* pvpstat2 = new UpdatePVPStatusMessage(player, targetPlayer, player->getPvpStatusBitmask() | ObjectFlag::ATTACKABLE | ObjectFlag::AGGRESSIVE);
		targetPlayer->sendMessage(pvpstat2);

		for (int i = 0; i < ghost->getActivePetsSize(); i++) {
			ManagedReference<AiAgent*> pet = ghost->getActivePet(i);

			if (pet != nullptr) {
				BaseMessage* petpvpstat = new UpdatePVPStatusMessage(pet, targetPlayer, pet->getPvpStatusBitmask() | ObjectFlag::ATTACKABLE | ObjectFlag::AGGRESSIVE);
				targetPlayer->sendMessage(petpvpstat);
			}
		}

		StringIdChatParameter stringId2("duel", "accept_target");
		stringId2.setTT(player->getObjectID());
		targetPlayer->sendSystemMessage(stringId2);
	} else {
		StringIdChatParameter stringId3("duel", "challenge_self");
		stringId3.setTT(targetPlayer->getObjectID());
		player->sendSystemMessage(stringId3);

		StringIdChatParameter stringId4("duel", "challenge_target");
		stringId4.setTT(player->getObjectID());
		targetPlayer->sendSystemMessage(stringId4);
	}
}

void CombatManager::requestEndDuel(CreatureObject* player, CreatureObject* targetPlayer) const {
	/* Pre: player != targetPlayer and not nullptr; player is locked
	 * Post: player requested to end the duel with targetPlayer
	 */

	Locker clocker(targetPlayer, player);

	PlayerObject* ghost = player->getPlayerObject();
	PlayerObject* targetGhost = targetPlayer->getPlayerObject();

	if (!ghost->requestedDuelTo(targetPlayer)) {
		StringIdChatParameter stringId("duel", "not_dueling");
		stringId.setTT(targetPlayer->getObjectID());
		player->sendSystemMessage(stringId);

		return;
	}

	player->debug() << "ending duel with " << targetPlayer->getObjectID();

	ghost->removeFromDuelList(targetPlayer);
	player->removeDefender(targetPlayer);

	if (targetGhost->requestedDuelTo(player)) {
		targetGhost->removeFromDuelList(player);
		targetPlayer->removeDefender(player);

		player->sendPvpStatusTo(targetPlayer);

		for (int i = 0; i < ghost->getActivePetsSize(); i++) {
			ManagedReference<AiAgent*> pet = ghost->getActivePet(i);

			if (pet != nullptr) {
				targetPlayer->removeDefender(pet);
				pet->sendPvpStatusTo(targetPlayer);

				ManagedReference<CreatureObject*> target = targetPlayer;

				Core::getTaskManager()->executeTask([=]() {
					 Locker locker(pet);

					 pet->removeDefender(target);
				 }, "PetRemoveDefenderLambda");
			}
		}

		StringIdChatParameter stringId("duel", "end_self");
		stringId.setTT(targetPlayer->getObjectID());
		player->sendSystemMessage(stringId);

		targetPlayer->sendPvpStatusTo(player);

		for (int i = 0; i < targetGhost->getActivePetsSize(); i++) {
			ManagedReference<AiAgent*> pet = targetGhost->getActivePet(i);

			if (pet != nullptr) {
				player->removeDefender(pet);
				pet->sendPvpStatusTo(player);

				ManagedReference<CreatureObject*> play = player;

				Core::getTaskManager()->executeTask([=]() {
					 Locker locker(pet);

					 pet->removeDefender(play);
				 }, "PetRemoveDefenderLambda2");
			}
		}

		StringIdChatParameter stringId2("duel", "end_target");
		stringId2.setTT(player->getObjectID());
		targetPlayer->sendSystemMessage(stringId2);
	}
}

void CombatManager::freeDuelList(CreatureObject* player, bool spam) const {
	/* Pre: player not nullptr and is locked
	 * Post: player removed and warned all of the objects from its duel list
	 */
	PlayerObject* ghost = player->getPlayerObject();

	if (ghost == nullptr || ghost->isDuelListEmpty())
		return;

	player->debug("freeing duel list");

	while (ghost->getDuelListSize() != 0) {
		ManagedReference<CreatureObject*> targetPlayer = ghost->getDuelListObject(0);
		PlayerObject* targetGhost = targetPlayer->getPlayerObject();

		if (targetPlayer != nullptr && targetGhost != nullptr && targetPlayer.get() != player) {
			try {
				Locker clocker(targetPlayer, player);

				ghost->removeFromDuelList(targetPlayer);
				player->removeDefender(targetPlayer);

				if (targetGhost->requestedDuelTo(player)) {
					targetGhost->removeFromDuelList(player);
					targetPlayer->removeDefender(player);

					player->sendPvpStatusTo(targetPlayer);

					for (int i = 0; i < ghost->getActivePetsSize(); i++) {
						ManagedReference<AiAgent*> pet = ghost->getActivePet(i);

						if (pet != nullptr) {
							targetPlayer->removeDefender(pet);
							pet->sendPvpStatusTo(targetPlayer);

							Core::getTaskManager()->executeTask([=]() {
								 Locker locker(pet);

								 pet->removeDefender(targetPlayer);
							 }, "PetRemoveDefenderLambda3");
						}
					}

					if (spam) {
						StringIdChatParameter stringId("duel", "end_self");
						stringId.setTT(targetPlayer->getObjectID());
						player->sendSystemMessage(stringId);
					}

					targetPlayer->sendPvpStatusTo(player);

					for (int i = 0; i < targetGhost->getActivePetsSize(); i++) {
						ManagedReference<AiAgent*> pet = targetGhost->getActivePet(i);

						if (pet != nullptr) {
							player->removeDefender(pet);
							pet->sendPvpStatusTo(player);

							ManagedReference<CreatureObject*> play = player;

							Core::getTaskManager()->executeTask([=]() {
								 Locker locker(pet);

								 pet->removeDefender(play);
							 }, "PetRemoveDefenderLambda4");
						}
					}

					if (spam) {
						StringIdChatParameter stringId2("duel", "end_target");
						stringId2.setTT(player->getObjectID());
						targetPlayer->sendSystemMessage(stringId2);
					}
				}

			} catch (ObjectNotDeployedException& e) {
				ghost->removeFromDuelList(targetPlayer);

				System::out << "Exception on CombatManager::freeDuelList()\n" << e.getMessage() << "\n";
			}
		}
	}
}

void CombatManager::declineDuel(CreatureObject* player, CreatureObject* targetPlayer) const {
	/* Pre: player != targetPlayer and not nullptr; player is locked
	 * Post: player declined Duel to targetPlayer
	 */

	Locker clocker(targetPlayer, player);

	PlayerObject* ghost = player->getPlayerObject();
	PlayerObject* targetGhost = targetPlayer->getPlayerObject();

	if (targetGhost->requestedDuelTo(player)) {
		targetGhost->removeFromDuelList(player);

		StringIdChatParameter stringId("duel", "cancel_self");
		stringId.setTT(targetPlayer->getObjectID());
		player->sendSystemMessage(stringId);

		StringIdChatParameter stringId2("duel", "cancel_target");
		stringId2.setTT(player->getObjectID());
		targetPlayer->sendSystemMessage(stringId2);

		player->debug() << "declined duel with " << targetPlayer->getObjectID();
	}
}

bool CombatManager::areInDuel(CreatureObject* player1, CreatureObject* player2) const {
	PlayerObject* ghost1 = player1->getPlayerObject().get();
	PlayerObject* ghost2 = player2->getPlayerObject().get();

	if (ghost1 != nullptr && ghost2 != nullptr) {
		if (ghost1->requestedDuelTo(player2) && ghost2->requestedDuelTo(player1))
			return true;
	}

	return false;
}

bool CombatManager::hasActiveDuelChallenge(CreatureObject* challenger, CreatureObject* targetPlayer) const {
	auto challengerGhost = challenger->getPlayerObject().get();

	if (challengerGhost != nullptr && challengerGhost->requestedDuelTo(targetPlayer))
		return true;

	return false;
}

// Check for Temporary Enemy Flags

void CombatManager::checkForTefs(CreatureObject* attacker, CreatureObject* defender, bool* shouldGcwCrackdownTef, bool* shouldGcwTef, bool* shouldBhTef) const {
	if (*shouldGcwCrackdownTef && *shouldGcwTef && *shouldBhTef) {
		return;
	}

	ManagedReference<CreatureObject*> attackingCreature = nullptr;
	ManagedReference<CreatureObject*> targetCreature = defender->isPet() || defender->isVehicleObject() ? defender->getLinkedCreature() : defender;

	if (attacker->isPet()) {
		ManagedReference<PetControlDevice*> controlDevice = attacker->getControlDevice().get().castTo<PetControlDevice*>();

		if (controlDevice != nullptr) {
			ManagedReference<SceneObject*> lastCommander = controlDevice->getLastCommander().get();

			if (lastCommander != nullptr && lastCommander->isCreatureObject()) {
				attackingCreature = lastCommander->asCreatureObject();
			} else {
				attackingCreature = attacker->getLinkedCreature();
			}
		}
	} else {
		attackingCreature = attacker;
	}

	if (attackingCreature != nullptr && targetCreature != nullptr) {
		bool covertOvert = ConfigManager::instance()->useCovertOvertSystem();
		uint32 targetFaction = targetCreature->getFaction();

		if (covertOvert && !areInDuel(attackingCreature, targetCreature) && targetFaction > 0 && attackingCreature->getFaction() != targetFaction && attackingCreature->getFactionStatus() >= FactionStatus::COVERT) {
			*shouldGcwTef = true;
		}

		if (attackingCreature->isPlayerCreature() && targetCreature->isPlayerCreature() && !areInDuel(attackingCreature, targetCreature)) {
			if (!(*shouldGcwTef) && !covertOvert) {
				if (attackingCreature->getFaction() != targetCreature->getFaction() && attackingCreature->getFactionStatus() == FactionStatus::OVERT && targetCreature->getFactionStatus() == FactionStatus::OVERT) {
					*shouldGcwTef = true;
				}
			}

			if (!(*shouldBhTef)) {
				// Infinity: Faction pets should never trigger BH TEF — they can only attack
				// declared overt enemies, which is factional warfare, not bounty hunting
				bool isFactionPetAttack = false;

				if (attacker->isPet()) {
					ManagedReference<PetControlDevice*> pcd = attacker->getControlDevice().get().castTo<PetControlDevice*>();
					if (pcd != nullptr && pcd->getPetType() == PetManager::FACTIONPET)
						isFactionPetAttack = true;
				}

				if (!isFactionPetAttack && (attackingCreature->hasBountyMissionFor(targetCreature) || targetCreature->hasBountyMissionFor(attackingCreature))) {
					*shouldBhTef = true;
				}
			}
		}

		if (!(*shouldGcwCrackdownTef)) {
			if (attackingCreature->isPlayerObject() && targetCreature->isAiAgent()) {
				Reference<PlayerObject*> ghost = attackingCreature->getPlayerObject();

				if (ghost->hasCrackdownTefTowards(targetCreature->getFaction())) {
					*shouldGcwCrackdownTef = true;
				}
			}
			if (targetCreature->isPlayerObject() && attackingCreature->isAiAgent()) {
				Reference<PlayerObject*> ghost = targetCreature->getPlayerObject();

				if (ghost->hasCrackdownTefTowards(attackingCreature->getFaction())) {
					*shouldGcwCrackdownTef = true;
				}
			}
		}
	}
}

void CombatManager::initializeDefaultAttacks() {
	defaultRangedAttacks.add(STRING_HASHCODE("fire_1_single_light"));
	defaultRangedAttacks.add(STRING_HASHCODE("fire_1_single_medium"));
	defaultRangedAttacks.add(STRING_HASHCODE("fire_1_single_light_face"));
	defaultRangedAttacks.add(STRING_HASHCODE("fire_1_single_medium_face"));

	defaultRangedAttacks.add(STRING_HASHCODE("fire_3_single_light"));
	defaultRangedAttacks.add(STRING_HASHCODE("fire_3_single_medium"));
	defaultRangedAttacks.add(STRING_HASHCODE("fire_3_single_light_face"));
	defaultRangedAttacks.add(STRING_HASHCODE("fire_3_single_medium_face"));

	defaultMeleeAttacks.add(STRING_HASHCODE("attack_high_left_light_0"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_high_center_light_0"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_high_right_light_0"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_mid_left_light_0"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_mid_center_light_0"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_mid_right_light_0"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_low_left_light_0"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_low_right_light_0"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_low_center_light_0"));

	defaultMeleeAttacks.add(STRING_HASHCODE("attack_high_left_medium_0"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_high_center_medium_0"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_high_right_medium_0"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_mid_left_medium_0"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_mid_center_medium_0"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_mid_right_medium_0"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_low_left_medium_0"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_low_right_medium_0"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_low_center_medium_0"));

	defaultMeleeAttacks.add(STRING_HASHCODE("attack_high_left_light_1"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_high_center_light_1"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_high_right_light_1"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_mid_left_light_1"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_mid_center_light_1"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_mid_right_light_1"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_low_left_light_1"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_low_right_light_1"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_low_center_light_1"));

	defaultMeleeAttacks.add(STRING_HASHCODE("attack_high_left_medium_1"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_high_center_medium_1"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_high_right_medium_1"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_mid_left_medium_1"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_mid_center_medium_1"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_mid_right_medium_1"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_low_left_medium_1"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_low_right_medium_1"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_low_center_medium_1"));

	defaultMeleeAttacks.add(STRING_HASHCODE("attack_high_left_light_2"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_high_center_light_2"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_high_right_light_2"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_mid_left_light_2"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_mid_center_light_2"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_mid_right_light_2"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_low_left_light_2"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_low_right_light_2"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_low_center_light_2"));

	defaultMeleeAttacks.add(STRING_HASHCODE("attack_high_left_medium_2"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_high_center_medium_2"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_high_right_medium_2"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_mid_left_medium_2"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_mid_center_medium_2"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_mid_right_medium_2"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_low_left_medium_2"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_low_right_medium_2"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_low_center_medium_2"));

	defaultMeleeAttacks.add(STRING_HASHCODE("attack_high_left_light_3"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_high_center_light_3"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_high_right_light_3"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_mid_left_light_3"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_mid_center_light_3"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_mid_right_light_3"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_low_left_light_3"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_low_right_light_3"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_low_center_light_3"));

	defaultMeleeAttacks.add(STRING_HASHCODE("attack_high_left_medium_3"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_high_center_medium_3"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_high_right_medium_3"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_mid_left_medium_3"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_mid_center_medium_3"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_mid_right_medium_3"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_low_left_medium_3"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_low_right_medium_3"));
	defaultMeleeAttacks.add(STRING_HASHCODE("attack_low_center_medium_3"));
}
