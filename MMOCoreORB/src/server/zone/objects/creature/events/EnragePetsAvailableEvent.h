/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef ENRAGEPETSAVAILABLEEVENT_H_
#define ENRAGEPETSAVAILABLEEVENT_H_

#include "server/zone/objects/creature/CreatureObject.h"

class EnragePetsAvailableEvent : public Task {
	ManagedWeakReference<CreatureObject*> creo;

public:
	EnragePetsAvailableEvent(CreatureObject* cr) : Task() {
		creo = cr;
	}

	void run() {
		ManagedReference<CreatureObject*> pet = creo.get();

		if (pet == nullptr)
			return;

		Locker locker(pet);

		pet->removePendingTask("enrage_notify");

		ManagedReference<CreatureObject*> owner = pet->getLinkedCreature().get();

		if (owner != nullptr) {
			String petName = pet->getCreatureName().toString();
			if (petName.beginsWith("("))
				petName = petName.subString(1, petName.length() - 1);
			owner->sendSystemMessage("Your pet, " +  petName + ", has recovered and can be enraged again."); 
		}
	}

};

#endif /*ENRAGEPETSAVAILABLEEVENT_H_*/
