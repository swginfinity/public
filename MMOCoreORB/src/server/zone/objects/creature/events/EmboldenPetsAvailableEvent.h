/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef EMBOLDENPETSAVAILABLEEVENT_H_
#define EMBOLDENPETSAVAILABLEEVENT_H_

#include "server/zone/objects/creature/CreatureObject.h"

class EmboldenPetsAvailableEvent : public Task {
	ManagedWeakReference<CreatureObject*> creo;

public:
	EmboldenPetsAvailableEvent(CreatureObject* cr) : Task() {
		creo = cr;
	}

	void run() {
		ManagedReference<CreatureObject*> pet = creo.get();

		if (pet == nullptr)
			return;

		Locker locker(pet);

		pet->removePendingTask("embolden_notify");

		ManagedReference<CreatureObject*> owner = pet->getLinkedCreature().get();

		if (owner != nullptr) {
			String petName = pet->getCreatureName().toString();
			if (petName.beginsWith("("))
				petName = petName.subString(1, petName.length() - 1);
			owner->sendSystemMessage("Your pet, " +  petName + ", has recovered and can be emboldened again."); 
		}
	}

};

#endif /*EMBOLDENPETSAVAILABLEEVENT_H_*/
