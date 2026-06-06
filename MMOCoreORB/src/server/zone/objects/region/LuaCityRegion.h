
#ifndef LUACITYREGION_H_
#define LUACITYREGION_H_

#include "engine/engine.h"

namespace server {
namespace zone {
namespace objects {
namespace region {
	class CityRegion;

	class LuaCityRegion {
	public:
		static const char className[];
		static Luna<LuaCityRegion>::RegType Register[];

		LuaCityRegion(lua_State *L);
		~LuaCityRegion();

		int _setObject(lua_State* L);
		int _getObject(lua_State* L);
		int isClientRegion(lua_State* L);
		// Infinity:  Custom hooks
		int isCityDecoration(lua_State* L);
		int removeDecoration(lua_State* L);
		int isGCWBaseInsideRadius(lua_State* L);
		int getRadius(lua_State* L);
		int addToTreasury(lua_State* L);
		int isCitizen(lua_State* L);
		int removeAllBannedPlayers(lua_State* L);
		int isMayor(lua_State* L);
		int getObjectID(lua_State* L);

	protected:
		CityRegion* _getRealCityRegion() {
			return realObject.get();
		}

	private:
		Reference<CityRegion*> realObject;
	};

}
}
}
}

using namespace server::zone::objects::region;

#endif /* LUACITYREGION_H_ */
