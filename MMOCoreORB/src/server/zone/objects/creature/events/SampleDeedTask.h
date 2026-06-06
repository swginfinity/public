#ifndef SAMPLEDEEDTASK_H_
#define SAMPLEDEEDTASK_H_

#include "server/zone/managers/creature/DnaManager.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/objects/tangible/deed/pet/PetDeed.h"
#include "templates/params/creature/CreatureAttribute.h"
#include "engine/engine.h"

class SampleDeedTask : public Task {
private:
	enum Phase { BEGIN, SAMPLING, END } currentPhase;
	int waitCount;
	ManagedReference<PetDeed*> deed;
	ManagedReference<CreatureObject*> player;

public:
	SampleDeedTask(PetDeed* obj, CreatureObject* playo) : Task() {
		currentPhase = BEGIN;
		waitCount = 0;
		deed = obj;
		player = playo;
	}

	void run() {
		if (deed == nullptr || player == nullptr)
			return;

		Locker locker(player);
		Locker crosslocker(deed, player);

		player->removePendingTask("sampledeed");

		if (deed->isGenerated()) {
			return;
		}

		if (!player->hasSkill("outdoors_bio_engineer_novice") || !deed->isASubChildOf(player)) {
			return;
		}

		//Infinity:  Define standard and custom initial values
		int mindCost = player->calculateCostAdjustment(CreatureAttribute::FOCUS, 200);
		int maxWaitCount = 5;  //Infinity:  Default live value
		int skillMod = player->getSkillMod("dna_harvesting");
		int maxSamples = floor(skillMod / 15.0f);
		int cl = deed->getLevel();
		bool liveServer = true;
		String galaxyName = "SWG Infinity";
		auto zoneServer = player->getZoneServer();
		if (zoneServer != nullptr) {
			galaxyName = zoneServer->getGalaxyName();
		}
		if (galaxyName != "SWG Infinity") { //Infinity: easier rules on test servers
			maxWaitCount = 0;
			liveServer = false;
			maxSamples = 100;
		}

		if (cl > 350) {
			player->sendSystemMessage("@bio_engineer:harvest_dna_level_too_high"); // You cannot sample DNA from a creature over level 350.
			return;
		}	

		//Infinity:  Can sample up to 350
		if (skillMod < 1 || (cl > 75 && !player->hasSkill("outdoors_bio_engineer_master"))) {
			player->sendSystemMessage("@bio_engineer:harvest_dna_skill_too_low");
			return;
		}

		switch (currentPhase) {
			case BEGIN:
				// We should be good to go now and try the sample
				if (player->getHAM(CreatureAttribute::MIND) < mindCost) {
					player->sendSystemMessage("@bio_engineer:harvest_dna_attrib_too_low");
				} else {
					player->inflictDamage(player, CreatureAttribute::MIND, mindCost, false, true);
					player->sendSystemMessage("@bio_engineer:harvest_dna_begin_harvest");
					currentPhase = SAMPLING;
					player->addPendingTask("sampledeed",this,1000);
				}
				break;
			case SAMPLING:
				// Infinity:  9 to 5, from calculation above for testing
				if (waitCount == maxWaitCount) {
					currentPhase = END;
				}else {
					waitCount++;
				}
				player->addPendingTask("sampledeed",this,1000);
				break;
			case END:
				int level = deed->getLevel();
				float rollMod = (((skillMod-level)/level))  + (skillMod-level);
				int sampleRoll = System::random(100);
				sampleRoll += System::random(player->getSkillMod("luck") + player->getSkillMod("force_luck"));
				deed->incrementSampleCount();

				if (liveServer && (50 + rollMod) < sampleRoll) {//Infinity:  No failure on TC and dev boxes
					// failure but we increment the count
					player->sendSystemMessage("@bio_engineer:harvest_dna_failed");
				}
				else {
					player->sendSystemMessage("@bio_engineer:harvest_dna_succeed");
					award(cl, rollMod);
				}
				if (deed->getSampleCount() > maxSamples) {
					// nuke deed you killed it
					ManagedReference<SceneObject*> deedContainer = deed->getParent().get();
					if (deedContainer != nullptr) {
						deed->destroyObjectFromWorld(true);
					}
					deed->destroyObjectFromDatabase(true);
				}
				break;
		}
		return;
	}

	void award(int cl, float rollMod) {
		int xp = DnaManager::instance()->generateXp(cl);
		ManagedReference<PlayerManager*> playerManager = player->getZoneServer()->getPlayerManager();

		if (playerManager != nullptr)
			playerManager->awardExperience(player, "bio_engineer_dna_harvesting", xp, true);

		int quality = deed->getQuality();
		int newQuality = quality;

		// generate quality based on skill
		int luckRoll = System::random(100);

		luckRoll += System::random(player->getSkillMod("luck") + player->getSkillMod("force_luck"));

		int qualityRoll = luckRoll + rollMod;

		String galaxyName = "SWG Infinity";
		auto zoneServer = player->getZoneServer();
		if (zoneServer != nullptr) {
			galaxyName = zoneServer->getGalaxyName();
		}
		if (!galaxyName.contains("SWG Infinity"))
			qualityRoll = 100;

		if (qualityRoll > 60)
			newQuality += 0;
		else if (qualityRoll > 50)
			newQuality += 1;
		else if (qualityRoll > 40)
			newQuality += 2;
		else if (qualityRoll > 30)
			newQuality += 3;
		else if (qualityRoll > 20)
			newQuality += 4;
		else if (qualityRoll > 10)
			newQuality += 5;
		else
			newQuality += 6;
		if (newQuality > 7)
			newQuality = 7;

		DnaManager::instance()->generationalSample(deed, player, newQuality);
	}
};
#endif // SAMPLEDEEDTASK_H_
