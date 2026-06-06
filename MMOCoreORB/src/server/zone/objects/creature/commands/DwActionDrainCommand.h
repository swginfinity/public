/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.

	Infinity: Death Watch Action Drain Command
	Applies an action bleed effect that ticks over time

	Architecture: ALL combat logic in C++ for consistency
	- Calculates total drain from weapon harmonic mean + dw_pot
	- Duration scales with dw_pot (6-12 seconds)
	- Tick interval scales with dw_speed (1-2 seconds per tick)
	- Anti-stacking: 98% miss if target already affected, 2% stack chance
	- Defender's dw_recovery reduces number of ticks (counterplay)

	Attacker Formula:
	- Total Drain = harmonicMean * damageMultiplier * (1 + dw_pot/100)
	- Duration = min(12, 6 + dw_pot/10) seconds
	- Tick Interval = max(1, 2 * (1 - dw_speed/200)) seconds

	Defender Formula:
	- Final Ticks = max(1, numTicks * (1 - dw_recovery/200))
	- At 0 recovery: 100% ticks, at 100 recovery: 50% ticks
*/

#ifndef DWACTIONDRAINCOMMAND_H_
#define DWACTIONDRAINCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/managers/director/DirectorManager.h"
#include "server/zone/managers/death_watch/DeathWatchManager.h"

class DwActionDrainCommand : public QueueCommand {
public:

	// Action drain constants
	static const int BASE_COOLDOWN_MS = 8000;       // 8 second base cooldown
	static constexpr float DAMAGE_MULTIPLIER = 0.4f;   // 40% of base damage as total drain
	static const int ACCURACY_PENALTY = -10;        // Slightly harder to hit
	static const int BASE_DURATION_SEC = 6;         // Base 6 second duration
	static const int MAX_DURATION_SEC = 12;         // Max 12 second duration
	static constexpr float BASE_TICK_INTERVAL = 2.0f;  // Base 2 second tick interval
	static constexpr float MIN_TICK_INTERVAL = 1.0f;   // Minimum 1 second tick

	DwActionDrainCommand(const String& name, ZoneProcessServer* server)
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

		// Centralized validation (skill, jetpack, speed, target, range)
		// ActionDrain is a melee ability
		uint64 targetId = 0;
		int validationResult = dwManager->validateJetpackAttack(creature, target, targetId, false); // false = melee
		if (validationResult != 0) {
			return GENERALERROR;
		}

		// Skill-based cooldown (8s base, reduced by dw_speed) - after validation
		if (!dwManager->checkAndApplySkillCooldown(creature, "dwActionDrain", BASE_COOLDOWN_MS)) {
			return GENERALERROR;
		}

		// Calculate drain parameters based on skills
		// Get skill mods
		int dwPot = creature->getSkillMod("dw_pot");
		int dwSpeed = creature->getSkillMod("dw_speed");

		// Get base weapon damage using harmonic mean
		int baseDamage = dwManager->getWeaponBaseDamage(creature);

		// Calculate total drain damage
		// Formula: totalDrain = harmonicMean * multiplier * (1 + dw_pot/100)
		float potencyBonus = 1.0f + (dwPot / 100.0f);
		int totalDrain = (int)(baseDamage * DAMAGE_MULTIPLIER * potencyBonus);
		if (totalDrain < 10) {
			totalDrain = 10;  // Minimum drain
		}

		// Calculate duration (6-12 seconds based on dw_pot)
		// Formula: duration = min(12, 6 + dw_pot/10)
		int durationSec = BASE_DURATION_SEC + (dwPot / 10);
		if (durationSec > MAX_DURATION_SEC) {
			durationSec = MAX_DURATION_SEC;
		}

		// Calculate tick interval (faster with dw_speed)
		// Formula: tickInterval = max(1, 2 * (1 - dw_speed/200))
		float speedReduction = 1.0f - (dwSpeed / 200.0f);
		if (speedReduction < 0.5f) speedReduction = 0.5f;  // Cap at 50% reduction

		float tickInterval = BASE_TICK_INTERVAL * speedReduction;
		if (tickInterval < MIN_TICK_INTERVAL) {
			tickInterval = MIN_TICK_INTERVAL;
		}

		// Apply the action drain effect via DeathWatchManager
		// This handles all target validation, anti-stacking, and effect creation
		int result = dwManager->applyActionDrain(creature, targetId, totalDrain, durationSec, tickInterval);

		if (result == 0) {
			// Failed to apply - messages already sent by applyActionDrain
			return GENERALERROR;
		}

		// Success feedback
		StringBuffer msg;
		msg << "\\#FF6600Action Drain applied! Total: " << totalDrain
			<< " over " << durationSec << "s (tick every "
			<< String::format("%.1f", tickInterval) << "s)";
		creature->sendSystemMessage(msg.toString());

		return SUCCESS;
	}

};

#endif //DWACTIONDRAINCOMMAND_H_
