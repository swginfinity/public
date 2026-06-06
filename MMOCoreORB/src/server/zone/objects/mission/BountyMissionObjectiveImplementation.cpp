/*
 * BountyMissionObjectiveImplementation.cpp
 *
 *  Created on: 20/08/2010
 *      Author: dannuic
 */

#include "server/zone/objects/mission/BountyMissionObjective.h"

#include "server/zone/objects/waypoint/WaypointObject.h"
#include "server/zone/Zone.h"
#include "server/zone/ZoneServer.h"
#include "server/zone/managers/mission/MissionManager.h"
#include "server/zone/managers/creature/CreatureManager.h"
#include "server/zone/managers/player/PlayerManager.h"
#include "server/zone/objects/mission/MissionObject.h"
#include "server/zone/objects/mission/MissionObserver.h"
#include "server/zone/objects/player/PlayerObject.h"
#include "server/zone/objects/creature/ai/AiAgent.h"
#include "server/zone/objects/group/GroupObject.h"
#include "server/chat/ChatManager.h"
#include "server/zone/objects/mission/bountyhunter/BountyHunterDroid.h"
#include "server/zone/objects/mission/bountyhunter/events/BountyHunterTargetTask.h"
#include "server/zone/managers/visibility/VisibilityManager.h"
//Infinity:  Custom includes
#include "server/zone/packets/player/PlayMusicMessage.h"
#include "server/zone/packets/chat/ChatRoomMessage.h"
#include "server/zone/managers/frs/FrsManager.h"

void BountyMissionObjectiveImplementation::setNpcTemplateToSpawn(SharedObjectTemplate* sp) {
	npcTemplateToSpawn = sp;
}

void BountyMissionObjectiveImplementation::activate() {
	Locker locker(&syncMutex);

	MissionObjectiveImplementation::activate();

	if (isPlayerTarget()) {
		ManagedReference<MissionObject* > mission = this->mission.get();
		auto owner = getPlayerOwner();
		if (owner == nullptr || owner->getZoneServer() == nullptr)
			return;
		MissionManager* missionManager = owner->getZoneServer()->getMissionManager();

		if (missionManager == nullptr || mission == nullptr || !missionManager->hasPlayerBountyTargetInList(mission->getTargetObjectId())
				|| !missionManager->hasBountyHunterInPlayerBounty(mission->getTargetObjectId(), getPlayerOwner()->getObjectID()) || !addPlayerTargetObservers()) {
			getPlayerOwner()->sendSystemMessage("@mission/mission_generic:failed"); // Mission failed
			abort();
			removeMissionFromPlayer();
		}
	} else {
		startNpcTargetTask();

		if (getObserverCount() == 2 && npcTarget == nullptr) {
			removeNpcTargetObservers();
		}
	}
}

void BountyMissionObjectiveImplementation::deactivate() {
	MissionObjectiveImplementation::deactivate();

	if (activeDroid != nullptr) {
		if (!activeDroid->isPlayerCreature()) {
			Locker locker(activeDroid);
			activeDroid->destroyObjectFromDatabase();
			activeDroid->destroyObjectFromWorld(true);
		}

		activeDroid = nullptr;
	}

	cancelAllTasks();

	if (!isPlayerTarget()) {
		removeNpcTargetObservers();
	}
}

void BountyMissionObjectiveImplementation::abort() {
	Locker locker(&syncMutex);

	ManagedReference<MissionObject*> strongRef = mission.get();

	MissionObjectiveImplementation::abort();

	cancelAllTasks();

	if (activeDroid != nullptr) {
		if (!activeDroid->isPlayerCreature()) {
			Locker locker(activeDroid);
			activeDroid->destroyObjectFromDatabase();
			activeDroid->destroyObjectFromWorld(true);
		}

		activeDroid = nullptr;
	}

	if (strongRef == nullptr)
		return;

	ManagedReference<WaypointObject*> waypoint = strongRef->getWaypointToMission();
	if (waypoint != nullptr && waypoint->isActive()) {
		Locker wplocker(waypoint);
		waypoint->setActive(false);
	}

	//Remove observers
	if (hasObservers()) {
		if (isPlayerTarget()) {
			removePlayerTargetObservers();
		} else {
			removeNpcTargetObservers();
		}
	}
}

void BountyMissionObjectiveImplementation::complete() {
	Locker locker(&syncMutex);

	if (completedMission)
		return;

	cancelAllTasks();

	ManagedReference<MissionObject* > mission = this->mission.get();

	if (mission == nullptr) {
		return;
	}

	ManagedReference<CreatureObject*> owner = getPlayerOwner();
	if (owner == nullptr) {
		return;
	}
	auto zoneServer = owner->getZoneServer();
	if (zoneServer == nullptr) {
		return;
	}

	auto playerManager = zoneServer->getPlayerManager();
	if (playerManager == nullptr) {
		return;
	}
	auto missionManager = zoneServer->getMissionManager();
	if (missionManager == nullptr) {
		return;
	}
	//Award bountyhunter xp.

	int expGain = (mission->getRewardCredits() + mission->getBonusCredits()) / 50;

	playerManager->awardExperience(owner, "bountyhunter", expGain, true, 1);

	missionManager->completePlayerBounty(mission->getTargetObjectId(), owner->getObjectID());

	//Player target was killed
	ManagedReference<CreatureObject*> targetCreo = zoneServer->getObject(mission->getTargetObjectId()).castTo<CreatureObject*>();
	if (targetCreo != nullptr && targetCreo->isPlayerCreature()) {

		//Record BH Death Timestamp for FRS maintenance check
		auto ghost = targetCreo->getPlayerObject();
		if (ghost != nullptr) {
			missionManager->updatePlayerBountyReward(targetCreo->getObjectID(), ghost->calculateBhReward());  //Infinity:  Reset bounty to default based on skills
			ghost->setScreenPlayData("frs", "bh_death_timestamp", String::valueOf(Time().getMiliTime()));
		}
	}

	completedMission = true;

	locker.release();

	MissionObjectiveImplementation::complete();
}

void BountyMissionObjectiveImplementation::spawnTarget(const String& zoneName) {
	Locker locker(&syncMutex);

	ManagedReference<MissionObject* > mission = this->mission.get();

	if (mission == nullptr || (npcTarget != nullptr && npcTarget->isInQuadTree()) || isPlayerTarget()) {
		return;
	}

	auto owner = getPlayerOwner();
	if (owner == nullptr) {
		return;
	}

	ZoneServer* zoneServer = owner->getZoneServer();
	if (zoneServer == nullptr) {
		return;
	}
	Zone* zone = zoneServer->getZone(zoneName);

	if (zone == nullptr){
		error("null zone " + zoneName + " in BountyMissionObjective::spawnTarget");

		return;
	}

	CreatureManager* cmng = zone->getCreatureManager();

	if (npcTarget == nullptr) {
		Vector3 position = getTargetPosition();

		try {
			npcTarget = cast<AiAgent*>(zone->getCreatureManager()->spawnCreatureWithAi(mission->getTargetOptionalTemplate().hashCode(), position.getX(), zone->getHeight(position.getX(), position.getY()), position.getY(), 0));
		} catch (Exception& e) {
			fail();
			ManagedReference<CreatureObject*> player = getPlayerOwner();
			if (player != nullptr) {
				player->sendSystemMessage("ERROR: could not find template for target. Please report this on Mantis to help us track down the root cause.");
			}
			error("Template error: " + e.getMessage() + " Template = '" + mission->getTargetOptionalTemplate() +"'");
		}
		if (npcTarget != nullptr) {
			npcTarget->setCustomObjectName(mission->getTargetName(), true);
			//TODO add observer to catch player kill and fail mission in that case.
			addObserverToCreature(ObserverEventType::OBJECTDESTRUCTION, npcTarget);
			addObserverToCreature(ObserverEventType::DAMAGERECEIVED, npcTarget);
		} else {
			fail();
			ManagedReference<CreatureObject*> player = getPlayerOwner();
			if (player != nullptr) {
				player->sendSystemMessage("ERROR: could not find template for target. Please report this on Mantis to help us track down the root cause.");
			}
			error("Could not spawn template: '" + mission->getTargetOptionalTemplate() + "'");
		}
	}
}

int BountyMissionObjectiveImplementation::notifyObserverEvent(MissionObserver* observer, uint32 eventType, Observable* observable, ManagedObject* arg1, int64 arg2) {
	Locker locker(&syncMutex);

	if (eventType == ObserverEventType::OBJECTDESTRUCTION) {
		handleNpcTargetKilled(observable);
	} else if (eventType == ObserverEventType::DAMAGERECEIVED) {
		return handleNpcTargetReceivesDamage(arg1);
	} else if (eventType == ObserverEventType::PLAYERKILLED) {
		CreatureObject* victim = cast<CreatureObject*>(observable);
		handlePlayerKilled(arg1, victim != nullptr ? victim->getObjectID() : 0);
	}

	return 0;
}

void BountyMissionObjectiveImplementation::updateMissionStatus(int informantLevel) {
	Locker locker(&syncMutex);

	ManagedReference<MissionObject* > mission = this->mission.get();

	if (getPlayerOwner() == nullptr || mission == nullptr) {
		return;
	}

	switch (objectiveStatus) {
	case INITSTATUS:
		if (mission->getTargetOptionalTemplate() != "" && (targetTask == nullptr || !targetTask->isScheduled())) {
			startNpcTargetTask();
		}

		if (informantLevel == 1) {
			updateWaypoint();
		}
		objectiveStatus = HASBIOSIGNATURESTATUS;
		break;
	case HASBIOSIGNATURESTATUS:
		if (informantLevel > 1) {
			updateWaypoint();
		}
		objectiveStatus = HASTALKED;
		break;
	case HASTALKED:
		if (informantLevel > 1) {
			updateWaypoint();
		}
		break;
	default:
		break;
	}
}

void BountyMissionObjectiveImplementation::updateWaypoint() {
	Locker locker(&syncMutex);

	ManagedReference<MissionObject* > mission = this->mission.get();

	if (mission == nullptr)
		return;

	ManagedReference<WaypointObject*> waypoint = mission->getWaypointToMission();

	Locker wplocker(waypoint);

	waypoint->setPlanetCRC(getTargetZoneName().hashCode());
	Vector3 position = getTargetPosition();
	waypoint->setPosition(position.getX(), 0, position.getY());
	waypoint->setActive(true);

	mission->updateMissionLocation();

	if (mission->getMissionLevel() == 1) {
		getPlayerOwner()->sendSystemMessage("@mission/mission_bounty_informant:target_location_received"); // Target Waypoint Received.
	}
}

void BountyMissionObjectiveImplementation::performDroidAction(int action, SceneObject* sceneObject, CreatureObject* player) {
	Locker locker(&syncMutex);

	if (!playerHasMissionOfCorrectLevel(action)) {
		player->sendSystemMessage("@mission/mission_generic:bounty_no_ability"); // You do not understand how to use this item.
		return;
	}

	if (droid == nullptr) {
		droid = new BountyHunterDroid();
	}

	Reference<Task*> task = droid->performAction(action, sceneObject, player, getMissionObject().get());

	if (task != nullptr)
		droidTasks.add(task);
}

bool BountyMissionObjectiveImplementation::hasArakydFindTask() {
	Locker locker(&syncMutex);

	for (int i = 0; i < droidTasks.size(); i++) {
		Reference<Task*> task = droidTasks.get(i);

		if (task != nullptr) {
			Reference<FindTargetTask*> findTask = task.castTo<FindTargetTask*>();

			if (findTask != nullptr) {
				if (!findTask->isCompleted() && findTask->isArakydTask())
					return true;
			}
		}
	}

	return false;
}

bool BountyMissionObjectiveImplementation::playerHasMissionOfCorrectLevel(int action) {
	Locker locker(&syncMutex);

	ManagedReference<MissionObject* > mission = this->mission.get();
	if (mission == nullptr)
		return false;

	int levelNeeded = 2;
	if (action == BountyHunterDroid::FINDANDTRACKTARGET) {
		levelNeeded = 3;
	}

	return mission->getMissionLevel() >= levelNeeded;
}

Vector3 BountyMissionObjectiveImplementation::getTargetPosition() {
	Locker locker(&syncMutex);

	Vector3 empty;

	ManagedReference<MissionObject* > mission = this->mission.get();

	if (mission == nullptr)
		return empty;

	if (isPlayerTarget()) {
		uint64 targetId = mission->getTargetObjectId();

		ZoneServer* zoneServer = getPlayerOwner()->getZoneServer();
		if (zoneServer != nullptr) {
			ManagedReference<CreatureObject*> creature = zoneServer->getObject(targetId).castTo<CreatureObject*>();

			if (creature != nullptr) {
				Vector3 targetPos = creature->getWorldPosition();
				targetPos.setZ(0);
				return targetPos;
			}
		}
	} else {
		if (targetTask != nullptr) {
			return targetTask->getTargetPosition();
		}
	}

	return empty;
}

void BountyMissionObjectiveImplementation::cancelAllTasks() {
	Locker locker(&syncMutex);

	if (targetTask != nullptr && targetTask->isScheduled()) {
		targetTask->cancel();
		targetTask = nullptr;
	}

	/*for (int i = 0; i < droidTasks.size(); i++) {
		Reference<Task*> droidTask = droidTasks.get(i);

		if (droidTask != nullptr && droidTask->isScheduled()) {
			droidTask->cancel();
		}
	}*/

	droidTasks.removeAll();
}

void BountyMissionObjectiveImplementation::cancelCallArakydTask() {
	Locker locker(&syncMutex);

	for (int i = 0; i < droidTasks.size(); i++) {
		Reference<Task*> task = droidTasks.get(i);

		if (task != nullptr) {
			Reference<CallArakydTask*> callTask = task.castTo<CallArakydTask*>();

			if (callTask != nullptr && callTask->isScheduled()) {
				callTask->cancel();
			}
		}
	}
}

String BountyMissionObjectiveImplementation::getTargetZoneName() {
	Locker locker(&syncMutex);

	ManagedReference<MissionObject* > mission = this->mission.get();
	if (mission == nullptr)
		return "dungeon1";

	if (isPlayerTarget()) {
		uint64 targetId = mission->getTargetObjectId();

		ZoneServer* zoneServer = getPlayerOwner()->getZoneServer();
		if (zoneServer != nullptr) {
			ManagedReference<CreatureObject*> creature = zoneServer->getObject(targetId).castTo<CreatureObject*>();

			if (creature != nullptr && creature->getZone() != nullptr) {
				return creature->getZone()->getZoneName();
			}
		}
	} else {
		if (targetTask != nullptr) {
			return targetTask->getTargetZoneName();
		}
	}

	//No target task, return dungeon1 which is not able to find.
	return "dungeon1";
}

void BountyMissionObjectiveImplementation::removePlayerTargetObservers() {
	Locker locker(&syncMutex);

	ManagedReference<MissionObject* > mission = this->mission.get();
	ManagedReference<CreatureObject*> owner = getPlayerOwner();

	if (owner == nullptr || mission == nullptr)
		return;

	removeObserver(1, ObserverEventType::PLAYERKILLED, owner);

	ZoneServer* zoneServer = owner->getZoneServer();

	if (zoneServer != nullptr) {
		ManagedReference<CreatureObject*> target = zoneServer->getObject(mission->getTargetObjectId()).castTo<CreatureObject*>();

		removeObserver(0, ObserverEventType::PLAYERKILLED, target);
	}
}

void BountyMissionObjectiveImplementation::removeNpcTargetObservers() {
	if (npcTarget != nullptr) {
		ManagedReference<SceneObject*> npcHolder = npcTarget.get();
		Locker locker(npcTarget);

		removeObserver(1, ObserverEventType::DAMAGERECEIVED, npcTarget);
		removeObserver(0, ObserverEventType::OBJECTDESTRUCTION, npcTarget);

		npcTarget->destroyObjectFromDatabase();
		npcTarget->destroyObjectFromWorld(true);

		npcTarget = nullptr;
	} else {
		// NPC not spawned, remove observers anyway.
		Locker locker(&syncMutex);

		for (int i = getObserverCount() - 1; i >= 0; i--) {
			dropObserver(getObserver(i), true);
		}
	}
}

void BountyMissionObjectiveImplementation::removeObserver(int observerNumber, unsigned int observerType, CreatureObject* creature) {
	Locker locker(&syncMutex);

	if (getObserverCount() <= observerNumber) {
		//Observer does not exist.
		return;
	}

	ManagedReference<MissionObserver*> observer = getObserver(observerNumber);

	if (creature != nullptr)
		creature->dropObserver(observerType, observer);

	dropObserver(observer, true);
}

void BountyMissionObjectiveImplementation::addObserverToCreature(unsigned int observerType, CreatureObject* creature) {
	ManagedReference<MissionObserver*> observer = new MissionObserver(_this.getReferenceUnsafeStaticCast());
	addObserver(observer, true);

	creature->registerObserver(observerType, observer);
}

bool BountyMissionObjectiveImplementation::addPlayerTargetObservers() {
	Locker locker(&syncMutex);

	ManagedReference<MissionObject* > mission = this->mission.get();
	ManagedReference<CreatureObject*> owner = getPlayerOwner();

	if (mission == nullptr || owner == nullptr)
		return false;

	ZoneServer* zoneServer = owner->getZoneServer();

	if (zoneServer != nullptr) {
		ManagedReference<CreatureObject*> target = zoneServer->getObject(mission->getTargetObjectId()).castTo<CreatureObject*>();

		if (target != nullptr) {
			addObserverToCreature(ObserverEventType::PLAYERKILLED, target);

			addObserverToCreature(ObserverEventType::PLAYERKILLED, owner);

			//Update aggressive status on target for bh.
			target->sendPvpStatusTo(owner);

			return true;
		}
	}

	return false;
}

void BountyMissionObjectiveImplementation::startNpcTargetTask() {
	Locker locker(&syncMutex);

	ManagedReference<MissionObject* > mission = this->mission.get();

	if (mission == nullptr)
		return;

	if (targetTask == nullptr)
		targetTask = new BountyHunterTargetTask(mission, getPlayerOwner(), mission->getEndPlanet());

	if (targetTask != nullptr && !targetTask->isScheduled()) {
		targetTask->schedule(10 * 1000);
	}
}

bool BountyMissionObjectiveImplementation::isPlayerTarget() {
	ManagedReference<MissionObject* > mission = this->mission.get();
	if (mission == nullptr)
		return false;

	return mission->getTargetOptionalTemplate() == "";
}

void BountyMissionObjectiveImplementation::handleNpcTargetKilled(Observable* observable) {
	CreatureObject* target =  cast<CreatureObject*>(observable);
	ManagedReference<CreatureObject*> owner = getPlayerOwner();

	if (owner == nullptr || target == nullptr)
		return;

	SceneObject* targetInventory = target->getSlottedObject("inventory");

	if (targetInventory == nullptr)
		return;

	uint64 lootOwnerID = targetInventory->getContainerPermissions()->getOwnerID();
	GroupObject* group = owner->getGroup();

	if (lootOwnerID == owner->getObjectID() || (group != nullptr && lootOwnerID == group->getObjectID())) {
		//Target killed by player, complete mission.
		complete();
	} else {
		//Target killed by other player, fail mission.
		owner->sendSystemMessage("@mission/mission_generic:failed"); // Mission failed
		abort();
		removeMissionFromPlayer();
	}
}

int BountyMissionObjectiveImplementation::handleNpcTargetReceivesDamage(ManagedObject* arg1) {
	CreatureObject* target = nullptr;

	target = cast<CreatureObject*>(arg1);

	ManagedReference<MissionObject* > mission = this->mission.get();
	ManagedReference<CreatureObject*> owner = getPlayerOwner();

	if (mission != nullptr && owner != nullptr && target != nullptr && target->getFirstName() == owner->getFirstName() &&
			target->isPlayerCreature() && objectiveStatus == HASBIOSIGNATURESTATUS) {
		updateMissionStatus(mission->getMissionLevel());

		String diffString = "easy";
		if (mission->getMissionLevel() == 3) {
			diffString = "hard";
		} else if (mission->getMissionLevel() == 2) {
			diffString = "medium";
		}

		target->getZoneServer()->getChatManager()->broadcastChatMessage(npcTarget, "@mission/mission_bounty_neutral_" + diffString + ":m" + String::valueOf(mission->getMissionNumber()) + "v", 0, 0, npcTarget->getMoodID());
		return 1;
	}

	return 0;
}

void BountyMissionObjectiveImplementation::handlePlayerKilled(ManagedObject* arg1, uint64 destructedID) {
	if (completedMission) {
		return;
	}

	ManagedReference<CreatureObject*> creo = cast<CreatureObject*>(arg1);
	if (creo == nullptr) {
		return;
	}
	ManagedReference<CreatureObject*> killer = nullptr;

	if (creo->isPet()) {
		killer = creo->getLinkedCreature().get();
	}
	else {
		killer = creo;
	}

	if (killer == nullptr) {
		return;
	}

	auto zoneServer = killer->getZoneServer();
	if (zoneServer == nullptr) {
		return;
	}

	ManagedReference<MissionObject*> mission = this->mission.get();
	ManagedReference<CreatureObject*> owner = getPlayerOwner();

	if (mission == nullptr || owner == nullptr) {
		return;
	}

	uint64 targetID = mission->getTargetObjectId();
	uint64 ownerID = owner->getObjectID();
	uint64 killerID = killer->getObjectID();

	ManagedReference<CreatureObject*> target = zoneServer->getObject(targetID).castTo<CreatureObject*>();
	if (target == nullptr) {
		return;
	}

	// DoT self-kill guard: skip if killer is the dead player (self-DoT death).
	// destructedID = observable->getObjectID() (the player the observer fired on).
	// INFINITY_BH_DOT_GUARD
	if (killerID == destructedID) {
		return;
	}
	// info(true) << "BountyMissionObjectiveImplementation::handlePlayerKilled -- Owner: " << ownerID << " Killer: " << killerID << " Mission Target ID: " << targetID << " Destructed ID: " << destructedID;

	// Fail Mission if the target killed the owner
	if (killerID == targetID && ownerID != killerID) {
		owner->sendSystemMessage("@mission/mission_generic:failed"); // Mission failed

		if (killer->isPlayerCreature()) {

			if (ConfigManager::instance()->getBool("Core3.MissionManager.PlayerBountyMarkVictoryCooldown", false)) {
				killer->updateCooldownTimer("BountyMarkVictory", ConfigManager::instance()->getInt("Core3.MissionManager.PlayerBountyMarkVictoryCooldownTime", 1800000));   //30 minute cooldown for mark who kills hunter
			}

			// Get Hunter Name
			String hunterName = owner->getFirstName();
			String hunterLast = owner->getLastName();
			if (hunterLast != "") {
				hunterName += " " + hunterLast;
			}
			hunterName = hunterName.trim();
		
			// Get Hunted Name
			String huntedName = killer->getFirstName();
			String huntedLast = killer->getLastName();
			if (huntedLast != "") {
				huntedName += " " + huntedLast;
			}
			huntedName = huntedName.trim();

			//Infinity: Setup logging and broadcast
			String guildTag = " Bounty Hunter ";
			if (owner->hasSkill("mandalorian_master"))
				guildTag = " Mandalorian ";

			killer->sendSystemMessage("\\#E66300[Unknown Transmission] \\#FFFFFFGreat job putting that" + guildTag + "in their place!  Don't let your guard down!");
			warning("Intercepted Hutt Transmission: A marked target by the name of " +  huntedName + " has defeated a" + guildTag + "named " + hunterName);

			if (ConfigManager::instance()->isPvpBroadcastChannelEnabled()) {

				auto chatManager = zoneServer->getChatManager();
				if (chatManager != nullptr) {

					StringBuffer zBroadcast;
					zBroadcast << "\r\\#FFA500" << "Bounty Hunter Guild Alert:" << "\r\\#FFFFFF" << "  A marked target named < " << huntedName;
					zBroadcast << " > has defeated a" << guildTag << "named < " << hunterName << " >.";

					auto pvpBroadcastRoom = chatManager->getPvpBroadcastRoom();
					if (pvpBroadcastRoom != nullptr) {

						UnicodeString message(zBroadcast.toString());
						UnicodeString formattedMsg(chatManager->formatMessage(message));
						BaseMessage* msg = new ChatRoomMessage("", zoneServer->getGalaxyName(), formattedMsg, pvpBroadcastRoom->getRoomID());
						pvpBroadcastRoom->broadcastMessage(msg);
					}
				}
			}

			// Setup music to play for the player upon victory!
			PlayMusicMessage* pmm = new PlayMusicMessage("sound/music_themequest_victory_imperial.snd");
			killer->sendMessage(pmm);		
		}

		fail();

		return;
	}

	// Killer must be the mission owner to return successful
	if (killerID != ownerID)
		return;

	// Target killed by player, complete mission.

	if (ConfigManager::instance()->getBool("Core3.MissionManager.PlayerBountyMarkVictoryCooldown", false))
		owner->updateCooldownTimer("BountyMarkVictory", 0);   //Infinity:  Hunter just claimed a bounty, remove BH terminal immunity cooldown

	// Get Hunter Name
	String hunterName = owner->getDisplayedName();

	// Get Hunted Name
	String huntedName = target->getDisplayedName();

	//Infinity: Setup logging and broadcast
	String guildTag = " Bounty Hunter ";
	if (owner->hasSkill("mandalorian_master"))
		guildTag = " Mandalorian ";

	warning("Bounty Hunter Guild Broadcast: " + guildTag + hunterName + " has claimed the bounty on " + huntedName);

	if (ConfigManager::instance()->isPvpBroadcastChannelEnabled()) {

		auto chatManager = zoneServer->getChatManager();
		if (chatManager != nullptr) {

			StringBuffer zBroadcast;
			zBroadcast << "\r\\#FFA500" << "Bounty Hunter Guild Alert:" << "\r\\#FFFFFF" << "  A" << guildTag << "named < " << hunterName;
			zBroadcast << " > has claimed the bounty on a marked target named < " << huntedName << " >.";

			auto pvpBroadcastRoom = chatManager->getPvpBroadcastRoom();
			if (pvpBroadcastRoom != nullptr) {

				UnicodeString message(zBroadcast.toString());
				UnicodeString formattedMsg(chatManager->formatMessage(message));
				BaseMessage* msg = new ChatRoomMessage("", zoneServer->getGalaxyName(), formattedMsg, pvpBroadcastRoom->getRoomID());
				pvpBroadcastRoom->broadcastMessage(msg);
			}
		}
	}

	//Infinity: Play music for the player only!!
	PlayMusicMessage* pmm = new PlayMusicMessage("sound/music_themequest_victory_imperial.snd");
	owner->sendMessage(pmm);

	//InfinitY:  Clear visibility on target
	VisibilityManager* visibilityManager = VisibilityManager::instance();
	if (visibilityManager != nullptr)
		visibilityManager->clearVisibility(target);

	if (target->hasSkill("force_title_jedi_rank_02")) {  //Only take XP if Padawan or higher
		int minXpLoss = -50000;
		int maxXpLoss = -500000;

		int rewardCreds = mission->getRewardCredits() + mission->getBonusCredits();
		int xpLoss = rewardCreds * -2;

		if (xpLoss > minXpLoss)
			xpLoss = minXpLoss;
		else if (xpLoss < maxXpLoss)
			xpLoss = maxXpLoss;

		auto playerManager = zoneServer->getPlayerManager();
		if (playerManager != nullptr) {
			playerManager->awardExperience(target, "jedi_general", xpLoss, true, 1.0, false);  //Don't use XP modifier

			StringIdChatParameter message("base_player", "prose_revoke_xp");
			message.setDI(-xpLoss);
			message.setTO("exp_n", "jedi_general");
			target->sendSystemMessage(message);
		}

		if (target->hasSkill("force_rank_light_novice") || target->hasSkill("force_rank_dark_novice")) {

			bool verboseLogging = ConfigManager::instance()->getBool("Core3.PlayerManager.VerboseFRSLogging", false);
			auto frsManager = zoneServer->getFrsManager();
			if (frsManager != nullptr) {

				int bhPenalty = 3500;

				ManagedReference<CreatureObject*> targetRef = target;
				Core::getTaskManager()->executeTask([targetRef, frsManager, bhPenalty] () {
					Locker locker(targetRef);
					Locker clocker(frsManager, targetRef);
					frsManager->adjustFrsExperience(targetRef, -bhPenalty);
				}, "FrsExperienceAdjustLambdaTask");

				if (verboseLogging) {
					error("FRS: killer is BH and target is FRS Jedi, deducting " + String::valueOf(bhPenalty) + " FRS from target");
				}
			}
		}
	}

	complete();
}
