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

		if (targetObject == NULL || !targetObject->isCreatureObject()) {
			return INVALIDTARGET;
		}

		ManagedReference<CreatureObject*> creatureTarget = targetObject.castTo<CreatureObject*>();
		if (creatureTarget == NULL){
			return INVALIDTARGET;
		}

		String name = "forceweaken2";
		uint32 crc = name.hashCode();

		if (creatureTarget->hasBuff(crc)){
			creature->sendSystemMessage("They have already been weakened");
			return INVALIDTARGET;
		}

		String name3 = "forceweaken3";
		uint32 crc3 = name3.hashCode();

		if (creatureTarget->hasBuff(crc3)){
			creature->sendSystemMessage("They have already been weakened by a stronger force");
			return INVALIDTARGET;
		}

		int res = doCombatAction(creature, target);

		if (res == SUCCESS) {

			// Setup debuff.

			if (creatureTarget != NULL) {
				Locker clocker(creatureTarget, creature);

				float frsBonus = creature->getFrsMod("power");
				ManagedReference<Buff*> buff = NULL;

				if (creature->isPlayerCreature())
					buff = new Buff(creatureTarget, getNameCRC(), 30, BuffType::JEDI);
				else
					buff = new Buff(creatureTarget, getNameCRC(), 15, BuffType::JEDI);


				if (buff == NULL)
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

				CombatManager::instance()->broadcastCombatSpam(creature, creatureTarget, NULL, 0, "cbt_spam", combatSpam + "_hit", 1);
			}

		}

		return res;
	}

};

#endif //FORCEWEAKEN2COMMAND_H_
