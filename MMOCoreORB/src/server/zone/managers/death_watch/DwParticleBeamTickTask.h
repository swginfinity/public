/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.

	Infinity: Death Watch Particle Beam "Mand'alor's Grip" Tick Task

	Scheduled task to apply defense reduction ticks from Particle Beam hits.
	Flow: Ramp UP (4 ticks) -> PEAK hold -> Ramp DOWN (4 ticks) -> CLEAR -> Immunity

	Each tick applies/removes private_mitigation_divisor buff.
	After full cycle completes, defender gains immunity for configurable duration.
*/

#ifndef DWPARTICLEBEAMTICKTASK_H_
#define DWPARTICLEBEAMTICKTASK_H_

#include "engine/engine.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/ZoneServer.h"
#include "server/zone/managers/death_watch/DeathWatchManager.h"
#include "server/zone/managers/skill/SkillModManager.h"

namespace server {
namespace zone {
namespace managers {
namespace death_watch {

// Phases of the Mand'alor's Grip effect
enum ParticleBeamPhase : uint8 {
	PHASE_RAMP_UP = 1,
	PHASE_PEAK = 2,
	PHASE_RAMP_DOWN = 3,
	PHASE_COMPLETE = 4
};

class DwParticleBeamTickTask : public Task {
protected:
	ManagedWeakReference<CreatureObject*> defenderRef;
	ManagedWeakReference<CreatureObject*> attackerRef;
	ManagedWeakReference<ZoneServer*> zoneServerRef;

	// Effect state
	ParticleBeamPhase phase;
	int currentTick;        // Current tick (1-4 for ramp phases)
	int totalTicks;         // Total ticks per phase (default 4)
	float reductionPerTick; // Calculated reduction per tick
	int tickIntervalMs;     // Time between ticks (default 5000ms)
	int peakDurationMs;     // How long to hold at peak
	int immunityDurationMs; // Immunity after effect ends

	// Accumulated reduction (for cleanup if defender dies)
	float accumulatedReduction;

public:
	DwParticleBeamTickTask(CreatureObject* attacker, CreatureObject* defender, ZoneServer* zoneServer,
						   float tickReduction, int numTicks, int intervalMs, int peakMs, int immunityMs)
		: Task() {
		attackerRef = attacker;
		defenderRef = defender;
		zoneServerRef = zoneServer;

		phase = PHASE_RAMP_UP;
		currentTick = 0;
		totalTicks = numTicks;
		reductionPerTick = tickReduction;
		tickIntervalMs = intervalMs;
		peakDurationMs = peakMs;
		immunityDurationMs = immunityMs;
		accumulatedReduction = 0.0f;
	}

	void run() {
		ManagedReference<CreatureObject*> defender = defenderRef.get();
		ManagedReference<ZoneServer*> zoneServer = zoneServerRef.get();

		if (defender == nullptr || zoneServer == nullptr) {
			return;  // Target gone
		}

		// Check if defender is dead - if so, clean up
		if (defender->isDead()) {
			cleanupEffect(defender);
			return;
		}

		Locker locker(defender);

		switch (phase) {
			case PHASE_RAMP_UP:
				processRampUp(defender);
				break;

			case PHASE_PEAK:
				processPeak(defender);
				break;

			case PHASE_RAMP_DOWN:
				processRampDown(defender, zoneServer);
				break;

			case PHASE_COMPLETE:
				// Should not reach here, but just in case
				return;
		}
	}

private:
	void processRampUp(CreatureObject* defender) {
		currentTick++;

		// Apply mitigation reduction buff
		// private_mitigation_divisor reduces damage mitigation
		// Positive value = less damage mitigation = more damage taken
		applyReductionTick(defender);

		// Send message on first tick
		if (currentTick == 1) {
			defender->sendSystemMessage("\\#FF6666Mand'alor's Grip takes hold! Your defenses are weakening...");
			defender->playEffect("clienteffect/pl_force_choke.cef", "");
		}

		// Check if ramp up complete
		if (currentTick >= totalTicks) {
			// Move to peak phase
			phase = PHASE_PEAK;

			StringBuffer peakMsg;
			peakMsg << "\\#FF0000Mand'alor's Grip is at full strength! (-" << (int)accumulatedReduction << "% mitigation)";
			defender->sendSystemMessage(peakMsg.toString());
			defender->playEffect("clienteffect/pl_force_choke_grip.cef", "");

			// Schedule peak duration
			reschedule(peakDurationMs);
		} else {
			// Schedule next ramp up tick
			reschedule(tickIntervalMs);
		}
	}

	void processPeak(CreatureObject* defender) {
		// Peak duration complete, start ramp down
		phase = PHASE_RAMP_DOWN;
		currentTick = totalTicks;  // Start from max tick

		defender->sendSystemMessage("\\#FFFF00Mand'alor's Grip is fading...");

		// Schedule first ramp down tick
		reschedule(tickIntervalMs);
	}

	void processRampDown(CreatureObject* defender, ZoneServer* zoneServer) {
		// Remove one tick's worth of reduction
		removeReductionTick(defender);
		currentTick--;

		// Check if ramp down complete
		if (currentTick <= 0) {
			// Effect complete
			phase = PHASE_COMPLETE;

			defender->sendSystemMessage("\\#00FF00Mand'alor's Grip has released. You are temporarily immune.");
			defender->playEffect("clienteffect/pl_force_absorb_hit.cef", "");

			// Apply immunity
			DeathWatchManager* dwManager = zoneServer->getDeathWatchManager();
			if (dwManager != nullptr) {
				dwManager->applyParticleBeamImmunity(defender, immunityDurationMs);
			}

			// Task complete, do not reschedule
			return;
		}

		// Schedule next ramp down tick
		reschedule(tickIntervalMs);
	}

	void applyReductionTick(CreatureObject* defender) {
		// Apply mitigation divisor buff
		// This effectively reduces their damage reduction
		// Value is added to the mitigation divisor (higher = less mitigation)
		int reductionInt = (int)(reductionPerTick * 10);  // Scale for buff system

		// Add skill mod buff
		defender->addSkillMod(SkillModManager::BUFF, "private_mitigation_divisor", reductionInt, false);
		accumulatedReduction += reductionPerTick;
	}

	void removeReductionTick(CreatureObject* defender) {
		int reductionInt = (int)(reductionPerTick * 10);

		defender->addSkillMod(SkillModManager::BUFF, "private_mitigation_divisor", -reductionInt, false);
		accumulatedReduction -= reductionPerTick;

		// Clamp to 0 to avoid negative accumulation
		if (accumulatedReduction < 0) {
			accumulatedReduction = 0;
		}
	}

	void cleanupEffect(CreatureObject* defender) {
		// Remove all accumulated reduction if defender dies mid-effect
		if (accumulatedReduction > 0 && defender != nullptr) {
			int reductionInt = (int)(accumulatedReduction * 10);
			Locker locker(defender);
			defender->addSkillMod(SkillModManager::BUFF, "private_mitigation_divisor", -reductionInt, false);
		}
	}
};

} // death_watch
} // managers
} // zone
} // server

using namespace server::zone::managers::death_watch;

#endif /* DWPARTICLEBEAMTICKTASK_H_ */
