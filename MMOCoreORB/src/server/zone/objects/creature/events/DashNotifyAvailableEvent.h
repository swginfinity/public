/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef DASHNOTIFYAVAILABLEEVENT_H_
#define DASHNOTIFYAVAILABLEEVENT_H_

#include "server/zone/objects/creature/CreatureObject.h"

class DashNotifyAvailableEvent : public Task {
	ManagedWeakReference<CreatureObject*> creo;

public:
	DashNotifyAvailableEvent(CreatureObject* cr) : Task() {
		creo = cr;
	}

	void run() {
		ManagedReference<CreatureObject*> creature = creo.get();

		if (creature == NULL)
			return;

		Locker locker(creature);

		creature->removePendingTask("dash_notify");
		creature->sendSystemMessage("You've now recovered from your last dash maneuver"); //"You are no longer tired.";
	}

};

#endif /*DASHNOTIFYAVAILABLEEVENT_H_*/
