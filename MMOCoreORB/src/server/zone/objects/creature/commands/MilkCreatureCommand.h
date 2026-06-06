/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef MILKCREATURECOMMAND_H_
#define MILKCREATURECOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/objects/creature/ai/Creature.h"
#include "server/zone/managers/creature/CreatureManager.h"
#include "server/zone/Zone.h"

class MilkCreatureCommand : public QueueCommand {
public:

	MilkCreatureCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		if (!creature->isPlayerCreature())
			return INVALIDTARGET;

		auto zoneServer = server->getZoneServer();
		if (zoneServer == nullptr)
			return GENERALERROR;

		if (target == 0) {
			creature->sendSystemMessage("You must have a creature targeted to use the /milkCreature command.");
			return INVALIDTARGET;
		}

		ManagedReference<SceneObject*> object = zoneServer->getObject(target);

		if (object == nullptr || !object->isCreature()) {
			creature->sendSystemMessage("You must have a creature targeted to use the /milkCreature command.");
			return INVALIDTARGET;
		}

		ManagedReference<Creature*> targetCreature = cast<Creature*>(object.get());
		if (targetCreature == nullptr) {
			creature->sendSystemMessage("You must have a creature targeted to use the /milkCreature command.");
			return INVALIDTARGET;
		}

		if (targetCreature->canMilkMe(creature)) {

			auto zone = targetCreature->getZone();
			if (zone == nullptr)
				return GENERALERROR;

			ManagedReference<CreatureManager*> creatureManager = zone->getCreatureManager();
			if (creatureManager == nullptr)
				return GENERALERROR;

			creatureManager->milk(targetCreature, creature);
		}
		else {
			float maxRange = 9;
				
			if (!creature->isInRange(targetCreature, maxRange)) {
				StringIdChatParameter milkMessage("skl_use", "milk_out_of_range");
				milkMessage.setDI(maxRange);
				creature->sendSystemMessage(milkMessage);    // You must be within X meters of the creature to begin milking it.
			}
			return INVALIDTARGET;
		}

		return SUCCESS;
	}
};

#endif //MILKCREATURECOMMAND_H_
