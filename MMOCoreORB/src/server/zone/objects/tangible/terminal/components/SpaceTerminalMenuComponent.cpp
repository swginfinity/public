/*
 * SpaceTerminalMenuComponent.cpp
 *
 *  Created on: Jan 30, 2026
 *
 *  Infinity: Adds ETA-1 instant travel option to space terminals
 *  for players who have an ETA-1 ship in their datapad.
 */

#include "SpaceTerminalMenuComponent.h"
#include "server/zone/packets/object/ObjectMenuResponse.h"
#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/managers/director/DirectorManager.h"
#include "engine/lua/LuaFunction.h"

// Infinity: ETA-1 travel radial ID
static const int ETA1_TRAVEL = 200;
static const String ETA1_TEMPLATE = "object/tangible/mission/eta1_ship_usable.iff";

void SpaceTerminalMenuComponent::fillObjectMenuResponse(SceneObject* sceneObject, ObjectMenuResponse* menuResponse, CreatureObject* player) const {
	if (player == nullptr || !player->isPlayerCreature()) {
		return;
	}

	// Infinity: Check if player has an ETA-1 ship in datapad
	auto datapad = player->getSlottedObject("datapad");

	if (datapad == nullptr) {
		return;
	}

	uint32 eta1CRC = ETA1_TEMPLATE.hashCode();

	for (int i = 0; i < datapad->getContainerObjectsSize(); i++) {
		auto item = datapad->getContainerObject(i);

		if (item != nullptr && item->getServerObjectCRC() == eta1CRC) {
			menuResponse->addRadialMenuItem(ETA1_TRAVEL, 3, "ETA-1 Travel");
			break;
		}
	}
}

int SpaceTerminalMenuComponent::handleObjectMenuSelect(SceneObject* sceneObject, CreatureObject* player, byte selectedID) const {
	// Infinity: ETA-1 instant travel — invoke Lua SUI
	if (selectedID == ETA1_TRAVEL) {
		if (player == nullptr || !player->isPlayerCreature()) {
			return 1;
		}

		Lua* lua = DirectorManager::instance()->getLuaInstance();

		if (lua != nullptr) {
			Reference<LuaFunction*> luaFunc = lua->createFunction("instant_travel_sui", "showMainPage", 0);
			*luaFunc << player;
			luaFunc->callFunction();
		}

		return 0;
	}

	return 0;
}
