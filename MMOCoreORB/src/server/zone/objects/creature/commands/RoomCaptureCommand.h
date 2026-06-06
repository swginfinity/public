/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef ROOMCAPTURECOMMAND_H_
#define ROOMCAPTURECOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/objects/player/PlayerObject.h"
#include "server/zone/objects/building/BuildingObject.h"
#include "server/zone/objects/cell/CellObject.h"
#include "QueueCommand.h"
#include <fstream>

class RoomCaptureCommand : public QueueCommand {
public:

	RoomCaptureCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {
	}

	static String escapeJson(const String& input) {
		StringBuffer result;
		for (int i = 0; i < input.length(); i++) {
			char c = input.charAt(i);
			switch (c) {
				case '"': result << "\\\""; break;
				case '\\': result << "\\\\"; break;
				case '\n': result << "\\n"; break;
				case '\r': result << "\\r"; break;
				case '\t': result << "\\t"; break;
				default: result << c; break;
			}
		}
		return result.toString();
	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (creature == nullptr)
			return GENERALERROR;

		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		if (!creature->isPlayerCreature())
			return GENERALERROR;

		ManagedReference<PlayerObject*> ghost = creature->getPlayerObject();
		if (ghost == nullptr)
			return GENERALERROR;

		if (ghost->getAdminLevel() < 15) {
			creature->sendSystemMessage("You must be in god mode to use this command.");
			return GENERALERROR;
		}

		// Get root parent - must be a building or POB ship
		ManagedReference<SceneObject*> rootParent = creature->getRootParent();
		if (rootParent == nullptr) {
			creature->sendSystemMessage("You must be inside a building to use /roomcapture.");
			return GENERALERROR;
		}

		BuildingObject* building = nullptr;

		if (rootParent->isBuildingObject()) {
			building = cast<BuildingObject*>(rootParent.get());
		} else if (rootParent->isPobShip()) {
			building = cast<BuildingObject*>(rootParent.get());
		}

		if (building == nullptr) {
			creature->sendSystemMessage("You must be inside a building or POB ship to use /roomcapture.");
			return GENERALERROR;
		}

		// Get building template path
		String buildingTemplate = "";
		SharedObjectTemplate* tmpl = building->getObjectTemplate();
		if (tmpl != nullptr) {
			buildingTemplate = tmpl->getFullTemplateString();
		}

		// Determine output path from arguments or default
		String outputPath = "/home/swgemu/workspace/roomcapture.json";
		if (!arguments.isEmpty()) {
			String argStr = arguments.toString().trim();
			if (argStr.length() > 0) {
				outputPath = argStr;
			}
		}

		// Iterate all cells and collect objects
		int totalCells = building->getTotalCellNumber();
		int totalObjects = 0;

		StringBuffer jsonBuf;
		jsonBuf << "[\n";

		bool firstObject = true;

		for (int i = 1; i <= totalCells; i++) {
			CellObject* cell = building->getCell(i);
			if (cell == nullptr)
				continue;

			String cellName = building->getCellName(cell->getObjectID());

			int containerSize = cell->getContainerObjectsSize();

			for (int j = 0; j < containerSize; j++) {
				ManagedReference<SceneObject*> child = cell->getContainerObject(j);
				if (child == nullptr)
					continue;

				int got = child->getGameObjectType();

				// Skip players, vendors, terminals, elevators
				if (got == 0x409  // PLAYERCREATURE
					|| got == 7   // vendors
					|| got == 16392 // structure terminal
					|| got == 16404 // guild terminal
					|| got == 16384 // elevator button
					|| child->isCreatureObject()) {
					continue;
				}

				// Get template path
				String templatePath = "";
				SharedObjectTemplate* childTmpl = child->getObjectTemplate();
				if (childTmpl != nullptr) {
					templatePath = childTmpl->getFullTemplateString();
				}

				if (templatePath.isEmpty())
					continue;

				// Get position
				float posX = child->getPositionX();
				float posY = child->getPositionY();
				float posZ = child->getPositionZ();

				// Get quaternion
				const Quaternion* dir = child->getDirection();
				float quatW = dir->getW();
				float quatX = dir->getX();
				float quatY = dir->getY();
				float quatZ = dir->getZ();

				if (!firstObject) {
					jsonBuf << ",\n";
				}
				firstObject = false;

				jsonBuf << "  {\n";
				jsonBuf << "    \"templatePath\": \"" << escapeJson(templatePath) << "\",\n";
				jsonBuf << "    \"cellName\": \"" << escapeJson(cellName) << "\",\n";
				jsonBuf << "    \"posX\": " << String::format("%.6f", posX) << ",\n";
				jsonBuf << "    \"posY\": " << String::format("%.6f", posY) << ",\n";
				jsonBuf << "    \"posZ\": " << String::format("%.6f", posZ) << ",\n";
				jsonBuf << "    \"quatW\": " << String::format("%.6f", quatW) << ",\n";
				jsonBuf << "    \"quatX\": " << String::format("%.6f", quatX) << ",\n";
				jsonBuf << "    \"quatY\": " << String::format("%.6f", quatY) << ",\n";
				jsonBuf << "    \"quatZ\": " << String::format("%.6f", quatZ) << "\n";
				jsonBuf << "  }";

				totalObjects++;
			}
		}

		jsonBuf << "\n]\n";

		// Write to file
		std::ofstream outFile(outputPath.toCharArray());
		if (!outFile.is_open()) {
			creature->sendSystemMessage("Failed to write file: " + outputPath);
			return GENERALERROR;
		}

		String jsonStr = jsonBuf.toString();
		outFile << jsonStr.toCharArray();
		outFile.close();

		StringBuffer msg;
		msg << "Room capture complete: " << totalObjects << " objects from " << totalCells << " cells written to " << outputPath;
		creature->sendSystemMessage(msg.toString());

		return SUCCESS;
	}

};

#endif //ROOMCAPTURECOMMAND_H_
