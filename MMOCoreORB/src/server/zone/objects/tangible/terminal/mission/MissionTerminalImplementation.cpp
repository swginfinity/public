/*
 * MissionTerminalImplementation.cpp
 *
 *  Created on: 03/05/11
 *      Author: polonel
 */

#include "server/zone/objects/tangible/terminal/mission/MissionTerminal.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/packets/object/ObjectMenuResponse.h"
#include "server/zone/objects/region/CityRegion.h"
#include "server/zone/managers/city/CityManager.h"
#include "server/zone/managers/city/CityRemoveAmenityTask.h"
#include "server/zone/objects/player/sessions/SlicingSession.h"
//Infinity: Custom Includes
#include "server/zone/managers/director/DirectorManager.h"

void MissionTerminalImplementation::fillObjectMenuResponse(ObjectMenuResponse* menuResponse, CreatureObject* player) {
	TerminalImplementation::fillObjectMenuResponse(menuResponse, player);

	ManagedReference<CityRegion*> city = player->getCityRegion().get();

	if (city != nullptr && city->isMayor(player->getObjectID()) && getParent().get() == nullptr) {

		menuResponse->addRadialMenuItem(72, 3, "@city/city:mt_remove"); // Remove

		menuResponse->addRadialMenuItem(73, 3, "@city/city:align"); // Align
		menuResponse->addRadialMenuItemToRadialID(73, 74, 3, "@city/city:north"); // North
		menuResponse->addRadialMenuItemToRadialID(73, 75, 3, "@city/city:east"); // East
		menuResponse->addRadialMenuItemToRadialID(73, 76, 3, "@city/city:south"); // South
		menuResponse->addRadialMenuItemToRadialID(73, 77, 3, "@city/city:west"); // West
	}

	if (terminalType == "general" || terminalType == "imperial" || terminalType == "rebel") {

		//Infinity:  Display the mission direction and level choices
		Lua* lua = DirectorManager::instance()->getLuaInstance();
		if (lua == nullptr) {
			return;
		}

		menuResponse->addRadialMenuItem(112, 3, "Choose Mission Direction");

		if (!player->isGrouped()) {
			menuResponse->addRadialMenuItem(113, 3, "Choose Mission Level");
		}

		if (player->checkCooldownRecovery("selected_mission_status")) {

			//Display the mission direction and level choices
			Reference<LuaFunction*> mission_direction_choice = lua->createFunction("mission_direction_choice", "status", 0);
			if (mission_direction_choice == nullptr) {
					return;
			}
			*mission_direction_choice << player;
			mission_direction_choice->callFunction();

			if (!player->isGrouped()) {
				Reference<LuaFunction*> mission_level_choice = lua->createFunction("mission_level_choice", "status", 0);
				if (mission_level_choice == nullptr) {
					return;
				}
				*mission_level_choice << player;
				mission_level_choice->callFunction();
			}

			player->updateCooldownTimer("selected_mission_status", 5 * 60 * 1000);  //Only show status once every 5 minutes
		}	
	}
}

int MissionTerminalImplementation::handleObjectMenuSelect(CreatureObject* player, byte selectedID) {
	ManagedReference<CityRegion*> city = player->getCityRegion().get();

	if (selectedID == 69 && player->getSkillMod("slicing") >= 15) { //Infinity:  Skillmod check instead of skill box
		if (isBountyTerminal())
			return 0;

		if (city != nullptr && !city->isClientRegion() && city->isBanned(player->getObjectID())) {
			player->sendSystemMessage("@city/city:banned_services"); // You are banned from using this city's services.
			return 0;
		}

		if (player->containsActiveSession(SessionFacadeType::SLICING)) {
			player->sendSystemMessage("@slicing/slicing:already_slicing");
			return 0;
		}

		if (!player->checkCooldownRecovery("slicing.terminal")) {
			StringIdChatParameter message;
			message.setStringId("@slicing/slicing:not_yet"); // You will be able to hack the network again in %DI seconds.
			message.setDI(player->getCooldownTime("slicing.terminal")->getTime() - Time().getTime());
			player->sendSystemMessage(message);
			return 0;
		}

		//Create Session
		ManagedReference<SlicingSession*> session = new SlicingSession(player);
		session->initalizeSlicingMenu(player, _this.getReferenceUnsafeStaticCast());

		return 0;

	} else if (selectedID == 72) {

		if (city != nullptr && city->isMayor(player->getObjectID())) {
			CityRemoveAmenityTask* task = new CityRemoveAmenityTask(_this.getReferenceUnsafeStaticCast(), city);
			task->execute();

			player->sendSystemMessage("@city/city:mt_removed"); // The object has been removed from the city.
		}

		return 0;

	} else if (selectedID == 74 || selectedID == 75 || selectedID == 76 || selectedID == 77) {

		CityManager* cityManager = getZoneServer()->getCityManager();
		cityManager->alignAmenity(city, player, _this.getReferenceUnsafeStaticCast(), selectedID - 74);

		return 0;
	
	} else if (selectedID == 112) {

		Lua* lua = DirectorManager::instance()->getLuaInstance();
		if (lua == nullptr) {
			return 0;
		}

		Reference<LuaFunction*> mission_direction_choice = lua->createFunction("mission_direction_choice", "openWindow", 0);
		if (mission_direction_choice == nullptr) {
			return 0;
		}

		*mission_direction_choice << player;
		mission_direction_choice->callFunction();

		return 0;

	} else if (selectedID == 113) {

		Lua* lua = DirectorManager::instance()->getLuaInstance();
		if (lua == nullptr) {
			return 0;
		}

		Reference<LuaFunction*> mission_level_choice = lua->createFunction("mission_level_choice", "openWindow", 0);
		if (mission_level_choice == nullptr) {
			return 0;
		}

		*mission_level_choice << player;
		mission_level_choice->callFunction();

		return 0;
	}

	return TangibleObjectImplementation::handleObjectMenuSelect(player, selectedID);
}

String MissionTerminalImplementation::getTerminalName() {
	String name = "@terminal_name:terminal_mission";

	if (terminalType == "artisan" || terminalType == "entertainer" || terminalType == "bounty" || terminalType == "imperial" || terminalType == "rebel" || terminalType == "scout")
		name = name + "_" + terminalType;

	return name;
}
