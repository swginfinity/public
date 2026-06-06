/*
 * MissionObjectiveImplementation.cpp
 *
 *  Created on: 22/06/2010
 *      Author: victor
 */

#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/objects/mission/MissionObjective.h"
#include "server/zone/objects/mission/MissionObserver.h"
#include "server/zone/objects/mission/MissionObject.h"
#include "server/zone/managers/planet/PlanetManager.h"
#include "terrain/manager/TerrainManager.h"
#include "server/zone/managers/object/ObjectManager.h"
#include "server/zone/Zone.h"
#include "server/zone/objects/player/PlayerObject.h"
#include "server/zone/objects/group/GroupObject.h"
#include "server/zone/managers/mission/MissionManager.h"
#include "server/zone/managers/statistics/StatisticsManager.h"
#include "server/zone/packets/player/PlayMusicMessage.h"
#include "server/zone/objects/mission/events/FailMissionAfterCertainTimeTask.h"
#include "events/CompleteMissionObjectiveTask.h"
#include "server/zone/objects/transaction/TransactionLog.h"
//Infinity:  Custom includes
#include "server/zone/managers/faction/FactionManager.h"

void MissionObjectiveImplementation::destroyObjectFromDatabase() {
	for (int i = 0; i < observers.size(); ++i) {
		MissionObserver* observer = observers.get(i);

		Locker clocker(observer, _this.getReferenceUnsafeStaticCast());

		observer->destroyObjectFromDatabase();
	}

	ObjectManager::instance()->destroyObjectFromDatabase(_this.getReferenceUnsafeStaticCast()->_getObjectID());
}

Reference<CreatureObject*> MissionObjectiveImplementation::getPlayerOwner() {
	ManagedReference<MissionObject*> strongReference = mission.get();

	if (strongReference != nullptr)
		return strongReference->getParentRecursively(SceneObjectType::PLAYERCREATURE).castTo<CreatureObject*>();

	return nullptr;
}

void MissionObjectiveImplementation::activate() {
	if (!activated) {
		ManagedReference<MissionObject* > mission = this->mission.get();

		if (mission == nullptr) {
			return;
		}

		activated = true;
		int64 timeElapsed = missionStartTime.miliDifference();
		int64 missionDuration = MISSIONDURATION;

		if (mission->getTypeCRC() == MissionTypes::BOUNTY) {
			missionDuration = ConfigManager::instance()->getInt("Core3.MissionManager.BountyExpirationTime", MISSIONDURATION);
		}

		int64 timeRemaining = missionDuration - timeElapsed;

		if (timeRemaining < 1) {
			timeRemaining = 1;
		}

		failTask = new FailMissionAfterCertainTimeTask(mission.get());
		failTask->schedule(timeRemaining);
	}
}

void MissionObjectiveImplementation::complete() {
	Locker _lock(_this.getReferenceUnsafeStaticCast());

	ManagedReference<CreatureObject*> player = getPlayerOwner();

	if (player == nullptr)
		return;

	_lock.release();

	Reference<CompleteMissionObjectiveTask*> task = new CompleteMissionObjectiveTask(_this.getReferenceUnsafeStaticCast());
	task->execute();

	if (player->isGrouped() && player->getGroup() != nullptr) {
		GroupObject* group = player->getGroup();
		Locker locker(group);
		group->scheduleUpdateNearestMissionForGroup(player->getPlanetCRC());
	}

	clearFailTask();
}

void MissionObjectiveImplementation::addObserver(MissionObserver* observer, bool makePersistent) {
	Locker _lock(_this.getReferenceUnsafeStaticCast());

	if (makePersistent) {
		ObjectManager::instance()->persistObject(observer, 1, "missionobservers");
	} else if (!observer->isDeployed())
		observer->deploy();

	observers.put(observer);
}

void MissionObjectiveImplementation::abort() {
	clearFailTask();
}

void MissionObjectiveImplementation::clearFailTask() {
	if (failTask != nullptr) {
		if (failTask->isScheduled())
			failTask->cancel();

		failTask = nullptr;
	}
}

void MissionObjectiveImplementation::awardFactionPoints() {

	ManagedReference<MissionObject*> mission = this->mission.get();
	if (mission == nullptr)
		return;

	//Infinity:  All heavily customized to split faction mission payouts

	FactionManager* factionManager = FactionManager::instance();
	if (factionManager == nullptr) {
		error("Faction manager not found.");
		return;
	}

	uint32 faction = mission->getFaction();	//Infinity:  Note, this is the faction of the mission terminal, not the NPCs
	int factionPointsImperial = mission->getRewardFactionPointsImperial();
	int factionPointsRebel = mission->getRewardFactionPointsRebel();

	if ((factionPointsRebel <= 0 && factionPointsImperial <= 0) || faction == Factions::FACTIONNEUTRAL || (faction != Factions::FACTIONIMPERIAL && faction != Factions::FACTIONREBEL)) {//Infinity:  Extra check
		return;
	}

	int factionPoints = 0;
	String factionString = "";

	if (faction == Factions::FACTIONIMPERIAL) { //Mission type is IMPERIAL, award the IMPERIAL mission faction amount. Mobs are REBEL.
		factionPoints = factionPointsImperial;
		factionString = "rebel";
	}
	else {
		factionPoints = factionPointsRebel; //Mission type is REBEL, award the REBEL mission faction amount. Mobs are IMPERIALk.
		factionString = "imperial";
	}

	auto player = getPlayerOwner();
	if (player != nullptr) {
	
		if (player->isGrouped()) {
		
			auto group = player->getGroup();
			if (group != nullptr) {

				Vector<ManagedReference<CreatureObject*> > players;

				for (int i = 0; i < group->getGroupSize(); i++) {

					auto groupMember = group->getGroupMember(i);
					if (groupMember != nullptr && groupMember->isPlayerCreature() ) {
						if (groupMember->isInRange(player, 128.0f)) {
							players.add(groupMember);
						}
					}
				}
				int groupSize = players.size();
				if (groupSize < 1)
						groupSize = 1;

				if (groupSize > 10)
					groupSize = 10;

				float groupDivisor = 1.0f;
				if (groupSize > 1) {
					groupDivisor += groupSize/10.0f;
				}
				factionPoints /= groupDivisor;

				for (int j=0; j < players.size(); j++) {
					factionManager->awardFactionStanding(players.elementAt(j), factionString, factionPoints);
				}
			}
		}
		else {
			factionManager->awardFactionStanding(player, factionString, factionPoints);
		}
	}
}

void MissionObjectiveImplementation::removeMissionFromPlayer() {
	ManagedReference<CreatureObject*> player = getPlayerOwner();
	ManagedReference<MissionObject* > mission = this->mission.get();

	if (player != nullptr && mission != nullptr) {
		ZoneServer* zoneServer = player->getZoneServer();
		MissionManager* missionManager = zoneServer->getMissionManager();

		missionManager->removeMission(mission, player);
	}
}

void MissionObjectiveImplementation::fail() {
	abort();
	removeMissionFromPlayer();
}

void MissionObjectiveImplementation::addMissionStats(TransactionLog& trx) {
	// Stub for subclasses to add mission type specific stats
}

void MissionObjectiveImplementation::awardReward() {
	ManagedReference<MissionObject* > mission = this->mission.get();

	if (mission == nullptr) {
		return;
	}

	Vector<ManagedReference<CreatureObject*> > players;
	PlayMusicMessage* pmm = new PlayMusicMessage("sound/music_mission_complete.snd");

	Vector3 missionEndPoint = getEndPosition();

	ManagedReference<CreatureObject*> owner = getPlayerOwner();

	if (owner == nullptr) {
		error() << "Mission " << mission->getObjectID() << " had nullptr owner";
		return;
	}

	ManagedReference<GroupObject*> group = owner->getGroup();

	auto ownerZone = owner->getZone();

	TransactionLog trx(owner, TrxCode::MISSIONCOMPLETE, mission, false);
	trx.addWorldPosition("src", owner);
	trx.addState("missionID", mission->getObjectID());
	trx.addState("missionType", mission->getTypeAsString());
	trx.addState("missionTitle", mission->getMissionTitle()->toString());
	trx.addState("missionDescription", mission->getMissionDescription()->toString());
	trx.addState("missionDifficulty", mission->getDifficulty());
	trx.addState("missionDifficultyDisplay", mission->getDifficultyDisplay());
	trx.addState("missionDifficultyLevel", mission->getDifficultyLevel());
	trx.addState("missionStartWorldPositionX", int(mission->getStartPositionX()));
	trx.addState("missionStartWorldPositionY", int(mission->getStartPositionY()));
	trx.addState("missionStartPlanet", mission->getStartPlanet());
	trx.addState("missionEndWorldPositionX", int(missionEndPoint.getX()));
	trx.addState("missionEndWorldPositionY", int(missionEndPoint.getY()));
	trx.addState("missionEndPlanet", ownerZone != nullptr ? ownerZone->getZoneName() : "-nullptr-");
	trx.addState("missionRewardCredits", mission->getRewardCredits());
	trx.addState("missionRefreshCounter", mission->getRefreshCounter());
	trx.addState("missionTarget", mission->getTargetName());
	trx.addState("missionSize", mission->getSize());
	trx.addState("missionTimeTotal", missionStartTime.miliDifference() / 1000);

	if (mission->getFaction() != Factions::FACTIONNEUTRAL) {
		trx.addState("missionFaction", mission->getFaction() == Factions::FACTIONIMPERIAL ? "imperial" : "rebel");
		trx.addState("missionRewardFactionPointsRebel", mission->getRewardFactionPointsRebel());
		trx.addState("missionRewardFactionPointsImperial", mission->getRewardFactionPointsImperial());
	}

	addMissionStats(trx);

	int playerCount = 1;
	int petCount = 0;
	int petOutOfRangeCount = 0;
	int petFactionCount = 0;
	int petFactionOutOfRangeCount = 0;

	if (group != nullptr) {
		Locker lockerGroup(group, _this.getReferenceUnsafeStaticCast());

		playerCount = group->getNumberOfPlayerMembers();

#ifdef LOCKFREE_BCLIENT_BUFFERS
	Reference<BasePacket*> pack = pmm;
#endif

		for (int i = 0; i < group->getGroupSize(); i++) {
			Reference<CreatureObject*> groupMember = group->getGroupMember(i);

			if (groupMember == nullptr) {
				continue;
			}

			trx.addRelatedObject(groupMember);

			if (groupMember->isPlayerCreature()) {
				//Play mission complete sound.
#ifdef LOCKFREE_BCLIENT_BUFFERS
				groupMember->sendMessage(pack);
#else
				groupMember->sendMessage(pmm->clone());
#endif
				Vector3 memberPosition = groupMember->getWorldPosition();

				if (mission->getTypeCRC() == MissionTypes::BOUNTY) {
					memberPosition.setZ(0);
				}

				if (memberPosition.distanceTo(missionEndPoint) < 128) {
					players.add(groupMember);
				}
			} else if(groupMember->isPet()) {
				Vector3 petPosition = groupMember->getWorldPosition();

				if (groupMember->getFaction() != 0) {
					petFactionCount++;
				} else {
					petCount++;
				}

				if (petPosition.distanceTo(missionEndPoint) >= 128) {
					if (groupMember->getFaction() != 0) {
						petFactionOutOfRangeCount++;
					} else {
						petOutOfRangeCount++;
					}
				}
			}
		}

#ifndef LOCKFREE_BCLIENT_BUFFERS
		delete pmm;
#endif
	} else {
		//Play mission complete sound.
		owner->sendMessage(pmm);
		players.add(owner);
	}

	if (players.size() == 0) {
		players.add(owner);
	}

	int divisor = mission->getRewardCreditsDivisor();
	bool expanded = false;

	/* Infinity:   Not adjusting payouts for group size
	if (playerCount > divisor) {
		divisor = playerCount;
		expanded = true;
	}

	if (playerCount > players.size()) {
		owner->sendSystemMessage("@mission/mission_generic:group_too_far"); // Mission Alert! Some group members are too far away from the group to receive their reward and and are not eligible for reward.
	}

	*/
	//int dividedReward = mission->getRewardCredits() / Math::max(divisor, 1);
	int dividedReward = mission->getRewardCredits() / players.size();   //Infinity:  Only split payout among players in range
	int bonusCreds = mission->getBonusCredits();
	int dividedBonus = 0;

	bool anonymousPlayerBounties = ConfigManager::instance()->getBool("Core3.MissionManager.AnonymousBountyTerminals", false);

	if (anonymousPlayerBounties && bonusCreds > 0) {
		trx.addState("missionBonusCredits", bonusCreds);
		//dividedBonus = bonusCreds / Math::max(divisor, 1);
		dividedBonus = bonusCreds / players.size();   //Infinity:  Only split payout among players in range
	}

	if (expanded) {
		trx.addState("missionExpanded", true);
	}

	trx.addState("missionRewardCreditsDivisor", divisor);
	trx.addState("missionPlayerInRangeCount", players.size());
	trx.addState("missionPlayerOutOfRangeCount", playerCount - players.size());
	trx.addState("missionPetCount", petCount);
	trx.addState("missionPetOutOfRangeCount", petOutOfRangeCount);
	trx.addState("missionPetFactionCount", petFactionCount);
	trx.addState("missionPetFactionOutOfRange", petFactionOutOfRangeCount);

	int totalRewarded = 0;
	int totalBonusRewarded = 0;

	for (int i = 0; i < players.size(); i++) {
		ManagedReference<CreatureObject*> player = players.get(i);
		StringIdChatParameter stringId("mission/mission_generic", "success_w_amount");
		stringId.setDI(dividedReward);
		player->sendSystemMessage(stringId);

		if (anonymousPlayerBounties && dividedBonus > 0) {
			String bonusString = "The Bounty Hunter guild has paid you a bonus in the amount of: " + String::valueOf(dividedBonus);
			player->sendSystemMessage(bonusString);
			totalBonusRewarded += dividedBonus;
		}

		Locker lockerPl(player, _this.getReferenceUnsafeStaticCast());
		TransactionLog trxReward(TrxCode::MISSIONSYSTEMDYNAMIC, player, dividedReward, false);
		trxReward.groupWith(trx);
		trxReward.addState("missionTrxId", trx.getTrxID());
		trxReward.addState("missionID", mission->getObjectID());

		player->addBankCredits(dividedReward + dividedBonus, true);
		totalRewarded += dividedReward;
	}

	// Catch any rounding errors etc.
	trx.addState("missionTotalRewarded", totalRewarded);

	if (anonymousPlayerBounties)
		trx.addState("missionTotalBonusRewarded", totalBonusRewarded);


	/* Remove unnecessary group payout messages
	if (group != nullptr) {
		if (expanded) {
			owner->sendSystemMessage("@mission/mission_generic:group_expanded"); // Group Mission Success! Reward credits have been transmitted to the bank account of all group members in the immediate area. They have been recalculated to reflect the newly added members.
		} else {
			owner->sendSystemMessage("@mission/mission_generic:group_success"); // Group Mission Success! Reward credits have been transmitted to the bank account of all group members in the immediate area.
		}
	}
	*/

	StatisticsManager::instance()->completeMission(mission->getTypeCRC(), totalRewarded);
}

Vector3 MissionObjectiveImplementation::getEndPosition() {
	ManagedReference<MissionObject* > mission = this->mission.get();

	Vector3 missionEndPoint;

	if (mission != nullptr) {

		missionEndPoint.setX(mission->getStartPositionX());
		missionEndPoint.setY(mission->getStartPositionY());

		auto playerOwner = getPlayerOwner();

		if (playerOwner != nullptr) {

			auto zone = playerOwner->getZone();
			if (zone != nullptr) {
				missionEndPoint.setZ(zone->getHeight(missionEndPoint.getX(), missionEndPoint.getY()));
			}
		}
	}

	return missionEndPoint;
}
