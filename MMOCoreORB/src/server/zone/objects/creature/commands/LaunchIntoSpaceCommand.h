/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef LAUNCHINTOSPACECOMMAND_H_
#define LAUNCHINTOSPACECOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/managers/planet/PlanetManager.h"
#include "conf/ConfigManager.h"
#include "server/zone/SpaceZone.h"
#include "server/zone/managers/space/SpaceManager.h"
#include "server/zone/objects/intangible/ShipControlDevice.h"
#include "server/zone/packets/scene/UpdateTransformMessage.h"
#include "server/zone/objects/intangible/tasks/LaunchShipTask.h"

class LaunchIntoSpaceCommand : public QueueCommand {
public:
	LaunchIntoSpaceCommand(const String& name, ZoneProcessServer* server) : QueueCommand(name, server) {
	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {
		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		// Infinity: Block all space launch when JTL is disabled
		if (!ConfigManager::instance()->isJtlEnabled()) {
			creature->sendSystemMessage("Ships are currently disabled.");
			return GENERALERROR;
		}

		ZoneServer* zoneServer = server->getZoneServer();

		if (zoneServer == nullptr)
			return GENERALERROR;

		StringTokenizer tokenizer(arguments.toString());

		if (!tokenizer.hasMoreTokens())
			return GENERALERROR;

		uint64 scdID = tokenizer.hasMoreTokens() ? tokenizer.getLongToken() : 0;
		uint32 groupSize = tokenizer.hasMoreTokens() ? tokenizer.getIntToken() : 0;
		Vector<uint64> groupMembers;

		for (int i = 0; i < groupSize; i++) {
			if (!tokenizer.hasMoreTokens())
				break;

			uint64 memberID = tokenizer.getLongToken();
			groupMembers.add(memberID);
		}

		String arrivalPlanet = tokenizer.hasMoreTokens() ? tokenizer.getStringToken() : "";
		String arrivalPointName = tokenizer.hasMoreTokens() ? tokenizer.getStringToken().replaceAll("_", " ") : "";

		ManagedReference<SceneObject*> terminal = zoneServer->getObject(target);

		if (terminal == nullptr || terminal->getGameObjectType() != SceneObjectType::SPACETERMINAL) {
			return INVALIDSTATE;
		}

		ManagedReference<ShipControlDevice*> shipDevice = zoneServer->getObject(scdID).castTo<ShipControlDevice*>();

		if (shipDevice == nullptr) {
			return GENERALERROR;
		}

		auto shipControlledObject = shipDevice->getControlledObject();

		if (shipControlledObject == nullptr) {
			return GENERALERROR;
		}

		ManagedReference<ShipObject*> ship = shipControlledObject->asShipObject();

		if (ship == nullptr || ship->getOwner().get() != creature) {
			return GENERALERROR;
		}

		if (!ship->canBePilotedBy(creature)) {
			creature->sendSystemMessage("@space/space_interaction:no_ship_certification"); // "You are not certified to pilot this ship."
			return GENERALERROR;
		}

		// JTL Fast Travel
		if (arrivalPlanet != "") {
			if (ship->getComponentObject(Components::REACTOR) == nullptr) {
				creature->sendSystemMessage("There is no reactor in this ship, it cannot be use to travel.");
				return GENERALERROR;
			} else if (ship->getComponentObject(Components::ENGINE) == nullptr) {
				creature->sendSystemMessage("There is no engine in this ship, it cannot be use to travel.");
				return GENERALERROR;
			}

			Zone* arrivalZone = zoneServer->getZone(arrivalPlanet);

			if (arrivalZone == nullptr) {
				creature->sendSystemMessage("@travel:route_not_available"); // This ticket's route is no longer available.
				return GENERALERROR;
			}

			PlanetManager* planetManager = arrivalZone->getPlanetManager();

			if (planetManager == nullptr || !planetManager->isExistingPlanetTravelPoint(arrivalPointName)) {
				creature->sendSystemMessage("@travel:route_not_available"); // This ticket's route is no longer available.
				return GENERALERROR;
			}

			auto planetTravelPoint = planetManager->getPlanetTravelPoint(arrivalPointName);

			if (planetTravelPoint == nullptr || !planetTravelPoint->isInterplanetary() || planetTravelPoint->isJtlDisabled()) {
				return GENERALERROR;
			}

			// Arrival Position on Planet
			Coordinate arrivalPosition;
			arrivalPosition.initializePosition(planetTravelPoint->getArrivalPosition());

			float landingRange = planetTravelPoint->getLandingRange();
			float arrivalZ = planetTravelPoint->getArrivalPositionZ();

			uint64 pilotID = creature->getObjectID();

			for (int j = 0; j < groupMembers.size(); j++) {
				auto memberID = groupMembers.get(j);

				if (memberID == pilotID)
					continue;

				auto memberScno = zoneServer->getObject(memberID);

				if (memberScno == nullptr || !memberScno->isPlayerCreature()) {
					continue;
				}

				// Get a random landing position
				Coordinate newPosition(arrivalPosition);
				newPosition.randomizePosition(landingRange);

				Locker memLock(memberScno, creature);

				memberScno->switchZone(arrivalPlanet, newPosition.getPositionX(), arrivalZ, newPosition.getPositionY(), 0);
			}

			// Get a random landing position
			Coordinate pilotPosition(arrivalPosition);
			pilotPosition.randomizePosition(landingRange);

			creature->switchZone(arrivalPlanet, pilotPosition.getPositionX(), arrivalZ, pilotPosition.getPositionY(), 0);

			Locker cLock(shipDevice, creature);
			shipDevice->setStoredLocationData(creature);

			// Infinity: Update parking location to the arrival starport
			shipDevice->setParkingLocation(arrivalPointName);
		 // launch into space
		} else {
			creature->info(true) << "LaunchIntoSpaceCommand -- scheduling LaunchShipTask for ship device: " << shipDevice->getObjectID() << " groupSize: " << groupMembers.size();

			LaunchShipTask* launchTask = new LaunchShipTask(creature, shipDevice, groupMembers);

			if (launchTask != nullptr) {
				launchTask->schedule(100);
			}
		}

		return SUCCESS;
	}
};

#endif // LAUNCHINTOSPACECOMMAND_H_
