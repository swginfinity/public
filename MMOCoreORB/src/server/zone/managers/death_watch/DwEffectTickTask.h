/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.

	Infinity: Death Watch Effect Tick Task

	Generic DoT/tick-based effect handler for Death Watch combat.
	Supports action drain, poison, bleeds, and other periodic effects.

	Effect Types:
	- DW_EFFECT_ACTION_DRAIN: Periodic action damage
	- DW_EFFECT_POISON: Periodic health damage
	- DW_EFFECT_BURN: Fire-like periodic damage

	Anti-Stacking:
	- Effects are tracked per-attacker, per-type
	- If target already has this effect type from ANY attacker:
	  - 98% chance to miss
	  - 2% chance to stack (add to existing effect)
*/

#ifndef DWEFFECTTICKTASK_H_
#define DWEFFECTTICKTASK_H_

#include "engine/engine.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/objects/player/PlayerObject.h"
#include "server/zone/ZoneServer.h"

namespace server {
namespace zone {
namespace managers {
namespace death_watch {

// Effect types for DW combat
enum DwEffectType : uint8 {
	DW_EFFECT_NONE = 0,
	DW_EFFECT_ACTION_DRAIN = 1,    // Action pool drain over time
	DW_EFFECT_POISON = 2,          // Health poison DoT
	DW_EFFECT_BURN = 3,            // Fire damage DoT
	DW_EFFECT_SLOW = 4,            // Movement slow (future)
	DW_EFFECT_MIND_DRAIN = 5       // Mind pool drain (future)
};

class DwEffectTickTask : public Task {
protected:
	// Weak references to prevent dangling pointers
	ManagedWeakReference<CreatureObject*> targetRef;
	ManagedWeakReference<CreatureObject*> attackerRef;

	// Effect parameters
	DwEffectType effectType;
	int damagePerTick;
	int ticksRemaining;
	int tickIntervalMs;
	uint8 damagePool;  // 0=health, 3=action, 6=mind

	// Unique effect ID for tracking
	uint64 effectID;

public:
	DwEffectTickTask(CreatureObject* target, CreatureObject* attacker,
					 DwEffectType type, int damage, int ticks, int intervalMs, uint8 pool)
		: Task() {

		targetRef = target;
		attackerRef = attacker;
		effectType = type;
		damagePerTick = damage;
		ticksRemaining = ticks;
		tickIntervalMs = intervalMs;
		damagePool = pool;

		// Generate unique effect ID from attacker + type + timestamp
		effectID = (attacker != nullptr ? attacker->getObjectID() : 0) ^
				   ((uint64)type << 56) ^
				   Time().getMiliTime();
	}

	void run() {
		ManagedReference<CreatureObject*> target = targetRef.get();
		ManagedReference<CreatureObject*> attacker = attackerRef.get();

		if (target == nullptr) {
			return;  // Target gone, effect ends
		}

		// Check if target is still valid for damage
		if (target->isDead()) {
			onEffectEnd(target, false);
			return;
		}

		// Decrement ticks
		ticksRemaining--;

		// Apply the tick damage
		Locker locker(target);

		if (attacker != nullptr) {
			Locker clocker(attacker, target);
		}

		int actualDamage = applyTickDamage(target, attacker);

		// Send tick notification
		if (actualDamage > 0) {
			sendTickMessage(target, attacker, actualDamage);
		}

		// Schedule next tick or end effect
		if (ticksRemaining > 0 && !target->isDead()) {
			// Reschedule for next tick
			this->reschedule(tickIntervalMs);
		} else {
			// Effect complete
			onEffectEnd(target, true);
		}
	}

protected:
	int applyTickDamage(CreatureObject* target, CreatureObject* attacker) {
		if (target == nullptr) {
			return 0;
		}

		int damage = damagePerTick;

		// Don't overkill - cap at current pool - 1
		int currentPool = target->getHAM(damagePool);
		if (damage >= currentPool) {
			damage = currentPool - 1;
		}

		if (damage <= 0) {
			return 0;
		}

		// Apply the damage
		target->inflictDamage(attacker, damagePool, damage, false, "dwDotDMG", true, false);

		return damage;
	}

	void sendTickMessage(CreatureObject* target, CreatureObject* attacker, int damage) {
		if (target == nullptr) {
			return;
		}

		StringBuffer targetMsg;
		String effectName = getEffectName();

		switch (effectType) {
			case DW_EFFECT_ACTION_DRAIN:
				targetMsg << "\\#FF6600" << effectName << " drains " << damage << " action!";
				break;
			case DW_EFFECT_POISON:
				targetMsg << "\\#00FF00" << effectName << " deals " << damage << " poison damage!";
				break;
			case DW_EFFECT_BURN:
				targetMsg << "\\#FF4400" << effectName << " burns you for " << damage << " damage!";
				break;
			default:
				targetMsg << "\\#FFFF00" << effectName << " deals " << damage << " damage!";
				break;
		}

		target->sendSystemMessage(targetMsg.toString());

		// Also notify attacker if they're online
		if (attacker != nullptr && attacker->isPlayerCreature() &&
			attacker->getObjectID() != target->getObjectID()) {
			StringBuffer attackerMsg;
			attackerMsg << "\\#00FF00Your " << effectName << " deals " << damage << " damage to "
						<< target->getFirstName() << "!";
			attacker->sendSystemMessage(attackerMsg.toString());
		}
	}

	String getEffectName() const {
		switch (effectType) {
			case DW_EFFECT_ACTION_DRAIN:
				return "Action Drain";
			case DW_EFFECT_POISON:
				return "Death Watch Poison";
			case DW_EFFECT_BURN:
				return "Death Watch Burn";
			case DW_EFFECT_SLOW:
				return "Death Watch Slow";
			case DW_EFFECT_MIND_DRAIN:
				return "Mind Drain";
			default:
				return "Death Watch Effect";
		}
	}

	void onEffectEnd(CreatureObject* target, bool completed) {
		if (target == nullptr) {
			return;
		}

		// Send completion message
		String effectName = getEffectName();
		StringBuffer msg;

		if (completed) {
			msg << "\\#AAAAAA" << effectName << " has worn off.";
		} else {
			msg << "\\#AAAAAA" << effectName << " was interrupted.";
		}

		target->sendSystemMessage(msg.toString());

		// Clear effect from tracking (done by DeathWatchManager)
		// This task will be garbage collected after run() completes
	}

public:
	// Getters for effect tracking
	DwEffectType getEffectType() const { return effectType; }
	uint64 getEffectID() const { return effectID; }
	int getTicksRemaining() const { return ticksRemaining; }

	uint64 getTargetID() {
		ManagedReference<CreatureObject*> target = targetRef.get();
		return target != nullptr ? target->getObjectID() : 0;
	}

	uint64 getAttackerID() {
		ManagedReference<CreatureObject*> attacker = attackerRef.get();
		return attacker != nullptr ? attacker->getObjectID() : 0;
	}

	// Allow stacking by adding ticks and averaging damage
	void stackEffect(int additionalDamage, int additionalTicks) {
		// Average the damage
		int totalDamage = (damagePerTick * ticksRemaining) + (additionalDamage * additionalTicks);
		ticksRemaining += additionalTicks;
		damagePerTick = totalDamage / ticksRemaining;
	}

	// Force end the effect early
	void endEffect() {
		ticksRemaining = 0;
	}
};

} // death_watch
} // managers
} // zone
} // server

using namespace server::zone::managers::death_watch;

#endif /* DWEFFECTTICKTASK_H_ */
