/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.

	Infinity: Death Watch Door Buster Command
	Building breach ability - forcibly ejects enemy DW players from buildings

	Architecture: ALL combat logic in C++ for consistency
	- Targets a building within 32m
	- Ejects enemy DW players (at war, BH target, or Greef contract)
	- Max ejects based on Zann skill progression (1/2/3/4/all)
	- Priority: BH targets > Greef contracts > At-war faction (random)
	- Ejected players get invincibility (1-10s based on dw_deflection)
	- Applies dwTef and pvpTef to prevent re-entry
	- Sets first ejected player as attacker's target
	- Cannot be used while on jetpack (ground assault only)
	- 1 hour base cooldown, reduced by dw_speed+dw_recovery (min 30 min)
*/

#ifndef DWDOORBUSTERCOMMAND_H_
#define DWDOORBUSTERCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/managers/director/DirectorManager.h"
#include "server/zone/managers/death_watch/DeathWatchManager.h"

class DwDoorBusterCommand : public QueueCommand {
public:

	static const int BASE_COOLDOWN_MS = 3600000;  // 1 hour base cooldown
	static const int MIN_COOLDOWN_MS = 1800000;   // 30 minute minimum cooldown

	DwDoorBusterCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!creature->isPlayerCreature()) {
			return GENERALERROR;
		}

		// Skill check - requires Zann Box 1 minimum
		if (!creature->hasSkill("combat_death_watch_zan_1")) {
			creature->sendSystemMessage("You must have Zann Consortium training to use this ability.");
			return GENERALERROR;
		}

		auto zoneServer = server->getZoneServer();
		if (zoneServer == nullptr) {
			return GENERALERROR;
		}

		DeathWatchManager* dwManager = zoneServer->getDeathWatchManager();
		if (dwManager == nullptr) {
			return GENERALERROR;
		}

		// Cannot be on jetpack - this is a ground assault ability
		if (dwManager->isOnDeathWatchJetpack(creature)) {
			creature->sendSystemMessage("You cannot use this ability while mounted on a jetpack. Dismount first.");
			return GENERALERROR;
		}

		// Calculate cooldown reduction based on dw_speed + dw_recovery (cap 200 total)
		int dwSpeed = creature->getSkillMod("dw_speed");
		int dwRecovery = creature->getSkillMod("dw_recovery");
		int combinedSkill = Math::min(200, dwSpeed + dwRecovery);

		// Reduce cooldown: at 200 combined skill, reduce to 50% (30 min)
		// Formula: baseCooldown * (1 - combinedSkill/400)
		int adjustedCooldown = BASE_COOLDOWN_MS - (BASE_COOLDOWN_MS * combinedSkill / 400);
		adjustedCooldown = Math::max((int)MIN_COOLDOWN_MS, adjustedCooldown);

		// Cooldown check with adjusted duration
		if (!dwManager->checkAndApplyCooldownWithDuration(creature, "dwDoorBuster", adjustedCooldown)) {
			return GENERALERROR;
		}

		// Execute door buster - ALL logic in C++
		int result = dwManager->applyDoorBuster(creature, target);

		if (result < 0) {
			// Attack failed - error message already sent by applyDoorBuster
			return GENERALERROR;
		}

		// Result is number of players ejected (0 = no valid targets found)
		return SUCCESS;
	}

};

#endif //DWDOORBUSTERCOMMAND_H_
