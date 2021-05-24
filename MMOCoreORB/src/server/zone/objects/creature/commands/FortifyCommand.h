/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef FORTIFYCOMMAND_H_
#define FORTIFYCOMMAND_H_

#include "JediQueueCommand.h"
#include "server/zone/objects/scene/SceneObject.h"

class FortifyCommand : public JediQueueCommand {
public:

	FortifyCommand(const String& name, ZoneProcessServer* server)
		: JediQueueCommand(name, server) {
			buffCRC = BuffCRC::JEDI_FORTIFY;
	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {
		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		if (isWearingArmor(creature)) {
			return NOJEDIARMOR;
		}

		return doJediSelfBuffCommand(creature);
	}

	float getCommandDuration(CreatureObject* object, const UnicodeString& arguments) const {
		return defaultTime;
	}

};

#endif //FORTIFYCOMMAND_H_
