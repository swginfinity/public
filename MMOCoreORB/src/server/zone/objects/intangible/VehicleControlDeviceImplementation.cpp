/*
 * VehicleControlDeviceImplementation.cpp
 *
 *  Created on: 10/04/2010
 *      Author: victor
 */

#include "server/zone/objects/intangible/VehicleControlDevice.h"
#include "server/zone/objects/intangible/VehicleControlObserver.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/objects/creature/VehicleObject.h"
#include "server/zone/objects/creature/events/VehicleDecayTask.h"
#include "server/zone/packets/scene/AttributeListMessage.h"
#include "server/zone/ZoneServer.h"
#include "server/zone/Zone.h"
#include "tasks/CallMountTask.h"
#include "server/zone/objects/region/CityRegion.h"
#include "server/zone/objects/player/sessions/TradeSession.h"
#include "server/zone/managers/player/PlayerManager.h"
#include "server/zone/managers/director/DirectorManager.h"

void VehicleControlDeviceImplementation::generateObject(CreatureObject* player) {
	if (player->isDead() || player->isIncapacitated())
		return;

	if (!isASubChildOf(player))
		return;

	ManagedReference<TangibleObject*> controlledObject = this->controlledObject.get();

	if (controlledObject == nullptr || controlledObject->getLocalZone() != nullptr) {
		return;
	}

	// Death Watch Jetpack requires Novice Death Watch skill
	bool isDwJetpack = (controlledObject->getServerObjectCRC() == 0x8A24BE7D);
	if (isDwJetpack) {
		if (!player->hasSkill("combat_death_watch_novice")) {
			player->sendSystemMessage("You must be at least Novice Death Watch to use this vehicle.");
			return;
		}
	}

	// Check if player is inside or in combat (DW jetpack has combat exception)
	if (player->getParent() != nullptr) {
		player->sendSystemMessage("@pet/pet_menu:cant_call_vehicle"); // You can only unpack vehicles while Outside and not in Combat.
		return;
	}

	if (player->isInCombat() && !isDwJetpack) {
		player->sendSystemMessage("@pet/pet_menu:cant_call_vehicle"); // You can only unpack vehicles while Outside and not in Combat.
		return;
	}

	ManagedReference<TradeSession*> tradeContainer = player->getActiveSession(SessionFacadeType::TRADE).castTo<TradeSession*>();

	if (tradeContainer != nullptr) {
		server->getZoneServer()->getPlayerManager()->handleAbortTradeMessage(player);
	}

	if (player->getPendingTask("call_mount") != nullptr) {
		StringIdChatParameter waitTime("pet/pet_menu", "call_delay_finish_vehicle");
		AtomicTime nextExecution;
		Core::getTaskManager()->getNextExecutionTime(player->getPendingTask("call_mount"), nextExecution);
		int timeLeft = (nextExecution.getMiliTime() / 1000) - System::getTime();
		waitTime.setDI(timeLeft);

		player->sendSystemMessage(waitTime);
		return;
	}

	ManagedReference<SceneObject*> datapad = player->getSlottedObject("datapad");

	if (datapad == nullptr) {
		return;
	}

	int currentlySpawned = 0;

	for (int i = 0; i < datapad->getContainerObjectsSize(); ++i) {
		ManagedReference<SceneObject*> object = datapad->getContainerObject(i);

		if (object->isVehicleControlDevice()) {
			VehicleControlDevice* device = cast<VehicleControlDevice*>(object.get());

			ManagedReference<SceneObject*> vehicle = device->getControlledObject();

			if (vehicle != nullptr && vehicle->isInQuadTree()) {
				if (++currentlySpawned > 2)
					player->sendSystemMessage("@pet/pet_menu:has_max_vehicle");

				return;
			}
		}
	}

	// DW Jetpack spawns instantly anywhere, other vehicles have 3s delay outside cities/camps
	if (player->getCurrentCamp() == nullptr && player->getCityRegion() == nullptr && !isDwJetpack) {
		Reference<CallMountTask*> callMount = new CallMountTask(_this.getReferenceUnsafeStaticCast(), player, "call_mount");

		StringIdChatParameter message("pet/pet_menu", "call_vehicle_delay");
		message.setDI(3);
		player->sendSystemMessage(message);

		player->addPendingTask("call_mount", callMount, 3 * 1000);

		if (vehicleControlObserver == nullptr) {
			vehicleControlObserver = new VehicleControlObserver(_this.getReferenceUnsafeStaticCast());
			vehicleControlObserver->deploy();
		}

		player->registerObserver(ObserverEventType::STARTCOMBAT, vehicleControlObserver);

	} else {
		Locker clocker(controlledObject, player);
		spawnObject(player);
	}
}

void VehicleControlDeviceImplementation::spawnObject(CreatureObject* player) {
	ZoneServer* zoneServer = getZoneServer();

	ManagedReference<TangibleObject*> controlledObject = this->controlledObject.get();

	if (controlledObject == nullptr)
		return;

	if (!isASubChildOf(player))
		return;

	if (player->getParent() != nullptr) {
		player->sendSystemMessage("@pet/pet_menu:cant_call_vehicle"); // You can only unpack vehicles while Outside and not in Combat.
		return;
	}

	// Death Watch Jetpack can be called in combat if player has Novice Death Watch AND is in Death Watch PvP
	bool canCallInCombat = false;

	if (controlledObject->getServerObjectCRC() == 0x8A24BE7D && player->hasSkill("combat_death_watch_novice")) {
		Lua* lua = DirectorManager::instance()->getLuaInstance();
		if (lua != nullptr) {
			Reference<LuaFunction*> canCallFunc = lua->createFunction("death_watch", "canCallJetpackInCombat", 1);
			*canCallFunc << player;
			lua_State* L = canCallFunc->callFunction();
			if (L != nullptr && lua_gettop(L) >= 1) {
				canCallInCombat = lua_toboolean(L, -1) != 0;
				lua_pop(L, 1);
			}
		}
	}

	if (player->isInCombat() && !canCallInCombat) {
		player->sendSystemMessage("@pet/pet_menu:cant_call_vehicle"); // You can only unpack vehicles while Outside and not in Combat.
		return;
	}

	auto zone = player->getZone();

	if (zone == nullptr) {
		return;
	}

	// Snowspeeder is Hoth-only (#1413)
	if (controlledObject != nullptr) {
		String vehicleTemplate = controlledObject->getObjectTemplate()->getTemplateFileName();

		if (vehicleTemplate == "snowspeeder" && zone->getZoneName() != "hoth") {
			player->sendSystemMessage("The snowspeeder can only be used on Hoth.");
			return;
		}
	}

	ManagedReference<TradeSession*> tradeContainer = player->getActiveSession(SessionFacadeType::TRADE).castTo<TradeSession*>();

	if (tradeContainer != nullptr) {
		server->getZoneServer()->getPlayerManager()->handleAbortTradeMessage(player);
	}

	Vector3 playerWorld = player->getWorldPosition();

	controlledObject->initializePosition(playerWorld.getX(), playerWorld.getZ(), playerWorld.getY());
	ManagedReference<CreatureObject*> vehicle = nullptr;

	if (controlledObject->isCreatureObject()) {
		vehicle = cast<CreatureObject*>(controlledObject.get());
		vehicle->setCreatureLink(player);
		vehicle->setControlDevice(_this.getReferenceUnsafeStaticCast());
	}

	zone->transferObject(controlledObject, -1, true);

	Reference<VehicleDecayTask*> decayTask = new VehicleDecayTask(controlledObject);

	if (decayTask != nullptr) {
		decayTask->execute();
	}

	if (vehicle != nullptr && (controlledObject->getServerObjectCRC() == 0x32F87A54 || controlledObject->getServerObjectCRC() == 0x8A24BE7D)) { // Jetpack
		if (controlledObject->getServerObjectCRC() == 0x8A24BE7D) {
			controlledObject->setCustomizationVariable("/private/index_hover_height", 80, true);				  // Illusion of flying.
		} else {
			controlledObject->setCustomizationVariable("/private/index_hover_height", 40, true);				  // Illusion of flying.
		}
		player->executeObjectControllerAction(STRING_HASHCODE("mount"), controlledObject->getObjectID(), ""); // Auto mount.
	}

	updateStatus(1);

	if (vehicleControlObserver != nullptr) {
		player->dropObserver(ObserverEventType::STARTCOMBAT, vehicleControlObserver);
	}
}

void VehicleControlDeviceImplementation::cancelSpawnObject(CreatureObject* player) {
	Reference<Task*> mountTask = player->getPendingTask("call_mount");
	if (mountTask) {
		mountTask->cancel();
		player->removePendingTask("call_mount");
	}

	if (vehicleControlObserver != nullptr)
		player->dropObserver(ObserverEventType::STARTCOMBAT, vehicleControlObserver);
}

void VehicleControlDeviceImplementation::storeObject(CreatureObject* player, bool force) {

	ManagedReference<TangibleObject*> controlledObject = this->controlledObject.get();
	if (controlledObject == nullptr)
		return;

	/*if (!controlledObject->isInQuadTree())
		return;*/

	/* Infinity:  We allow players to store vehicles in combat and dead.
	if (!force && (player->isInCombat() || player->isDead()))
		return;
	*/

	if (player->isRidingMount() && player->getParent() == controlledObject) {
		if (!force && !player->checkCooldownRecovery("mount_dismount"))
			return;

		player->executeObjectControllerAction(STRING_HASHCODE("dismount"));

		if (player->isRidingMount())
			return;
	}

	Locker crossLocker(controlledObject, player);

	Reference<Task*> decayTask = controlledObject->getPendingTask("decay");

	if (decayTask != nullptr) {
		decayTask->cancel();
		controlledObject->removePendingTask("decay");
	}

	//Infinity: Fix knocked down vehicles
	ManagedReference<CreatureObject*> creo = controlledObject->asCreatureObject();
	if (creo != nullptr) {

		if (creo->getPosture() != 0) {	//Set to upright
			creo->setPosture(0);
		}

		if (creo->getAccelerationMultiplierMod() != 1.0f) {
			creo->setAccelerationMultiplierMod(1.0f, false, false);
		}

		if (creo->getSpeedMultiplierMod() != 1.0f) {
			creo->setSpeedMultiplierMod(1.0f, false, false);
		}

		if (creo->isInCombat()) {
			creo->clearCombatState(false);  //
		}
	}

	controlledObject->destroyObjectFromWorld(true);

	if (controlledObject->isCreatureObject())
		(cast<CreatureObject*>(controlledObject.get()))->setCreatureLink(nullptr);

	crossLocker.release();

	Locker deviceLocker(_this.getReferenceUnsafeStaticCast(), player);

	updateStatus(0);

	ManagedReference<VehicleObject*> vehicle = cast<VehicleObject*>(controlledObject.get());

	if (vehicle != nullptr && vehicle->isRentalVehicle() && vehicle->getRentalUses() <= 0) {
		destroyObjectFromWorld(true);
		destroyObjectFromDatabase(true);

		StringIdChatParameter param;
		param.setStringId("pet/pet_menu", "uses_complete");
		player->sendSystemMessage(param.toString());

		return;
	}
}

void VehicleControlDeviceImplementation::destroyObjectFromDatabase(bool destroyContainedObjects) {
	ManagedReference<TangibleObject*> controlledObject = this->controlledObject.get();

	if (controlledObject != nullptr) {
		Locker locker(controlledObject);

		ManagedReference<CreatureObject*> object = controlledObject->getSlottedObject("rider").castTo<CreatureObject*>();

		if (object != nullptr) {
			Locker clocker(object, controlledObject);

			object->executeObjectControllerAction(STRING_HASHCODE("dismount"));

			object = controlledObject->getSlottedObject("rider").castTo<CreatureObject*>();

			if (object != nullptr) {
				controlledObject->removeObject(object, nullptr, true);

				Zone* zone = getZone();

				if (zone != nullptr)
					zone->transferObject(object, -1, true);
			}
		}

		controlledObject->destroyObjectFromDatabase(true);
	}

	IntangibleObjectImplementation::destroyObjectFromDatabase(destroyContainedObjects);
}

int VehicleControlDeviceImplementation::canBeDestroyed(CreatureObject* player) {
	ManagedReference<TangibleObject*> controlledObject = this->controlledObject.get();

	if (controlledObject != nullptr) {
		if (controlledObject->isInQuadTree()) {
			if (player != nullptr) {
				player->sendSystemMessage("You must store your vehicle before you can delete it.");
			}
			return 1;
		}
	}

	return IntangibleObjectImplementation::canBeDestroyed(player);
}

bool VehicleControlDeviceImplementation::canBeTradedTo(CreatureObject* player, CreatureObject* receiver, int numberInTrade) {
	ManagedReference<SceneObject*> datapad = receiver->getSlottedObject("datapad");

	if (datapad == nullptr)
		return false;

	ManagedReference<PlayerManager*> playerManager = player->getZoneServer()->getPlayerManager();

	int vehiclesInDatapad = numberInTrade;
	int maxStoredVehicles = playerManager->getBaseStoredVehicles();

	for (int i = 0; i < datapad->getContainerObjectsSize(); i++) {
		Reference<SceneObject*> obj = datapad->getContainerObject(i).castTo<SceneObject*>();

		if (obj != nullptr && obj->isVehicleControlDevice()) {
			vehiclesInDatapad++;
		}
	}

	if (vehiclesInDatapad >= maxStoredVehicles) {
		player->sendSystemMessage("That person has too many vehicles in their datapad");
		receiver->sendSystemMessage("@pet/pet_menu:has_max_vehicle"); // You already have the maximum number of vehicles that you can own.
		return false;
	}

	return true;
}

void VehicleControlDeviceImplementation::fillAttributeList(AttributeListMessage* alm, CreatureObject* object) {
	SceneObjectImplementation::fillAttributeList(alm, object);

	if (this->controlledObject == nullptr)
		return;

	ManagedReference<VehicleObject*> vehicle = this->controlledObject.get().castTo<VehicleObject*>();
	if (vehicle == nullptr)
		return;

	if (vehicle->getPaintCount() > 0) {
		alm->insertAttribute("customization_cnt", vehicle->getPaintCount());
	}
}
