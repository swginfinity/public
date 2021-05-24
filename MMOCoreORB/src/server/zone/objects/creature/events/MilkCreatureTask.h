
#ifndef MILKCREATURETASK_H_
#define MILKCREATURETASK_H_

#include "server/zone/managers/resource/ResourceManager.h"
#include "server/zone/managers/combat/CombatManager.h"
#include "server/zone/managers/creature/CreatureManager.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "engine/engine.h"

class MilkCreatureTask : public Task {

private:
	enum Phase { INITIAL, ONESUCCESS, ONEFAILURE, FINAL} currentPhase;
	ManagedReference<Creature*> creature;
	ManagedReference<CreatureObject*> player;

public:
	MilkCreatureTask(Creature* cre, CreatureObject* playo) : Task() {
		currentPhase = INITIAL;
		creature = cre;
		player = playo;
	}

	void run() {
		Locker locker(creature);

		Locker _clocker(player, creature);

		if (!creature->isInRange(player, 10.f) || creature->isDead()) {
			updateMilkState(CreatureManager::NOTMILKED);
			player->sendSystemMessage("@skl_use:milk_too_far"); // The creature has moved too far away to continue milking it.
			return;
		}

		if (!player->hasState(CreatureState::MASKSCENT) || player->isInCombat() || creature->isInCombat()) {
			updateMilkState(CreatureManager::NOTMILKED);
			player->sendSystemMessage("@skl_use:milk_not_hidden"); // The creature is spooked and won't let you milk it.
			return;
		}

		float failureChance = 5 + (5 * creature->getFerocity());
		float skill = 100;
		bool success = true;

		skill += player->getSkillMod("creature_harvesting");

		failureChance /= (skill / 100);

		if (System::random(100) < failureChance)
			success = false;

		switch (currentPhase) {
		case INITIAL:
			player->sendSystemMessage("@skl_use:milk_continue"); // You continue to milk the creature.
			if (success) {
				currentPhase = ONESUCCESS;
			} else {
				currentPhase = ONEFAILURE;
			}
			this->reschedule(5000);
			break;
		case ONESUCCESS:
			if (success) {
					player->sendSystemMessage("@skl_use:milk_success"); // You have successfully gathered milk from the creature!
					giveMilkToPlayer();
			} else {
					player->sendSystemMessage("@skl_use:milk_continue"); // You continue to milk the creature.
					currentPhase = FINAL;
					this->reschedule(5000);
			}
			break;
		case ONEFAILURE:
			if (success) {
				player->sendSystemMessage("@skl_use:milk_continue"); // You continue to milk the creature.
				currentPhase = FINAL;
				this->reschedule(5000);
			} else {
				updateMilkState(CreatureManager::NOTMILKED);
				_clocker.release();
				CombatManager::instance()->startCombat(creature, player, true);
			}
			break;
		case FINAL:
			if (success) {
				player->sendSystemMessage("@skl_use:milk_success"); // You have successfully gathered milk from the creature!
				giveMilkToPlayer();
			} else {
				updateMilkState(CreatureManager::NOTMILKED);
				_clocker.release();
				CombatManager::instance()->startCombat(creature, player, true);
			}
			break;
		}

		return;
	}

	void giveMilkToPlayer() {
		ManagedReference<ResourceManager*> resourceManager = player->getZoneServer()->getResourceManager();

		String restype = creature->getMilkType();
		int playerSkill = player->getSkillMod("creature_harvesting");
		int quantity = int(creature->getMilk() * float(playerSkill / 100.0f));

		int quantityExtracted = Math::max(quantity, 3);

		ManagedReference<ResourceSpawn*> resourceSpawn = resourceManager->getCurrentSpawn(restype, player->getZone()->getZoneName());

		if (resourceSpawn == NULL) {
			player->sendSystemMessage("Error: Server cannot locate a current spawn of " + restype);
			return;
		}

		float density = resourceSpawn->getDensityAt(player->getZone()->getZoneName(), player->getPositionX(), player->getPositionY());

		String creatureHealth = "";

		if (density > 0.75f) {
			quantityExtracted = int(quantityExtracted * 1.25f);
			creatureHealth = "creature_quality_fat";
		} else if (density > 0.50f) {
			quantityExtracted = int(quantityExtracted * 1.00f);
			creatureHealth = "creature_quality_medium";
		} else if (density > 0.25f) {
			quantityExtracted = int(quantityExtracted * 0.75f);
			creatureHealth = "creature_quality_skinny";
		} else {
			quantityExtracted = int(quantityExtracted * 0.50f);
			creatureHealth = "creature_quality_scrawny";
		}


		resourceManager->harvestResourceToPlayer(player, resourceSpawn, quantityExtracted);

		/// Send System Messages
		StringIdChatParameter harvestMessage("skl_use", creatureHealth);

		harvestMessage.setDI(quantityExtracted);
		harvestMessage.setTU(resourceSpawn->getFinalClass());

		player->sendSystemMessage(harvestMessage);

		updateMilkState(CreatureManager::ALREADYMILKED);
	}

	void updateMilkState(const short milkState) {
		Locker clocker(creature);
		creature->setMilkState(milkState);
	}
};

#endif /* MILKCREATURETASK_H_ */
