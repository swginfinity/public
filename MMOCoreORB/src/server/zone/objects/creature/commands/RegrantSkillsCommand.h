/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef REGRANTSKILLSCOMMAND_H_
#define REGRANTSKILLSCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/managers/skill/SkillManager.h"

class RegrantSkillsCommand : public QueueCommand {
public:

	RegrantSkillsCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {
	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {
		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;


		// This command only works on the player executing the command

		SkillManager* skillManager = SkillManager::instance();
		if (skillManager == nullptr)
			return GENERALERROR;
			
		const SkillList* skillList = creature->getSkillList();
		if (skillList == nullptr)
			return GENERALERROR;

		auto zoneServer = creature->getZoneServer();
		if (zoneServer == nullptr)
			return GENERALERROR;

		bool liveServer = (zoneServer->getGalaxyName() == "SWG Infinity") ? true : false;

		if (!creature->checkCooldownRecovery("regrantSkills")) {
			if (liveServer)
				creature->sendSystemMessage("You may only use the regrantSkills command once every 12 hours.");
			else
				creature->sendSystemMessage("You may only use the regrantSkills command once per minute.");
			return GENERALERROR;
        }

		if (liveServer)
			creature->updateCooldownTimer("regrantSkills", 1000 * 60 * 60 * 12); //12 hours on live
		else
			creature->updateCooldownTimer("regrantSkills", 1000 * 60); //One minute on test servers

		ManagedReference<PlayerObject*> ghost = creature->getPlayerObject();
		if (ghost != nullptr) {

			int preFrsXP = ghost->getExperience("force_rank_xp");
			int force = ghost->getForcePower();

			//Have to do this because the skillslist is changing as we drop/add skills
			Vector<String> listOfNames;
			skillList->getStringList(listOfNames);
			std::sort(listOfNames.begin(), listOfNames.end(), std::less<std::string>());
			SkillList skillListCopy;
			skillListCopy.loadFromNames(listOfNames);

			for (int i = 0; i < skillListCopy.size(); ++i) {
				Skill* skill = skillListCopy.get(i);
				if (skill == nullptr)
					continue;
				String skillName = skill->getSkillName();

				if (!(skillName.beginsWith("admin") || skillName.beginsWith("infinity_") || skillName.beginsWith("social_language") || skillName.beginsWith("species"))) {				
					creature->sendSystemMessage("Regranting skill: " + skillName);
					skillManager->surrenderSkill(skillName, creature, true, false, true);    
					bool skillGranted = skillManager->awardSkill(skillName, creature, true, false, true, true);   //skillName, creature, notifyClient, awardRequiredSkills, noXpRequired, regrant
				}			
			}

			//Update maximum experience.
			skillManager->updateXpLimits(ghost);

			// Update Force Power Max.
			ghost->recalculateForcePower();
			ghost->setForcePower(force);

			int postFrsXP = ghost->getExperience("force_rank_xp");
			if (preFrsXP > postFrsXP) {
				auto playerManager = zoneServer->getPlayerManager();
				if (playerManager == nullptr)
					return GENERALERROR;
				playerManager->awardExperience(creature, "force_rank_xp", (preFrsXP - postFrsXP), false);
			}

			return SUCCESS;
		}
		else {
			creature->sendSystemMessage("The player object could not be found so skill were not regranted.");
			return GENERALERROR;
		}
	}

};

#endif //REGRANTSKILLSCOMMAND_H_
