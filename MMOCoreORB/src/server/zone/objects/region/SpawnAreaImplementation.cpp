/*
 * SpawnAreaImplementation.cpp
 *
 *  Created on: 11/12/2011
 *      Author: victor
 */

#include "server/zone/objects/region/SpawnArea.h"
#include "server/zone/Zone.h"
#include "server/zone/managers/creature/CreatureManager.h"
#include "server/zone/managers/player/PlayerManager.h"
#include "server/zone/managers/creature/CreatureTemplateManager.h"
#include "server/zone/managers/creature/SpawnGroup.h"
#include "server/zone/managers/planet/PlanetManager.h"
#include "server/zone/objects/region/SpawnAreaObserver.h"
#include "server/zone/objects/area/areashapes/AreaShape.h"
#include "server/ServerCore.h"
#include "server/zone/objects/area/events/RemoveNoSpawnAreaTask.h"

// #define DEBUG_SPAWNING
// #define DEBUG_LAIR_DIFFICULTY

void SpawnAreaImplementation::notifyPositionUpdate(TreeEntry* entry) {
	if (numberOfPlayersInRange <= 0 || entry == nullptr) {
		return;
	}

	SceneObject* sceneObject = cast<SceneObject*>(entry);

	if (sceneObject == nullptr || !sceneObject->isPlayerCreature()) {
		return;
	}

	auto zoneServer = getZoneServer();

	if (zoneServer != nullptr && (zoneServer->isServerLoading() || zoneServer->isServerShuttingDown())) {
		return;
	}

	if (lastSpawn.miliDifference() < ConfigManager::instance()->getMinLairSpawnInterval()) {
		return;
	}

	if (ConfigManager::instance()->disableWorldSpawns()) {
		return;
	}

#ifdef DEBUG_SPAWNING
	info(true) << getAreaName() << " --SpawnAreaImplementation::notifyPositionUpdate called";
#endif // DEBUG_SPAWNING

	CreatureObject* player = sceneObject->asCreatureObject();

	if (player == nullptr || player->isInvisible())
		return;

	tryToSpawn(player);
}

void SpawnAreaImplementation::notifyEnter(SceneObject* sceneO) {
	if (sceneO == nullptr || !sceneO->isPlayerCreature())
		return;

	CreatureObject* creO = cast<CreatureObject*>(sceneO);
	if(creO != nullptr && creO->isInvisible())
		return;	

	if (sceneO->isDebuggingRegions())
		sendDebugMessage(sceneO, true);

#ifdef DEBUG_SPAWNING
	info(true) << getAreaName() << " --SpawnAreaImplementation::notifyEnter for " << sceneO->getCustomObjectName();
#endif // DEBUG_SPAWNING

	numberOfPlayersInRange.increment();
}

void SpawnAreaImplementation::notifyExit(SceneObject* sceneO) {
	if (sceneO == nullptr || !sceneO->isPlayerCreature())
		return;

	CreatureObject* creO = cast<CreatureObject*>(sceneO);
	if(creO != nullptr && creO->isInvisible())
		return;			

	if (sceneO->isDebuggingRegions())
		sendDebugMessage(sceneO, false);

#ifdef DEBUG_SPAWNING
	info(true) << getAreaName() << " --SpawnAreaImplementation::notifyExit for " << sceneO->getCustomObjectName();
#endif // DEBUG_SPAWNING

	numberOfPlayersInRange.decrement();

	if (numberOfPlayersInRange <= 0) {
		numberOfPlayersInRange.set(0);
	}
}

void SpawnAreaImplementation::buildSpawnList(Vector<uint32>* groupCRCs) {
	CreatureTemplateManager* ctm = CreatureTemplateManager::instance();

	for (int i = 0; i < groupCRCs->size(); i++) {
		SpawnGroup* group = ctm->getSpawnGroup(groupCRCs->get(i));

		if (group == nullptr)
			continue;

		const Vector<Reference<LairSpawn*> >& spawnList = group->getSpawnList();

		for (int j = 0; j < spawnList.size(); j++) {
			const auto& spawn = spawnList.get(j);

			possibleSpawns.emplace(spawn);

			totalWeighting += spawn->getWeighting();
		}
	}
}

// This will return a random position near the player within the area shape
Vector3 SpawnAreaImplementation::getRandomPosition(SceneObject* player) {
	Vector3 position;

	if (player == nullptr) {
		position.set(0, 0, 0);
		return position;
	}

#ifdef DEBUG_SPAWNING
	info(true) << getAreaName() << " -- getRandomPosition -- for Player " << player->getDisplayedName() << " ID: " << player->getObjectID();
	info(true) << getAreaName() << " Location = " << getPositionX() << " , " << getPositionY();
#endif // DEBUG_SPAWNING

	const auto worldPosition = player->getWorldPosition();

	position = areaShape->getRandomPosition(worldPosition, 32.0f, ZoneServer::CLOSEOBJECTRANGE);

	return position;
}

int SpawnAreaImplementation::notifyObserverEvent(unsigned int eventType, Observable* observable, ManagedObject* arg1, int64 arg2) {
	if (eventType != ObserverEventType::OBJECTREMOVEDFROMZONE)
		return 0;

	SceneObject* sceneO = dynamic_cast<SceneObject*>(observable);

	if (sceneO == nullptr)
		return 1;

#ifdef DEBUG_SPAWNING
	info(true) << "SpawnAreaImplementation::notifyObserverEvent -- Event Type: " << eventType << " for " << sceneO->getCustomObjectName() << " ID: " << sceneO->getObjectID();
#endif // DEBUG_SPAWNING

	Locker locker(_this.getReferenceUnsafeStaticCast());

	uint32 lairTemplate = spawnTypes.remove(sceneO->getObjectID());

	if (lairTemplate != 0) {
		int currentSpawnCount = spawnCountByType.get(lairTemplate) - 1;

		if (currentSpawnCount < 1)
			spawnCountByType.remove(lairTemplate);
		else
			spawnCountByType.put(lairTemplate, currentSpawnCount);

		--totalSpawnCount;

		locker.release();

		Zone* thisZone = getZone();

		/* Infinity:  This is now handled in LairObserverImplementation
		if (sceneO->isLairObject() && thisZone != nullptr) {

			ManagedReference<ActiveArea*> area = (ServerCore::getZoneServer()->createObject(STRING_HASHCODE("object/active_area.iff"), 0)).castTo<ActiveArea*>();

			if (area != nullptr) {
				Locker locker(area);

				area->setAreaName("destructed_lair_nospawn");
				area->setRadius(ConfigManager::instance()->getSpawnCheckRange());   //Default 64 meters
				area->addAreaFlag(ActiveArea::NOSPAWNAREA);
				area->initializePosition(sceneO->getPositionX(), sceneO->getPositionZ(), sceneO->getPositionY());

				thisZone->transferObject(area, -1, true);

				Reference<Task*> task = new RemoveNoSpawnAreaTask(area);

				if (task != nullptr) {
					// Infinity:  Remove nospawn area after time has elapsed (set in config)
					task->schedule(ConfigManager::instance()->getNoSpawnAreaDuration());  //Default 60 seconds
				}

#ifdef DEBUG_SPAWNING
				info(true) << "NOSPAWNAREA radius = " << ConfigManager::instance()->getSpawnCheckRange() << ", scheduled to depawn in " << ConfigManager::instance()->getNoSpawnAreaDuration()/1000.0f << " seconds.";
#endif // DEBUG_SPAWNING
			}
		}
		*/
	}

	return 1;
}

void SpawnAreaImplementation::tryToSpawn(CreatureObject* player) {
	if (player == nullptr)
		return;

#ifdef DEBUG_SPAWNING
	info(true) << "SpawnAreaImplementation::tryToSpawn for " << player->getDisplayedName() << " ID: " << player->getObjectID() << " Total Possible Lair Types = " << possibleSpawns.size();
#endif // DEBUG_SPAWNING

	ReadLocker _readlocker(_this.getReferenceUnsafeStaticCast());

	auto zone = getZone();

	if (zone == nullptr) {
		return;
	}

	if (totalSpawnCount >= maxSpawnLimit) {
#ifdef DEBUG_SPAWNING
		info(true) << "Total Spawn Count Reached for - " << getAreaName() << " Max Limit: " <<  maxSpawnLimit << " Total Spawn Count: " << totalSpawnCount;
#endif // DEBUG_SPAWNING
		return;
	}

	int choice = System::random(totalWeighting - 1);
	int counter = 0;

	LairSpawn* finalSpawn = nullptr;

	for (int i = 0; i < possibleSpawns.size(); i++) {
		LairSpawn* spawn = possibleSpawns.get(i);

		counter += spawn->getWeighting();

		if (choice < counter) {
			finalSpawn = spawn;
			break;
		}
	}

	if (finalSpawn == nullptr) {
#ifdef DEBUG_SPAWNING
		info(true) << "tryToSpawn -- finalSpawnis nullptr";
#endif // DEBUG_SPAWNING
		return;
	}

	String lairTemplate = finalSpawn->getLairTemplateName();
	uint32 lairHashCode = lairTemplate.hashCode();

	int spawnLimit = finalSpawn->getSpawnLimit();
	int currentSpawnCount = spawnCountByType.get(lairHashCode);

	currentSpawnCount = ((currentSpawnCount < 0) ? 0 : currentSpawnCount);

	// Make sure spawn area limit has not been reached
	if (spawnLimit > -1 && currentSpawnCount >= spawnLimit) {
#ifdef DEBUG_SPAWNING
		info(true) << "tryToSpawn -- Spawn Limit Reached - spawnLimit: " << spawnLimit << " currentSpawnCount: " << currentSpawnCount;
#endif // DEBUG_SPAWNING
		return;
	}

	auto planetManager = zone->getPlanetManager();

	if (planetManager == nullptr) {
		return;
	}

	Vector3 randomPosition = getRandomPosition(player);
	float randomX = randomPosition.getX();
	float randomY = randomPosition.getY();

	// We do not want the random position to be 0, 0
	if ((randomX < 0.01) && (randomX > -0.01) && (randomY < 0.01) && (randomY > -0.01)) {
#ifdef DEBUG_SPAWNING
		info(true) << "tryToSpawn -- Failed due to random position near 0, 0 -- Chosen position: " << randomPosition.toString();
#endif // DEBUG_SPAWNING

		return;
	}

	float checkRange = finalSpawn->getSize() + ConfigManager::instance()->getSpawnCheckRange();   //Default 64 meters

	// Check the spot to see if spawning is allowed
	if (!planetManager->isSpawningPermittedAt(randomPosition.getX(), randomPosition.getY(), checkRange, isWorldSpawnArea())) {
#ifdef DEBUG_SPAWNING
		info(true) << "tryToSpawn Spawning is not permitted at " << randomPosition.toString();
#endif // DEBUG_SPAWNING
		return;
	}

	// Get terrain height for lair
	float spawnZ = zone->getHeight(randomPosition.getX(), randomPosition.getY());
	randomPosition.setZ(spawnZ);

	// Caculate the lair difficulty
	// Highest player level is 25 and highest group level is 120
	int playerLevel = getPlayerSpawnLevel(player);

#ifdef DEBUG_LAIR_DIFFICULTY
	playerLevel = System::random(120);
#endif //DEBUG_LAIR_DIFFICULTY

	// Lair template difficulty
	int maxDifficulty = finalSpawn->getMaxDifficulty();
	int minDifficulty = finalSpawn->getMinDifficulty();

	// Infinity: Restore pre-JTL difficulty formula — upstream JTL formula
	// (playerLevel / lairLevel) is broken for high-level content because
	// playerLevel caps at ~25 while lair levels go to 300+, resulting in
	// lairBuildingLevel always being 1-2 instead of scaling 0-4.
	// INFINITY_LAIR_DIFFICULTY_FIX
	if (maxDifficulty == 500)
		maxDifficulty = minDifficulty + (playerLevel * 2.f);

	float difficultyLevel = (((maxDifficulty - minDifficulty) / 25.f) * playerLevel) + minDifficulty + System::random(5);

	if (difficultyLevel < minDifficulty)
		difficultyLevel = minDifficulty;

	float maxRange = maxDifficulty > (minDifficulty + 5) ? (float)(maxDifficulty - minDifficulty) : 5.f;
	int lairBuildingLevel = (float)(difficultyLevel - minDifficulty) / (maxRange / 5.f);

	if (lairBuildingLevel >= 5)
		lairBuildingLevel = 4;

	float lairLevel = difficultyLevel;

	_readlocker.release();

	auto creatureManager = zone->getCreatureManager();

	if (creatureManager == nullptr) {
		return;
	}

	ManagedReference<SceneObject*> obj = creatureManager->spawn(lairHashCode, lairLevel, lairBuildingLevel, randomPosition.getX(), spawnZ, randomPosition.getY(), finalSpawn->getSize());

	if (obj == nullptr) {
		error("Failed to spawn lair: " + lairTemplate);
		return;
	}

#ifdef DEBUG_SPAWNING
	info(true) << "lair spawned at " << obj->getPositionX() << " " << obj->getPositionY();
#endif // DEBUG_SPAWNING

#ifdef DEBUG_LAIR_DIFFICULTY
	float baseCondition = CreatureManager::CREATURE_LAIR_MIN;

	switch(lairBuildingLevel) {
		case 2: {
			baseCondition = 3000.f;
			break;
		}
		case 3: {
			baseCondition = 6000.f;
			break;
		}
		case 4: {
			baseCondition = 9000.f;
			break;
		}
		case 5: {
			baseCondition = 18000.f;
			break;
		}
		default:
			break;
	}

	uint32 conditionCalc = Math::min((float)CreatureManager::CREATURE_LAIR_MAX, (System::random(baseCondition) + ((baseCondition / 10) * lairLevel)));

	StringBuffer lairDifficultyMsg;
	lairDifficultyMsg
	<< endl << "-----------------------------------" << endl
	<< "--- Wild Lair Spawn Difficulty ---" << endl
	<< "-----------------------------------" << endl
	<< "Lair: " << lairTemplate << endl
	<< "Player Level = " << playerLevel << endl
	<< "Min Difficulty = " << minDifficulty << endl
	<< "Max Difficulty = " << maxDifficulty << endl
	<< "Difficulty Range (Max - Min) = " << lairRange << endl
	<< "Lair Level: " << lairLevel << endl
	<< "Calculated Difficulty Level: " << difficultyVariable << endl
	<< "Lair Building Level: " << lairBuildingLevel << endl
	<< "Lair Condition Calculated: " << conditionCalc << endl
	<< "-----------------------------------" << endl;
	info(true) << lairDifficultyMsg.toString();
#endif // DEBUG_LAIR_DIFFICULTY

	Locker _locker2(_this.getReferenceUnsafeStaticCast());

	lastSpawn.updateToCurrentTime();

	if (spawnAreaObserver == nullptr) {
		spawnAreaObserver = new SpawnAreaObserver(_this.getReferenceUnsafeStaticCast());
		spawnAreaObserver->deploy();
	}

	spawnTypes.put(obj->getObjectID(), lairHashCode);

	Locker clocker(obj, _this.getReferenceUnsafeStaticCast());

	obj->registerObserver(ObserverEventType::OBJECTREMOVEDFROMZONE, spawnAreaObserver);

	totalSpawnCount++;

	spawnCountByType.put(lairTemplate.hashCode(), currentSpawnCount + 1);

#ifdef DEBUG_SPAWNING
		info(true) << "tryToSpawn Complete";
#endif // DEBUG_SPAWNING

	return;
}

int SpawnAreaImplementation::getPlayerSpawnLevel(CreatureObject* player) {
	int level = 1;

	if (player == nullptr) {
		return level;
	}

	if (player->isGrouped()) {
		auto group = player->getGroup();

		if (group != nullptr) {
			level = group->getGroupLevel();

#ifdef DEBUG_LAIR_DIFFICULTY
			info(true) << "Returning Group Level: " << level;
#endif // DEBUG_LAIR_DIFFICULTY
		}
	} else {
		auto zoneServer = player->getZoneServer();

		if (zoneServer != nullptr) {
			auto playerMan = zoneServer->getPlayerManager();

			if (playerMan != nullptr) {
				level = playerMan->calculatePlayerLevel(player);

#ifdef DEBUG_LAIR_DIFFICULTY
				info(true) << "Returning Players Solo Level: " << level;
#endif // DEBUG_LAIR_DIFFICULTY
			}
		}
	}

	return level;
}
