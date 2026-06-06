/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.

	Infinity: Death Watch PvP Refresh Task

	Periodic task that runs the PvP broadcast self-correction cycle.
	Ensures all Death Watch players have synchronized PvP status,
	handling edge cases like cross-planet arrivals and missed broadcasts.

	Runs every 30-60 seconds and processes up to MAX_REFRESH_PER_CYCLE
	players per run to avoid server lag spikes.
*/

#ifndef DWPVPREFRESH_TASK_H_
#define DWPVPREFRESH_TASK_H_

#include "engine/core/ManagedReference.h"
#include "server/zone/managers/death_watch/DeathWatchManager.h"

class DwPvpRefreshTask : public Task {
	ManagedWeakReference<DeathWatchManager*> dwManager;

	// Interval in milliseconds between refresh cycles
	static const int REFRESH_INTERVAL_MS = 45000;  // 45 seconds

public:
	DwPvpRefreshTask(DeathWatchManager* manager) {
		dwManager = manager;
	}

	void run() {
		ManagedReference<DeathWatchManager*> manager = dwManager.get();

		if (manager == nullptr) {
			return;
		}

		try {
			// Run the refresh cycle
			manager->runBroadcastRefreshCycle();
		} catch (...) {
			// Log but don't crash - we'll try again next cycle
		}

		// Reschedule for next cycle
		reschedule(REFRESH_INTERVAL_MS);
	}
};

#endif // DWPVPREFRESH_TASK_H_
