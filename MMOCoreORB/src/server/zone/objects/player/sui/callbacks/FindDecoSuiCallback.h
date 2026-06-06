
#ifndef FINDDECOSUICALLBACK_H_
#define FINDDECOSUICALLBACK_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/objects/player/sui/SuiCallback.h"
#include "server/zone/managers/objectcontroller/ObjectController.h"
#include "server/zone/managers/city/CityDecorationTask.h"



class FindDecoSuiCallback : public SuiCallback {

public:
	FindDecoSuiCallback(ZoneServer* serv) : SuiCallback(serv) {

	}

	void run(CreatureObject* creature, SuiBox* sui, uint32 eventIndex, Vector<UnicodeString>* args) {
		bool cancelPressed = (eventIndex == 1);
		if (!sui->isListBox() || cancelPressed || server == nullptr)
			return;

		SuiListBox* listBox = cast<SuiListBox*>( sui);

		if(creature == nullptr || !creature->isPlayerCreature())
			return;

		bool otherPressed = Bool::valueOf(args->get(0).toString());

		int index = Integer::valueOf(args->get(1).toString());

		if(index < 0)
			return;

		uint64 objectID = listBox->getMenuObjectID(index);

		SceneObject* object = server->getObject(objectID);

		if (object == nullptr)
			return;

		if(otherPressed) {// Remove

			Reference<CityDecorationTask*> task = new CityDecorationTask(creature, cast<TangibleObject*>(object), CityDecorationTask::REMOVE);
			task->execute();

		} 
		else {  // Teleport Target to Mayor

			object->teleport(creature->getWorldPositionX(), creature->getWorldPositionZ(), creature->getWorldPositionY());
			//creature->sendSystemMessage("Attempting to target decoration by name.  Please verify target.  You might need to remove decorations with similar names.");
			//creature->sendExecuteConsoleCommand("/target " + object->getDisplayedName());


		}
	}
};

#endif /* FINDDECOSUICALLBACK_H_ */
