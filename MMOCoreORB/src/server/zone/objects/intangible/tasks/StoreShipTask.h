/*
 * StoreShipTask.h
 */

#ifndef STORESHIPTASK_H_
#define STORESHIPTASK_H_

#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/objects/intangible/ShipControlDevice.h"
#include "server/zone/objects/ship/ShipObject.h"
#include "server/zone/objects/ship/PobShipObject.h"
#include "templates/params/creature/PlayerArrangement.h"
#include "server/zone/objects/player/PlayerObject.h"

//#define DEBUG_SHIP_STORE

class StoreShipTask : public Task, public Logger {
	ManagedWeakReference<CreatureObject*> play;
	ManagedWeakReference<ShipControlDevice*> shipDev;
	String zoneName;
	Vector3 coordinates;

public:
	StoreShipTask(CreatureObject* creo, ShipControlDevice* controlDevice, String zName, Vector3 coords) : play(creo), shipDev(controlDevice), zoneName(zName), coordinates(coords) {
		setLoggingName("StoreShipTask");
	}

	void run() {
		ManagedReference<CreatureObject*> player = play.get();
		ManagedReference<ShipControlDevice*> shipControlDevice = shipDev.get();

		if (player == nullptr || shipControlDevice == nullptr)
			return;

		// Infinity: Wait for load screen to finish before storing (LAUNCH-5)
		auto ghost = player->getPlayerObject();

		if (ghost != nullptr && ghost->isOnLoadScreen()) {
			reschedule(2000);
			return;
		}

		ManagedReference<ShipObject*> ship = shipControlDevice->getControlledObject()->asShipObject();

		if (ship == nullptr) {
			return;
		}

#ifdef DEBUG_SHIP_STORE
		info(true) << "StoreShipTask called for Player: " << player->getDisplayedName() << " Ship: " << ship->getDisplayedName() << " Zone: " << zoneName << " Loc: " << coordinates.toString();
#endif

		// Lock the ship
		Locker shipLock(ship);

		// Infinity: Updated for unstable API - getPlayersOnBoard returns Vector<uint64>
		Vector<uint64> playerIDs = ship->getPlayersOnBoard();

#ifdef DEBUG_SHIP_STORE
		info(true) << "StoreShipTask seeing " << playerIDs.size() << " player(s) on board.";
#endif

		auto zoneServer = ship->getZoneServer();

		// This function should remove all players in the ship.
		for (int i = playerIDs.size() - 1; i >= 0; --i) {
			ManagedReference<SceneObject*> obj = zoneServer->getObject(playerIDs.get(i));

			if (obj == nullptr)
				continue;

			ManagedReference<CreatureObject*> shipMember = obj->asCreatureObject();

			if (shipMember == nullptr)
				continue;

			try {
				// Cross lock the player for removal
				Locker playerLock(shipMember, ship);

				if (!removePlayer(shipMember, zoneName, coordinates)) {
					error() << "Failed to remove player from Ship - ShipID: " << ship->getObjectID() << " Player ID: " << shipMember->getObjectID();
					return;
				}
			} catch (...) {
				error() << "Failed to remove player from Ship - ShipID: " << ship->getObjectID() << " Player ID: " << shipMember->getObjectID();
			}
		}

		ship->clearPlayersOnBoard();

		// Infinity: Clear staff speed on store (LAUNCH-6)
		ship->setStaffShipSpeed(0.f);

		// Destroy the ship from the zone.
		ship->destroyObjectFromWorld(false);

		// Lock the device and transfer the ship inside.
		Locker sLock(shipControlDevice, ship);

		if (shipControlDevice->transferObject(ship, PlayerArrangement::RIDER, false, false, false)) {
			ship->cancelRecovery();
			ship->clearOptionBit(OptionBitmask::WINGS_OPEN, true);
			ship->clearPlayersOnBoard();
		}

		shipControlDevice->updateStatus(shipControlDevice->isShipLaunched(), true);

#ifdef DEBUG_SHIP_STORE
		info(true) << "Setting stored postion";
#endif

		Locker cLock(player, shipControlDevice);
		shipControlDevice->setStoredLocationData(player);
		cLock.release();

		if (player->isGrouped()) {
			auto group = player->getGroup();

			if (group == nullptr)
				return;

			Locker glocker(group, ship);

			group->updateMemberShip(player, nullptr);
		}
	}

	// Player is locked coming into this function
	bool removePlayer(CreatureObject* player, String newZoneName, Vector3 location) {
#ifdef DEBUG_SHIP_STORE
		info(true) << "removePlayer called";
#endif

		if (player == nullptr)
			return false;

		auto zoneServer = player->getZoneServer();

		if (zoneServer == nullptr)
			return false;

		auto zone = zoneServer->getZone(newZoneName);

#ifdef DEBUG_SHIP_STORE
		info(true) << "removing player: " << player->getDisplayedName() << " to zone: " << newZoneName;
#endif

		player->clearSpaceStates();

		if (player->isOnline() && zone != nullptr) {
			player->switchZone(newZoneName, location.getX(), location.getZ(), location.getY(), 0, false);
		} else {
			player->setPosition(location.getX(), location.getZ(), location.getY());

			player->setParent(nullptr);

			auto ghost = player->getPlayerObject();

			if (ghost != nullptr) {
				ghost->setSavedParentID(0);
				ghost->setSavedTerrainName(newZoneName);
			}
		}

		return true;
	}
};

#endif /* STORESHIPTASK_H_ */
