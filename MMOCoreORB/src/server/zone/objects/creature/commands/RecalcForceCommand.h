/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef RECALCFORCECOMMAND_H_
#define RECALCFORCECOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/managers/skill/SkillManager.h"

class RecalcForceCommand : public QueueCommand {
public:

	RecalcForceCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {
		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		ManagedReference<SceneObject*> object = server->getZoneServer()->getObject(target);

		if (object == NULL || !object->isCreatureObject())
			return INVALIDTARGET;

		CreatureObject* targetCreature = cast<CreatureObject*>( object.get());

		Locker clocker(targetCreature, creature);

		SkillManager* skillManager = SkillManager::instance();
		skillManager->surrenderSkill(arguments.toString(), targetCreature, true);
		skillManager->awardForceFromSkills(targetCreature);


			creature->sendSystemMessage("Recalculated Max force and Regen");

		return SUCCESS;
	}

};

#endif //RecalcForceCommand_H_
