/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef FORCEWEAKEN2COMMAND_H_
#define FORCEWEAKEN2COMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "ForcePowersQueueCommand.h"

class ForceWeaken2Command : public ForcePowersQueueCommand {
public:

	ForceWeaken2Command(const String& name, ZoneProcessServer* server)
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

		if (targetObject == nullptr || !targetObject->isCreatureObject()) {
			return INVALIDTARGET;
		}

		ManagedReference<CreatureObject*> creatureTarget = targetObject.castTo<CreatureObject*>();
		if (creatureTarget == nullptr){
			return INVALIDTARGET;
		}

		CombatManager* combatManager = CombatManager::instance();
		if (combatManager == nullptr) {
			return GENERALERROR;
		}

		Locker clocker(creatureTarget, creature);

		if (creatureTarget->hasBuff(BuffCRC::JEDI_FORCEWEAKEN2)) {
			creature->sendSystemMessage("Target has already been weakened.");
			return INVALIDTARGET;
		}

		if (creatureTarget->hasBuff(BuffCRC::JEDI_FORCEWEAKEN3)) {
			creature->sendSystemMessage("Target has already been weakened by a stronger force.");
			return INVALIDTARGET;
		}

		int res = doCombatAction(creature, target);

		if (res == SUCCESS) {

			// Setup debuff.
			float frsBonus = creature->getFrsMod("power");
			ManagedReference<Buff*> buff = nullptr;

			uint32 duration = 60;  //Infinity:  Player default duration
			if (!creature->isPlayerCreature())   //Infinity:  NPC Duration
				duration = 20;

			buff = new Buff(creatureTarget, BuffCRC::JEDI_FORCEWEAKEN2, duration, BuffType::JEDI);

			if (buff == nullptr)
				return GENERALERROR;

			Locker locker(buff);
			int hamStrength =  -1 * creatureTarget->getMaxHAM(CreatureAttribute::HEALTH) * .15 * frsBonus;
			buff->setAttributeModifier(CreatureAttribute::HEALTH, hamStrength);
			hamStrength =  -1 * creatureTarget->getMaxHAM(CreatureAttribute::ACTION) * .15 * frsBonus;
			buff->setAttributeModifier(CreatureAttribute::ACTION, hamStrength);
			hamStrength =  -1 * creatureTarget->getMaxHAM(CreatureAttribute::MIND) * .15 * frsBonus;
			buff->setAttributeModifier(CreatureAttribute::MIND, hamStrength);

			buff->setSkillModifier("weaken_delay", 10);

			creatureTarget->addBuff(buff);

			combatManager->broadcastCombatSpam(creature, creatureTarget, nullptr, 0, "cbt_spam", combatSpam + "_hit", 1);

		}

		return res;
	}

};

#endif //FORCEWEAKEN2COMMAND_H_
