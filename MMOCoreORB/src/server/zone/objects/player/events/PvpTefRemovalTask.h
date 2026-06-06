
#ifndef PVPTEFREMOVALTASK_H_
#define PVPTEFREMOVALTASK_H_

#include "server/zone/objects/player/PlayerObject.h"
#include "templates/params/creature/ObjectFlag.h"

namespace server {
namespace zone {
namespace objects {
namespace player {
namespace events {

class PvpTefRemovalTask: public Task {
	ManagedWeakReference<CreatureObject*> creature;
	bool hadDwTef;

public:
	PvpTefRemovalTask(CreatureObject* creo) {
		creature = creo;
		hadDwTef = false;
	}

	void run() {
		ManagedReference<CreatureObject*> player = creature.get();

		if (player == nullptr)
			return;

		ManagedReference<PlayerObject*> ghost = player->getPlayerObject().get();

		if (ghost == nullptr) {
			return;
		}

		Locker locker(player);

		// Track if DW TEF was active before this check
		bool currentlyHasDwTef = ghost->hasDwTef();

		if (ghost->hasTef() || currentlyHasDwTef) {
			// Remember DW TEF state for next run
			hadDwTef = currentlyHasDwTef;

			auto gcwCrackdownTefMs = ghost->getLastGcwCrackdownCombatActionTimestamp().miliDifference();
			auto gcwTefMs = ghost->getLastGcwPvpCombatActionTimestamp().miliDifference();
			auto bhTefMs = ghost->getLastBhPvpCombatActionTimestamp().miliDifference();
			auto pvpAreaMs = ghost->getLastPvpAreaCombatActionTimestamp().miliDifference();
			auto dwTefMs = ghost->getLastDwPvpCombatActionTimestamp().miliDifference();

			auto rescheduleTime = gcwTefMs < bhTefMs ? gcwTefMs : bhTefMs;
			rescheduleTime = gcwCrackdownTefMs < rescheduleTime ? gcwCrackdownTefMs : rescheduleTime;
			rescheduleTime = pvpAreaMs < rescheduleTime ? pvpAreaMs : rescheduleTime;
			rescheduleTime = dwTefMs < rescheduleTime ? dwTefMs : rescheduleTime;

			this->reschedule(llabs(rescheduleTime));
		} else {
			// TEF expired - notify if DW TEF just expired
			if (hadDwTef) {
				player->sendSystemMessage("Your Death Watch TEF has expired. You may now enter buildings.");
				hadDwTef = false;
			}

			ghost->updateInRangeBuildingPermissions();
			ghost->setCrackdownTefTowards(0, false);
			player->clearPvpStatusBit(ObjectFlag::TEF, true);

			if (ConfigManager::instance()->useCovertOvertSystem())
				player->broadcastPvpStatusBitmask();
		}

		if (!ghost->hasBhTef())
			player->notifyObservers(ObserverEventType::BHTEFCHANGED);
	}
};

}
}
}
}
}

using namespace server::zone::objects::player::events;

#endif /* PVPTEFREMOVALTASK_H_ */
