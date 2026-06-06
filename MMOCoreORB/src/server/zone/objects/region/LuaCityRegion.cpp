
#include "server/zone/objects/region/LuaCityRegion.h"
#include "server/zone/objects/region/CityRegion.h"

const char LuaCityRegion::className[] = "LuaCityRegion";

Luna<LuaCityRegion>::RegType LuaCityRegion::Register[] = {
		{ "_setObject", &LuaCityRegion::_setObject },
		{ "_getObject", &LuaCityRegion::_getObject },
		{ "isClientRegion", &LuaCityRegion::isClientRegion },
		// Infinity:  Custom hooks
		{ "isCityDecoration", &LuaCityRegion::isCityDecoration },
		{ "removeDecoration", &LuaCityRegion::removeDecoration },
		{ "isGCWBaseInsideRadius", &LuaCityRegion::isGCWBaseInsideRadius },
		{ "getRadius", &LuaCityRegion::getRadius },
		{ "addToTreasury", &LuaCityRegion::addToTreasury },
		{ "isCitizen", &LuaCityRegion::isCitizen },
		{ "removeAllBannedPlayers", &LuaCityRegion::removeAllBannedPlayers },
		{ "isMayor", &LuaCityRegion::isMayor },
		{ "getObjectID", &LuaCityRegion::getObjectID },
		{ 0, 0 }
};

LuaCityRegion::LuaCityRegion(lua_State *L) {
	realObject = reinterpret_cast<CityRegion*>(lua_touserdata(L, 1));
}

LuaCityRegion::~LuaCityRegion() {
}

int LuaCityRegion::_getObject(lua_State* L) {
	if (realObject == nullptr)
		lua_pushnil(L);
	else
		lua_pushlightuserdata(L, realObject.get());

	return 1;
}

int LuaCityRegion::_setObject(lua_State* L) {
	realObject = reinterpret_cast<CityRegion*>(lua_touserdata(L, -1));

	return 0;
}

int LuaCityRegion::isClientRegion(lua_State* L) {
	bool val = realObject->isClientRegion();

	lua_pushboolean(L, val);

	return 1;
}

int LuaCityRegion::isCityDecoration(lua_State* L) {

	bool val = false;

	SceneObject* sceneObject = (SceneObject*) lua_touserdata(L, -1);

	if (realObject != nullptr && sceneObject != nullptr)
		val = realObject->isCityDecoration(sceneObject);

	lua_pushboolean(L, val);

	return 1;
}

int LuaCityRegion::removeDecoration(lua_State* L) {

	SceneObject* deco = (SceneObject*) lua_touserdata(L, -1);

	if (realObject != nullptr && deco != nullptr && realObject->isCityDecoration(deco)) {

		Locker locker(realObject);
		Locker clocker(deco, realObject);
		realObject->removeDecoration(deco);
	}

	return 0;
}

int LuaCityRegion::isGCWBaseInsideRadius(lua_State* L) {

	bool val = false;

	int radius = lua_tointeger(L, -1);

	if (realObject != nullptr && radius >= 1) {

		val = realObject->isGCWBaseInsideRadius(radius);
	}

	lua_pushboolean(L, val);

	return 1;
}

int LuaCityRegion::getRadius(lua_State* L) {

	float cityRadius = 0;
	if (realObject != nullptr)
		cityRadius = realObject->getRadius();

	lua_pushnumber(L, cityRadius);

	return 1;
}

int LuaCityRegion::addToTreasury(lua_State* L) {

	double payment = lua_tonumber(L, -1);;

	bool val = false;

	if (realObject != nullptr) {

		Locker locker(realObject);
		val = realObject->addToCityTreasury(payment);
	}		

	lua_pushboolean(L, val);

	return 1;
}

int LuaCityRegion::isCitizen(lua_State* L) {

	CreatureObject* player = (CreatureObject*)lua_touserdata(L, -1);

	uint64 playerid = player->getObjectID();

	bool val = false;

	if (realObject != nullptr) {

		val = realObject->isCitizen(playerid);
	}		

	lua_pushboolean(L, val);

	return 1;
}

int LuaCityRegion::removeAllBannedPlayers(lua_State* L) {

	bool result = false;

	if (realObject != nullptr) {
		Locker locker(realObject);
		realObject->removeAllBannedPlayers();
		result = true;
	}

	lua_pushboolean(L, result);

	return 1;
}

int LuaCityRegion::isMayor(lua_State* L) {
	uint64 playerID = lua_tointeger(L, -1);

	bool val = false;

	if (realObject != nullptr) {
		val = realObject->isMayor(playerID);
	}

	lua_pushboolean(L, val);

	return 1;
}
int LuaCityRegion::getObjectID(lua_State* L) {
	if (realObject != nullptr) {
		lua_pushinteger(L, realObject->getObjectID());
	} else {
		lua_pushinteger(L, 0);
	}
	return 1;
}
