/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.

	Infinity: Death Watch Disarming Shot Command
	Prevents target from attacking by stunning them and optionally unequipping their weapon

	Architecture: ALL combat logic in C++ for consistency
	- Two effects:
	  1. Attempts to unequip weapon from hold_r to inventory (if possible)
	  2. Applies STUNNED state via buff to prevent attacks for duration
	- Defender can resist via dw_deflection (% chance = deflection/2, max 50%)
	- Duration scales with attacker dw_pot, reduced by defender dw_recovery

	Resistance Formula:
	- resistChance = min(50, dw_deflection / 2)

	Duration Formula:
	- duration = baseDuration * (1 + dw_pot/100) * (1 - dw_recovery/200)
	- Minimum: 1 second, Maximum: 10 seconds

	Return Values:
	- 1 = Full success (weapon unequipped + stunned)
	- 2 = Partial success (stunned only)
	- 0 = Resisted
	- -1 = Error
*/

#ifndef DWDISARMINGSHOTCOMMAND_H_
#define DWDISARMINGSHOTCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/managers/director/DirectorManager.h"
#include "server/zone/managers/death_watch/DeathWatchManager.h"

class DwDisarmingShotCommand : public QueueCommand {
public:

	// Disarming shot constants
	static const int BASE_COOLDOWN_MS = 20000;      // 20 second base cooldown (powerful ability)
	static const int BASE_DURATION_MS = 4000;       // 4 second base stun duration
	static const int ACCURACY_PENALTY = -20;        // Harder to hit (precision shot)

	DwDisarmingShotCommand(const String& name, ZoneProcessServer* server)
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

		// Skill-based cooldown (20s base, reduced by dw_speed) - after validation
		if (!dwManager->checkAndApplySkillCooldown(creature, "dwDisarmingShot", BASE_COOLDOWN_MS)) {
			return GENERALERROR;
		}

		// Apply the disarming shot via DeathWatchManager
		// This handles target validation, resistance check, unequip attempt, and stun
		int result = dwManager->applyDisarmingShot(creature, targetId, BASE_DURATION_MS);

		if (result < 0) {
			// Error - messages already sent by applyDisarmingShot
			return GENERALERROR;
		}

		// Result 0 = resisted, 1 = full success, 2 = partial success
		// All valid outcomes, cooldown already applied
		return SUCCESS;
	}

};

#endif //DWDISARMINGSHOTCOMMAND_H_
