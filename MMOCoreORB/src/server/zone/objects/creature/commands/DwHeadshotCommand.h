/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.

	Infinity: Death Watch Headshot Command
	Precision shot that deals 50% damage to MIND pool only
	Mind damage is brutal because DW players cannot heal this pool

	Architecture: ALL combat logic in C++ for consistency
	- Uses weapon harmonic mean damage: 2*min*max/(min+max)
	- Damage multiplier: 0.5 (50% of normal)
	- Damage type: Mind only (pool 6)
	- Accuracy penalty: -20 (harder to hit, offset by dw_acc)
	- Base cooldown: 5 seconds (reduced by dw_speed)
	- Lua stub exists only for future extensibility (effects, etc.)
*/

#ifndef DWHEADSHOTCOMMAND_H_
#define DWHEADSHOTCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/managers/director/DirectorManager.h"
#include "server/zone/managers/death_watch/DeathWatchManager.h"

class DwHeadshotCommand : public QueueCommand {
public:

	// Headshot constants
	static const int BASE_COOLDOWN_MS = 5000;      // 5 second base cooldown
	static constexpr float DAMAGE_MULTIPLIER = 0.5f;  // 50% damage (mind is brutal)
	static const int ACCURACY_PENALTY = -20;       // Harder to hit
	static const int DAMAGE_TYPE_MIND = 3;         // Mind only (attribute index 6, but we use type 3 for mind in our system)

	DwHeadshotCommand(const String& name, ZoneProcessServer* server)
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
		uint64 targetId = 0;
		int validationResult = dwManager->validateJetpackAttack(creature, target, targetId, true); // true = ranged
		if (validationResult != 0) {
			return GENERALERROR;
		}

		// Skill-based cooldown (5s base, reduced by dw_speed) - after validation
		if (!dwManager->checkAndApplySkillCooldown(creature, "dwHeadshot", BASE_COOLDOWN_MS)) {
			return GENERALERROR;
		}

		// Execute attack - ALL logic in C++
		// Parameters: damageMultiplier=0.5 (50%), accuracyMod=-20 (harder), damageType=3 (mind)
		int damage = dwManager->executeDwRangedAttack(creature, targetId, DAMAGE_MULTIPLIER, ACCURACY_PENALTY, DAMAGE_TYPE_MIND);

		if (damage < 0) {
			// Attack failed - error message already sent by executeDwRangedAttack
			return GENERALERROR;
		}

		// Send combat feedback
		if (damage > 0) {
			ManagedReference<SceneObject*> targetObj = zoneServer->getObject(targetId);
			String targetName = "target";
			CreatureObject* targetCreature = nullptr;
			if (targetObj != nullptr && targetObj->isCreatureObject()) {
				targetCreature = targetObj->asCreatureObject();
				// If target is a vehicle/mount, get the rider's name instead
				if (targetCreature->isVehicleObject() || targetCreature->isMount()) {
					ManagedReference<CreatureObject*> rider = targetCreature->getLinkedCreature();
					if (rider != nullptr) {
						targetName = rider->getFirstName();
						targetCreature = rider;
					}
				} else {
					targetName = targetCreature->getFirstName();
				}
			}

			StringBuffer msg;
			msg << "\\#FF0000HEADSHOT! You hit " << targetName << "'s mind for " << damage << " damage!";
			creature->sendSystemMessage(msg.toString());

			// Notify target
			if (targetCreature != nullptr) {
				StringBuffer targetMsg;
				targetMsg << "\\#FF0000" << creature->getFirstName() << " lands a headshot on you for " << damage << " mind damage!";
				targetCreature->sendSystemMessage(targetMsg.toString());
			}
		} else if (damage == 0) {
			// Miss was already reported by executeDwRangedAttack
		}

		// Optional: Call Lua stub for future effects/hooks
		// Uncomment if you want Lua extensibility:
		// Lua* lua = DirectorManager::instance()->getLuaInstance();
		// Reference<LuaFunction*> luaFunc = lua->createFunction("DwJetpackCombat", "onHeadshotHit", 0);
		// *luaFunc << creature;
		// *luaFunc << target;
		// *luaFunc << damage;
		// luaFunc->callFunction();

		return SUCCESS;
	}

};

#endif //DWHEADSHOTCOMMAND_H_
