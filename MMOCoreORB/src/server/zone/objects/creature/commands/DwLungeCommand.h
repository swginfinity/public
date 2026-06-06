/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.

	Infinity: Death Watch Lunge Command
	High-risk/high-reward melee gap closer that dismounts target from their jetpack

	Architecture: ALL combat logic in C++ for consistency
	- Attempts to dismount target from their DW jetpack
	- Deals condition damage to the affected jetpack
	- Defender's dw_deflection can reflect the attack, dismounting ATTACKER instead

	Dismount Formula:
	- dismountChance = 70% + dw_pot/5 (max 90% at 100 pot)

	Deflection Formula (only on successful dismount):
	- deflectChance = dw_deflection/2 (max 50% at 100 deflection)

	Outcomes:
	- 1 = Success: Target dismounted, target's jetpack damaged
	- 2 = Deflected: Attacker dismounted, attacker's jetpack damaged
	- 0 = Miss: No dismount, no damage
	- -1 = Error

	Risk/Reward Examples:
	- 100 pot vs 0 deflection: 90% success, 0% backfire
	- 0 pot vs 100 deflection: 70% hit, but 50% of hits backfire
*/

#ifndef DWLUNGECOMMAND_H_
#define DWLUNGECOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/managers/director/DirectorManager.h"
#include "server/zone/managers/death_watch/DeathWatchManager.h"

class DwLungeCommand : public QueueCommand {
public:

	// Lunge constants
	static const int BASE_COOLDOWN_MS = 18000;      // 18 second base cooldown
	static const int BASE_JETPACK_DAMAGE = 100;     // Base condition damage to jetpack

	DwLungeCommand(const String& name, ZoneProcessServer* server)
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

		// Skill-based cooldown (18s base, reduced by dw_speed) - after validation
		if (!dwManager->checkAndApplySkillCooldown(creature, "dwLunge", BASE_COOLDOWN_MS)) {
			return GENERALERROR;
		}

		// Apply the lunge via DeathWatchManager
		// This handles target validation, dismount mechanics, and deflection
		int result = dwManager->applyLunge(creature, targetId, BASE_JETPACK_DAMAGE);

		if (result < 0) {
			// Error - messages already sent by applyLunge
			return GENERALERROR;
		}

		// Result 0 = miss, 1 = target dismounted, 2 = deflected (attacker dismounted)
		// All valid outcomes, cooldown already applied
		return SUCCESS;
	}

};

#endif //DWLUNGECOMMAND_H_
