/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef GMSCREENPLAYSCOMMAND_H_
#define GMSCREENPLAYSCOMMAND_H_

#include "server/zone/managers/director/DirectorManager.h"


class GmScreenplaysCommand : public QueueCommand {
public:

	GmScreenplaysCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {
		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		Lua* lua = DirectorManager::instance()->getLuaInstance();

		Reference<LuaFunction*> luaGmScreenplaysCmd = lua->createFunction("infinity_screenplay_gm_sui", "showMainPage", 0);
		*luaGmScreenplaysCmd << creature;

		luaGmScreenplaysCmd->callFunction();

		return SUCCESS;
	}

};

#endif //GMSCREENPLAYSCOMMAND_H_
