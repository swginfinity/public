/*
 * LuaCreatureObject.cpp
 *
 *  Created on: 31/08/2009
 *      Author: victor
 */

#include "LuaCreatureObject.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/objects/creature/ai/DroidObject.h"
#include "server/zone/objects/cell/CellObject.h"
#include "server/zone/objects/player/sessions/ConversationSession.h"
#include "server/zone/ZoneServer.h"
#include "server/zone/objects/group/GroupObject.h"
#include "server/zone/objects/player/sessions/EntertainingSession.h"
#include "server/zone/objects/player/PlayerObject.h"
#include "server/zone/managers/player/PlayerManager.h"
#include "server/zone/managers/skill/SkillManager.h"
#include "server/zone/objects/tangible/threat/ThreatMap.h"
#include "server/zone/objects/transaction/TransactionLog.h"
#include "server/zone/Zone.h"
#include "server/zone/managers/combat/CombatManager.h"
// Infinity: JTL includes
#include "server/zone/objects/player/events/StoreSpawnedChildrenTask.h"
#include "server/zone/objects/intangible/ShipControlDevice.h"
// Infinity:  Custom includes
#include "server/zone/objects/mission/MissionObject.h"
#include "server/zone/managers/mission/MissionManager.h"
#include "server/zone/managers/loot/LootManager.h"
#include "server/zone/managers/stringid/StringIdManager.h"
#include "templates/params/creature/CreatureAttribute.h"

const char LuaCreatureObject::className[] = "LuaCreatureObject";

Luna<LuaCreatureObject>::RegType LuaCreatureObject::Register[] = {
		{ "_setObject", &LuaCreatureObject::_setObject },
		{ "_getObject", &LuaSceneObject::_getObject },
		{ "sendSystemMessage", &LuaCreatureObject::sendSystemMessage },
		{ "sendSystemMessageWithDI", &LuaCreatureObject::sendSystemMessageWithDI },
		{ "sendSystemMessageWithTO", &LuaCreatureObject::sendSystemMessageWithTO },
		{ "sendGroupMessage", &LuaCreatureObject::sendGroupMessage },
		{ "playMusicMessage", &LuaCreatureObject::playMusicMessage },
		{ "sendNewbieTutorialRequest", &LuaCreatureObject::sendNewbieTutorialRequest },
		{ "hasScreenPlayState", &LuaCreatureObject::hasScreenPlayState },
		{ "setScreenPlayState", &LuaCreatureObject::setScreenPlayState },
		{ "getScreenPlayState", &LuaCreatureObject::getScreenPlayState },
		{ "playEffect", &LuaSceneObject::playEffect },
		{ "sendNewbieTutorialEnableHudElement", &LuaCreatureObject::sendNewbieTutorialEnableHudElement },
		{ "getInCellNumber", &LuaCreatureObject::getInCellNumber },
		{ "getBuildingParentID", &LuaCreatureObject::getBuildingParentID },
		{ "sendOpenHolocronToPageMessage", &LuaCreatureObject::sendOpenHolocronToPageMessage },
		{ "inflictDamage", &LuaCreatureObject::inflictDamage },
		{ "setHAM", &LuaCreatureObject::setHAM },
		{ "setBaseHAM", &LuaCreatureObject::setBaseHAM },
		{ "setMaxHAM", &LuaCreatureObject::setMaxHAM },
		{ "getHAM", &LuaCreatureObject::getHAM },
		{ "getBaseHAM", &LuaCreatureObject::getBaseHAM },
		{ "getMaxHAM", &LuaCreatureObject::getMaxHAM },
		{ "getTargetID", &LuaCreatureObject::getTargetID },
		{ "clearCombatState", &LuaCreatureObject::clearCombatState },
		{ "getParent", &LuaSceneObject::getParent },
		{ "getZoneName", &LuaSceneObject::getZoneName },
		{ "getObjectID", &LuaSceneObject::getObjectID },
		{ "getPositionX", &LuaSceneObject::getPositionX },
		{ "getPositionY", &LuaSceneObject::getPositionY },
		{ "getPositionZ", &LuaSceneObject::getPositionZ },
		{ "getDirectionAngle", &LuaSceneObject::getDirectionAngle },
		{ "getWorldPositionX", &LuaSceneObject::getWorldPositionX },
		{ "getWorldPositionY", &LuaSceneObject::getWorldPositionY },
		{ "getWorldPositionZ", &LuaSceneObject::getWorldPositionZ },
		{ "getParentID", &LuaSceneObject::getParentID },
		{ "isInRangeWithObject", &LuaSceneObject::isInRangeWithObject },
		{ "isInRangeWithObject3d", &LuaSceneObject::isInRangeWithObject3d },
		{ "getDistanceTo", &LuaSceneObject::getDistanceTo },
		{ "getDistanceToPosition", &LuaSceneObject::getDistanceToPosition },
		{ "getDistanceTo3d", &LuaSceneObject::getDistanceTo3d },
		{ "getDistanceToPosition3d", &LuaSceneObject::getDistanceToPosition3d },
		{ "getServerObjectCRC", &LuaSceneObject::getServerObjectCRC },
		{ "isFeigningDeath", &LuaCreatureObject::isFeigningDeath},
		{ "hasState", &LuaCreatureObject::hasState},
		{ "setState", &LuaCreatureObject::setState},
		{ "clearState", &LuaCreatureObject::clearState},
		{ "setLootRights", &LuaCreatureObject::setLootRights},
		{ "getPosture", &LuaCreatureObject::getPosture},
		{ "setPosture", &LuaCreatureObject::setPosture},
		{ "setMoodString", &LuaCreatureObject::setMoodString},
		{ "getMoodString", &LuaCreatureObject::getMoodString},
		{ "hasSkill", &LuaCreatureObject::hasSkill},
		{ "removeSkill", &LuaCreatureObject::removeSkill},
		{ "surrenderSkill", &LuaCreatureObject::surrenderSkill},
		{ "getConversationSession", &LuaCreatureObject::getConversationSession},
		{ "doAnimation", &LuaCreatureObject::doAnimation},
		{ "engageCombat", &LuaCreatureObject::engageCombat},
		{ "getPlayerObject", &LuaCreatureObject::getPlayerObject},
		{ "setCustomObjectName", &LuaSceneObject::setCustomObjectName},
		{ "getFaction", &LuaCreatureObject::getFaction},
		{ "setFaction", &LuaCreatureObject::setFaction},
		{ "isRebel", &LuaCreatureObject::isRebel},
		{ "isImperial", &LuaCreatureObject::isImperial},
		{ "isNeutral", &LuaCreatureObject::isNeutral},
		{ "teleport", &LuaSceneObject::teleport},
		{ "getFirstName", &LuaCreatureObject::getFirstName},
		{ "getLastName", &LuaCreatureObject::getLastName},
		{ "setLastName", &LuaCreatureObject::setLastName},
		{ "isAiAgent", &LuaCreatureObject::isAiAgent},
		{ "setFactionRank", &LuaCreatureObject::setFactionRank},
		{ "getFactionRank", &LuaCreatureObject::getFactionRank},
		{ "getCashCredits", &LuaCreatureObject::getCashCredits},
		{ "getBankCredits", &LuaCreatureObject::getBankCredits },
		{ "subtractCashCredits", &LuaCreatureObject::subtractCashCredits},
		{ "subtractBankCredits", &LuaCreatureObject::subtractBankCredits},
		{ "addCashCredits", &LuaCreatureObject::addCashCredits},
		{ "addBankCredits", &LuaCreatureObject::addBankCredits},
		{ "removeScreenPlayState", &LuaCreatureObject::removeScreenPlayState},
		{ "isGrouped", &LuaCreatureObject::isGrouped},
		{ "isGroupedWith", &LuaCreatureObject::isGroupedWith},
		{ "getGroupSize", &LuaCreatureObject::getGroupSize},
		{ "getGroupMember", &LuaCreatureObject::getGroupMember},
		{ "getOptionsBitmask", &LuaTangibleObject::getOptionsBitmask},
		{ "setOptionsBitmask", &LuaCreatureObject::setOptionsBitmask},
		{ "setOptionBit", &LuaTangibleObject::setOptionBit},
		{ "clearOptionBit", &LuaTangibleObject::clearOptionBit},
		{ "getPvpStatusBitmask", &LuaTangibleObject::getPvpStatusBitmask},
		{ "setPvpStatusBitmask", &LuaTangibleObject::setPvpStatusBitmask},
		{ "setPvpStatusBit", &LuaTangibleObject::setPvpStatusBit},
		{ "isChangingFactionStatus", &LuaTangibleObject::isChangingFactionStatus },
		{ "getFactionStatus", &LuaTangibleObject::getFactionStatus },
		{ "setFutureFactionStatus", &LuaTangibleObject::setFutureFactionStatus },
		{ "addDotState", &LuaCreatureObject::addDotState},
		{ "getSlottedObject", &LuaSceneObject::getSlottedObject},
		{ "checkCooldownRecovery", &LuaCreatureObject::checkCooldownRecovery},
		{ "addCooldown", &LuaCreatureObject::addCooldown},
		{ "isDead", &LuaCreatureObject::isDead},
		{ "isIncapacitated", &LuaCreatureObject::isIncapacitated },
		{ "getLevel", &LuaCreatureObject::getLevel},
		{ "getQueueSize", &LuaCreatureObject::getQueueSize },
		{ "isDancing", &LuaCreatureObject::isDancing},
		{ "isPlayingMusic", &LuaCreatureObject::isPlayingMusic},
		{ "getPerformanceName", &LuaCreatureObject::getPerformanceName},
		{ "getWalkSpeed", &LuaCreatureObject::getWalkSpeed },
		{ "isAttackableBy", &LuaCreatureObject::isAttackableBy },
		{ "getSpecies", &LuaCreatureObject::getSpecies },
		{ "isDroidPet", &LuaCreatureObject::isDroidPet },
		{ "isCombatDroidPet", &LuaCreatureObject::isCombatDroidPet },
		{ "awardExperience", &LuaCreatureObject::awardExperience },
		{ "getOwner", &LuaCreatureObject::getOwner },
		{ "getCurrentSpeed", &LuaCreatureObject::getCurrentSpeed },
		{ "isInvisible", &LuaTangibleObject::isInvisible },
		{ "isInCombat", &LuaCreatureObject::isInCombat },
		{ "healDamage", &LuaCreatureObject::healDamage },
		{ "getGroupID", &LuaCreatureObject::getGroupID },
		{ "enhanceCharacter", &LuaCreatureObject::enhanceCharacter },
		{ "setWounds", &LuaCreatureObject::setWounds },
		{ "setShockWounds", &LuaCreatureObject::setShockWounds },
		{ "getForceSensitiveSkillCount", &LuaCreatureObject::getForceSensitiveSkillCount },
		{ "villageKnightPrereqsMet", &LuaCreatureObject::villageKnightPrereqsMet },
		{ "isOnLeave", &LuaTangibleObject::isOnLeave },
		{ "isOvert", &LuaTangibleObject::isOvert },
		{ "isCovert", &LuaTangibleObject::isCovert },
		{ "setFactionStatus", &LuaTangibleObject::setFactionStatus },
		{ "getDamageDealerList", &LuaCreatureObject::getDamageDealerList },
		{ "getHealingThreatList", &LuaCreatureObject::getHealingThreatList },
		{ "getAllThreatsList", &LuaCreatureObject::getAllThreatsList },
		{ "dropFromThreatMap", &LuaCreatureObject::dropFromThreatMap },
		{ "getSkillMod", &LuaCreatureObject::getSkillMod },
		{ "getGender", &LuaCreatureObject::getGender },
		{ "isRidingMount", &LuaCreatureObject::isRidingMount },
		{ "isPilotingShip", &LuaCreatureObject::isPilotingShip },
		// Infinity: JTL Lua methods
		{ "setSpawnerID", &LuaCreatureObject::setSpawnerID },
		{ "getSpawnerID", &LuaCreatureObject::getSpawnerID },
		{ "storePets", &LuaCreatureObject::storePets },
		{ "isRebelPilot", &LuaCreatureObject::isRebelPilot },
		{ "isImperialPilot", &LuaCreatureObject::isImperialPilot },
		{ "isNeutralPilot", &LuaCreatureObject::isNeutralPilot },
		{ "hasCertifiedShip", &LuaCreatureObject::hasCertifiedShip },
		{ "failQuestMission", &LuaCreatureObject::failQuestMission },
		{ "removeQuestMission", &LuaCreatureObject::removeQuestMission },
		{ "addSpaceMissionObject", &LuaCreatureObject::addSpaceMissionObject },
		{ "removeSpaceMissionObject", &LuaCreatureObject::removeSpaceMissionObject },
		{ "dismount", &LuaCreatureObject::dismount },
		{ "setAppearance", &LuaCreatureObject::setAppearance },
		{ "getMainDefender", &LuaTangibleObject::getMainDefender },
		{ "getWeaponType", &LuaCreatureObject::getWeaponType },
		{ "attemptPeace", &LuaCreatureObject::attemptPeace },
		{ "forcePeace", &LuaCreatureObject::forcePeace },
		// Infinity:  Custom hooks
		{ "gmenhanceCharacter", &LuaCreatureObject::gmenhanceCharacter },
		{ "enhanceCreature", &LuaCreatureObject::enhanceCreature },
		{ "resetBuffs", &LuaCreatureObject::resetBuffs },
		{ "clearDots", &LuaCreatureObject::clearDots },
		{ "removeFeignedDeath", &LuaCreatureObject::removeFeignedDeath },
		{ "addBuff", &LuaCreatureObject::addBuff },
		{ "hasBuff", &LuaCreatureObject::hasBuff },
		{ "removeBuff", &LuaCreatureObject::removeBuff },
		{ "getBuffAmount", &LuaCreatureObject::getBuffAmount },
		{ "getBuffDuration", &LuaCreatureObject::getBuffDuration },
		{ "getBuffTimeLeft", &LuaCreatureObject::getBuffTimeLeft },
		{ "getPlayerBountyTarget", &LuaCreatureObject::getPlayerBountyTarget },
		{ "areInDuel", &LuaCreatureObject::areInDuel },
		{ "addWounds", &LuaCreatureObject::addWounds },
		{ "addShockWounds", &LuaCreatureObject::addShockWounds },
		{ "docBuff", &LuaCreatureObject::docBuff },
		{ "getGuildID", &LuaCreatureObject::getGuildID },
		{ "getShockWounds", &LuaCreatureObject::getShockWounds },
		{ "isHarvesterDroidPet", &LuaCreatureObject::isHarvesterDroidPet },
		{ "isInCamp", &LuaCreatureObject::isInCamp },
		{ "isSwimming", &LuaCreatureObject::isSwimming },
		{ "isCityBanned", &LuaCreatureObject::isCityBanned },
		{ "failPlayerBountyMission", &LuaCreatureObject::failPlayerBountyMission },
		{ "createAttachment", &LuaCreatureObject::createAttachment },
		{ "setSpeedBuff", &LuaCreatureObject::setSpeedBuff },
		{ "setRootedState", &LuaCreatureObject::setRootedState },
		{ "setSnaredState", &LuaCreatureObject::setSnaredState },
		{ "removeSpeedBuffs", &LuaCreatureObject::removeSpeedBuffs },
		{ "getSkillsHash", &LuaCreatureObject::getSkillsHash },
		{ "surrenderAllSkills", &LuaCreatureObject::surrenderAllSkills },
		{ "ejectPlayerFromBuilding", &LuaCreatureObject::ejectPlayerFromBuilding },
        { "addSkillModifierBuff", &LuaCreatureObject::addSkillModifierBuff },
        { "addAttributeModifierBuff", &LuaCreatureObject::addAttributeModifierBuff },
        { "getSlottedObjects", &LuaSceneObject::getSlottedObjects },
        { "getHighestDamageGroupLeader", &LuaCreatureObject::getHighestDamageGroupLeader },
        { "feignDeath", &LuaCreatureObject::feignDeath },
		{ "setOwner", &LuaCreatureObject::setOwner },
		{ "executeCommand", &LuaCreatureObject::executeCommand },
		{ "addPersonalEnemyFlag", &LuaCreatureObject::addPersonalEnemyFlag },
		{ "removePersonalEnemyFlag", &LuaCreatureObject::removePersonalEnemyFlag },
		{ "hasPersonalEnemyFlag", &LuaCreatureObject::hasPersonalEnemyFlag },
		{ 0, 0 }
};

LuaCreatureObject::LuaCreatureObject(lua_State* L) : LuaTangibleObject(L) {
#ifdef DYNAMIC_CAST_LUAOBJECTS
	realObject = dynamic_cast<CreatureObject*>(_getRealSceneObject());

	// Infinity:  No need to crash server with assertion, just throw a Lua error and return
	if (_getRealSceneObject() != nullptr && realObject == nullptr) {
		Logger::console.error(LuaSceneObject::getLuaCallStack(L, "LuaCreatureObject::LuaCreatureObject - Detected assertion failure condition."));
		return;
	}

	// E3_ASSERT(!_getRealSceneObject() || realObject != nullptr);
#else
	realObject = static_cast<CreatureObject*>(lua_touserdata(L, 1));
#endif
}

LuaCreatureObject::~LuaCreatureObject(){
}

int LuaCreatureObject::_setObject(lua_State* L) {
	LuaTangibleObject::_setObject(L);

#ifdef DYNAMIC_CAST_LUAOBJECTS
	auto obj = dynamic_cast<CreatureObject*>(_getRealSceneObject());

	if (obj != realObject) {
		realObject = obj;
	}

	// Infinity:  No need to crash server with assertion, just throw a Lua error and return
	if (_getRealSceneObject() != nullptr && realObject == nullptr) {
		String msg = "LuaCreatureObject::_setObject - Detected assertion failure condition." + LuaSceneObject::getLuaCallStack(L);
		return luaL_error(L, msg.toCharArray());
	}

	// E3_ASSERT(_getRealSceneObject() == nullptr || realObject != nullptr);
#else
	auto obj = static_cast<CreatureObject*>(lua_touserdata(L, -1));

	if (realObject != obj) {
		realObject = obj;
	}
#endif

	return 0;
}

int LuaCreatureObject::getFirstName(lua_State* L) {
	String text = realObject->getFirstName();
	lua_pushstring(L, text.toCharArray());
	return 1;
}

int LuaCreatureObject::getLastName(lua_State* L) {
	String text = realObject->getLastName();
	lua_pushstring(L, text.toCharArray());
	return 1;
}

int LuaCreatureObject::setLastName(lua_State* L) {
	int argc = lua_gettop(L) - 1;

	if (argc < 1 || argc > 2) {
		Logger::console.error("incorrect number of arguments for LuaCreatureObject::setLastName");
		return 0;
	}

	String newLastName;
	bool skipVerify;

	if (argc == 1) {
		newLastName = lua_tostring(L, -1);
		skipVerify = false;
	} else {
		newLastName = lua_tostring(L, -2);
		skipVerify = lua_toboolean(L, -1);
	}

	Locker locker(realObject);

	auto errmsg = realObject->setLastName(newLastName, skipVerify);

	lua_pushstring(L, errmsg.toCharArray());

	return 1;
}

int LuaCreatureObject::addDotState(lua_State* L) {
	//addDotState(unsigned long dotType, unsigned int strength, byte type, unsigned int duration, float potency, unsigned int defense)
	uint32 defense = lua_tointeger(L, -1);
	uint32 objectID = lua_tointeger(L, -2);
	float potency = lua_tonumber(L, -3);
	uint32 duration = lua_tointeger(L, -4);
	byte type = lua_tointeger(L, -5);
	uint32 strength = lua_tointeger(L, -6);
	uint64 dotType = lua_tointeger(L, -7);
	CreatureObject* attacker = (CreatureObject*)lua_touserdata(L, -8);

	Locker locker(realObject);

	realObject->addDotState(attacker, dotType, objectID, strength, type, duration, potency, defense);

	return 0;
}

int LuaCreatureObject::isFeigningDeath(lua_State* L) {
	lua_pushnumber(L, realObject->isFeigningDeath());
	return 1;
}

int LuaCreatureObject::hasState(lua_State* L) {
	uint64 state = (uint64) lua_tonumber(L, -1);

	lua_pushnumber(L, realObject->hasState(state));
	return 1;
}

int LuaCreatureObject::setState(lua_State* L) {
	uint64 state = (uint64) lua_tonumber(L, -1);

	Locker locker(realObject);

	realObject->setState(state, true);

	return 0;
}

int LuaCreatureObject::getPosture(lua_State* L) {
	lua_pushnumber(L, realObject->getPosture());

	return 1;
}

int LuaCreatureObject::setPosture(lua_State* L) {
	uint32 posture = (uint32) lua_tonumber(L, -1);

	Locker locker(realObject);

	realObject->setPosture(posture, true);

	return 0;
}

int LuaCreatureObject::setMoodString(lua_State* L) {
	String value = lua_tostring(L, -1);

	Locker locker(realObject);

	realObject->setMoodString(value);

	return 0;
}

int LuaCreatureObject::getMoodString(lua_State* L) {
	String mood = realObject->getMoodString();

	lua_pushstring(L, mood.toCharArray());

	return 1;
}

int LuaCreatureObject::sendOpenHolocronToPageMessage(lua_State* L) {
	String value = lua_tostring(L, -1);

	realObject->sendOpenHolocronToPageMessage(value);

	return 0;
}

int LuaCreatureObject::sendSystemMessage(lua_State* L) {
	if (lua_islightuserdata(L, -1)) {
		StringIdChatParameter* message = (StringIdChatParameter*)lua_touserdata(L, -1);
		realObject->sendSystemMessage(*message);
	} else {
		String value = lua_tostring(L, -1);
		realObject->sendSystemMessage(value);
	}
	return 0;
}

int LuaCreatureObject::sendSystemMessageWithDI(lua_State* L) {
	String text = lua_tostring(L, -2);
	uint32 value = (uint32) lua_tonumber(L, -1);

	StringIdChatParameter param(text);
	param.setDI(value);

	realObject->sendSystemMessage(param);

	return 0;
}

int LuaCreatureObject::sendSystemMessageWithTO(lua_State* L) {
	String text = lua_tostring(L, -2);
	String value = lua_tostring(L, -1);

	StringIdChatParameter param(text);
	param.setTO(value);

	realObject->sendSystemMessage(param);

	return 0;
}

int LuaCreatureObject::sendSystemMessageWithTT(lua_State* L) {
	String text = lua_tostring(L, -2);
	String value = lua_tostring(L, -1);

	StringIdChatParameter param(text);
	param.setTT(value);

	realObject->sendSystemMessage(param);

	return 0;
}

int LuaCreatureObject::sendGroupMessage(lua_State* L) {
	String value = lua_tostring(L, -1);

	if (realObject == nullptr)
		return 0;

	if (!realObject->isGrouped()) {
		realObject->sendSystemMessage(value);
	} else {
		GroupObject* group = realObject->getGroup();

		group->sendSystemMessage(value);
	}

	return 0;
}

int LuaCreatureObject::setHAM(lua_State* L) {
	uint32 type = (uint32) lua_tonumber(L, -2);
	uint32 value = (uint32) lua_tonumber(L, -1);

	Locker locker(realObject);

	realObject->setHAM(type, value);

	return 0;
}

int LuaCreatureObject::setBaseHAM(lua_State* L) {
	uint32 type = (uint32) lua_tonumber(L, -2);
	uint32 value = (uint32) lua_tonumber(L, -1);

	Locker locker(realObject);

	realObject->setBaseHAM(type, value);

	return 0;
}

int LuaCreatureObject::setMaxHAM(lua_State* L) {
	uint32 type = (uint32) lua_tonumber(L, -2);
	uint32 value = (uint32) lua_tonumber(L, -1);

	Locker locker(realObject);

	realObject->setMaxHAM(type, value);

	return 0;
}

int LuaCreatureObject::playMusicMessage(lua_State *L) {
	String value = lua_tostring(L, -1);
	realObject->playMusicMessage(value);

	return 0;
}

int LuaCreatureObject::sendNewbieTutorialRequest(lua_State *L) {
	String value = lua_tostring(L, -1);
	realObject->sendNewbieTutorialRequest(value);

	return 0;
}

int LuaCreatureObject::hasScreenPlayState(lua_State *L) {
	String play = lua_tostring(L, -1);
	uint64 stateToCheck = lua_tointeger(L, -2);

	uint64 state = realObject->getScreenPlayState(play);

	bool check = state & stateToCheck;

	lua_pushboolean(L, check);

	return 1;
}

int LuaCreatureObject::getScreenPlayState(lua_State *L) {
	String play = lua_tostring(L, -1);
	uint64 state = realObject->getScreenPlayState(play);

	lua_pushinteger(L, state);

	return 1;
}

int LuaCreatureObject::sendNewbieTutorialEnableHudElement(lua_State* L) {
	String ui = lua_tostring(L, -3);
	int enable = lua_tonumber(L, -2);
	float blinkCount = lua_tonumber(L, -1);

	realObject->sendNewbieTutorialEnableHudElement(ui, enable, blinkCount);

	return 0;
}

int LuaCreatureObject::hasSkill(lua_State* L) {
	String value = lua_tostring(L, -1);

	bool check = realObject->hasSkill(value);

	lua_pushboolean(L, check);

	return 1;
}

int LuaCreatureObject::removeSkill(lua_State* L) {
	String value = lua_tostring(L, -1);

	Locker locker(realObject);

	realObject->removeSkill(value, true);
	return 0;
}

int LuaCreatureObject::surrenderSkill(lua_State* L) {
	
	String value = lua_tostring(L, -1);

	SkillManager* skillManager = SkillManager::instance();

	if (realObject != nullptr && skillManager != nullptr) {
		skillManager->surrenderSkill(value, realObject, true, true, false);  //Infinity:  Notify, CheckFRS, noReGrant
	}

	return 0;
}

int LuaCreatureObject::getInCellNumber(lua_State* L) {
	SceneObject* parent = realObject->getParent().get().get();

	if (parent == nullptr || !parent->isCellObject())
		lua_pushnumber(L, -1);
	else {
		int cellId = ((CellObject*)parent)->getCellNumber();

		lua_pushnumber(L, cellId);
	}

	return 1; // we return the number of vars pushed to the L stack
}

int LuaCreatureObject::getBuildingParentID(lua_State* L) {
	ManagedReference<SceneObject*> parent = realObject->getParentRecursively(SceneObjectType::BUILDING);

	if (parent == nullptr)
		lua_pushnumber(L, 0);
	else
		lua_pushnumber(L, parent->getObjectID());

	return 1;
}

int LuaCreatureObject::setScreenPlayState(lua_State *L) {
	String play = lua_tostring(L, -1);
	uint64 stateToSet = lua_tointeger(L, -2);

	realObject->setScreenPlayState(play, stateToSet | realObject->getScreenPlayState(play));
	realObject->notifyObservers(ObserverEventType::SCREENPLAYSTATECHANGED, realObject);
	return 0;
}

int LuaCreatureObject::removeScreenPlayState(lua_State* L) {
	String play = lua_tostring(L, -1);
	uint64 stateToClear = lua_tointeger(L, -2);

	if (realObject != nullptr) {
		realObject->setScreenPlayState(play, realObject->getScreenPlayState(play) & (~stateToClear));
		realObject->notifyObservers(ObserverEventType::SCREENPLAYSTATECHANGED, realObject);
	}
	return 0;
}

int LuaCreatureObject::getTargetID(lua_State* L) {
	uint64 targetID = realObject->getTargetID();

	lua_pushinteger(L, targetID);

	return 1;
}

int LuaCreatureObject::clearCombatState(lua_State* L) {
	bool clearDef = lua_tonumber(L, -1);

	Locker locker(realObject);

	realObject->clearCombatState(clearDef);

	return 0;
}

int LuaCreatureObject::getHAM(lua_State* L) {
	int type = lua_tonumber(L, -1);

	int value = realObject->getHAM(type);

	lua_pushnumber(L, value);

	return 1;
}

int LuaCreatureObject::getBaseHAM(lua_State* L) {
	int type = lua_tonumber(L, -1);

	int value = realObject->getBaseHAM(type);

	lua_pushnumber(L, value);

	return 1;
}

int LuaCreatureObject::getMaxHAM(lua_State* L) {
	int type = lua_tonumber(L, -1);

	int value = realObject->getMaxHAM(type);

	lua_pushnumber(L, value);

	return 1;
}

int LuaCreatureObject::inflictDamage(lua_State* L) {
	// int inflictDamage(TangibleObject attacker, int damageType, int damage, boolean destroy, boolean notifyClient = true);
	int destroy = lua_tonumber(L, -1);
	int damage = lua_tonumber(L, -2);
	int damageType = lua_tonumber(L, -3);

	SceneObject* scene = (SceneObject*)lua_touserdata(L, -4);

	TangibleObject* attacker = cast<TangibleObject*>(scene);

	// Infinity:  No need to crash server with assertion, just throw a Lua error and return
	if (attacker == nullptr) {
		String msg = "LuaCreatureObject::inflictDamage - Attacker is nullptr." + LuaSceneObject::getLuaCallStack(L);
		return luaL_error(L, msg.toCharArray());
	}

	// E3_ASSERT(attacker);

	Locker locker(realObject);

	Locker crossLocker(attacker, realObject);

	realObject->inflictDamage(attacker, damageType, damage, destroy);

	return 0;
}

int LuaCreatureObject::getBankCredits(lua_State *L) {
	lua_pushinteger(L, realObject->getBankCredits());

	return 1; // we return the number of vars pushed to the L stack
}

int LuaCreatureObject::getConversationSession(lua_State* L) {
	Reference<ConversationSession*> session = realObject->getActiveSession(SessionFacadeType::CONVERSATION).castTo<ConversationSession*>();

	if (session != nullptr) {
		session->_setUpdated(true);
		lua_pushlightuserdata(L, session);
	} else
		lua_pushnil(L);

	return 1;
}

int LuaCreatureObject::doAnimation(lua_State* L) {
	String animString = lua_tostring(L, -1);

	if (realObject != nullptr)
		realObject->doAnimation(animString);

	return 0;
}

int LuaCreatureObject::engageCombat(lua_State* L) {
	CreatureObject* enemy = (CreatureObject*)lua_touserdata(L, -1);

	Locker locker(realObject);

	if (enemy != nullptr)
		realObject->addDefender(enemy);

	return 0;
}

int LuaCreatureObject::getPlayerObject(lua_State* L) {
	Reference<PlayerObject*> obj = realObject->getPlayerObject();

	if (obj != nullptr) {
		obj->_setUpdated(true);
		lua_pushlightuserdata(L, obj);
	} else
		lua_pushnil(L);

	return 1;
}

int LuaCreatureObject::getFaction(lua_State* L) {
	uint32 faction = realObject->getFaction();

	lua_pushinteger(L, faction);

	return 1;
}

int LuaCreatureObject::isRebel(lua_State* L) {
	lua_pushboolean(L, realObject->isRebel());

	return 1;
}

int LuaCreatureObject::isNeutral(lua_State* L) {
	lua_pushboolean(L, realObject->isNeutral());

	return 1;
}

int LuaCreatureObject::isImperial(lua_State* L) {
	lua_pushboolean(L, realObject->isImperial());

	return 1;
}

int LuaCreatureObject::setFaction(lua_State* L) {
	uint32 faction = lua_tointeger(L, -1);

	Locker locker(realObject);

	realObject->setFaction(faction);

	return 0;
}

int LuaCreatureObject::setFactionRank(lua_State* L) {
	int rank = lua_tointeger(L, -1);

	Locker locker(realObject);

	realObject->setFactionRank(rank);

	return 0;
}

int LuaCreatureObject::getFactionRank(lua_State* L) {
	lua_pushinteger(L, realObject->getFactionRank());

	return 1;
}

int LuaCreatureObject::getCashCredits(lua_State* L) {
	lua_pushinteger(L, realObject->getCashCredits());

	return 1;
}

int LuaCreatureObject::subtractCashCredits(lua_State* L) {
	Locker locker(realObject);

	int credits = lua_tointeger(L, -1);
	TransactionLog trx(realObject, TrxCode::LUASCRIPT, credits, true);
	trx.addContextFromLua(L);
	realObject->subtractCashCredits(credits);

	return 0;
}

int LuaCreatureObject::subtractBankCredits(lua_State* L) {
	Locker locker(realObject);

	int credits = lua_tointeger(L, -1);
	TransactionLog trx(realObject, TrxCode::LUASCRIPT, credits, false);
	trx.addContextFromLua(L);
	realObject->subtractBankCredits(credits);

	return 0;
}

int LuaCreatureObject::addCashCredits(lua_State* L) {
	bool notifyClient = lua_toboolean(L, -1);
	int credits = lua_tointeger(L, -2);

	Locker locker(realObject);

	TransactionLog trx(TrxCode::LUASCRIPT, realObject, credits, true);
	trx.addContextFromLua(L);
	realObject->addCashCredits(credits, notifyClient);

	return 0;
}

int LuaCreatureObject::addBankCredits(lua_State* L) {
	bool notifyClient = lua_toboolean(L, -1);
	int credits = lua_tointeger(L, -2);

	Locker locker(realObject);

	TransactionLog trx(TrxCode::LUASCRIPT, realObject, credits, false);
	trx.addContextFromLua(L);
	realObject->addBankCredits(credits, notifyClient);

	return 0;
}

int LuaCreatureObject::isAiAgent(lua_State* L) {
	bool val = realObject->isAiAgent();

	lua_pushboolean(L, val);

	return 1;
}

int LuaCreatureObject::isGrouped(lua_State* L) {
	bool val = realObject->isGrouped();

	lua_pushboolean(L, val);

	return 1;
}

int LuaCreatureObject::isGroupedWith(lua_State* L) {
	CreatureObject* groupMember = (CreatureObject*) lua_touserdata(L, -1);

	if (realObject == nullptr || groupMember == nullptr || !realObject->isGrouped())
		return 0;

	GroupObject* group = realObject->getGroup();

	lua_pushboolean(L, group != nullptr && group->hasMember(groupMember));

	return 1;
}

int LuaCreatureObject::setLootRights(lua_State* L) {
	CreatureObject* player = (CreatureObject*) lua_touserdata(L, -1);

	if (realObject == nullptr)
		return 0;

	uint64 ownerID = 0;

	if (player != nullptr) {
		ownerID = player->getObjectID();
	}

	SceneObject* inventory = realObject->getSlottedObject("inventory");

	if (inventory == nullptr)
		return 0;

	Locker locker(inventory);

	inventory->setContainerOwnerID(ownerID);
	return 1;
}

int LuaCreatureObject::getGroupSize(lua_State* L) {
	Locker locker(realObject);

	GroupObject* group = realObject->getGroup();

	if (group == nullptr) {
		lua_pushnumber(L, 0);
		return 1;
	}

	Locker lock(group, realObject);

	lua_pushnumber(L, group->getGroupSize());

	return 1;
}

int LuaCreatureObject::setOptionsBitmask(lua_State* L) {
	uint32 bitmask = lua_tointeger(L, -1);

	Locker locker(realObject);

	realObject->setOptionsBitmask(bitmask, true);

	return 0;
}

int LuaCreatureObject::getGroupMember(lua_State* L) {
	int i = lua_tonumber(L, -1);

	if (i < 0)
		i = 0;

	Locker locker(realObject);

	GroupObject* group = realObject->getGroup();

	if (group == nullptr) {
		lua_pushnil(L);
		return 1;
	}

	Locker lock(group, realObject);

	if (group->getGroupSize() < i) {
		lua_pushnil(L);
		return 1;
	}

	CreatureObject* creo = group->getGroupMember(i);

	if (creo == nullptr) {
		realObject->info("LuaCreatureObject::getGroupMember GroupMember is nullptr.");
		lua_pushnil(L);
	} else {
		creo->_setUpdated(true);
		lua_pushlightuserdata(L, creo);
	}

	return 1;
}

int LuaCreatureObject::checkCooldownRecovery(lua_State* L) {
	String cooldownName = lua_tostring(L, -1);

	lua_pushboolean(L, realObject->checkCooldownRecovery(cooldownName));

	return 1;
}

int LuaCreatureObject::addCooldown(lua_State* L) {
	String cooldownName = lua_tostring(L, -2);
	int milliseconds = lua_tointeger(L, -1);

	realObject->addCooldown(cooldownName, milliseconds);

	return 0;
}

int LuaCreatureObject::isDead(lua_State* L) {
	bool retVal = realObject->isDead();

	lua_pushboolean(L, retVal);

	return 1;
}

int LuaCreatureObject::isIncapacitated(lua_State* L) {
	lua_pushboolean(L, realObject->isIncapacitated());
	return 1;
}

int LuaCreatureObject::getLevel(lua_State* L) {
	int level = realObject->getLevel();

	lua_pushinteger(L, level);

	return 1;
}

int LuaCreatureObject::getQueueSize(lua_State* L) {
	int size = realObject->getCommandQueueSize();

	lua_pushinteger(L, size);

	return 1;
}

int LuaCreatureObject::isDancing(lua_State* L) {
	bool retVal = realObject->isDancing();

	lua_pushboolean(L, retVal);

	return 1;
}

int LuaCreatureObject::isPlayingMusic(lua_State* L) {
	bool retVal = realObject->isPlayingMusic();

	lua_pushboolean(L,  retVal);

	return 1;
}

int LuaCreatureObject::getPerformanceName(lua_State* L) {
	ManagedReference<Facade*> facade = realObject->getActiveSession(SessionFacadeType::ENTERTAINING);

	if (facade == nullptr) {
		lua_pushnil(L);
		return 1;
	}

	ManagedReference<EntertainingSession*> session = dynamic_cast<EntertainingSession*> (facade.get());

	if (session == nullptr) {
		lua_pushnil(L);
		return 1;
	}

	if (!session->isPlayingMusic() && !session->isDancing())
		lua_pushnil(L);
	else
		lua_pushstring(L, session->getPerformanceName().toCharArray());

	return 1;
}

int LuaCreatureObject::getWalkSpeed(lua_State* L) {
	lua_pushnumber(L, realObject->getWalkSpeed());
	return 1;
}

int LuaCreatureObject::isAttackableBy(lua_State* L) {
	TangibleObject* obj = (TangibleObject*) lua_touserdata(L, -1);

	bool retVal = realObject->isAttackableBy(obj);
	lua_pushboolean(L, retVal);

	return 1;
}

int LuaCreatureObject::getSpecies(lua_State* L) {
	lua_pushinteger(L, realObject->getSpecies());

	return 1;
}

int LuaCreatureObject::isDroidPet(lua_State* L) {
	bool retVal = realObject->isDroidObject() && realObject->isPet();
	lua_pushboolean(L, retVal);

	return 1;
}

int LuaCreatureObject::isCombatDroidPet(lua_State* L) {
	bool retVal = false;
	if (realObject->isDroidObject() && realObject->isPet()) {
		DroidObject* d = cast<DroidObject*>(realObject);
		retVal = d->isCombatDroid();
	}
	lua_pushboolean(L, retVal);
	return 1;
}

int LuaCreatureObject::awardExperience(lua_State* L) {

	//Infinity:  Hook heavily customized

	if (realObject == nullptr) {
		Logger::console.error("LuaCreatureObject::awardExperience - realObject is nullptr");
		return 0;
	}

	String experienceType;
	int experienceAmount;
	bool sendSysMessage;
	bool applyModifiers = true;

	int numberOfArguments = lua_gettop(L) - 1;

	if (numberOfArguments == 3) {

		experienceType = lua_tostring(L, -3);
		experienceAmount = lua_tointeger(L, -2);
		sendSysMessage = lua_toboolean(L, -1);
	}
	else if (numberOfArguments == 4) {

		experienceType = lua_tostring(L, -4);
		experienceAmount = lua_tointeger(L, -3);
		sendSysMessage = lua_toboolean(L, -2);
		applyModifiers = lua_toboolean(L, -1);
	}
	else {
		Logger::console.error("LuaCreatureObject::awardExperience - invalid number of arguments");
		return 0;
	}

	auto zoneServer = realObject->getZoneServer();
	if (zoneServer == nullptr) {
		Logger::console.error("LuaCreatureObject::awardExperience - zoneServer is nullptr");
		return 0;
	}

	auto playerManager = zoneServer->getPlayerManager();
	if (playerManager == nullptr) {	
		Logger::console.error("LuaCreatureObject::awardExperience - playermanager is nullptr");
		return 0;
	}

	playerManager->awardExperience(realObject, experienceType, experienceAmount, sendSysMessage, 1.0f, applyModifiers);  //Infinity:  Added support for applyModifiers argument

	return 0;
}

int LuaCreatureObject::getOwner(lua_State* L) {
	CreatureObject* retVal = realObject->getLinkedCreature().get();

	if (retVal == nullptr)
		lua_pushnil(L);
	else
		lua_pushlightuserdata(L, retVal);
	return 1;
}

int LuaCreatureObject::getCurrentSpeed(lua_State* L) {
	float currentSpeed = realObject->getCurrentSpeed();

	lua_pushnumber(L, currentSpeed);

	return 1;
}

int LuaCreatureObject::isInCombat(lua_State* L) {
	bool retVal = realObject->isInCombat();

	lua_pushboolean(L, retVal);

	return 1;
}

int LuaCreatureObject::healDamage(lua_State* L) {

	if (realObject == nullptr) {
		Logger::console.error("LuaCreatureObject::healDamage - realObject is nullptr");
		return 0;
	}

	int numberOfArguments = lua_gettop(L) - 1;
	if (numberOfArguments != 2) {
		Logger::console.error("LuaCreatureObject::healDamage - incorrect number of arguments, 2 are required");
		return 0;
	}

	int damageType = lua_tointeger(L, -2);
	int damageHealed = lua_tointeger(L, -1);

	realObject->healDamage(nullptr, damageType, damageHealed, true, true);  //Let's send nullptr as the healer since there is no Creo healer.  This will not notify observers.

	return 0;
}

int LuaCreatureObject::getGroupID(lua_State* L) {

	lua_pushnumber(L, realObject->getGroupID());

	return 1;
}

int LuaCreatureObject::enhanceCharacter(lua_State* L) {

	if (realObject == nullptr) {
		Logger::console.error("LuaCreatureObject::enhanceCharacter - realObject is nullptr");
		return 0;
	}

	auto zoneServer = realObject->getZoneServer();
	if (zoneServer == nullptr) {
		Logger::console.error("LuaCreatureObject::enhanceCharacter - zoneServer is nullptr");
		return 0;
	}

	auto playerManager = zoneServer->getPlayerManager();
	if (playerManager == nullptr) {	
		Logger::console.error("LuaCreatureObject::enhanceCharacter - playermanager is nullptr");
		return 0;
	}

	playerManager->enhanceCharacter(realObject);

	return 0;
}

int LuaCreatureObject::setWounds(lua_State* L) {
	int amount = lua_tointeger(L, -1);
	int pool = lua_tointeger(L, -2);

	realObject->setWounds(pool, amount, true);

	return 0;
}

int LuaCreatureObject::setShockWounds(lua_State* L) {
	int amount = lua_tointeger(L, -1);

	realObject->setShockWounds(amount, true);

	return 0;
}

int LuaCreatureObject::getForceSensitiveSkillCount(lua_State* L) {
	bool includeNoviceMasterBoxes = lua_toboolean(L, -1);

	int result = SkillManager::instance()->getForceSensitiveSkillCount(realObject, includeNoviceMasterBoxes);

	lua_pushnumber(L, result);

	return 1;
}

int LuaCreatureObject::villageKnightPrereqsMet(lua_State* L) {
	String skillToDrop = lua_tostring(L, -1);

	bool result = SkillManager::instance()->villageKnightPrereqsMet(realObject, skillToDrop);

	lua_pushboolean(L, result);

	return 1;
}

int LuaCreatureObject::getDamageDealerList(lua_State* L) {
	ThreatMap* threatMap = realObject->getThreatMap();
	ThreatMap copyThreatMap(*threatMap);

	lua_newtable(L);

	int count = 0;
	for (int i = 0; i < copyThreatMap.size(); ++i) {
		ThreatMapEntry* entry = &copyThreatMap.elementAt(i).getValue();

		if (entry->getTotalDamage() > 0) {
			TangibleObject* attacker = copyThreatMap.elementAt(i).getKey();

			if (attacker == nullptr || !attacker->isCreatureObject()) {
				continue;
			}

			CreatureObject* creoAttacker = attacker->asCreatureObject();

			count++;
			lua_pushlightuserdata(L, creoAttacker);
			lua_rawseti(L, -2, count);
		}
	}

	return 1;
}

int LuaCreatureObject::getHealingThreatList(lua_State* L) {
	ThreatMap* threatMap = realObject->getThreatMap();
	ThreatMap copyThreatMap(*threatMap);

	lua_newtable(L);

	int count = 0;

	for (int i = 0; i < copyThreatMap.size(); ++i) {
		ThreatMapEntry* entry = &copyThreatMap.elementAt(i).getValue();

		if (entry == nullptr || entry->getHeal() <= 0)
			continue;

		TangibleObject* entryTano = copyThreatMap.elementAt(i).getKey();

		if (entryTano == nullptr || !entryTano->isCreatureObject()) {
			continue;
		}

		CreatureObject* creoHealer = entryTano->asCreatureObject();

		count++;
		lua_pushlightuserdata(L, creoHealer);
		lua_rawseti(L, -2, count);
	}

	return 1;
}

int LuaCreatureObject::getAllThreatsList(lua_State* L) {
	ThreatMap* threatMap = realObject->getThreatMap();
	ThreatMap copyThreatMap(*threatMap);

	lua_newtable(L);
	int count = 0;

	for (int i = 0; i < copyThreatMap.size(); ++i) {
		TangibleObject* attacker = copyThreatMap.elementAt(i).getKey();

		if (attacker == nullptr || !attacker->isCreatureObject()) {
			continue;
		}

		CreatureObject* creoAttacker = attacker->asCreatureObject();

		count++;
		lua_pushlightuserdata(L, creoAttacker);
		lua_rawseti(L, -2, count);
	}

	return 1;
}

int LuaCreatureObject::dropFromThreatMap(lua_State* L) {
	TangibleObject* attackerTano = (TangibleObject*)lua_touserdata(L, -1);

	if (attackerTano == nullptr)
		return 0;

	Locker lock(realObject);

	ThreatMap* threatMap = realObject->getThreatMap();

	if (threatMap == nullptr)
		return 0;

	for (int i = 0; i < threatMap->size(); i++) {
		TangibleObject* threatTano = threatMap->elementAt(i).getKey();

		if (threatTano == nullptr)
			continue;

		if (threatTano == attackerTano) {
			threatMap->remove(i);
			return 0;
		}
	}

	return 0;
}

int LuaCreatureObject::getSkillMod(lua_State* L) {
	String skillMod = lua_tostring(L, -1);

	int result = realObject->getSkillMod(skillMod);

	lua_pushnumber(L, result);

	return 1;
}

int LuaCreatureObject::getGender(lua_State* L) {

	lua_pushnumber(L, realObject->getGender());

	return 1;
}

int LuaCreatureObject::isRidingMount(lua_State* L) {
	bool isMounted = realObject->isRidingMount();

	lua_pushboolean(L, isMounted);

	return 1;
}

// Infinity: Restored from 3.0.0 for space station support
int LuaCreatureObject::isPilotingShip(lua_State* L) {
	bool isPiloting = realObject->isPilotingShip();

	lua_pushboolean(L, isPiloting);

	return 1;
}

// Infinity: JTL Lua methods

int LuaCreatureObject::setSpawnerID(lua_State* L) {
	// TODO: Requires CreatureObject IDL change to add spawnerID field
	realObject->info(true) << "LuaCreatureObject::setSpawnerID - not yet implemented (needs IDL change)";
	return 0;
}

int LuaCreatureObject::getSpawnerID(lua_State* L) {
	// TODO: Requires CreatureObject IDL change to add spawnerID field
	lua_pushnumber(L, 0);
	return 1;
}

int LuaCreatureObject::storePets(lua_State* L) {
	Locker lock(realObject);

	ManagedReference<SceneObject*> datapad = realObject->getSlottedObject("datapad");

	if (datapad == nullptr) {
		return 0;
	}

	Vector<ManagedReference<ControlDevice*> > devicesToStore;

	for (int i = 0; i < datapad->getContainerObjectsSize(); i++) {
		ManagedReference<SceneObject*> object = datapad->getContainerObject(i);

		if (object == nullptr || !object->isPetControlDevice()) {
			continue;
		}

		ControlDevice* device = cast<ControlDevice*>(object.get());

		if (device != nullptr) {
			devicesToStore.add(device);
		}
	}

	StoreSpawnedChildrenTask* task = new StoreSpawnedChildrenTask(realObject, std::move(devicesToStore));

	if (task != nullptr) {
		task->execute();
	}

	return 0;
}

int LuaCreatureObject::isRebelPilot(lua_State* L) {
	bool check = realObject->hasSkill("pilot_rebel_navy_novice");
	lua_pushboolean(L, check);
	return 1;
}

int LuaCreatureObject::isImperialPilot(lua_State* L) {
	bool check = realObject->hasSkill("pilot_imperial_navy_novice");
	lua_pushboolean(L, check);
	return 1;
}

int LuaCreatureObject::isNeutralPilot(lua_State* L) {
	bool check = realObject->hasSkill("pilot_neutral_novice");
	lua_pushboolean(L, check);
	return 1;
}

int LuaCreatureObject::hasCertifiedShip(lua_State* L) {
	bool skipYacht = lua_toboolean(L, -1);

	auto datapad = realObject->getSlottedObject("datapad");
	bool hasShip = false;

	if (datapad != nullptr) {
		for (int i = 0; i < datapad->getContainerObjectsSize(); i++) {
			ManagedReference<SceneObject*> object = datapad->getContainerObject(i);

			if (object == nullptr || !object->isShipControlDevice()) {
				continue;
			}

			if (skipYacht && object->getServerObjectCRC() == STRING_HASHCODE("object/intangible/ship/sorosuub_space_yacht_pcd.iff")) {
				continue;
			}

			auto shipDevice = object.castTo<ShipControlDevice*>();

			if (shipDevice == nullptr) {
				continue;
			}

			auto controlledObject = shipDevice->getControlledObject();

			if (controlledObject == nullptr) {
				continue;
			}

			auto ship = controlledObject->asShipObject();

			if (ship == nullptr || !ship->canBePilotedBy(realObject)) {
				continue;
			}

			hasShip = true;
			break;
		}
	}

	lua_pushboolean(L, hasShip);

	return 1;
}

int LuaCreatureObject::failQuestMission(lua_State* L) {
	// TODO: Requires MissionObject IDL change (getQuestCRC, isAborted) and MissionManager (handleMissionFail)
	realObject->info(true) << "LuaCreatureObject::failQuestMission - not yet implemented (needs IDL changes)";
	return 0;
}

int LuaCreatureObject::removeQuestMission(lua_State* L) {
	// TODO: Requires MissionObject IDL change (getQuestCRC)
	realObject->info(true) << "LuaCreatureObject::removeQuestMission - not yet implemented (needs IDL changes)";
	return 0;
}

int LuaCreatureObject::addSpaceMissionObject(lua_State* L) {
	// TODO: Requires CreatureObject IDL change to add addSpaceMissionObject method
	realObject->info(true) << "LuaCreatureObject::addSpaceMissionObject - not yet implemented (needs IDL change)";
	return 0;
}

int LuaCreatureObject::removeSpaceMissionObject(lua_State* L) {
	// TODO: Requires CreatureObject IDL change to add removeSpaceMissionObject method
	realObject->info(true) << "LuaCreatureObject::removeSpaceMissionObject - not yet implemented (needs IDL change)";
	return 0;
}

int LuaCreatureObject::dismount(lua_State* L) {
	realObject->dismount();
	return 0;
}

int LuaCreatureObject::setAppearance(lua_State* L){
	String appearanceString = lua_tostring(L, -1);

	Locker pLocker(realObject);

	// Reset Template - Pass empty string
	if (appearanceString == "") {
		Zone* zone = realObject->getZone();

		realObject->setAlternateAppearance(appearanceString , true);

		if (zone != nullptr) {
			realObject->switchZone(zone->getZoneName(), realObject->getPositionX(), realObject->getPositionZ(), realObject->getPositionY(), realObject->getParentID());
		}
		return 0;
	}

	String templateName = "";

	if (appearanceString.indexOf(".iff") == -1 || appearanceString.indexOf("object/mobile/shared_") == -1) {
		return 0;
	} else if (appearanceString != "") {
		TemplateManager* templateManager = TemplateManager::instance();
		String templateTest = appearanceString.replaceFirst("shared_", "");

		if (templateManager != nullptr) {
			SharedObjectTemplate* templateData = templateManager->getTemplate(templateTest.hashCode());

			if (templateData == nullptr) {
				realObject->sendSystemMessage("Unable to find template.");
				return 0;
			}
			templateName = appearanceString;

			realObject->setAlternateAppearance(templateName, true);
		}
	}

	return 0;
}

int LuaCreatureObject::getWeaponType(lua_State* L) {
	Locker lock(realObject);

	WeaponObject* weapon = realObject->getWeapon();
	uint32 weaponType;

	if (weapon == nullptr) {
		weaponType = SharedWeaponObjectTemplate::UNARMEDWEAPON;
	} else {
		weaponType = weapon->getWeaponBitmask();
	}

	lua_pushinteger(L, weaponType);
	return 1;
}

int LuaCreatureObject::attemptPeace(lua_State* L) {
	Locker lock(realObject);

	CombatManager::instance()->attemptPeace(realObject);

	return 0;
}

int LuaCreatureObject::forcePeace(lua_State* L) {
	Locker lock(realObject);

	CombatManager::instance()->forcePeace(realObject);

	return 0;
}

int LuaCreatureObject::gmenhanceCharacter(lua_State* L) {

	if (realObject == nullptr) {
		Logger::console.error("LuaCreatureObject::gmenhanceCharacter - realObject is nullptr");
		return 0;
	}

	auto zoneServer = realObject->getZoneServer();
	if (zoneServer == nullptr) {
		Logger::console.error("LuaCreatureObject::gmenhanceCharacter - zoneServer is nullptr");
		return 0;
	}

	auto playerManager = zoneServer->getPlayerManager();
	if (playerManager == nullptr) {	
		Logger::console.error("LuaCreatureObject::gmenhanceCharacter - playermanager is nullptr");
		return 0;
	}

	playerManager->gmenhanceCharacter(realObject, 3500);

	return 0;
}

int LuaCreatureObject::enhanceCreature(lua_State* L) {

	if (realObject == nullptr) {
		Logger::console.error("LuaCreatureObject::enhanceCreature - realObject is nullptr");
		return 0;
	}

	int medicalBuffAmount, performanceBuffAmount = 0;
	int durationSecs = 1800;
	bool buffed = false;
	int numberOfArguments = lua_gettop(L) - 1;

	if (numberOfArguments == 3) {

		medicalBuffAmount = lua_tointeger(L, -3);
		performanceBuffAmount = lua_tointeger(L, -2);
		durationSecs = lua_tointeger(L, -1);

		auto zoneServer = realObject->getZoneServer();
		if (zoneServer == nullptr) {
			Logger::console.error("LuaCreatureObject::enhanceCreature - zoneServer is nullptr");
			return 0;
		}

		auto playerManager = zoneServer->getPlayerManager();
		if (playerManager == nullptr) {
			Logger::console.error("LuaCreatureObject::enhanceCreature - PlayerManager is nullptr");
			return 1;
		}
		Locker locker(realObject);
		buffed = playerManager->enhanceCreature(realObject, medicalBuffAmount, performanceBuffAmount, durationSecs);

		lua_pushboolean(L, buffed);
	}
	else
		Logger::console.error("LuaCreatureObject::enhanceCreature - incorrect number of arguments (3 required).");

	return 1;
}

int LuaCreatureObject::resetBuffs(lua_State* L) {

	bool clearFilling = false;

	int numberOfArguments = lua_gettop(L) - 1;

	if (numberOfArguments == 1) {
		clearFilling = lua_toboolean(L, -1);
	}

	Locker locker(realObject);
	realObject->clearBuffs(true, true);

	if (clearFilling) {

		auto ghost = realObject->getPlayerObject();

		if (ghost != nullptr) {
			ghost->setFoodFilling(0);
			ghost->setDrinkFilling(0);
		}
	}

	return 0;
}

int LuaCreatureObject::clearDots(lua_State* L) {

	if (realObject != nullptr) {
		Locker locker(realObject);
		realObject->clearDots();
	}
	return 0;
}

int LuaCreatureObject::removeFeignedDeath(lua_State* L) {

	realObject->removeFeignedDeath();

	return 0;
}

int LuaCreatureObject::addBuff(lua_State* L) {

	unsigned int buffCRC = lua_tointeger(L, -3);
	int duration = lua_tointeger(L, -2);
	int strength = lua_tointeger(L, -1);
	bool result = false;

	if (buffCRC <= 0 || !realObject->isCreatureObject()) {
		result = false;
		return 1;
	}

	Buff* buff = realObject->getBuff(buffCRC);

	if (buff != nullptr) { //buff currently active, extend
		Locker locker(realObject);
		realObject->renewBuff(buffCRC, duration, true);
		result = true;
		return 1;
	}

	switch (buffCRC) {
		case BuffCRC::JEDI_CLOAKOFPAIN: {
			ManagedReference<Buff*> cloakBuff = new Buff(realObject, buffCRC, duration, BuffType::JEDI);
			Locker blocker(cloakBuff);
			cloakBuff->setSkillModifier("cloakofpain", strength);
			Locker locker(realObject);
			realObject->addBuff(cloakBuff);
			result = true;
			break;
		}

		case BuffCRC::JEDI_CORTOSIS: {
			ManagedReference<Buff*> cortosisBuff = new Buff(realObject, buffCRC, duration, BuffType::JEDI);
			Locker blocker(cortosisBuff);
			cortosisBuff->setSkillModifier("jedi_force_power_regen", strength);   //Strength should be negative number to debuff force regen
			Locker locker(realObject);
			realObject->addBuff(cortosisBuff);
			result = true;
			break;
		}

		// Infinity: Death Watch Bacta Denial - blocks stimpack usage
		case BuffCRC::DW_BACTA_DENIAL: {
			ManagedReference<Buff*> bactaDenialBuff = new Buff(realObject, buffCRC, duration, BuffType::OTHER);
			Locker blocker(bactaDenialBuff);
			// No skill modifiers needed - just a marker buff checked by StimPack.idl
			Locker locker(realObject);
			realObject->addBuff(bactaDenialBuff);
			result = true;
			break;
		}
	}

	lua_pushboolean(L, result);
	return 1;
	
}

int LuaCreatureObject::hasBuff(lua_State* L) {

	unsigned int buffCRC = lua_tointeger(L, -1);
	bool result = false;

	if (buffCRC <= 0 || !realObject->isCreatureObject()) {
		lua_pushboolean(L, result);  // return false
		return 1;
	}

	result = realObject->hasBuff(buffCRC);

	lua_pushboolean(L, result);
	return 1;
	
}

int LuaCreatureObject::removeBuff(lua_State* L) {

	unsigned int buffCRC = lua_tointeger(L, -1);
	bool result = false;

	if (buffCRC <= 0 || !realObject->isCreatureObject()) {
		lua_pushboolean(L, result);  // return false
		return 1;
	}

	Locker locker(realObject);
	result = realObject->removeBuff(buffCRC);
	
	lua_pushboolean(L, result);
	return 1;
	
}

int LuaCreatureObject::getBuffAmount(lua_State* L) {

	uint8 attribute = lua_tointeger(L, -1);
	int amount = 0;

	if ((attribute > 8) || !realObject->isCreatureObject()) {
		Logger::console.error("LuaCreatureObject::getBuffAmount - invalid attribute or creature");
		lua_pushinteger(L, amount);  // return 0
		return 1;
	}

	unsigned int buffCRC = 0;

	switch (attribute) {
		case BuffAttribute::HEALTH:
			buffCRC = BuffCRC::MEDICAL_ENHANCE_HEALTH;
			break;
		case BuffAttribute::ACTION:
			buffCRC = BuffCRC::MEDICAL_ENHANCE_ACTION;
			break;
		case BuffAttribute::STRENGTH:
			buffCRC = BuffCRC::MEDICAL_ENHANCE_STRENGTH;
			break;
		case BuffAttribute::CONSTITUTION:
			buffCRC = BuffCRC::MEDICAL_ENHANCE_CONSTITUTION;
			break;
		case BuffAttribute::QUICKNESS:
			buffCRC = BuffCRC::MEDICAL_ENHANCE_QUICKNESS;
			break;
		case BuffAttribute::STAMINA:
			buffCRC = BuffCRC::MEDICAL_ENHANCE_STAMINA;
			break;
		case BuffAttribute::MIND:
			buffCRC = BuffCRC::PERFORMANCE_ENHANCE_DANCE_MIND;
			break;
		case BuffAttribute::FOCUS:
			buffCRC = BuffCRC::PERFORMANCE_ENHANCE_MUSIC_FOCUS;
			break;
		case BuffAttribute::WILLPOWER:
			buffCRC = BuffCRC::PERFORMANCE_ENHANCE_MUSIC_WILLPOWER;
			break;

	}

	if (buffCRC == 0) {
		Logger::console.error("LuaCreatureObject::getBuffAmount - invalid attribute, no CRC found");
		lua_pushinteger(L, amount);  // return 0
		return 1;
	}

	Locker locker(realObject);

	if (realObject->hasBuff(buffCRC)) {

		Buff* buff = realObject->getBuff(buffCRC);

		if (buff != nullptr) {
			amount = buff->getAttributeModifierValue(attribute);
		}
	}

	lua_pushinteger(L, amount);
	return 1;
}

int LuaCreatureObject::getBuffDuration(lua_State* L) {

	uint8 attribute = lua_tointeger(L, -1);
	float duration = 0;

	if ((attribute > 8) || !realObject->isCreatureObject()) {
		Logger::console.error("LuaCreatureObject::getBuffDuration - invalid attribute or creature");
		lua_pushnumber(L, duration);  // return 0
		return 1;
	}

	unsigned int buffCRC = 0;

	switch (attribute) {
		case BuffAttribute::HEALTH:
			buffCRC = BuffCRC::MEDICAL_ENHANCE_HEALTH;
			break;
		case BuffAttribute::ACTION:
			buffCRC = BuffCRC::MEDICAL_ENHANCE_ACTION;
			break;
		case BuffAttribute::STRENGTH:
			buffCRC = BuffCRC::MEDICAL_ENHANCE_STRENGTH;
			break;
		case BuffAttribute::CONSTITUTION:
			buffCRC = BuffCRC::MEDICAL_ENHANCE_CONSTITUTION;
			break;
		case BuffAttribute::QUICKNESS:
			buffCRC = BuffCRC::MEDICAL_ENHANCE_QUICKNESS;
			break;
		case BuffAttribute::STAMINA:
			buffCRC = BuffCRC::MEDICAL_ENHANCE_STAMINA;
			break;
		case BuffAttribute::MIND:
			buffCRC = BuffCRC::PERFORMANCE_ENHANCE_DANCE_MIND;
			break;
		case BuffAttribute::FOCUS:
			buffCRC = BuffCRC::PERFORMANCE_ENHANCE_MUSIC_FOCUS;
			break;
		case BuffAttribute::WILLPOWER:
			buffCRC = BuffCRC::PERFORMANCE_ENHANCE_MUSIC_WILLPOWER;
			break;

	}

	if (buffCRC == 0) {
		Logger::console.error("LuaCreatureObject::getBuffAmount - invalid attribute, no CRC found");
		lua_pushnumber(L, duration);  // return 0
		return 1;
	}

	Locker locker(realObject);

	if (realObject->hasBuff(buffCRC)) {

		Buff* buff = realObject->getBuff(buffCRC);

		if (buff != nullptr) {
			duration = buff->getBuffDuration();
		}
	}

	lua_pushnumber(L, duration);
	return 1;
		
}

int LuaCreatureObject::getBuffTimeLeft(lua_State* L) {

	uint8 attribute = lua_tointeger(L, -1);
	float timeLeft = 0;

	if ((attribute > 8) || !realObject->isCreatureObject()) {
		Logger::console.error("LuaCreatureObject::getBuffTimeLeft - invalid attribute or creature");
		lua_pushnumber(L, timeLeft);  // return 0
		return 1;
	}

	unsigned int buffCRC = 0;

	switch (attribute) {
		case BuffAttribute::HEALTH:
			buffCRC = BuffCRC::MEDICAL_ENHANCE_HEALTH;
			break;
		case BuffAttribute::ACTION:
			buffCRC = BuffCRC::MEDICAL_ENHANCE_ACTION;
			break;
		case BuffAttribute::STRENGTH:
			buffCRC = BuffCRC::MEDICAL_ENHANCE_STRENGTH;
			break;
		case BuffAttribute::CONSTITUTION:
			buffCRC = BuffCRC::MEDICAL_ENHANCE_CONSTITUTION;
			break;
		case BuffAttribute::QUICKNESS:
			buffCRC = BuffCRC::MEDICAL_ENHANCE_QUICKNESS;
			break;
		case BuffAttribute::STAMINA:
			buffCRC = BuffCRC::MEDICAL_ENHANCE_STAMINA;
			break;
		case BuffAttribute::MIND:
			buffCRC = BuffCRC::PERFORMANCE_ENHANCE_DANCE_MIND;
			break;
		case BuffAttribute::FOCUS:
			buffCRC = BuffCRC::PERFORMANCE_ENHANCE_MUSIC_FOCUS;
			break;
		case BuffAttribute::WILLPOWER:
			buffCRC = BuffCRC::PERFORMANCE_ENHANCE_MUSIC_WILLPOWER;
			break;

	}

	if (buffCRC == 0) {
		Logger::console.error("LuaCreatureObject::getBuffTimeLeft - invalid attribute, no CRC found");
		lua_pushnumber(L, timeLeft);  // return 0
		return 1;
	}

	Locker locker(realObject);

	if (realObject->hasBuff(buffCRC)) {

		Buff* buff = realObject->getBuff(buffCRC);

		if (buff != nullptr) {
			timeLeft = buff->getTimeLeft();
		}

	}

	lua_pushnumber(L, timeLeft);
	return 1;
		
}

int LuaCreatureObject::getPlayerBountyTarget(lua_State* L) {

	uint64 targetID = 0;

	if (realObject != nullptr && realObject->isPlayerCreature()) {

		SceneObject* datapad = realObject->getSlottedObject("datapad");

		for (int i = 0; i < datapad->getContainerObjectsSize(); ++i) {

			SceneObject* obj = datapad->getContainerObject(i);
			if (obj->isMissionObject()) {
				MissionObject* datapadMission = cast<MissionObject*>(obj);
				if (datapadMission->getTypeCRC() == MissionTypes::BOUNTY) {
					targetID = datapadMission->getTargetObjectId();
				}
			}
		}
	}
	else {
		Logger::console.error("LuaCreatureObject::getPlayerBountyTarget - realObject is nullptr or not playerObject");
	}

	lua_pushinteger(L, targetID);

	return 1;
}

int LuaCreatureObject::areInDuel(lua_State* L) {

	bool retVal = false;

	CreatureObject* player2 = (CreatureObject*) lua_touserdata(L, -1);

	if (realObject != nullptr && player2 != nullptr) {

		PlayerObject* ghost1 = realObject->getPlayerObject().get();
		PlayerObject* ghost2 = player2->getPlayerObject().get();

		if (ghost1 != nullptr && ghost2 != nullptr) {
			if (ghost1->requestedDuelTo(player2) && ghost2->requestedDuelTo(realObject))
				retVal = true;
		}
	}

	lua_pushboolean(L, retVal);
	return 1;
}

int LuaCreatureObject::addWounds(lua_State* L) {
	//int CreatureObjectImplementation::addWounds(int type, int value, bool notifyClient, bool doShockWounds)

		if (realObject == nullptr) {
			Logger::console.error("LuaCreatureObject::addWounds - realObject is nullptr");
			return 0;
		}

		int type = lua_tointeger(L, -2);
		int value = lua_tointeger(L, -1);

		Locker locker(realObject);
		realObject->addWounds(type, value, true, true);
	
		return 0;
}

int LuaCreatureObject::addShockWounds(lua_State* L) {
	//void CreatureObjectImplementation::addShockWounds(int shockToAdd, bool notifyClient, bool sendSpam)

		if (realObject == nullptr) {
			Logger::console.error("LuaCreatureObject::addShockWounds - realObject is nullptr");
			return 0;
		}

		int value = lua_tointeger(L, -1);

		Locker locker(realObject);
		realObject->addShockWounds(value, true, true);

		return 0;
}

int LuaCreatureObject::docBuff(lua_State* L) {

		if (realObject == nullptr) {
			Logger::console.error("LuaCreatureObject::docBuff - realObject is nullptr");
			lua_pushnil(L);
			return 1;
		}
		
		int numberOfArguments = lua_gettop(L) - 1;
		CreatureObject* enhancer = nullptr;
		uint8 attribute = 0;

		if (numberOfArguments != 2) {
			Logger::console.error("LuaCreatureObject::docBuff - wrong number of arguments provided");
			lua_pushnil(L);
			return 1;
		}

		enhancer = (CreatureObject*) lua_touserdata(L, -2);
		attribute = lua_tointeger(L, -1);

		if (enhancer == nullptr) {
			lua_pushnil(L);
			return 1;
		}

		if (attribute == BuffAttribute::MIND || attribute == BuffAttribute::FOCUS || attribute == BuffAttribute::WILLPOWER || attribute > BuffAttribute::DISEASE) {
			Logger::console.error("LuaCreatureObject::docBuff - invalid attribute");
			lua_pushnil(L);
			return 1;
		}	

		int result = realObject->docBuff(enhancer, attribute);
		lua_pushinteger(L, result);

		return 1;
}

int LuaCreatureObject::getGuildID(lua_State* L) {

	int retVal = 0;

	if (realObject != nullptr)
		retVal = realObject->getGuildID();

	lua_pushinteger(L, retVal);

	return 1;
}

int LuaCreatureObject::getShockWounds(lua_State* L) {

	int retVal = 0;

	if (realObject != nullptr)
		retVal = realObject->getShockWounds();

	lua_pushinteger(L, retVal);

	return 1;
}

int LuaCreatureObject::isHarvesterDroidPet(lua_State* L) {

	if (realObject == nullptr) {
		Logger::console.error("LuaCreatureObject::isHarvesterDroidPet - realObject is nullptr");
		lua_pushnil(L);
		return 1;
	}

	bool retVal = false;
	if (realObject->isDroidObject() && realObject->isPet()) {
		DroidObject* droid = cast<DroidObject*>(realObject);

		if (droid != nullptr) {
			retVal = droid->isHarvesterDroid();
		}
	}
	lua_pushboolean(L, retVal);
	return 1;
}

int LuaCreatureObject::isInCamp(lua_State* L) {

	if (realObject == nullptr) {
		Logger::console.error("LuaCreatureObject::isInCamp - realObject is nullptr");
		lua_pushnil(L);
		return 1;
	}

	bool retVal = false;
	if (realObject->getCurrentCamp() != nullptr) {
		retVal = true;
	}
	lua_pushboolean(L, retVal);
	return 1;
}

int LuaCreatureObject::isSwimming(lua_State* L) {

	if (realObject == nullptr) {
		Logger::console.error("LuaCreatureObject::isSwimming - realObject is nullptr");
		lua_pushnil(L);
		return 1;
	}

	lua_pushboolean(L, realObject->isSwimming());
	return 1;
}

int LuaCreatureObject::isCityBanned(lua_State *L) {

	bool result = false;

	if (realObject != nullptr) {

		ManagedReference<CityRegion* > region = realObject->getCityRegion().get();
		if (region != nullptr) {
			if (region->isBanned(realObject->getObjectID())) {
				result = true;
			}
		}
	}

	lua_pushboolean(L, result);

	return 1;
}

int LuaCreatureObject::failPlayerBountyMission(lua_State* L) {

	if (realObject != nullptr and realObject->isPlayerCreature() && (realObject->hasSkill("combat_bountyhunter_investigation_03") || realObject->hasSkill("mandalorian_master"))) {

		auto zoneServer = realObject->getZoneServer();
		if(zoneServer != nullptr) {
			auto missionManager = zoneServer->getMissionManager();
			if (missionManager != nullptr) {
				missionManager->failPlayerBountyMission(realObject->getObjectID(), 0);
			}
		}
	}

	return 0;
}

int LuaCreatureObject::createAttachment(lua_State* L) {

	if (realObject == nullptr) {
		Logger::console.error("LuaCreatureObject::createAttachment - realObject is nullptr");
		return 0;
	}

	String attachmentType = "";
	String skillMod = "";
	int skillModValue = 0;
	bool ignoreChecks = false;

	int numberOfArguments = lua_gettop(L) - 1;
	if (numberOfArguments == 3) {
		attachmentType = lua_tostring(L, -3);
		skillMod = lua_tostring(L, -2);
		skillModValue = lua_tointeger(L, -1);
	}
	else if (numberOfArguments == 4) {
		attachmentType = lua_tostring(L, -4);
		skillMod = lua_tostring(L, -3);
		skillModValue = lua_tointeger(L, -2);
		ignoreChecks = lua_toboolean(L, -1);
	}
	else {
		Logger::console.error("LuaCreatureObject::createAttachment incorrect number of arguments passed to LuaCreatureObject::createAttachment");
		return 0;
	}

	if (attachmentType != "AA" && attachmentType != "CA") {
		Logger::console.error("LuaCreatureObject::createAttachment - incorrect attachment type");
		return 0;
	}

	auto zoneServer = realObject->getZoneServer();
	if (zoneServer == nullptr) {
		Logger::console.error("LuaCreatureObject::createAttachment - zoneServer is nullptr");
		return 0;
	}

	auto lootManager = zoneServer->getLootManager();	
	if (lootManager == nullptr) {
		Logger::console.error("LuaCreatureObject::createAttachment - lootManager is nullptr");
		return 0;
	}

	bool validAttachment = lootManager->isValidAttachment(attachmentType, skillMod);

	ManagedReference<SceneObject*> inventory = realObject->getSlottedObject("inventory");

	if (inventory == nullptr) {
		Logger::console.error("LuaCreatureObject::createAttachment - inventory is nullptr");
		return 0;
	}

	if (inventory->isContainerFullRecursive()) {
		realObject->sendSystemMessage("Your inventory is full, so the item could not be created.");
		return 0;
	}

	Reference<const LootItemTemplate*> itemTemplate = nullptr;
	LootGroupMap* lootGroupMap = LootGroupMap::instance();
	if (lootGroupMap == nullptr) {
		Logger::console.error("LuaCreatureObject::createAttachment - lootGroupMap is nullptr");
		return 0;
	}

	if (attachmentType == "AA")
		itemTemplate = lootGroupMap->getLootItemTemplate("attachment_armor");
	else
		itemTemplate = lootGroupMap->getLootItemTemplate("attachment_clothing");
				
	ManagedReference<SceneObject*> SEA = lootManager->createLootAttachment(itemTemplate, skillMod, skillModValue, ignoreChecks); 

	if (SEA != nullptr){
					
		Locker objLocker(SEA);
		if (inventory->transferObject(SEA, -1, true, true)) { //Transfer tape to player inventory
			inventory->broadcastObject(SEA, true);
			if (validAttachment) {
				realObject->sendSystemMessage("Created SEA:  " + SEA->getCustomObjectName().toString());
			}
			else {
				realObject->sendSystemMessage("Created SEA:  " + skillMod + " " + String::valueOf(skillModValue));
			}
			return 0;
		}
		else {
			SEA->destroyObjectFromWorld(true);
			SEA->destroyObjectFromDatabase(true);
			realObject->sendSystemMessage("Unable to place SEA in player's inventory!");
		}		
	}

	realObject->sendSystemMessage("Unable to create SEA with skillmod \"" + skillMod + "\"");

	return 0;
}

int LuaCreatureObject::setSpeedBuff(lua_State* L) {

	if (realObject == nullptr) {
		Logger::console.error("LuaCreatureObject::setSpeedBuff - realObject is nullptr");
		return 0;
	}

	float speed = 1.0f;
	int duration = 0;
	String startMessage = "";
	String endMessage = "";

	int numberOfArguments = lua_gettop(L) - 1;

	switch (numberOfArguments) {
	case 2:
		speed = lua_tonumber(L, -2);
		duration = lua_tointeger(L, -1);
	break;
	case 3:
		speed = lua_tonumber(L, -3);
		duration = lua_tointeger(L, -2);
		startMessage = lua_tostring(L, -1);
	break;
	case 4:
		speed = lua_tonumber(L, -4);
		duration = lua_tointeger(L, -3);
		startMessage = lua_tostring(L, -2);
		endMessage = lua_tostring(L, -1);
	break;
	default:
		Logger::console.error("LuaCreatureObject::setSpeedBuff - invalid number of arguments provided");
		return 0;
	}

	uint32 buffCRC = STRING_HASHCODE("luaSetSpeed");

	Locker locker(realObject);

	realObject->removeSpeedBuffs();		//Remove all speed buffs

	ManagedReference<Buff*> buff = new Buff(realObject, buffCRC, duration, BuffType::OTHER);

	Locker blocker(buff);

	buff->setSpeedMultiplierMod(speed);
	buff->setAccelerationMultiplierMod(speed);

	if (!startMessage.isEmpty()) {

		StringIdChatParameter stringId;
		stringId.setStringId("@player/player_utility:generic_message"); 
		stringId.setTO(startMessage); 
		buff->setStartMessage(stringId);
	}

	if (!endMessage.isEmpty()) {

		StringIdChatParameter stringId;
		stringId.setStringId("@player/player_utility:generic_message"); 
		stringId.setTO(endMessage); 
		buff->setEndMessage(stringId);
	}

	realObject->addBuff(buff);

	return 0;
}

int LuaCreatureObject::setRootedState(lua_State* L) {

	if (realObject == nullptr) {
		Logger::console.error("LuaCreatureObject::setRootedState - realObject is nullptr");
		return 0;
	}

	int duration = lua_tointeger(L, -1);	

	Locker locker(realObject);

	realObject->setRootedState(duration);

	return 0;
}

int LuaCreatureObject::setSnaredState(lua_State* L) {

	if (realObject == nullptr) {
		Logger::console.error("LuaCreatureObject::setSnaredState - realObject is nullptr");
		return 0;
	}

	int duration = lua_tointeger(L, -1);	

	Locker locker(realObject);

	realObject->setSnaredState(duration);

	return 0;
}

int LuaCreatureObject::removeSpeedBuffs(lua_State* L) {

	if (realObject == nullptr) {
		Logger::console.error("LuaCreatureObject::removeSpeedBuffs - realObject is nullptr");
		return 0;
	}

	Locker locker(realObject);

	realObject->removeSpeedBuffs();

	return 0;
}

int LuaCreatureObject::getSkillsHash(lua_State* L) {

	bool includeForceSensitive = false;
	
	int numberOfArguments = lua_gettop(L) - 1;

	if (numberOfArguments == 1) {
		includeForceSensitive = lua_toboolean(L, -1);
	}

	if  (realObject != nullptr) {
		lua_pushinteger(L, SkillManager::instance()->getSkillsHash(realObject, includeForceSensitive));
	}
	else {
		lua_pushnil(L);
	}

	return 1;
}

int LuaCreatureObject::surrenderAllSkills(lua_State* L) {

	if  (realObject != nullptr && realObject->isPlayerCreature()) {
		SkillManager::instance()->surrenderAllSkills(realObject, true, true);  //Infinity:  Remove all but force_sensitive skills, including Force progression
	}

	return 0;
}


int LuaCreatureObject::ejectPlayerFromBuilding(lua_State* L) {

	if (realObject == nullptr) {
		Logger::console.error("LuaCreatureObject::ejectPlayerFromBuilding - realObject is nullptr");
		return 0;
	}

	if (!realObject->isPlayerCreature()) {
		Logger::console.error("LuaCreatureObject::ejectPlayerFromBuilding - realObject is not player creature");
		return 0;
	}

	auto zoneServer = realObject->getZoneServer();
	if (zoneServer == nullptr) {
		Logger::console.error("LuaCreatureObject::ejectPlayerFromBuilding - zoneServer is nullptr");
		return 0;
	}

	auto playerManager = zoneServer->getPlayerManager();
	if (playerManager == nullptr) {	
		Logger::console.error("LuaCreatureObject::ejectPlayerFromBuilding - playermanager is nullptr");
		return 0;
	}

	playerManager->ejectPlayerFromBuilding(realObject);

	return 0;
}

int LuaCreatureObject::addSkillModifierBuff(lua_State* L) {

    String modifier = lua_tostring(L, -3);
	int strength = lua_tointeger(L, -2);
    int duration = lua_tointeger(L, -1);

	if (!realObject->isCreatureObject()) {
		lua_pushboolean(L, false);
		return 1;
	}

    String modifierToHash = "Lua_" + modifier;
    unsigned int buffCRC = modifierToHash.hashCode();

    Locker locker(realObject);

	if (realObject->hasBuff(buffCRC)) { //buff currently active, remove
	    realObject->removeBuff(buffCRC);
	}

    ManagedReference<Buff*> luaBuff = new Buff(realObject, buffCRC, duration, BuffType::OTHER);
    Locker blocker(luaBuff);
    luaBuff->setBuffName(modifierToHash);
    luaBuff->setSkillModifier(modifier, strength);
    realObject->addBuff(luaBuff);

	lua_pushboolean(L, true);
	return 1;
}

int LuaCreatureObject::addAttributeModifierBuff(lua_State* L) {

    uint8 attribute = lua_tointeger(L, -3);
	int strength = lua_tointeger(L, -2);
    int duration = lua_tointeger(L, -1);

	if (!realObject->isCreatureObject()) {
		lua_pushboolean(L, false);
		return 1;
	}

    String modifierToHash = "Lua_attribute_" + CreatureAttribute::getName(attribute);
    unsigned int buffCRC = modifierToHash.hashCode();

    Locker locker(realObject);

	if (realObject->hasBuff(buffCRC)) { //buff currently active, remove
	    realObject->removeBuff(buffCRC);
	}

    ManagedReference<Buff*> luaBuff = new Buff(realObject, buffCRC, duration, BuffType::OTHER);
    Locker blocker(luaBuff);
    luaBuff->setBuffName(modifierToHash);
    luaBuff->setAttributeModifier(attribute, strength);
    realObject->addBuff(luaBuff);

	lua_pushboolean(L, true);
	return 1;
}

int LuaCreatureObject::getHighestDamageGroupLeader(lua_State* L) {
	if (realObject == nullptr) {
		lua_pushnil(L);
	} else {
		ThreatMap* threatMap = realObject->getThreatMap();
		ThreatMap copyThreatMap(*threatMap);

		ManagedReference<CreatureObject*> leader = copyThreatMap.getHighestDamageGroupLeader(); // Note:   This returns either the leader of the group with highest damage, or the ungrouped player with the highest damage

		// getHighestDamageGroupLeader() returns nullptr when the threat map has no qualifying
		// grouped/player leader (e.g. a DOT tick after the threat list cleared). Push nil rather
		// than a null-wrapped light userdata, which would slip a Lua `== nil` guard and panic in
		// LuaSceneObject::_setObject on the next CreatureObject() wrap (GH #1711).
		if (leader == nullptr) {
			lua_pushnil(L);
		} else {
			lua_pushlightuserdata(L, leader);
		}
	}

	return 1;
}

int LuaCreatureObject::feignDeath(lua_State* L) {

	if (realObject == nullptr) {
		Logger::console.error("LuaCreatureObject::feignDeath - realObject is nullptr");
		return 0;
	}

    if (realObject->isPlayerCreature() && !realObject->isRidingMount() && !realObject->isDead() && !realObject->isIncapacitated() && !realObject->isFeigningDeath()) {

        Locker locker(realObject);

        realObject->feignDeath();
    }

	return 0;
}

int LuaCreatureObject::setOwner(lua_State* L) {
	CreatureObject* owner = (CreatureObject*) lua_touserdata(L, -1);

	if (realObject == nullptr || owner == nullptr) {
		lua_pushboolean(L, false);
		return 1;
	}

	Locker locker(realObject);
	realObject->setCreatureLink(owner, true);

	lua_pushboolean(L, true);
	return 1;
}

int LuaCreatureObject::executeCommand(lua_State* L) {
	String commandName = lua_tostring(L, -2);
	uint64 targetID = lua_tonumber(L, -1);

	if (realObject == nullptr || commandName.isEmpty()) {
		lua_pushboolean(L, false);
		return 1;
	}

	Locker locker(realObject);
	realObject->executeObjectControllerAction(commandName.hashCode(), targetID, "");

	lua_pushboolean(L, true);
	return 1;
}

int LuaCreatureObject::addPersonalEnemyFlag(lua_State* L) {
	int numberOfArguments = lua_gettop(L) - 1;

	if (realObject == nullptr) {
		return 0;
	}

	CreatureObject* enemy = nullptr;
	uint64 duration = 0; // Default to permanent

	if (numberOfArguments >= 1) {
		enemy = (CreatureObject*)lua_touserdata(L, -numberOfArguments);
	}

	if (numberOfArguments >= 2) {
		duration = lua_tointeger(L, -1);
	}

	if (enemy == nullptr) {
		return 0;
	}

	Locker locker(realObject);
	Locker crossLocker(enemy, realObject);

	realObject->addPersonalEnemyFlag(enemy, duration);

	return 0;
}

int LuaCreatureObject::removePersonalEnemyFlag(lua_State* L) {
	CreatureObject* enemy = (CreatureObject*)lua_touserdata(L, -1);

	if (realObject == nullptr) {
		return 0;
	}

	if (enemy == nullptr) {
		return 0;
	}

	Locker locker(realObject);
	Locker crossLocker(enemy, realObject);

	realObject->removePersonalEnemyFlag(enemy);

	return 0;
}

int LuaCreatureObject::hasPersonalEnemyFlag(lua_State* L) {
	CreatureObject* enemy = (CreatureObject*)lua_touserdata(L, -1);
	bool result = false;

	if (realObject != nullptr && enemy != nullptr) {
		result = realObject->hasPersonalEnemyFlag(enemy);
	}

	lua_pushboolean(L, result);
	return 1;
}

int LuaCreatureObject::clearState(lua_State* L) {
	uint32 state = (uint32)lua_tonumber(L, -1);

	if (realObject == nullptr) {
		return 0;
	}

	Locker locker(realObject);
	realObject->clearState(state, true);

	return 0;
}
