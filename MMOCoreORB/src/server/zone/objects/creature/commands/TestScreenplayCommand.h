/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef TESTSCREENPLAYCOMMAND_H_
#define TESTSCREENPLAYCOMMAND_H_

#include "server/zone/managers/director/DirectorManager.h"

class TestStackTraceTask : public Task {
	ManagedWeakReference<CreatureObject*> creatureRef;

public:
	TestStackTraceTask(CreatureObject* creature) : creatureRef(creature) {
	}

	void run() {
		try {
			Vector<int> testVector;
			testVector.get(100);
		} catch (Exception& e) {
			e.printStackTrace();

			auto creature = creatureRef.get();
			if (creature != nullptr) {
				creature->error() << "TestStackTraceTask - exception from task thread: " << e.getMessage();
			}
		}
	}
};

class TestScreenplayCommand : public QueueCommand {
public:

	TestScreenplayCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {
		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		// Disabled on live server
		auto zoneServer = creature->getZoneServer();
		if (zoneServer != nullptr && zoneServer->getGalaxyName() == "SWG Infinity") {
			return GENERALERROR;
		}

		String args = arguments.toString().trim();

		if (args.isEmpty()) {
			args = "testScreenplayCommand";
		}

		Lua* lua = DirectorManager::instance()->getLuaInstance();
		Reference<LuaFunction*> luaFunc = lua->createFunction("TestScreenplay", args, 0);
		*luaFunc << creature;
		luaFunc->callFunction();

		return SUCCESS;
	}

};

#endif //TESTSCREENPLAYCOMMAND_H_
