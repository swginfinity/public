
#ifndef SCHEDULESHUTTLETASK_H_
#define SCHEDULESHUTTLETASK_H_

#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/managers/planet/PlanetManager.h"

class ScheduleShuttleTask : public Task, public Logger {
	ManagedWeakReference<CreatureObject*> shuttleObject;
	Zone* zone;

public:
	ScheduleShuttleTask(CreatureObject* shuttle, Zone* zon) : Task() {
		shuttleObject = shuttle;
		zone = zon;

		Logger::setLoggingName("ScheduleShuttleTask");
	}

	void run() {
		if (zone == nullptr) {
			error() << " zone has a nullptr.";
			return;
		}

		auto zoneServer = zone->getZoneServer();

		if (zoneServer == nullptr) {
			error() << " zoneServer is nullptr.";
			return;
		}

		if (zoneServer->isServerLoading()) {
			schedule(1000);
			return;
		}

		ManagedReference<CreatureObject*> strongShuttle = shuttleObject.get();

		if (strongShuttle == nullptr) {
			error() << " Shuttle strongShuttle has a nullptr in Zone: " << zone->getZoneName();
			return;
		}

		Locker lock(strongShuttle);

		ManagedReference<PlanetManager*> planetManager = zone->getPlanetManager();

		if (planetManager == nullptr) {
			zone->error() << " planetManager has a nullptr in Zone: " << zone->getZoneName();
			return;
		}

		ManagedReference<CityRegion*> cityRegion = strongShuttle->getCityRegion().get();

		float x = strongShuttle->getWorldPositionX();
		float y = strongShuttle->getWorldPositionY();
		float z = strongShuttle->getWorldPositionZ();

		/* Infinity:  Shuttle travel point logging
		bool isHG = false;
		if (x >= -5044 && x <= -5043) {

			isHG = true;
			StringBuffer msg;
			msg << "Checking Highgarden Shuttle" << endl;
			msg << "Zone: " << zone->getZoneName() << "  X: " << x << "  Y: " << y << endl;

			Logger::console.error(msg.toString());

		}
		*/

		// Player City
		if ((cityRegion != nullptr) && !cityRegion->isClientRegion()) {
			Vector3 arrivalVector(x, y, z);

			String zoneName = zone->getZoneName();

			Locker clocker(cityRegion, strongShuttle);

			cityRegion->setShuttleID(strongShuttle->getObjectID());
			clocker.release();

			PlanetTravelPoint* planetTravelPoint = new PlanetTravelPoint(zoneName, cityRegion->getCityRegionName(), arrivalVector, arrivalVector, strongShuttle, 6.f);

			/* Infinity:  Shuttle travel point logging
			if (isHG) {
				Logger::console.error("Created planetTravelPoint for HG shuttle");
			}
			*/

			planetManager->addPlayerCityTravelPoint(planetTravelPoint);
			planetManager->scheduleShuttle(strongShuttle, PlanetManager::SHUTTLEPORT);

			strongShuttle->updateCooldownTimer("shuttleStart", 2000);
		} else {
			Reference<PlanetTravelPoint*> travelPoint = planetManager->getNearestPlanetTravelPoint(strongShuttle, 128.f);

			if (travelPoint == nullptr) {
				// Infinity: include shuttle identifiers so a destroyed-but-still-
				// scheduled shuttle (orphaned in the scheduler list / BDB) can be
				// tracked down and cleaned up. Downgraded to warning() — this fires
				// once per scheduler tick for the lingering shuttle, not an engine
				// internal error; the early return is the correct handling.
				warning() << "Planet Travel Point (travelPoint) has a nullptr at (" << x << ", " << y << ", " << z << ") in Zone: " << zone->getZoneName()
						  << " shuttleOID=" << strongShuttle->getObjectID()
						  << " shuttleName=[" << strongShuttle->getDisplayedName() << "]";
				return;
			}

			/* Infinity:  Shuttle travel point logging
			if (isHG) {
				Logger::console.error("Should never get here, something is FUBAR and it is probably assigning Theed Shuttle B");
			}
			*/

			auto oldShuttle = travelPoint->getShuttle();

			if (oldShuttle == nullptr) {
				travelPoint->setShuttle(strongShuttle);

				if (travelPoint->isInterplanetary()) {
					planetManager->scheduleShuttle(strongShuttle, PlanetManager::STARPORT);
				} else {
					planetManager->scheduleShuttle(strongShuttle, PlanetManager::SHUTTLEPORT);
				}
			} else if (oldShuttle != strongShuttle) {
				/* Infinity:  Shuttle travel point logging
				if (isHG) {
					Logger::console.error("Deleting shuttle, something is FUBAR");
				}
				*/
				strongShuttle->destroyObjectFromWorld(true);
				strongShuttle->destroyObjectFromDatabase(true);
			}
		}
	}
};

#endif /* SCHEDULESHUTTLETASK_H_ */
