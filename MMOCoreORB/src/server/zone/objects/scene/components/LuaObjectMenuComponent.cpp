/*
 * LuaObjectMenuComponent.cpp
 *
 *  Created on: 29/01/2012
 *      Author: Elvaron
 */

#include "LuaObjectMenuComponent.h"
#include "server/zone/managers/director/DirectorManager.h"
#include "server/zone/packets/object/ObjectMenuResponse.h"

// Infinity:  Custom includes
#include "server/zone/objects/creature/CreatureObject.h"

// Infinity: Forward declaration for logLuaError used before its definition below
static void logLuaError(SceneObject* sceneObject, CreatureObject* player, const String& errorContext, int selectedID = -1);

LuaObjectMenuComponent::LuaObjectMenuComponent(const String& className) : luaClassName(className) {
}

LuaObjectMenuComponent::~LuaObjectMenuComponent() {
}

void LuaObjectMenuComponent::fillObjectMenuResponse(SceneObject* sceneObject, ObjectMenuResponse* menuResponse, CreatureObject* player) const {
	if (sceneObject == nullptr || player == nullptr) {
		return;
	}

	Lua* lua = DirectorManager::instance()->getLuaInstance();
	if (lua == nullptr) {
		return;
	}

	lua_State* L = lua->getLuaState();
	if (!L) {
		logLuaError(sceneObject, player, "LuaObjectMenuComponent::fillObjectMenuResponse - Invalid Lua state."); // Infinity:  Call custom logging function
		return;
	}

	LuaFunction runMethod(L, luaClassName, "fillObjectMenuResponse", 0);
	runMethod << sceneObject;
	runMethod << menuResponse;
	runMethod << player;

	if (!runMethod.callFunction()) {
		logLuaError(sceneObject, player, "LuaObjectMenuComponent::fillObjectMenuResponse - Lua function call failed."); // Infinity:  Call custom logging function
	}
}

int LuaObjectMenuComponent::handleObjectMenuSelect(SceneObject* sceneObject, CreatureObject* player, byte selectedID) const {
	if (sceneObject == nullptr || player == nullptr) {
		return 0;
	}

	Lua* lua = DirectorManager::instance()->getLuaInstance();
	if (lua == nullptr) {
		return 0;
	}

	lua_State* L = lua->getLuaState();
	if (!L) {
		logLuaError(sceneObject, player, "LuaObjectMenuComponent::handleObjectMenuSelect - Invalid Lua state.", selectedID); // Infinity:  Call custom logging function
		return 0;
	}

	LuaFunction runMethod(L, luaClassName, "handleObjectMenuSelect", 1);
	runMethod << sceneObject;
	runMethod << player;
	runMethod << selectedID;

	try {
		if (!runMethod.callFunction()) {
			logLuaError(sceneObject, player, "LuaObjectMenuComponent::handleObjectMenuSelect - Lua function call failed.", selectedID); // Infinity:  Call custom logging function
			return 0;
		}
	} catch (const std::exception& e) {
		logLuaError(sceneObject, player, "LuaObjectMenuComponent::handleObjectMenuSelect - Exception occurred: " + String(e.what()), selectedID);
		return 0;
	} catch (...) {
		logLuaError(sceneObject, player, "LuaObjectMenuComponent::handleObjectMenuSelect - Unknown exception occurred.", selectedID);
		return 0;
	}

	if (lua_gettop(L) == 0) {
		logLuaError(sceneObject, player, "LuaObjectMenuComponent::handleObjectMenuSelect - No return value on Lua stack.", selectedID); // Infinity:  Call custom logging function
		return 0;
	}

	if (!lua_isnumber(L, -1)) {
		logLuaError(sceneObject, player, "LuaObjectMenuComponent::handleObjectMenuSelect - Lua return value is not a number.", selectedID);
		lua_pop(L, 1);
		return 0;
	}

	int result = lua_tonumber(L, -1);
	lua_pop(L, 1);

	return result;
}

// Infinity: Custom logging function
static void logLuaError(SceneObject* sceneObject, CreatureObject* player, const String& errorContext, int selectedID) {
	String templateFilename = "Unknown";
	if (sceneObject != nullptr && sceneObject->getObjectTemplate() != nullptr) {
		templateFilename = sceneObject->getObjectTemplate()->getTemplateFileName();
	}

	StringBuffer msg;
	msg << errorContext;
	msg << "  Object ID: " << (sceneObject != nullptr ? sceneObject->getObjectID() : 0);
	msg << "  Object Template: " << templateFilename;
	msg << "  Object Name: " << (sceneObject != nullptr ? sceneObject->getDisplayedName() : "Unknown");
	msg << "  Player: " << (player != nullptr ? player->getFirstName() : "Unknown");

	// Infinity:  Added selectedID to error message, we know if it is not -1 it was passed by handleObjectMenuSelect function with a valid value
	if (selectedID > -1) {
		msg << "  Selected ID: " << selectedID;
	}

	if (sceneObject != nullptr) {
		String luaMenuComponentName;

		auto directorManager = DirectorManager::instance();

		if (directorManager != nullptr) {
			luaMenuComponentName = directorManager->readStringSharedMemory(String::valueOf(sceneObject->getObjectID()) + ":luaMenuComponentName");
		}

		if (!luaMenuComponentName.isEmpty()) {
			msg << "  Lua Menu Component : " << luaMenuComponentName;
		}
	}

	Logger::console.error(msg.toString());
}
