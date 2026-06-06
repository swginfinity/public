/*
 * LuaTangibleObject.cpp
 *
 *  Created on: 24/01/2012
 *      Author: xyborn
 */

#ifndef LUATANGIBLEOBJECT_H_
#define LUATANGIBLEOBJECT_H_

#include "engine/engine.h"

#include "server/zone/objects/scene/LuaSceneObject.h"

namespace server {
namespace zone {
namespace objects {
namespace tangible {
	class TangibleObject;

	class LuaTangibleObject : public LuaSceneObject {
	public:
		static const char className[];
		static Luna<LuaTangibleObject>::RegType Register[];

		LuaTangibleObject(lua_State *L);
		~LuaTangibleObject();

		int _setObject(lua_State* L);
		int getOptionsBitmask(lua_State *L);
		int setOptionsBitmask(lua_State *L);
		int setPvpStatusBitmask(lua_State *L);
		int setPvpStatusBit(lua_State *L);
		int clearPvpStatusBit(lua_State *L);
		int getPvpStatusBitmask(lua_State *L);
		int broadcastPvpStatusBitmask(lua_State *L);
		int sendPvpStatusTo(lua_State *L);
		int isChangingFactionStatus(lua_State* L);
		int getFactionStatus(lua_State* L);
		int setFutureFactionStatus(lua_State* L);
		int isOnLeave(lua_State* L);
		int isOvert(lua_State* L);
		int isCovert(lua_State* L);
		int setCustomizationVariable(lua_State *L);
		int getPaletteColorCount(lua_State *L);
		int setConditionDamage(lua_State *L);
		int setMaxCondition(lua_State *L);
		int setFaction(lua_State *L);
		int setFactionStatus(lua_State* L);
		int getFaction(lua_State *L);
		int isImperial(lua_State *L);
		int isRebel(lua_State *L);
		int isNeutral(lua_State *L);
		int hasActiveArea(lua_State* L);
		int isInvisible(lua_State* L);
		int getLuaStringData(lua_State *L);
		int setLuaStringData(lua_State *L);
		int deleteLuaStringData(lua_State *L);
		int setOptionBit(lua_State* L);
		int clearOptionBit(lua_State* L);
		int hasOptionBit(lua_State* L);
		int getCraftersName(lua_State* L);
		int getJunkDealerNeeded(lua_State* L);
		int getJunkValue(lua_State* L);
		int isBroken(lua_State* L);
		int isSliced(lua_State* L);
		int isNoTrade(lua_State* L);
		int getMainDefender(lua_State* L);
		int getConditionDamage(lua_State* L);
		int isActivated(lua_State* L);

		// Infinity:  Custom hooks below
		int isDestroyed(lua_State* L);
		int getFishLength(lua_State *L);
		int getFishPlanet(lua_State *L);
		int getUseCount(lua_State* L);
		int setUseCount(lua_State* L);
		int getMinDamage(lua_State* L);
		int getMaxDamage(lua_State* L);
		int getAttackSpeed(lua_State* L);
		int inflictDamage(lua_State* L);
		int setDamageSlice(lua_State* L);
		int setSpeedSlice(lua_State* L);
		int setEffectivenessSlice(lua_State* L);
		int setEncumbranceSlice(lua_State* L);
		int isSliceable(lua_State* L);
		int getSerialNumber(lua_State* L);
        int hasDefender(lua_State* L);
		int startSlicingSession(lua_State* L);
		int containsActiveSession(lua_State* L);
		int getMaxCondition(lua_State* L);
		int getAppearanceTemplate(lua_State* L);
        int getAttributeValue(lua_State* L);
        int changeAttributeValue(lua_State* L);

	protected:
		TangibleObject* realObject;
	};

}
}
}
}

using namespace server::zone::objects::tangible;


#endif /* LUATANGIBLEOBJECT_H_ */
