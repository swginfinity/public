/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef VENOMARTCOMMAND_H_
#define VENOMARTCOMMAND_H_

#include "DotPackCommand.h"

class VenomDartCommand : public DotPackCommand {
public:

	VenomDartCommand(const String& name, ZoneProcessServer* server)
		: DotPackCommand(name, server) {
		skillName = "venomdart";
		effectName = "clienteffect/throw_trap_drowsy_dart.cef";  //find a better effect
	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!creature->hasSkill("combat_bountyhunter_novice")) {
			creature->sendSystemMessage("You are not trained in the application of the Hssiss Venom.");
			return GENERALERROR;
		}

		int result = doCommonMedicalCommandChecks(creature);
		if (result != SUCCESS) {
			return result;
		}

		int cost = hasCost(creature);
		if (cost < 0) {
			return INSUFFICIENTHAM;
		}

		auto zoneServer = server->getZoneServer();
		if (zoneServer == nullptr) {
			return GENERALERROR;
		}

		ManagedReference<SceneObject*> object = zoneServer->getObject(target);
		if (object == nullptr || !object->isCreatureObject() || creature == object){
			return INVALIDTARGET;
		}

		ManagedReference<CreatureObject*> targetCreature = cast<CreatureObject*>(object.get());
		if (targetCreature == nullptr) {
			return GENERALERROR;
		}

		if (!targetCreature->isPlayerCreature()) {
			creature->sendSystemMessage("Your dart would be ineffective against that non-player target.");
			return INVALIDTARGET;
		}

		if (!targetCreature->isAttackableBy(creature)) {
			return INVALIDTARGET;
		}

		ManagedReference<PlayerObject*> ghost = creature->getPlayerObject();
		ManagedReference<PlayerObject*> targetGhost = targetCreature->getPlayerObject();
		if (ghost == nullptr || targetGhost == nullptr)
			return GENERALERROR;

		if (!targetCreature->checkCooldownRecovery("venomdart")) {
			creature->sendSystemMessage("You cannot use this ability again on that target so soon.");
			return GENERALERROR;
        }

		CombatManager* combatManager = CombatManager::instance();
		if (combatManager == nullptr) {
			return GENERALERROR;
		}

		if (!CollisionManager::checkLineOfSight(creature, targetCreature)) {
			creature->sendSystemMessage("@healing:no_line_of_sight"); // You cannot see your target.
			return GENERALERROR;
		}
		
		uint8 pool = BuffAttribute::UNKNOWN;
		bool poolGiven = false;
		uint64 objectId = 0;
		parseModifier(arguments.toString(), pool, objectId);
		ManagedReference<DotPack*> dotPack = nullptr;

		SceneObject* inventory = creature->getSlottedObject("inventory");
		if (inventory != nullptr) {
			for (int i = 0; i < inventory->getContainerObjectsSize(); ++i) {
				ManagedReference<SceneObject*> sceno = inventory->getContainerObject(i);
				if (sceno != nullptr && sceno->getObjectTemplate()->getTemplateFileName() == "medpack_poison_hssiss") {
					dotPack = sceno.castTo<DotPack*>();
					break;
				}
			}
		}

		if (dotPack == nullptr) {
			return GENERALERROR;
		}

		int	range = int(dotPack->getRange() + creature->getSkillMod("healing_range") / 100 * 14);
		if (!checkDistance(creature, targetCreature, range)) {
			return TOOFAR;
		}

		// Checks Successful
		Locker clocker(targetCreature, creature);

		if (!creature->hasBountyMissionFor(targetCreature)) {
			creature->sendSystemMessage("You must have a bounty mission for a Jedi target to use an Hssiss venom dart.");
			return GENERALERROR;
		}

		ManagedReference<WeaponObject*> defenderWeapon = targetCreature->getWeapon();
		if (defenderWeapon == nullptr) {
			creature->sendSystemMessage("Your dart would be ineffective against that target.");
			return GENERALERROR;				
		}
		else if (!defenderWeapon->isJediWeapon() && !targetCreature->hasSkill("force_title_jedi_rank_03")) {   // Can Dart them if weapon is lightsaber or they are Jedi Knight
			creature->sendSystemMessage("Your dart would be ineffective against that target.");
			return GENERALERROR;				
		}
	
		applyCost(creature, cost);

		int dotPower = dotPack->calculatePower(creature);
		int dotDMG = 0;

		if (dotPack->isPoisonDeliveryUnit()) {
			if (!targetCreature->hasDotImmunity(dotPack->getDotType())) {
				StringIdChatParameter stringId("healing", "apply_poison_self");
				stringId.setTT(targetCreature->getObjectID());

				creature->sendSystemMessage(stringId);

				StringIdChatParameter stringId2("healing", "apply_poison_other");
				stringId2.setTU(creature->getObjectID());

				targetCreature->sendSystemMessage(stringId2);
				//put check in here so only hssiss venom works (crc lookup)
				dotDMG = targetCreature->addDotState(creature, CreatureState::POISONED, dotPack->getServerObjectCRC(), dotPower, dotPack->getPool(), dotPack->getDuration(), dotPack->getPotency(), targetCreature->getSkillMod("resistance_poison") + targetCreature->getSkillMod("poison_disease_resist"));
			}

		}

		if (dotDMG) {// Things to do only if poison lands
			awardXp(creature, "medical", dotDMG); //No experience for healing yourself.
			targetCreature->getThreatMap()->addDamage(creature, dotDMG, "");

			bool removed = false;
			if (targetCreature->hasBuff(BuffCRC::JEDI_FORCE_RUN_3)) {
				targetCreature->removeBuff(BuffCRC::JEDI_FORCE_RUN_3);
				removed = true;
			}
			if (targetCreature->hasBuff(BuffCRC::JEDI_FORCE_RUN_2)) {
				targetCreature->removeBuff(BuffCRC::JEDI_FORCE_RUN_2);
				removed = true;
			}
			if (targetCreature->hasBuff(BuffCRC::JEDI_FORCE_RUN_1)) {
				targetCreature->removeBuff(BuffCRC::JEDI_FORCE_RUN_1);
				removed = true;
			}
			if (removed) {
				targetCreature->updateCooldownTimer("venomdart", 15000);
				creature->sendSystemMessage("Your venom dart causes the target to lose focus on their speed.");
				targetCreature->sendSystemMessage("A strange venom spreads through you like fire, momentarily interrupting your connection with the force.");
			}	
		}
		else {
			StringIdChatParameter stringId("dot_message", "dot_resisted");
			stringId.setTT(targetCreature->getObjectID());
			creature->sendSystemMessage(stringId);
			StringIdChatParameter stringId2("healing", "dot_resist_other");
			targetCreature->sendSystemMessage(stringId2);
		}

		// Check for TEFs
		bool shouldGcwCrackdownTef = false, shouldGcwTef = false, shouldBhTef = false;
		combatManager->checkForTefs(creature, targetCreature, &shouldGcwCrackdownTef, &shouldGcwTef, &shouldBhTef);
        if (shouldGcwCrackdownTef || shouldGcwTef || shouldBhTef)
		    ghost->updateLastCombatActionTimestamp(shouldGcwCrackdownTef, shouldGcwTef, shouldBhTef);

		String crc = "fire_acrobatic";
		CombatAction* action = new CombatAction(creature, targetCreature, crc.hashCode(), 1, 0L);
		creature->broadcastMessage(action, true);

		clocker.release();

		Locker dlocker(dotPack, creature);
		dotPack->decreaseUseCount();
		dlocker.release();

		creature->notifyObservers(ObserverEventType::MEDPACKUSED);

		return SUCCESS;
	}

};

#endif //VENOMARTCOMMAND_H_