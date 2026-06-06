/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef GETATTRIBUTESBATCHCOMMAND_H_
#define GETATTRIBUTESBATCHCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/packets/scene/AttributeListMessage.h"
#include "server/zone/Zone.h"

class GetAttributesBatchCommand : public QueueCommand {
public:

	const int MAXIMUM_BATCH = 999;

	GetAttributesBatchCommand(const String& name, ZoneProcessServer* server) : QueueCommand(name, server) {
	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {
		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		if (!creature->isPlayerCreature())
			return GENERALERROR;

		ManagedReference<PlayerObject*> ghost = creature->getPlayerObject();

		if (ghost == nullptr)
			return GENERALERROR;

		StringTokenizer ids(arguments.toString());

		Zone* zone = creature->getZone();

		if (zone == nullptr)
			return GENERALERROR;

		int incr = 0;

		// Infinity diagnostic: sample what the over-cap batch actually contains so a cap-exceed
		// (rare) self-identifies the root cause — vendor stock vs house decorations vs runaway awareness.
		StringBuffer batchSample;
		int sampleCount = 0;

		while (ids.hasMoreTokens()) {
			uint64 objid = 0;

			try {
				objid = ids.getLongToken();

			} catch (Exception& e) {
				error(e.getMessage());
			}

			if (objid == 0)
				continue;

			ManagedReference<SceneObject*> object = zone->getZoneServer()->getObject(objid);

			if (object != nullptr) {
				if (sampleCount < 10) {
					batchSample << object->getServerObjectCRC() << "(got" << object->getGameObjectType() << ") ";
					++sampleCount;
				}

				ManagedReference<SceneObject*> parent = object->getParent().get();

				if (parent != nullptr && parent->isCreatureObject() && !ghost->isPrivileged() && !object->isASubChildOf(creature)) {
					sendEmptyAttributes(creature, objid);
				} else {
					int count = (incr == 0 && !ids.hasMoreTokens()) ? 0 : incr;

					sendAttributes(creature, object, count);
				}

			} else {
				sendEmptyAttributes(creature, objid);
			}

			if (++incr > MAXIMUM_BATCH) {
				ManagedReference<SceneObject*> rootParent = creature->getRootParent();
				Vector3 worldPos = creature->getWorldPosition();

				creature->warning() << creature->getDisplayedName() << " ID: " << creature->getObjectID()
					<< "  GetAttributesBatchCommand -- Objects attribute limit exceeded with more than " << incr << " total objects."
					<< " command=" << getQueueCommandName()
					<< " zone=" << zone->getZoneName()
					<< " pos=(" << worldPos.getX() << ", " << worldPos.getY() << ")"
					<< " rootParentOID=" << (rootParent != nullptr ? rootParent->getObjectID() : (uint64)0)
					<< " rootParentCRC=" << (rootParent != nullptr ? rootParent->getServerObjectCRC() : (uint32)0)
					<< " objSample=[" << batchSample.toString() << "]";

				return GENERALERROR;
			}
		}

		return SUCCESS;
	}

	void sendEmptyAttributes(CreatureObject* creature, uint64 objid) const {
		AttributeListMessage* msg = new AttributeListMessage(objid);
		creature->sendMessage(msg);
	}

	void sendAttributes(CreatureObject* creature, SceneObject* object, int incr) const {
		object->sendAttributeListTo(creature);

		creature->notifyObservers(ObserverEventType::GETATTRIBUTESBATCHCOMMAND, object, incr);
	}
};

#endif // GETATTRIBUTESBATCHCOMMAND_H_
