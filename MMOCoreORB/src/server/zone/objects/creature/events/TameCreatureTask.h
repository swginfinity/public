
#ifndef TAMECREATURETASK_H_
#define TAMECREATURETASK_H_

#include "server/zone/managers/combat/CombatManager.h"
#include "server/zone/managers/creature/PetManager.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/objects/intangible/PetControlDevice.h"
#include "server/zone/objects/transaction/TransactionLog.h"
#include "server/chat/ChatManager.h"
#include "server/zone/objects/creature/events/SpawnCreatureTask.h"
#include "engine/engine.h"
//Infinity:  Custom includes
#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/managers/creature/DynamicSpawnObserver.h"
#include "server/zone/managers/gcw/observers/SquadObserver.h"
#include "server/zone/managers/creature/observers/CreatureHerdObserver.h"

class TameCreatureTask : public Task {

private:
	enum Phase { INITIAL, SECOND, FINAL} currentPhase;
	int originalMask;
	int creatureMask;  //Infinity:  Added creatureMask
	ManagedWeakReference<Creature*> mob;
	ManagedWeakReference<CreatureObject*> play;
	bool force;
	bool adult;

public:
	TameCreatureTask(Creature* cre, CreatureObject* playo, int pvpMask, int creatureBitmask, bool forced, bool adults) : Task() {  //Infinity:  Added args
		currentPhase = INITIAL;
		mob = cre;
		play = playo;
		originalMask = pvpMask;
		creatureMask = creatureBitmask;  //Infinity
		force = forced;
		adult = adults;
	}

	void run() {
		ManagedReference<Creature*> creature = mob.get();
		ManagedReference<CreatureObject*> player = play.get();

		if (creature == nullptr || player == nullptr)
			return;

		Locker locker(creature);

		Locker _clocker(player, creature);

		player->removePendingTask("tame_pet");

		if (force) {
			success(adult);
			return;
		}

		if (!creature->isInRange(player, 8.0f)) {
			player->sendSystemMessage("@hireling/hireling:taming_toofar"); // You are too far away to continue taming.
			creature->showFlyText("npc_reaction/flytext","toofar", 204, 0, 0);  // You are too far away to tame the creature.
			resetStatus();
			return;
		}

		if (!creature->canTameMe(player)) {
			player->sendSystemMessage("@pet/pet_menu:sys_cant_tame"); // You can't tame that
			resetStatus();
			return;
		}

		ChatManager* chatManager = player->getZoneServer()->getChatManager();
		ManagedReference<PlayerObject*> ghost = player->getPlayerObject();

		if (ghost == nullptr)
			return;

		switch (currentPhase) {
		case INITIAL:
			chatManager->broadcastChatMessage(player, "@hireling/hireling:taming_" + String::valueOf(System::random(4) + 1), 0, 0, player->getMoodID(), 0, ghost->getLanguageID());
			player->doAnimation("");
			currentPhase = SECOND;
			player->addPendingTask("tame_pet", this, 10000);
			break;
		case SECOND:
			chatManager->broadcastChatMessage(player, "@hireling/hireling:taming_" + String::valueOf(System::random(4) + 1), 0, 0, player->getMoodID(), 0, ghost->getLanguageID());
			currentPhase = FINAL;
			player->addPendingTask("tame_pet", this, 10000);
			break;
		case FINAL:
			float tamingChance = creature->getChanceToTame(player);

			if (tamingChance > System::random(100))
				success(false);
			else {
				player->sendSystemMessage("@hireling/hireling:taming_fail"); // You fail to tame the creature.
				creature->showFlyText("npc_reaction/flytext","fail", 204, 0, 0);  // You fail to tame the creature.
				resetStatus();

				int ferocity = creature->getFerocity();
				int aggroChance = System::random(20 - ferocity);

				if (aggroChance == 0 && creature->isAiAgent()) {
					AiAgent* agent = creature->asAiAgent();

					if (agent != nullptr) {
						Locker aLock(agent);
						agent->addDefender(player);
					}
				}
			}

			break;
		}

		return;
	}

	void success(bool adult) {
		ManagedReference<CreatureObject*> player = play.get();
		ManagedReference<Creature*> creature = mob.get();

		if (creature == nullptr || player == nullptr)
			return;

		ZoneServer* zoneServer = player->getZoneServer();

		String objectString = creature->getControlDeviceTemplate();
		if (objectString == "")
			objectString = "object/intangible/pet/pet_control.iff";

		SceneObject* datapad = player->getSlottedObject("datapad");
		PlayerManager* playerManager = zoneServer->getPlayerManager();
		ObjectManager* objectManager = zoneServer->getObjectManager();

		if (datapad == nullptr || playerManager == nullptr || objectManager == nullptr) {
			resetStatus();
			return;
		}

		ManagedReference<PetControlDevice*> controlDevice = zoneServer->createObject(objectString.hashCode(), 1).castTo<PetControlDevice*>();

		if (controlDevice == nullptr) {
			resetStatus();
			return;
		}

		Locker deviceLocker(controlDevice);

		controlDevice->setControlledObject(creature);

		StringId s;
		s.setStringId(creature->getObjectName()->getFullPath());

		controlDevice->setObjectName(s, false);
		controlDevice->setPetType(PetManager::CREATUREPET);
		controlDevice->setMaxVitality(100);
		controlDevice->setVitality(100);
		controlDevice->setGrowthStage(1);
		controlDevice->updateStatus(1);
		controlDevice->setCustomObjectName(creature->getCustomObjectName(), true);

		if (!datapad->transferObject(controlDevice, -1)) {
			resetStatus();
			controlDevice->destroyObjectFromDatabase(true);
			return;
		}

		objectManager->persistSceneObjectsRecursively(creature, 1);

		if (adult) {
			controlDevice->growPet(player, true, true);
		}
		else {
			float startingLevel = ((float)creature->getAdultLevel() / 10.0);
			creature->setPetLevel(startingLevel);
		}

		creature->setControlDevice(controlDevice);
		creature->setObjectMenuComponent("PetMenuComponent");
		creature->setCreatureLink(player);
		creature->setFaction(player->getFaction());

		uint32 playerPvpStatusBitmask = player->getPvpStatusBitmask();

		if (playerPvpStatusBitmask & ObjectFlag::PLAYER) {
			playerPvpStatusBitmask &= ~ObjectFlag::PLAYER;

			creature->setPvpStatusBitmask(playerPvpStatusBitmask, false);
		} else {
			creature->setPvpStatusBitmask(playerPvpStatusBitmask, false);
		}

		creature->setBaby(false);

		if (creature->isAiAgent()) {

			AiAgent* agent = cast<AiAgent*>(creature.get());
			ManagedReference<CellObject*> parent = player->getParent().get().castTo<CellObject*>();

			float respawn = agent->getRespawnTimer() * 1000;

			ManagedReference<SceneObject*> home = agent->getHomeObject().get();

			if (respawn > 0 && home == nullptr) {  //This is for static spawns only, all others are handled by lair or simply removed

				if (agent->getRandomRespawn()) {
					respawn = System::random(respawn) + (respawn / 2.f);
				}

				uint32 tempCRC = 0;
				auto crTemplate = agent->getCreatureTemplate();

				if (crTemplate != nullptr)
					tempCRC = crTemplate->getTemplateName().hashCode();

				PatrolPoint* homeLoc = agent->getHomeLocation();

				Reference<Task*> task = new SpawnCreatureTask(tempCRC, agent->getRespawnTimer(), creature->getZone()->getZoneName(), homeLoc->getPositionX(), homeLoc->getPositionZ(),
											homeLoc->getPositionY(), agent->getParentID(), agent->getRandomRespawn());
				task->schedule(respawn);

			}
			else if (respawn == 0 && home != nullptr) {//This is for dynamic lairs

				SortedVector<ManagedReference<Observer*> > observers = home->getObservers(ObserverEventType::CREATUREDESPAWNED);
				DynamicSpawnObserver* dynamicObserver = nullptr;

				for (int i = 0; i < observers.size(); i++) {
					dynamicObserver = observers.get(i).castTo<DynamicSpawnObserver*>();

					if (dynamicObserver != nullptr) {
						break;
					}
				}

				if (dynamicObserver != nullptr) {							//Infinity:  Remove from spawnedCreatures vector
					dynamicObserver->removeSpawnedCreature(agent);
				}

				// Remove Squad observer from herding creatures
				if (agent->isMonster()) {
					SortedVector<ManagedReference<Observer* > > observers = agent->getObservers(ObserverEventType::SQUAD);
					for (int i = observers.size() - 1; i >= 0; --i) {
						ManagedReference<SquadObserver*> squadObserver = cast<SquadObserver*>(observers.get(i).get());

						if (squadObserver != nullptr) {
							agent->dropObserver(ObserverEventType::SQUAD, squadObserver);
						}
					}
				}
			}

			agent->setLairTemplateCRC(0);

			// Infinity: Tamed pets are limited to 2 specials per attack map
			agent->trimAttackMaps(2);

			// Drop HERD observer — tamed pets should not retain herd behavior
			if (agent->getObserverCount(ObserverEventType::HERD) > 0) {
				SortedVector<ManagedReference<Observer*> > herdObservers = agent->getObservers(ObserverEventType::HERD);

				for (int i = 0; i < herdObservers.size(); i++) {
					CreatureHerdObserver* herdObserver = cast<CreatureHerdObserver*>(herdObservers.get(i).get());

					if (herdObserver != nullptr) {
						agent->dropObserver(ObserverEventType::HERD, herdObserver);
					}
				}
			}

			agent->setCreatureBitmask(ObjectFlag::PET);
			agent->setAITemplate();
			agent->clearPatrolPoints();

			agent->setHomeLocation(player->getPositionX(), player->getPositionZ(), player->getPositionY(), parent);
			agent->setNextStepPosition(player->getPositionX(), player->getPositionZ(), player->getPositionY(), parent);

			controlDevice->setLastCommand(PetManager::FOLLOW);
			controlDevice->setLastCommander(player);

			agent->eraseBlackboard("restingTime");
			agent->setPosture(CreaturePosture::UPRIGHT, true, true);

			agent->setFollowObject(player);
			agent->storeFollowObject();
			agent->setMovementState(AiAgent::FOLLOWING);
		}

		creature->getZone()->broadcastObject(creature, true);
		datapad->broadcastObject(controlDevice, true);

		ManagedReference<PlayerObject*> ghost = player->getPlayerObject();
		ghost->addToActivePets(creature);

		auto creatureTemplate = creature->getCreatureTemplate();

		if (creatureTemplate != nullptr)
			playerManager->awardExperience(player, "creaturehandler", 20 * creatureTemplate->getLevel());
		else
			playerManager->awardExperience(player, "creaturehandler", 20 * creature->getLevel());

		player->sendSystemMessage("@hireling/hireling:taming_success"); // You successfully tame the creature.
		creature->showFlyText("npc_reaction/flytext","success", 0, 204, 0);  // You tame the creature.
		creature->notifyObservers(ObserverEventType::CREATURETAMED, player, 0);

		// Infinity: log creature tame to trx so we can track which species/locations are popular
		TransactionLog trx(player, creature, controlDevice, TrxCode::CREATURETAME);
		auto creatureTmpl = creature->getObjectTemplate();
		trx.addState("subjectTemplate", creatureTmpl != nullptr ? creatureTmpl->getFullTemplateString() : "");
		trx.addState("subjectIsAdult", adult);
		trx.addState("subjectLevel", creature->getLevel());

		auto creatureZone = creature->getZone();
		if (creatureZone != nullptr)
			trx.addState("subjectPlanet", creatureZone->getZoneName());

		trx.addState("subjectPosX", creature->getPositionX());
		trx.addState("subjectPosY", creature->getPositionY());
		trx.addState("subjectPosZ", creature->getPositionZ());
	}

	void resetStatus() {
		ManagedReference<Creature*> creature = mob.get();
		if (creature == nullptr)
			return;

		creature->setPvpStatusBitmask(originalMask, true);

		//Infinity:  Only restore original creature mask if a special BABYONLY spawn
		if (creatureMask & ObjectFlag::BABYONLY) {
			creature->setCreatureBitmask(creatureMask);
		}
		else {
			creature->removeObjectFlag(ObjectFlag::STATIONARY);
		}

		creature->setAITemplate();

		creature->setMovementState(AiAgent::FOLLOWING);
	}
};

#endif /* TAMECREATURETASK_H_ */
