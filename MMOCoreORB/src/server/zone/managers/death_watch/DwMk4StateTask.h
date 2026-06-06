/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.

	Infinity: Death Watch Mk4 Ground Striker State Clear Task

	Scheduled task to clear states and restore HUD after Mk4 missile effects.
	Supports clearing DIZZY, BLINDED states and restoring HUD elements.
*/

#ifndef DWMK4STATETASK_H_
#define DWMK4STATETASK_H_

#include "engine/engine.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "templates/params/creature/CreatureState.h"

namespace server {
namespace zone {
namespace managers {
namespace death_watch {

// Task types for Mk4 state clearing
enum DwMk4TaskType : uint8 {
	DW_MK4_CLEAR_DIZZY = 1,
	DW_MK4_CLEAR_BLINDED = 2,
	DW_MK4_RESTORE_HUD = 3
};

class DwMk4StateTask : public Task {
protected:
	ManagedWeakReference<CreatureObject*> targetRef;
	DwMk4TaskType taskType;

public:
	DwMk4StateTask(CreatureObject* target, DwMk4TaskType type)
		: Task() {
		targetRef = target;
		taskType = type;
	}

	void run() {
		ManagedReference<CreatureObject*> target = targetRef.get();

		if (target == nullptr) {
			return;  // Target gone
		}

		Locker locker(target);

		switch (taskType) {
			case DW_MK4_CLEAR_DIZZY:
				if (target->hasState(CreatureState::DIZZY)) {
					target->clearState(CreatureState::DIZZY, true);
				}
				break;

			case DW_MK4_CLEAR_BLINDED:
				if (target->hasState(CreatureState::BLINDED)) {
					target->clearState(CreatureState::BLINDED, true);
				}
				break;

			case DW_MK4_RESTORE_HUD:
				// Restore all HUD elements
				target->sendNewbieTutorialEnableHudElement("all", 1, 0);
				target->sendSystemMessage("\\#00FF00Your HUD systems have recovered.");
				break;
		}
	}
};

} // death_watch
} // managers
} // zone
} // server

using namespace server::zone::managers::death_watch;

#endif /* DWMK4STATETASK_H_ */
