/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef FINDDECOCOMMAND_H_
#define FINDDECOCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/objects/player/PlayerObject.h"
#include "server/zone/objects/player/sui/callbacks/FindDecoSuiCallback.h"
#include "server/zone/objects/tangible/components/CityDecorationMenuComponent.h"
#include "server/zone/objects/tangible/components/DestructibleCityDecorationMenuComponent.h"

class FindDecoCommand : public QueueCommand {
public:

	FindDecoCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {

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

		auto zoneServer = creature->getZoneServer();
		if (zoneServer == nullptr)
			return GENERALERROR;

		auto zone = creature->getZone();
		if (zone == nullptr)
			return GENERALERROR;

		if (!creature->hasSkill("social_politician_master") && !ghost->isAdmin()) {
			creature->sendSystemMessage("You must be a master politician to use this command.");
			return GENERALERROR;
		}

		ManagedReference<CityRegion*> city = creature->getCityRegion().get();

		if (city == nullptr) {
			creature->sendSystemMessage("You must be in a player city to use this command."); 
			return GENERALERROR;
		}

		if (!city->isMayor(creature->getObjectID()) && !ghost->isAdmin()) { 
			creature->sendSystemMessage("You must be in a player city and be the mayor to use this command."); 
			return GENERALERROR;
		}

		if (creature->getRootParent() != nullptr) {  // Must be outside
			creature->sendSystemMessage("You must be outside in a player city to use this command."); 
			return GENERALERROR;
		}

		try {

			StringTokenizer tokenizer(arguments.toString());
			String objectFilter = "";

			if (tokenizer.hasMoreTokens())
				tokenizer.getStringToken(objectFilter);

			int cityRadius = city->getRadius();
			int range = cityRadius;

			if (tokenizer.hasMoreTokens())
				range = tokenizer.getIntToken();

			if (range > cityRadius || range <= 0)
				range = cityRadius;

			ManagedReference<SuiListBox*> findResults = new SuiListBox(creature, SuiWindowType::FIND_DECO);
			findResults->setCallback(new FindDecoSuiCallback(zoneServer));
			findResults->setPromptTitle("Find Decoration");
			findResults->setPromptText("Here are the decoration objects found within your city:");
			findResults->setCancelButton(true, "");
			findResults->setOkButton(true, "@ui:teleport_decoration");
			findResults->setOtherButton(true, "@ui:remove_decoration");

			SortedVector<ManagedReference<TreeEntry*> > objects(512, 512);

			if (range == cityRadius) { //Search from center of city
				zone->getInRangeObjects(city->getPositionX(), 0, city->getPositionY(), range, &objects, true);
			}
			else { //Search from creature using custom range
				zone->getInRangeObjects(creature->getPositionX(), 0, creature->getPositionY(), range, &objects, true);
			}
			for (int i = 0; i < objects.size(); ++i) {

				ManagedReference<SceneObject*> object = cast<SceneObject*>(objects.get(i).get());

				if (object == nullptr)
					continue;

				if (object == creature)
					continue;

				if (object->getRootParent() != nullptr)  //Skip anything inside a cell
					continue;

				if (object->isClientObject())
					continue;

 				if (!city->isCityDecoration(object))
					continue;

 				if (object->isStructureObject())
					continue;

				Locker clocker(object, creature);

				CityDecorationMenuComponent* decorationMenu = cast<CityDecorationMenuComponent*>(object->getObjectMenuComponent());
				DestructibleCityDecorationMenuComponent* destructibleDecorationMenu = static_cast<DestructibleCityDecorationMenuComponent*>(object->getObjectMenuComponent());

				if (decorationMenu == nullptr && destructibleDecorationMenu == nullptr) { // Not city deco
					continue;
				}

				String name = object->getDisplayedName();

				if (objectFilter != "") {
					if (objectFilter != "-r") {
						if (!name.toLowerCase().contains(objectFilter.toLowerCase())) {
							continue;
						}
					}
				}

				StringBuffer results;	
				results << name;
				results << "  (" << String::format("%.1f", object->getWorldPositionX());
				results << ", " << String::format("%.1f", object->getWorldPositionY());
				results << ", " << String::format("%.1f", object->getWorldPositionZ());
				results << ")";
				findResults->addMenuItem(results.toString(), object->getObjectID());
			}

			if (findResults->getMenuSize() < 1) {
				creature->sendSystemMessage("No objects were found that matched those parameters.");
				return SUCCESS;
			}

			ghost->addSuiBox(findResults);
			creature->sendMessage(findResults->generateMessage());

		} catch (Exception& e) {
			creature->sendSystemMessage("Syntax: /finddeco <string filter> <range>");
			creature->sendSystemMessage("Syntax: /finddeco <-r> <range>");
		}

		return SUCCESS;
	}

};

#endif //FINDDECOCOMMAND_H_
