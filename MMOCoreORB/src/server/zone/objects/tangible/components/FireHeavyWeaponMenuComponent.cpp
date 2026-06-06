/*
 * FireHeavyWeaponMenuComponent.cpp
 *
 *  Created on: May 10, 2012
 *      Author: chris.rush
 */

#include "FireHeavyWeaponMenuComponent.h"
#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/packets/object/ObjectMenuResponse.h"
#include "server/zone/managers/director/DirectorManager.h"

void FireHeavyWeaponMenuComponent::fillObjectMenuResponse(SceneObject* sceneObject, ObjectMenuResponse* menuResponse, CreatureObject* player) const {
	if (sceneObject == nullptr || !sceneObject->isWeaponObject() || player == nullptr)
		return;

	WeaponObjectMenuComponent::fillObjectMenuResponse(sceneObject, menuResponse, player);

	if (sceneObject->getObjectTemplate()->getTemplateFileName() == "heavy_rocket_launcher") {
		if (player->hasSkill("combat_demolition_pyro_3")) {
			menuResponse->addRadialMenuItem(232, 3, "Paint Target"); 
		}
	}
	if (sceneObject->getObjectTemplate()->getTemplateFileName() == "heavy_particle_beam") {
		if (player->hasSkill("combat_demolition_master")) {
			menuResponse->addRadialMenuItem(232, 3, "Disintegrate Target"); 
		}
	}
	if (sceneObject->getObjectTemplate()->getTemplateFileName() == "heavy_lightning_beam") {
		if (player->hasSkill("combat_death_watch_sun_2")) {
			menuResponse->addRadialMenuItem(233, 3, "Lock Target Location");
		}
	}
}

int FireHeavyWeaponMenuComponent::handleObjectMenuSelect(SceneObject* sceneObject, CreatureObject* player, byte selectedID) const {
	if (!sceneObject->isWeaponObject() || !player->isPlayerCreature() || !sceneObject->isASubChildOf(player))
		return 0;

	if (selectedID == 20) {
		player->sendCommand(STRING_HASHCODE("fireheavyweapon"), String::valueOf(sceneObject->getObjectID()), player->getTargetID());

		return 1;
	}
	else if (selectedID == 233) {
		if (player->hasSkill("combat_death_watch_sun_2")) {
			Lua* lua = DirectorManager::instance()->getLuaInstance();
			Reference<LuaFunction*> luaObtainData = lua->createFunction("dw_special_attacks", "heavyLightningBeamLock", 0);
			*luaObtainData << player;
			*luaObtainData << sceneObject;

			luaObtainData->callFunction();
		}
		return 1;
	}
	return WeaponObjectMenuComponent::handleObjectMenuSelect(sceneObject, player, selectedID);
}
