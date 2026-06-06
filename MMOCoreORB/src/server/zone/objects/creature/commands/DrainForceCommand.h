/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef DRAINFORCECOMMAND_H_
#define DRAINFORCECOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "CombatQueueCommand.h"

class DrainForceCommand : public CombatQueueCommand {
public:

	DrainForceCommand(const String& name, ZoneProcessServer* server) : CombatQueueCommand(name, server) {
	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {
		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		if (isWearingArmor(creature)) {
			return NOJEDIARMOR;
		}

		if (!creature->checkCooldownRecovery("drainforce")) {
			creature->sendSystemMessage("You cannot drain force again yet.");
			return GENERALERROR;
		}

		ManagedReference<SceneObject*> object = server->getZoneServer()->getObject(target);
		// Fail if target is not a player...
		if (object == nullptr || !object->isPlayerCreature())
			return INVALIDTARGET;

		CreatureObject* targetCreature = cast<CreatureObject*>( object.get());

		if (targetCreature == nullptr || targetCreature->isDead() || (targetCreature->isIncapacitated() && !targetCreature->isFeigningDeath()) || !targetCreature->isAttackableBy(creature))
			return INVALIDTARGET;

		if (!checkDistance(creature, targetCreature, range))
			return TOOFAR;

		if (!CollisionManager::checkLineOfSight(creature, targetCreature)) {
			creature->sendSystemMessage("@combat_effects:cansee_fail");		//You cannot see your target.
			return GENERALERROR;
		}

		if (!playerEntryCheck(creature, targetCreature)) {
			return GENERALERROR;
		}

		Locker clocker(targetCreature, creature);

		ManagedReference<PlayerObject*> targetGhost = targetCreature->getPlayerObject();
		ManagedReference<PlayerObject*> playerGhost = creature->getPlayerObject();

		if (targetGhost == nullptr || playerGhost == nullptr)
			return GENERALERROR;

		CombatManager* combatManager = CombatManager::instance();
		if (combatManager == nullptr)
			return GENERALERROR;

		if (combatManager->startCombat(creature, targetCreature, false)) { 	//lockDefender = false because already locked above.

			int forceSpace = playerGhost->getForcePowerMax() - playerGhost->getForcePower();
			if (forceSpace <= 0) { 											//Cannot Force Drain if attacker can't hold any more Force.
				return GENERALERROR;
			}

			if (playerGhost->getForcePower() < forceCost) {
				creature->sendSystemMessage("@jedi_spam:no_force_power"); 	//You do not have sufficient Force power to perform that action.
				return GENERALERROR;
			}

			int targetForce = targetGhost->getForcePower();
			if (targetForce <= 0) {
				creature->sendSystemMessage("@jedi_spam:target_no_force"); 	//That target does not have any Force Power.
				return GENERALERROR;
			}


			int maxDrain = maxDamage;

			maxDrain *= creature->getFrsMod("power"); 						//FRS drains more
			maxDrain /= targetCreature->getFrsMod("control"); 				//FRS gets drained less

			int drain = maxDrain *.75 + System::random(maxDrain * 0.5f);	//Randomized the amount a bit

			int forceDrain = targetForce >= drain ? drain : targetForce; 	//Drain whatever Force the target has, up to max.

			if (forceDrain > forceSpace) {									//Drain only what attacker can hold in their own Force pool.
				forceDrain = forceSpace;		
			}

			// Force Defense.  No spam or observer notification since force defense is handled like toughness
			int forceDefense = Math::min(targetCreature->getSkillMod("force_defense"),125);						//Infinity: 125 Cap
			if (forceDefense > 0) {
				int forceDefenseMitigation = forceDrain * 0.45f * (forceDefense / 125.0f) * targetCreature->getFrsMod("manipulation");
				forceDrain -= forceDefenseMitigation;
			}

			// Force Shield
			int forceShield = targetCreature->getSkillMod("force_shield");
			if (forceShield > 0) {
				int shieldMitigation = forceDrain * (forceShield / 100.f) * targetCreature->getFrsMod("control");
				forceDrain -= shieldMitigation;
				targetCreature->notifyObservers(ObserverEventType::FORCESHIELD, targetCreature, shieldMitigation);
				combatManager->sendMitigationCombatSpam(targetCreature, nullptr, shieldMitigation, CombatManager::FORCESHIELD);
			}

			playerGhost->setForcePower(playerGhost->getForcePower() + (forceDrain - forceCost));
			targetGhost->setForcePower(targetGhost->getForcePower() - forceDrain);

			uint32 animCRC = getAnimationString().hashCode();
			creature->doCombatAnimation(targetCreature, animCRC, 0x1, 0xFF);
			combatManager->broadcastCombatSpam(creature, targetCreature, nullptr, forceDrain, "cbt_spam", combatSpam, 1);

			VisibilityManager* visibilityManager = VisibilityManager::instance();
			if (visibilityManager != nullptr) {
				visibilityManager->increaseVisibility(creature, visMod);
			}

			bool shouldGcwCrackdownTef = false, shouldGcwTef = false, shouldBhTef = false;
			combatManager->checkForTefs(creature, targetCreature, &shouldGcwCrackdownTef, &shouldGcwTef, &shouldBhTef);
			if (shouldGcwCrackdownTef || shouldGcwTef || shouldBhTef) {
				playerGhost->updateLastCombatActionTimestamp(shouldGcwCrackdownTef, shouldGcwTef, shouldBhTef);
			}
			
			creature->updateCooldownTimer("drainforce", (7000/creature->getFrsMod("manipulation"))); //Infinity:  Cooldown on drain force

			return SUCCESS;
		}

		return GENERALERROR;

	}

	float getCommandDuration(CreatureObject* object, const UnicodeString& arguments) const {
		float combatHaste = object->getSkillMod("combat_haste");

		if (combatHaste > 0) {
			return defaultTime * (1.f - (combatHaste / 100.f));
		} else {
			return defaultTime;
		}
	}

};

#endif //DRAINFORCECOMMAND_H_
