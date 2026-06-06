
#ifndef CLEARACTIVEPETSTASK_H_
#define CLEARACTIVEPETSTASK_H_

#include "server/zone/objects/player/PlayerObject.h"
#include "server/zone/objects/creature/ai/AiAgent.h"

class ClearActivePetsTask : public Task {
	ManagedReference<PlayerObject*> player;

public:
	ClearActivePetsTask(PlayerObject* playerObject) : Task() {
		player = playerObject;
	}

	void run() {
		if (player == nullptr)
			return;

		Locker locker(player);

		for (int i = 0; i < player->getActivePetsSize(); i++) {
			ManagedReference<AiAgent*> pet = player->getActivePet(i);
			if (pet != nullptr) {
				player->removeFromActivePets(pet);
			}
		}
	}
};

#endif /* CLEARACTIVEPETSTASK_H_ */
