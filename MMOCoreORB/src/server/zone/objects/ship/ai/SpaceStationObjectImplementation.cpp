/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.
*/

#include "server/zone/objects/ship/ai/SpaceStationObject.h"
#include "server/zone/ZoneServer.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/packets/object/ObjectMenuResponse.h"

float SpaceStationObjectImplementation::getOutOfRangeDistance(uint64 specialRangeID) {
	return ZoneServer::SPACESTATIONRANGE;
}

void SpaceStationObjectImplementation::fillObjectMenuResponse(ObjectMenuResponse* menuResponse, CreatureObject* player) {
}

int SpaceStationObjectImplementation::handleObjectMenuSelect(CreatureObject* player, byte selectedID) {
	return 0;
}
