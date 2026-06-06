/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.

	Infinity: Death Watch Crippling Blow Command
	Melee attack that applies movement speed reduction

	Architecture: ALL combat logic in C++ for consistency
	- Deals moderate melee damage (40% harmonic mean)
	- Applies 50% speed reduction via buff
	- Duration: 8s base + dw_pot/25 - defender dw_recovery/25
	- Evasion: dw_evasion/3 (max 33%)

	Returns:
	- 1 = Slow applied successfully
	- 0 = Evaded
	- -1 = Error
*/

#ifndef DWCRIPPLINGBLOWCOMMAND_H_
#define DWCRIPPLINGBLOWCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/managers/death_watch/DeathWatchManager.h"

class DwCripplingBlowCommand : public QueueCommand {
public:

	// Crippling blow constants
	static const int BASE_COOLDOWN_MS = 15000;      // 15 second base cooldown
	static const int BASE_SLOW_DURATION_MS = 20000; // 20 second base slow duration (range: 8-32s)
	static constexpr float SPEED_MULTIPLIER = 0.5f; // 50% speed (50% reduction)

	DwCripplingBlowCommand(const String& name, ZoneProcessServer* server)
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
		int validationResult = dwManager->validateJetpackAttack(creature, target, targetId, false); // false = melee
		if (validationResult != 0) {
			return GENERALERROR;
		}

		// Skill-based cooldown (15s base, reduced by dw_speed) - after validation
		if (!dwManager->checkAndApplySkillCooldown(creature, "dwCripplingBlow", BASE_COOLDOWN_MS)) {
			return GENERALERROR;
		}

		// Apply the crippling blow via DeathWatchManager
		int result = dwManager->applyCripplingBlow(creature, targetId, BASE_SLOW_DURATION_MS, SPEED_MULTIPLIER);

		if (result < 0) {
			// Error - messages already sent by applyCripplingBlow
			return GENERALERROR;
		}

		// Result 0 = evaded, 1 = slowed successfully
		// All valid outcomes, cooldown already applied
		return SUCCESS;
	}

};

#endif //DWCRIPPLINGBLOWCOMMAND_H_
