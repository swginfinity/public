/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef MOVEFURNITURECOMMAND_H_
#define MOVEFURNITURECOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/objects/building/BuildingObject.h"
#include "server/zone/objects/tangible/components/CityDecorationMenuComponent.h"

class MoveFurnitureCommand : public QueueCommand {
public:

	MoveFurnitureCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {

	}

	//returns false on collision detection
	bool checkCollision(SceneObject* object, Vector3& endPoint) const {
		return CollisionManager::checkLineOfSightInParentCell(object, endPoint);
	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {
		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		ManagedReference<PlayerObject*> ghost = creature->getPlayerObject();

		if (ghost == nullptr)
			return GENERALERROR;

		ManagedReference<SceneObject*> obj = server->getZoneServer()->getObject(target);

		if (obj == nullptr || !obj->isTangibleObject() || obj->isPlayerCreature() || obj->isPet()) {
			creature->sendSystemMessage("@player_structure:move_what"); //What do you want to move?
			return GENERALERROR;
		}

		ManagedReference<SceneObject*> creatureParent = creature->getRootParent();
		ManagedReference<SceneObject*> objectParent = obj->getRootParent();
		BuildingObject* buildingObject = creatureParent != nullptr ? (creatureParent->isBuildingObject() ? cast<BuildingObject*>( creatureParent.get()) : nullptr) : nullptr;

		if (!(ghost->getAdminLevel() == 15))  { //// Restrictions for non-admin characters

			if (buildingObject == nullptr) {  // We're outside

				if (creatureParent != objectParent) {
					creature->sendSystemMessage("@player_structure:move_what"); //What do you want to move?
					return GENERALERROR;
				}

				ManagedReference<CreatureObject*> mayor = creature;

				Locker locker(mayor);

				ManagedReference<CityRegion*> city = mayor->getCityRegion().get();

				if (city == nullptr) { // Not in a city
					creature->sendSystemMessage("@player_structure:move_what"); //What do you want to move?
					return GENERALERROR;
				}

				CityManager* cityManager = mayor->getZoneServer()->getCityManager();

				if(!city->isMayor(mayor->getObjectID())) {  // Not mayor
					creature->sendSystemMessage("@player_structure:move_what"); //What do you want to move?
					return GENERALERROR;
				}

				if (!mayor->hasSkill("social_politician_master")) {
					mayor->sendSystemMessage("You must be a master politician to move objects in your city."); 
					return GENERALERROR;
				}

				CityDecorationMenuComponent* decorationMenu = cast<CityDecorationMenuComponent*>(obj->getObjectMenuComponent());

				if (decorationMenu == nullptr && !obj->isMissionTerminal() && !obj->getObjectTemplate()->getFullTemplateString().contains("trainer")) {
					mayor->sendSystemMessage("You can only move objects that are city decorations, mission terminals, or trainers."); 
					return GENERALERROR;
				}
			} 
			else {
				if (obj->isVendor() && !obj->checkContainerPermission(creature, ContainerPermissions::MOVEVENDOR)) {
					return GENERALERROR;
				}
				/*if (obj->isVendor()) {
					creature->sendSystemMessage("@player_structure:cant_move_vendor"); // To move a vendor, pick it up and drop it again at the new location.
					return GENERALERROR;
				}*/

				if (!obj->isVendor() && !buildingObject->isOnAdminList(creature)) {
					creature->sendSystemMessage("@player_structure:must_be_admin"); //You must be a building admin to do that.
					return GENERALERROR;
				}

				if (objectParent != buildingObject || buildingObject->containsChildObject(obj)) {
					creature->sendSystemMessage("@player_structure:move_what"); //What do you want to move?
					return GENERALERROR;
				}

				if (buildingObject->isGCWBase()) {
					creature->sendSystemMessage("@player_structure:no_move_hq"); // You may not move or rotate objects inside a factional headquarters.
					return GENERALERROR;
				}
			}
		}


		String dir;
		float dist = 0.f;

		try {
			UnicodeTokenizer tokenizer(arguments);
			tokenizer.getStringToken(dir);
			dir = dir.toLowerCase();

			if (Character::isDigit(dir.charAt(0)))
				throw Exception("Please specify the name of the object before the direction and distance.");

			if (dir != "up" && dir != "down" && dir != "forward" && dir != "back")
				throw Exception("@player_structure:format_movefurniture_distance"); //Format: /moveFurniture <FORWARD/BACK/UP/DOWN> <distance>

			dist = tokenizer.getIntToken();

			if (dist < 1.f || dist > 500.f)
				throw Exception("@player_structure:movefurniture_params"); //The amount to move must be between 1 and 500.

		} catch (ArrayIndexOutOfBoundsException& e) {
			throw Exception("@player_structure:format_movefurniture_distance"); //Format: /moveFurniture <FORWARD/BACK/UP/DOWN> <distance>
			return INVALIDPARAMETERS;

		} catch (Exception& e) {
			creature->sendSystemMessage(e.getMessage());
			return INVALIDPARAMETERS;
		}

		float degrees = creature->getDirectionAngle();
		float radians = Math::deg2rad(degrees);

		dist /= 100.0f;

		float offsetX = dist * sin(radians);
		float offsetY = dist * cos(radians);

		float x = obj->getPositionX();
		float y = obj->getPositionY();
		float z = obj->getPositionZ();

		if (dir == "forward") {
			x += (offsetX);
			y += (offsetY);
		} else if (dir == "back") {
			x -= (offsetX);
			y -= (offsetY);
		} else if (dir == "up") {
			z += dist;
		} else if (dir == "down") {
			z -= dist;
		}

		Vector3 endPoint(x, y, z);

		if (!checkCollision(obj, endPoint)) {
			creature->sendSystemMessage("@player_structure:not_valid_location"); //That is not a valid location.
			return GENERALERROR;
		}

		obj->incrementMovementCounter();

		ManagedReference<SceneObject*> objParent = obj->getParent().get();
		if (objParent != nullptr)
			obj->teleport(x, z, y, objParent->getObjectID());
		else
			obj->teleport(x, z, y);


		return SUCCESS;
	}

};

#endif //MOVEFURNITURECOMMAND_H_
