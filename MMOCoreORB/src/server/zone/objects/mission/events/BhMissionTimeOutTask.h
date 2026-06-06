/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions. */

#ifndef BHMISSIONTIMEOUTTASK_H_
#define BHMISSIONTIMEOUTTASK_H_

#include "server/zone/objects/mission/MissionObject.h"

namespace server {
namespace zone {
namespace objects {
namespace mission {
namespace events {

class BhMissionTimeoutTask : public Task {
	ManagedWeakReference<MissionObject*> object;

public:
	BhMissionTimeoutTask(MissionObject* object) {
		this->object = object;
	}

	void run() {
		ManagedReference<MissionObject*> objectRef = object.get();

		if (objectRef != nullptr) {
			ManagedReference<MissionObjective*> objectiveRef = objectRef->getMissionObjective();

			if (objectiveRef != nullptr) {
				ManagedReference<CreatureObject*> owner = objectiveRef->getPlayerOwner();
				
				if (owner != nullptr) {

					auto ghost = owner->getPlayerObject();

					if (ghost != nullptr) {

						if (!ghost->isOnline()) {  //Only cancel mission if still not online and timeout has been exceeded

							Time currentTime;
							uint64 timeout = 1000 * 60 * 10 / 10;   //BH Mission Timeout after BH is out of game for 10 minutes
							uint64 logoutDelta = currentTime.getMiliTime() - ghost->getLastLogout()->getMiliTime();
							//Logger::console.error("logoutDelta = " + String::valueOf(logoutDelta / 1000.0f) + " seconds, timeout = " + String::valueOf(timeout / 1000.0f));

							if (logoutDelta >= timeout) {
								owner->sendSystemMessage("Your bounty hunter mission has expired.");
								Locker locker(owner);
								objectiveRef->fail();
							}
						}
					}
				}
			}
		}
	}
};

} // namespace events
} // namespace mission
} // namespace objects
} // namespace zone
} // namespace server

using namespace server::zone::objects::mission::events;

#endif /* BHMISSIONTIMEOUTTASK_H_ */
