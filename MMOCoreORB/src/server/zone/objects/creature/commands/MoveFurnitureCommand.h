/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef MOVEFURNITURECOMMAND_H_
#define MOVEFURNITURECOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/objects/building/BuildingObject.h"
#include "server/zone/objects/ship/PobShipObject.h"
#include "server/zone/objects/tangible/components/CityDecorationMenuComponent.h"
#include "server/zone/objects/tangible/components/DestructibleCityDecorationMenuComponent.h"
#include "server/zone/objects/tangible/components/vendor/VendorDataComponent.h"

class MoveFurnitureCommand : public QueueCommand {
public:
	MoveFurnitureCommand(const String& name, ZoneProcessServer* server) : QueueCommand(name, server) {
	}

	// returns false on collision detection
	bool checkCollision(SceneObject* object, Vector3& endPoint) const {
		return CollisionManager::checkLineOfSightInParentCell(object, endPoint);
	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {
		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		auto ghost = creature->getPlayerObject();
		if (ghost == nullptr) {
			return GENERALERROR;
		}

		if (target == 0) {
			creature->sendSystemMessage("No target is selected.  What do you want to move?");
			return GENERALERROR;
		}

		auto zoneServer = server->getZoneServer();
		if (zoneServer == nullptr) {
			return GENERALERROR;
		}

		ManagedReference<SceneObject*> object = zoneServer->getObject(target);

		if (object == nullptr || !object->isTangibleObject()) {
			creature->sendSystemMessage("Target object is null or not a tangible object.");
			return GENERALERROR;
		}

		bool isTrainer = object->getObjectTemplate()->getFullTemplateString().contains("trainer");

		if (object->isPlayerCreature() || object->isPet() || (object->isCreatureObject() && !object->isVendor() && !isTrainer)) {
			creature->sendSystemMessage("Players and non-vendor/non-trainer creature objects cannot be moved with this command.");
			return GENERALERROR;
		}

		ManagedReference<SceneObject*> creatureRootParent = creature->getRootParent();
		ManagedReference<SceneObject*> objectRootParent = object->getRootParent();
		ManagedReference<BuildingObject*> buildingObject = creatureRootParent != nullptr ? (creatureRootParent->isBuildingObject() ? cast<BuildingObject*>(creatureRootParent.get()) : nullptr) : nullptr;
		ManagedReference<CityRegion*> city = nullptr;
		bool isValidCityItem = false;

		// Infinity: POB ship furniture moving (JTL)
		if (creatureRootParent != nullptr && creatureRootParent->isPobShip()) {
			PobShipObject* pobShip = creatureRootParent->asPobShip();

			if (pobShip == nullptr || objectRootParent == nullptr || (objectRootParent->getObjectID() != pobShip->getObjectID()) || pobShip->containsChildObject(object)) {
				creature->sendSystemMessage("@player_structure:move_what"); //What do you want to move?
				return GENERALERROR;
			}

			if (!pobShip->isOnAdminList(creature)) {
				creature->sendSystemMessage("@player_structure:must_be_admin"); //You must be a building admin to do that.
				return GENERALERROR;
			}
		} else if (ghost->getAdminLevel() != 15) { // Infinity: Restrictions for non-admin characters

			if (buildingObject == nullptr) { // We're outside

				if (creatureRootParent != nullptr && objectRootParent != nullptr) {
					creature->sendSystemMessage("The player and the object both need to be outside.");
					return GENERALERROR;
				}

				city = creature->getCityRegion().get();
				if (city == nullptr) { // Not in a city
					creature->sendSystemMessage("You are not located in a player city.");
					return GENERALERROR;
				}

				if (!city->isMayor(creature->getObjectID())) { // Not mayor
					creature->sendSystemMessage("You must be mayor to move objects in a city.");
					return GENERALERROR;
				}

				if (!creature->hasSkill("social_politician_master")) {
					creature->sendSystemMessage("You must be a master politician to move objects in your city.");
					return GENERALERROR;
				}

				CityDecorationMenuComponent* decorationMenu = cast<CityDecorationMenuComponent*>(object->getObjectMenuComponent());
				DestructibleCityDecorationMenuComponent* destructibleDecorationMenu = cast<DestructibleCityDecorationMenuComponent*>(object->getObjectMenuComponent());

				bool isDeco = decorationMenu != nullptr || destructibleDecorationMenu != nullptr;
				isValidCityItem = isDeco | object->isMissionTerminal() || isTrainer;

				if (!isValidCityItem) {
					creature->sendSystemMessage("You can only move objects that are city decorations, mission terminals, or trainers.");
					return GENERALERROR;
				}
			} else {
				bool onAdmin = buildingObject != nullptr && buildingObject->isOnAdminList(creature);
				bool onVendor = buildingObject != nullptr && buildingObject->isOnPermissionList("VENDOR", creature);

				if (object->isVendor()) {
					if (!onAdmin && !onVendor) {
						creature->sendSystemMessage("You must have admin or vendor access rights to the structure to move a vendor.");
						return GENERALERROR;
					}

					DataObjectComponentReference* data = object->getDataObjectComponent();
					if (data == nullptr || data->get() == nullptr || !data->get()->isVendorData()) {
						creature->sendSystemMessage("Cannot determine the owner of vendor so unable to move vendor.");
						return GENERALERROR;
					}

					VendorDataComponent* vendorData = cast<VendorDataComponent*>(data->get());
					if (vendorData == nullptr) {
						creature->sendSystemMessage("Cannot determine the owner of vendor so unable to move vendor.");
						return GENERALERROR;
					}

					if (!vendorData->isVendorOwner(creature)) {
						creature->sendSystemMessage("You are not the owner of the vendor and cannot move the vendor.");
						return GENERALERROR;
					}

				} else if (!onAdmin) {
					creature->sendSystemMessage("@player_structure:admin_move_only"); // You must be an admin to manipulate objects.
					return GENERALERROR;
				}

				if (objectRootParent != buildingObject || buildingObject->containsChildObject(object)) {
					creature->sendSystemMessage("Error moving object " + object->getCustomObjectName().toString());
					creature->sendSystemMessage("@player_structure:move_what"); // What do you want to move?
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

			if (dir != "up" && dir != "upward" && dir != "down" && dir != "downward" && dir != "forward" && dir != "ahead" && dir != "back" && dir != "backward") {
				StringBuffer message;
				message << "Invalid direction parameter " << '"' << dir << '"' << " received.  Targeted object is " << '"' << object->getCustomObjectName().toString() << '"' << ", template is " << object->getObjectTemplate()->getTemplateFileName() << ".";
				creature->sendSystemMessage(message.toString());
				throw Exception();
			}

			dist = tokenizer.getIntToken();

			if (dist < 1.f || dist > 500.f) {
				throw Exception("@player_structure:movefurniture_params"); // The amount to move must be between 1 and 500.
			}

		} catch (Exception& e) {
			creature->sendSystemMessage("@player_structure:format_movefurniture_distance"); // Format: /moveFurniture <FORWARD/AHEAD/BACK/BACKWARD/UP/UPWARD/DOWN/DOWNWARD> <distance>
			return INVALIDPARAMETERS;
		}

		float degrees = creature->getDirectionAngle();
		float radians = Math::deg2rad(degrees);

		dist /= 100.0f;

		float offsetX = dist * sin(radians);
		float offsetY = dist * cos(radians);

		float x = object->getPositionX();
		float y = object->getPositionY();
		float z = object->getPositionZ();

		if (dir == "forward" || dir == "ahead") {
			x += (offsetX);
			y += (offsetY);
		} else if (dir == "back" || dir == "backward") {
			x -= (offsetX);
			y -= (offsetY);
		} else if (dir == "up" || dir == "upward") {
			if (object->isVendor()) {
				creature->sendSystemMessage("Vendors cannot be moved up or down on the z-axis.");
				return GENERALERROR;
			}
			z += dist;
		} else if (dir == "down" || dir == "downward") {
			if (object->isVendor()) {
				creature->sendSystemMessage("Vendors cannot be moved up or down on the z-axis.");
				return GENERALERROR;
			}
			z -= dist;
		}

		Vector3 endPoint(x, y, z);

		// Infinity: Prevent moving city decorations outside city radius
		if (isValidCityItem && city != nullptr) {
			Vector3 cityCenter(city->getPositionX(), city->getPositionY(), z); // Use same z value as object to avoid elevation issues
			if (endPoint.distanceTo(cityCenter) > city->getRadius()) {
				creature->sendSystemMessage("You cannot move city decorations and objects outside of the city limit radius.");
				return GENERALERROR;
			}
		}

		if (!checkCollision(object, endPoint)) {
			creature->sendSystemMessage("@player_structure:not_valid_location"); // That is not a valid location.
			return GENERALERROR;
		}

		ManagedReference<SceneObject*> objectParent = object->getParent().get();
		if (objectParent != nullptr) {
			object->teleport(x, z, y, objectParent->getObjectID());
		} else {
			// Infinity: Use switchZone for outdoor objects to properly update z-axis movement
			auto zone = object->getZone();
			if (zone != nullptr) {
				unsigned int count = object->getMovementCounter();
				object->switchZone(zone->getZoneName(), x, z, y);
				object->setMovementCounter(count + 1);
			} else {
				object->teleport(x, z, y);
			}
		}

		return SUCCESS;
	}
};

#endif // MOVEFURNITURECOMMAND_H_
