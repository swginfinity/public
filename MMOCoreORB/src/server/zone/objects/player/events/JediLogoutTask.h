/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions. */

#ifndef JEDILOGOUTTASK_H_
#define JEDILOGOUTTASK_H_

#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/managers/jedi/JediManager.h"
#include "server/zone/managers/director/DirectorManager.h"

// Infinity: Deferred JediManager + screenplay logout to avoid deadlock.
// Creature lock held by PlayerDisconnectEvent conflicts with Zone wlock
// needed by village Lua destroyObjectFromWorld cleanup.
class JediLogoutTask : public Task {
	ManagedWeakReference<CreatureObject*> creature;

public:
	JediLogoutTask(CreatureObject* cr) : creature(cr) {
	}

	void run() {
		ManagedReference<CreatureObject*> strongRef = creature.get();

		if (strongRef == nullptr) {
			return;
		}

		Locker locker(strongRef);

		JediManager::instance()->onPlayerLoggedOut(strongRef);

		Lua* lua = DirectorManager::instance()->getLuaInstance();
		Reference<LuaFunction*> luaOnPlayerLoggedOut = lua->createFunction("PlayerTriggers", "playerLoggedOut", 0);
		*luaOnPlayerLoggedOut << strongRef;
		luaOnPlayerLoggedOut->callFunction();
	}
};

#endif /* JEDILOGOUTTASK_H_ */
