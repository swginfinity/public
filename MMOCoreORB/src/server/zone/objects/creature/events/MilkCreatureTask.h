
#ifndef MILKCREATURETASK_H_
#define MILKCREATURETASK_H_

#include "server/zone/managers/resource/ResourceManager.h"
#include "server/zone/managers/combat/CombatManager.h"
#include "server/zone/managers/creature/CreatureManager.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/objects/transaction/TransactionLog.h"
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

		if (!creature->isInRange(player, 10.f) || creature->isDead()) {//Infinity:  Up to 10 meters
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

		skill += player->getSkillMod("creature_harvesting");  //Infinity:  use creature harvesting skill

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
				clearStationary();
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
				clearStationary();
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
		float quantity = creature->getMilk();

		quantity = Math::max((quantity * 1.5f), 100.0f); //Infinity:  Minimum 100 units of milk, with 50% bonus

		int quantityExtracted = int(quantity * float(player->getSkillMod("creature_harvesting") / 100.0f));

		quantityExtracted = Math::max(quantityExtracted, 3);

		auto zone = player->getZone();

		if (zone == nullptr) {
			player->sendSystemMessage("Error: Server cannot locate player's zone");
			return;
		}

		String zoneName = zone->getZoneName();

		ManagedReference<ResourceSpawn*> resourceSpawn = resourceManager->getCurrentSpawn(restype, zoneName);

		if (resourceSpawn == nullptr) {
			player->sendSystemMessage("Error: Server cannot locate a current spawn of " + restype);
			return;
		}

		String creatureHealth = "";
		float density = resourceSpawn->getDensityAt(zoneName, player->getPositionX(), player->getPositionY());

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

		TransactionLog trx(TrxCode::HARVESTED, player, resourceSpawn);
		resourceManager->harvestResourceToPlayer(trx, player, resourceSpawn, quantityExtracted);

		/// Send System Messages
		StringIdChatParameter harvestMessage("skl_use", creatureHealth);
		harvestMessage.setDI(quantityExtracted);
		harvestMessage.setTU(resourceSpawn->getFinalClass());
		player->sendSystemMessage(harvestMessage);

		updateMilkState(CreatureManager::ALREADYMILKED);
	}

	void clearStationary() {
		creature->removeObjectFlag(ObjectFlag::STATIONARY);
		creature->setAITemplate();
	}

	void updateMilkState(const short milkState) {
		Locker clocker(creature);
		creature->setMilkState(milkState);
		clearStationary();
	}
};

#endif /* MILKCREATURETASK_H_ */
