/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.
 */

#include "server/zone/managers/creature/LairObserver.h"
#include "server/zone/objects/tangible/LairObject.h"
#include "templates/params/ObserverEventType.h"
#include "server/zone/objects/creature/ai/AiAgent.h"
#include "server/zone/objects/creature/ai/Creature.h"
#include "server/zone/packets/object/PlayClientEffectObjectMessage.h"
#include "server/zone/packets/scene/PlayClientEffectLocMessage.h"
#include "server/zone/objects/tangible/threat/ThreatMap.h"
#include "server/zone/Zone.h"
#include "server/zone/managers/creature/CreatureManager.h"
#include "LairAggroTask.h"
#include "server/zone/objects/creature/ai/CreatureTemplate.h"
#include "server/zone/managers/creature/CreatureTemplateManager.h"
#include "server/zone/managers/creature/DisseminateExperienceTask.h"
#include "server/zone/managers/creature/LairRepopulateTask.h"
#include "server/zone/managers/creature/SpawnLairMobileTask.h"
#include "server/chat/ChatManager.h"
#include "server/zone/managers/combat/CombatManager.h"
// Infinity:  Custom includes
#include "server/zone/objects/area/ActiveArea.h"
#include "server/zone/objects/area/events/RemoveNoSpawnAreaTask.h"

//#define DEBUG_WILD_LAIRS
// #define DEBUG_LAIR_HEALING

int LairObserverImplementation::notifyObserverEvent(unsigned int eventType, Observable* observable, ManagedObject* arg1, int64 arg2) {
	// info(true) << "LairObserverImplementation::notifyObserverEvent -- " << " Observer Event Type: " << eventType;

	if (observable == nullptr) {
		return 1;
	}

	ManagedReference<TangibleObject*> attacker = nullptr;

	if (arg1 != nullptr) {
		attacker = cast<TangibleObject*>(arg1);
	}

	ManagedReference<TangibleObject*> lair = cast<TangibleObject*>(observable);

	if (lair == nullptr) {
		return 1;
	}

	Reference<LairAggroTask*> task = nullptr;
	AiAgent* agent = nullptr;

	ManagedReference<LairObserver*> lairObserver = _this.getReferenceUnsafeStaticCast();

#ifdef DEBUG_WILD_LAIRS
	info(true) << "Lair - Name: " << lair->getDisplayedName() << " ID: " << lair->getObjectID() << " Observer Event Type = " << eventType;
#endif // DEBUG_WILD_LAIRS

	switch (eventType) {
		// Lair despawned, remove mobiles
		case ObserverEventType::OBJECTREMOVEDFROMZONE:
			// info(true) << "Lair - Name: " << lair->getDisplayedName() << " ID: " << lair->getObjectID() << " Observer Event Type = " << eventType;

			despawnSpawns();
			return 1;
		// Lair destroyed notify destruction observers
		case ObserverEventType::OBJECTDESTRUCTION:
			// info(true) << "Lair - Name: " << lair->getDisplayedName() << " ID: " << lair->getObjectID() << " Observer Event Type = " << eventType;

			notifyDestruction(lair, attacker, (int)arg2);
			return 1;
		// Lair received damage — handle spawning, healing, aggro, and boss mobiles
		case ObserverEventType::DAMAGERECEIVED: {
			if (attacker == nullptr) {
				return 1;
			}

			auto zone = lair->getZone();

			if (zone == nullptr) {
				return 1;
			}

			String zoneQueueName = zone->getZoneName();

			Reference<TangibleObject*> lairRef = lair;
			Reference<TangibleObject*> attackerRef = attacker;

			// Aggro defenders when lair takes damage
			if (getLivingCreatureCount() > 0) {
				Reference<LairAggroTask*> aggroTask = new LairAggroTask(lair, attacker.get(), lairObserver, false);
				aggroTask->execute();
			}

			// Check for new spawns when the lair is not past the max spawn waves
			if (spawnNumber < 3) {
				Core::getTaskManager()->scheduleTask([lairObserver, lairRef, attackerRef]() {
					if (lairObserver == nullptr || lairRef == nullptr || attackerRef == nullptr) {
						return;
					}

					Locker locker(lairRef);

					lairObserver->checkForNewSpawns(lairRef, attackerRef);
				}, "CheckForNewSpawnsLambda", 1000, zoneQueueName.toCharArray());
			// Check to see if we should spawn a boss mobile (only after all waves spawned)
			} else if ((getMobType() == LairTemplate::CREATURE) && !lair->isDestroyed() && lairTemplate->hasBossMobs()) {
				int lairMaxCondition = lair->getMaxCondition();
				int lairConditionDamage = lair->getConditionDamage();
				int difficultyLevel = getDifficultyLevel();

				float bossChance = lairTemplate->getBossMobileChance();
				if (bossChance == 0) {
					bossChance = 20;
				}

				int bossRoll = System::random(100);

				if (bossRoll <= bossChance && (difficultyLevel * (lairMaxCondition / 5)) <= lairConditionDamage) {
					Core::getTaskManager()->scheduleTask([lairObserver, lairRef, attackerRef]() {
						if (lairObserver == nullptr || lairRef == nullptr || attackerRef == nullptr) {
							return;
						}

						Locker locker(lairRef);

						lairObserver->checkForBossSpawn(lairRef, attackerRef);
					}, "CheckForBossSpawnLambda", 200, zoneQueueName.toCharArray());
				}
			}

			// Pre-JTL event-based healing: all nearby defenders passively heal the lair
			checkForHeal(lair, attacker);

			break;
		}
		case ObserverEventType::AIMESSAGE:
			for (int i = 0; i < spawnedCreatures.size(); i++) {
				agent = cast<AiAgent*>(spawnedCreatures.get(i).get());

				if (agent == nullptr) {
					continue;
				}

				agent->notifyObservers(arg2, attacker);
			}

			break;
		// Used for checking if a creature lair should be repopulated for "milking"
		case ObserverEventType::NOPLAYERSINRANGE:
			// Check if the lair condition is below half
			if (lair->getConditionDamage() < (lair->getMaxCondition() * 0.5f)) {
				return 1;
			}

			if ((getMobType() == LairTemplate::CREATURE) && getSpawnNumber() > 1 && getLivingCreatureCount() >= 1) {
				Reference<LairRepopulateTask*> repopTask = new LairRepopulateTask(lair, lairObserver);

				if (repopTask != nullptr) {
					repopTask->schedule(10 * 1000);
				}
			}

			break;
		case ObserverEventType::CREATUREDESPAWNED: {
			// Attacker is the agent to be respawn for this observer trigger
			if (attacker == nullptr) {
				return 1;
			}

			auto zone = lair->getZone();

			if (zone == nullptr) {
				return 1;
			}

			String zoneQueueName = zone->getZoneName();

			Reference<LairObject*> lairRef = lair.castTo<LairObject*>();
			Reference<TangibleObject*> mobileRef = attacker;

			Core::getTaskManager()->scheduleTask([lairObserver, lairRef, mobileRef]() {
				if (lairObserver == nullptr || lairRef == nullptr || mobileRef == nullptr) {
					return;
				}

				Locker locker(lairRef);

				lairObserver->checkRespawn(lairRef, mobileRef);
			}, "RespawnLairMobileLambda", 1000, zoneQueueName.toCharArray());

			break;
		}
		default:
			break;
	}

	return 0;
}

void LairObserverImplementation::notifyDestruction(TangibleObject* lair, TangibleObject* attacker, int condition) {
	ThreatMap* threatMap = lair->getThreatMap();

	Reference<DisseminateExperienceTask*> deTask = new DisseminateExperienceTask(lair, threatMap, &spawnedCreatures, lair->getZone());
	deTask->execute();

	threatMap->removeObservers();
	threatMap->removeAll(); // we can clear the original one

	auto zone = lair->getZone();
	if (zone == nullptr) {
		spawnedCreatures.removeAll();
		return;
	}

	PlayClientEffectObjectMessage* explode = new PlayClientEffectObjectMessage(lair, "clienteffect/lair_damage_heavy.cef", "");
	lair->broadcastMessage(explode, false);

	PlayClientEffectLoc* explodeLoc = new PlayClientEffectLoc("clienteffect/lair_damage_heavy.cef", lair->getZone()->getZoneName(), lair->getPositionX(), lair->getPositionZ(), lair->getPositionY());
	lair->broadcastMessage(explodeLoc, false);

	ManagedReference<Zone*> strongZone = zone;
	Vector3 position = lair->getWorldPosition(); 

	Core::getTaskManager()->executeTask([strongZone, position] () {

		if (strongZone == nullptr) {
			return;
		}
					
		ManagedReference<ActiveArea*> area = (ServerCore::getZoneServer()->createObject(STRING_HASHCODE("object/active_area.iff"), 0)).castTo<ActiveArea*>();

		if (area != nullptr) {

			Locker locker(area);

			area->setAreaName("destructed_lair_nospawn");
			area->setRadius(ConfigManager::instance()->getSpawnCheckRange());   //Default 64 meters
			area->addAreaFlag(ActiveArea::NOSPAWNAREA);
			area->initializePosition(position.getX(), position.getZ(), position.getY());

			strongZone->transferObject(area, -1, true);

			Reference<Task*> task = new RemoveNoSpawnAreaTask(area);

			// Infinity:  Remove nospawn area after time has elapsed (set in config)
			if (task != nullptr) {
				task->schedule(ConfigManager::instance()->getNoSpawnAreaDuration());  //Default 60 seconds
			}
		}

	}, "CreateNoSpawnAreaTask");

	lair->destroyObjectFromWorld(true);

}

int LairObserverImplementation::getLivingCreatureCount() {
	int totalLiving = 0;
	Vector<ManagedReference<CreatureObject*>> pets;

	for (int i = 0; i < spawnedCreatures.size(); i++) {
		ManagedReference<CreatureObject*> cr = spawnedCreatures.get(i);

		if (cr->isPet()) {
			pets.add(cr);
			continue;
		}

		if (!cr->isDead() && cr->getZone() != nullptr) {
			totalLiving++;
		}
	}

	for (int j = 0; j < pets.size(); j++) {
		CreatureObject* pet = pets.get(j);
		spawnedCreatures.removeElement(pet);
	}

	return totalLiving;
}

/*
*
* Aggro
*
*/

void LairObserverImplementation::doAggro(TangibleObject* lair, TangibleObject* attacker, bool allAttack) {
	if (lair == nullptr || attacker == nullptr) {
		return;
	}

	for (int i = 0; i < spawnedCreatures.size(); ++i) {
		// If allAttack is false, roll now before running checks
		if (!allAttack && (System::random(100) < 50)) {
			continue;
		}

		auto creO = spawnedCreatures.get(i);

		if (creO == nullptr || creO->isDead() || creO->getZone() == nullptr || creO->isPet() || !creO->isAiAgent()) {
			continue;
		}

		Locker clocker(creO, lair);

		auto agent = creO->asAiAgent();

		if (agent == nullptr) {
			continue;
		}

		// Don't pull creatures off lair healing duty
		if (agent->getMovementState() == AiAgent::LAIR_HEALING) {
			continue;
		}

		Locker tarLock(attacker, creO);

		// Infinity: Use setDefender instead of addDefender — setDefender also sets
		// followObject and movementState to FOLLOWING, so the creature actually chases.
		// addDefender only updates the defender list/threat map but leaves the creature OBLIVIOUS.
		agent->setDefender(attacker);
	}
}

/*
*
* Healing — Pre-JTL passive approach
* All live defenders within 20m passively heal the lair (1% max HP per creature per tick).
* checkForHeal gates on lastHealTime to prevent spam. healLair reschedules via checkForHeal
* if damage remains, creating a continuous 1s heal loop while the lair is under attack.
*
*/

void LairObserverImplementation::checkForHeal(TangibleObject* lair, bool forceNewUpdate) {
	if (lair == nullptr || lair->isDestroyed() || getMobType() == LairTemplate::NPC) {
		return;
	}

	if (!(getLivingCreatureCount() > 0 && lair->getConditionDamage() > 0)) {
		return;
	}

	auto zone = lair->getZone();

	if (zone == nullptr) {
		return;
	}

	String zoneQueueName = zone->getZoneName();
	Reference<LairObserver*> lairObserver = _this.getReferenceUnsafeStaticCast();
	Reference<TangibleObject*> lairRef = lair;

	Core::getTaskManager()->scheduleTask([lairObserver, lairRef]() {
		if (lairObserver == nullptr || lairRef == nullptr) {
			return;
		}

		Locker lock(lairRef);

		lairObserver->healLair(lairRef);

		// If lair still has damage, reschedule healing in 1s (continuous loop)
		if (lairRef->getConditionDamage() > 0) {
			lairObserver->checkForHeal(lairRef, true);
		}
	}, "HealLairTask", 1000, zoneQueueName.toCharArray());
}

void LairObserverImplementation::healLair(TangibleObject* lair) {
	if (lair == nullptr || lair->getZone() == nullptr) {
		return;
	}

	int damageToHeal = 0;
	int lairMaxCondition = lair->getMaxCondition();

	for (int i = 0; i < spawnedCreatures.size(); ++i) {
		CreatureObject* creo = spawnedCreatures.get(i);

		if (creo == nullptr || creo->isDead() || creo->getZone() == nullptr) {
			continue;
		}

		if (lair->getDistanceTo(creo) > 20.0f) {
			continue;
		}

		damageToHeal += lairMaxCondition / 100;
	}

	if (damageToHeal == 0) {
		return;
	}

	lair->healDamage(lair, 0, damageToHeal, true);

	PlayClientEffectObjectMessage* heal = new PlayClientEffectObjectMessage(lair, "clienteffect/healing_healdamage.cef", "");
	lair->broadcastMessage(heal, false);

	PlayClientEffectLoc* healLoc = new PlayClientEffectLoc("clienteffect/healing_healdamage.cef", lair->getZone()->getZoneName(), lair->getPositionX(), lair->getPositionZ(), lair->getPositionY());
	lair->broadcastMessage(healLoc, false);
}

/*
*
* Spawning
*
*/

bool LairObserverImplementation::checkForNewSpawns(TangibleObject* lair, TangibleObject* attacker, bool forceSpawn) {
	if (lair == nullptr) {
		return false;
	}

	auto zone = lair->getZone();

	if (zone == nullptr) {
		return false;
	}

	auto lairObject = cast<LairObject*>(lair);

	if (lairObject == nullptr) {
		return false;
	}

	auto creatureManager = zone->getCreatureManager();

	if (creatureManager == nullptr) {
		return false;
	}

	bool isCreatureLair = false;
	int spawnLimit = lairTemplate->getSpawnLimit();

	if (getMobType() == LairTemplate::CREATURE) {
		isCreatureLair = true;
		// Infinity: drop upstream #9034's spawnLimit *= 3 — players reported the
		// resulting 45-creature cap (Kyrenic ticket-6105, "~30 as if static") felt
		// like an over-spawn. Restore pre-JTL behavior where the template's
		// spawnLimit IS the cap. The "milking" doubler below still applies, so a
		// repopulated lair can still produce 2× the base spawnLimit over its life.
	}

	// Lair limit is double due to "milking"
	if (lairObject->isRepopulated()) {
		spawnLimit *= 2;
	}

	// Schedule the aggro task
	if (attacker != nullptr && attacker->isCreatureObject() && lastAggroTime.isPast()) {
		lastAggroTime.updateToCurrentTime();
		lastAggroTime.addMiliTime(LairObserver::AGGRO_CHECK_INTERVAL * 1000);

		auto aggroTask = new LairAggroTask(lairObject, attacker, _this.getReferenceUnsafeStaticCast(), false);

		if (aggroTask != nullptr) {
			aggroTask->schedule(LairObserver::AGGRO_TASK_DELAY * 1000);
		}
	}

#ifdef DEBUG_WILD_LAIRS
	info(true) << "Wild Lair - Name: " << lair->getDisplayedName() << " ID: " << lair->getObjectID() << " Checking for new spawns with spawnNumber = " << spawnNumber << " Spawn Limit = " << spawnLimit << " Total Spawned Creatures = " << spawnedCreatures.size();
#endif // DEBUG_WILD_LAIRS

	if (forceSpawn) {
		spawnNumber.increment();
	} else if (getMobType() == LairTemplate::NPC) {
		return false;
	} else {
		// Spawn limit has been reached for lair
		if (spawnedCreatures.size() >= spawnLimit) {
			return false;
		}

		int conditionDamage = lairObject->getConditionDamage();
		int maxCondition = lairObject->getMaxCondition();

		switch (spawnNumber) {
			// Initial Spawn when lair is inserted to zone
			case 0:
				spawnNumber.increment();
				break;
			// 1st Wave of spawns when lair HP drops to 90% (10% damage threshold)
			case 1:
				if (conditionDamage > (maxCondition / 10)) {
					spawnNumber.increment();
				} else {
					return false;
				}
				break;
			// 2nd Wave of spawns when lair condition drops past half of the total condition
			case 2:
				if (conditionDamage > (maxCondition / 2)) {
					spawnNumber.increment();
				} else {
					return false;
				}
				break;
			default:
				return false;
				break;
		}
	}

	const Vector<String>* mobiles = lairTemplate->getWeightedMobiles();

	if (mobiles == nullptr || mobiles->size() == 0) {
		return false;
	}

	int amountToSpawn = 0;

	// The amount to spawn should be figured out using the lair difficulty
	if (isCreatureLair) {
		// Infinity: restore pre-JTL per-wave formula. Each wave (0, 1 at >10%
		// damage, 2 at >50% damage) spawns the same small chunk — for typical
		// creature lair spawnLimit=15 that's 3-5 creatures per wave, total
		// 9-15 across the lair's life. The gate at line ~475 caps total at
		// spawnLimit (or 2× when isRepopulated() — the "milking" doubler).
		//
		// Prior commit 12bb3427ba over-corrected by front-loading wave 0 with
		// the FULL spawnLimit, which immediately maxed the spawnedCreatures
		// counter and blocked waves 1 and 2 entirely (gate at 475 returned
		// false). Players reported: "wave 0 full set, then nothing" instead
		// of the expected pre-JTL multi-wave shape (Musashi 2026-05-27 TC).
		amountToSpawn = System::random(3) + ((lairTemplate->getSpawnLimit() / 3) - 2);

		if (amountToSpawn < 1) {
			amountToSpawn = 1;
		}
	} else {
		amountToSpawn = System::random(lairTemplate->getSpawnLimit() / 2) + (lairTemplate->getSpawnLimit() / 2);

		if (amountToSpawn < 1) {
			amountToSpawn = 1;
		}
	}

	// Build a spawn template map
	VectorMap<String, int> objectsToSpawn;

	for (int i = 0; i < amountToSpawn; i++) {
		int num = System::random(mobiles->size() - 1);
		const String& mob = mobiles->get(num);

		int find = objectsToSpawn.find(mob);

		if (find != -1) {
			int& value = objectsToSpawn.elementAt(find).getValue();

			value++;
		} else {
			objectsToSpawn.put(mob, 1);
		}
	}

	for (int i = 0; i < objectsToSpawn.size(); ++i) {
		const String& templateToSpawn = objectsToSpawn.elementAt(i).getKey();
		int numberToSpawn = objectsToSpawn.elementAt(i).getValue();

		for (int j = 0; j < numberToSpawn; j++) {
			// Lair mobile spawn task with passive mobile set to spawn
			Reference<SpawnLairMobileTask*> spawnTask = new SpawnLairMobileTask(lairObject, spawnNumber, templateToSpawn, true);

			if (spawnTask == nullptr) {
				continue;
			}

			// Initial spawn wave should spawn quicker then the others
			if (spawnNumber < 2) {
				// Initial spawn is spawned almost immediately
				spawnTask->schedule((j + 1) * 200);
			} else {
				// Create a spawn time that is at least between the min and max
				int spawnTime = System::random(LairObserver::SPAWN_TIME_MAX - LairObserver::SPAWN_TIME_MIN) + LairObserver::SPAWN_TIME_MIN;

				spawnTask->schedule(spawnTime * 1000);
			}
		}
	}

	return objectsToSpawn.size() > 0;
}

void LairObserverImplementation::checkForBossSpawn(TangibleObject* lair, TangibleObject* attacker) { //Infinity:  Lots of custom stuff here
	if (lair == nullptr || attacker == nullptr || lair->isDestroyed()) {
		return;
	}

	auto lairObject = cast<LairObject*>(lair);

	if (lairObject == nullptr) {
		return;
	}

	bool isRepopulated = lairObject->isRepopulated();

	if ((!isRepopulated && bossesSpawned > 0) || (isRepopulated && bossesSpawned > 1)) {
		return;
	}

	const VectorMap<String, int>* bossMobiles = lairTemplate->getBossMobiles();

	if (bossMobiles == nullptr || bossMobiles->size() < 1) {
		return;
	}

	int totalBossMobiles = bossMobiles->size();

	// Get a random boss mobile template from the list and spawn it
	try {
		int randomBoss = System::random(totalBossMobiles - 1);
		String templateToSpawn = bossMobiles->elementAt(randomBoss).getKey();

		Reference<SpawnLairMobileTask*> spawnTask = new SpawnLairMobileTask(lairObject, spawnNumber, templateToSpawn, false);

		if (spawnTask == nullptr) {
			return;
		}

		spawnTask->schedule(500);

		bossesSpawned.increment();
	} catch (Exception& e) {
		e.printStackTrace();
		error() << "exception in LairObserverImplementation::checkForBossSpawn -- " << e.getMessage();
	}

	Reference<LairAggroTask*> aggroTask = new LairAggroTask(lair, attacker, _this.getReferenceUnsafeStaticCast(), true);

	if (aggroTask != nullptr) {
		aggroTask->schedule(LairObserver::AGGRO_TASK_DELAY * 1000);
	}
}

void LairObserverImplementation::checkRespawn(LairObject* lair, TangibleObject* tanoAgent) {
	if (lair == nullptr || tanoAgent == nullptr || lair->isDestroyed()) {
		return;
	}

	auto zone = lair->getZone();

	// Lair will have a null zone if it is destroyed
	if (zone == nullptr) {
		return;
	}

	auto agent = tanoAgent->asAiAgent();

	if (agent == nullptr) {
		return;
	}

	if (agent->isScoutCreature()) {
		Locker clock(agent, lair);

		agent->removeObjectFlag(ObjectFlag::SCOUT);
		agent->setCustomObjectName("", false);

		// Clear stored scout creature ID
		scoutCreatureId = 0;
	}

	// Destroy missions do not respawn mobiles
	if (isDestroyMissionLairObserver()) {
		return;
	}

	String agentTemplate = agent->getObjectNameStringIdName();

	Reference<SpawnLairMobileTask*> spawnTask = new SpawnLairMobileTask(lair, spawnNumber, agentTemplate, false);

	if (spawnTask == nullptr) {
		return;
	}

	// Agents at lairs should auto respawn until the lair is destroyed
	int randomRespawn = (System::random(LairObserver::RESPAWN_TIME_MAX - LairObserver::RESPAWN_TIME_MIN) + LairObserver::RESPAWN_TIME_MIN);

#ifdef DEBUG_WILD_LAIRS
	randomRespawn = 20;
	info(true) << "Wild Lair - Name: " << lair->getDisplayedName() << " ID: " << lair->getObjectID() << " Respawning: " << agentTemplate << " in " << randomRespawn << " seconds.";
#endif // DEBUG_WILD_LAIRS

	spawnTask->schedule(randomRespawn * 1000);
}

void LairObserverImplementation::spawnLairMobile(LairObject* lair, int spawnNumber, const String& templateToSpawn, bool spawnPassive) {
	if (lair == nullptr) {
		return;
	}

	auto zone = lair->getZone();

	// Lair will have a null zone if it is destroyed
	if (zone == nullptr) {
		return;
	}

	auto creatureManager = zone->getCreatureManager();

	if (creatureManager == nullptr) {
		return;
	}

	auto creatureTemplate = CreatureTemplateManager::instance()->getTemplate(templateToSpawn);

	if (creatureTemplate == nullptr) {
		return;
	}

	int spawnLimit = lairTemplate->getSpawnLimit();
	bool isCreatureLair = false;
	bool spawnScout = false;

	if (getMobType() == LairTemplate::CREATURE) {
		isCreatureLair = true;
		// Infinity: matching cap change at line 441 in checkForNewSpawns —
		// drop upstream #9034's spawnLimit *= 3 here too so the CREATUREDESPAWNED
		// → checkRespawn → spawnLairMobile path honors the same pre-JTL cap.
		// Without removing this, killed creatures would silently respawn up to
		// 45 via spawnLairMobile even though checkForNewSpawns is capped at 15.

		if (scoutCreatureId == 0 && (System::random(100) <= LairObserver::SCOUT_SPAWN_CHANCE)) {
			spawnScout = true;
		}
	}

	if (spawnedCreatures.size() >= spawnLimit) {
		return;
	}

	float tamingChance = creatureTemplate->getTame();

	auto zoneServer = lair->getZoneServer();  // Infinity:  Better baby chance on TC
	if (zoneServer != nullptr && zoneServer->getGalaxyName() != "SWG Infinity") {
		tamingChance = Math::max(tamingChance, 0.25f);
	}

	uint32 lairTemplateCRC = templateToSpawn.hashCode();

	float x = lair->getPositionX() + (size - System::random(size * 20) / 10.0f);
	float y = lair->getPositionY() + (size - System::random(size * 20) / 10.0f);
	float z = zone->getHeight(x, y);

	ManagedReference<CreatureObject*> creature = nullptr;

	if (spawnNumber > 0 && creatureManager->checkSpawnAsBaby(tamingChance, babiesSpawned, LairObserver::BABY_SPAWN_CHANCE)) {
		creature = creatureManager->spawnCreatureAsBaby(lairTemplateCRC, x, z, y);

		babiesSpawned++;

		// Don't spawn baby as a scout
		spawnScout = false;
	}

	if (creature == nullptr) {
		creature = creatureManager->spawnCreatureWithAi(lairTemplateCRC, x, z, y);
	}

	if (creature == nullptr || !creature->isAiAgent()) {
		return;
	}

	auto agent = creature->asAiAgent();

	if (agent == nullptr) {
		return;
	}

#ifdef DEBUG_WILD_LAIRS
	if (spawnPassive) {
		info(true) << "Wild Lair - Name: " << lair->getDisplayedName() << " ID: " << lair->getObjectID() << " Spawned a regular Agent: " << agent->getDisplayedName() << " Total Spawned Creatures = " << spawnedCreatures.size();
	} else {
		info(true) << "Wild Lair - Name: " << lair->getDisplayedName() << " ID: " << lair->getObjectID() << " Spawned a (PASSIVE/Respawn/Boss) Wave Agent: " << agent->getDisplayedName() << " Total Spawned Creatures = " << spawnedCreatures.size();
	}
#endif // DEBUG_WILD_LAIRS

	Locker clocker(agent, lair);

	agent->setDespawnOnNoPlayerInRange(false);
	agent->setRespawnTimer(0);
	agent->setHomeLocation(x, z, y);
	agent->setHomeObject(lair);
	agent->setLairTemplateCRC(lairTemplateCRC);

	if (spawnScout) {
		agent->addObjectFlag(ObjectFlag::SCOUT);
		agent->setAITemplate();

		agent->setCustomObjectName(agent->getDisplayedName() + " (scout)", true);

		scoutCreatureId = agent->getObjectID();
	}

	// Add agent to the lairs creature list
	spawnedCreatures.add(agent);

	// Must be at least the baby and one other creature on the spawn to set a adult creature to social follow
	if (spawnedCreatures.size() > 1 && agent->isCreature()) {
		Creature* creature = cast<Creature*>(agent);

		if (creature != nullptr && creature->isBaby()) {
			ManagedReference<CreatureObject*> adultCreo = spawnedCreatures.get(0);

			if (adultCreo != nullptr && adultCreo->getObjectID() != creature->getObjectID()) {
				Locker adultLock(adultCreo, agent);

				agent->addObjectFlag(ObjectFlag::ESCORT);
				agent->addObjectFlag(ObjectFlag::FOLLOW);

				agent->setFollowObject(adultCreo);
				agent->setMovementState(AiAgent::FOLLOWING);

				agent->setAITemplate();

				float side = 1.f;

				if (System::random(100) < 50) {
					side = -1.f;
				}

				Vector3 formationOffset;
				formationOffset.setX(side * (adultCreo->getTemplateRadius() + agent->getTemplateRadius()));
				agent->writeBlackboard("formationOffset", formationOffset);
			}
		}
	}

	// Damage is not applied to initial spawn or NPC lairs
	if (!isCreatureLair || (spawnNumber < 2)) {
		return;
	}

	// Any spawn wave with the exception of the initial wave causes lair damage
	int totalAllowedSpawns = Math::max(1, Math::min(getDifficultyLevel(), lairTemplate->getSpawnLimit() / 3));
	int newDamage = (lair->getMaxCondition() / (totalAllowedSpawns * 5));

#ifdef DEBUG_WILD_LAIRS
	info(true) << "Wild Lair - Name: " << lair->getDisplayedName() << " ID: " << lair->getObjectID() << " Damaging Self from creature spawn: " << newDamage;
#endif // DEBUG_WILD_LAIRS

	lair->inflictDamage(lair, 0, newDamage, true, true, false);

	// Returning here for no passive spawn, lair is destroyed or we have hit the max passive spawns
	if (!spawnPassive || lair->isDestroyed() || spawnedCreatures.size() > LairObserver::WILD_LAIR_PASSIVE_MAX) {
		return;
	}

	// Spawn passive mobile
	Reference<SpawnLairMobileTask*> spawnTask = new SpawnLairMobileTask(lair, spawnNumber, templateToSpawn, false);

	if (spawnTask == nullptr) {
		return;
	}

	int totalThreats = 1;
	auto threatMap = lair->getThreatMap();

	if (threatMap != nullptr) {
		totalThreats = threatMap->size();
		threatMap = nullptr;
	}

#ifdef DEBUG_WILD_LAIRS
	info(true) << "Wild Lair - Name: " << lair->getDisplayedName() << " ID: " << lair->getObjectID() << " Total Threats = " << totalThreats;
#endif // DEBUG_WILD_LAIRS

	// Create a spawn time that is at least between the min and max
	int min = LairObserver::PASSIVE_SPAWN_TIME_MIN;
	int max = LairObserver::PASSIVE_SPAWN_TIME_MAX;

	if (totalThreats > 15) {
		min = 1;
		max = 2;
	} else if (totalThreats > 10) {
		min = LairObserver::PASSIVE_SPAWN_TIME_MIN - 10;
		max = LairObserver::PASSIVE_SPAWN_TIME_MAX - 10;
	} else if (totalThreats > 5) {
		min = LairObserver::PASSIVE_SPAWN_TIME_MIN - 5;
		max = LairObserver::PASSIVE_SPAWN_TIME_MAX - 5;
	}

	int spawnTime = System::random(max - min) + min;
	spawnTask->schedule(spawnTime * 1000);
}

void LairObserverImplementation::repopulateLair(TangibleObject* lairTano) {
	if (lairTano == nullptr) {
		return;
	}

	Locker lock(lairTano);

	int lairCond = lairTano->getMaxCondition();

	lairTano->setMaxCondition(lairCond * 0.75);
	lairTano->healDamage(lairTano, 0, lairCond, true);
	spawnNumber.set(0);

	LairObject* lair = cast<LairObject*>(lairTano);

	if (lair != nullptr)
		lair->setLairRepopulated(true);

	Reference<LairObserver*> lairObserver = _this.getReferenceUnsafeStaticCast();
	Reference<TangibleObject*> lairTanoRef = lairTano;

	Core::getTaskManager()->scheduleTask([lairTanoRef, lairObserver]() {
		Locker locker(lairTanoRef);
		lairObserver->checkForNewSpawns(lairTanoRef, nullptr);
	}, "CheckForNewSpawnsLambda", 1000);
}
