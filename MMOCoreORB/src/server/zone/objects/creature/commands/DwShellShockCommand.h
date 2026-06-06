/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.

	Infinity: Death Watch Shell Shock Command
	Applies battle fatigue (shock wounds) to impair target's healing

	Architecture: ALL combat logic in C++ for consistency
	- Conservative BF values to prevent oppressive stacking
	- Slower dw_pot scaling (1 + dw_pot/150) than damage abilities
	- Defender dw_recovery reduces BF by up to 50%
	- Color-coded severity messages at thresholds

	Battle Fatigue Thresholds:
	- 0-250: Minor impact (gray)
	- 250-400: Moderate - healing starts to degrade (yellow)
	- 400-750: Significant - healing seriously impaired (orange)
	- 750-1000: Severe - healing nearly useless (red)

	Attacker Formula:
	- BF = baseBF * (1 + dw_pot/150)

	Defender Formula:
	- finalBF = BF * (1 - dw_recovery/200)

	NOTE: Consider adding BF clearing to dwAdrenalineStim as counterplay
	Stim could clear 150-250 BF per use, creating consumable economy
*/

#ifndef DWSHELLSHOCKCOMMAND_H_
#define DWSHELLSHOCKCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/managers/director/DirectorManager.h"
#include "server/zone/managers/death_watch/DeathWatchManager.h"

class DwShellShockCommand : public QueueCommand {
public:

	// Shell shock constants
	static const int BASE_COOLDOWN_MS = 15000;      // 15 second base cooldown (longer than damage abilities)
	static const int BASE_BF = 50;                  // Conservative: 5-8 shots to reach 400 threshold
	static const int ACCURACY_PENALTY = -10;        // Slightly harder to hit

	DwShellShockCommand(const String& name, ZoneProcessServer* server)
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

		// Skill-based cooldown (15s base, reduced by dw_speed) - after validation
		if (!dwManager->checkAndApplySkillCooldown(creature, "dwShellShock", BASE_COOLDOWN_MS)) {
			return GENERALERROR;
		}

		// Apply the shell shock via DeathWatchManager
		// This handles all target validation, BF calculation, and messaging
		int result = dwManager->applyShellShock(creature, targetId, BASE_BF);

		if (result < 0) {
			// Failed to apply - messages already sent by applyShellShock
			return GENERALERROR;
		}

		return SUCCESS;
	}

};

#endif //DWSHELLSHOCKCOMMAND_H_
