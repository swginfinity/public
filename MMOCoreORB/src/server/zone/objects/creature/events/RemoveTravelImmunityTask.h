/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef REMOVETRAVELIMMUNITYTASK_H_
#define REMOVETRAVELIMMUNITYTASK_H_

#include "server/zone/objects/creature/CreatureObject.h"

class RemoveTravelImmunityTask : public Task {
	ManagedReference<CreatureObject*> player;
	String taskName;

public:
	RemoveTravelImmunityTask(CreatureObject* creo, const String& task) : Task() {
		player = creo;
	}

	void run() {

		if (player == nullptr)
			return;

		Locker locker(player);
		player->removePendingTask("travel_immunity");
		player->broadcastPvpStatusBitmask();

	}
};

#endif /*REMOVETRAVELIMMUNITYTASK_H_*/
