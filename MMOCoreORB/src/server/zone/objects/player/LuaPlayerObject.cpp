/*
 * LuaPlayerObject.cpp
 *
 *  Created on: 13/01/2012
 *      Author: victor
 */

#include "LuaPlayerObject.h"
#include "engine/engine.h"
#include "server/zone/managers/frs/FrsManager.h"
#include "server/zone/managers/crafting/schematicmap/SchematicMap.h"
#include "server/zone/objects/tangible/deed/eventperk/EventPerkDeed.h"
#include "server/zone/objects/tangible/eventperk/Jukebox.h"
#include "server/zone/objects/tangible/eventperk/ShuttleBeacon.h"
#include "server/zone/managers/skill/SkillManager.h"
#include "server/zone/Zone.h"
#include "server/zone/objects/region/CityRegion.h"
#include "server/zone/objects/player/sessions/SlicingSession.h"
#include "server/zone/objects/player/sui/listbox/SuiListBox.h"
#include "server/zone/objects/player/sui/callbacks/EnclaveCouncilRankSuiCallback.h"
#include "server/zone/managers/stringid/StringIdManager.h"
// Infinity:  Custom includes
#include "server/zone/managers/mission/MissionManager.h"
#include "server/zone/managers/visibility/VisibilityManager.h"
#include "server/login/account/Account.h"
#include "server/zone/objects/intangible/PetControlDevice.h"
#include "server/zone/objects/player/events/StoreSpawnedChildrenTask.h"
#include "server/zone/objects/player/events/ClearActivePetsTask.h"

//Infinity:  Custom logging with function name
#define ERROR_F(msg)  Logger::console.error(String(__func__) + " - " + String(msg))

const char LuaPlayerObject::className[] = "LuaPlayerObject";

Luna<LuaPlayerObject>::RegType LuaPlayerObject::Register[] = {
		{ "_setObject", &LuaPlayerObject::_setObject },
		{ "_getObject", &LuaSceneObject::_getObject },
		{ "getFactionStanding", &LuaPlayerObject::getFactionStanding },
		{ "increaseFactionStanding", &LuaPlayerObject::increaseFactionStanding },
		{ "decreaseFactionStanding", &LuaPlayerObject::decreaseFactionStanding },
		{ "setFactionStanding", &LuaPlayerObject::setFactionStanding },
		{ "addWaypoint", &LuaPlayerObject::addWaypoint },
		{ "removeWaypoint", &LuaPlayerObject::removeWaypoint },
		{ "removeWaypointBySpecialType", &LuaPlayerObject::removeWaypointBySpecialType },
		{ "getWaypointAt", &LuaPlayerObject::getWaypointAt },
		{ "updateWaypoint", &LuaPlayerObject::updateWaypoint },
		{ "addRewardedSchematic", &LuaPlayerObject::addRewardedSchematic },
		{ "removeRewardedSchematic", &LuaPlayerObject::removeRewardedSchematic },
		{ "hasSchematic", &LuaPlayerObject::hasSchematic },
		{ "addPermissionGroup", &LuaPlayerObject::addPermissionGroup },
		{ "removePermissionGroup", &LuaPlayerObject::removePermissionGroup },
		{ "hasPermissionGroup", &LuaPlayerObject::hasPermissionGroup },
		{ "awardBadge", &LuaPlayerObject::awardBadge },
		{ "hasBadge", &LuaPlayerObject::hasBadge },
		{ "addHologrindProfession", &LuaPlayerObject::addHologrindProfession },
		{ "getHologrindProfessions", &LuaPlayerObject::getHologrindProfessions },
		{ "getForcePower", &LuaPlayerObject::getForcePower },
		{ "getForcePowerMax", &LuaPlayerObject::getForcePowerMax },
		{ "setForcePower", &LuaPlayerObject::setForcePower },
		{ "isJedi", &LuaPlayerObject::isJedi },
		{ "isJediLight", &LuaPlayerObject::isJediLight },
		{ "isJediDark", &LuaPlayerObject::isJediDark },
		{ "setJediState", &LuaPlayerObject::setJediState },
		{ "getJediState", &LuaPlayerObject::getJediState },
		{ "isOnline", &LuaPlayerObject::isOnline },
		{ "activateJournalQuest", &LuaPlayerObject::activateJournalQuest },
		{ "completeJournalQuest", &LuaPlayerObject::completeJournalQuest },
		{ "clearJournalQuest", &LuaPlayerObject::clearJournalQuest },
		{ "activateJournalQuestTask", &LuaPlayerObject::activateJournalQuestTask },
		{ "completeJournalQuestTask", &LuaPlayerObject::completeJournalQuestTask },
		{ "clearJournalQuestTask", &LuaPlayerObject::clearJournalQuestTask },
		{ "isJournalQuestActive", &LuaPlayerObject::isJournalQuestActive },
		{ "isJournalQuestComplete", &LuaPlayerObject::isJournalQuestComplete },
		{ "isJournalQuestTaskActive", &LuaPlayerObject::isJournalQuestTaskActive },
		{ "isJournalQuestTaskComplete", &LuaPlayerObject::isJournalQuestTaskComplete },
		{ "setActiveQuestsBit", &LuaPlayerObject::setActiveQuestsBit },
		{ "clearActiveQuestsBit", &LuaPlayerObject::clearActiveQuestsBit },
		{ "hasActiveQuestBitSet", &LuaPlayerObject::hasActiveQuestBitSet },
		{ "hasCompletedQuestsBitSet", &LuaPlayerObject::hasCompletedQuestsBitSet },
		{ "setCompletedQuestsBit", &LuaPlayerObject::setCompletedQuestsBit },
		{ "clearCompletedQuestsBit", &LuaPlayerObject::clearCompletedQuestsBit },
		{ "hasAbility", &LuaPlayerObject::hasAbility},
		{ "addAbility", &LuaPlayerObject::addAbility},
		{ "removeAbility", &LuaPlayerObject::removeAbility},
		{ "getExperience", &LuaPlayerObject::getExperience },
		{ "addEventPerk", &LuaPlayerObject::addEventPerk},
		{ "getEventPerkCount", &LuaPlayerObject::getEventPerkCount},
		{ "hasEventPerk", &LuaPlayerObject::hasEventPerk},
		{ "getCharacterAgeInDays", &LuaPlayerObject::getCharacterAgeInDays},
		{ "hasGodMode", &LuaPlayerObject::hasGodMode},
		{ "isPrivileged", &LuaPlayerObject::isPrivileged},
		{ "closeSuiWindowType", &LuaPlayerObject::closeSuiWindowType},
		{ "getExperienceList", &LuaPlayerObject::getExperienceList},
		{ "getExperienceCap", &LuaPlayerObject::getExperienceCap},
		{ "activateQuest", &LuaPlayerObject::activateQuest },
		{ "canActivateQuest", &LuaPlayerObject::canActivateQuest },
		{ "getSuiBox", &LuaPlayerObject::getSuiBox },
		{ "addSuiBox", &LuaPlayerObject::addSuiBox },
		{ "removeSuiBox", &LuaPlayerObject::removeSuiBox },
		{ "isJediTrainer", &LuaPlayerObject::isJediTrainer },
		{ "getVisibility", &LuaPlayerObject::getVisibility },
		{ "setFrsCouncil", &LuaPlayerObject::setFrsCouncil },
		{ "setFrsRank", &LuaPlayerObject::setFrsRank },
		{ "getFrsRank", &LuaPlayerObject::getFrsRank },
		{ "getFrsCouncil", &LuaPlayerObject::getFrsCouncil },
		{ "showCouncilRank", &LuaPlayerObject::showCouncilRank },
		{ "startSlicingSession", &LuaPlayerObject::startSlicingSession },
		{ "setVisibility", &LuaPlayerObject::setVisibility },
		{ "getPlayedTimeString", &LuaPlayerObject::getPlayedTimeString },
		{ "getAccountID", &LuaPlayerObject::getAccountID },
		{ "hasPvpTef", &LuaPlayerObject::hasPvpTef },
		{ "hasGcwTef", &LuaPlayerObject::hasGcwTef },
		{ "getPvpRating", &LuaPlayerObject::getPvpRating },
		// Infinity:  custom hooks below
		{ "getPlayerBounty", &LuaPlayerObject::getPlayerBounty },
		{ "updatePlayerBountyReward", &LuaPlayerObject::updatePlayerBountyReward },
		{ "updateLastCombatActionTimestamp", &LuaPlayerObject::updateLastCombatActionTimestamp },
		{ "getAccountAgeInDays", &LuaPlayerObject::getAccountAgeInDays },
		{ "getMaximumLots", &LuaPlayerObject::getMaximumLots },
		{ "setMaximumLots", &LuaPlayerObject::setMaximumLots },
		{ "storePets", &LuaPlayerObject::storePets },
		{ "getLotsRemaining", &LuaPlayerObject::getLotsRemaining },
		{ "hasBhTef", &LuaPlayerObject::hasBhTef },
		{ "isAdmin", &LuaPlayerObject::isAdmin },
		{ "hasActivePets", &LuaPlayerObject::hasActivePets },
		{ "removeFromFrs", &LuaPlayerObject::removeFromFrs },
		{ "getServerMovementTimeDelta", &LuaPlayerObject::getServerMovementTimeDelta },
		{ "getActivePetsSize", &LuaPlayerObject::getActivePetsSize },
		{ "getActivePet", &LuaPlayerObject::getActivePet },
		{ "removeTefs", &LuaPlayerObject::removeTefs },
		{ "hasTef", &LuaPlayerObject::hasTef },
		{ "clearActivePets", &LuaPlayerObject::clearActivePets },
		{ "getStarterProfession", &LuaPlayerObject::getStarterProfession },
		{ "getTotalOwnedStructureCount", &LuaPlayerObject::getTotalOwnedStructureCount },
        { "getOwnedStructure", &LuaPlayerObject::getOwnedStructure },
        { "removeOwnedStructure", &LuaPlayerObject::removeOwnedStructure },
        { "getSecondsSinceLastPvp", &LuaPlayerObject::getSecondsSinceLastPvp },
		{ "hasDwTef", &LuaPlayerObject::hasDwTef },
		{ "updateDwTef", &LuaPlayerObject::updateDwTef },
		{ "getDwFaction", &LuaPlayerObject::getDwFaction },
		{ "setDwFaction", &LuaPlayerObject::setDwFaction },
		{ "setTitle", &LuaPlayerObject::setTitle },
		{ "getTitle", &LuaPlayerObject::getTitle },
		{ "resetPilotTier", &LuaPlayerObject::resetPilotTier },
		{ "setPilotTier", &LuaPlayerObject::setPilotTier },
		{ "getPilotTier", &LuaPlayerObject::getPilotTier },
		{ "isSquadronType", &LuaPlayerObject::isSquadronType },
		{ "setSquadronType", &LuaPlayerObject::setSquadronType },
		{ "getSquadronType", &LuaPlayerObject::getSquadronType },
		{ "addDroidCommand", &LuaPlayerObject::addDroidCommand },
		{ "removeDroidCommands", &LuaPlayerObject::removeDroidCommands },
		{ 0, 0 }
};

LuaPlayerObject::LuaPlayerObject(lua_State* L) : LuaIntangibleObject(L) {
#ifdef DYNAMIC_CAST_LUAOBJECTS
	realObject = dynamic_cast<PlayerObject*>(_getRealSceneObject());
	// Infinity:  No need to crash server with assertion, just throw a Lua error and return
	if (_getRealSceneObject() != nullptr && realObject == nullptr) {
		Logger::console.error(LuaSceneObject::getLuaCallStack(L, String(__func__) + " - Detected assertion failure condition."));
		return;
	}

	// E3_ASSERT(!_getRealSceneObject() || realObject != nullptr);
#else
	realObject = reinterpret_cast<PlayerObject*>(lua_touserdata(L, 1));
#endif
}

LuaPlayerObject::~LuaPlayerObject() {
}

int LuaPlayerObject::_setObject(lua_State* L) {
	LuaIntangibleObject::_setObject(L);

#ifdef DYNAMIC_CAST_LUAOBJECTS
	realObject = dynamic_cast<PlayerObject*>(_getRealSceneObject());

	// Infinity:  No need to crash server with assertion, just throw a Lua error and return
	if (_getRealSceneObject() != nullptr && realObject == nullptr) {
		String msg = "LuaPlayerObject::_setObject - Detected assertion failure condition." + LuaSceneObject::getLuaCallStack(L);
		return luaL_error(L, msg.toCharArray());
	}

	// E3_ASSERT(!_getRealSceneObject() || realObject != nullptr);
#else
	realObject = (PlayerObject*)lua_touserdata(L, -1);
#endif

	return 0;
}

int LuaPlayerObject::getFactionStanding(lua_State* L) {
	//String faction = lua_tostring(L, -1);

	const char* str = lua_tostring(L, -1);

	String faction(str);

	float standing = realObject->getFactionStanding(faction);

	lua_pushnumber(L, standing);

	return 1;
}

int LuaPlayerObject::increaseFactionStanding(lua_State* L) {
	float val = lua_tonumber(L, -1);
	const char* str = lua_tostring(L, -2);

	realObject->increaseFactionStanding(str, val);

	return 0;
}

int LuaPlayerObject::decreaseFactionStanding(lua_State* L) {
	float val = lua_tonumber(L, -1);
	const char* str = lua_tostring(L, -2);

	realObject->decreaseFactionStanding(str, val);

	return 0;
}

int LuaPlayerObject::setFactionStanding(lua_State* L) {
	float val = lua_tonumber(L, -1);
	const char* str = lua_tostring(L, -2);

	realObject->setFactionStanding(str, val);

	return 0;
}

//addWaypoint(planet, name, desc, x, z, y, color, active, notifyClient, specialTypeID [, persistence])
int LuaPlayerObject::addWaypoint(lua_State* L) {
	int numberOfArguments = lua_gettop(L) - 1;

	if (numberOfArguments != 10 && numberOfArguments != 11) {
		realObject->error() << "Improper number of arguments in LuaPlayerObject::addWaypoint.";
		return 0;
	}

	String planet, customName, desc;
	float x, z, y;
	int color, persistence = 1, specialTypeID;
	bool active, notifyClient;

	if (numberOfArguments == 10) {
		planet = lua_tostring(L, -10);
		customName = lua_tostring(L, -9);
		desc = lua_tostring(L, -8);
		x = lua_tonumber(L, -7);
		z = lua_tonumber(L, -6);
		y = lua_tonumber(L, -5);
		color = lua_tointeger(L, -4);
		active = lua_toboolean(L, -3);
		notifyClient = lua_toboolean(L, -2);
		specialTypeID = lua_tointeger(L, -1);
	} else {
		planet = lua_tostring(L, -11);
		customName = lua_tostring(L, -10);
		desc = lua_tostring(L, -9);
		x = lua_tonumber(L, -8);
		z = lua_tonumber(L, -7);
		y = lua_tonumber(L, -6);
		color = lua_tointeger(L, -5);
		active = lua_toboolean(L, -4);
		notifyClient = lua_toboolean(L, -3);
		specialTypeID = lua_tointeger(L, -2);
		persistence = lua_tonumber(L, -1);
	}

	ManagedReference<WaypointObject*> waypoint = realObject->getZoneServer()->createObject(0xc456e788, persistence).castTo<WaypointObject*>();
	Locker locker(waypoint);

	waypoint->setPlanetCRC(planet.hashCode());
	waypoint->setPosition(x, z, y);
	waypoint->setSpecialTypeID(specialTypeID);
	waypoint->setCustomObjectName(customName, false);
	waypoint->setColor(color);
	waypoint->setActive(active);

	if (!desc.isEmpty()) {
		waypoint->setDetailedDescription(desc);
	}

	realObject->addWaypoint(waypoint, false, notifyClient);

	lua_pushinteger(L, waypoint->getObjectID());

	return 1;
}

int LuaPlayerObject::removeWaypoint(lua_State* L) {
	unsigned long long int waypointID = lua_tointeger(L, -2);
	bool notifyClient = lua_toboolean(L, -1);

	realObject->removeWaypoint(waypointID, notifyClient);

	return 0;
}

int LuaPlayerObject::removeWaypointBySpecialType(lua_State* L) {
	int specialTypeID = lua_tointeger(L, -1);

	realObject->removeWaypointBySpecialType(specialTypeID);

	return 0;
}

int LuaPlayerObject::getWaypointAt(lua_State* L) {
	float x = lua_tonumber(L, -3);
	float y = lua_tonumber(L, -2);
	String planet = lua_tostring(L, -1);

	WaypointObject* waypoint = realObject->getWaypointAt(x, y, planet);

	if (waypoint != nullptr)
		lua_pushlightuserdata(L, waypoint);
	else
		lua_pushnil(L);

	return 1;
}

int LuaPlayerObject::updateWaypoint(lua_State* L) {
	unsigned long long int waypointID = lua_tointeger(L, -1);

	realObject->updateWaypoint(waypointID);

	return 0;
}

int LuaPlayerObject::addRewardedSchematic(lua_State* L) {
	if (!lua_isstring(L, -4)) {
		ERROR_F("templateString must be a string");
		lua_pushboolean(L, false);
		return 1;
	}

	String templateString = lua_tostring(L, -4);

	if (templateString.isEmpty()) {
		ERROR_F("templateString cannot be empty");
		lua_pushboolean(L, false);
		return 1;
	}

	short type = lua_tointeger(L, -3);
	int quantity = lua_tointeger(L, -2);
	bool notifyClient = lua_toboolean(L, -1);

	DraftSchematic* schematic = SchematicMap::instance()->get(templateString.hashCode());

	if (schematic == nullptr) {
		lua_pushboolean(L, false);
		return 1;
	}

	bool result = realObject->addRewardedSchematic(schematic, type, quantity, notifyClient);

	lua_pushboolean(L, result);

	return 1;
}

int LuaPlayerObject::hasSchematic(lua_State* L) {
	if (!lua_isstring(L, -1)) {
		ERROR_F("templateString must be a string");
		lua_pushboolean(L, false);
		return 1;
	}

	String templateString = lua_tostring(L, -1);

	if (templateString.isEmpty()) {
		ERROR_F("templateString cannot be empty");
		lua_pushboolean(L, false);
		return 1;
	}

	DraftSchematic* schematic = SchematicMap::instance()->get(templateString.hashCode());

	if (schematic == nullptr) {
		lua_pushboolean(L, false);
		return 1;
	}

	lua_pushboolean(L, realObject->hasSchematic(schematic));

	return 1;
}

int LuaPlayerObject::removeRewardedSchematic(lua_State* L) {
	if (!lua_isstring(L, -2)) {
		ERROR_F("templateString must be a string");
		lua_pushboolean(L, false);
		return 1;
	}

	String templateString = lua_tostring(L, -2);

	if (templateString.isEmpty()) {
		ERROR_F("templateString cannot be empty");
		lua_pushboolean(L, false);
		return 1;
	}

	bool notifyClient = lua_toboolean(L, -1);

	DraftSchematic* schematic = SchematicMap::instance()->get(templateString.hashCode());

	if (schematic != nullptr) {
		realObject->removeRewardedSchematic(schematic, notifyClient);
	}

	return 0;
}

int LuaPlayerObject::addPermissionGroup(lua_State* L){

	int numberOfArguments = lua_gettop(L) - 1;

	if (numberOfArguments != 2) {
		ERROR_F("incorrect number of arguments.");
		return 0;
	}

	String permissionGroup = lua_tostring(L, -2);
	bool updateBuildings = lua_toboolean(L, -1);

	if (!realObject->hasPermissionGroup(permissionGroup)) {	//Infinity:  only add it if they don't already have it
		realObject->addPermissionGroup(permissionGroup, updateBuildings);
	}

	return 0;
}

int LuaPlayerObject::removePermissionGroup(lua_State* L) {

	int numberOfArguments = lua_gettop(L) - 1;

	if (numberOfArguments != 2) {
		ERROR_F("incorrect number of arguments.");
		return 0;
	}

	String permissionGroup = lua_tostring(L, -2);
	bool updateBuildings = lua_toboolean(L, -1);

	realObject->removePermissionGroup(permissionGroup, updateBuildings);

	return 0;
}

int LuaPlayerObject::hasPermissionGroup(lua_State* L){
	String permissionGroup = lua_tostring(L, -1);

	lua_pushboolean(L, realObject->hasPermissionGroup(permissionGroup));

	return 1;
}

int LuaPlayerObject::awardBadge(lua_State* L){
	int badgeId = lua_tointeger(L, -1);

	realObject->awardBadge(badgeId);

	return 0;
}

int LuaPlayerObject::hasBadge(lua_State* L){
	int badgeId = lua_tointeger(L, -1);

	lua_pushboolean(L, realObject->hasBadge(badgeId));

	return 1;
}

int LuaPlayerObject::addHologrindProfession(lua_State* L){
	byte profession = lua_tointeger(L, -1);

	realObject->addHologrindProfession(profession);

	return 0;
}

int LuaPlayerObject::getHologrindProfessions(lua_State* L) {
	const Vector<byte>* professions = realObject->getHologrindProfessions();

	lua_newtable(L);

	for (int i = 0; i < professions->size(); i++) {
		lua_pushnumber(L, professions->get(i));

		lua_rawseti(L, -2, i + 1);
	}

	return 1;
}

int LuaPlayerObject::getForcePower(lua_State* L) {
	lua_pushinteger(L, realObject->getForcePower());

	return 1;
}

int LuaPlayerObject::getForcePowerMax(lua_State* L) {
	lua_pushinteger(L, realObject->getForcePowerMax());

	return 1;
}

int LuaPlayerObject::setForcePower(lua_State* L) {
	int forcePower = lua_tointeger(L, -1);

	realObject->setForcePower(forcePower, true);

	return 0;
}

int LuaPlayerObject::isJedi(lua_State* L) {
	lua_pushboolean(L, realObject->isJedi());

	return 1;
}

int LuaPlayerObject::isJediLight(lua_State* L) {
	lua_pushboolean(L, realObject->isJediLight());

	return 1;
}

int LuaPlayerObject::isJediDark(lua_State* L) {
	lua_pushboolean(L, realObject->isJediDark());

	return 1;
}

int LuaPlayerObject::setJediState(lua_State* L) {
	int jediState = lua_tointeger(L, -1);

	realObject->setJediState(jediState);

	return 0;
}

int LuaPlayerObject::getJediState(lua_State* L) {
	lua_pushinteger(L, realObject->getJediState());

	return 1;
}

int LuaPlayerObject::isOnline(lua_State* L) {
	lua_pushboolean(L, realObject->isOnline());

	return 1;
}

int LuaPlayerObject::activateJournalQuest(lua_State* L) {
	int questCrc = lua_tointeger(L, -2);
	bool notify = lua_toboolean(L, -1);

	Locker locker(realObject);

	realObject->activateJournalQuest(questCrc, notify);

	return 0;
}

int LuaPlayerObject::completeJournalQuest(lua_State* L) {
	int questCrc = lua_tointeger(L, -2);
	bool notify = lua_toboolean(L, -1);

	Locker locker(realObject);

	realObject->completeJournalQuest(questCrc, notify);

	return 0;
}

int LuaPlayerObject::clearJournalQuest(lua_State* L) {
	int questCrc = lua_tointeger(L, -2);
	bool notify = lua_toboolean(L, -1);

	Locker locker(realObject);

	realObject->clearJournalQuest(questCrc, notify);

	return 0;
}

int LuaPlayerObject::activateJournalQuestTask(lua_State* L) {
	int questCrc = lua_tointeger(L, -3);
	int task = lua_tointeger(L, -2);
	bool notify = lua_toboolean(L, -1);

	Locker locker(realObject);

	realObject->activateJournalQuestTask(questCrc, task, notify);

	return 0;
}

int LuaPlayerObject::completeJournalQuestTask(lua_State* L) {
	int questCrc = lua_tointeger(L, -3);
	int task = lua_tointeger(L, -2);
	bool notify = lua_toboolean(L, -1);

	Locker locker(realObject);

	realObject->completeJournalQuestTask(questCrc, task, notify);

	return 0;
}

int LuaPlayerObject::clearJournalQuestTask(lua_State* L) {
	int questCrc = lua_tointeger(L, -3);
	int task = lua_tointeger(L, -2);
	bool notify = lua_toboolean(L, -1);

	Locker locker(realObject);

	realObject->clearJournalQuestTask(questCrc, task, notify);

	return 0;
}

int LuaPlayerObject::isJournalQuestActive(lua_State* L) {
	int questCrc = lua_tointeger(L, -1);

	lua_pushboolean(L, realObject->isJournalQuestActive(questCrc));

	return 1;
}

int LuaPlayerObject::isJournalQuestComplete(lua_State* L) {
	int questCrc = lua_tointeger(L, -1);

	lua_pushboolean(L, realObject->isJournalQuestComplete(questCrc));

	return 1;
}

int LuaPlayerObject::isJournalQuestTaskActive(lua_State* L) {
	int questCrc = lua_tointeger(L, -2);
	int task = lua_tointeger(L, -1);

	lua_pushboolean(L, realObject->isJournalQuestTaskActive(questCrc, task));

	return 1;
}

int LuaPlayerObject::isJournalQuestTaskComplete(lua_State* L) {
	int questCrc = lua_tointeger(L, -2);
	int task = lua_tointeger(L, -1);

	lua_pushboolean(L, realObject->isJournalQuestTaskComplete(questCrc, task));

	return 1;
}

int LuaPlayerObject::setActiveQuestsBit(lua_State* L) {
	int quest = lua_tointeger(L, -2);
	byte value = lua_tointeger(L, -1);

	realObject->setActiveQuestsBit(quest, value, true);

	return 0;
}

int LuaPlayerObject::clearActiveQuestsBit(lua_State* L) {
	int quest = lua_tointeger(L, -1);

	realObject->clearActiveQuestsBit(quest, true);

	return 0;
}

int LuaPlayerObject::hasActiveQuestBitSet(lua_State* L) {
	int quest = lua_tointeger(L, -1);

	lua_pushboolean(L, realObject->hasActiveQuestBitSet(quest));

	return 1;
}

int LuaPlayerObject::hasCompletedQuestsBitSet(lua_State* L) {
	int quest = lua_tointeger(L, -1);

	lua_pushboolean(L, realObject->hasCompletedQuestsBitSet(quest));

	return 1;
}

int LuaPlayerObject::setCompletedQuestsBit(lua_State* L) {
	int quest = lua_tointeger(L, -2);
	byte value = lua_tointeger(L, -1);

	realObject->setCompletedQuestsBit(quest, value, true);

	return 0;
}

int LuaPlayerObject::clearCompletedQuestsBit(lua_State* L) {
	int quest = lua_tointeger(L, -1);

	realObject->clearCompletedQuestsBit(quest, true);

	return 0;
}

int LuaPlayerObject::activateQuest(lua_State* L) {
	int quest = lua_tointeger(L, -1);

	realObject->activateQuest(quest);

	return 0;
}

int LuaPlayerObject::canActivateQuest(lua_State* L) {
	int quest = lua_tointeger(L, -1);

	lua_pushboolean(L, realObject->canActivateQuest(quest));

	return 1;
}


int LuaPlayerObject::hasAbility(lua_State* L) {
	String value = lua_tostring(L, -1);

	bool check = realObject->hasAbility(value);

	lua_pushboolean(L, check);

	return 1;
}

int LuaPlayerObject::addAbility(lua_State* L) {
	String value = lua_tostring(L, -1);

	SkillManager* skillManager = SkillManager::instance();

	if (!realObject->hasAbility(value))
		skillManager->addAbility(realObject, value);

	return 1;
}

int LuaPlayerObject::removeAbility(lua_State* L) {
	String value = lua_tostring(L, -1);

	SkillManager* skillManager = SkillManager::instance();

	Locker locker(realObject);

	if (realObject->hasAbility(value))
		skillManager->removeAbility(realObject, value);

	return 1;
}

int LuaPlayerObject::getExperience(lua_State* L) {
	String type = lua_tostring(L, -1);

	lua_pushinteger(L, realObject->getExperience(type));

	return 1;
}

int LuaPlayerObject::getEventPerkCount(lua_State* L) {
	lua_pushinteger(L, realObject->getEventPerkCount());

	return 1;
}

int LuaPlayerObject::hasEventPerk(lua_State* L) {
	String templateString = lua_tostring(L, -1);

	lua_pushboolean(L, realObject->hasEventPerk(templateString));

	return 1;
}

int LuaPlayerObject::addEventPerk(lua_State* L) {
	SceneObject* item = (SceneObject*) lua_touserdata(L, -1);

	if (item == nullptr) {
		return 0;
	}

	Locker locker(item);

	ManagedReference<CreatureObject*> creature = dynamic_cast<CreatureObject*>(realObject->getParent().get().get());

	if (creature != nullptr) {
		if (item->isEventPerkDeed()) {
			EventPerkDeed* deed = cast<EventPerkDeed*>(item);
			deed->setOwner(creature);
		} else if (item->isEventPerkItem()) {
			if (item->getServerObjectCRC() == 0x46BD798B) { // Jukebox
				Jukebox* jbox = cast<Jukebox*>(item);

				if (jbox != nullptr)
					jbox->setOwner(creature);
			} else if (item->getServerObjectCRC() == 0x255F612C) { // Shuttle Beacon
				ShuttleBeacon* beacon = cast<ShuttleBeacon*>(item);

				if (beacon != nullptr)
					beacon->setOwner(creature);
			}
		}
	}

	realObject->addEventPerk(item);

	return 0;
}

int LuaPlayerObject::getCharacterAgeInDays(lua_State* L) {
	lua_pushinteger(L, realObject->getCharacterAgeInDays());

	return 1;
}

int LuaPlayerObject::hasGodMode(lua_State* L) {
	lua_pushboolean(L, realObject->hasGodMode());

	return 1;
}

int LuaPlayerObject::isPrivileged(lua_State* L) {
	lua_pushboolean(L, realObject->isPrivileged());

	return 1;
}

int LuaPlayerObject::closeSuiWindowType(lua_State* L) {
	int type = lua_tointeger(L, -1);
	unsigned suiType = (unsigned)type;

	realObject->closeSuiWindowType( suiType );

	return 0;
}

int LuaPlayerObject::getExperienceList(lua_State* L) {
	const DeltaVectorMap<String, int>* expList = realObject->getExperienceList();

	lua_newtable(L);

	for (int i = 0; i < expList->size(); i++) {
		const auto& value = expList->getKeyAt(i);

		lua_pushstring(L, value.toCharArray());
		lua_rawseti(L, -2, i + 1);
	}

	return 1;
}

int LuaPlayerObject::getExperienceCap(lua_State* L) {
	String type = lua_tostring(L, -1);
	lua_pushinteger(L, realObject->getXpCap(type));

	return 1;
}

int LuaPlayerObject::getSuiBox(lua_State* L) {
	uint32 pageId = lua_tointeger(L, -1);
	Reference<SuiBox*> object = realObject->getSuiBox(pageId);

	if (object == nullptr) {
		lua_pushnil(L);
	} else {
		lua_pushlightuserdata(L, object.get());
		object->_setUpdated(true); //mark updated so the GC doesnt delete it while in LUA
	}

	return 1;
}

int LuaPlayerObject::addSuiBox(lua_State* L) {
	Reference<SuiBox*> box = (SuiBox*) lua_touserdata(L, -1);

	if (box == nullptr)
		return 0;

	realObject->addSuiBox(box);

	return 0;
}

int LuaPlayerObject::removeSuiBox(lua_State* L) {
	uint32 pageId = lua_tointeger(L, -1);
	realObject->removeSuiBox(pageId, true);

	return 1;
}


int LuaPlayerObject::isJediTrainer(lua_State* L) {
	CreatureObject* trainer = (CreatureObject*)lua_touserdata(L, -1);

	Vector3 npc(trainer->getWorldPositionX(), trainer->getWorldPositionY(), 0);
	Vector3 playerCoord = realObject->getJediTrainerCoordinates();
	Vector3 player(playerCoord.getX(), playerCoord.getY(), 0);

	bool result = (npc == player) && (realObject->getTrainerZoneName() == trainer->getZone()->getZoneName());

	lua_pushboolean(L, result);

	return 1;
}

int LuaPlayerObject::getVisibility(lua_State* L) {

	lua_pushnumber(L, realObject->getVisibility());

	return 1;
}

int LuaPlayerObject::setFrsCouncil(lua_State* L) {
	int councilType = lua_tointeger(L, -1);

	FrsData* frsData = realObject->getFrsData();

	frsData->setCouncilType(councilType);

	return 0;
}

int LuaPlayerObject::setVisibility(lua_State* L) {
	
	if (realObject == nullptr) {
		return 0;
	}

	float visValue = lua_tonumber(L, -1);

	if (visValue < 0)
		visValue = 0;

	if (visValue > 8000)
		visValue = 8000;

	Locker locker(realObject);

	realObject->setVisibility(visValue);

	locker.release();

	//Infinity:  Let's add to visibility list and bounty list if not already there
	ManagedReference<SceneObject*> parent = realObject->getParent().get();
	if (parent != nullptr) {

		ManagedReference<CreatureObject*> playerCreature = parent->asCreatureObject();
		if (playerCreature != nullptr) {

			//Add player to visibility list
			VisibilityManager::instance()->addToVisibilityList(playerCreature);

			auto zoneServer = realObject->getZoneServer();
			if (zoneServer != nullptr) {
				auto missionManager = zoneServer->getMissionManager();
				if (missionManager != nullptr) {

					uint64 oid = playerCreature->getObjectID();

					if (!missionManager->hasPlayerBountyTargetInList(oid)) {
						missionManager->addPlayerToBountyList(oid, realObject->calculateBhReward());	
					}
				}
			}
		}
	}
	
	return 0;
}

int LuaPlayerObject::setFrsRank(lua_State* L) {

	int rank = 0;
	bool adjustXP = false;

	if (realObject == nullptr) {
		ERROR_F("realObject is nullptr");
		return 0;
	}

	int numberOfArguments = lua_gettop(L) - 1;

	if (numberOfArguments == 1) {
		rank = lua_tointeger(L, -1);
	}
	else if (numberOfArguments == 2) {
		rank = lua_tointeger(L, -2);
		adjustXP = lua_toboolean(L, -1);
	}
	else {
		ERROR_F("incorrect number of arguments.");
		return 0;
	}

	auto zoneServer = realObject->getZoneServer();
	if (zoneServer == nullptr) {
		ERROR_F("zoneServer is nullptr");
		return 0;
	}

	auto frsManager = zoneServer->getFrsManager();
	if (frsManager == nullptr) {
		ERROR_F("frsManager is nullptr");
		return 0;
	}

	ManagedReference<CreatureObject*> player = realObject->getParentRecursively(SceneObjectType::PLAYERCREATURE).castTo<CreatureObject*>();

	if (frsManager != nullptr && player != nullptr) {
		Locker locker(player);

		frsManager->setPlayerRank(player, rank, "lua");

		if (adjustXP) {  //Adjust experience to match rank

			FrsData* frsData = realObject->getFrsData();
			int councilType = frsData->getCouncilType();
			int curExperience = realObject->getExperience("force_rank_xp");
			int requiredExperience = frsManager->getRequiredXP(councilType, rank);

			if (curExperience >= requiredExperience) {
				return 0;
			}
			else {
				frsManager->adjustFrsExperience(player, requiredExperience - curExperience, true);
			}
		}		
	}

	return 0;
}

int LuaPlayerObject::getFrsRank(lua_State* L) {
	FrsData* frsData = realObject->getFrsData();

	lua_pushinteger(L, frsData->getRank());

	return 1;
}

int LuaPlayerObject::getFrsCouncil(lua_State* L) {
	FrsData* frsData = realObject->getFrsData();

	lua_pushinteger(L, frsData->getCouncilType());

	return 1;
}

int LuaPlayerObject::showCouncilRank(lua_State* L) {
	int council = lua_tointeger(L, -1);

	ManagedReference<CreatureObject*> player = realObject->getParentRecursively(SceneObjectType::PLAYERCREATURE).castTo<CreatureObject*>();

	if (player == nullptr)
		return 0;

	auto zoneServer = player->getZoneServer();

	if (zoneServer == nullptr)
		return 0;

	Locker lock(realObject);

	ManagedReference<SuiListBox*> box = new SuiListBox(player, SuiWindowType::ENCLAVE_VOTING, SuiListBox::HANDLETWOBUTTON);

	box->setCallback(new EnclaveCouncilRankSuiCallback(zoneServer, council));
	box->setPromptText("Select the rank whose members you wish to view.");
	box->setPromptTitle("@force_rank:rank_selection"); // Rank Selection
	box->setUsingObject(player);
	box->setOkButton(true, "@ok");
	box->setCancelButton(true, "@cancel");

	for (int i = 1; i < 12; i++) {
		String stfRank = "@force_rank:rank" + String::valueOf(i);
		String rankString = StringIdManager::instance()->getStringId(stfRank.hashCode()).toString();
		box->addMenuItem(rankString);
	}

	realObject->addSuiBox(box);
	player->sendMessage(box->generateMessage());

	return 0;
}

int LuaPlayerObject::startSlicingSession(lua_State* L) {
	TangibleObject* objToSlice = (TangibleObject*) lua_touserdata(L, -2);
	bool isKeypadSlice = lua_toboolean(L, -1);

	if (objToSlice == nullptr)
		return 0;

	ManagedReference<CreatureObject*> player = realObject->getParentRecursively(SceneObjectType::PLAYERCREATURE).castTo<CreatureObject*>();

	if (player == nullptr)
		return 0;

	if (player->containsActiveSession(SessionFacadeType::SLICING)) {
		player->sendSystemMessage("@slicing/slicing:already_slicing");
		return 0;
	}

	//Create Session
	ManagedReference<SlicingSession*> session = new SlicingSession(player);
	session->setKeypadSlice(isKeypadSlice);
	session->initalizeSlicingMenu(player, objToSlice);

	return 0;
}

int LuaPlayerObject::getPlayedTimeString(lua_State* L) {
	int argc = lua_gettop(L) - 1;

	bool verbose = false;

	if (argc == 1) {
		verbose = lua_toboolean(L, -1);
	}

	Locker locker(realObject);

	lua_pushstring(L, realObject->getPlayedTimeString(verbose).toCharArray());

	return 1;
}

int LuaPlayerObject::getAccountID(lua_State* L) {
	Locker locker(realObject);

	lua_pushinteger(L, realObject->getAccountID());

	return 1;
}

int LuaPlayerObject::hasPvpTef(lua_State* L) {
	lua_pushboolean(L, realObject->hasPvpTef());

	return 1;
}

int LuaPlayerObject::hasGcwTef(lua_State* L) {
	lua_pushboolean(L, realObject->hasGcwTef());

	return 1;
}

int LuaPlayerObject::getPvpRating(lua_State* L) {
	lua_pushinteger(L, realObject->getPvpRating());

	return 1;
}

// Infinity:  Custom hooks below
int LuaPlayerObject::getPlayerBounty(lua_State* L) {

	int currentBounty = 0;

	if (realObject != nullptr) {

		MissionManager* missionManager = realObject->getZoneServer()->getMissionManager();

		if (missionManager != nullptr) {
			ManagedReference<CreatureObject*> creo = dynamic_cast<CreatureObject*>(realObject->getParent().get().get());
			if (creo != nullptr) {
				uint64 oid = creo->getObjectID();
				currentBounty = missionManager->getPlayerBounty(oid);
			}
		}
	}

	lua_pushinteger(L, currentBounty);

	return 1;
}

int LuaPlayerObject::updatePlayerBountyReward(lua_State* L) {

	int newBounty = lua_tointeger(L, -1);

	if (realObject != nullptr && newBounty > 0) {

		auto zoneServer = realObject->getZoneServer();
		if (zoneServer != nullptr) {

			MissionManager* missionManager = zoneServer->getMissionManager();

			if (missionManager != nullptr) {
				ManagedReference<CreatureObject*> creo = dynamic_cast<CreatureObject*>(realObject->getParent().get().get());
				if (creo != nullptr) {
					uint64 oid = creo->getObjectID();

					if (!missionManager->hasPlayerBountyTargetInList(oid))
						missionManager->addPlayerToBountyList(oid, newBounty);
					else {
						missionManager->updatePlayerBountyReward(oid, newBounty);
						missionManager->updatePlayerBountyOnlineStatus(oid, true);
					}
				}
			}

		}
	}

	return 0;
}

int LuaPlayerObject::updateLastCombatActionTimestamp(lua_State* L) {

	int numberOfArguments = lua_gettop(L) - 1;

	if (numberOfArguments != 3) {
		ERROR_F("incorrect number of arguments.  Three are required.");
		return 0;
	}

	bool updateGcwCrackdownAction = lua_toboolean(L, -3);
	bool updateGcwAction = lua_toboolean(L, -2);
	bool updateBhAction  = lua_toboolean(L, -1);

	if (realObject != nullptr) {

		if (updateGcwCrackdownAction || updateGcwAction || updateBhAction) {
			realObject->updateLastCombatActionTimestamp(updateGcwCrackdownAction, updateGcwAction, updateBhAction);
		}
	}
	else {
		ERROR_F("realObject is nullptr");
	}

	return 0;
}

int LuaPlayerObject::getAccountAgeInDays(lua_State* L) {

	if (realObject == nullptr) {
		lua_pushnil(L);
		return 1;
	}

	ManagedReference<Account*> account = realObject->getAccount();

	if (account == nullptr) {
		lua_pushnil(L);
		return 1;
	}

	lua_pushinteger(L, account->getAgeInDays());

	return 1;
}

int LuaPlayerObject::getMaximumLots(lua_State* L) {

	if (realObject != nullptr) {
		lua_pushinteger(L, realObject->getMaximumLots());	
	}
	else {
		ERROR_F("realObject is nullptr");
		lua_pushnil(L);
	}
	return 1;
}

int LuaPlayerObject::setMaximumLots(lua_State* L) {

	int8 maxLots = lua_tointeger(L, -1);

	if (maxLots < 0) {
		ERROR_F("invalid value for maximum lots");
		return 0;
	}

	if (realObject != nullptr) {
		realObject->setMaximumLots(maxLots);
	}
	else {
		ERROR_F("realObject is nullptr");
	}
	return 0;
}

int LuaPlayerObject::storePets(lua_State* L) {

	if (realObject != nullptr) {

		ManagedReference<CreatureObject*> player = dynamic_cast<CreatureObject*>(realObject->getParent().get().get());

		if (player == nullptr) {
			ERROR_F("playerCreo is nullptr");
			return 0;
		}

		Vector<ManagedReference<ControlDevice*>> petsToStore;

		for (int i = 0; i < realObject->getActivePetsSize(); i++) {
			ManagedReference<AiAgent*> pet = realObject->getActivePet(i);

			if (pet != nullptr) {

				ManagedReference<PetControlDevice*> controlDevice = pet->getControlDevice().get().castTo<PetControlDevice*>();

				if (controlDevice != nullptr) {
					petsToStore.add(controlDevice.castTo<ControlDevice*>());
				}
			}
		}

		Reference<StoreSpawnedChildrenTask*> task = new StoreSpawnedChildrenTask(player, std::move(petsToStore));
		task->execute();

	}
	else {
		ERROR_F("realObject is nullptr");
	}
	return 0;
}

int LuaPlayerObject::getLotsRemaining(lua_State* L) {

	if (realObject != nullptr) {
		lua_pushinteger(L, realObject->getLotsRemaining());	
	}
	else {
		ERROR_F("realObject is nullptr");
		lua_pushnil(L);
	}
	
	return 1;
}

int LuaPlayerObject::hasBhTef(lua_State* L) {
	lua_pushboolean(L, realObject->hasBhTef());

	return 1;
}

int LuaPlayerObject::isAdmin(lua_State* L) {
	lua_pushboolean(L, realObject->isAdmin());

	return 1;
}

int LuaPlayerObject::hasActivePets(lua_State* L) {

	if (realObject != nullptr) {
		lua_pushboolean(L, realObject->getActivePetsSize() > 0);
	}
	else {
		ERROR_F("realObject is nullptr");
		lua_pushnil(L);
	}
	return 1;
}

int LuaPlayerObject::removeFromFrs(lua_State* L) {

	if (realObject == nullptr) {
		ERROR_F("realObject is nullptr");
		return 0;
	}

	auto zoneServer = realObject->getZoneServer();
	if (zoneServer == nullptr) {
		ERROR_F("zoneServer is nullptr");
		return 0;
	}

	auto frsManager = zoneServer->getFrsManager();
	if (frsManager == nullptr) {
		ERROR_F("frsManager is nullptr");
		return 0;
	}

	ManagedReference<CreatureObject*> player = realObject->getParentRecursively(SceneObjectType::PLAYERCREATURE).castTo<CreatureObject*>();

	if (frsManager != nullptr && player != nullptr) {
		Locker locker(player);

		frsManager->removeFromFrs(player, "admin");
	}

	return 0;
}

int LuaPlayerObject::getServerMovementTimeDelta(lua_State* L) {

	unsigned long delta = 0;

	if (realObject != nullptr) {
		delta = realObject->getServerMovementTimeDelta();
	}

	lua_pushinteger(L, delta);

	return 1;
}

int LuaPlayerObject::getActivePetsSize(lua_State* L) {

	if (realObject != nullptr) {
		lua_pushinteger(L, realObject->getActivePetsSize());
	}
	else {
		ERROR_F("realObject is nullptr");
		lua_pushnil(L);
	}
	return 1;
}

int LuaPlayerObject::getActivePet(lua_State* L) {
	if (realObject != nullptr) {
		int index = lua_tointeger(L, -1);

		if (index >= 0 && index < realObject->getActivePetsSize()) {
			lua_pushlightuserdata(L, realObject->getActivePet(index));
		} else {
			lua_pushnil(L);
		}
	} else {
		ERROR_F("realObject is nullptr");
		lua_pushnil(L);
	}
	return 1;
}

int LuaPlayerObject::removeTefs(lua_State* L) {

	if (realObject == nullptr) {
		ERROR_F("realObject is nullptr");
		return 0;
	}

	realObject->schedulePvpTefRemovalTask(true);  //Remove Now for all

	return 0;
}

int LuaPlayerObject::hasTef(lua_State* L) {
	lua_pushboolean(L, realObject->hasTef());

	return 1;
}

int LuaPlayerObject::clearActivePets(lua_State* L) {

	if (realObject == nullptr) {
		ERROR_F("realObject is nullptr");
		return 0;
	}

	//Infinity:  Custom code to clear active pets
	ClearActivePetsTask* task = new ClearActivePetsTask(realObject);
	task->schedule(2000);  //Wait 2 seconds for pets to store just in case

	return 0;
}

int LuaPlayerObject::getStarterProfession(lua_State* L) {

	if (realObject != nullptr) {
		lua_pushstring(L, realObject->getStarterProfession().toCharArray());
	}
	else {
		ERROR_F("realObject is nullptr");
		lua_pushstring(L, "");
	}
	return 1;
}

int LuaPlayerObject::removeOwnedStructure(lua_State* L) {
	SceneObject* obj = (SceneObject*) lua_touserdata(L, -1);

	if (obj == nullptr || !obj->isStructureObject()) {
		return 0;
	}

	if (realObject == nullptr) {
		ERROR_F("realObject is nullptr");
		return 0;
	}

	StructureObject* structure = cast<StructureObject*>(obj);

	Locker locker(realObject);

	realObject->removeOwnedStructure(structure);

	return 0;
}

int LuaPlayerObject::getTotalOwnedStructureCount(lua_State* L) {

	if (realObject != nullptr) {
		lua_pushinteger(L, realObject->getTotalOwnedStructureCount());	
	}
	else {
		ERROR_F("realObject is nullptr");
		lua_pushnil(L);
	}
	return 1;
}


int LuaPlayerObject::getOwnedStructure(lua_State* L) {

    int numberOfArguments = lua_gettop(L) - 1;

	if (numberOfArguments != 1) {
		ERROR_F("no argument provided");
        lua_pushnil(L);
		return 1;
	}

    int index = lua_tonumber(L, -1);

	if (realObject != nullptr) {

        auto zoneServer = realObject->getZoneServer();
        if (zoneServer == nullptr) {
            ERROR_F("zoneServer is nullptr");
            lua_pushnil(L);
            return 1;
        }

        uint64 oid = realObject->getOwnedStructure(index);
        auto structure = zoneServer->getObject(oid).castTo<StructureObject*>();

        if (structure != nullptr) {
            lua_pushlightuserdata(L, structure);
        }
        else {
            lua_pushnil(L);
        }
        return 1;
	
	}
	else {
		ERROR_F("realObject is nullptr");
		lua_pushnil(L);
	}
	return 1;
}

int LuaPlayerObject::getSecondsSinceLastPvp(lua_State* L) {

	if (realObject != nullptr) {
        int64 gcwTefSeconds = realObject->getLastGcwPvpCombatActionTimestamp().miliDifference() / 1000.0f;
        int64 bhTefSeconds = realObject->getLastBhPvpCombatActionTimestamp().miliDifference() / 1000.0f;
        int64 pvpAreaTefSeconds = realObject->getLastPvpAreaCombatActionTimestamp().miliDifference() / 1000.0f;
        lua_pushnumber(L, Math::min(gcwTefSeconds, Math::min(bhTefSeconds, pvpAreaTefSeconds)));
	}
	else {
		ERROR_F("realObject is nullptr");
		lua_pushnil(L);
	}
	return 1;
}

int LuaPlayerObject::hasDwTef(lua_State* L) {
	if (realObject != nullptr) {
		lua_pushboolean(L, realObject->hasDwTef());
	} else {
		ERROR_F("realObject is nullptr");
		lua_pushboolean(L, false);
	}
	return 1;
}

int LuaPlayerObject::updateDwTef(lua_State* L) {
	if (realObject == nullptr) {
		ERROR_F("realObject is nullptr");
		return 0;
	}

	realObject->updateLastDwPvpCombatActionTimestamp();

	return 0;
}

int LuaPlayerObject::getDwFaction(lua_State* L) {
	if (realObject != nullptr) {
		lua_pushinteger(L, realObject->getDwFaction());
	} else {
		ERROR_F("realObject is nullptr");
		lua_pushinteger(L, 0);
	}
	return 1;
}

int LuaPlayerObject::setDwFaction(lua_State* L) {
	if (realObject == nullptr) {
		ERROR_F("realObject is nullptr");
		return 0;
	}

	int faction = lua_tointeger(L, -1);

	if (faction < 0 || faction > 4) {
		ERROR_F("invalid faction value (must be 0-4)");
		return 0;
	}

	Locker locker(realObject);

	realObject->setDwFaction((byte)faction);

	return 0;
}

int LuaPlayerObject::setTitle(lua_State* L) {
	if (realObject == nullptr) {
		ERROR_F("realObject is nullptr");
		return 0;
	}

	String title = lua_tostring(L, -1);

	Locker locker(realObject);

	realObject->setTitle(title);

	return 0;
}

int LuaPlayerObject::getTitle(lua_State* L) {
	if (realObject != nullptr) {
		lua_pushstring(L, realObject->getTitle().toCharArray());
	} else {
		ERROR_F("realObject is nullptr");
		lua_pushstring(L, "");
	}
	return 1;
}

int LuaPlayerObject::resetPilotTier(lua_State* L) {
	Locker lock(realObject);

	realObject->resetPilotTier();

	return 0;
}

int LuaPlayerObject::setPilotTier(lua_State* L) {
	int argCount = lua_gettop(L) - 1;

	if (argCount < 1) {
		realObject->error() << "LuaPlayerObject::setPilotTier - Pilot Tier arg not provided.";
		return 0;
	}

	int newTier = lua_tointeger(L, -1);

	Locker lock(realObject);

	realObject->setPilotTier(newTier);

	return 0;
}

int LuaPlayerObject::getPilotTier(lua_State* L) {
	lua_pushinteger(L, realObject->getPilotTier());

	return 1;
}

int LuaPlayerObject::isSquadronType(lua_State* L) {
	uint32 squadron = lua_tointeger(L, -1);
	bool ret = false;

	if (squadron > 0 && squadron < 10) {
		ret = realObject->getPilotSquadron() == squadron;
	}

	lua_pushboolean(L, ret);

	return 1;
}

int LuaPlayerObject::setSquadronType(lua_State* L) {
	uint32 squadron = lua_tointeger(L, -1);

	if (squadron < 1 || squadron > 9) {
		return 0;
	}

	Locker lock(realObject);

	realObject->setPilotSquadron(squadron);

	return 0;
}

int LuaPlayerObject::getSquadronType(lua_State* L) {
	uint32 squadronType = realObject->getPilotSquadron();

	lua_pushinteger(L, squadronType);

	return 1;
}

int LuaPlayerObject::addDroidCommand(lua_State* L) {
	String value = lua_tostring(L, -1);

	SkillManager* skillManager = SkillManager::instance();

	Locker locker(realObject);

	Vector<String> droidCommandNames;

	droidCommandNames.add(value);

	skillManager->addDroidCommands(realObject, droidCommandNames, true);

	return 1;
}

int LuaPlayerObject::removeDroidCommands(lua_State* L) {
	SkillManager* skillManager = SkillManager::instance();

	Locker locker(realObject);

	skillManager->removeDroidCommands(realObject);

	return 1;
}
