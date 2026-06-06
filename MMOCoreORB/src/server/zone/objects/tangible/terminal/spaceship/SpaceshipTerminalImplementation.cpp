/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.
*/

#include "server/zone/objects/tangible/terminal/spaceship/SpaceshipTerminal.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/objects/player/PlayerObject.h"
#include "server/zone/Zone.h"
#include "server/zone/managers/planet/PlanetManager.h"
#include "server/zone/objects/intangible/ShipControlDevice.h"
#include "server/zone/packets/object/JtlShipListResponse.h"

int SpaceshipTerminalImplementation::handleObjectMenuSelect(CreatureObject* player, byte selectedID) {
	// Infinity: Handle Use (20) to show ship list - the client used to send JtlShipListRequest
	// directly but now routes through the radial system
	if (selectedID == 20) {
		if (player == nullptr || !player->isPlayerCreature()) {
			return 1;
		}

		JtlShipListResponse* response = new JtlShipListResponse(player, asSceneObject());
		player->sendMessage(response);
		return 0;
	}

	// Infinity: Delegate other radial options to the objectMenuComponent (SpaceTerminalMenuComponent)
	return SceneObjectImplementation::handleObjectMenuSelect(player, selectedID);
}
