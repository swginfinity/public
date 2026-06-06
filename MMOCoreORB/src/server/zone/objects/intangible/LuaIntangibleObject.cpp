/*
 * LuaIntangibleObject.cpp
 *
 *  Created on: 13/01/2012
 *      Author: victor
 */

#include "LuaIntangibleObject.h"
#include "server/zone/objects/intangible/IntangibleObject.h"
// Infinity:  Custom
#include "server/zone/objects/tangible/TangibleObject.h"
#include "server/zone/objects/intangible/ControlDevice.h"
#include "server/zone/objects/intangible/PetControlDevice.h"
#include "server/zone/objects/player/PlayerObject.h"
#include "server/zone/objects/creature/ai/AiAgent.h"

const char LuaIntangibleObject::className[] = "LuaIntangibleObject";

Luna<LuaIntangibleObject>::RegType LuaIntangibleObject::Register[] = {
		{ "_setObject", &LuaIntangibleObject::_setObject },
		{ "_getObject", &LuaSceneObject::_getObject },
		// Infinity:  Custom hooks below
		{ "destroyPCD", &LuaIntangibleObject::destroyPCD },
		{ "getControlledObject", &LuaIntangibleObject::getControlledObject },
		{ 0, 0 }
};

LuaIntangibleObject::LuaIntangibleObject(lua_State* L) : LuaSceneObject(L) {
#ifdef DYNAMIC_CAST_LUAOBJECTS
	realObject = dynamic_cast<IntangibleObject*>(_getRealSceneObject());

	// Infinity:  No need to crash server with assertion, just throw a Lua error and return
	if (_getRealSceneObject() != nullptr && realObject == nullptr) {
		Logger::console.error(LuaSceneObject::getLuaCallStack(L, "LuaIntangibleObject::LuaIntangibleObject - Detected assertion failure condition."));
		return;
	}

	// E3_ASSERT(!_getRealSceneObject() || realObject != nullptr);
#else
	realObject = reinterpret_cast<IntangibleObject*>(lua_touserdata(L, 1));
#endif
}

LuaIntangibleObject::~LuaIntangibleObject() {
}

int LuaIntangibleObject::_setObject(lua_State* L) {
	LuaSceneObject::_setObject(L);

#ifdef DYNAMIC_CAST_LUAOBJECTS
	realObject = dynamic_cast<IntangibleObject*>(_getRealSceneObject());

	// Infinity:  No need to crash server with assertion, just throw a Lua error and return
	if (_getRealSceneObject() != nullptr && realObject == nullptr) {
		String msg = "LuaIntangibleObject::_setObject - Detected assertion failure condition." + LuaSceneObject::getLuaCallStack(L);
		return luaL_error(L, msg.toCharArray());
	}

	// E3_ASSERT(!_getRealSceneObject() || realObject != nullptr);
#else
	realObject = reinterpret_cast<IntangibleObject*>(lua_touserdata(L, -1));
#endif

	return 0;
}

int LuaIntangibleObject::destroyPCD(lua_State* L) {

	if (realObject != nullptr) {

		int numberOfArguments = lua_gettop(L) - 1;

		if (numberOfArguments != 1) {
			Logger::console.error("LuaIntangibleObject::destroyPCD - incorrect number of arguments");
			return 0;
		}

		CreatureObject* owner = (CreatureObject*) lua_touserdata(L, -1);
		if (owner == nullptr || !owner->isPlayerCreature()) {
			Logger::console.error("LuaIntangibleObject::destroyPCD - invalid creature object");
			return 0;
		}

		auto ghost = owner->getPlayerObject();
		if (ghost == nullptr) {
			Logger::console.error("LuaIntangibleObject::destroyPCD - invalid player object");
			return 0;
		}

		ManagedReference<PetControlDevice*> petControlDevice = cast<PetControlDevice*>(realObject);
		if (petControlDevice == nullptr) {
			Logger::console.error("LuaIntangibleObject::destroyPCD - invalid PCD");
			return 0;
		}

		ManagedReference<TangibleObject*> controlledObject = petControlDevice->getControlledObject();
		if (controlledObject == nullptr || !controlledObject->isAiAgent()) {
			Logger::console.error("LuaIntangibleObject::destroyPCD - controlledObject is nullptr or not AiAgent");
			return 0;
		}

		ManagedReference<AiAgent*> pet = controlledObject.castTo<AiAgent*>();
		if (pet == nullptr) {
			Logger::console.error("LuaIntangibleObject::destroyPCD - pet is nullptr");
			return 0;
		}

		int storeDelay = 0;

		if (ghost->hasActivePet(pet)) {

            storeDelay = 500;   //Delay the PCD delete by 500ms

            Reference<PetControlDevice*> strongRef = petControlDevice;
            Reference<CreatureObject*> ownerRef = owner;
            Core::getTaskManager()->executeTask([strongRef, ownerRef] () {    //Moving this to a task to possibly prevent deadlocks

                if (strongRef == nullptr || ownerRef == nullptr)
                    return;

                Locker locker(strongRef);
                strongRef->storeObject(ownerRef, true);  //Force Store the pet, this runs as a task

            }, "DestroyPCDStoreTask");
		}

		Reference<PetControlDevice*> strongRef = petControlDevice;
		Core::getTaskManager()->scheduleTask([strongRef] () {

			if (strongRef == nullptr)
				return;

			Locker locker(strongRef);
			strongRef->destroyObjectFromWorld(true);
			strongRef->destroyObjectFromDatabase(true);

		}, "DestroyPCDTask", storeDelay);

		return 0;
	}
	else {
		Logger::console.error("LuaIntangibleObject::destroyPCD - realObject is nullptr");
		return 0;
	}

	return 0;
}

int LuaIntangibleObject::getControlledObject(lua_State* L) {

	if (realObject == nullptr || !realObject->isControlDevice()) {
		Logger::console.error("LuaIntangibleObject::getControlledObject - null or invalid control device");
		lua_pushnil(L);
		return 1;
	}

	ManagedReference<ControlDevice*> controlDevice = cast<ControlDevice*>(realObject);
	if (controlDevice == nullptr) {
		Logger::console.error("LuaIntangibleObject::getControlledObject - invalid control device");
		return 1;
	}

	lua_pushlightuserdata(L, controlDevice->getControlledObject());

	return 1;
}

