/*
 * DestroyStructureTask.h
 *
 *  Created on: Jun 29, 2013
 *      Author: TheAnswer
 */

#ifndef DESTROYSTRUCTURETASK_H_
#define DESTROYSTRUCTURETASK_H_

#include "server/zone/Zone.h"
#include "server/zone/objects/structure/StructureObject.h"
#include "server/zone/objects/cell/CellObject.h"
#include "server/zone/objects/player/PlayerObject.h"
#include "server/zone/objects/building/BuildingObject.h"
#include "server/zone/objects/transaction/TransactionLog.h"
#include "server/zone/packets/object/PlayClientEffectObjectMessage.h"
#include "server/zone/packets/scene/PlayClientEffectLocMessage.h"

class DestroyStructureTask : public Task {
protected:
	ManagedReference<StructureObject*> structureObject;
	bool playEffect;
	bool killOccupants;

public:
	DestroyStructureTask(StructureObject* structure, bool doEffect = false, bool killStuff = false) {
		structureObject = structure;
		playEffect = doEffect;
		killOccupants = killStuff;

		setCustomTaskQueue("slowQueue");
	}

	void run() {
		if (structureObject == nullptr) {
			return;
		}

		Locker locker(structureObject);

		auto zone = structureObject->getZone();

		if (zone == nullptr)
			return;

		auto zoneServer = structureObject->getZoneServer();

		if (zoneServer == nullptr) {
			return;
		}

		if (zoneServer->isServerLoading()) {
			schedule(120000); // Infinity:  Delay for 2 minutes
			return;
		}

		float x = structureObject->getPositionX();
		float y = structureObject->getPositionY();
		float z = zone->getHeight(x, y);

		if (playEffect) {
			PlayClientEffectLoc* explodeLoc = new PlayClientEffectLoc("clienteffect/combat_explosion_lair_large.cef", structureObject->getZone()->getZoneName(), structureObject->getPositionX(), structureObject->getPositionZ(), structureObject->getPositionY());
			structureObject->broadcastMessage(explodeLoc, false);
		}

		if (structureObject->isBuildingObject()) {
			auto buildingObject = cast<BuildingObject*>(structureObject.get());

			for (uint32 i = 1; i <= buildingObject->getTotalCellNumber(); ++i) {
				auto cellObject = buildingObject->getCell(i);

				if (cellObject == nullptr)
					continue;

				// Traverse the vector backwards since the size will change as objects are removed.    Infinity:  Removed redundant if check
				for (int j = cellObject->getContainerObjectsSize() - 1; j >= 0; --j) {
					auto obj = cellObject->getContainerObject(j);

					if (obj != nullptr && (obj->isPlayerCreature() || obj->isPet())) {
						auto creature = cast<CreatureObject*>(obj.get());

						if (creature == nullptr) {
							continue;
						}
						structureObject->unlock();

						try {
							Locker plocker(creature);

							if (killOccupants) {
								creature->inflictDamage(creature, 0, 9999999, true, true);
								creature->inflictDamage(creature, 3, 9999999, true, true);
								creature->inflictDamage(creature, 6, 9999999, true, true);
							}

							creature->teleport(x, z, y, 0);
						} catch (...) {
							creature->error("unreported exception caught while teleporting");
						}

						structureObject->wlock();
					}
				}
			}
		}

		// Get the owner of the structure, and remove the structure from their possession.  Infinity:  cleaned up
		auto owner = structureObject->getOwnerCreatureObject();

		if (owner != nullptr && owner->isPlayerCreature()) {
			auto ghost = owner->getPlayerObject();

			if (ghost != nullptr) {
				// Infinity: lock the owner creature before mutating ghost's
				// ownedStructures list. Without this, engine3's transactional
				// save layer never associates the drop with the owner's commit
				// cycle; for offline owners (loaded ad-hoc only for this
				// destroy task) the dirty state evicts before BDB persistence,
				// leaving nullptr OIDs that the boot cleanup at
				// PlayerObjectImplementation::notifyOnline has to scrub on
				// next login. Mirrors the pattern in
				// TransferstructureCommand.h:205.
				Locker ownerLock(owner);

				ghost->removeOwnedStructure(structureObject);

				uint64 waypointID = structureObject->getWaypointID();

				if (waypointID != 0)
					ghost->removeWaypoint(waypointID, true, true);

				ownerLock.release();
			}
		}

		// Infinity: Audit-log the destruction before the world/database drop.
		// Captures who+what+where so CSRs can answer "when was structure X
		// destroyed" and so the PlayerObject:2197 self-heal warning has a
		// canonical correlation source. Includes camps (low volume, useful if
		// they're the ghosting source). Per [[project_trxlog_supersedes_core3log_audits]].
		{
			SceneObject* trxSrc = (owner != nullptr) ? (SceneObject*)owner : (SceneObject*)structureObject.get();
			TransactionLog trx(trxSrc, structureObject, TrxCode::STRUCTUREDESTROY);
			trx.addState("structureName", structureObject->getDisplayedName());
			trx.addState("isCamp", structureObject->isCampStructure() ? "true" : "false");
			trx.addState("posX", structureObject->getPositionX());
			trx.addState("posY", structureObject->getPositionY());
			trx.addState("zone", structureObject->getZone() != nullptr ? structureObject->getZone()->getZoneName() : "unknown");
			trx.addState("ownerPresent", owner != nullptr ? "true" : "false");
			trx.addState("ownerOnline", (owner != nullptr && owner->isPlayerCreature() && owner->getPlayerObject() != nullptr && owner->getPlayerObject()->isOnline()) ? "true" : "false");
			trx.addState("killOccupants", killOccupants ? "true" : "false");
		}

		structureObject->notifyObservers(ObserverEventType::OBJECTDESTRUCTION, structureObject, 0);
		structureObject->destroyObjectFromWorld(true);
		structureObject->destroyObjectFromDatabase(true);
	}
};

#endif /* DESTROYSTRUCTURETASK_H_ */

