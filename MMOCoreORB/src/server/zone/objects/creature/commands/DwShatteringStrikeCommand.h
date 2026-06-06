/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.

	Infinity: Death Watch Shattering Strike Command
	Armor-penetrating melee attack with mitigation debuff

	Architecture: ALL combat logic in C++ for consistency
	- Deals 20% harmonic mean damage as initial hit
	- Applies private_mitigation_divisor_x100 debuff (target takes more damage)
	- Mitigation: 125 base (1.25x damage) + dw_pot/4 (up to 150 = 1.5x)
	- Duration from weapon: harmonicDamage/50 seconds base
	- Duration scales: +50ms per dw_pot, -50ms per target's dw_recovery
	- Duration clamped 5-20 seconds
	- Resistance: target's dw_deflection/4 (max 25% chance to resist)
*/

#ifndef DWSHATTERINGSTRIKECOMMAND_H_
#define DWSHATTERINGSTRIKECOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/managers/director/DirectorManager.h"
#include "server/zone/managers/death_watch/DeathWatchManager.h"

class DwShatteringStrikeCommand : public QueueCommand {
public:

	static const int BASE_COOLDOWN_MS = 12000;  // 12 second base cooldown (powerful debuff)

	DwShatteringStrikeCommand(const String& name, ZoneProcessServer* server)
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
		// Note: ShatteringStrike is melee but has no weapon type check
		uint64 targetId = 0;
		int validationResult = dwManager->validateJetpackAttack(creature, target, targetId, false); // false = melee
		if (validationResult != 0) {
			return GENERALERROR;
		}

		// Skill-based cooldown (12s base, reduced by dw_speed) - after validation
		if (!dwManager->checkAndApplySkillCooldown(creature, "dwShatteringStrike", BASE_COOLDOWN_MS)) {
			return GENERALERROR;
		}

		// Execute shattering strike - ALL logic in C++
		int result = dwManager->applyShatteringStrike(creature, targetId);

		if (result <= 0) {
			// Attack failed - error message already sent by applyShatteringStrike
			return GENERALERROR;
		}

		return SUCCESS;
	}

};

#endif //DWSHATTERINGSTRIKECOMMAND_H_
