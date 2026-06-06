/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef FORCEREFRESHCLOSEOBJECTSCOMMAND_H_
#define FORCEREFRESHCLOSEOBJECTSCOMMAND_H_

// Infinity: GM diagnostic command for close objects debugging
// Dumps detailed close objects state, clears the vector, forces updateZone() rebuild,
// then dumps the new state. Used to diagnose stale close objects after long idle periods.

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/Zone.h"
#include "server/chat/ChatManager.h"
#include "QueueCommand.h"

class ForceRefreshCloseObjectsCommand : public QueueCommand {
public:

	ForceRefreshCloseObjectsCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {
	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		if (!creature->isPlayerCreature())
			return GENERALERROR;

		Zone* zone = creature->getZone();

		if (zone == nullptr)
			return GENERALERROR;

		ChatManager* chatManager = server->getZoneServer()->getChatManager();

		if (chatManager == nullptr)
			return GENERALERROR;

		StringBuffer msg;
		msg << "=== CLOSE OBJECTS DIAGNOSTIC ===" << endl;
		msg << "Player: " << creature->getFirstName() << endl;
		msg << "Position: (" << creature->getPositionX() << ", " << creature->getPositionZ() << ", " << creature->getPositionY() << ")" << endl;
		msg << "Zone: " << zone->getZoneName() << endl;
		msg << endl;

		CloseObjectsVector* vec = (CloseObjectsVector*)creature->getCloseObjects();

		if (vec == nullptr) {
			creature->sendSystemMessage("ERROR: CloseObjectsVector is null");
			return GENERALERROR;
		}

		// --- BEFORE snapshot ---
		Vector<ManagedReference<TreeEntry*>> beforeObjects;
		vec->safeCopyTo(beforeObjects);

		int beforeSize = beforeObjects.size();

		msg << "--- BEFORE REFRESH ---" << endl;
		msg << "Total close objects: " << beforeSize << endl;
		msg << endl;

		// Count by type and collect details
		int playerCount = 0, creatureCount = 0, buildingCount = 0;
		int shipCount = 0, installationCount = 0, tangibleCount = 0;
		int cellCount = 0, otherCount = 0, nullCount = 0;

		for (int i = 0; i < beforeObjects.size(); ++i) {
			SceneObject* obj = static_cast<SceneObject*>(beforeObjects.get(i).get());

			if (obj == nullptr) {
				nullCount++;
				continue;
			}

			if (obj->isPlayerCreature())
				playerCount++;
			else if (obj->isCreatureObject())
				creatureCount++;
			else if (obj->isBuildingObject())
				buildingCount++;
			else if (obj->isShipObject())
				shipCount++;
			else if (obj->isInstallationObject())
				installationCount++;
			else if (obj->isCellObject())
				cellCount++;
			else if (obj->isTangibleObject())
				tangibleCount++;
			else
				otherCount++;
		}

		msg << "By type:" << endl;
		msg << "  Players:       " << playerCount << endl;
		msg << "  Creatures:     " << creatureCount << endl;
		msg << "  Buildings:     " << buildingCount << endl;
		msg << "  Ships:         " << shipCount << endl;
		msg << "  Installations: " << installationCount << endl;
		msg << "  Cells:         " << cellCount << endl;
		msg << "  Tangibles:     " << tangibleCount << endl;
		msg << "  Other:         " << otherCount << endl;
		msg << "  NULL entries:  " << nullCount << endl;
		msg << endl;

		// Detail list — every object with OID, type, template, distance
		msg << "--- OBJECT DETAIL (BEFORE) ---" << endl;

		float playerX = creature->getWorldPositionX();
		float playerY = creature->getWorldPositionY();
		float playerZ = creature->getWorldPositionZ();

		for (int i = 0; i < beforeObjects.size(); ++i) {
			SceneObject* obj = static_cast<SceneObject*>(beforeObjects.get(i).get());

			if (obj == nullptr) {
				msg << "  [" << i << "] NULL" << endl;
				continue;
			}

			float objX = obj->getWorldPositionX();
			float objY = obj->getWorldPositionY();
			float objZ = obj->getWorldPositionZ();

			float dx = objX - playerX;
			float dy = objY - playerY;
			float dz = objZ - playerZ;
			float dist = Math::sqrt(dx * dx + dy * dy + dz * dz);

			String typeName = "unknown";

			if (obj->isPlayerCreature())
				typeName = "PLAYER";
			else if (obj->isCreatureObject())
				typeName = "CREATURE";
			else if (obj->isBuildingObject())
				typeName = "BUILDING";
			else if (obj->isShipObject())
				typeName = "SHIP";
			else if (obj->isInstallationObject())
				typeName = "INSTALL";
			else if (obj->isCellObject())
				typeName = "CELL";
			else if (obj->isTangibleObject())
				typeName = "TANGIBLE";
			else
				typeName = "OTHER";

			String templateFile = "";
			SharedObjectTemplate* tmpl = obj->getObjectTemplate();

			if (tmpl != nullptr)
				templateFile = tmpl->getTemplateFileName();

			msg << "  [" << i << "] OID=" << obj->getObjectID()
				<< " " << typeName
				<< " dist=" << String::valueOf((int)dist) << "m"
				<< " pos=(" << String::valueOf((int)objX) << "," << String::valueOf((int)objZ) << "," << String::valueOf((int)objY) << ")";

			if (templateFile.length() > 0)
				msg << " tmpl=" << templateFile;

			Zone* objZone = obj->getZone();

			if (objZone == nullptr)
				msg << " [NO ZONE]";

			msg << endl;
		}

		msg << endl;

		// --- CLEAR AND REBUILD ---
		msg << "--- CLEARING CLOSE OBJECTS ---" << endl;

		vec->removeAll();

		msg << "Vector cleared. Size after clear: " << vec->size() << endl;
		msg << endl;

		msg << "--- FORCING updateZone() ---" << endl;

		creature->updateZone(false, true);

		msg << "updateZone() complete." << endl;
		msg << endl;

		// --- AFTER snapshot ---
		Vector<ManagedReference<TreeEntry*>> afterObjects;
		vec->safeCopyTo(afterObjects);

		int afterSize = afterObjects.size();

		msg << "--- AFTER REFRESH ---" << endl;
		msg << "Total close objects: " << afterSize << endl;
		msg << "Delta: " << (afterSize - beforeSize) << " objects" << endl;
		msg << endl;

		// Recount by type
		playerCount = 0; creatureCount = 0; buildingCount = 0;
		shipCount = 0; installationCount = 0; tangibleCount = 0;
		cellCount = 0; otherCount = 0; nullCount = 0;

		for (int i = 0; i < afterObjects.size(); ++i) {
			SceneObject* obj = static_cast<SceneObject*>(afterObjects.get(i).get());

			if (obj == nullptr) {
				nullCount++;
				continue;
			}

			if (obj->isPlayerCreature())
				playerCount++;
			else if (obj->isCreatureObject())
				creatureCount++;
			else if (obj->isBuildingObject())
				buildingCount++;
			else if (obj->isShipObject())
				shipCount++;
			else if (obj->isInstallationObject())
				installationCount++;
			else if (obj->isCellObject())
				cellCount++;
			else if (obj->isTangibleObject())
				tangibleCount++;
			else
				otherCount++;
		}

		msg << "By type:" << endl;
		msg << "  Players:       " << playerCount << endl;
		msg << "  Creatures:     " << creatureCount << endl;
		msg << "  Buildings:     " << buildingCount << endl;
		msg << "  Ships:         " << shipCount << endl;
		msg << "  Installations: " << installationCount << endl;
		msg << "  Cells:         " << cellCount << endl;
		msg << "  Tangibles:     " << tangibleCount << endl;
		msg << "  Other:         " << otherCount << endl;
		msg << "  NULL entries:  " << nullCount << endl;
		msg << endl;

		// Find objects that were REMOVED (in before but not in after)
		msg << "--- STALE OBJECTS (removed during refresh) ---" << endl;

		HashSet<uint64> afterOIDs;

		for (int i = 0; i < afterObjects.size(); ++i) {
			SceneObject* obj = static_cast<SceneObject*>(afterObjects.get(i).get());

			if (obj != nullptr)
				afterOIDs.add(obj->getObjectID());
		}

		int staleCount = 0;

		for (int i = 0; i < beforeObjects.size(); ++i) {
			SceneObject* obj = static_cast<SceneObject*>(beforeObjects.get(i).get());

			if (obj == nullptr) {
				staleCount++;
				msg << "  NULL entry (index " << i << ")" << endl;
				continue;
			}

			if (!afterOIDs.contains(obj->getObjectID())) {
				staleCount++;

				String typeName = "unknown";

				if (obj->isPlayerCreature())
					typeName = "PLAYER";
				else if (obj->isCreatureObject())
					typeName = "CREATURE";
				else if (obj->isBuildingObject())
					typeName = "BUILDING";
				else if (obj->isShipObject())
					typeName = "SHIP";
				else if (obj->isInstallationObject())
					typeName = "INSTALL";
				else if (obj->isCellObject())
					typeName = "CELL";
				else if (obj->isTangibleObject())
					typeName = "TANGIBLE";
				else
					typeName = "OTHER";

				String templateFile = "";
				SharedObjectTemplate* tmpl = obj->getObjectTemplate();

				if (tmpl != nullptr)
					templateFile = tmpl->getTemplateFileName();

				Zone* objZone = obj->getZone();

				msg << "  STALE OID=" << obj->getObjectID()
					<< " " << typeName;

				if (templateFile.length() > 0)
					msg << " tmpl=" << templateFile;

				if (objZone == nullptr)
					msg << " [NO ZONE - destroyed/despawned]";
				else if (objZone != zone)
					msg << " [WRONG ZONE: " << objZone->getZoneName() << "]";

				msg << endl;
			}
		}

		if (staleCount == 0)
			msg << "  (none)" << endl;

		msg << endl;

		// Find objects that were ADDED (in after but not in before)
		msg << "--- NEW OBJECTS (appeared during refresh) ---" << endl;

		HashSet<uint64> beforeOIDs;

		for (int i = 0; i < beforeObjects.size(); ++i) {
			SceneObject* obj = static_cast<SceneObject*>(beforeObjects.get(i).get());

			if (obj != nullptr)
				beforeOIDs.add(obj->getObjectID());
		}

		int newCount = 0;

		for (int i = 0; i < afterObjects.size(); ++i) {
			SceneObject* obj = static_cast<SceneObject*>(afterObjects.get(i).get());

			if (obj == nullptr)
				continue;

			if (!beforeOIDs.contains(obj->getObjectID())) {
				newCount++;

				String typeName = "unknown";

				if (obj->isPlayerCreature())
					typeName = "PLAYER";
				else if (obj->isCreatureObject())
					typeName = "CREATURE";
				else if (obj->isBuildingObject())
					typeName = "BUILDING";
				else if (obj->isShipObject())
					typeName = "SHIP";
				else if (obj->isInstallationObject())
					typeName = "INSTALL";
				else if (obj->isCellObject())
					typeName = "CELL";
				else if (obj->isTangibleObject())
					typeName = "TANGIBLE";
				else
					typeName = "OTHER";

				String templateFile = "";
				SharedObjectTemplate* tmpl = obj->getObjectTemplate();

				if (tmpl != nullptr)
					templateFile = tmpl->getTemplateFileName();

				msg << "  NEW OID=" << obj->getObjectID()
					<< " " << typeName;

				if (templateFile.length() > 0)
					msg << " tmpl=" << templateFile;

				msg << endl;
			}
		}

		if (newCount == 0)
			msg << "  (none)" << endl;

		msg << endl;
		msg << "=== SUMMARY ===" << endl;
		msg << "Before: " << beforeSize << " | After: " << afterSize << " | Stale removed: " << staleCount << " | New found: " << newCount << endl;
		msg << "=== END DIAGNOSTIC ===" << endl;

		// Send summary to system message (chat window)
		StringBuffer summary;
		summary << "Close Objects Refresh: before=" << beforeSize << " after=" << afterSize
			<< " stale=" << staleCount << " new=" << newCount;
		creature->sendSystemMessage(summary.toString());

		// Send full report via mail (system messages truncate)
		chatManager->sendMail("System", "Close Objects Diagnostic", msg.toString(), creature->getFirstName());

		creature->sendSystemMessage("Full diagnostic report sent to your mailbox.");

		return SUCCESS;
	}
};

#endif //FORCEREFRESHCLOSEOBJECTSCOMMAND_H_
