/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef CREATENPCCOMMAND_H_
#define CREATENPCCOMMAND_H_

#include "server/zone/managers/director/DirectorManager.h"
#include "server/zone/objects/creature/ai/AiAgent.h"
#include "server/zone/objects/creature/commands/QueueCommand.h"
#include "server/zone/objects/ship/ai/ShipAiAgent.h"
#include "server/zone/objects/tangible/spawning/SpawnEggObject.h"
#include "templates/params/ship/ShipFlag.h"
//Infinity:  Custom includes
#include "server/zone/objects/area/ActiveArea.h"

class CreateNPCCommand : public QueueCommand {
public:

	CreateNPCCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		StringTokenizer args(arguments.toString());

		if (!args.hasMoreTokens()) {
            sendSyntax(creature);
			return INVALIDPARAMETERS;
        }

		String arg = "";

		args.getStringToken(arg);

		arg = arg.toLowerCase();

		ManagedReference<SceneObject*> targetObject = server->getZoneServer()->getObject(creature->getTargetID());

		if (arg == "tools") {
			Lua* lua = DirectorManager::instance()->getLuaInstance();

			Reference<LuaFunction*> staffTools = lua->createFunction("StaffTools", "openToolsSUI", 0);
			*staffTools << creature;

			staffTools->callFunction();
		} else if (arg == "toggledebug") {
			if (targetObject == nullptr)
				return GENERALERROR;

			StringBuffer msg;

			if (targetObject->isAiAgent()) {
				AiAgent* aiAgent = targetObject->asAiAgent();

				if (aiAgent == nullptr)
					return GENERALERROR;

				Locker clocker(aiAgent, creature);

				bool curDebug = aiAgent->getAIDebug();

				aiAgent->setAIDebug(!curDebug);

				msg << "AiAgent " << aiAgent->getObjectID() << " debug set to " << aiAgent->getAIDebug();

#ifndef DEBUG_AI
				msg << " not compiled with DEBUG_AI, using LogLevel";
#endif // DEBUG_AI

				String logFileName = aiAgent->getLogFileName();

				if (!logFileName.isEmpty()) {
					msg << " logging to " << logFileName;
				}
			} else if (targetObject->isShipAiAgent()) {
				ShipAiAgent* agent = targetObject->asShipAiAgent();

				if (agent == nullptr)
					return GENERALERROR;

				Locker clocker(agent, creature);

				bool curDebug = agent->getShipAiDebug();

				agent->setShipAiDebug(!curDebug);

				msg << "AiAgent: " << agent->getDisplayedName() << " ID: " << agent->getObjectID() << " debug set to " << agent->getShipAiDebug();
			} else {
				msg << "Failed to start debug on target.";
			}

			creature->sendSystemMessage(msg.toString());
		} else if (arg == "shipfollow") {
			if (targetObject == nullptr || !targetObject->isShipAiAgent())
				return GENERALERROR;

			ShipAiAgent* shipAgent = targetObject->asShipAiAgent();

			if (shipAgent == nullptr)
				return GENERALERROR;

			ManagedReference<SceneObject*> rootParent = creature->getRootParent();

			if (rootParent == nullptr || !rootParent->isShipObject())
				return GENERALERROR;

			ShipObject* shipParent = rootParent->asShipObject();

			if (shipParent == nullptr)
				return GENERALERROR;

			Locker clocker(shipAgent, creature);

			shipAgent->addShipFlag(ShipFlag::ESCORT);
			shipAgent->setShipAiTemplate();

			shipAgent->setFollowShipObject(shipParent);

			return SUCCESS;
		} else if (arg == "locatespawners") {
			auto zone = creature->getZone();

			if (zone == nullptr)
				return GENERALERROR;

			auto playerPosition = creature->getWorldPosition();

			SortedVector<TreeEntry*> closeObjects;
			zone->getInRangeObjects(playerPosition.getX(), playerPosition.getZ(), playerPosition.getY(), 128, &closeObjects, true);

			for (int i = 0; i < closeObjects.size(); i++) {
				SceneObject* object = static_cast<SceneObject*>(closeObjects.get(i));

				if (object == nullptr || !object->isSpawnEggObject())
					continue;

				auto spawnEgg = dynamic_cast<SpawnEggObject*>(object);

				if (spawnEgg == nullptr)
					continue;

				Locker clock(spawnEgg, creature);

				spawnEgg->setInvisible(false);
				creature->broadcastObject(spawnEgg, false);

				StringBuffer msg;
				msg << spawnEgg->getDisplayedName() << " --- Location: " << spawnEgg->getWorldPosition().toString()
					<< " -- Spawner Set Visible. Please target with /tar spawn and use the radial option to set invisible.";
				creature->sendSystemMessage(msg.toString());
			}
		} else if (arg == "debugregions") {
			if (creature->isDebuggingRegions()) {
				creature->setDebuggingRegions(false);
				creature->sendSystemMessage("Region System Message Debug Disabled");
			} else {
				creature->setDebuggingRegions(true);
				creature->sendSystemMessage("Region System Message Debug Enabled");
			}

		} else if (arg == "activeareas") {

			auto zone = creature->getZone();
			if (zone == nullptr) {
				return GENERALERROR;
			}

			StringBuffer msg;

			msg << endl << "List of active areas at current position" << endl;
			msg << "-----------------------------------------------------------------------------------------------------------------------------------------------------------------" << endl;


			SortedVector<ManagedReference<ActiveArea*> >* activeAreas = creature->getActiveAreas();

			//zone->getInRangeActiveAreas(creature->getWorldPositionX(), creature->getWorldPositionY(), &activeAreas, true);

			for (int i = 0; i < activeAreas->size(); ++i) {
				ActiveArea* area = activeAreas->get(i);

				if (area == nullptr) {
					continue;
				}

				String areaName = (area->getAreaName().isEmpty()) ? "Undefined" : area->getAreaName();

				msg << " Area name: " << areaName << "  ";

				String shape = "Undefined";
				if (area->isSphereAreaShape())
					shape = "Sphere";
				else if (area->isRectangularAreaShape())
					shape = "Rectangle";
				else if (area->isCuboidAreaShape())
					shape = "Cuboid";
				msg << "Shape: " << shape << "  ";

				Vector3 coords = area->getWorldPosition();
				msg << "Position: " << coords.getX() <<  ", " << coords.getZ() << ", " << coords.getY() << "  ";

				msg << "Radius: " << area->getRadius() << endl;

				StringBuffer regionTypes;

				if (area->isUndefinedRegion())
					regionTypes << "UNDEFINEDAREA ";
				if (area->isSpawnArea())
					regionTypes << "SPAWNAREA ";
				if (area->isNoSpawnArea())
					regionTypes << "NOSPAWNAREA ";
				if (area->isWorldSpawnArea())
					regionTypes << "WORLDSPAWNAREA ";
				if (area->isNoWorldSpawnArea())
					regionTypes << "NOWORLDSPAWNAREA ";
				if (area->isNoBuildZone())
					regionTypes << "NOBUILDZONEAREA ";
				if (area->isCampingArea())
					regionTypes << "CAMPINGAREA ";
				if (area->isCityRegion())
					regionTypes << "CITY ";
				if (area->shouldBuildNavmesh())
					regionTypes << "NAVAREA ";
				if (area->isNamedRegion())
					regionTypes << "NAMEDREGION ";
				if (area->isLockedArea())
					regionTypes << "LOCKEDAREA";

				msg << " Region Types: (" << regionTypes.toString() << ")" << endl;

				msg << "-----------------------------------------------------------------------------------------------------------------------------------------------------------------" << endl;
			}

			creature->sendSystemMessage(msg.toString());
		} else {
            sendSyntax(creature);
            return INVALIDPARAMETERS;
        }

		return SUCCESS;
	}

    	static void sendSyntax(CreatureObject* player) {
		if (player != nullptr) {
            player->sendSystemMessage("Syntax: /createNPC [tools]");
            player->sendSystemMessage("Syntax: /createNPC [toggledebug]");
			player->sendSystemMessage("Syntax: /createNPC [debugregions]");
			player->sendSystemMessage("Syntax: /createNPC [activeareas]");
			player->sendSystemMessage("Syntax: /createNPC [shipfollow]");
			player->sendSystemMessage("Syntax: /createNPC [locatespawners]");

		}
	}

};

#endif //CREATENPCCOMMAND_H_
