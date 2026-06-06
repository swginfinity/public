/*
 * LuaActiveArea.cpp
 *
 *  Created on: 21/01/2012
 *      Author: xyborn
 */

#include "LuaActiveArea.h"
#include "server/zone/objects/area/ActiveArea.h"

const char LuaActiveArea::className[] = "LuaActiveArea";

Luna<LuaActiveArea>::RegType LuaActiveArea::Register[] = {
		{ "_setObject", &LuaActiveArea::_setObject },
		{ "_getObject", &LuaSceneObject::_getObject },
		{ "setRadius", &LuaActiveArea::setRadius },
		{ "getRadius", &LuaActiveArea::getRadius },
		{ "setNoBuildArea", &LuaActiveArea::setNoBuildArea },
		{ "isNoBuildArea", &LuaActiveArea::isNoBuildArea },
		{ "getObjectID", &LuaSceneObject::getObjectID },
		{ "setCellObjectID", &LuaActiveArea::setCellObjectID },
		{ "getCellObjectID", &LuaActiveArea::getCellObjectID },
		{ "isNoSpawnArea", &LuaActiveArea::isNoSpawnArea },
		{ "setNoSpawnArea", &LuaActiveArea::setNoSpawnArea },
		{ "isPvpArea", &LuaActiveArea::isPvpArea },
		{ "setPvpArea", &LuaActiveArea::setPvpArea },
		{ "isOvertOnlyArea", &LuaActiveArea::isOvertOnlyArea },
		{ "setOvertOnlyArea", &LuaActiveArea::setOvertOnlyArea },
		{ "isRebelOnlyArea", &LuaActiveArea::isRebelOnlyArea },
		{ "setRebelOnlyArea", &LuaActiveArea::setRebelOnlyArea },
		{ "isImperialOnlyArea", &LuaActiveArea::isImperialOnlyArea },
		{ "setImperialOnlyArea", &LuaActiveArea::setImperialOnlyArea },
		{ "isNoCombatArea", &LuaActiveArea::isNoCombatArea },
		{ "setNoCombatArea", &LuaActiveArea::setNoCombatArea },
		{ "isNoPetArea", &LuaActiveArea::isNoPetArea },
		{ "setNoPetArea", &LuaActiveArea::setNoPetArea },
		{ "isLockedArea", &LuaActiveArea::isLockedArea },
		{ "setLockedArea", &LuaActiveArea::setLockedArea },
		{ "setCampingArea", &LuaActiveArea::setCampingArea },
		{ "isCampingArea", &LuaActiveArea::isCampingArea },
		{ "setNoDuelArea", &LuaActiveArea::setNoDuelArea },
		{ "isNoDuelArea", &LuaActiveArea::isNoDuelArea },
		{ "getAreaName", &LuaActiveArea::getAreaName },
		// Infinity:  Custom hooks
		{ "getAreaCenterX", &LuaActiveArea::getAreaCenterX },
		{ "getAreaCenterY", &LuaActiveArea::getAreaCenterY },
		{ "getAreaCenterZ", &LuaActiveArea::getAreaCenterZ },
		{ "setAreaName", &LuaActiveArea::setAreaName },
		{ 0, 0 }
};

LuaActiveArea::LuaActiveArea(lua_State* L) : LuaSceneObject(L) {
#ifdef DYNAMIC_CAST_LUAOBJECTS
	realObject = dynamic_cast<ActiveArea*>(_getRealSceneObject());

	// Infinity:  No need to crash server with assertion, just throw a Lua error and return
	if (_getRealSceneObject() != nullptr && realObject == nullptr) {
		Logger::console.error(LuaSceneObject::getLuaCallStack(L, "LuaActiveArea::LuaActiveArea - Detected assertion failure condition."));
		return;
	}

	// E3_ASSERT(!_getRealSceneObject() || realObject != nullptr);
#else
	realObject = static_cast<ActiveArea*>(lua_touserdata(L, 1));
#endif
}

LuaActiveArea::~LuaActiveArea(){
}

int LuaActiveArea::_setObject(lua_State* L) {
	LuaSceneObject::_setObject(L);

#ifdef DYNAMIC_CAST_LUAOBJECTS
	realObject = dynamic_cast<ActiveArea*>(_getRealSceneObject());

	// Infinity:  No need to crash server with assertion, just throw a Lua error and return
	if (_getRealSceneObject() != nullptr && realObject == nullptr) {
		String msg = "LuaActiveArea::_setObject - Detected assertion failure condition." + LuaSceneObject::getLuaCallStack(L);
		return luaL_error(L, msg.toCharArray());
	}

	// E3_ASSERT(!_getRealSceneObject() || realObject != nullptr);
#else
	realObject = static_cast<ActiveArea*>(lua_touserdata(L, -1));
#endif

	return 0;
}

int LuaActiveArea::getRadius(lua_State* L) {
	lua_pushnumber(L, realObject->getRadius());

	return 1;
}

int LuaActiveArea::setRadius(lua_State* L) {
	int radius = lua_tonumber(L, -1);
	Locker realObjectLocker(realObject);
	realObject->setRadius(radius);

	return 0;
}

int LuaActiveArea::setNoBuildArea(lua_State* L) {
	bool val = lua_toboolean(L, -1);
	Locker realObjectLocker(realObject);

	if (val) {
		realObject->addAreaFlag(ActiveArea::NOBUILDZONEAREA);
	} else {
		realObject->removeAreaFlag(ActiveArea::NOBUILDZONEAREA);
	}

	return 0;
}

int LuaActiveArea::isNoBuildArea(lua_State* L) {
	bool val = realObject->isNoBuildZone();

	lua_pushboolean(L, val);

	return 1;
}

int LuaActiveArea::setCellObjectID(lua_State* L) {
	uint64 val = lua_tointeger(L, -1);
	Locker realObjectLocker(realObject);
	realObject->setCellObjectID(val);

	return 0;
}

int LuaActiveArea::getCellObjectID(lua_State* L) {
	uint64 val = realObject->getCellObjectID();

	lua_pushinteger(L, val);

	return 1;
}

int LuaActiveArea::setNoSpawnArea(lua_State* L) {
	bool val = lua_toboolean(L, -1);
	Locker realObjectLocker(realObject);

	if (val) {
		realObject->addAreaFlag(ActiveArea::NOSPAWNAREA);
		realObject->addAreaFlag(ActiveArea::NOWORLDSPAWNAREA);
	} else {
		realObject->removeAreaFlag(ActiveArea::NOSPAWNAREA);
		realObject->removeAreaFlag(ActiveArea::NOWORLDSPAWNAREA);
	}

	return 0;
}

int LuaActiveArea::isNoSpawnArea(lua_State* L) {
	bool val = realObject->isNoSpawnArea();

	lua_pushboolean(L, val);

	return 1;
}

int LuaActiveArea::setPvpArea(lua_State* L) {
	bool val = lua_toboolean(L, -1);
	Locker realObjectLocker(realObject);

	if (val) {
		realObject->addAreaFlag(ActiveArea::PVPAREA);
	} else {
		realObject->removeAreaFlag(ActiveArea::PVPAREA);
	}

	return 0;
}

int LuaActiveArea::isPvpArea(lua_State* L) {
	bool val = realObject->isPvpArea();

	lua_pushboolean(L, val);

	return 1;
}

int LuaActiveArea::setOvertOnlyArea(lua_State* L) {
	bool val = lua_toboolean(L, -1);
	Locker realObjectLocker(realObject);

	if (val) {
		realObject->addAreaFlag(ActiveArea::OVERTAREA);
	} else {
		realObject->removeAreaFlag(ActiveArea::OVERTAREA);
	}

	return 0;
}

int LuaActiveArea::isOvertOnlyArea(lua_State* L) {
	bool val = realObject->isOvertArea();

	lua_pushboolean(L, val);

	return 1;
}

int LuaActiveArea::setRebelOnlyArea(lua_State* L) {
	bool val = lua_toboolean(L, -1);
	Locker realObjectLocker(realObject);

	if (val) {
		realObject->addAreaFlag(ActiveArea::REBELAREA);
	} else {
		realObject->removeAreaFlag(ActiveArea::REBELAREA);
	}

	return 0;
}

int LuaActiveArea::isRebelOnlyArea(lua_State* L) {
	bool val = realObject->isRebelArea();

	lua_pushboolean(L, val);

	return 1;
}

int LuaActiveArea::setImperialOnlyArea(lua_State* L) {
	bool val = lua_toboolean(L, -1);
	Locker realObjectLocker(realObject);

	if (val) {
		realObject->addAreaFlag(ActiveArea::IMPERIALAREA);
	} else {
		realObject->removeAreaFlag(ActiveArea::IMPERIALAREA);
	}

	return 0;
}

int LuaActiveArea::isImperialOnlyArea(lua_State* L) {
	bool val = realObject->isImperialArea();

	lua_pushboolean(L, val);

	return 1;
}

int LuaActiveArea::setNoCombatArea(lua_State* L) {
	bool val = lua_toboolean(L, -1);
	Locker realObjectLocker(realObject);

	if (val) {
		realObject->addAreaFlag(ActiveArea::NOCOMBATAREA);
	} else {
		realObject->removeAreaFlag(ActiveArea::NOCOMBATAREA);
	}

	return 0;
}

int LuaActiveArea::isNoCombatArea(lua_State* L) {
	bool val = realObject->isNoCombatArea();

	lua_pushboolean(L, val);

	return 1;
}

int LuaActiveArea::setNoPetArea(lua_State* L) {
	bool val = lua_toboolean(L, -1);
	Locker realObjectLocker(realObject);

	if (val) {
		realObject->addAreaFlag(ActiveArea::NOPETAREA);
	} else {
		realObject->removeAreaFlag(ActiveArea::NOPETAREA);
	}

	return 0;
}

int LuaActiveArea::isNoPetArea(lua_State* L) {
	bool val = realObject->isNoPetArea();

	lua_pushboolean(L, val);

	return 1;
}

int LuaActiveArea::setLockedArea(lua_State* L) {
	bool val = lua_toboolean(L, -1);
	Locker realObjectLocker(realObject);

	if (val) {
		realObject->addAreaFlag(ActiveArea::LOCKEDAREA);
	} else {
		realObject->removeAreaFlag(ActiveArea::LOCKEDAREA);
	}

	return 0;
}

int LuaActiveArea::isLockedArea(lua_State* L) {
	bool val = realObject->isLockedArea();

	lua_pushboolean(L, val);

	return 1;
}

int LuaActiveArea::setCampingArea(lua_State* L) {
	bool val = lua_toboolean(L, -1);
	Locker realObjectLocker(realObject);

	if (val) {
		realObject->addAreaFlag(ActiveArea::CAMPINGAREA);
	} else {
		realObject->removeAreaFlag(ActiveArea::CAMPINGAREA);
	}

	return 0;
}

int LuaActiveArea::isCampingArea(lua_State* L) {
	bool val = realObject->isCampingArea();

	lua_pushboolean(L, val);

	return 1;
}

int LuaActiveArea::setNoDuelArea(lua_State* L) {
	bool val = lua_toboolean(L, -1);
	Locker realObjectLocker(realObject);

	if (val) {
		realObject->addAreaFlag(ActiveArea::NODUELAREA);
	} else {
		realObject->removeAreaFlag(ActiveArea::NODUELAREA);
	}

	return 0;
}

int LuaActiveArea::isNoDuelArea(lua_State* L) {
	bool val = realObject->isNoDuelArea();

	lua_pushboolean(L, val);

	return 1;
}

int LuaActiveArea::getAreaName(lua_State* L) {
	String name = realObject->getAreaName();

	lua_pushstring(L, name.toCharArray());

	return 1;
}

// Infinity:  Custom hooks

int LuaActiveArea::getAreaCenterX(lua_State* L) {

	Vector3 center = realObject->getAreaCenter();

	lua_pushnumber(L, center.getX());

	return 1;
}

int LuaActiveArea::getAreaCenterY(lua_State* L) {

	Vector3 center = realObject->getAreaCenter();

	lua_pushnumber(L, center.getY());

	return 1;
}

int LuaActiveArea::getAreaCenterZ(lua_State* L) {

	Vector3 center = realObject->getAreaCenter();

	lua_pushnumber(L, center.getZ());

	return 1;
}

int LuaActiveArea::setAreaName(lua_State* L) {

	if (realObject != nullptr) {

	String areaName = lua_tostring(L, -1);

	Locker locker(realObject);
	realObject->setAreaName(areaName);

	}

	return 0;
}