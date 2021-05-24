/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef ROTATEFURNITURECOMMAND_H_
#define ROTATEFURNITURECOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/objects/tangible/components/EventPerkDataComponent.h"
#include "server/zone/objects/tangible/components/CityDecorationMenuComponent.h"

class RotateFurnitureCommand : public QueueCommand {
public:

	RotateFurnitureCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		ManagedReference<PlayerObject*> ghost = creature->getPlayerObject();

		if (ghost == nullptr)
			return GENERALERROR;

		String dir;
		int degrees = 0;

		try {
			UnicodeTokenizer tokenizer(arguments.toString());
			tokenizer.getStringToken(dir);
			degrees = tokenizer.getIntToken();

			dir = dir.toLowerCase();

			if (dir != "left" && dir != "right" && dir != "yaw" && dir != "roll" && dir != "pitch" && dir !="reset")
				throw Exception();

		} catch (Exception& e) {
			//creature->sendSystemMessage("@player_structure:formet_rotratefurniture_degrees"); //Format: /rotateFurniture <LEFT/RIGHT> <degrees>
			creature->sendSystemMessage("Standard Format: /rotateFurniture <LEFT/RIGHT> <degrees>. Degrees can be 1 to 180 when using this format.");
 			creature->sendSystemMessage("Enhanced Format: /rotateFurniture <YAW/PITCH/ROLL> <degrees>. Degrees can be -360 to 360 when using this format.");
 			creature->sendSystemMessage("Reset Rotation to Defaults: /rotateFurniture reset 1");

			return INVALIDPARAMETERS;

		}

		if ((dir == "left" || dir == "right") && (degrees < 1 || degrees > 180)) {
			creature->sendSystemMessage("@player_structure:rotate_params"); //The amount to rotate must be between 1 and 180.
			return INVALIDPARAMETERS;
		}

		
		if((dir == "roll" || dir == "pitch" || dir == "yaw") && (degrees < -360 || degrees > 360)) {
 			creature->sendSystemMessage("Unsing Enhanced Format: The amount to rotate must be between -360 and 360.");
 			return INVALIDPARAMETERS;
 		}

		ZoneServer* zoneServer = creature->getZoneServer();
		ManagedReference<SceneObject*> obj = zoneServer->getObject(target);

		if (obj == nullptr || !obj->isTangibleObject() || obj->isPlayerCreature() || obj->isPet()) {
			creature->sendSystemMessage("@player_structure:rotate_what"); //What do you want to rotate?
			return GENERALERROR;
		}

		ManagedReference<SceneObject*> creatureParent = creature->getRootParent();
		ManagedReference<SceneObject*> objectParent = obj->getRootParent();
		BuildingObject* buildingObject = creatureParent != nullptr ? (creatureParent->isBuildingObject() ? cast<BuildingObject*>( creatureParent.get()) : nullptr) : nullptr;

		EventPerkDataComponent* data = cast<EventPerkDataComponent*>(obj->getDataObjectComponent()->get());

		if (!(ghost->getAdminLevel() == 15))  { //// Restrictions for non-admin characters

			if (data != nullptr) {
				EventPerkDeed* deed = data->getDeed();

				if (deed == nullptr) {
					return GENERALERROR;
				}

				ManagedReference<CreatureObject*> owner = deed->getOwner().get();

				if (owner != creature) {
					return GENERALERROR;
				}

			} else if (buildingObject == nullptr) {  // We're outside

				if (creatureParent != objectParent) {
					creature->sendSystemMessage("@player_structure:move_what"); //What do you want to move?
					return GENERALERROR;
				}

				ManagedReference<CreatureObject*> mayor = creature;

				Locker locker(mayor);

				ManagedReference<CityRegion*> city = mayor->getCityRegion().get();

				if (city == nullptr) { // Not in a city
					mayor->sendSystemMessage("@player_structure:rotate_what"); //What do you want to rotate?
					return GENERALERROR;
				}

				CityManager* cityManager = mayor->getZoneServer()->getCityManager();

				if(!city->isMayor(mayor->getObjectID())) {  // Not mayor
					creature->sendSystemMessage("@player_structure:rotate_what"); //What do you want to rotate?
					return GENERALERROR;
				}

				if (!mayor->hasSkill("social_politician_master")) {
					mayor->sendSystemMessage("You must be a master politician to rotate objects in your city."); 
					return GENERALERROR;
				}

				CityDecorationMenuComponent* decorationMenu = cast<CityDecorationMenuComponent*>(obj->getObjectMenuComponent());

				if (decorationMenu == nullptr && !obj->isMissionTerminal() && !obj->getObjectTemplate()->getFullTemplateString().contains("trainer")) {
					mayor->sendSystemMessage("You can only rotate objects that are city decorations, mission terminals, or trainers."); 
					return GENERALERROR;
				}
			} 
			else {
				if (obj->isVendor() && !obj->checkContainerPermission(creature, ContainerPermissions::MOVEVENDOR)) {
					return GENERALERROR;
				}

				if (!obj->isVendor() && !buildingObject->isOnAdminList(creature)) {
					creature->sendSystemMessage("@player_structure:must_be_admin"); //You must be a building admin to do that.
					return GENERALERROR;
				}

				if (objectParent != buildingObject || buildingObject->containsChildObject(obj)) {
					creature->sendSystemMessage("@player_structure:rotate_what"); //What do you want to rotate?
					return GENERALERROR;
				}

				if (buildingObject->isGCWBase()) {
					creature->sendSystemMessage("@player_structure:no_move_hq"); // You may not move or rotate objects inside a factional headquarters.
					return GENERALERROR;
				}
			}
		}

		
 		// Modifed to add in pitch/yah/roll/reset
 		if(dir == "right")
 			obj->rotate(-degrees);
 		else if(dir == "left")
 			obj->rotate(degrees);
 		else if(dir == "yaw")
 			obj->rotate(degrees);
 		else if(dir == "pitch")
 			obj->rotateYaxis(degrees);
 		else if(dir == "roll")
 			obj->rotateXaxis(degrees);
 		else if(dir == "reset")
 			obj->setDirection(1, 0, 0, 0);
 		else
 			obj->rotate(degrees);
 		
 		/*if (dir == "right")
			obj->rotate(-degrees);
		else
			obj->rotate(degrees);
		*/
		
		obj->incrementMovementCounter();

		ManagedReference<SceneObject*> objParent = obj->getParent().get();
		if (objParent != nullptr)
			obj->teleport(obj->getPositionX(), obj->getPositionZ(), obj->getPositionY(), objParent->getObjectID());
		else
			obj->teleport(obj->getPositionX(), obj->getPositionZ(), obj->getPositionY());

		return SUCCESS;
	}

};

#endif //ROTATEFURNITURECOMMAND_H_
