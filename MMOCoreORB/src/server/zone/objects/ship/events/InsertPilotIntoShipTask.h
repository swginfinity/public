/*
 * InsertPilotIntoShipTask.h
 */

#ifndef INSERTPILOTINTOSHIPTASK_H_
#define INSERTPILOTINTOSHIPTASK_H_

#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/SpaceZone.h"
#include "server/zone/objects/ship/PobShipObject.h"
#include "server/zone/objects/player/PlayerObject.h"
#include "templates/params/creature/PlayerArrangement.h"

class InsertPilotIntoShipTask : public Task, public Logger {
	ManagedWeakReference<CreatureObject*> play;
	ManagedWeakReference<ShipObject*> shipObject;

public:
	InsertPilotIntoShipTask(CreatureObject* creo, ShipObject* shipObj) : play(creo), shipObject(shipObj) {
		setLoggingName("InsertPlayerToShipTask");
	}

	void run() {
		ManagedReference<CreatureObject*> player = play.get();
		ManagedReference<ShipObject*> ship = shipObject.get();

		if (player == nullptr || ship == nullptr) {
			Logger::console.error("InsertPilotIntoShipTask -- player or ship is null");
			return;
		}

		auto zone = ship->getLocalZone();

		if (zone == nullptr) {
			player->error() << "InsertPilotIntoShipTask -- ship localZone is null, shipID: " << ship->getObjectID();
			return;
		}

		player->info(true) << "InsertPilotIntoShipTask -- starting. Ship: " << ship->getObjectID() << " isPob: " << ship->isPobShip() << " zone: " << zone->getZoneName();

		// Lock the ship
		Locker lock(ship);

		// Cross lock the player to be inserted as the pilot
		Locker clock(player, ship);

		// Update the players group
		if (player->isGrouped()) {
			auto group = player->getGroup();

			if (group == nullptr) {
				player->error() << "InsertPilotIntoShipTask -- player grouped but group is null";
				return;
			}

			Locker glocker(group, ship);

			group->updateMemberShip(player, ship);
		}

		if (ship->isPobShip()) {
			auto pilotChair = ship->getPilotChair().get();

			if (pilotChair == nullptr) {
				player->error() << "InsertPilotIntoShipTask -- POB ship pilotChair is null";
				return;
			}

			player->info(true) << "InsertPilotIntoShipTask -- POB ship, pilotChair: " << pilotChair->getObjectID() << " calling switchZone to " << zone->getZoneName();

			// Must send the cell permissions prior to player inserting or it will crash the client
			ship->sendContainerObjectsTo(player, true);

			// Always apply the piloting state
			player->setState(CreatureState::PILOTINGPOBSHIP);

			// set pob pilot to the same direction as the chair
			player->setDirection(*ship->getDirection());

			player->switchZone(zone->getZoneName(), 0.0f, 0.5f, 0.0f, pilotChair->getObjectID(), false, PlayerArrangement::SHIP_PILOT_POB);
		} else {
			player->info(true) << "InsertPilotIntoShipTask -- non-POB ship, calling switchZone to " << zone->getZoneName() << " parentID: " << ship->getObjectID() << " pos: (" << ship->getPositionX() << ", " << ship->getPositionZ() << ", " << ship->getPositionY() << ")";

			ship->populateDroidCommands(player);

			player->setState(CreatureState::PILOTINGSHIP);

			// Set the pilots direction to the same as the ship
			player->setDirection(*ship->getDirection());

			player->switchZone(zone->getZoneName(), ship->getPositionX(), ship->getPositionZ(), ship->getPositionY(), ship->getObjectID(), false, PlayerArrangement::SHIP_PILOT);
		}

		player->info(true) << "InsertPilotIntoShipTask -- switchZone called, adding player on board";

		ship->addPlayerOnBoard(player);
	}
};

#endif /* INSERTPILOTINTOSHIPTASK_H_ */
