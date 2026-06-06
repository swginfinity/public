/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.

	Infinity: Death Watch Wounding Shot Command
	Applies wounds based on attacker's DW faction targeting different pools

	Architecture: ALL combat logic in C++ for consistency
	- Faction-based wound targeting:
	  - XON (Xonti Brothers): Action pools (3,4,5) - mobility/stamina focus
	  - SUN (Black Sun): Health pools (0,1,2) - brutal efficiency
	  - KRR (Krrsantan): Health pools (0,1,2) - raw power
	  - ZAN (Zann Consortium): Mind pools (6,7,8) - psychological warfare
	- Wound distribution: 50% primary, 25% each secondary
	- Light direct damage: 15% of harmonic mean

	Attacker Formula:
	- Wounds = baseWounds * (1 + dw_pot/100)

	Defender Formula:
	- Final Wounds = wounds * (1 - dw_recovery/200)
	- At 0 recovery: 100% wounds, at 100 recovery: 50% wounds
*/

#ifndef DWWOUNDINGSHOTCOMMAND_H_
#define DWWOUNDINGSHOTCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/managers/director/DirectorManager.h"
#include "server/zone/managers/death_watch/DeathWatchManager.h"

class DwWoundingShotCommand : public QueueCommand {
public:

	// Wounding shot constants
	static const int BASE_COOLDOWN_MS = 12000;      // 12 second base cooldown
	static const int BASE_WOUNDS = 50;              // Base wound amount
	static const int ACCURACY_PENALTY = -15;        // Harder to hit (precision shot)

	DwWoundingShotCommand(const String& name, ZoneProcessServer* server)
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
		if (!dwManager->checkAndApplySkillCooldown(creature, "dwWoundingShot", BASE_COOLDOWN_MS)) {
			return GENERALERROR;
		}

		// Apply the wounding shot via DeathWatchManager
		// This handles all target validation, faction-based wound targeting, and damage
		int result = dwManager->applyWoundingShot(creature, targetId, BASE_WOUNDS);

		if (result < 0) {
			// Failed to apply - messages already sent by applyWoundingShot
			return GENERALERROR;
		}

		return SUCCESS;
	}

};

#endif //DWWOUNDINGSHOTCOMMAND_H_
