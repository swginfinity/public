/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.

	Infinity: Death Watch Jetpack Buster Command
	Devastating melee attack targeting enemy jetpacks

	Architecture: ALL combat logic in C++ for consistency
	- Deals massive condition damage to target's jetpack
	- Damage: weapon harmonic mean * 2 (double weapon damage to jetpack)
	- dw_pot bonus: up to +50% more damage
	- Backfire: dw_deflection/2 chance (max 50%) to damage attacker's jetpack instead
	- Can destroy jetpacks outright
*/

#ifndef DWJETPACKBUSTERCOMMAND_H_
#define DWJETPACKBUSTERCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/managers/director/DirectorManager.h"
#include "server/zone/managers/death_watch/DeathWatchManager.h"

class DwJetpackBusterCommand : public QueueCommand {
public:

	static const int BASE_COOLDOWN_MS = 20000;  // 20 second base cooldown (devastating ability)

	DwJetpackBusterCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {
		// Skip state/locomotion checks - this command works while mounted

		if (!creature->isPlayerCreature()) {
			return GENERALERROR;
		}

		// Skill check - C++
		if (!creature->hasSkill("combat_death_watch_novice")) {
			creature->sendSystemMessage("You must be Death Watch to use this command.");
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

		// Jetpack check - C++
		if (!dwManager->isOnDeathWatchJetpack(creature)) {
			creature->sendSystemMessage("This command can only be used while mounted on a Death Watch jetpack.");
			return GENERALERROR;
		}

		// Skill-based cooldown (20s base, reduced by dw_speed)
		if (!dwManager->checkAndApplySkillCooldown(creature, "dwJetpackBuster", BASE_COOLDOWN_MS)) {
			return GENERALERROR;
		}

		// Execute jetpack buster - ALL logic in C++
		int result = dwManager->applyJetpackBuster(creature, target);

		if (result < 0) {
			// Attack failed - error message already sent by applyJetpackBuster
			return GENERALERROR;
		}

		// Result of 0 means backfire (not an error, but no damage to target)
		// Positive result means damage dealt

		return SUCCESS;
	}

};

#endif //DWJETPACKBUSTERCOMMAND_H_
