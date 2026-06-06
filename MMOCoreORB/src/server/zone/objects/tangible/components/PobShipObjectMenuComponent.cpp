/*
 * PobShipObjectMenuComponent.cpp
 *
 * Created on: 1/16/2024
 * Author: Hakry
 */

#include "PobShipObjectMenuComponent.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/packets/object/ObjectMenuResponse.h"
#include "server/zone/objects/ship/ShipObject.h"
#include "server/zone/SpaceZone.h"
#include "templates/params/creature/CreatureState.h"
#include "templates/params/creature/PlayerArrangement.h"

void PobShipObjectMenuComponent::fillObjectMenuResponse(SceneObject* sceneObject, ObjectMenuResponse* menuResponse, CreatureObject* player) const {
	if (!sceneObject->isTangibleObject())
		return;

	auto objectRoot = sceneObject->getRootParent();

	if (objectRoot == nullptr)
		return;

	if (objectRoot->containsChildObject(sceneObject)) {
		// Infinity: Pilot chair shows "Pilot Ship" when player is inside the POB ship
		if (sceneObject->isPilotChair() && player != nullptr && objectRoot->isPobShip()) {
			menuResponse->addRadialMenuItem(200, 3, "Pilot Ship");
		}

		return;
	}

	TangibleObjectMenuComponent::fillObjectMenuResponse(sceneObject, menuResponse, player);
}

int PobShipObjectMenuComponent::handleObjectMenuSelect(SceneObject* sceneObject, CreatureObject* player, byte selectedID) const {
	// Infinity: Handle pilot chair re-entry
	if (selectedID == 200 && sceneObject->isPilotChair() && player != nullptr) {
		ManagedReference<SceneObject*> rootParent = player->getRootParent();

		if (rootParent == nullptr || !rootParent->isPobShip())
			return 1;

		ShipObject* ship = cast<ShipObject*>(rootParent.get());

		if (ship == nullptr)
			return 1;

		Locker sLock(ship, player);

		if (ship->getPilot() != nullptr)
			return 1;

		auto pilotChair = ship->getPilotChair().get();

		if (pilotChair == nullptr)
			return 1;

		player->clearSpaceStates();
		player->setState(CreatureState::PILOTINGPOBSHIP);
		player->setDirection(*pilotChair->getDirection());

		Vector3 chairLocation = pilotChair->getPosition();

		auto spaceZone = dynamic_cast<SpaceZone*>(ship->getZone());

		if (spaceZone != nullptr) {
			player->switchZone(spaceZone->getZoneName(), chairLocation.getX(), chairLocation.getZ() + 0.5, chairLocation.getY(), pilotChair->getObjectID(), false, PlayerArrangement::SHIP_PILOT_POB);
			ship->addPlayerOnBoard(player);
		}

		return 0;
	}

	return TangibleObjectMenuComponent::handleObjectMenuSelect(sceneObject, player, selectedID);
}
