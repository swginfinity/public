/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.

	Infinity: Death Watch Rooting Strike Command
	Melee attack that roots target in place (FROZEN state)

	Architecture: ALL combat logic in C++ for consistency
	- Deals light melee damage (25% harmonic mean)
	- Applies FROZEN state preventing movement
	- Duration: 5s base + dw_pot/20 - defender dw_recovery/20
	- Evasion: dw_evasion/2 (max 50%)

	Returns:
	- 1 = Root applied successfully
	- 0 = Evaded
	- -1 = Error
*/

#ifndef DWROOTINGSTRIKECOMMAND_H_
#define DWROOTINGSTRIKECOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/managers/death_watch/DeathWatchManager.h"

class DwRootingStrikeCommand : public QueueCommand {
public:

	// Root constants
	static const int BASE_COOLDOWN_MS = 12000;      // 12 second base cooldown
	static const int BASE_ROOT_DURATION_MS = 20000; // 20 second base slow duration (range: 8-32s)

	DwRootingStrikeCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {
		// Skip state/locomotion checks - this command works while mounted

		auto zoneServer = server->getZoneServer();
		if (zoneServer == nullptr) {
			return GENERALERROR;
		}

		DeathWatchManager* dwManager = zoneServer->getDeathWatchManager();
		if (dwManager == nullptr) {
			return GENERALERROR;
		}

		// Centralized validation (target, skill, jetpack, weapon, speed, range)
		uint64 targetId = 0;
		int validationResult = dwManager->validateJetpackAttack(creature, target, targetId, true); // true = ranged
		if (validationResult != 0) {
			return GENERALERROR;
		}

		// Skill-based cooldown (12s base, reduced by dw_speed) - after validation
		if (!dwManager->checkAndApplySkillCooldown(creature, "dwRootingStrike", BASE_COOLDOWN_MS)) {
			return GENERALERROR;
		}

		// Apply the rooting strike via DeathWatchManager
		int result = dwManager->applyRootingStrike(creature, targetId, BASE_ROOT_DURATION_MS);

		if (result < 0) {
			// Error - messages already sent by applyRootingStrike
			return GENERALERROR;
		}

		// Result 0 = evaded, 1 = rooted successfully
		// All valid outcomes, cooldown already applied
		return SUCCESS;
	}

};

#endif //DWROOTINGSTRIKECOMMAND_H_
