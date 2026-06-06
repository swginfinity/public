/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef ROCKETBOOTSNOTIFYAVAILABLEEVENT_H_
#define ROCKETBOOTSNOTIFYAVAILABLEEVENT_H_

#include "server/zone/objects/creature/CreatureObject.h"

class RocketBootsNotifyAvailableEvent : public Task {
	ManagedWeakReference<CreatureObject*> creo;

public:
	RocketBootsNotifyAvailableEvent(CreatureObject* cr) : Task() {
		creo = cr;
	}

	void run() {
		ManagedReference<CreatureObject*> creature = creo.get();

		if (creature == nullptr)
			return;

		Locker locker(creature);

		creature->removePendingTask("rocket_boots_notify");
		creature->sendSystemMessage("Your rocketboots have recharged");
	}

};

#endif /*ROCKETBOOTSNOTIFYAVAILABLEEVENT_H_*/
