/*
 * DirectorManager.cpp
 *
 *  Created on: 26/05/2011
 *      Author: victor
 */

#include "DirectorManager.h"
#include "server/zone/TreeEntry.h"
#include "server/zone/objects/cell/CellObject.h"
#include "server/zone/objects/creature/LuaCreatureObject.h"
#include "templates/params/creature/ObjectFlag.h"
#include "server/zone/objects/scene/LuaSceneObject.h"
#include "server/zone/objects/building/LuaBuildingObject.h"
#include "server/zone/objects/intangible/LuaIntangibleObject.h"
#include "server/zone/objects/mission/LuaMissionObject.h"
#include "server/zone/objects/intangible/ControlDevice.h"
#include "server/zone/objects/intangible/PetControlDevice.h"
#include "server/zone/objects/player/LuaPlayerObject.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/objects/tangible/LuaTangibleObject.h"
#include "server/zone/objects/region/LuaCityRegion.h"
#include "server/zone/packets/cell/UpdateCellPermissionsMessage.h"
#include "server/zone/managers/structure/tasks/DestroyStructureTask.h"
#include "server/zone/managers/object/ObjectManager.h"
#include "server/zone/managers/structure/StructureManager.h"
#include "server/zone/managers/faction/FactionManager.h"
#include "server/zone/managers/combat/CombatManager.h"
#include "server/zone/managers/collision/PathFinderManager.h"
#include "server/zone/objects/tangible/threat/ThreatMap.h"
#include "templates/manager/TemplateManager.h"
#include "server/zone/managers/stringid/StringIdManager.h"
#include "server/zone/managers/name/NameManager.h"
#include "server/zone/managers/collision/CollisionManager.h"
#include "server/zone/managers/director/ScreenPlayObserver.h"
#include "server/zone/managers/director/PersistentEvent.h"
#include "server/zone/managers/creature/CreatureManager.h"
#include "server/zone/managers/creature/PetManager.h"
#include "server/zone/managers/planet/PlanetManager.h"
#include "server/zone/managers/player/PlayerManager.h"
#include "server/zone/managers/player/creation/PlayerCreationManager.h"
#include "server/zone/managers/reaction/ReactionManager.h"
#include "server/ServerCore.h"
#include "server/chat/ChatManager.h"
#include "server/chat/ChatMessage.h"
#include "server/zone/managers/loot/LootManager.h"
#include "templates/params/ObserverEventType.h"
#include "templates/params/creature/CreatureAttribute.h"
#include "templates/params/creature/CreatureState.h"
#include "templates/params/creature/CreaturePosture.h"
#include "server/zone/objects/creature/ai/LuaAiAgent.h"
#include "server/zone/objects/area/LuaActiveArea.h"
#include "server/zone/objects/creature/conversation/ConversationScreen.h"
#include "server/zone/objects/creature/conversation/ConversationTemplate.h"
#include "server/zone/objects/creature/conversation/LuaConversationScreen.h"
#include "server/zone/objects/creature/conversation/LuaConversationTemplate.h"
#include "server/zone/objects/player/sessions/LuaConversationSession.h"
#include "server/zone/objects/tangible/terminal/startinglocation/StartingLocationTerminal.h"
#include "server/zone/objects/group/GroupObject.h"
#include "server/zone/managers/sui/LuaSuiManager.h"
#include "server/zone/managers/skill/LuaSkillManager.h"
#include "server/zone/objects/player/sui/LuaSuiBox.h"
#include "server/zone/objects/scene/components/LuaObjectMenuResponse.h"
#include "server/zone/objects/scene/variables/ContainerPermissions.h"
#include "server/zone/objects/tangible/deed/Deed.h"
#include "server/zone/managers/gcw/GCWManager.h"
#include "server/zone/managers/jedi/JediManager.h"
#include "server/zone/managers/skill/SkillManager.h"
#include "server/zone/objects/region/CityRegion.h"
#include "server/zone/managers/creature/CreatureTemplateManager.h"
#include "server/zone/managers/creature/AiMap.h"
#include "server/chat/LuaStringIdChatParameter.h"
#include "server/zone/objects/tangible/ticket/TicketObject.h"
#include "server/zone/objects/player/sui/SuiWindowType.h"
#include "server/zone/packets/scene/PlayClientEffectLocMessage.h"
#include "server/zone/managers/player/BadgeList.h"
#include "server/zone/managers/player/LuaQuestInfo.h"
#include "server/zone/managers/player/LuaQuestTask.h"
#include "server/zone/managers/player/LuaQuestTasks.h"
#include "server/zone/objects/tangible/misc/FsPuzzlePack.h"
#include "server/zone/objects/tangible/misc/FsCsObject.h"
#include "server/zone/objects/tangible/misc/CustomIngredient.h"
#include "server/zone/objects/tangible/misc/FsCraftingComponentObject.h"
#include "server/zone/objects/tangible/misc/FsBuffItem.h"
#include "server/zone/objects/player/sui/LuaSuiPageData.h"
#include "server/zone/objects/player/sui/SuiBoxPage.h"
#include "server/zone/objects/tangible/powerup/PowerupObject.h"
#include "server/zone/objects/resource/ResourceSpawn.h"
#include "server/zone/objects/tangible/component/Component.h"
#include "server/zone/objects/pathfinding/NavArea.h"
#include "server/zone/objects/player/sui/listbox/LuaSuiListBox.h"
#include "server/zone/objects/tangible/component/lightsaber/LightsaberCrystalComponent.h"
#include "server/zone/objects/creature/variables/LuaSkill.h"
#include "server/zone/objects/intangible/TheaterObject.h"
#include "server/zone/objects/tangible/misc/ContractCrate.h"
#include "server/zone/objects/tangible/spawning/SpawnEggObject.h"
#include "server/zone/managers/crafting/schematicmap/SchematicMap.h"
#include "server/zone/managers/resource/ResourceManager.h"
#include "server/zone/managers/gcw/observers/SquadObserver.h"
#include "server/zone/managers/ship/ShipManager.h"
#include "templates/params/ship/ShipFlag.h"
#include "templates/params/creature/PlayerArrangement.h"
#include "server/zone/objects/ship/components/ShipChassisComponent.h"

#include "server/zone/objects/ship/LuaShipObject.h"
#include "server/zone/objects/ship/LuaPobShipObject.h"
#include "server/zone/objects/ship/ai/LuaShipAiAgent.h"
#include "server/zone/objects/ship/components/LuaShipComponent.h"
#include "server/zone/objects/ship/components/ShipComponent.h"
#include "server/zone/objects/area/space/SpaceActiveArea.h"
#include "server/zone/objects/area/areashapes/SphereAreaShape.h"
#include "server/zone/packets/ui/CreateClientPathMessage.h"
#include "server/zone/objects/ship/squadron/ShipSquadronFormation.h"
// Infinity:  Custom includes
#include "server/zone/objects/creature/buffs/BuffCRC.h"
#include "server/zone/objects/area/areashapes/RectangularAreaShape.h"
#include "terrain/ProceduralTerrainAppearance.h"
#include "server/zone/packets/chat/ChatRoomMessage.h"
#include "server/zone/managers/group/GroupManager.h"
#include "server/zone/managers/death_watch/DeathWatchManager.h"

int DirectorManager::DEBUG_MODE = 0;
int DirectorManager::ERROR_CODE = NO_ERROR;

SynchronizedHashTable<uint32, Reference<PersistentEvent*> > DirectorManager::persistentEvents;

DirectorManager::DirectorManager() : Logger("DirectorManager") {
	if (!DEBUG_MODE)
		info("loading..", true);

	sharedMemory = new DirectorSharedMemory();
	sharedMemory->setNullValue(0);

	screenPlays.setNullValue(false);
	screenPlays.setNoDuplicateInsertPlan();

	questStatuses.setNullValue(nullptr);
	questStatuses.setNoDuplicateInsertPlan();

	questVectorMaps.setNullValue(nullptr);
	questVectorMaps.setNoDuplicateInsertPlan();

	masterScreenPlayVersion.set(0);
}

DirectorManager::~DirectorManager() {
}

void DirectorManager::loadPersistentEvents() {
	info("Loading persistent events from events.db");

	ObjectDatabaseManager* dbManager = ObjectDatabaseManager::instance();
	ObjectDatabase* eventDatabase = ObjectDatabaseManager::instance()->loadObjectDatabase("events", true);

	if (eventDatabase == nullptr) {
		error("Could not load the event database.");
		return;
	}

	int i = 0;

	try {
		ObjectDatabaseIterator iterator(eventDatabase);

		uint64 objectID;

		while (iterator.getNextKey(objectID)) {
			Reference<PersistentEvent*> event = Core::getObjectBroker()->lookUp(objectID).castTo<PersistentEvent*>();
			Locker locker(event);

			++i;

			Reference<PersistentEvent*> oldEvent = persistentEvents.put(event->getEventName().hashCode(), event);

			if (event != nullptr && oldEvent != nullptr) {
				error("duplicate persistent event " + event->getEventName() + " loading from database!");
			} else if (event != nullptr) {
				event->loadTransientTask();
			} else {
				error("could not load PersistentEvent!");
			}

			if (ConfigManager::instance()->isProgressMonitorActivated())
				printf("\r\tLoading persistent events [%d] / [?]\t", i);
		}
	} catch (DatabaseException& e) {
		error("Database exception in DirectorManager::loadPersistentEvents(): "	+ e.getMessage());
	}

	info(true) << i << " persistent events loaded.";
}

void DirectorManager::loadPersistentStatus() {
	info("Loading persistent quest status from questdata.db");

	ObjectDatabaseManager* dbManager = ObjectDatabaseManager::instance();
	ObjectDatabase* statusDatabase = dbManager->loadObjectDatabase("questdata", true);

	if (statusDatabase == nullptr) {
		error("Could not load the questdata database.");
		return;
	}

    int keyDeleteCount = 0;
    int moverDeleteCount = 0;

	try {
		ObjectDatabaseIterator iterator(statusDatabase);

		uint64 objectID;

        Vector<String> keysToDelete = {"geonosian:code:", "hg:safeZone:which", "newplayerservicesterminal:code", "gm:sui", "buffbot:patient"}; 
                                        //"instant_pve","kash_raids", "stadium", "wordle"}; // keys to check for removal

		while (iterator.getNextKey(objectID)) {
			Reference<QuestStatus*> status = Core::getObjectBroker()->lookUp(objectID).castTo<QuestStatus*>();

			if (status != nullptr) {
                const String key = status->getKey();
				questStatuses.put(key, status);

                bool continueOuter = true;

                for (int i = 0; i < keysToDelete.size(); ++i) {
                    const String targetKey = keysToDelete.get(i);

                    //error("key = " +  status->getKey() + ", targetKey = " + targetKey);

                    if (key.contains(targetKey)) {
                        removeQuestStatus(key);
                        keyDeleteCount++;
                        error("Deleted persistent status, key = " +  status->getKey() + ", status = " + status->getStatus());
                        continueOuter = false;
                        break;
                    }
                }

                if (not continueOuter) {
                    continue;
                }

                if (false and key.contains(":house:")) {//Disable for now
                    String sid = key.subString(0, key.indexOf(':'));
                    uint64 oid = Long::unsignedvalueOf(sid);
                    auto zoneServer = ServerCore::getZoneServer();
                    if (zoneServer != nullptr) {

                        ManagedReference<SceneObject*> sceno = zoneServer->getObject(oid);

                        if (sceno == nullptr) {
                            removeQuestStatus(key);

                            String moveID = getQuestStatus(sid + ":house:moveID:");

                            if (moveID != "") {
                                removeQuestStatus(moveID + ":house:diskStatus:");
                                removeQuestStatus(moveID + ":house:sid:");
		                        removeQuestStatus(moveID + ":house:diskName:");
                                removeQuestStatus(moveID + ":house:player:");
                                removeQuestStatus(moveID + ":house:ix:");
                                removeQuestStatus(moveID + ":house:iy:");
                                removeQuestStatus(moveID + ":house:iz:");

                                removeQuestStatus(moveID + ":house:iloc:");
                                removeQuestStatus(moveID + ":house:izone:");
                                removeQuestStatus(moveID + ":house:icell:");
                            }

                            moverDeleteCount++;
                            continue;
                        }
                    }
                }

                //error("Loaded persistent status, key = " +  status->getKey() + ", status = " + status->getStatus());
			}

			Reference<QuestVectorMap*> questMap = Core::getObjectBroker()->lookUp(objectID).castTo<QuestVectorMap*>();

			if (questMap != nullptr)
				questVectorMaps.put(questMap->getKey(), questMap);
		}
	} catch (DatabaseException& e) {
		error("Database exception in DirectorManager::loadPersistentStatus(): "	+ e.getMessage());
	}

    log() << "Deleted " << keyDeleteCount << " persistent statuses and " << moverDeleteCount << " :house:object statuses.";

	info(true) << questStatuses.size() + questVectorMaps.size() << " persistent statuses loaded.";
}

void DirectorManager::setQuestStatus(const String& keyString, const String& valString) {
	ManagedReference<QuestStatus*> status = questStatuses.get(keyString);

	if (status == nullptr) {
		status = new QuestStatus();
		status->setKey(keyString);
		questStatuses.put(keyString, status);

		ObjectManager::instance()->persistObject(status, 1, "questdata");
	}

	status->setStatus(valString);
}

String DirectorManager::getQuestStatus(const String& keyString) const {
	String str = "";

	Reference<QuestStatus*> status = questStatuses.get(keyString);
	if (status != nullptr)
		str = status->getStatus();

	return str;
}

void DirectorManager::removeQuestStatus(const String& key) {
	ManagedReference<QuestStatus*> status = nullptr;

	status = questStatuses.get(key);

	questStatuses.drop(key);

	if (status != nullptr)
		ObjectManager::instance()->destroyObjectFromDatabase(status->_getObjectID());
}

String DirectorManager::readStringSharedMemory(const String& key) {
#ifndef WITH_STM
	DirectorManager::instance()->rlock();
#endif

	String data = DirectorManager::instance()->sharedMemory->getString(key);

#ifndef WITH_STM
	DirectorManager::instance()->runlock();
#endif

	return data;
}

Vector3 DirectorManager::readVector3SharedMemory(const String& key) {
#ifndef WITH_STM
	DirectorManager::instance()->rlock();
#endif

	Vector3 data = DirectorManager::instance()->sharedMemory->getVector3(key);

#ifndef WITH_STM
	DirectorManager::instance()->runlock();
#endif

	return data;
}

uint64 DirectorManager::readSharedMemory(const String& key) {
#ifndef WITH_STM
	DirectorManager::instance()->rlock();
#endif

	uint64 data = DirectorManager::instance()->sharedMemory->get(key);

#ifndef WITH_STM
	DirectorManager::instance()->runlock();
#endif

	return data;
}

Vector<String> DirectorManager::readStringVectorSharedMemory(const String& key) {
#ifndef WITH_STM
	DirectorManager::instance()->rlock();
#endif

	Vector<String> table = DirectorManager::instance()->sharedMemory->getStringVector(key);

#ifndef WITH_STM
	DirectorManager::instance()->runlock();
#endif

	return table;
}

Vector<Reference<ScreenPlayTask*> > DirectorManager::getObjectEvents(SceneObject* obj) const {
	Vector<Reference<ScreenPlayTask*> > eventList;

	for (int i = 0; i < screenplayTasks.size(); i++) {
		Reference<ScreenPlayTask*> task = screenplayTasks.get(i);

		if (task->getSceneObject() == obj) {
			eventList.emplace(std::move(task));
		}
	}

	return eventList;
}

Logger& DirectorManager::getEventLogger() {
	auto static eventLogger = [] () {
		Logger eventLogger("LuaEventLogger");

		eventLogger.setFileLogger("log/luaEvent.log", true, ConfigManager::instance()->getRotateLogAtStart());
		eventLogger.setLogToConsole(false);
		eventLogger.setRotateLogSizeMB(ConfigManager::instance()->getRotateLogSizeMB());
		eventLogger.setLogLevel(ConfigManager::instance()->getLogLevel("Core3.LuaEngine.LuaEventLogLevel", Logger::INFO));

		return eventLogger;
	} ();

	return eventLogger;
};

void DirectorManager::printTraceError(lua_State* L, const String& error) {
    if (L == nullptr) {
        Logger::console.error("DirectorManager::printTraceError - Lua state is null, cannot log error.");
        return;
    }

    Lua* lua = instance()->getLuaInstance();

    if (lua == nullptr) {
        Logger::console.error("DirectorManager::printTraceError - Lua instance is nullptr, cannot log error.");
        return;
    }

    StringBuffer msg;

    // Add the error message
    msg << error;

    // Add stack traceback
    msg << endl << "----- Lua Call Stack -----";

    try {
        lua_Debug info;
        int level = 1; // Start at level 1 to skip the C function itself
        bool anyFrames = false;

        while (lua_getstack(L, level, &info)) {
            lua_getinfo(L, "nSl", &info);

            String source = info.source ? info.source : "Unknown";
            int currentLine = info.currentline;
            int lineDefined = info.linedefined;

            // Format the stack frame
            msg << endl << "  ";
            if (String(info.what) == "C") {
                msg << "[C]: in " << (info.name ? String(info.name) : "function");
            } else {
                // Strip leading '@' from source for cleaner output
                if (source.beginsWith("@")) {
                    source = source.subString(1);
                }
                msg << source << ":" << currentLine << ": in " << (info.name ? String(info.name) : "function");
                if (lineDefined > 0) {
                    msg << " (defined at " << source << ":" << lineDefined << ")";
                }
            }
            anyFrames = true;

            ++level;
        }

        if (!anyFrames) {
            msg << endl <<  "\t(No Lua stack frames available)";
        }
    } catch (...) {
        msg << endl << "\t(Exception occurred while dumping Lua stack)";
    }

    // Report the error
    lua->error(msg.toString());
}

String DirectorManager::getStringSharedMemory(const String& key) const {
	return sharedMemory->getString(key);
}

void DirectorManager::startGlobalScreenPlays() {
	info(true) << "Starting " << screenPlays.size() << " global screenplays.";

	auto slowLoadMs = ConfigManager::instance()->getInt("Core3.DirectorManager.SlowLoadMs", 1000);

	Time nextReport;
	nextReport.addMiliTime(5000);
	int countStarted = 0;

	Timer profileAll;
	profileAll.start();

	for (int i = 0; i < screenPlays.size(); ++i) {
		String screenPlay = screenPlays.elementAt(i).getKey();
		bool start = screenPlays.elementAt(i).getValue();

		if (!nextReport.isFuture()) {
			nextReport.updateToCurrentTime();
			nextReport.addMiliTime(5000);
			info(true) << "Started " << countStarted << " of " << screenPlays.size() << " screenplays.";
		}

		if (start) {
			Timer profileStart;
			profileStart.start();

			if (ConfigManager::instance()->getBool("Core3.DirectorManager.LogScreenplayStart", false)) {
				info(true) << "Starting screenplay: " << screenPlay;
			}

			startScreenPlay(nullptr, screenPlay);
			auto elapsedMs = profileStart.stopMs();

			if (elapsedMs > slowLoadMs) {
				info(true) << "Slow screenplay: " << screenPlay << " took " << msToString(elapsedMs) << " to start.";
			}
			countStarted++;
		}
	}

	info(true) << "Started " << countStarted << " global screenplays in " << msToString(profileAll.stopMs());
}

void DirectorManager::setupLuaPackagePath(Lua* luaEngine) {
	lua_State* L = luaEngine->getLuaState();
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "path");
	String currentPath = lua_tostring(L, -1);
	currentPath = currentPath.concat(";scripts/?.lua");
	lua_pop(L, 1);
	lua_pushstring(L, currentPath.toCharArray());
	lua_setfield(L, -2, "path");
	lua_pop(L, 1);
}

void DirectorManager::initializeLuaEngine(Lua* luaEngine) {
	if (DEBUG_MODE)
		setLogLevel(Logger::DEBUG);

	debug("initializeLuaEngine started");

	luaEngine->init();
	luaEngine->setLoggingName("DirectorManagerLuaInstance");
	luaEngine->setGlobalLogging(true);

	if (DEBUG_MODE) {
		luaEngine->setLogLevel(Logger::DEBUG);
	} else {
		luaEngine->setLogLevel(ConfigManager::instance()->getLogLevel("Core3.LuaEngine.LogLevel", Logger::ERROR));
		luaEngine->setLogToConsole(false);
	}

	luaEngine->setFileLogger("log/lua.log", true, ConfigManager::instance()->getRotateLogAtStart());
	luaEngine->setLogJSON(ConfigManager::instance()->getLuaLogJSON());
	luaEngine->setRotateLogSizeMB(ConfigManager::instance()->getRotateLogSizeMB());

	if (luaEngine->getLogJSON()) {
		luaEngine->setLogSynchronized(true);
	}

	setupLuaPackagePath(luaEngine);

	//luaEngine->registerFunction("includeFile", includeFile);
	luaEngine->registerFunction("includeFile", includeFile);
	luaEngine->registerFunction("createEvent", createEvent);
	luaEngine->registerFunction("cancelEvent", cancelEvent);
	luaEngine->registerFunction("createEventActualTime", createEventActualTime);
	luaEngine->registerFunction("createServerEvent", createServerEvent);
	luaEngine->registerFunction("hasServerEvent", hasServerEvent);
	luaEngine->registerFunction("rescheduleServerEvent", rescheduleServerEvent);
	luaEngine->registerFunction("getServerEventID", getServerEventID);
	luaEngine->registerFunction("getServerEventTimeLeft", getServerEventTimeLeft);
	luaEngine->registerFunction("createObserver", createObserver);
	luaEngine->registerFunction("dropObserver", dropObserver);
	luaEngine->registerFunction("hasObserver", hasObserver);
	luaEngine->registerFunction("hasObserverType", hasObserverType);
	luaEngine->registerFunction("spawnMobile", spawnMobile);
	luaEngine->registerFunction("spawnEventMobile", spawnEventMobile);
	luaEngine->registerFunction("spawnShipAgent", spawnShipAgent);
	luaEngine->registerFunction("spatialChat", spatialChat);
	luaEngine->registerFunction("spatialMoodChat", spatialMoodChat);
	luaEngine->registerFunction("getRandomNumber", getRandomNumber);
	luaEngine->registerFunction("getHashCode", getHashCode);
	luaEngine->registerFunction("forcePeace", forcePeace);
	luaEngine->registerFunction("readSharedMemory", readSharedMemory);
	luaEngine->registerFunction("writeSharedMemory", writeSharedMemory);
	luaEngine->registerFunction("deleteSharedMemory", deleteSharedMemory);
	luaEngine->registerFunction("readStringSharedMemory", readStringSharedMemory);
	luaEngine->registerFunction("writeStringSharedMemory", writeStringSharedMemory);
	luaEngine->registerFunction("deleteStringSharedMemory", deleteStringSharedMemory);
	luaEngine->registerFunction("readVector3SharedMemory", readVector3SharedMemory);
	luaEngine->registerFunction("writeVector3SharedMemory", writeVector3SharedMemory);
	luaEngine->registerFunction("deleteVector3SharedMemory", deleteVector3SharedMemory);
	luaEngine->registerFunction("readStringVectorSharedMemory", readStringVectorSharedMemory);
	luaEngine->registerFunction("writeStringVectorSharedMemory", writeStringVectorSharedMemory);
	luaEngine->registerFunction("deleteStringVectorSharedMemory", deleteStringVectorSharedMemory);
	luaEngine->registerFunction("spawnSceneObject", spawnSceneObject);
	luaEngine->registerFunction("spawnActiveArea", spawnActiveArea);
	luaEngine->registerFunction("spawnSpaceActiveArea", spawnSpaceActiveArea);
	luaEngine->registerFunction("spawnBuilding", spawnBuilding);
	luaEngine->registerFunction("spawnSecurityPatrol", spawnSecurityPatrol);
	luaEngine->registerFunction("despawnSecurityPatrol", despawnSecurityPatrol);
	luaEngine->registerFunction("destroyBuilding", destroyBuilding);
	luaEngine->registerFunction("getSceneObject", getSceneObject);
	luaEngine->registerFunction("getCreatureObject", getCreatureObject);
	luaEngine->registerFunction("addStartingItemsInto", addStartingItemsInto);
	luaEngine->registerFunction("addStartingWeaponsInto", addStartingWeaponsInto);
	luaEngine->registerFunction("setAuthorizationState", setAuthorizationState);
	luaEngine->registerFunction("giveItem", giveItem);
	luaEngine->registerFunction("giveControlDevice", giveControlDevice);
	luaEngine->registerFunction("checkTooManyHirelings", checkTooManyHirelings);
	luaEngine->registerFunction("checkInt64Lua", checkInt64Lua);
	luaEngine->registerFunction("getChatMessage", getChatMessage);
	luaEngine->registerFunction("getStringId", getStringId);
	luaEngine->registerFunction("getRankName", getRankName);
	luaEngine->registerFunction("getRankCost", getRankCost);
	luaEngine->registerFunction("getRankDelegateRatioFrom", getRankDelegateRatioFrom);
	luaEngine->registerFunction("getRankDelegateRatioTo", getRankDelegateRatioTo);
	luaEngine->registerFunction("isHighestRank", isHighestRank);
	luaEngine->registerFunction("getFactionPointsCap", getFactionPointsCap);
	luaEngine->registerFunction("registerScreenPlay", registerScreenPlay);
	luaEngine->registerFunction("getZoneByName", getZoneByName);
	luaEngine->registerFunction("isZoneEnabled", isZoneEnabled);
	luaEngine->registerFunction("getZoneMaxX", getZoneMaxX);
	luaEngine->registerFunction("getZoneMaxY", getZoneMaxY);
	luaEngine->registerFunction("getContainerObjectByTemplate", getContainerObjectByTemplate);
	luaEngine->registerFunction("updateCellPermission", updateCellPermission);
	luaEngine->registerFunction("updateCellPermissionGroup", updateCellPermissionGroup);
	luaEngine->registerFunction("getQuestInfo", getQuestInfo);
	luaEngine->registerFunction("getPlayerQuestID", getPlayerQuestID);

	// call for createLoot(SceneObject* container, const String& lootGroup, int level)
	luaEngine->registerFunction("createLoot", createLoot);
	luaEngine->registerFunction("createLootSet", createLootSet);
	luaEngine->registerFunction("createLootFromCollection", createLootFromCollection);
	luaEngine->registerFunction("givePlayerResource", givePlayerResource);

	luaEngine->registerFunction("getRegion", getRegion);
	luaEngine->registerFunction("writeScreenPlayData", writeScreenPlayData);
	luaEngine->registerFunction("readScreenPlayData", readScreenPlayData);
	luaEngine->registerFunction("deleteScreenPlayData", deleteScreenPlayData);
	luaEngine->registerFunction("clearScreenPlayData", clearScreenPlayData);
	luaEngine->registerFunction("getObjectTemplatePathByCRC", getObjectTemplatePathByCRC);
	luaEngine->registerFunction("getTimestamp", getTimestamp);
	luaEngine->registerFunction("getTimestampMilli", getTimestampMilli);
	luaEngine->registerFunction("getFormattedTime", getFormattedTime);
	luaEngine->registerFunction("getSpawnPoint", getSpawnPoint);
	luaEngine->registerFunction("getDistanceToLand", getDistanceToLand);
	luaEngine->registerFunction("getRandomZonePoint", getRandomZonePoint);
	luaEngine->registerFunction("getSpawnPointInArea", getSpawnPointInArea);
	luaEngine->registerFunction("getSpawnArea", getSpawnArea);
	luaEngine->registerFunction("makeCreatureName", makeCreatureName);
	luaEngine->registerFunction("getGCWDiscount", getGCWDiscount);
	luaEngine->registerFunction("getTerrainHeight", getTerrainHeight);
	luaEngine->registerFunction("awardSkill", awardSkill);
	luaEngine->registerFunction("getCityRegionAt", getCityRegionAt);
	luaEngine->registerFunction("setDungeonTicketAttributes", setDungeonTicketAttributes);
	luaEngine->registerFunction("setQuestStatus", setQuestStatus);
	luaEngine->registerFunction("getQuestStatus", getQuestStatus);
	luaEngine->registerFunction("removeQuestStatus", removeQuestStatus);
	luaEngine->registerFunction("setCoaWinningFaction", setCoaWinningFaction);
	luaEngine->registerFunction("getControllingFaction", getControllingFaction);
	luaEngine->registerFunction("getImperialScore", getImperialScore);
	luaEngine->registerFunction("getRebelScore", getRebelScore);
	luaEngine->registerFunction("getWinningFactionDifficultyScaling", getWinningFactionDifficultyScaling);
	luaEngine->registerFunction("playClientEffectLoc", playClientEffectLoc);
	luaEngine->registerFunction("getQuestVectorMap", getQuestVectorMap);
	luaEngine->registerFunction("createQuestVectorMap", createQuestVectorMap);
	luaEngine->registerFunction("removeQuestVectorMap", removeQuestVectorMap);
	luaEngine->registerFunction("creatureTemplateExists", creatureTemplateExists);
	luaEngine->registerFunction("printLuaError", printLuaError);
	luaEngine->registerFunction("logLua", logLua);
	luaEngine->registerFunction("logLuaEvent", logLuaEvent);
	luaEngine->registerFunction("getPlayerByName", getPlayerByName);
	luaEngine->registerFunction("sendMail", sendMail);
	luaEngine->registerFunction("sendMailToOnlinePlayers", sendMailToOnlinePlayers);
	luaEngine->registerFunction("spawnTheaterObject", spawnTheaterObject);
	luaEngine->registerFunction("getSchematicItemName", getSchematicItemName);
	luaEngine->registerFunction("getBadgeListByType", getBadgeListByType);
	luaEngine->registerFunction("getGalaxyName", getGalaxyName);
	luaEngine->registerFunction("getQuestTasks", getQuestTasks);
	luaEngine->registerFunction("broadcastToGalaxy", broadcastToGalaxy);
	luaEngine->registerFunction("getWorldFloor", getWorldFloor);
	luaEngine->registerFunction("useCovertOvert", useCovertOvert);
	luaEngine->registerFunction("drawClientPath", drawClientPath);

	// JTL
	luaEngine->registerFunction("generateShipDeed", generateShipDeed);
	luaEngine->registerFunction("sellSpaceLoot", sellSpaceLoot);
	luaEngine->registerFunction("isJtlEnabled", isJtlEnabled);
	luaEngine->registerFunction("grantStarterShip", grantStarterShip);

	// Infinity:  Custom Functions
	luaEngine->registerFunction("spawnActiveAreaRect", spawnActiveAreaRect);
	luaEngine->registerFunction("printLuaLog", printLuaLog);
	luaEngine->registerFunction("spawnMobileAsBaby", spawnMobileAsBaby);
    luaEngine->registerFunction("getGalacticTime", getGalacticTime);
    luaEngine->registerFunction("getZoneTime", getZoneTime);
	luaEngine->registerFunction("withCommas", withCommas);
	luaEngine->registerFunction("broadcastToPvp", broadcastToPvp);
	luaEngine->registerFunction("logCore3", logCore3);
	luaEngine->registerFunction("getMasterScreenPlayVersion", getMasterScreenPlayVersion);
	luaEngine->registerFunction("getPlayersInRangeOfPos", getPlayersInRangeOfPos);
	luaEngine->registerFunction("getCreaturesInRangeOfPos", getCreaturesInRangeOfPos);
    luaEngine->registerFunction("getQuestStatuses", getQuestStatuses);
    luaEngine->registerFunction("isValidName", isValidName);
	// Infinity: Custom Code
	luaEngine->registerFunction("addCreatureToPlayerGroup", addCreatureToPlayerGroup);
	luaEngine->registerFunction("getSpawnPointInCell", getSpawnPointInCell);

	// Death Watch Manager bindings
	luaEngine->registerFunction("setDwWarState", setDwWarState);
	luaEngine->registerFunction("getDwWarState", getDwWarState);
	luaEngine->registerFunction("areDwFactionsAtWar", areDwFactionsAtWar);

	// Death Watch Kill Switch
	luaEngine->registerFunction("setDwKillSwitch", setDwKillSwitch);
	luaEngine->registerFunction("isDwKillSwitchActive", isDwKillSwitchActive);

	// Death Watch Jetpack Invulnerability
	luaEngine->registerFunction("setDwJetpackInvulnerable", setDwJetpackInvulnerable);
	luaEngine->registerFunction("isDwJetpackInvulnerable", isDwJetpackInvulnerable);
	luaEngine->registerFunction("removeDwJetpackInvulnerability", removeDwJetpackInvulnerability);

	// Death Watch Mk4 Ground Striker
	luaEngine->registerFunction("processDwMk4Hit", processDwMk4Hit);

	// Death Watch Missile Hit (Mk1/Mk2)
	luaEngine->registerFunction("processDwMissileHit", processDwMissileHit);

	// Death Watch damage calculations
	luaEngine->registerFunction("getDwMitigatedDamage", getDwMitigatedDamage);
	luaEngine->registerFunction("applyDwAcidDot", applyDwAcidDot);
	luaEngine->registerFunction("applyDwAcidCorrosion", applyDwAcidCorrosion);
	luaEngine->registerFunction("clearDwCombatSerumDebuffs", clearDwCombatSerumDebuffs);

	// Death Watch hidden template bonuses
	luaEngine->registerFunction("getDwHiddenDamageReduction", getDwHiddenDamageReduction);
	luaEngine->registerFunction("getDwHiddenEvasionChance", getDwHiddenEvasionChance);
	luaEngine->registerFunction("getDwHiddenSpecialEffect", getDwHiddenSpecialEffect);

	// Death Watch Stealth Protocol bindings
	luaEngine->registerFunction("activateShadowProtocolCpp", activateShadowProtocolCpp);
	luaEngine->registerFunction("deactivateShadowProtocolCpp", deactivateShadowProtocolCpp);
	luaEngine->registerFunction("isShadowProtocolActiveCpp", isShadowProtocolActiveCpp);

	//Navigation Mesh Management
	luaEngine->registerFunction("createNavMesh", createNavMesh);
	luaEngine->registerFunction("destroyNavMesh", destroyNavMesh);

	luaEngine->setGlobalInt("POSITIONCHANGED", ObserverEventType::POSITIONCHANGED);
	luaEngine->setGlobalInt("CLOSECONTAINER", ObserverEventType::CLOSECONTAINER);
	luaEngine->setGlobalInt("OBJECTDESTRUCTION", ObserverEventType::OBJECTDESTRUCTION);
	luaEngine->setGlobalInt("OBJECTDISABLED", ObserverEventType::OBJECTDISABLED);
	luaEngine->setGlobalInt("SAMPLE", ObserverEventType::SAMPLE);
	luaEngine->setGlobalInt("SAMPLETAKEN", ObserverEventType::SAMPLETAKEN);
	luaEngine->setGlobalInt("HARVESTEDCREATURE", ObserverEventType::HARVESTEDCREATURE);
	luaEngine->setGlobalInt("DEPLOYEDCAMP", ObserverEventType::DEPLOYEDCAMP);
	luaEngine->setGlobalInt("CONVERSE", ObserverEventType::CONVERSE);
	luaEngine->setGlobalInt("KILLEDCREATURE", ObserverEventType::KILLEDCREATURE);
	luaEngine->setGlobalInt("QUESTKILL", ObserverEventType::QUESTKILL);
	luaEngine->setGlobalInt("OBJECTREMOVEDFROMZONE", ObserverEventType::OBJECTREMOVEDFROMZONE);
	luaEngine->setGlobalInt("ENTEREDAREA", ObserverEventType::ENTEREDAREA);
	luaEngine->setGlobalInt("EXITEDAREA", ObserverEventType::EXITEDAREA);
	luaEngine->setGlobalInt("DESTINATIONREACHED", ObserverEventType::DESTINATIONREACHED);
	luaEngine->setGlobalInt("SPECIALATTACK", ObserverEventType::SPECIALATTACK);
	luaEngine->setGlobalInt("NEWBIETUTORIALZOOMCAMERA", ObserverEventType::NEWBIETUTORIALZOOMCAMERA);
	luaEngine->setGlobalInt("CHAT", ObserverEventType::CHAT);
	luaEngine->setGlobalInt("NEWBIETUTORIALHOLOCRON", ObserverEventType::NEWBIETUTORIALHOLOCRON);
	luaEngine->setGlobalInt("OBJECTINRANGEMOVED", ObserverEventType::OBJECTINRANGEMOVED);
	luaEngine->setGlobalInt("PLAYERCHANGEDTARGET", ObserverEventType::PLAYERCHANGEDTARGET);
	luaEngine->setGlobalInt("STARTCONVERSATION", ObserverEventType::STARTCONVERSATION);
	luaEngine->setGlobalInt("SELECTCONVERSATION", ObserverEventType::SELECTCONVERSATION);
	luaEngine->setGlobalInt("STOPCONVERSATION", ObserverEventType::STOPCONVERSATION);
	luaEngine->setGlobalInt("OPENCONTAINER", ObserverEventType::OPENCONTAINER);
	luaEngine->setGlobalInt("NEWBIEOPENINVENTORY", ObserverEventType::NEWBIEOPENINVENTORY);
	luaEngine->setGlobalInt("NEWBIECLOSEINVENTORY", ObserverEventType::NEWBIECLOSEINVENTORY);
	luaEngine->setGlobalInt("OBJECTRADIALUSED", ObserverEventType::OBJECTRADIALUSED);
	luaEngine->setGlobalInt("DAMAGERECEIVED", ObserverEventType::DAMAGERECEIVED);
	luaEngine->setGlobalInt("DAMAGECHECKPOINT", ObserverEventType::DAMAGECHECKPOINT);
	luaEngine->setGlobalInt("OBJECTNAMECHANGED", ObserverEventType::OBJECTNAMECHANGED);
	luaEngine->setGlobalInt("SURVEY", ObserverEventType::SURVEY);
	luaEngine->setGlobalInt("GETATTRIBUTESBATCHCOMMAND", ObserverEventType::GETATTRIBUTESBATCHCOMMAND);
	luaEngine->setGlobalInt("HEALINGRECEIVED", ObserverEventType::HEALINGRECEIVED);
	luaEngine->setGlobalInt("STARTCOMBAT", ObserverEventType::STARTCOMBAT);
	luaEngine->setGlobalInt("DEFENDERADDED", ObserverEventType::DEFENDERADDED);
	luaEngine->setGlobalInt("DEFENDERDROPPED", ObserverEventType::DEFENDERDROPPED);
	luaEngine->setGlobalInt("OBJECTRADIALOPENED", ObserverEventType::OBJECTRADIALOPENED);
	luaEngine->setGlobalInt("ENTEREDBUILDING", ObserverEventType::ENTEREDBUILDING);
	luaEngine->setGlobalInt("EXITEDBUILDING", ObserverEventType::EXITEDBUILDING);
	luaEngine->setGlobalInt("PLAYERKILLED", ObserverEventType::PLAYERKILLED);
	luaEngine->setGlobalInt("PLAYERCLONED", ObserverEventType::PLAYERCLONED);
	luaEngine->setGlobalInt("SPATIALCHATSENT", ObserverEventType::SPATIALCHATSENT);
	luaEngine->setGlobalInt("ITEMLOOTED", ObserverEventType::ITEMLOOTED);
	luaEngine->setGlobalInt("MEDPACKUSED", ObserverEventType::MEDPACKUSED);
	luaEngine->setGlobalInt("BADGEAWARDED", ObserverEventType::BADGEAWARDED);
	luaEngine->setGlobalInt("FACTIONBASEFLIPPED", ObserverEventType::FACTIONBASEFLIPPED);
	luaEngine->setGlobalInt("LOOTCREATURE", ObserverEventType::LOOTCREATURE);
	luaEngine->setGlobalInt("SCREENPLAYSTATECHANGED", ObserverEventType::SCREENPLAYSTATECHANGED);
	luaEngine->setGlobalInt("CREATUREDESPAWNED", ObserverEventType::CREATUREDESPAWNED);
	luaEngine->setGlobalInt("AIMESSAGE", ObserverEventType::AIMESSAGE);
	luaEngine->setGlobalInt("STARTENTERTAIN", ObserverEventType::STARTENTERTAIN);
	luaEngine->setGlobalInt("CHANGEENTERTAIN", ObserverEventType::CHANGEENTERTAIN);
	luaEngine->setGlobalInt("STOPENTERTAIN", ObserverEventType::STOPENTERTAIN);
	luaEngine->setGlobalInt("FLOURISH", ObserverEventType::FLOURISH);
	luaEngine->setGlobalInt("CONTAINERCONTENTSCHANGED", ObserverEventType::CONTAINERCONTENTSCHANGED);
	luaEngine->setGlobalInt("WASLISTENEDTO", ObserverEventType::WASLISTENEDTO);
	luaEngine->setGlobalInt("WASWATCHED", ObserverEventType::WASWATCHED);
	luaEngine->setGlobalInt("IMAGEDESIGNHAIR", ObserverEventType::IMAGEDESIGNHAIR);
	luaEngine->setGlobalInt("PARENTCHANGED", ObserverEventType::PARENTCHANGED);
	luaEngine->setGlobalInt("LOGGEDIN", ObserverEventType::LOGGEDIN);
	luaEngine->setGlobalInt("LOGGEDOUT", ObserverEventType::LOGGEDOUT);
	luaEngine->setGlobalInt("ZONESWITCHED", ObserverEventType::ZONESWITCHED);
	luaEngine->setGlobalInt("TUNEDCRYSTAL", ObserverEventType::TUNEDCRYSTAL);
	luaEngine->setGlobalInt("PROTOTYPECREATED", ObserverEventType::PROTOTYPECREATED);
	luaEngine->setGlobalInt("SLICED", ObserverEventType::SLICED);
	luaEngine->setGlobalInt("ABILITYUSED", ObserverEventType::ABILITYUSED);
	luaEngine->setGlobalInt("COMBATCOMMANDENQUEUED", ObserverEventType::COMBATCOMMANDENQUEUED);
	luaEngine->setGlobalInt("FACTIONCHAT", ObserverEventType::FACTIONCHAT);
	luaEngine->setGlobalInt("NOPLAYERSINRANGE", ObserverEventType::NOPLAYERSINRANGE);
	luaEngine->setGlobalInt("SPATIALCHAT", ObserverEventType::SPATIALCHAT);
	luaEngine->setGlobalInt("SHIPAGENTDESPAWNED", ObserverEventType::SHIPAGENTDESPAWNED);
	luaEngine->setGlobalInt("SQUADRON", ObserverEventType::SQUADRON);
	luaEngine->setGlobalInt("ENTEREDPOBSHIP", ObserverEventType::ENTEREDPOBSHIP);
	luaEngine->setGlobalInt("DESTROYEDSHIP", ObserverEventType::DESTROYEDSHIP);
	luaEngine->setGlobalInt("SHIPDOCKED", ObserverEventType::SHIPDOCKED);
	luaEngine->setGlobalInt("SHIPDISABLED", ObserverEventType::SHIPDISABLED);
	luaEngine->setGlobalInt("SHIPDESTROYED", ObserverEventType::SHIPDESTROYED);
	luaEngine->setGlobalInt("INSPECTEDSHIP", ObserverEventType::INSPECTEDSHIP);
	// Infinity:  Custom observer event types
	luaEngine->setGlobalInt("PVPTEFACTIVATED", ObserverEventType::PVPTEFACTIVATED);
	luaEngine->setGlobalInt("CREATURETAMED", ObserverEventType::CREATURETAMED);
	luaEngine->setGlobalInt("SKILLSURRENDERED", ObserverEventType::SKILLSURRENDERED);
	luaEngine->setGlobalInt("SKILLAWARDED", ObserverEventType::SKILLAWARDED);
    luaEngine->setGlobalInt("CREATUREKILLED", ObserverEventType::CREATUREKILLED);

	luaEngine->setGlobalInt("UPRIGHT", CreaturePosture::UPRIGHT);
	luaEngine->setGlobalInt("PRONE", CreaturePosture::PRONE);
	luaEngine->setGlobalInt("POSTURESITTING", CreaturePosture::SITTING);
	luaEngine->setGlobalInt("KNOCKEDDOWN", CreaturePosture::KNOCKEDDOWN);
	luaEngine->setGlobalInt("CROUCHED", CreaturePosture::CROUCHED);
	luaEngine->setGlobalInt("LYINGDOWN", CreaturePosture::LYINGDOWN);

	// Player Species
	luaEngine->setGlobalInt("SPECIES_HUMAN", CreatureObject::HUMAN);
	luaEngine->setGlobalInt("SPECIES_RODIAN", CreatureObject::RODIAN);
	luaEngine->setGlobalInt("SPECIES_TRANDOSHAN", CreatureObject::TRANDOSHAN);
	luaEngine->setGlobalInt("SPECIES_MONCAL", CreatureObject::MONCAL);
	luaEngine->setGlobalInt("SPECIES_WOOKIEE", CreatureObject::WOOKIEE);
	luaEngine->setGlobalInt("SPECIES_BOTHAN", CreatureObject::BOTHAN);
	luaEngine->setGlobalInt("SPECIES_TWILEK", CreatureObject::TWILEK);
	luaEngine->setGlobalInt("SPECIES_ZABRAK", CreatureObject::ZABRAK);
	luaEngine->setGlobalInt("SPECIES_ITHORIAN", CreatureObject::ITHORIAN);
	luaEngine->setGlobalInt("SPECIES_SULLUSTAN", CreatureObject::SULLUSTAN);

	//Player Arrangements
	luaEngine->setGlobalLong("RIDER", PlayerArrangement::RIDER);
	luaEngine->setGlobalLong("SHIP_PILOT", PlayerArrangement::SHIP_PILOT);
	luaEngine->setGlobalLong("SHIP_OPERATIONS", PlayerArrangement::SHIP_OPERATIONS);
	luaEngine->setGlobalLong("SHIP_GUNNER0", PlayerArrangement::SHIP_GUNNER0);
	luaEngine->setGlobalLong("SHIP_GUNNER1", PlayerArrangement::SHIP_GUNNER1);
	luaEngine->setGlobalLong("SHIP_GUNNER2", PlayerArrangement::SHIP_GUNNER2);
	luaEngine->setGlobalLong("SHIP_GUNNER3", PlayerArrangement::SHIP_GUNNER3);
	luaEngine->setGlobalLong("SHIP_GUNNER4", PlayerArrangement::SHIP_GUNNER4);
	luaEngine->setGlobalLong("SHIP_GUNNER5", PlayerArrangement::SHIP_GUNNER5);
	luaEngine->setGlobalLong("SHIP_GUNNER6", PlayerArrangement::SHIP_GUNNER6);
	luaEngine->setGlobalLong("SHIP_GUNNER7", PlayerArrangement::SHIP_GUNNER7);
	luaEngine->setGlobalLong("SHIP_PILOT_POB", PlayerArrangement::SHIP_PILOT_POB);
	luaEngine->setGlobalLong("SHIP_OPERATIONS_POB", PlayerArrangement::SHIP_OPERATIONS_POB);
	luaEngine->setGlobalLong("SHIP_GUNNER0_POB", PlayerArrangement::SHIP_GUNNER0_POB);
	luaEngine->setGlobalLong("SHIP_GUNNER1_POB", PlayerArrangement::SHIP_GUNNER1_POB);
	luaEngine->setGlobalLong("SHIP_GUNNER2_POB", PlayerArrangement::SHIP_GUNNER2_POB);
	luaEngine->setGlobalLong("SHIP_GUNNER3_POB", PlayerArrangement::SHIP_GUNNER3_POB);
	luaEngine->setGlobalLong("SHIP_GUNNER4_POB", PlayerArrangement::SHIP_GUNNER4_POB);
	luaEngine->setGlobalLong("SHIP_GUNNER5_POB", PlayerArrangement::SHIP_GUNNER5_POB);
	luaEngine->setGlobalLong("SHIP_GUNNER6_POB", PlayerArrangement::SHIP_GUNNER6_POB);
	luaEngine->setGlobalLong("SHIP_GUNNER7_POB", PlayerArrangement::SHIP_GUNNER7_POB);

	//Waypoint Colors
	luaEngine->setGlobalInt("WAYPOINT_WHITE", WaypointObject::COLOR_WHITE);
	luaEngine->setGlobalInt("WAYPOINT_BLUE", WaypointObject::COLOR_BLUE);
	luaEngine->setGlobalInt("WAYPOINT_GREEN", WaypointObject::COLOR_GREEN);
	luaEngine->setGlobalInt("WAYPOINT_ORANGE", WaypointObject::COLOR_ORANGE);
	luaEngine->setGlobalInt("WAYPOINT_YELLOW", WaypointObject::COLOR_YELLOW);
	luaEngine->setGlobalInt("WAYPOINT_PURPLE", WaypointObject::COLOR_PURPLE);
	luaEngine->setGlobalInt("WAYPOINT_WHITE2", WaypointObject::COLOR_WHITE2);
	luaEngine->setGlobalInt("WAYPOINT_SPACE", WaypointObject::COLOR_SPACE);

	//Waypoint Special Types
	luaEngine->setGlobalInt("WAYPOINTTHEMEPARK", WaypointObject::SPECIALTYPE_THEMEPARK);
	luaEngine->setGlobalInt("WAYPOINTRACETRACK", WaypointObject::SPECIALTYPE_RACETRACK);
	luaEngine->setGlobalInt("WAYPOINTTREASUREMAP", WaypointObject::SPECIALTYPE_TREASUREMAP);
	luaEngine->setGlobalInt("WAYPOINTQUESTTASK", WaypointObject::SPECIALTYPE_QUESTTASK);

	luaEngine->setGlobalInt("HEALTH", CreatureAttribute::HEALTH);
	luaEngine->setGlobalInt("CONSTITUTION", CreatureAttribute::CONSTITUTION);
	luaEngine->setGlobalInt("STAMINA", CreatureAttribute::STAMINA);
	luaEngine->setGlobalInt("ACTION", CreatureAttribute::ACTION);
	luaEngine->setGlobalInt("STRENGTH", CreatureAttribute::STRENGTH);
	luaEngine->setGlobalInt("QUICKNESS", CreatureAttribute::QUICKNESS);
	luaEngine->setGlobalInt("MIND", CreatureAttribute::MIND);
	luaEngine->setGlobalInt("FOCUS", CreatureAttribute::FOCUS);
	luaEngine->setGlobalInt("WILLPOWER", CreatureAttribute::WILLPOWER);
	luaEngine->setGlobalInt("UNKNOWN", CreatureAttribute::UNKNOWN);

	luaEngine->setGlobalInt("POISONED", CreatureState::POISONED);
	luaEngine->setGlobalInt("DISEASED", CreatureState::DISEASED);
	luaEngine->setGlobalInt("ONFIRE", CreatureState::ONFIRE);
	luaEngine->setGlobalInt("BLEEDING", CreatureState::BLEEDING);
	luaEngine->setGlobalInt("PEACE", CreatureState::PEACE);

	luaEngine->setGlobalLong("PILOTINGSHIP", CreatureState::PILOTINGSHIP);
	luaEngine->setGlobalLong("SHIPOPERATIONS", CreatureState::SHIPOPERATIONS);
	luaEngine->setGlobalLong("SHIPGUNNER", CreatureState::SHIPGUNNER);
	luaEngine->setGlobalLong("SHIPINTERIOR", CreatureState::SHIPINTERIOR);
	luaEngine->setGlobalLong("PILOTINGPOBSHIP", CreatureState::PILOTINGPOBSHIP);

	luaEngine->setGlobalInt("NONE", ObjectFlag::NONE);
	luaEngine->setGlobalInt("ATTACKABLE", ObjectFlag::ATTACKABLE);
	luaEngine->setGlobalInt("AGGRESSIVE", ObjectFlag::AGGRESSIVE);
	luaEngine->setGlobalInt("OVERT", ObjectFlag::OVERT);
	luaEngine->setGlobalInt("TEF", ObjectFlag::TEF);
	luaEngine->setGlobalInt("PLAYER", ObjectFlag::PLAYER);
	luaEngine->setGlobalInt("ENEMY", ObjectFlag::ENEMY);
	luaEngine->setGlobalInt("WILLBEDECLARED", ObjectFlag::WILLBEDECLARED);
	luaEngine->setGlobalInt("WASDECLARED", ObjectFlag::WASDECLARED);
	luaEngine->setGlobalInt("SCANNING_FOR_CONTRABAND", ObjectFlag::SCANNING_FOR_CONTRABAND);
	luaEngine->setGlobalInt("IGNORE_FACTION_STANDING", ObjectFlag::IGNORE_FACTION_STANDING);

	luaEngine->setGlobalInt("INSURED", OptionBitmask::INSURED);
	luaEngine->setGlobalInt("CONVERSABLE", OptionBitmask::CONVERSE);
	luaEngine->setGlobalInt("AIENABLED", OptionBitmask::AIENABLED);
	luaEngine->setGlobalInt("INVULNERABLE", OptionBitmask::INVULNERABLE);
	luaEngine->setGlobalInt("FACTIONAGGRO", OptionBitmask::FACTIONAGGRO);
	luaEngine->setGlobalInt("INTERESTING", OptionBitmask::INTERESTING);
	luaEngine->setGlobalInt("JTLINTERESTING", OptionBitmask::JTLINTERESTING);

	luaEngine->setGlobalInt("OPEN", ContainerPermissions::OPEN);
	luaEngine->setGlobalInt("MOVEIN", ContainerPermissions::MOVEIN);
	luaEngine->setGlobalInt("MOVEOUT", ContainerPermissions::MOVEOUT);
	luaEngine->setGlobalInt("WALKIN", ContainerPermissions::WALKIN);
	luaEngine->setGlobalInt("MOVECONTAINER", ContainerPermissions::MOVECONTAINER);

	// Transfer Error Codes
	luaEngine->setGlobalInt("TRANSFERCANADD", TransferErrorCode::SUCCESS);
	luaEngine->setGlobalInt("TRANSFERCANTADD", TransferErrorCode::CANTADD);
	luaEngine->setGlobalInt("TRANSFERCANTREMOVE", TransferErrorCode::CANTREMOVE);
	luaEngine->setGlobalInt("TRANSFERSUCCESS", 1);
	luaEngine->setGlobalInt("TRANSFERFAIL", 0);

	// NPC reaction chat types / states
	luaEngine->setGlobalInt("REACTION_HI", ReactionManager::HI);
	luaEngine->setGlobalInt("REACTION_BYE", ReactionManager::BYE);
	luaEngine->setGlobalInt("REACTION_NICE", ReactionManager::NICE);
	luaEngine->setGlobalInt("REACTION_MID", ReactionManager::MID);
	luaEngine->setGlobalInt("REACTION_MEAN", ReactionManager::MEAN);

	luaEngine->setGlobalLong("FACTIONNEUTRAL", Factions::FACTIONNEUTRAL);
	luaEngine->setGlobalLong("FACTIONIMPERIAL", Factions::FACTIONIMPERIAL);
	luaEngine->setGlobalLong("FACTIONREBEL", Factions::FACTIONREBEL);

	luaEngine->setGlobalInt("ONLEAVE", FactionStatus::ONLEAVE);
	luaEngine->setGlobalInt("COVERT", FactionStatus::COVERT);
	luaEngine->setGlobalInt("OVERT", FactionStatus::OVERT);

	// AI/creature bitmasks
	luaEngine->setGlobalInt("AI_NPC", ObjectFlag::NPC);
	luaEngine->setGlobalInt("AI_PACK", ObjectFlag::PACK);
	luaEngine->setGlobalInt("AI_HERD", ObjectFlag::HERD);
	luaEngine->setGlobalInt("AI_KILLER", ObjectFlag::KILLER);
	luaEngine->setGlobalInt("AI_STALKER", ObjectFlag::STALKER);
	luaEngine->setGlobalInt("AI_BABY", ObjectFlag::BABY);
	luaEngine->setGlobalInt("AI_LAIR", ObjectFlag::LAIR);
	luaEngine->setGlobalInt("AI_HEALER", ObjectFlag::HEALER);
	luaEngine->setGlobalInt("AI_SCOUT", ObjectFlag::SCOUT);
	luaEngine->setGlobalInt("AI_PET", ObjectFlag::PET);
	luaEngine->setGlobalInt("AI_DROID_PET", ObjectFlag::DROID_PET);
	luaEngine->setGlobalInt("AI_FACTION_PET", ObjectFlag::FACTION_PET);
	luaEngine->setGlobalInt("AI_ESCORT", ObjectFlag::ESCORT);
	luaEngine->setGlobalInt("AI_FOLLOW", ObjectFlag::FOLLOW);
	luaEngine->setGlobalInt("AI_STATIC", ObjectFlag::STATIC);
	luaEngine->setGlobalInt("AI_STATIONARY", ObjectFlag::STATIONARY);
	luaEngine->setGlobalInt("AI_NOAIAGGRO", ObjectFlag::NOAIAGGRO);
	luaEngine->setGlobalInt("AI_EVENTCONTROL", ObjectFlag::EVENTCONTROL);
	//Infinity: Custom
	luaEngine->setGlobalInt("AI_BABYONLY", ObjectFlag::BABYONLY);
	luaEngine->setGlobalInt("AI_ADULTONLY", ObjectFlag::ADULTONLY);

	// AI Movement States
	luaEngine->setGlobalInt("AI_OBLIVIOUS", AiAgent::OBLIVIOUS);
	luaEngine->setGlobalInt("AI_WATCHING", AiAgent::WATCHING);
	luaEngine->setGlobalInt("AI_STALKING", AiAgent::STALKING);
	luaEngine->setGlobalInt("AI_FOLLOWING", AiAgent::FOLLOWING);
	luaEngine->setGlobalInt("AI_PATROLLING", AiAgent::PATROLLING);
	luaEngine->setGlobalInt("AI_FLEEING", AiAgent::FLEEING);
	luaEngine->setGlobalInt("AI_LEASHING", AiAgent::LEASHING);
	luaEngine->setGlobalInt("AI_EVADING", AiAgent::EVADING);
	luaEngine->setGlobalInt("AI_PATHING_HOME", AiAgent::PATHING_HOME);
	luaEngine->setGlobalInt("AI_FOLLOW_FORMATION", AiAgent::FOLLOW_FORMATION);
	luaEngine->setGlobalInt("AI_MOVING_TO_HEAL", AiAgent::MOVING_TO_HEAL);
	luaEngine->setGlobalInt("AI_NOTIFY_ALLY", AiAgent::NOTIFY_ALLY);
	luaEngine->setGlobalInt("AI_CRACKDOWN_SCANNING", AiAgent::CRACKDOWN_SCANNING);
	luaEngine->setGlobalInt("AI_HARVESTING", AiAgent::HARVESTING);
	luaEngine->setGlobalInt("AI_RESTING", AiAgent::RESTING);
	luaEngine->setGlobalInt("AI_CONVERSING", AiAgent::CONVERSING);
	luaEngine->setGlobalInt("AI_LAIR_HEALING", AiAgent::LAIR_HEALING);

	// Ship Types
	luaEngine->setGlobalInt("SHIP", ShipManager::SHIP);
	luaEngine->setGlobalInt("FIGHTERSHIP", ShipManager::FIGHTERSHIP);
	luaEngine->setGlobalInt("POBSHIP", ShipManager::POBSHIP);
	luaEngine->setGlobalInt("SPACESTATION", ShipManager::SPACESTATION);

	// Ship Flags
	luaEngine->setGlobalInt("SHIP_AI_ESCORT", ShipFlag::ESCORT);
	luaEngine->setGlobalInt("SHIP_AI_FOLLOW", ShipFlag::FOLLOW);
	luaEngine->setGlobalInt("SHIP_AI_TURRETSHIP", ShipFlag::TURRETSHIP);
	luaEngine->setGlobalInt("SHIP_AI_GUARD_PATROL", ShipFlag::GUARD_PATROL);
	luaEngine->setGlobalInt("SHIP_AI_RANDOM_PATROL", ShipFlag::RANDOM_PATROL);
	luaEngine->setGlobalInt("SHIP_AI_FIXED_PATROL", ShipFlag::FIXED_PATROL);
	luaEngine->setGlobalInt("SHIP_AI_WAVE_ATTACK", ShipFlag::WAVE_ATTACK);
	luaEngine->setGlobalInt("SHIP_AI_DISABLED_INVULNERABLE", ShipFlag::DISABLED_INVULNERABLE);
	luaEngine->setGlobalInt("SHIP_AI_ATTACKABLE_SPACE_STATION", ShipFlag::ATTACKABLE_SPACE_STATION);
	luaEngine->setGlobalInt("SHIP_AI_SINGLE_PATROL_ROTATION", ShipFlag::SINGLE_PATROL_ROTATION);

	// Squad Formations
	luaEngine->setGlobalInt("SHIP_SQUADRON_FORM_NONE", ShipSquadronFormation::Type::NONE);
	luaEngine->setGlobalInt("SHIP_SQUADRON_FORM_LINE", ShipSquadronFormation::Type::LINE);
	luaEngine->setGlobalInt("SHIP_SQUADRON_FORM_WALL", ShipSquadronFormation::Type::WALL);
	luaEngine->setGlobalInt("SHIP_SQUADRON_FORM_WEDGE", ShipSquadronFormation::Type::WEDGE);

	luaEngine->setGlobalInt("SHIP_SPAWN_SINGLE", 1);
	luaEngine->setGlobalInt("SHIP_SPAWN_SQUADRON", 2);

	// ShipComponents
	luaEngine->setGlobalInt("SHIP_REACTOR", Components::REACTOR);
	luaEngine->setGlobalInt("SHIP_ENGINE", Components::ENGINE);
	luaEngine->setGlobalInt("SHIP_SHIELD0", Components::SHIELD0);
	luaEngine->setGlobalInt("SHIP_SHIELD1", Components::SHIELD1);
	luaEngine->setGlobalInt("SHIP_ARMOR0", Components::ARMOR0);
	luaEngine->setGlobalInt("SHIP_ARMOR1", Components::ARMOR1);
	luaEngine->setGlobalInt("SHIP_CAPACITOR", Components::CAPACITOR);
	luaEngine->setGlobalInt("SHIP_BOOSTER", Components::BOOSTER);
	luaEngine->setGlobalInt("SHIP_DROID_INTERFACE", Components::DROID_INTERFACE);
	luaEngine->setGlobalInt("SHIP_BRIDGE", Components::BRIDGE);
	luaEngine->setGlobalInt("SHIP_HANGAR", Components::HANGAR);
	luaEngine->setGlobalInt("SHIP_TARGETING_STATION", Components::TARGETING_STATION);
	luaEngine->setGlobalInt("SHIP_WEAPON_START", Components::WEAPON_START);

	// Pilot Squadrons
	luaEngine->setGlobalInt("CORSEC_SQUADRON", PlayerManager::CORSEC_SQUADRON);
	luaEngine->setGlobalInt("RSF_SQUADRON", PlayerManager::RSF_SQUADRON);
	luaEngine->setGlobalInt("SMUGGLER_SQUADRON", PlayerManager::SMUGGLER_SQUADRON);
	luaEngine->setGlobalInt("BLACK_EPSILON_SQUADRON", PlayerManager::BLACK_EPSILON_SQUADRON);
	luaEngine->setGlobalInt("STORM_SQUADRON", PlayerManager::STORM_SQUADRON);
	luaEngine->setGlobalInt("INQUISITION_SQUADRON", PlayerManager::INQUISITION_SQUADRON);
	luaEngine->setGlobalInt("CRIMSON_PHOENIX_SQUADRON", PlayerManager::CRIMSON_PHOENIX_SQUADRON);
	luaEngine->setGlobalInt("VORTEX_SQUADRON", PlayerManager::VORTEX_SQUADRON);
	luaEngine->setGlobalInt("HAVOC_SQUADRON", PlayerManager::HAVOC_SQUADRON);

	// Infinity:  Buff CRCs

	luaEngine->setGlobalInt("JEDI_CLOAKOFPAIN", BuffCRC::JEDI_CLOAKOFPAIN);
	luaEngine->setGlobalInt("JEDI_CORTOSIS", BuffCRC::JEDI_CORTOSIS);
	luaEngine->setGlobalInt("MEDICAL_ENHANCE_HEALTH", BuffCRC::MEDICAL_ENHANCE_HEALTH);
	luaEngine->setGlobalInt("MEDICAL_ENHANCE_STRENGTH", BuffCRC::MEDICAL_ENHANCE_STRENGTH);
	luaEngine->setGlobalInt("MEDICAL_ENHANCE_CONSTITUTION", BuffCRC::MEDICAL_ENHANCE_CONSTITUTION);
	luaEngine->setGlobalInt("MEDICAL_ENHANCE_ACTION", BuffCRC::MEDICAL_ENHANCE_ACTION);
	luaEngine->setGlobalInt("MEDICAL_ENHANCE_QUICKNESS", BuffCRC::MEDICAL_ENHANCE_QUICKNESS);
	luaEngine->setGlobalInt("MEDICAL_ENHANCE_STAMINA", BuffCRC::MEDICAL_ENHANCE_STAMINA);
	luaEngine->setGlobalInt("MEDICAL_ENHANCE_POISON", BuffCRC::MEDICAL_ENHANCE_POISON);
	luaEngine->setGlobalInt("MEDICAL_ENHANCE_DISEASE", BuffCRC::MEDICAL_ENHANCE_DISEASE);
	luaEngine->setGlobalInt("PERFORMANCE_ENHANCE_DANCE_MIND", BuffCRC::PERFORMANCE_ENHANCE_DANCE_MIND);
	luaEngine->setGlobalInt("PERFORMANCE_ENHANCE_MUSIC_FOCUS", BuffCRC::PERFORMANCE_ENHANCE_MUSIC_FOCUS);
	luaEngine->setGlobalInt("PERFORMANCE_ENHANCE_MUSIC_WILLPOWER", BuffCRC::PERFORMANCE_ENHANCE_MUSIC_WILLPOWER);


	// Badges
	const auto badges = BadgeList::instance()->getMap();
	for (const auto& entry : *badges) {
		const auto badge = entry.getValue();

		const String val = badge->getKey().toUpperCase();

		luaEngine->setGlobalInt(val, badge->getIndex());
	}

	// SUI Window Types (WIP)
	luaEngine->setGlobalInt("NEWSNET_INFO", SuiWindowType::NEWSNET_INFO);

	Luna<LuaCellObject>::Register(luaEngine->getLuaState());
	Luna<LuaBuildingObject>::Register(luaEngine->getLuaState());
	Luna<LuaCreatureObject>::Register(luaEngine->getLuaState());
	Luna<LuaShipObject>::Register(luaEngine->getLuaState());
	Luna<LuaPobShipObject>::Register(luaEngine->getLuaState());
	Luna<LuaShipAiAgent>::Register(luaEngine->getLuaState());
	Luna<LuaShipComponent>::Register(luaEngine->getLuaState());
	Luna<LuaSceneObject>::Register(luaEngine->getLuaState());
	Luna<LuaConversationScreen>::Register(luaEngine->getLuaState());
	Luna<LuaConversationSession>::Register(luaEngine->getLuaState());
	Luna<LuaConversationTemplate>::Register(luaEngine->getLuaState());
	Luna<LuaIntangibleObject>::Register(luaEngine->getLuaState());
	Luna<LuaMissionObject>::Register(luaEngine->getLuaState());
	Luna<LuaControlDevice>::Register(luaEngine->getLuaState());
	Luna<LuaPlayerObject>::Register(luaEngine->getLuaState());
	Luna<LuaAiAgent>::Register(luaEngine->getLuaState());
	Luna<LuaActiveArea>::Register(luaEngine->getLuaState());
	Luna<LuaTangibleObject>::Register(luaEngine->getLuaState());
	Luna<LuaSuiManager>::Register(luaEngine->getLuaState());
	Luna<LuaSuiBox>::Register(luaEngine->getLuaState());
	Luna<LuaObjectMenuResponse>::Register(luaEngine->getLuaState());
	Luna<LuaDeed>::Register(luaEngine->getLuaState());
	Luna<LuaCityRegion>::Register(luaEngine->getLuaState());
	Luna<LuaStringIdChatParameter>::Register(luaEngine->getLuaState());
	Luna<LuaTicketObject>::Register(luaEngine->getLuaState());
	Luna<LuaQuestInfo>::Register(luaEngine->getLuaState());
	Luna<LuaQuestTasks>::Register(luaEngine->getLuaState());
	Luna<LuaQuestTask>::Register(luaEngine->getLuaState());
	Luna<LuaFsPuzzlePack>::Register(luaEngine->getLuaState());
	Luna<LuaFsCsObject>::Register(luaEngine->getLuaState());
	Luna<LuaFsBuffItem>::Register(luaEngine->getLuaState());
	Luna<LuaResourceSpawn>::Register(luaEngine->getLuaState());
	Luna<LuaCustomIngredient>::Register(luaEngine->getLuaState());
	Luna<LuaFsCraftingComponentObject>::Register(luaEngine->getLuaState());
	Luna<LuaSuiPageData>::Register(luaEngine->getLuaState());
	Luna<LuaQuestVectorMap>::Register(luaEngine->getLuaState());
	Luna<LuaSuiBoxPage>::Register(luaEngine->getLuaState());
	Luna<LuaPowerupObject>::Register(luaEngine->getLuaState());
	Luna<LuaWaypointObject>::Register(luaEngine->getLuaState());
	Luna<LuaComponent>::Register(luaEngine->getLuaState());
	Luna<LuaSuiListBox>::Register(luaEngine->getLuaState());
	Luna<LuaLightsaberCrystalComponent>::Register(luaEngine->getLuaState());
	Luna<LuaSkill>::Register(luaEngine->getLuaState());
	Luna<LuaSkillManager>::Register(luaEngine->getLuaState());
	Luna<LuaContractCrate>::Register(luaEngine->getLuaState());
	Luna<LuaScreenPlayObserver>::Register(luaEngine->getLuaState());
	Luna<LuaSpawnEggObject>::Register(luaEngine->getLuaState());
}

int DirectorManager::loadScreenPlays(Lua* luaEngine) {
	Timer loadTimer;
	loadTimer.start();

	bool res = luaEngine->runFile("scripts/screenplays/screenplays.lua");

	if (!DEBUG_MODE) {
		auto elapsed = loadTimer.stopMs();

		info() << Thread::getCurrentThread()->getName()
			<< " loaded "
			<< instance()->screenPlays.size()
			<< " screenplays in "
			<< elapsed
			<< " ms.";
	}

	if (!res)
		return 1;

	return 0;
}

void DirectorManager::reloadScreenPlays() {
	masterScreenPlayVersion.increment();
}

int DirectorManager::writeScreenPlayData(lua_State* L) {
	if (checkArgumentCount(L, 4) != 0) {
		printTraceError(L, "incorrect number of arguments passed to DirectorManager::writeScreenPlayData");
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String data = lua_tostring(L, -1);
	String variable = lua_tostring(L, -2);
	String screenPlay = lua_tostring(L, -3);
	SceneObject* player = (SceneObject*) lua_touserdata(L, -4);

	if (player == nullptr || !player->isPlayerCreature()) {
		String err = "Attempted to write screen play data to a non-player Scene Object using screenplay " + screenPlay + " and variable " + variable;
		printTraceError(L, err);
		return 0;
	}

	Reference<PlayerObject*> ghost = player->getSlottedObject("ghost").castTo<PlayerObject*>();

	if (ghost == nullptr) {
		String err = "Attempted to write screen play data for a null ghost using screenplay " + screenPlay + " and variable " + variable;
		printTraceError(L, err);
		return 0;
	}

	ghost->setScreenPlayData(screenPlay, variable, data);

	return 0;
}

int DirectorManager::createLoot(lua_State* L) {

	int numberOfArguments = lua_gettop(L);
	if (numberOfArguments != 4 && numberOfArguments != 5) {
		String err = "incorrect number of arguments passed to DirectorManager::createLoot";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

    SceneObject* container = nullptr;
    String lootGroup;
    int level = 0;
    bool maxCondition = false;
    float legoChanceMultiplier = 1.0f;

    if (numberOfArguments == 4) {
        container = (SceneObject*)lua_touserdata(L, -4);
        lootGroup = lua_tostring(L, -3);
        level = lua_tonumber(L, -2);
        maxCondition = lua_toboolean(L, -1);
    } else {
        container = (SceneObject*)lua_touserdata(L, -5);
        lootGroup = lua_tostring(L, -4);
        level = lua_tonumber(L, -3);
        maxCondition = lua_toboolean(L, -2);
        legoChanceMultiplier = lua_tonumber(L, -1);
        if (legoChanceMultiplier <= 0) {
            printTraceError(L, "DirectorManager::createLoot - legoChanceMultiplier was less than or equal to 0, defaulting to 1.0f.");
            legoChanceMultiplier = 1.0f;
        }
    }

	if (container == nullptr) {
		printTraceError(L, "Failed to generate loot in DirectorManager::createLoot, null inventory.");
		return 0;
	}

	if (lootGroup == "") {
		printTraceError(L, "Failed to generate loot in DirectorManager::createLoot, empty loot group.");
		return 0;
	}

    auto zoneServer = ServerCore::getZoneServer();

    if (zoneServer == nullptr)  {
        printTraceError(L, "DirectorManager::createLoot - could not locate zoneServer");
		return 0;
    }

	auto lootManager = zoneServer->getLootManager();

    if (lootManager == nullptr)  {
        printTraceError(L, "DirectorManager::createLoot - could not locate lootManager");
		return 0;
    }

	auto dst = container;

	if (!container->isCreatureObject()) {
		auto parent = container->getParentRecursively(SceneObjectType::PLAYERCREATURE);

		if (parent != nullptr) {
			dst = parent;
		}
	}

	TransactionLog trx(TrxCode::LUALOOT, dst);
	trx.addContextFromLua(L);
	trx.addState("dstContainer", container->getObjectID());

	uint64 lootObjectID = lootManager->createLoot(trx, container, lootGroup, level, maxCondition, legoChanceMultiplier);

	if (lootObjectID > 0) {
		trx.commit(true);
	} else {
		trx.abort() << __FUNCTION__ << " failed: lootGroup=" << lootGroup << "; level=" << level << "; maxCondition=" << maxCondition << " legoChanceMultiplier=" << legoChanceMultiplier;
	}

	lua_pushinteger(L, lootObjectID);

	return 1;
}

int DirectorManager::createLootSet(lua_State* L) {
	if (checkArgumentCount(L, 5) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::createLootSet";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	SceneObject* container = (SceneObject*)lua_touserdata(L, -5);
	String lootGroup = lua_tostring(L, -4);
	int level = lua_tonumber(L, -3);
	bool maxCondition = lua_toboolean(L, -2);
	int setSize = lua_tonumber(L, -1);

	if (container == nullptr || lootGroup == "") {
		return 0;
    }

    auto zoneServer = ServerCore::getZoneServer();

    if (zoneServer == nullptr)  {
        printTraceError(L, "DirectorManager::createLootSet - could not locate zoneServer");
		return 0;
    }

	auto lootManager = zoneServer->getLootManager();

    if (lootManager == nullptr)  {
        printTraceError(L, "DirectorManager::createLootSet - could not locate lootrManager");
		return 0;
    }

	auto dst = container;

	if (!container->isCreatureObject()) {
		auto parent = container->getParentRecursively(SceneObjectType::PLAYERCREATURE);

		if (parent != nullptr) {
			dst = parent;
		}
	}

	TransactionLog trx(TrxCode::LUALOOT, dst);
	trx.addContextFromLua(L);
	trx.addState("dstContainer", container->getObjectID());
	if (lootManager->createLootSet(trx, container, lootGroup, level, maxCondition, setSize)) {
		trx.commit(true);
	} else {
		trx.abort() << __FUNCTION__ << " failed: lootGroup=" << lootGroup << "; level=" << level << "; maxCondition=" << maxCondition << "; setSize=" << setSize;
	}

	return 0;
}

int DirectorManager::createLootFromCollection(lua_State* L) {
    int numberOfArguments = lua_gettop(L);
	if (numberOfArguments != 3 && numberOfArguments != 4) {
		String err = "incorrect number of arguments passed to DirectorManager::createLootFromCollection";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

    SceneObject* container = nullptr;
    LootGroupCollection lootCollection;
    int level = 0;
    float legoChanceMultiplier = 1.0f;

    if (numberOfArguments == 3) {
        container = (SceneObject*)lua_touserdata(L, -3);

        if (container == nullptr) {
            printTraceError(L, "Failed to generate loot in DirectorManager::createLootFromCollection, null inventory.");
            return 0;
        }

        level = lua_tonumber(L, -1);
        lua_pop(L, 1);

        LuaObject luaObject(L);
        lootCollection.readObject(&luaObject, level);
	    luaObject.pop();
    } else {
        container = (SceneObject*)lua_touserdata(L, -4);

        if (container == nullptr) {
            printTraceError(L, "Failed to generate loot in DirectorManager::createLootFromCollection, null inventory.");
            return 0;
        }

        level = lua_tonumber(L, -2);

        legoChanceMultiplier = lua_tonumber(L, -1);
        if (legoChanceMultiplier <= 0) {
            printTraceError(L, "DirectorManager::createLoot - legoChanceMultiplier was less than or equal to 0, defaulting to 1.0f.");
            legoChanceMultiplier = 1.0f;
        }

        lua_pop(L, 2);

        LuaObject luaObject(L);
        lootCollection.readObject(&luaObject, level);
	    luaObject.pop();
    }

    auto zoneServer = ServerCore::getZoneServer();

    if (zoneServer == nullptr)  {
        printTraceError(L, "DirectorManager::createLootFromCollection - could not locate zoneServer");
		return 0;
    }

	auto lootManager = zoneServer->getLootManager();

    if (lootManager == nullptr)  {
        printTraceError(L, "DirectorManager::createLootFromCollection - could not locate lootManager");
		return 0;
    }

	auto dst = container;

	if (!container->isCreatureObject()) {
		auto parent = container->getParentRecursively(SceneObjectType::PLAYERCREATURE);

		if (parent != nullptr) {
			dst = parent;
		}
	}

	TransactionLog trx(TrxCode::LUALOOT, dst);
	trx.addContextFromLua(L);
	trx.addState("dstContainer", container->getObjectID());
	if (lootManager->createLootFromCollection(trx, container, &lootCollection, level, legoChanceMultiplier)) {
		trx.commit(true);
	} else {
		trx.abort() << __FUNCTION__ << " failed: level=" << level;
	}

	return 0;
}

int DirectorManager::givePlayerResource(lua_State* L) {
	if (checkArgumentCount(L, 3) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::givePlayerResource";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	const int quantity = lua_tointeger(L, -1);
	const String typeString = lua_tostring(L, -2);
	CreatureObject* player = (CreatureObject*)lua_touserdata(L, -3);

	if (player == nullptr)
		return 0;

	ZoneServer* zoneServer = ServerCore::getZoneServer();
	Zone* zone = player->getZone();

	if (zoneServer == nullptr || zone == nullptr)
		return 0;

	ResourceManager* resourceManager = zoneServer->getResourceManager();

	if (resourceManager == nullptr)
		return 0;

	int type = 0;

	// TODO: Global resource types
	// SOLAR = 1; CHEMICAL = 2; FLORA = 3; GAS = 4; GEOTHERMAL = 5; MINERAL = 6; WATER = 7; WIND = 8; FUSION = 9;
	if (typeString == "organic")
		type = -1;
	else if (typeString == "solar")
		type = 1;
	else if (typeString == "chemical")
		type = 2;
	else if (typeString == "flora")
		type = 3;
	else if (typeString == "gas")
		type = 4;
	else if (typeString == "geothermal")
		type = 5;
	else if (typeString == "mineral")
		type = 6;
	else if (typeString == "water")
		type = 7;
	else if (typeString == "wind")
		type = 8;
	else if (typeString == "fusion")
		type = 9;
	else if (typeString == "inorganic")
		type = 10;

	Vector<ManagedReference<ResourceSpawn*> > resources;
	resourceManager->getResourceListByType(resources, type, zone->getZoneName());

	if (resources.size() < 1)
		return 0;

	ManagedReference<ResourceSpawn*> spawn = resources.get(System::random(resources.size() - 1));

	if (spawn == nullptr) {
		return 0;
	}

	TransactionLog trx(TrxCode::LUASCRIPT, player);
	trx.addContextFromLua(L);

	resourceManager->harvestResourceToPlayer(trx, player, spawn, quantity);
	trx.commit();

	return 0;
}

int DirectorManager::getTimestamp(lua_State* L) {
	Time now;
	lua_pushinteger(L, now.getTime());

	return 1;
}

int DirectorManager::getTimestampMilli(lua_State* L) {
	Time now;
	lua_pushinteger(L, now.getMiliTime());

	return 1;
}

int DirectorManager::getFormattedTime(lua_State* L) {
	Time now;
	lua_pushstring(L, now.getFormattedTime().toCharArray());

	return 1;
}

int DirectorManager::readScreenPlayData(lua_State* L) {
	if (checkArgumentCount(L, 3) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::readScreenPlayData";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String variable = lua_tostring(L, -1);
	String screenPlay = lua_tostring(L, -2);
	SceneObject* player = (SceneObject*) lua_touserdata(L, -3);

	if (player == nullptr || !player->isPlayerCreature()) {
		String err = "Attempted to read screen play data from a non-player Scene Object using screenplay " + screenPlay + " and variable " + variable;
		printTraceError(L, err);

		lua_pushstring(L, "");

		return 1;
	}

	Reference<PlayerObject*> ghost = player->getSlottedObject("ghost").castTo<PlayerObject*>();

	if (ghost == nullptr) {
		String err = "Attempted to read screen play data for a null ghost using screenplay " + screenPlay + " and variable " + variable;
		printTraceError(L, err);

		lua_pushstring(L, "");

		return 1;
	}

	//readScreenPlayData(player, screenPlay, variable)

	lua_pushstring(L, ghost->getScreenPlayData(screenPlay, variable).toCharArray());

	return 1;
}

int DirectorManager::deleteScreenPlayData(lua_State* L) {
	if (checkArgumentCount(L, 3) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::deleteScreenPlayData";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String variable = lua_tostring(L, -1);
	String screenPlay = lua_tostring(L, -2);
	SceneObject* player = (SceneObject*) lua_touserdata(L, -3);

	if (player == nullptr || !player->isPlayerCreature()) {
		String err = "Attempted to delete screen play data for a non-player Scene Object using screenplay " + screenPlay + " and variable " + variable;
		printTraceError(L, err);
		return 0;
	}

	Reference<PlayerObject*> ghost = player->getSlottedObject("ghost").castTo<PlayerObject*>();

	if (ghost == nullptr) {
		String err = "Attempted to delete screen play data for a null ghost using screenplay " + screenPlay + " and variable " + variable;
		printTraceError(L, err);
		return 0;
	}

	ghost->deleteScreenPlayData(screenPlay, variable);

	return 0;
}

int DirectorManager::clearScreenPlayData(lua_State* L) {
	if (checkArgumentCount(L, 2) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::clearScreenPlayData";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String screenPlay = lua_tostring(L, -1);
	SceneObject* player = (SceneObject*) lua_touserdata(L, -2);

	if (player == nullptr || !player->isPlayerCreature()) {
		String err = "Attempted to clear screen play data for a non-player Scene Object using screenplay " + screenPlay;
		printTraceError(L, err);
		return 0;
	}

	Reference<PlayerObject*> ghost = player->getSlottedObject("ghost").castTo<PlayerObject*>();

	if (ghost == nullptr) {
		String err = "Attempted to clear screen play data for a null ghost using screenplay " + screenPlay;
		printTraceError(L, err);
		return 0;
	}

	ghost->clearScreenPlayData(screenPlay);

	return 0;
}

int DirectorManager::registerScreenPlay(lua_State* L) {
	if (checkArgumentCount(L, 2) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::registerScreenPlay";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String name = lua_tostring(L, -2);
	bool start = lua_toboolean(L, -1);

	instance()->wlock();

	instance()->screenPlays.put(name, start);

	instance()->unlock();

	return 0;
}

int DirectorManager::checkInt64Lua(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::checkInt64Lua";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	instance()->info(true) << "Lua version: " << *lua_version(L);

	uint64 data = lua_tointeger(L, -1);

	uint64 bigNumber = 0x01000000;
	bigNumber += bigNumber << 32;

	instance()->fatal(data >= bigNumber, "Lua not using lnum patch with 64 bit integers, please patch lua!!");

	lua_pop(L, 1);

	return 0;
}

int DirectorManager::includeFile(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::includeFile";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String filename = Lua::getStringParameter(L);

	int oldError = ERROR_CODE;

	bool ret = Lua::runFile("scripts/screenplays/" + filename, L);

	if (!ret) {
		ERROR_CODE = GENERAL_ERROR;

		DirectorManager::instance()->error("running file: scripts/screenplays/" + filename);
	} else if (!oldError && ERROR_CODE) {
		DirectorManager::instance()->error("running file: scripts/screenplays/" + filename);
	}

	return 0;
}

int DirectorManager::readSharedMemory(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::readSharedMemory";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String key = Lua::getStringParameter(L);

	uint64 data = instance()->readSharedMemory(key);

	lua_pushinteger(L, data);

	return 1;
}

int DirectorManager::deleteSharedMemory(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::deleteSharedMemory";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String key = Lua::getStringParameter(L);

#ifndef WITH_STM
	DirectorManager::instance()->wlock();
#endif

	DirectorManager::instance()->sharedMemory->remove(key);

#ifndef WITH_STM
	DirectorManager::instance()->unlock();
#endif

	return 0;
}

int DirectorManager::writeSharedMemory(lua_State* L) {
	if (checkArgumentCount(L, 2) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::writeSharedMemory";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String key = lua_tostring(L, -2);
	uint64 data = lua_tointeger(L, -1);

#ifndef WITH_STM
	DirectorManager::instance()->wlock();
#endif

	DirectorManager::instance()->sharedMemory->put(key, data);

#ifndef WITH_STM
	DirectorManager::instance()->unlock();
#endif

	return 0;
}

int DirectorManager::readStringSharedMemory(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::readStringSharedMemory";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String key = Lua::getStringParameter(L);

	String data = instance()->readStringSharedMemory(key);

	lua_pushstring(L, data.toCharArray());

	return 1;
}

int DirectorManager::deleteStringSharedMemory(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::deleteStringSharedMemory";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String key = Lua::getStringParameter(L);

#ifndef WITH_STM
	DirectorManager::instance()->wlock();
#endif

	DirectorManager::instance()->sharedMemory->removeString(key);

#ifndef WITH_STM
	DirectorManager::instance()->unlock();
#endif

	return 0;
}

int DirectorManager::writeStringSharedMemory(lua_State* L) {
	if (checkArgumentCount(L, 2) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::writeStringSharedMemory";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String key = lua_tostring(L, -2);
	String data = lua_tostring(L, -1);

#ifndef WITH_STM
	DirectorManager::instance()->wlock();
#endif

	DirectorManager::instance()->sharedMemory->putString(key, data);

#ifndef WITH_STM
	DirectorManager::instance()->unlock();
#endif

	return 0;
}

int DirectorManager::readVector3SharedMemory(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::readVector3SharedMemory";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String key = Lua::getStringParameter(L);

	Vector3 data = instance()->readVector3SharedMemory(key);

	lua_newtable(L);
	lua_pushnumber(L, data.getX());
	lua_pushnumber(L, data.getZ());
	lua_pushnumber(L, data.getY());
	lua_rawseti(L, -4, 3);
	lua_rawseti(L, -3, 2);
	lua_rawseti(L, -2, 1);

	return 1;
}

int DirectorManager::deleteVector3SharedMemory(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::deleteVector3SharedMemory";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String key = Lua::getStringParameter(L);

#ifndef WITH_STM
	DirectorManager::instance()->wlock();
#endif

	DirectorManager::instance()->sharedMemory->removeVector3(key);

#ifndef WITH_STM
	DirectorManager::instance()->unlock();
#endif

	return 0;
}

int DirectorManager::writeVector3SharedMemory(lua_State* L) {
	if (checkArgumentCount(L, 4) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::writeVector3SharedMemory";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String key = lua_tostring(L, -4);
	float x = lua_tonumber(L, -3);
	float z = lua_tonumber(L, -2);
	float y = lua_tonumber(L, -1);

	Vector3 newVector = Vector3(x, y, z);

#ifndef WITH_STM
	DirectorManager::instance()->wlock();
#endif

	DirectorManager::instance()->sharedMemory->putVector3(key, newVector);

#ifndef WITH_STM
	DirectorManager::instance()->unlock();
#endif

	return 0;
}

int DirectorManager::readStringVectorSharedMemory(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::readStringVectorSharedMemory";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String key = Lua::getStringParameter(L);

	Vector<String> table = instance()->readStringVectorSharedMemory(key);
	int tableSize = table.size();

	lua_newtable(L);

	for (int i = tableSize; i > 0; --i) {
		const String str = table.get(i - 1);

		lua_pushstring(L, str.toCharArray());
	}

	for (int j = tableSize; j > 0; --j) {
		int rawSet = (j + 1) * -1;

		lua_rawseti(L, rawSet, j);
	}

	return 1;
}

int DirectorManager::deleteStringVectorSharedMemory(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::deleteStringVectorSharedMemory";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String key = Lua::getStringParameter(L);

#ifndef WITH_STM
	DirectorManager::instance()->wlock();
#endif

	DirectorManager::instance()->sharedMemory->removeStringVector(key);

#ifndef WITH_STM
	DirectorManager::instance()->unlock();
#endif

	return 0;
}

int DirectorManager::writeStringVectorSharedMemory(lua_State* L) {
	if (checkArgumentCount(L, 2) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::writeStringVectorSharedMemory";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String key = lua_tostring(L, -2);
	lua_settop(L, -1);

	LuaObject table(L);
	int tableSize = table.getTableSize();

	Vector<String> strings;

	for (int i = 1; i <= tableSize; ++i) {
		String entry = table.getStringAt(i);

		strings.add(entry);
	}

	table.pop();

#ifndef WITH_STM
	DirectorManager::instance()->wlock();
#endif

	DirectorManager::instance()->sharedMemory->putStringVector(key, strings);

#ifndef WITH_STM
	DirectorManager::instance()->unlock();
#endif

	return 0;
}

int DirectorManager::createEvent(lua_State* L) {
	int numberOfArguments = lua_gettop(L);
	if (numberOfArguments != 5 && numberOfArguments != 6) {
		String err = "incorrect number of arguments passed to DirectorManager::createEvent";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String args = lua_tostring(L, -1);
	SceneObject* obj = (SceneObject*) lua_touserdata(L, -2);
	String key = lua_tostring(L, -3);
	String play = lua_tostring(L, -4);
	uint32 mili = lua_tonumber(L, -5);

	//System::out << "scheduling task with mili:" << mili << endl;

	Reference<ScreenPlayTask*> task = new ScreenPlayTask(obj, key, play, args);
	DirectorManager::instance()->screenplayTasks.put(task);

	if (numberOfArguments == 6) {
		bool save = lua_toboolean(L, -6);

		if (save && obj != nullptr) {
			Time expireTime;
			uint64 currentTime = expireTime.getMiliTime();

			Reference<PersistentEvent*> pevent = new PersistentEvent();
			pevent->setObject(obj);
			pevent->setKey(key);
			pevent->setScreenplay(play);
			pevent->setArgs(args);
			pevent->setMiliDiff(mili);
			pevent->setCurTime(currentTime);
			pevent->setScreenplayTask(task);

			StringBuffer eventName;
			eventName << key << ":" << play << obj->getObjectID();

			String eventStringName = eventName.toString();

			pevent->setEventName(eventStringName);

			ObjectManager::instance()->persistObject(pevent, 1, "events");

			task->setPersistentEvent(pevent.get());

			if (persistentEvents.put(eventStringName.hashCode(), pevent) != nullptr) {
				String err = "Duplicate persistent event for " + eventStringName;
				printTraceError(L, err);
			}
		}

		task->schedule(mili);
	} else {
		task->schedule(mili);
	}

	return 0;
}

int DirectorManager::cancelEvent(lua_State* L) {
	int numberOfArguments = lua_gettop(L);

	if (numberOfArguments != 3) {
		String err = "incorrect number of arguments passed to DirectorManager::cancelEvent";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	SceneObject* sceneO = (SceneObject*) lua_touserdata(L, -1);
	String screenplayFunction = lua_tostring(L, -2);
	String screenplayName = lua_tostring(L, -3);

	if (sceneO == nullptr) {
		return 0;
	}

	auto eventsList = DirectorManager::instance()->getObjectEvents(sceneO);

	for (int i = 0; i < eventsList.size(); i++) {
		Reference<ScreenPlayTask*> task = eventsList.get(i);

		if (task == nullptr || task->getSceneObject() != sceneO) {
			continue;
		}

		auto taskScreenplay = task->getScreenPlay();
		auto taskKey = task->getTaskKey();

		// instance()->info(true) << "DirectorManager::cancelEvent -- Checking Screenplay Name: " << screenplayName << " Function: " << screenplayFunction << " Object: " << sceneO->getDisplayedName();

		if (taskScreenplay != screenplayName || taskKey != screenplayFunction) {
			continue;
		}

		// Cancel the task
		task->cancel();

		// Remove it from the list
		instance()->screenplayTasks.drop(task);
	}

	return 0;
}

int DirectorManager::createEventActualTime(lua_State* L) {
	if (checkArgumentCount(L, 3) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::createEventActualTime";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	SceneObject* obj = (SceneObject*) nullptr;
	String key = lua_tostring(L, -1);
	String play = lua_tostring(L, -2);
	uint32 timeInMinutes = lua_tonumber(L, -3);
	Reference<ScreenPlayTask*> task = new ScreenPlayTask(obj, key, play, "");
	Time actualTime = Time(timeInMinutes);
	Time now;
	uint64 days=now.getMiliTime()/(24*60*60000);
	uint64 dModifier = now.getMiliTime() - (days * (24*60*60000));
	uint64 interval =(24*60*60000);
	if (actualTime.getMiliTime()<= dModifier){
		interval =(24*60*60000) - (dModifier - actualTime.getMiliTime());
	}
	DirectorManager::instance()->screenplayTasks.put(task);

	task->schedule(interval);

	return 0;
}

int DirectorManager::createServerEvent(lua_State* L) {
	if (checkArgumentCount(L, 4) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::createServerEvent";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String eventName = lua_tostring(L, -1);
	String key = lua_tostring(L, -2);
	String play = lua_tostring(L, -3);
	uint64 mili = lua_tonumber(L, -4);

	Reference<PersistentEvent*> pEvent = getServerEvent(eventName);

	if (pEvent != nullptr) {
		String err = "Server event " + eventName + " already exists, exiting...";
		printTraceError(L, err);
		ERROR_CODE = GENERAL_ERROR;
		return 0;
	}

	Time expireTime;
	uint64 currentTime = expireTime.getMiliTime();

	Reference<ScreenPlayTask*> task = new ScreenPlayTask(nullptr, key, play, "");

	Reference<PersistentEvent*> pevent = new PersistentEvent();
	pevent->setMiliDiff(mili);
	pevent->setCurTime(currentTime);
	pevent->setEventName(eventName);
	pevent->setKey(key);
	pevent->setScreenplay(play);
	pevent->setScreenplayTask(task);

	if (persistentEvents.put(eventName.hashCode(), pevent) != nullptr) {
		String err = "Persistent event with " + eventName + " already exists!";
		printTraceError(L, err);
	}

	task->setPersistentEvent(pevent);

	ObjectManager::instance()->persistObject(pevent, 1, "events");

	task->schedule(mili);

	lua_pushinteger(L, pevent->_getObjectID());

	return 1;
}

int DirectorManager::hasServerEvent(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::hasServerEvent";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String eventName = lua_tostring(L, -1);

	Reference<PersistentEvent*> pEvent = getServerEvent(eventName);

	if (pEvent != nullptr)
		lua_pushboolean(L, true);
	else
		lua_pushboolean(L, false);

	return 1;
}

int DirectorManager::rescheduleServerEvent(lua_State* L) {
	if (checkArgumentCount(L, 2) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::rescheduleServerEvent";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String eventName = lua_tostring(L, -2);
	uint32 mili = lua_tonumber(L, -1);

	Reference<PersistentEvent*> pEvent = getServerEvent(eventName);

	if (pEvent == nullptr) {
		String err = "Unable to find server event " + eventName + " in DirectorManager::rescheduleServerEvent";
		printTraceError(L, err);
		return 0;
	}

	Reference<ScreenPlayTask*> task = pEvent->getScreenplayTask().get();

	if (task == nullptr) {
		String err = "Unable to find task for server event " + eventName + " in DirectorManager::rescheduleServerEvent";
		printTraceError(L, err);
		return 0;
	}

	Time curTime;
	uint64 currentTime = curTime.getMiliTime();

	pEvent->setMiliDiff(mili);
	pEvent->setCurTime(currentTime);
	task->reschedule(mili);

	return 0;
}

int DirectorManager::getServerEventTimeLeft(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::getServerEventTimeLeft";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	uint64 objectID = lua_tointeger(L, -1);

	Reference<PersistentEvent*> pEvent = Core::getObjectBroker()->lookUp(objectID).castTo<PersistentEvent*>();

	if (pEvent == nullptr)
		lua_pushnil(L);
	else {
		Time curTime;
		uint64 currentTime = curTime.getMiliTime();
		uint64 origTime = pEvent->getCurTime();
		uint64 timeStamp = pEvent->getMiliDiff();
		uint64 timeLeft = origTime + timeStamp - currentTime;

		lua_pushinteger(L, timeLeft);
	}

	return 1;
}

int DirectorManager::getServerEventID(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::getServerEventID";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String eventName = lua_tostring(L, -1);

	Reference<PersistentEvent*> pEvent = getServerEvent(eventName);

	if (pEvent == nullptr) {
		lua_pushnil(L);
	} else {
		lua_pushnumber(L, pEvent->_getObjectID());
	}

	return 1;
}

void DirectorManager::dropServerEventReference(const String& eventName) {
	persistentEvents.remove(eventName.hashCode());
}

Reference<PersistentEvent*> DirectorManager::getServerEvent(const String& eventName) {
	return persistentEvents.get(eventName.hashCode());
}

int DirectorManager::getChatMessage(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::getChatMessage";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	ChatMessage* cm = (ChatMessage*)lua_touserdata(L, -1);

	String text = "";

	if (cm != nullptr)
		text = cm->toString();

	lua_pushstring(L, text.toCharArray());

	return 1;
}

int DirectorManager::spatialChat(lua_State* L) {
	if (checkArgumentCount(L, 2) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::spatialChat";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	ManagedReference<CreatureObject*> creature = (CreatureObject*)lua_touserdata(L, -2);

	if (creature == nullptr)
		return 0;

	ZoneServer* zoneServer = ServerCore::getZoneServer();

	if (zoneServer == nullptr)
		return 0;

	ChatManager* chatManager = zoneServer->getChatManager();

	if (chatManager == nullptr)
		return 0;

	if (lua_islightuserdata(L, -1)) {
		StringIdChatParameter* message = (StringIdChatParameter*)lua_touserdata(L, -1);

		if (creature != nullptr && message != nullptr) {
			Reference<StringIdChatParameter*> param = new StringIdChatParameter(*message);

			Core::getTaskManager()->executeTask([=] () {
				Locker locker(creature);

				chatManager->broadcastChatMessage(creature, *param.get(), 0, 0, creature->getMoodID());
			}, "BroadcastChatLambda");
		}
	} else {
		String message = lua_tostring(L, -1);

		if (creature != nullptr) {
			Core::getTaskManager()->executeTask([=] () {
				Locker locker(creature);

				chatManager->broadcastChatMessage(creature, message, 0, 0, creature->getMoodID());
			}, "BroadcastChatLambda2");
		}
	}

	return 0;
}

int DirectorManager::spatialMoodChat(lua_State* L) {
	if (checkArgumentCount(L, 4) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::spatialMoodChat";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	ZoneServer* zoneServer = ServerCore::getZoneServer();

	if (zoneServer == nullptr)
		return 0;

	ChatManager* chatManager = zoneServer->getChatManager();

	SceneObject* scene = (SceneObject*)lua_touserdata(L, -4);

	CreatureObject* creature = dynamic_cast<CreatureObject*>(scene);

	// Infinity:  No need to crash server with assertion, just throw a Lua error and return
	if (creature == nullptr) {
		String msg = "DirectorManager::spatialMoodChat - Detected assertion failure condition." + LuaSceneObject::getLuaCallStack(L);
		return luaL_error(L, msg.toCharArray());
	}

	// assert(creature);

	int moodType = lua_tonumber(L, -2);
	int chatType = lua_tonumber(L, -1);

	Locker locker(creature);

	if (lua_isuserdata(L, -3)) {
		StringIdChatParameter* message = (StringIdChatParameter*)lua_touserdata(L, -3);

		chatManager->broadcastChatMessage(creature, *message, 0, chatType, moodType);
	} else {
		String message = lua_tostring(L, -3);

		chatManager->broadcastChatMessage(creature, message, 0, chatType, moodType);
	}

	return 0;
}

int DirectorManager::getSceneObject(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::getSceneObject";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	uint64 objectID = lua_tointeger(L, -1);
	ZoneServer* zoneServer = ServerCore::getZoneServer();
	Reference<SceneObject*> object = zoneServer->getObject(objectID);

	if (object == nullptr) {
		lua_pushnil(L);
	} else {
		lua_pushlightuserdata(L, object.get());
		object->_setUpdated(true); //mark updated so the GC doesnt delete it while in LUA
	}

	return 1;
}

int DirectorManager::getRandomNumber(lua_State* L) {
	int numberOfArguments = lua_gettop(L);
	if (numberOfArguments != 1 && numberOfArguments != 2) {
		String err = "incorrect number of arguments passed to DirectorManager::getRandomNumber";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}
	int random;

	if (numberOfArguments == 1) {
		int max = lua_tointeger(L, -1);
		random = System::random(max - 1) + 1;
	} else {
		int min = lua_tointeger(L, -2);
		int max = lua_tointeger(L, -1);
		random = min + System::random(max - min);
	}
	lua_pushinteger(L, random);

	return 1;
}

int DirectorManager::getHashCode(lua_State* L) {
	int numberOfArguments = lua_gettop(L);

	if (numberOfArguments != 1) {
		String err = "incorrect number of arguments passed to DirectorManager::getHashCode";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String argument = lua_tostring(L, -1);
	uint32 hash = argument.hashCode();

	lua_pushinteger(L, hash);

	return 1;
}


int DirectorManager::getRegion(lua_State* L) {
	if (checkArgumentCount(L, 2) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::getRegion";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String regionName = lua_tostring(L, -1);
	String zoneName = lua_tostring(L, -2);

	ZoneServer* zoneServer = ServerCore::getZoneServer();

	if (zoneServer == nullptr) {
		lua_pushnil(L);
		return 1;
	}

	Zone* zone = zoneServer->getZone(zoneName);

	if (zone == nullptr) {
		lua_pushnil(L);
		return 1;
	}

	PlanetManager* planetMan = zone->getPlanetManager();

	if (planetMan == nullptr) {
		lua_pushnil(L);
		return 1;
	}

	SceneObject* region = planetMan->getRegion(regionName);

	if (region == nullptr) {
		lua_pushnil(L);
	} else {
		region->_setUpdated(true); //mark updated so the GC doesnt delete it while in LUA
		lua_pushlightuserdata(L, region);
	}

	return 1;
}

int DirectorManager::getCreatureObject(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::getCreatureObject";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	uint64 objectID = lua_tointeger(L, -1);
	ZoneServer* zoneServer = ServerCore::getZoneServer();
	Reference<SceneObject*> object = zoneServer->getObject(objectID);

	if (object != nullptr && object->isCreatureObject()) {
		lua_pushlightuserdata(L, object.get());
		object->_setUpdated(true); //mark updated so the GC doesnt delete it while in LUA
	} else {
		lua_pushnil(L);
	}

	return 1;
}

int DirectorManager::getContainerObjectByTemplate(lua_State* L) {
	if (checkArgumentCount(L, 3) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::getContainerObjectByTemplate";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	auto container = (SceneObject*)lua_touserdata(L, -3);
	String objectTemplate = lua_tostring(L, -2);
	bool checkChildren = lua_toboolean(L, -1);

	uint32 objectCRC = objectTemplate.hashCode();

	if (container == nullptr) {
		instance()->info("getContainerObjectByTemplate: SceneObject nullptr", true);
		lua_pushnil(L);

		return 1;
	}

	for (int i=0; i< container->getContainerObjectsSize(); i++) {
		auto sceno = container->getContainerObject(i);

		if (sceno == nullptr)
			continue;

		if (sceno->getServerObjectCRC() == objectCRC) {
			sceno->_setUpdated(true); //mark updated so the GC doesnt delete it while in LUA
			lua_pushlightuserdata(L, sceno);
			return 1;
		}

		if (checkChildren && sceno->isContainerObject()) {
			for (int j=0; j < sceno->getContainerObjectsSize(); j++) {
				auto child = sceno->getContainerObject(j);

				if (child == nullptr)
					continue;

				if (child->getServerObjectCRC() == objectCRC) {
					child->_setUpdated(true); //mark updated so the GC doesnt delete it while in LUA
					lua_pushlightuserdata(L, child);
					return 1;
				}

                if (child->isContainerObject()) {   //Infinity:  Check grandchildren
                    for (int k=0; k < child->getContainerObjectsSize(); k++) {
                        auto grandChild = child->getContainerObject(k);
        
                        if (grandChild == nullptr)
                            continue;
        
                        if (grandChild->getServerObjectCRC() == objectCRC) {
                            grandChild->_setUpdated(true); //mark updated so the GC doesnt delete it while in LUA
                            lua_pushlightuserdata(L, grandChild);
                            return 1;
                        }

                        if (grandChild->isContainerObject()) {      //Infinity:  Check greatgrandchildren
                            for (int l=0; l < grandChild->getContainerObjectsSize(); l++) {
                                auto greatGrandChild = grandChild->getContainerObject(l);
                
                                if (greatGrandChild == nullptr)
                                    continue;
                
                                if (greatGrandChild->getServerObjectCRC() == objectCRC) {
                                    greatGrandChild->_setUpdated(true); //mark updated so the GC doesnt delete it while in LUA
                                    lua_pushlightuserdata(L, greatGrandChild);
                                    return 1;
                                }
                            }
                        }
                    }
                }
			}
		}
	}

	lua_pushnil(L);

	return 1;
}

int DirectorManager::updateCellPermission(lua_State* L) {
	//realObject->info("getting values",true);
	if (checkArgumentCount(L, 3) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::updateCellPermission";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	SceneObject* sco = (SceneObject*)lua_touserdata(L, -3);
	bool allowEntry = lua_toboolean(L, -2);
	CreatureObject* obj = (CreatureObject*)lua_touserdata(L, -1);

	//sco->info("allowentry:" + String::valueOf(allowEntry), true);
	if (obj == nullptr) {
		instance()->info("Object nullptr", true);
		return 0;
	}

	//sco->info("values not nullptr", true);

	if (sco == nullptr) {
		obj->info("Cell nullptr", true);
		return 0;
	}

	if (!sco->isCellObject()) {
		sco->info("Unknown entity error: Cell", true);
		return 0;
	}

	if (!obj->isCreatureObject()) {
		//sco->info("Unknown entity error: Creature", true);
		obj->info("Unknown entity error: Creature", true);
		return 0;
	}

	//sco->info("checks are fine", true);

	BaseMessage* perm = new UpdateCellPermissionsMessage(sco->getObjectID(), allowEntry);
	obj->sendMessage(perm);

	return 0;
}

int DirectorManager::updateCellPermissionGroup(lua_State* L) {
	if (checkArgumentCount(L, 3) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::updateCellPermissionGroup";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	//realObject->info("getting values",true);
	SceneObject* sco = (SceneObject*)lua_touserdata(L, -3);
	int allowEntry = lua_tonumber(L, -2);
	CreatureObject* obj = (CreatureObject*)lua_touserdata(L, -1);
	//realObject->info("allowentry:" + String::valueOf(allowEntry), true);
	if (obj == nullptr || sco == nullptr)
		return 0;

	//realObject->info("values not nullptr", true);

	if (!sco->isCellObject()) {
		sco->info("Unknown entity error: Cell", true);
		return 0;
	}

	if (!obj->isCreatureObject()) {
		//sco->info("Unknown entity error: Creature", true);
		obj->info("Unknown entity error: Creature", true);
		return 0;
	}

	BaseMessage* perm = new UpdateCellPermissionsMessage(sco->getObjectID(), allowEntry);

	//sco->info("checks are fine", true);
	if (obj->isGrouped()) {
		// do group
		GroupObject* group = obj->getGroup();
		if (group != nullptr) {
			group->broadcastMessage(perm);
		} else {
			delete perm;
		}
	} else {
		// do single creature
		obj->sendMessage(perm);
	}

	return 0;
}

int DirectorManager::forcePeace(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::forcePeace";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	CreatureObject* creatureObject = (CreatureObject*)lua_touserdata(L, -1);

	if (creatureObject != nullptr) {
		Locker locker(creatureObject);

		ThreatMap* threatMap = creatureObject->getThreatMap();

		if (threatMap != nullptr)
			threatMap->removeAll();

		CombatManager::instance()->forcePeace(creatureObject);
	}

	return 0;
}

int DirectorManager::addStartingItemsInto(lua_State* L) {
	if (checkArgumentCount(L, 2) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::addStartingItemsInto";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	CreatureObject* creatureObject = (CreatureObject*)lua_touserdata(L, -2);
	SceneObject* sceneObject = (SceneObject*)lua_touserdata(L, -1);

	if (creatureObject != nullptr && sceneObject != nullptr) {
		PlayerCreationManager* pcm = PlayerCreationManager::instance();
		pcm->addStartingItemsInto(creatureObject, sceneObject);
	}

	return 0;
}

int DirectorManager::addStartingWeaponsInto(lua_State* L) {
	if (checkArgumentCount(L, 2) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::addStartingWeaponsInto";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	CreatureObject* creatureObject = (CreatureObject*)lua_touserdata(L, -2);
	SceneObject* sceneObject = (SceneObject*)lua_touserdata(L, -1);

	//SceneObject* sceneObject = creatureObject->getSlottedObject("inventory");

	if (creatureObject != nullptr && sceneObject != nullptr) {
		PlayerCreationManager* pcm = PlayerCreationManager::instance();
		pcm->addStartingWeaponsInto(creatureObject, sceneObject);
	}

	return 0;
}

int DirectorManager::giveItem(lua_State* L) {
	int numberOfArguments = lua_gettop(L);
	if (numberOfArguments != 3 && numberOfArguments != 4) {
		String err = "incorrect number of arguments passed to DirectorManager::giveItem";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	SceneObject* obj;
	String objectString;
	int slot = -1;
	bool overload = false;

	if (numberOfArguments == 3) {
		obj = (SceneObject*) lua_touserdata(L, -3);
        if (!lua_isstring(L, -2)) {
            String err = "Invalid or nil string passed to DirectorManager::giveItem";
            printTraceError(L, err);
            ERROR_CODE = INCORRECT_ARGUMENTS;
            return 0;
        }
		objectString = lua_tostring(L, -2);
		slot = lua_tointeger(L, -1);
	} else {
		obj = (SceneObject*) lua_touserdata(L, -4);
        if (!lua_isstring(L, -3)) {
            String err = "Invalid or nil string passed to DirectorManager::giveItem";
            printTraceError(L, err);
            ERROR_CODE = INCORRECT_ARGUMENTS;
            return 0;
        }
		objectString = lua_tostring(L, -3);
		slot = lua_tointeger(L, -2);
		overload = lua_toboolean(L, -1);
	}

	if (obj == nullptr)
		return 0;

	ZoneServer* zoneServer = obj->getZoneServer();

	ManagedReference<SceneObject*> item = zoneServer->createObject(objectString.hashCode(), 1);

	if (item != nullptr && obj != nullptr) {
		Locker objLocker(obj);
		Locker itemLocker(item, obj);

		if (obj->transferObject(item, slot, true, overload)) {
			item->_setUpdated(true); //mark updated so the GC doesnt delete it while in LUA

			ManagedReference<SceneObject*> parent = item->getParentRecursively(SceneObjectType::PLAYERCREATURE);
			if (parent != nullptr && parent->isPlayerCreature()) {
				item->sendTo(parent, true);
			}

			lua_pushlightuserdata(L, item.get());
		} else {
			item->destroyObjectFromDatabase(true);
			lua_pushnil(L);
		}
	} else {
		lua_pushnil(L);
	}

	return 1;
}

int DirectorManager::giveControlDevice(lua_State* L) {
	if (checkArgumentCount(L, 5) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::giveControlDevice";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	SceneObject* datapad = (SceneObject*) lua_touserdata(L, -5);
	String objectString = lua_tostring(L, -4);
	String controlledObjectPath = lua_tostring(L, -3);
	int slot = lua_tointeger(L, -2);
	bool mobile = lua_toboolean(L, -1);

	if (datapad == nullptr) {
		lua_pushnil(L);
		return 1;
	}

	ZoneServer* zoneServer = datapad->getZoneServer();
	Zone* zone = datapad->getZone();

	if (zone == nullptr) {
		lua_pushnil(L);
		return 1;
	}

	ManagedReference<ControlDevice*> controlDevice = zoneServer->createObject(objectString.hashCode(), 1).castTo<ControlDevice*>();

	if (controlDevice == nullptr) {
		lua_pushnil(L);
		return 1;
	}

	Locker locker(controlDevice);

	ManagedReference<TangibleObject*> controlledObject = nullptr;
	ManagedReference<CreatureObject*> player = (datapad->getParent().get()).castTo<CreatureObject*>();

	if (mobile) {
		CreatureManager* creatureManager = zone->getCreatureManager();
		CreatureTemplate* creoTempl = CreatureTemplateManager::instance()->getTemplate(controlledObjectPath.hashCode());

		if (creoTempl == nullptr) {
			controlDevice->destroyObjectFromDatabase(true);
			lua_pushnil(L);
			return 1;
		}

		String templateToSpawn = creatureManager->getTemplateToSpawn(controlledObjectPath.hashCode());
		controlledObject = creatureManager->createCreature(templateToSpawn.hashCode(), true, controlledObjectPath.hashCode());

		if (controlledObject == nullptr) {
			controlDevice->destroyObjectFromDatabase(true);
			lua_pushnil(L);
			return 1;
		}

		Locker locker2(controlledObject, controlDevice);

		if (!controlledObject->isAiAgent()) {
			controlDevice->destroyObjectFromDatabase(true);
			controlledObject->destroyObjectFromDatabase(true);
			lua_pushnil(L);
			return 1;
		}

		AiAgent* pet = controlledObject.castTo<AiAgent*>();
		pet->loadTemplateData(creoTempl);

	} else {
		controlledObject = zoneServer->createObject(controlledObjectPath.hashCode(), 1).castTo<TangibleObject*>();

		if (controlledObject == nullptr) {
			controlDevice->destroyObjectFromDatabase(true);
			lua_pushnil(L);
			return 1;
		}

		Locker locker2(controlledObject, controlDevice);

		SharedObjectTemplate* temp = controlledObject->getObjectTemplate();
		controlledObject->loadTemplateData(temp);
	}

	controlDevice->setControlledObject(controlledObject);
	StringId s;
	s.setStringId(controlledObject->getObjectName()->getFullPath());
	controlDevice->setObjectName(s, false);

	PetControlDevice* petControlDevice = cast<PetControlDevice*>(controlDevice.get());
	if( petControlDevice != nullptr ){
		petControlDevice->setDefaultCommands();
	}

	if (datapad->transferObject(controlDevice, slot, true)) {
		controlDevice->_setUpdated(true); //mark updated so the GC doesnt delete it while in LUA
		lua_pushlightuserdata(L, controlDevice.get());
	} else {
		controlDevice->destroyObjectFromDatabase(true);
		lua_pushnil(L);
	}

	return 1;
}

int DirectorManager::checkTooManyHirelings(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::checkTooManyHirelings";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	SceneObject* datapad = (SceneObject*) lua_touserdata(L, -1);

	if (datapad == nullptr) {
		lua_pushboolean(L, true);
		return 1;
	}

	int numberOfHirelings = 0;

	for (int i = 0; i < datapad->getContainerObjectsSize(); i++) {
		Reference<SceneObject*> object = datapad->getContainerObject(i);

		if (object != nullptr && object->isPetControlDevice()) {
			PetControlDevice* device = object.castTo<PetControlDevice*>();

			if (device->getPetType() == PetManager::FACTIONPET)
				numberOfHirelings++;
		}
	}

	ManagedReference<PlayerManager*> playerManager = ServerCore::getZoneServer()->getPlayerManager();

	bool result = numberOfHirelings >= playerManager->getBaseStoredFactionPets();

	lua_pushboolean(L, result);

	return 1;
}

int DirectorManager::setAuthorizationState(lua_State* L) {
	if (checkArgumentCount(L, 2) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::setAuthorizationState";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	SceneObject* terminal = (SceneObject*)lua_touserdata(L, -2);
	bool state = lua_toboolean(L, -1);

	//SceneObject* sceneObject = creatureObject->getSlottedObject("inventory");

	if (terminal == nullptr) {
		instance()->info("setAuthorizationState: Terminal is nullptr");
		return 0;
	}

	if (terminal->getGameObjectType() != SceneObjectType::NEWBIETUTORIALTERMINAL) {
		instance()->info("setAuthorizationState: Wrong SceneObjectType:" + String::valueOf(terminal->getGameObjectType()));
		return 0;
	}

	ManagedReference<StartingLocationTerminal*> item = (StartingLocationTerminal*)terminal;
	item->setAuthorizationState(state);

	return 0;
}

int DirectorManager::spawnMobile(lua_State* L) {
	int numberOfArguments = lua_gettop(L);
	if (numberOfArguments != 8 && numberOfArguments != 9) {
		String err = "incorrect number of arguments passed to DirectorManager::spawnMobile";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	bool randomRespawn = false;
	uint64 parentID;
	float x, y, z, headingDegrees;
	int respawnTimer;
	String mobile, zoneid;

	if (numberOfArguments == 8) {
		parentID = lua_tointeger(L, -1);
		headingDegrees = lua_tonumber(L, -2); 
		y = lua_tonumber(L, -3);
		z = lua_tonumber(L, -4);
		x = lua_tonumber(L, -5);
		respawnTimer = lua_tointeger(L, -6);
		mobile = lua_tostring(L, -7);
		zoneid = lua_tostring(L, -8);
	} else {
		randomRespawn = lua_toboolean(L, -1);
		parentID = lua_tointeger(L, -2);
		headingDegrees = lua_tonumber(L, -3);
		y = lua_tonumber(L, -4);
		z = lua_tonumber(L, -5);
		x = lua_tonumber(L, -6);
		respawnTimer = lua_tointeger(L, -7);
		mobile = lua_tostring(L, -8);
		zoneid = lua_tostring(L, -9);
	}

	ZoneServer* zoneServer = ServerCore::getZoneServer();

	Zone* zone = zoneServer->getZone(zoneid);

	if (zone == nullptr) {
		lua_pushnil(L);
		return 1;
	}

	CreatureManager* creatureManager = zone->getCreatureManager();

	/*StringBuffer msg;
	msg << "trying to spawn with mobile: " << mobile << " x:" << x;
	DirectorManager::instance()->info(msg.toString(), true);*/

	CreatureObject* creature = creatureManager->spawnCreature(mobile.hashCode(), 0, x, z, y, parentID, false, headingDegrees);

	if (creature == nullptr) {
		String err = "could not spawn mobile with template name: " + mobile;
		printTraceError(L, err);

		lua_pushnil(L);
	} else {
		Locker locker(creature);

		//creature->updateDirection(Math::deg2rad(heading));

		if (creature->isAiAgent()) {
			AiAgent* ai = cast<AiAgent*>(creature);
			ai->setRespawnTimer(respawnTimer);

			if (respawnTimer > 0 && parentID > 0) {
				zone->incrementSpawnedAgents();
			}

			if (randomRespawn)
				ai->setRandomRespawn(true);

			ai->setAITemplate();
		}

		creature->_setUpdated(true); //mark updated so the GC doesnt delete it while in LUA
		lua_pushlightuserdata(L, creature);
	}

	return 1;
	//public native CreatureObject spawnCreature(unsigned int templateCRC, float x, float z, float y, unsigned long parentID = 0);
}

int DirectorManager::spawnEventMobile(lua_State* L) {
	int numberOfArguments = lua_gettop(L);
	if (numberOfArguments != 8) {
		String err = "incorrect number of arguments passed to DirectorManager::spawnEventMobile";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	uint64 parentID;
	float x, y, z, headingDegrees;
	int respawnTimer, level;
	String mobile, zoneid;

	if (numberOfArguments == 8) {
		parentID = lua_tointeger(L, -1);
		headingDegrees = lua_tonumber(L, -2);
		y = lua_tonumber(L, -3);
		z = lua_tonumber(L, -4);
		x = lua_tonumber(L, -5);
		level = lua_tointeger(L, -6);
		mobile = lua_tostring(L, -7);
		zoneid = lua_tostring(L, -8);
	}

	ZoneServer* zoneServer = ServerCore::getZoneServer();

	Zone* zone = zoneServer->getZone(zoneid);

	if (zone == nullptr) {
		lua_pushnil(L);
		return 1;
	}

	CreatureManager* creatureManager = zone->getCreatureManager();

	CreatureObject* creature = creatureManager->spawnCreatureAsEventMob(mobile.hashCode(), level, x, z, y, parentID, headingDegrees);

	if (creature == nullptr) {
		String err = "could not spawn mobile with template name: " + mobile;
		printTraceError(L, err);

		lua_pushnil(L);
	} else {
		Locker locker(creature);

		//creature->updateDirection(Math::deg2rad(heading));

		creature->_setUpdated(true); //mark updated so the GC doesnt delete it while in LUA
		lua_pushlightuserdata(L, creature);
	}

	return 1;
}

int DirectorManager::spawnShipAgent(lua_State* L) {
	int numberOfArguments = lua_gettop(L);

	if (numberOfArguments < 5 || numberOfArguments > 6) {
		String err = "incorrect number of arguments passed to DirectorManager::spawnShipAgent";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	auto shipManager = ShipManager::instance();

	if (shipManager == nullptr) {
		lua_pushnil(L);
		return 1;
	}

	float x, z, y;
	Vector3 spawnPosition = Vector3::ZERO;
	String shipName, zoneName;
	ShipObject* targetShip = nullptr;

	if (numberOfArguments == 6) {
		targetShip = (ShipObject*) lua_touserdata(L, -1);
		spawnPosition.setY(lua_tonumber(L, -2));
		spawnPosition.setZ(lua_tonumber(L, -3));
		spawnPosition.setX(lua_tonumber(L, -4));
		zoneName = lua_tostring(L, -5);
		shipName = lua_tostring(L, -6);
	} else {
		spawnPosition.setY(lua_tonumber(L, -1));
		spawnPosition.setZ(lua_tonumber(L, -2));
		spawnPosition.setX(lua_tonumber(L, -3));
		zoneName = lua_tostring(L, -4);
		shipName = lua_tostring(L, -5);
	}

	auto zoneServer = ServerCore::getZoneServer();

	if (zoneServer == nullptr) {
		lua_pushnil(L);
		return 1;
	}

	auto spaceZone = zoneServer->getZone(zoneName);

	if (spaceZone == nullptr || !spaceZone->isSpaceZone()) {
		lua_pushnil(L);
		return 1;
	}

	ShipAiAgent* shipAgent = shipManager->createAiShip(shipName);

	if (shipAgent == nullptr) {
		lua_pushnil(L);
		return 1;
	}

	Locker lock(shipAgent);

	Quaternion targetDirection = Quaternion::IDENTITY;

	if (targetShip != nullptr) {
		const auto& targetPosition = targetShip->getPosition();

		Vector3 velocity = targetPosition - spawnPosition; // direction to target
		float distance = SpaceMath::qNormalize(velocity);
		auto rotation = SpaceMath::velocityToRotation(velocity); // get our phi vector

		targetDirection = SpaceMath::rotationToQuaternion(rotation, false);
	}

	// Set the home location
	shipAgent->setHomeLocation(spawnPosition.getX(), spawnPosition.getZ(), spawnPosition.getY(), targetDirection);

	// Set position in zone
	shipAgent->initializePosition(spawnPosition);

	// Set Ship direction
	shipAgent->setDirection(targetDirection);

	shipAgent->setHyperspacing(true);

	shipAgent->initializeTransform(spawnPosition, targetDirection);

	if (!spaceZone->transferObject(shipAgent, -1, true)) {
		shipAgent->destroyObjectFromWorld(true);

		lua_pushnil(L);
		return 1;
	}

 	// mark updated so the GC doesnt delete it while in LUA
	shipAgent->_setUpdated(true);
	lua_pushlightuserdata(L, shipAgent);

	shipAgent->setHyperspacing(false);

	return 1;
}

int DirectorManager::spawnSecurityPatrol(lua_State* L) {
	int numberOfArguments = lua_gettop(L);

	if (numberOfArguments != 9) {
		String err = "incorrect number of arguments passed to DirectorManager::spawnSecurityPatrol";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	bool attackable = true;
	bool stationary = false;

	attackable = lua_toboolean(L, -1);
	stationary = lua_toboolean(L, -2);

	int direction = lua_tointeger(L, -3);
	uint64 parentID = lua_tointeger(L, -4);
	float y = lua_tonumber(L, -5);
	float z = lua_tonumber(L, -6);
	float x = lua_tonumber(L, -7);
	String planet = lua_tostring(L, -8);
	String patrol = lua_tostring(L, -9);

	ZoneServer* zoneServer = ServerCore::getZoneServer();

	if (zoneServer == nullptr)
		return 0;

	Zone* zone = zoneServer->getZone(planet);

	if (zone == nullptr)
		return 0;

	GCWManager* gcwMan = zone->getGCWManager();

	if (gcwMan == nullptr)
		return 0;

	Vector3 location(x, y, z);

	uint64 squadLeaderID = gcwMan->spawnSecurityPatrol(nullptr, patrol, location, parentID, direction, stationary, attackable);

	lua_pushinteger(L, squadLeaderID);

	return 1;
}

int DirectorManager::despawnSecurityPatrol(lua_State* L) {
	int numberOfArguments = lua_gettop(L);

	if (numberOfArguments != 1) {
		String err = "incorrect number of arguments passed to DirectorManager::despawnSecurityPatrol";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	CreatureObject* creature = (CreatureObject*)lua_touserdata(L, -1);

	if (creature == nullptr)
		return 0;

	SortedVector<ManagedReference<Observer* > > observers = creature->getObservers(ObserverEventType::SQUAD);

	if (observers.size() > 0) {
		ManagedReference<SquadObserver*> squadObserver = cast<SquadObserver*>(observers.get(0).get());

		if (squadObserver != nullptr) {
			squadObserver->despawnSquad();
		}
	}

	return 0;
}

int DirectorManager::spawnBuilding(lua_State* L) {
	int numberOfArguments = lua_gettop(L);
	if (numberOfArguments < 5 || numberOfArguments > 6) {
		String err = "incorrect number of arguments passed to DirectorManager::spawnBuilding";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

    ManagedReference<CreatureObject*> creature;
	float x, y, angle;
	uint64 parentID;
	String script, zoneID;
    int persistenceLevel = 0;

    if (numberOfArguments == 6) {
        if (!lua_isuserdata(L, -6) ||
            !lua_isstring(L, -5) || 
            !lua_isnumber(L, -4) || 
            !lua_isnumber(L, -3) || 
            !lua_isnumber(L, -2) || 
            !(lua_isnumber(L,-1) || lua_isnil(L,-1))) {
            String err = "One or more invalid arguments passed to DirectorManager::spawnBuilding";
            printTraceError(L, err);
            ERROR_CODE = INCORRECT_ARGUMENTS;
            return 0;
        }
        else {
            if (lua_isnumber(L,-1)) {
                persistenceLevel = lua_tonumber(L, -1);
            }
            // else: persistenceLevel stays 0 (default — shadow declaration removed)
            angle = lua_tointeger(L, -2);
            y = lua_tonumber(L, -3);
            x = lua_tonumber(L, -4);
            script = lua_tostring(L, -5);
            creature = (CreatureObject*)lua_touserdata(L, -6);
        }
    }
    else {
        if (!lua_isuserdata(L, -5) ||
            !lua_isstring(L, -4) || 
            !lua_isnumber(L, -3) || 
            !lua_isnumber(L, -2) || 
            !lua_isnumber(L,-1)) {
            String err = "One or more invalid arguments passed to DirectorManager::spawnBuilding";
            printTraceError(L, err);
            ERROR_CODE = INCORRECT_ARGUMENTS;
            return 0;
        }
        else {
            angle = lua_tointeger(L, -1);
            y = lua_tonumber(L, -2);
            x = lua_tonumber(L, -3);
            script = lua_tostring(L, -4);
            creature = (CreatureObject*)lua_touserdata(L, -5);
        }
    }

	SharedStructureObjectTemplate* serverTemplate = dynamic_cast<SharedStructureObjectTemplate*>(TemplateManager::instance()->getTemplate(script.hashCode()));

	if (serverTemplate == nullptr) {
		String err = "Unable to find template for building " + script;
		printTraceError(L, err);
		lua_pushnil(L);
	} else {
		auto structure = StructureManager::instance()->placeStructure(creature, script, x, y, angle, persistenceLevel);
		if (structure == nullptr) {
			String err = "Unable to spawn building " + script;
			printTraceError(L, err);
			lua_pushnil(L);
		} else {
			structure->_setUpdated(true);
			lua_pushlightuserdata(L, structure);
		}
	}
	return 1;
}

int DirectorManager::destroyBuilding(lua_State* L) {
	int numberOfArguments = lua_gettop(L);
	if (numberOfArguments != 1) {
		String err = "incorrect number of arguments passed to DirectorManager::destroyBuilding";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}
	uint64 objectID = lua_tointeger(L, -1);
	ZoneServer* zoneServer = ServerCore::getZoneServer();
	Reference<SceneObject*> object = zoneServer->getObject(objectID);

	if (object == nullptr)
		return 0;

	ManagedReference<StructureObject*> building = object.castTo<StructureObject*>();

	if (building == nullptr || !building->isStructureObject())
		return 0;

	Reference<Task*> pendingTask = building->getPendingTask("destruction");

	if (pendingTask != nullptr)
		return 0;

	Reference<DestroyStructureTask*> task = new DestroyStructureTask(building);
	task->execute();
	return 1;
}

int DirectorManager::spawnSceneObject(lua_State* L) {
	int numberOfArguments = lua_gettop(L);
	if (numberOfArguments != 10 && numberOfArguments != 7) {
		String err = "incorrect number of arguments passed to DirectorManager::spawnSceneObject";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	float dz, dy, dx, dw, x, y, z;
	uint64 parentID;
	String script, zoneID;

	if (numberOfArguments == 10) {
		dz = lua_tonumber(L, -1);
		dy = lua_tonumber(L, -2);
		dx = lua_tonumber(L, -3);
		dw = lua_tonumber(L, -4);
		parentID = lua_tointeger(L, -5);
		y = lua_tonumber(L, -6);
		z = lua_tonumber(L, -7);
		x = lua_tonumber(L, -8);
		script = lua_tostring(L, -9);
		zoneID = lua_tostring(L, -10);
	} else {
		Quaternion direction;
		direction.setHeadingDirection(lua_tonumber(L, -1));
		dz = direction.getZ();
		dy = direction.getY();
		dx = direction.getX();
		dw = direction.getW();
		parentID = lua_tointeger(L, -2);
		y = lua_tonumber(L, -3);
		z = lua_tonumber(L, -4);
		x = lua_tonumber(L, -5);
		script = lua_tostring(L, -6);
		zoneID = lua_tostring(L, -7);
	}

	ZoneServer* zoneServer = ServerCore::getZoneServer();
	Zone* zone = zoneServer->getZone(zoneID);

	if (zone == nullptr) {
		lua_pushnil(L);
		return 1;
	}

	ManagedReference<SceneObject*> object = zoneServer->createObject(script.hashCode(), 0);

	if (object != nullptr) {
		Locker objLocker(object);

		object->initializePosition(x, z, y);
		object->setDirection(dw, dx, dy, dz);

		if (object->isBuildingObject()) {
			BuildingObject* building = object->asBuildingObject();

			building->createCellObjects();
		}

		Reference<SceneObject*> cellParent = nullptr;

		if (parentID != 0) {
			cellParent = zoneServer->getObject(parentID);

			if (cellParent != nullptr && !cellParent->isCellObject()) {
				//error("trying to set a parent that is not a cell to creature");
				cellParent = nullptr;
			}
		}

		if (cellParent != nullptr) {
			cellParent->broadcastObject(object, true);
			cellParent->transferObject(object, -1, true);
		} else {
			zone->transferObject(object, -1, true);
		}

		object->createChildObjects();

		object->_setUpdated(true); //mark updated so the GC doesnt delete it while in LUA

		lua_pushlightuserdata(L, object.get());
	} else {
		String err = "could not spawn object with template: " + script;
		printTraceError(L, err);
		lua_pushnil(L);
	}

	return 1;
}

int DirectorManager::spawnActiveArea(lua_State* L) {
	int numberOfArguments = lua_gettop(L);
	if (numberOfArguments != 7) {
		String err = "incorrect number of arguments passed to DirectorManager::spawnActiveArea";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	uint64 cellID = lua_tointeger(L, -1);
	int radius = lua_tointeger(L, -2);
	float y = lua_tonumber(L, -3);
	float z = lua_tonumber(L, -4);
	float x = lua_tonumber(L, -5);
	String script = lua_tostring(L, -6);
	String zoneID = lua_tostring(L, -7);

	ZoneServer* zoneServer = ServerCore::getZoneServer();
	Zone* zone = zoneServer->getZone(zoneID);

	if (zone == nullptr) {
		lua_pushnil(L);
		return 1;
	}

	ManagedReference<SceneObject*> object = zoneServer->createObject(script.hashCode(), 0);

	if (object != nullptr && object->isActiveArea()) {
		ActiveArea* area = object.castTo<ActiveArea*>();

		if (area == nullptr) {
			lua_pushnil(L);
			return 1;
		}

		Locker locker(area);

		area->initializePosition(x, z, y);
		area->setRadius(radius);

		Reference<SceneObject*> cellParent = nullptr;

		if (cellID != 0) {
			cellParent = zoneServer->getObject(cellID);

			if (cellParent == nullptr || !cellParent->isCellObject()) {
				cellID = 0;
			}
		}

		area->setCellObjectID(cellID);

		Locker zoneLocker(zone);

		zone->transferObject(area, -1, true);

		area->_setUpdated(true); //mark updated so the GC doesnt delete it while in LUA

		lua_pushlightuserdata(L, area);
	} else {
		lua_pushnil(L);
	}

	return 1;
}

int DirectorManager::spawnSpaceActiveArea(lua_State* L) {
	int numberOfArguments = lua_gettop(L);

	if (numberOfArguments != 6) {
		String err = "incorrect number of arguments passed to DirectorManager::spawnSpaceActiveArea";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;

		lua_pushnil(L);
		return 1;
	}

	float radius = lua_tonumber(L, -1);
	float y = lua_tonumber(L, -2);
	float z = lua_tonumber(L, -3);
	float x = lua_tonumber(L, -4);
	String areaTemplateName = lua_tostring(L, -5);
	String zoneName = lua_tostring(L, -6);

	auto zoneServer = ServerCore::getZoneServer();

	if (zoneServer == nullptr) {
		lua_pushnil(L);
		return 1;
	}

	auto zone = zoneServer->getZone(zoneName);

	if (zone == nullptr) {
		lua_pushnil(L);
		return 1;
	}

	ManagedReference<SceneObject*> object = zoneServer->createObject(areaTemplateName.hashCode(), 0);

	if (object == nullptr) {
		lua_pushnil(L);
		return 1;
	}

	auto spaceArea = (object).castTo<SpaceActiveArea*>();
	ManagedReference<SphereAreaShape*> sphereAreaShape = new SphereAreaShape();

	if (spaceArea == nullptr || sphereAreaShape == nullptr) {
		lua_pushnil(L);
		return 1;
	}

	Locker locker(spaceArea);

	spaceArea->setRadius(radius);
	spaceArea->initializePosition(x, z, y);

	// Lock the shape for mutation
	Locker shapeLocker(sphereAreaShape, spaceArea);

	sphereAreaShape->setAreaCenter(x, z, y);
	sphereAreaShape->setRadius(radius);

	spaceArea->setAreaShape(sphereAreaShape);

	shapeLocker.release();

	Locker zoneLocker(zone, spaceArea);

	zone->transferObject(spaceArea, -1, true);

	spaceArea->_setUpdated(true); //mark updated so the GC doesnt delete it while in LUA

	lua_pushlightuserdata(L, spaceArea);

	return 1;
}

int DirectorManager::createObserver(lua_State* L) {
	int numberOfArguments = lua_gettop(L);
	if (numberOfArguments != 4 && numberOfArguments != 5) {
		String err = "incorrect number of arguments passed to DirectorManager::createObserver";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	SceneObject* sceneObject;
	String key, play;
	uint32 eventType;
	int persistence = 0;

	if (numberOfArguments == 4) {
		sceneObject = (SceneObject*) lua_touserdata(L, -1);
		key = lua_tostring(L, -2);
		play = lua_tostring(L, -3);
		eventType = lua_tointeger(L, -4);
	} else {
		persistence = lua_tointeger(L, -1);
		sceneObject = (SceneObject*) lua_touserdata(L, -2);
		key = lua_tostring(L, -3);
		play = lua_tostring(L, -4);
		eventType = lua_tointeger(L, -5);
	}

	if (sceneObject == nullptr)
		return 0;

	ManagedReference<ScreenPlayObserver*> observer = dynamic_cast<ScreenPlayObserver*>(ObjectManager::instance()->createObject("ScreenPlayObserver", persistence, ""));

	if (observer == nullptr)
		return 0;

	observer->setScreenPlay(play);
	observer->setScreenKey(key);
	observer->setObserverType(ObserverType::SCREENPLAY);

	sceneObject->registerObserver(eventType, observer);

	lua_pushlightuserdata(L, observer);

	return 1;
}

int DirectorManager::hasObserver(lua_State* L) {
	int numberOfArguments = lua_gettop(L);

	if (numberOfArguments != 4) {
		String err = "incorrect number of arguments passed to DirectorManager::hasObserver";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	SceneObject* sceneObject = (SceneObject*) lua_touserdata(L, -1);
	String key = lua_tostring(L, -2);
	String play = lua_tostring(L, -3);
	uint32 eventType = lua_tointeger(L, -4);

	SortedVector<ManagedReference<Observer* > > observers = sceneObject->getObservers(eventType);
	bool ret = false;

	for (int i = 0; i < observers.size(); i++) {
		Observer* observer = observers.get(i).get();

		if (observer == nullptr || !observer->isObserverType(ObserverType::SCREENPLAY)) {
			continue;
		}

		auto screenplayObserver = cast<ScreenPlayObserver*>(observer);

		if (screenplayObserver == nullptr || !(screenplayObserver->getScreenPlay() == play && screenplayObserver->getScreenKey() == key)) {
			continue;
		}

		ret = true;
		break;
	}

	lua_pushboolean(L, ret);

	return 1;
}

int DirectorManager::hasObserverType(lua_State* L) {
	int numberOfArguments = lua_gettop(L);

	if (numberOfArguments != 2) {
		String err = "incorrect number of arguments passed to DirectorManager::hasObserver";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	SceneObject* sceneObject = (SceneObject*) lua_touserdata(L, -1);
	uint32 eventType = lua_tointeger(L, -2);

	SortedVector<ManagedReference<Observer* > > observers = sceneObject->getObservers(eventType);

	lua_pushboolean(L, observers.size() > 0);

	return 1;
}

int DirectorManager::dropObserver(lua_State* L) {
	int numberOfArguments = lua_gettop(L);
	if (numberOfArguments != 2 && numberOfArguments != 4) {
		String err = "incorrect number of arguments passed to DirectorManager::dropObserver";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	SceneObject* sceneObject = nullptr;
	uint32 eventType = 0;

	if (numberOfArguments == 2) {
		sceneObject = (SceneObject*) lua_touserdata(L, -1);
		eventType = lua_tointeger(L, -2);

		if (sceneObject == nullptr)
			return 0;

		SortedVector<ManagedReference<Observer* > > observers = sceneObject->getObservers(eventType);
		for (int i = 0; i < observers.size(); i++) {
			Observer* observer = observers.get(i).get();
			if (observer != nullptr && observer->isObserverType(ObserverType::SCREENPLAY)) {
				sceneObject->dropObserver(eventType, observer);

				if (observer->isPersistent())
					ObjectManager::instance()->destroyObjectFromDatabase(observer->_getObjectID());
			}
		}
	} else {
		sceneObject = (SceneObject*) lua_touserdata(L, -1);
		String key = lua_tostring(L, -2);
		String play = lua_tostring(L, -3);
		eventType = lua_tointeger(L, -4);

		if (sceneObject == nullptr)
			return 0;

		SortedVector<ManagedReference<Observer* > > observers = sceneObject->getObservers(eventType);
		for (int i = 0; i < observers.size(); i++) {
			Observer* observer = observers.get(i).get();
			if (observer != nullptr && observer->isObserverType(ObserverType::SCREENPLAY)) {
				ManagedReference<ScreenPlayObserver*> spObserver = dynamic_cast<ScreenPlayObserver*>(observer);

				if (spObserver->getScreenPlay() == play && spObserver->getScreenKey() == key) {
					sceneObject->dropObserver(eventType, observer);

					if (observer->isPersistent())
						ObjectManager::instance()->destroyObjectFromDatabase(observer->_getObjectID());
				}
			}
		}
	}

	return 0;
}

Lua* DirectorManager::getLuaInstance() {
	Lua* lua = localLua.get();
	uint32* version = localScreenPlayVersion.get();

	if (version == nullptr) {
		version = new uint32;
		*version = 0;
		localScreenPlayVersion.set(version);
	}

	if (lua == nullptr) {
		lua = new Lua();
		localLua.set(lua);

		initializeLuaEngine(lua);
		loadScreenPlays(lua);
		JediManager::instance()->loadConfiguration(lua);

		if (!lua->checkStack(0)) {
			error()
				<< __FILE__ << ":" << __LINE__ << ":" <<  __FUNCTION__ << "()"
				<< " LUA Stack Leak: Found " << lua_gettop(lua->getLuaState()) << " item(s) on stack.";
		}
	}

	if (*version != masterScreenPlayVersion.get()) {
		int stackSize = lua_gettop(lua->getLuaState());

		loadScreenPlays(lua);
		*version = masterScreenPlayVersion.get();

		if (!lua->checkStack(stackSize)) {
			error()
				<< __FILE__ << ":" << __LINE__ << ":" <<  __FUNCTION__ << "()"
				<< " LUA Stack Leak: Found " << lua_gettop(lua->getLuaState()) << " item(s) on stack.";
		}
	}

	return lua;
}

int DirectorManager::runScreenPlays() {
	Lua* lua = localLua.get();
	uint32* version = localScreenPlayVersion.get();
	int ret = 0;

	if (version == nullptr) {
		version = new uint32;
		*version = 0;
		localScreenPlayVersion.set(version);
	}

	if (lua == nullptr) {
		lua = new Lua();
		initializeLuaEngine(lua);
		ret = loadScreenPlays(lua);
		JediManager::instance()->loadConfiguration(lua);

		localLua.set(lua);

		if (!lua->checkStack(0)) {
			error()
				<< __FILE__ << ":" << __LINE__ << ":" <<  __FUNCTION__ << "()"
				<< " LUA Stack Leak: Found " << lua_gettop(lua->getLuaState()) << " item(s) on stack.";
		}
	}

	if (*version != masterScreenPlayVersion.get()) {
		int stackSize = lua_gettop(lua->getLuaState());

		ret = loadScreenPlays(lua);
		*version = masterScreenPlayVersion.get();

		if (!lua->checkStack(stackSize)) {
			error()
				<< __FILE__ << ":" << __LINE__ << ":" <<  __FUNCTION__ << "()"
				<< " LUA Stack Leak: Found " << lua_gettop(lua->getLuaState()) << " item(s) on stack.";
		}
	}

	return ret || ERROR_CODE;
}

void DirectorManager::startScreenPlay(CreatureObject* creatureObject, const String& screenPlayName) {
	Lua* lua = getLuaInstance();

	LuaFunction startScreenPlay(lua->getLuaState(), screenPlayName, "start", 0);
	startScreenPlay << creatureObject;

	startScreenPlay.callFunction();
}

ConversationScreen* DirectorManager::getNextConversationScreen(const String& luaClass, ConversationTemplate* conversationTemplate, CreatureObject* conversingPlayer, int selectedOption, SceneObject* conversingNPC) {
	Lua* lua = getLuaInstance();

	LuaFunction runMethod(lua->getLuaState(), luaClass, "getNextConversationScreen", 1);
	runMethod << conversationTemplate;
	runMethod << conversingPlayer;
	runMethod << selectedOption;
	runMethod << conversingNPC;

	runMethod.callFunction();

	ConversationScreen* result = (ConversationScreen*) lua_touserdata(lua->getLuaState(), -1);

	lua_pop(lua->getLuaState(), 1);

	return result;
}

ConversationScreen* DirectorManager::runScreenHandlers(const String& luaClass, ConversationTemplate* conversationTemplate, CreatureObject* conversingPlayer, SceneObject* conversingNPC, int selectedOption, ConversationScreen* conversationScreen) {
	Lua* lua = getLuaInstance();

	LuaFunction runMethod(lua->getLuaState(), luaClass, "runScreenHandlers", 1);
	runMethod << conversationTemplate;
	runMethod << conversingPlayer;
	runMethod << conversingNPC;
	runMethod << selectedOption;
	runMethod << conversationScreen;

	runMethod.callFunction();

	ConversationScreen* result = (ConversationScreen*) lua_touserdata(lua->getLuaState(), -1);

	lua_pop(lua->getLuaState(), 1);

	return result;
}

void DirectorManager::activateEvent(ScreenPlayTask* task) {
	ManagedReference<SceneObject*> obj = task->getSceneObject();
	const String& play = task->getScreenPlay();
	const String& key = task->getTaskKey();
	const String& args = task->getArgs();

	Reference<PersistentEvent*> persistentEvent = task->getPersistentEvent();

	if (persistentEvent != nullptr) {
		persistentEvent->setEventExecuted(true);

		dropServerEventReference(persistentEvent->getEventName());

		ObjectManager::instance()->destroyObjectFromDatabase(persistentEvent->_getObjectID());
	} /*else {
		StringBuffer msg;
		msg << "no PersistentEvent object in the task " << play << ":" << task;

		error(msg.toString());
	}*/

	Lua* lua = getLuaInstance();

	try {
		LuaFunction startScreenPlay(lua->getLuaState(), play, key, 0);
		startScreenPlay << obj.get();
		startScreenPlay << args.toCharArray();

		startScreenPlay.callFunction();
	} catch (Exception& e) {
		StringBuffer msg;
		msg << "exception while running lua task " << play << ":" << key;
		error(msg.toString());

		e.printStackTrace();
	}

	screenplayTasks.drop(task);
}

int DirectorManager::createConversationScreen(lua_State* L) {
	ConversationScreen* screen = new ConversationScreen();

	lua_pushlightuserdata(L, screen);

	return 1;
}

int DirectorManager::getStringId(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::getStringId";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String stringid = lua_tostring(L, -1);
	String stringvalue = StringIdManager::instance()->getStringId(stringid.hashCode()).toString();
	lua_pushstring(L, stringvalue.toCharArray());

	return 1;
}

int DirectorManager::getRankName(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::getRankName";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	int rank = lua_tointeger(L, -1);

	String rankName = FactionManager::instance()->getRankName(rank);

	lua_pushstring(L, rankName.toCharArray());

	return 1;
}

int DirectorManager::getRankCost(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::getRankCost";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	int rank = lua_tointeger(L, -1);

	lua_pushinteger(L, FactionManager::instance()->getRankCost(rank));

	return 1;
}

int DirectorManager::getRankDelegateRatioFrom(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::getRankDelegateRatioFrom";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	int rank = lua_tointeger(L, -1);

	lua_pushinteger(L, FactionManager::instance()->getRankDelegateRatioFrom(rank));

	return 1;
}

int DirectorManager::getRankDelegateRatioTo(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::getRankDelegateRatioTo";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	int rank = lua_tointeger(L, -1);

	lua_pushinteger(L, FactionManager::instance()->getRankDelegateRatioTo(rank));

	return 1;
}

int DirectorManager::isHighestRank(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::isHighestRank";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	int rank = lua_tointeger(L, -1);

	bool result = FactionManager::instance()->isHighestRank(rank);

	lua_pushboolean(L, result);

	return 1;
}

int DirectorManager::getFactionPointsCap(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::getFactionPointsCap";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	int rank = lua_tointeger(L, -1);

	lua_pushinteger(L, FactionManager::instance()->getFactionPointsCap(rank));

	return 1;
}

int DirectorManager::getObjectTemplatePathByCRC(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::getObjectTemplatePathByCRC";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	uint32 crc = lua_tointeger(L, -1);

	lua_pushstring(L, TemplateManager::instance()->getTemplateFile(crc).toCharArray());

	return 1;
}

int DirectorManager::getZoneByName(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::getZoneByName";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	auto zoneServer = ServerCore::getZoneServer();

	if (zoneServer == nullptr)
		return 0;

	String zoneName = lua_tostring(L, -1);

	Zone* zone = zoneServer->getZone(zoneName);

	if (zone == nullptr) {
		lua_pushnil(L);
	} else {
		lua_pushlightuserdata(L, zone);
	}

	return 1;
}

int DirectorManager::isZoneEnabled(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::isZoneEnabled";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String zoneid = lua_tostring(L, -1);

	Zone* zone = ServerCore::getZoneServer()->getZone(zoneid);

	lua_pushboolean(L, (zone != nullptr));

	return 1;
}

int DirectorManager::getZoneMaxX(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::getZoneMaxX";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String zoneid = lua_tostring(L, -1);

	Zone* zone = ServerCore::getZoneServer()->getZone(zoneid);

	if (zone == nullptr) {
		lua_pushnumber(L, 0);
		return 1;
	}

	float maxX = zone->getMaxX() * 15.f / 16.f;
	lua_pushnumber(L, maxX);
	return 1;
}

int DirectorManager::getZoneMaxY(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::getZoneMaxY";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String zoneid = lua_tostring(L, -1);

	Zone* zone = ServerCore::getZoneServer()->getZone(zoneid);

	if (zone == nullptr) {
		lua_pushnumber(L, 0);
		return 1;
	}

	float maxY = zone->getMaxY() * 15.f / 16.f;
	lua_pushnumber(L, maxY);
	return 1;
}

// Infinity: Radial scan for the nearest non-water point from (x,y).
// Returns 0 if the origin is already on land, the distance in meters to the
// nearest land sample (within stepSize precision), or -1 if no land was found
// within maxScan. Shared by DirectorManager::generateSpawnPoint (water-aware
// expansion) and DirectorManager::getDistanceToLand (Lua-exposed helper).
static float scanForNearestLand(Zone* zone, PlanetManager* planetManager, float x, float y,
                                float maxScan = 5000.0f, float stepSize = 100.0f) {
	if (planetManager == nullptr || zone == nullptr)
		return -1.0f;

	if (!planetManager->isInWater(x, y))
		return 0.0f;

	if (stepSize < 10.0f) stepSize = 10.0f;
	if (maxScan < stepSize) maxScan = stepSize;

	float minBoundX = zone->getMinX() * 15.f / 16.f;
	float maxBoundX = zone->getMaxX() * 15.f / 16.f;
	float minBoundY = zone->getMinY() * 15.f / 16.f;
	float maxBoundY = zone->getMaxY() * 15.f / 16.f;

	const int directions = 16; // 22.5° step
	int maxSteps = (int)(maxScan / stepSize);

	// Expand radius outward; first direction at any given radius that returns
	// non-water wins. Since r only increases, the first hit IS the nearest
	// land in the sampled directions (within stepSize precision).
	for (int step = 1; step <= maxSteps; step++) {
		float r = step * stepSize;

		for (int d = 0; d < directions; d++) {
			float angle = (2.0f * M_PI * d) / directions;
			float nx = x + cos(angle) * r;
			float ny = y + sin(angle) * r;

			if (nx < minBoundX || nx > maxBoundX || ny < minBoundY || ny > maxBoundY)
				continue;

			if (!planetManager->isInWater(nx, ny))
				return r;
		}
	}

	return -1.0f;
}

Vector3 DirectorManager::generateSpawnPoint(String zoneName, float x, float y, float minimumDistance, float maximumDistance, float extraNoBuildRadius, float sphereCollision, bool forceSpawn, bool strict) {
	bool found = false;
	Vector3 position(0, 0, 0);
	int retries = 75; // Infinity

	auto zoneServer = ServerCore::getZoneServer();

	if (zoneServer == nullptr) {
		return position;
	}

	auto zone = zoneServer->getZone(zoneName);

	if (zone == nullptr) {
		return position;
	}

	auto planetManager = zone->getPlanetManager();

	if (planetManager == nullptr) {
		return position;
	}

	int spawnDenied = 0;
	int sphereBlocked = 0;

	// Use actual zone boundaries (with 15/16 client adjustment) instead of hardcoded values
	float minBoundX = zone->getMinX() * 15.f / 16.f;
	float maxBoundX = zone->getMaxX() * 15.f / 16.f;
	float minBoundY = zone->getMinY() * 15.f / 16.f;
	float maxBoundY = zone->getMaxY() * 15.f / 16.f;

	// Infinity: Water-aware radius expansion.
	// Every caller of generateSpawnPoint already rejects water samples
	// (isInWater / isSpawningPermittedAt), so if the origin itself is in water
	// and the caller's requested annulus is also entirely in water, we waste
	// all 75 retries on guaranteed-water candidates. Pre-scan for the nearest
	// land and bump maximumDistance to reach it plus a small buffer.
	//
	// Callers that genuinely want strict-fail-in-water semantics (none known
	// today) can pass strict=true to opt out.
	if (!strict) {
		float landDist = scanForNearestLand(zone, planetManager, x, y);
		if (landDist > 0.0f) {
			float needed = landDist + 200.0f;
			if (maximumDistance < needed)
				maximumDistance = needed;
		}
		// landDist == 0: origin already on land — no change.
		// landDist == -1: no land within scan range — fall through and let the
		//                 normal ray-trim loop fail with the existing log.
	}

	int rayRejected = 0;

	// Infinity: Ray-trim sampling.
	// Old behavior picked a random (angle, distance) in the annulus and then CLAMPED
	// out-of-bounds candidates onto the zone edge, which silently broke the distance
	// invariant and caused most retries to land on the same dead edge points when the
	// origin was near a boundary (e.g. FS outro go_to_theater theaters at corellia
	// -7432,-5005).
	//
	// Now: pick an angle first, then compute how far the ray can travel from (x,y)
	// before it exits the zone rectangle, and sample the radius inside
	// [minimumDistance, min(maximumDistance, rayLen)]. The candidate is guaranteed
	// in-zone and honors both min/max distance from the origin.
	while (!found && retries > 0) {
		float angle = System::random(360);
		float newAngle = angle * (M_PI / 180.0f);
		float dx = cos(newAngle);
		float dy = sin(newAngle);

		// Distance along this ray from (x,y) to the nearest zone edge.
		// Origin is assumed in-zone, so only one of east/west and one of
		// north/south produces a positive, meaningful t.
		float rayLen = maximumDistance;
		if (dx > 1e-4f) {
			float tEdge = (maxBoundX - x) / dx;
			if (tEdge < rayLen) rayLen = tEdge;
		} else if (dx < -1e-4f) {
			float tEdge = (minBoundX - x) / dx;
			if (tEdge < rayLen) rayLen = tEdge;
		}
		if (dy > 1e-4f) {
			float tEdge = (maxBoundY - y) / dy;
			if (tEdge < rayLen) rayLen = tEdge;
		} else if (dy < -1e-4f) {
			float tEdge = (minBoundY - y) / dy;
			if (tEdge < rayLen) rayLen = tEdge;
		}

		// No room in this direction to place a point at least minimumDistance
		// away while staying in-zone — burn a retry and re-roll the angle.
		if (rayLen < minimumDistance) {
			rayRejected++;
			retries--;
			continue;
		}

		float range = rayLen - minimumDistance;
		float distance = minimumDistance + ((range > 0.0f) ? System::random((unsigned int)range) : 0);

		float newX = x + (dx * distance); // client has x/y inverted
		float newY = y + (dy * distance);

		float newZ = zone->getHeight(newX, newY);

		position.set(newX, newZ, newY);

		bool sphereCollisionDetected = CollisionManager::checkSphereCollision(position, sphereCollision, zone);

		if (forceSpawn) {
			// Skip no-spawn zone checks but still respect water and physical collisions (#1445)
			bool inWater = planetManager->isInWater(position.getX(), position.getY());

			if (inWater)
				spawnDenied++;
			else if (sphereCollisionDetected)
				sphereBlocked++;

			found = !inWater && !sphereCollisionDetected;
		} else {
			bool spawningPermitted = planetManager->isSpawningPermittedAt(position.getX(), position.getY(), extraNoBuildRadius);

			if (!spawningPermitted)
				spawnDenied++;
			else if (sphereCollisionDetected)
				sphereBlocked++;

			found = spawningPermitted && !sphereCollisionDetected;
		}

		retries--;
	}

	if (!found) {
		Logger::console.info(true)
			<< "generateSpawnPoint FAILED - Zone: " << zoneName
			<< " Origin: (" << x << ", " << y << ")"
			<< " Range: " << minimumDistance << "-" << maximumDistance
			<< " Retries: 75, SpawnDenied: " << spawnDenied
			<< " SphereBlocked: " << sphereBlocked
			<< " RayRejected: " << rayRejected
			<< " ForceSpawn: " << (forceSpawn ? "true" : "false");

		position.set(0, 0, 0);
	}

	return position;
}

// Infinity: getRandomZonePoint(zoneName [, forceSpawn])
// Picks a random X,Y uniformly within the zone rectangle, validates water/collision/no-build.
// Returns {x, z, y} or nil. No circle math — full rectangular coverage including corners.
int DirectorManager::getRandomZonePoint(lua_State* L) {
	int numberOfArguments = lua_gettop(L);
	if (numberOfArguments < 1 || numberOfArguments > 2) {
		printTraceError(L, "incorrect number of arguments passed to DirectorManager::getRandomZonePoint (expected 1-2)");
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	if (!lua_isstring(L, 1)) {
		printTraceError(L, "zoneName must be a string in getRandomZonePoint");
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String zoneName = lua_tostring(L, 1);
	bool forceSpawn = (numberOfArguments >= 2) ? lua_toboolean(L, 2) : false;

	auto zoneServer = ServerCore::getZoneServer();
	if (zoneServer == nullptr) {
		printTraceError(L, "ZoneServer is nullptr in getRandomZonePoint");
		return 0;
	}

	auto zone = zoneServer->getZone(zoneName);
	if (zone == nullptr) {
		printTraceError(L, "Zone is nullptr in getRandomZonePoint: " + zoneName);
		return 0;
	}

	auto planetManager = zone->getPlanetManager();
	if (planetManager == nullptr) {
		return 0;
	}

	float minX = zone->getMinX() * 15.f / 16.f;
	float maxX = zone->getMaxX() * 15.f / 16.f;
	float minY = zone->getMinY() * 15.f / 16.f;
	float maxY = zone->getMaxY() * 15.f / 16.f;

	float rangeX = maxX - minX;
	float rangeY = maxY - minY;

	int retries = 75;
	int spawnDenied = 0;
	int sphereBlocked = 0;

	while (retries > 0) {
		float newX = minX + System::random((unsigned int)rangeX);
		float newY = minY + System::random((unsigned int)rangeY);
		float newZ = zone->getHeight(newX, newY);

		Vector3 position;
		position.set(newX, newZ, newY);

		bool sphereCollisionDetected = CollisionManager::checkSphereCollision(position, 2.0f, zone);

		if (forceSpawn) {
			bool inWater = planetManager->isInWater(newX, newY);

			if (inWater)
				spawnDenied++;
			else if (sphereCollisionDetected)
				sphereBlocked++;
			else {
				lua_newtable(L);
				lua_pushnumber(L, position.getX());
				lua_pushnumber(L, position.getZ());
				lua_pushnumber(L, position.getY());
				lua_rawseti(L, -4, 3);
				lua_rawseti(L, -3, 2);
				lua_rawseti(L, -2, 1);
				return 1;
			}
		} else {
			bool spawningPermitted = planetManager->isSpawningPermittedAt(newX, newY, 5.0f);

			if (!spawningPermitted)
				spawnDenied++;
			else if (sphereCollisionDetected)
				sphereBlocked++;
			else {
				lua_newtable(L);
				lua_pushnumber(L, position.getX());
				lua_pushnumber(L, position.getZ());
				lua_pushnumber(L, position.getY());
				lua_rawseti(L, -4, 3);
				lua_rawseti(L, -3, 2);
				lua_rawseti(L, -2, 1);
				return 1;
			}
		}

		retries--;
	}

	Logger::console.error()
		<< "getRandomZonePoint FAILED - Zone: " << zoneName
		<< " Retries: 75, SpawnDenied: " << spawnDenied
		<< " SphereBlocked: " << sphereBlocked
		<< " ForceSpawn: " << (forceSpawn ? "true" : "false");

	return 0;
}

// Infinity: getDistanceToLand(zoneName, x, y [, maxScan [, stepSize]])
// Returns distance in meters from (x,y) to the nearest non-water point. Returns
// 0 if the origin is on land, or -1 if no land was found within maxScan. This
// is a Lua-exposed wrapper around scanForNearestLand(); generateSpawnPoint uses
// the same helper internally for water-aware radius expansion, so this is just
// here for explicit Lua queries and debugging.
int DirectorManager::getDistanceToLand(lua_State* L) {
	int numberOfArguments = lua_gettop(L);
	if (numberOfArguments < 3 || numberOfArguments > 5) {
		printTraceError(L, "incorrect number of arguments passed to DirectorManager::getDistanceToLand (expected 3-5)");
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	if (!lua_isstring(L, -numberOfArguments)) {
		printTraceError(L, "zoneName must be a string in getDistanceToLand");
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String zoneName = lua_tostring(L, -numberOfArguments);
	int baseIndex = -numberOfArguments + 1;

	if (!lua_isnumber(L, baseIndex) || !lua_isnumber(L, baseIndex + 1)) {
		printTraceError(L, "x and y must be numbers in getDistanceToLand");
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	float x = static_cast<float>(lua_tonumber(L, baseIndex));
	float y = static_cast<float>(lua_tonumber(L, baseIndex + 1));
	float maxScan = (numberOfArguments >= 4) ? static_cast<float>(lua_tonumber(L, baseIndex + 2)) : 3000.0f;
	float stepSize = (numberOfArguments >= 5) ? static_cast<float>(lua_tonumber(L, baseIndex + 3)) : 100.0f;

	auto zoneServer = ServerCore::getZoneServer();
	if (zoneServer == nullptr) {
		printTraceError(L, "ZoneServer is nullptr in getDistanceToLand");
		return 0;
	}

	auto zone = zoneServer->getZone(zoneName);
	if (zone == nullptr) {
		printTraceError(L, "Zone is nullptr in getDistanceToLand: " + zoneName);
		return 0;
	}

	auto planetManager = zone->getPlanetManager();
	if (planetManager == nullptr) {
		return 0;
	}

	lua_pushnumber(L, scanForNearestLand(zone, planetManager, x, y, maxScan, stepSize));
	return 1;
}

int DirectorManager::getSpawnPoint(lua_State* L) {
    // getSpawnPoint(zoneName, x, y, minDist, maxDist [, forceSpawn [, sphereRadius [, strict]]])
    // 5-8 arguments. strict=true disables water-aware radius expansion.
    int numberOfArguments = lua_gettop(L);
    if (numberOfArguments < 5 || numberOfArguments > 8) {
        printTraceError(L, "incorrect number of arguments passed to DirectorManager::getSpawnPoint (expected 5-8)");
        ERROR_CODE = INCORRECT_ARGUMENTS;
        return 0;
    }

    // Validate and extract zoneName
    if (!lua_isstring(L, -numberOfArguments)) {
        printTraceError(L, "zoneName must be a string");
        ERROR_CODE = INCORRECT_ARGUMENTS;
        return 0;
    }
    String zoneName = lua_tostring(L, -numberOfArguments);

    // Validate and extract numeric arguments
    int baseIndex = -numberOfArguments + 1;
    if (!lua_isnumber(L, baseIndex) || !lua_isnumber(L, baseIndex + 1) ||
        !lua_isnumber(L, baseIndex + 2) || !lua_isnumber(L, baseIndex + 3)) {
        printTraceError(L, "x, y, minimumDistance, and maximumDistance must be numbers");
        ERROR_CODE = INCORRECT_ARGUMENTS;
        return 0;
    }

    // Infinity: Reworked these arguments. Explicitly convert double to float
    float x = static_cast<float>(lua_tonumber(L, baseIndex));
    float y = static_cast<float>(lua_tonumber(L, baseIndex + 1));
    float minimumDistance = static_cast<float>(lua_tonumber(L, baseIndex + 2));
    float maximumDistance = static_cast<float>(lua_tonumber(L, baseIndex + 3));

    // Handle optional forceSpawn (arg 6), sphereRadius (arg 7), strict (arg 8)
    bool forceSpawn = (numberOfArguments >= 6) ? lua_toboolean(L, baseIndex + 4) : false;
    float sphereRadius = (numberOfArguments >= 7) ? static_cast<float>(lua_tonumber(L, baseIndex + 5)) : 2.0f;
    bool strict = (numberOfArguments >= 8) ? lua_toboolean(L, baseIndex + 6) : false;

    auto zoneServer = ServerCore::getZoneServer();

    if (zoneServer == nullptr) {
        String err = "ZoneServer is nullptr in DirectorManager::getSpawnPoint";
        printTraceError(L, err);
        return 0;
    }

	auto zone = zoneServer->getZone(zoneName);

	if (zone == nullptr) {
		String err = "Zone is nullptr in DirectorManager::getSpawnPoint. zoneName = " + zoneName;
		printTraceError(L, err);
		return 0;
	}

	bool found = false;
	int retries = 75; //Infinity


	Vector3 position;

	while (!found && retries > 0) {
		position = generateSpawnPoint(zoneName, x, y, minimumDistance, maximumDistance, 5.0, sphereRadius, forceSpawn, strict);

		if (position != Vector3(0, 0, 0))
			found = true;

		retries--;
	}

	if (position != Vector3(0, 0, 0)) {
		lua_newtable(L);
		lua_pushnumber(L, position.getX());
		lua_pushnumber(L, position.getZ());
		lua_pushnumber(L, position.getY());
		lua_rawseti(L, -4, 3);
		lua_rawseti(L, -3, 2);
		lua_rawseti(L, -2, 1);

		return 1;
	} else {
		// Infinity: console-only, no file trace — high volume when quests retry near boundaries
		Logger::console.info(true)
			<< "getSpawnPoint failed - Zone: " << zoneName
			<< " X: " << x << ", Y: " << y
			<< " Min: " << minimumDistance << " Max: " << maximumDistance;
		return 0;
	}
}

int DirectorManager::getSpawnArea(lua_State* L) {
    // getSpawnArea(zoneName, x, y, minDist, maxDist, areaSize, maxHeightDiff [, forceSpawn [, sphereRadius [, strict]]])
    // 7-10 arguments. strict=true disables water-aware radius expansion.
    int numberOfArguments = lua_gettop(L);
    if (numberOfArguments < 7 || numberOfArguments > 10) {
        printTraceError(L, "incorrect number of arguments passed to DirectorManager::getSpawnArea (expected 7-10)");
        ERROR_CODE = INCORRECT_ARGUMENTS;
        return 0;
    }

    // Validate and extract zoneName
    if (!lua_isstring(L, -numberOfArguments)) {
        printTraceError(L, "zoneName must be a string");
        ERROR_CODE = INCORRECT_ARGUMENTS;
        return 0;
    }
    String zoneName = lua_tostring(L, -numberOfArguments);

    // Validate and extract numeric arguments
    int baseIndex = -numberOfArguments + 1;
    if (!lua_isnumber(L, baseIndex) || !lua_isnumber(L, baseIndex + 1) ||
        !lua_isnumber(L, baseIndex + 2) || !lua_isnumber(L, baseIndex + 3) ||
        !lua_isnumber(L, baseIndex + 4) || !lua_isnumber(L, baseIndex + 5)) {
        printTraceError(L, "x, y, minimumDistance, maximumDistance, areaSize, and maximumHeightDifference must be numbers");
        ERROR_CODE = INCORRECT_ARGUMENTS;
        return 0;
    }

    // Infinity: Reworked these arguments. Explicitly convert double to float
    float x = static_cast<float>(lua_tonumber(L, baseIndex));
    float y = static_cast<float>(lua_tonumber(L, baseIndex + 1));
    float minimumDistance = static_cast<float>(lua_tonumber(L, baseIndex + 2));
    float maximumDistance = static_cast<float>(lua_tonumber(L, baseIndex + 3));
    float areaSize = static_cast<float>(lua_tonumber(L, baseIndex + 4));
    float maximumHeightDifference = static_cast<float>(lua_tonumber(L, baseIndex + 5));

    // Handle optional forceSpawn (arg 8), sphereRadius (arg 9), strict (arg 10)
    bool forceSpawn = (numberOfArguments >= 8) ? lua_toboolean(L, baseIndex + 6) : false;
    float sphereRadius = (numberOfArguments >= 9) ? static_cast<float>(lua_tonumber(L, baseIndex + 7)) : 10.0f;
    bool strict = (numberOfArguments >= 10) ? lua_toboolean(L, baseIndex + 8) : false;

    auto zoneServer = ServerCore::getZoneServer();
    if (zoneServer == nullptr) {
        String err = "ZoneServer is nullptr in DirectorManager::getSpawnArea";
        printTraceError(L, err);
        return 0;
    }

	auto zone = zoneServer->getZone(zoneName);

	if (zone == nullptr) {
		String err = "Zone is nullptr in DirectorManager::getSpawnArea using zone name: " + zoneName;
		printTraceError(L, err);
		return 0;
	}

	auto planetManager = zone->getPlanetManager();

	if (planetManager == nullptr) {
		return 0;
	}

	auto terrainManager = planetManager->getTerrainManager();

	if (terrainManager == nullptr) {
		return 0;
	}

	bool found = false;
	Vector3 position;
	int retries = 75;  //Infinity

	while (!found && retries > 0) {
		position = generateSpawnPoint(zoneName, x, y, minimumDistance, maximumDistance, areaSize + 5.0, areaSize + sphereRadius, forceSpawn, strict);

		int x0 = position.getX() - areaSize;
		int x1 = position.getX() + areaSize;
		int y0 = position.getY() - areaSize;
		int y1 = position.getY() + areaSize;

		found = terrainManager->getHighestHeightDifference(x0, y0, x1, y1) <= maximumHeightDifference;
		retries--;
	}

	if (found) {
		lua_newtable(L);
		lua_pushnumber(L, position.getX());
		lua_pushnumber(L, position.getZ());
		lua_pushnumber(L, position.getY());
		lua_rawseti(L, -4, 3);
		lua_rawseti(L, -3, 2);
		lua_rawseti(L, -2, 1);

		return 1;
	} else {
		// Infinity: console-only, no file trace — high volume when quests retry near boundaries
		Logger::console.info(true)
			<< "getSpawnArea failed - Zone: " << zoneName
			<< " Origin: (" << x << ", " << y << ")"
			<< " Area: " << areaSize << " Range: " << minimumDistance << "-" << maximumDistance;
		return 0;
	}
}

int DirectorManager::makeCreatureName(lua_State* L) {
	ZoneServer* zoneServer = ServerCore::getZoneServer();

	String name = "";
	if (zoneServer != nullptr) {
		NameManager* nameManager = zoneServer->getNameManager();
		name = nameManager->makeCreatureName();
	}

	lua_pushstring(L, name.toCharArray());
	return 1;
}


int DirectorManager::getGCWDiscount(lua_State* L){
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::getGCWDiscount";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	CreatureObject* creature = (CreatureObject*)lua_touserdata(L, -1);
	if (creature == nullptr) {
		lua_pushnumber(L, 1.0);
		return 0;
	}

	Zone* zone = creature->getZone();
	if (zone == nullptr) {
		lua_pushnumber(L, 1.0);
		return 0;
	}

	GCWManager* gcwMan = zone->getGCWManager();
	if (gcwMan == nullptr) {
		lua_pushnumber(L, 1.0);
		return 0;
	}

	lua_pushnumber(L, gcwMan->getGCWDiscount(creature));
	return 1;
}

int DirectorManager::getTerrainHeight(lua_State* L) {
	if (checkArgumentCount(L, 3) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::getTerrainHeight";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	float y = lua_tonumber(L, -1);
	float x = lua_tonumber(L, -2);
	CreatureObject* creatureObject = (CreatureObject*)lua_touserdata(L, -3);

	if (creatureObject == nullptr) {
		lua_pushnil(L);
		return 1;
	}

	auto zone = creatureObject->getZone();

	if (zone != nullptr) {
		lua_pushnumber(L, zone->getHeight(x, y));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int DirectorManager::checkArgumentCount(lua_State* L, int args) {
	int parameterCount = lua_gettop(L);

	if (parameterCount < args) {
		return 1;
	} else if (parameterCount > args)
		return 2;

	return 0;
}

int DirectorManager::awardSkill(lua_State* L) {
	if (checkArgumentCount(L, 2) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::awardSkill";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	CreatureObject* creature = (CreatureObject*)lua_touserdata(L, -2);
	String skillName = lua_tostring(L, -1);

	if(creature == nullptr)
		return 0;

	SkillManager::instance()->awardSkill(skillName, creature, true, true, true);

	return 0;
}

int DirectorManager::getCityRegionAt(lua_State* L) {
	if (checkArgumentCount(L, 3) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::getCityRegionAt";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String zoneid = lua_tostring(L, -3);
	float x = lua_tonumber(L, -2);
	float y = lua_tonumber(L, -1);

	auto zone =  ServerCore::getZoneServer()->getZone(zoneid);

	if (zone != nullptr) {
		PlanetManager* planetManager = zone->getPlanetManager();
		CityRegion* cityRegion = nullptr;

		if (planetManager != nullptr) {
			cityRegion = planetManager->getCityRegionAt(x, y);
		}

		if (cityRegion != nullptr) {
			lua_pushlightuserdata(L, cityRegion);
		} else {
			lua_pushnil(L);
		}
	} else {
		lua_pushnil(L);
	}

	return 1;
}

int DirectorManager::setDungeonTicketAttributes(lua_State* L) {
	if (checkArgumentCount(L, 4) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::setDungeonTicketAttributes";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	SceneObject* scene = (SceneObject*) lua_touserdata(L, -4);
	String ticketDepartPlanet = lua_tostring(L, -3);
	String ticketDepartPoint = lua_tostring(L, -2);
	String ticketArrivePoint = lua_tostring(L, -1);


	if (scene == nullptr)
		return 0;

	ManagedReference<TangibleObject*> tano = cast<TangibleObject*>(scene);

	if (tano == nullptr)
		return 0;

	TicketObject* tObj = tano.castTo<TicketObject*>();

	if (tObj == nullptr)
		return 0;

	tObj->setDeparturePlanet(ticketDepartPlanet);
	tObj->setDeparturePoint(ticketDepartPoint);
	tObj->setArrivalPoint(ticketArrivePoint);

	return 0;
}

int DirectorManager::setQuestStatus(lua_State* L) {
	if (checkArgumentCount(L, 2) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::setQuestStatus";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String valueString = lua_tostring(L, -1);
	String keyString = lua_tostring(L, -2);

	instance()->setQuestStatus(keyString, valueString);

	return 0;
}

int DirectorManager::getQuestStatus(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::getQuestStatus";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String keyString = lua_tostring(L, -1);

	String str = instance()->getQuestStatus(keyString);

	if (str == "")
		lua_pushnil(L);
	else
		lua_pushstring(L, str.toCharArray());

	return 1;
}

int DirectorManager::removeQuestStatus(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::removeQuestStatus";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String keyString = lua_tostring(L, -1);

	instance()->removeQuestStatus(keyString);

	return 0;
}

int DirectorManager::setCoaWinningFaction(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::setCoaWinningFaction";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	uint32 faction = lua_tointeger(L, -1);

	ManagedReference<PlayerManager*> playerManager = ServerCore::getZoneServer()->getPlayerManager();

	if (playerManager == nullptr)
		return 0;

	playerManager->setCoaWinningFaction(faction);

	return 0;
}

int DirectorManager::getControllingFaction(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::getControllingFaction";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String zoneName = lua_tostring(L, -1);

	Zone* zone = ServerCore::getZoneServer()->getZone(zoneName);
	if (zone == nullptr) {
		lua_pushinteger(L, 0);
		return 1;
	}

	GCWManager* gcwMan = zone->getGCWManager();
	if (gcwMan == nullptr) {
		lua_pushinteger(L, 0);
	} else {
		lua_pushinteger(L, gcwMan->getWinningFaction());
	}

	return 1;
}

int DirectorManager::getImperialScore(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::getImperialScore";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String zoneName = lua_tostring(L, -1);

	Zone* zone = ServerCore::getZoneServer()->getZone(zoneName);
	if (zone == nullptr) {
		lua_pushinteger(L, 0);
		return 1;
	}

	GCWManager* gcwMan = zone->getGCWManager();
	if (gcwMan == nullptr) {
		lua_pushinteger(L, 0);
	} else {
		lua_pushinteger(L, gcwMan->getImperialScore());
	}

	return 1;
}

int DirectorManager::getRebelScore(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::getRebelScore";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String zoneName = lua_tostring(L, -1);

	Zone* zone = ServerCore::getZoneServer()->getZone(zoneName);
	if (zone == nullptr) {
		lua_pushinteger(L, 0);
		return 1;
	}

	GCWManager* gcwMan = zone->getGCWManager();
	if (gcwMan == nullptr) {
		lua_pushinteger(L, 0);
	} else {
		lua_pushinteger(L, gcwMan->getRebelScore());
	}

	return 1;
}

int DirectorManager::getWinningFactionDifficultyScaling(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::getWinningFactionDifficultyScaling";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String zoneName = lua_tostring(L, -1);

	Zone* zone = ServerCore::getZoneServer()->getZone(zoneName);
	if (zone == nullptr) {
		lua_pushinteger(L, 0);
		return 1;
	}

	GCWManager* gcwMan = zone->getGCWManager();
	if (gcwMan == nullptr) {
		lua_pushinteger(L, 0);
	} else {
		lua_pushinteger(L, gcwMan->getWinningFactionDifficultyScaling());
	}

	return 1;
}

int DirectorManager::playClientEffectLoc(lua_State* L) {
	SceneObject* obj = nullptr;

	if (lua_isnumber(L, -7)) {
		uint64 objectID = lua_tointeger(L, -7);
		ZoneServer* zoneServer = ServerCore::getZoneServer();
		obj = zoneServer->getObject(objectID);
	} else {
		obj = (SceneObject*) lua_touserdata(L, -7);
	}

	String effect = lua_tostring(L, -6);
	String zone = lua_tostring(L, -5);
	float x = lua_tonumber(L, -4);
	float z = lua_tonumber(L, -3);
	float y = lua_tonumber(L, -2);
	int cell = lua_tonumber(L, -1);

	if (obj == nullptr)
		return 0;

	PlayClientEffectLoc* effectLoc = new PlayClientEffectLoc(effect, zone, x, z, y, cell);
	obj->broadcastMessage(effectLoc, true);

	return 0;
}

int DirectorManager::getPlayerQuestID(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::getPlayerQuestID";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	ManagedReference<PlayerManager*> playerManager = ServerCore::getZoneServer()->getPlayerManager();
	String questName = lua_tostring(L, -1);
	int questID = playerManager->getPlayerQuestID(questName);

	if (questID >= 0)
		lua_pushinteger(L, questID);
	else
		lua_pushnil(L);

	return 1;
}

int DirectorManager::getQuestInfo(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::getQuestInfo";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	int questID = 0;
	ManagedReference<PlayerManager*> playerManager = ServerCore::getZoneServer()->getPlayerManager();

	if (lua_isnumber(L, -1)) {
		questID = lua_tointeger(L, -1);
	} else {
		String questName = lua_tostring(L, -1);
		questID = playerManager->getPlayerQuestID(questName);
	}

	QuestInfo* questInfo = playerManager->getQuestInfo(questID);

	if (questInfo == nullptr)
		lua_pushnil(L);
	else
		lua_pushlightuserdata(L, questInfo);

	return 1;
}

int DirectorManager::getQuestVectorMap(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::getQuestVectorMap";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String keyString = lua_tostring(L, -1);

	Reference<QuestVectorMap*> questMap = instance()->getQuestVectorMap(keyString);

	if (questMap == nullptr)
		lua_pushnil(L);
	else
		lua_pushlightuserdata(L, questMap.get());

	return 1;
}

int DirectorManager::createQuestVectorMap(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::createQuestVectorMap";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String keyString = lua_tostring(L, -1);

	Reference<QuestVectorMap*> questMap = instance()->createQuestVectorMap(keyString);

	if (questMap == nullptr)
		lua_pushnil(L);
	else
		lua_pushlightuserdata(L, questMap);

	return 1;
}

int DirectorManager::removeQuestVectorMap(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::removeQuestVectorMap";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String keyString = lua_tostring(L, -1);

	instance()->removeQuestVectorMap(keyString);

	return 0;
}


QuestVectorMap* DirectorManager::createQuestVectorMap(const String& keyString) {
	Reference<QuestVectorMap*> questMap = questVectorMaps.get(keyString);

	if (questMap == nullptr) {
		questMap = new QuestVectorMap();
		questMap->setKey(keyString);
		questVectorMaps.put(keyString, questMap);

		ObjectManager::instance()->persistObject(questMap, 1, "questdata");
	}

	return questMap;
}

QuestVectorMap* DirectorManager::getQuestVectorMap(const String& keyString) {
	Reference<QuestVectorMap*> questMap = questVectorMaps.get(keyString);

	return questMap;
}

void DirectorManager::removeQuestVectorMap(const String& keyString) {
	Reference<QuestVectorMap*> questMap = nullptr;

	questMap = questVectorMaps.get(keyString);

	questVectorMaps.drop(keyString);

	if (questMap != nullptr)
		ObjectManager::instance()->destroyObjectFromDatabase(questMap->_getObjectID());
}

int DirectorManager::createNavMesh(lua_State *L) {
	if (checkArgumentCount(L, 6) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::createNavMesh";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}
	String name  = lua_tostring(L, -1);
	bool dynamic = lua_toboolean(L, -2);
	float radius = lua_tonumber(L, -3);
	float z = lua_tonumber(L, -4);
	float x = lua_tonumber(L, -5);
	String zoneName  = lua_tostring(L, -6);

	Zone* zone = ServerCore::getZoneServer()->getZone(zoneName);

	if (zone == nullptr) {
		instance()-> error("Zone == nullptr in DirectorManager::createNavMesh (" + zoneName + ")");
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	ManagedReference<NavArea*> navArea = ServerCore::getZoneServer()->createObject(STRING_HASHCODE("object/region_navmesh.iff"), "navareas", 0).castTo<NavArea*>();

	if (navArea == nullptr) {
		instance()->error("navArea creation failed in DirectorManager::createNavMesh (" + zoneName + ")");
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	if (name.length() == 0) {
		name = String::valueOf(navArea->getObjectID());
	}

	name = "screenplay_" + zoneName + "_" + name;

	Core::getTaskManager()->scheduleTask([=]{
		String str = name;
		Vector3 position = Vector3(x, 0, z);

		Locker locker(navArea);

		navArea->disableMeshUpdates(!dynamic);
		navArea->initializeNavArea(position, radius, zone, str);

		zone->transferObject(navArea, -1, false);
		zone->getPlanetManager()->addNavArea(str, navArea);
	}, "create_lua_navmesh", 1000);
	lua_pushlightuserdata(L, navArea);
	return 1;
}

int DirectorManager::destroyNavMesh(lua_State *L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::destroyNavMesh. Proper arguments is: (navmeshID)";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	uint64 navMeshID = lua_tointeger(L, -1);

	ZoneServer* zoneServer = ServerCore::getZoneServer();

	if (zoneServer == nullptr)
		return 0;

	ManagedReference<NavArea*> navMesh = zoneServer->getObject(navMeshID).castTo<NavArea*>();

	if (navMesh != nullptr) {
		Locker locker(navMesh);

		navMesh->destroyObjectFromWorld(true);

		if (navMesh->isPersistent())
			navMesh->destroyObjectFromDatabase(true);
	}

	return 0;
}

int DirectorManager::creatureTemplateExists(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::creatureTemplateExists";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String templateName = lua_tostring(L, -1);

	bool result = CreatureTemplateManager::instance()->getTemplate(templateName) != nullptr;

	lua_pushboolean(L, result);

	return 1;
}

int DirectorManager::printLuaError(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::printLuaError";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String error = lua_tostring(L, -1);

	printTraceError(L, error);

	return 0;
}

int DirectorManager::logLua(lua_State* L) {
	if (checkArgumentCount(L, 2) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::logLua";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	Lua* lua = instance()->getLuaInstance();

	if (lua == nullptr) {
		return 0;
	}

	/*	bin/scripts/utils/logger.lua
		LT_ERROR = 1
		LT_WARNING = 2
		LT_LOG = 3
		LT_INFO = 4
		LT_DEBUG = 5
		LT_TRACE = 6
	*/

	int logLevel = lua_tonumber(L, -2);
	String message = lua_tostring(L, -1);

	const int printTrace = 6;

	switch(logLevel) {
		case Logger::ERROR:
			lua->error() << message;
			break;
		case Logger::WARNING:
			lua->warning() << message;
			break;
		case Logger::LOG:
			lua->log() << message;
			break;
		case Logger::INFO:
			lua->info() << message;
			break;
		case Logger::DEBUG:
			lua->debug() << message;
			break;
		case printTrace:
			printTraceError(L, message);
			break;
		default:
			lua->info() << message;
			break;
	}

	return 0;
}

int DirectorManager::logLuaEvent(lua_State* L) {
	if (checkArgumentCount(L, 2) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::logLuaEvent";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	int level = lua_tonumber(L, -2);
	const char* message = lua_tostring(L, -1);
	const int printTrace = 6;

	switch(level) {
		case Logger::ERROR:
			getEventLogger().error() << message;
			break;
		case Logger::WARNING:
			getEventLogger().warning() << message;
			break;
		case Logger::LOG:
			getEventLogger().log() << message;
			break;
		case Logger::INFO:
			getEventLogger().info() << message;
			break;
		case Logger::DEBUG:
			getEventLogger().debug() << message;
			break;
		case printTrace:
			printTraceError(L, message);
			break;
		default:
			break;
	}

	return 0;
}

int DirectorManager::getSpawnPointInArea(lua_State* L) {
	if (checkArgumentCount(L, 4) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::getSpawnPointInArea";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String zoneName  = lua_tostring(L, -4);
	Zone* zone = ServerCore::getZoneServer()->getZone(zoneName);

	if (zone == nullptr) {
		instance()-> error("Zone == nullptr in DirectorManager::getSpawnPointInArea (" + zoneName + ")");
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	float x = lua_tonumber(L, -3);
	float y = lua_tonumber(L, -2);
	float radius = lua_tonumber(L, -1);

	Sphere sphere(Vector3(x, y, zone->getHeightNoCache(x, y)), radius);
	Vector3 result;

	for (int i = 0; i < SPAWNPOINTSEARCHATTEMPTS; i++) {
		if (PathFinderManager::instance()->getSpawnPointInArea(sphere, zone, result)) {
			lua_newtable(L);
			lua_pushnumber(L, result.getX());
			lua_pushnumber(L, result.getZ());
			lua_pushnumber(L, result.getY());
			lua_rawseti(L, -4, 3);
			lua_rawseti(L, -3, 2);
			lua_rawseti(L, -2, 1);

			return 1;
		}
	}

	instance()->debug() << "Unable to generate spawn point in DirectorManager::getSpawnPointInArea, x: " + String::valueOf(x) + ", y: " + String::valueOf(y) + ", zone: " + zoneName + ", radius: " + String::valueOf(radius);
	return 0;
}

int DirectorManager::getPlayerByName(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::getPlayerByName";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String playerName = lua_tostring(L, -1);

	ManagedReference<PlayerManager*> playerManager = ServerCore::getZoneServer()->getPlayerManager();

	CreatureObject* player = playerManager->getPlayer(playerName);

	if (player != nullptr) {
		lua_pushlightuserdata(L, player);
	} else {
		lua_pushnil(L);
	}

	return 1;
}

int DirectorManager::sendMail(lua_State* L) {
	if (checkArgumentCount(L, 4) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::sendMail";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String recipient = lua_tostring(L, -1);
	String body = lua_tostring(L, -2);
	String subject = lua_tostring(L, -3);
	String senderName = lua_tostring(L, -4);

	ManagedReference<ChatManager*> chatManager = ServerCore::getZoneServer()->getChatManager();

	if (chatManager != nullptr)
		chatManager->sendMail(senderName, subject, body, recipient);

	return 0;
}

int DirectorManager::sendMailToOnlinePlayers(lua_State* L) {
	if (checkArgumentCount(L, 3) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::sendMailToOnlinePlayers";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	ZoneServer* zoneServer = ServerCore::getZoneServer();

	if (zoneServer == nullptr)
		return 0;

	PlayerManager* playerManager = zoneServer->getPlayerManager();
	ManagedReference<ChatManager*> chatManager = zoneServer->getChatManager();

	if (playerManager == nullptr || chatManager == nullptr)
		return 0;

	String body = lua_tostring(L, -1);
	String subject = lua_tostring(L, -2);
	String senderName = lua_tostring(L, -3);

	auto playerList = playerManager->getOnlinePlayerList();
	auto playername = "";

	for (int i = 0; i < playerList.size(); i++) {
		auto playerID = playerList.get(i);
		auto playerName = playerManager->getPlayerName(playerID);

		chatManager->sendMail(senderName, subject, body, playerName);
	}

	return 0;
}

int DirectorManager::spawnTheaterObject(lua_State* L) {
	int numberOfArguments = lua_gettop(L);
	if (numberOfArguments != 5) {
		String err = "incorrect number of arguments passed to DirectorManager::spawnTheaterObject";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	bool flatten = lua_toboolean(L, -1);
	float y = lua_tonumber(L, -2);
	float z = lua_tonumber(L, -3);
	float x = lua_tonumber(L, -4);
	String zoneID = lua_tostring(L, -5);

	ZoneServer* zoneServer = ServerCore::getZoneServer();
	Zone* zone = zoneServer->getZone(zoneID);

	if (zone == nullptr) {
		lua_pushnil(L);
		return 1;
	}

	ManagedReference<SceneObject*> object = zoneServer->createObject(STRING_HASHCODE("object/intangible/theater/base_theater.iff"), 0);

	if (object != nullptr && object->isTheaterObject()) {
		TheaterObject* theater = object.castTo<TheaterObject*>();

		if (theater == nullptr) {
			lua_pushnil(L);
			return 1;
		}

		Locker locker(theater);

		theater->initializePosition(x, z, y);

		if (flatten)
			theater->setShouldFlatten(true);

		Locker zoneLocker(zone);

		zone->transferObject(theater, -1, true);

		theater->_setUpdated(true); //mark updated so the GC doesnt delete it while in LUA

		lua_pushlightuserdata(L, theater);
	} else {
		lua_pushnil(L);
	}

	return 1;
}

int DirectorManager::getSchematicItemName(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::getSchematicItemName";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String itemName = "";
	String templatePath = Lua::getStringParameter(L);

	DraftSchematic* schematicTemplate = SchematicMap::instance()->get(templatePath.hashCode());

	if (schematicTemplate != nullptr)
		itemName = schematicTemplate->getCustomName();

	lua_pushstring(L, itemName.toCharArray());

	return 1;
}

int DirectorManager::getBadgeListByType(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::getBadgeListByType";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String type = Lua::getStringParameter(L);

	lua_newtable(L);

	int count = 0;

	const auto badgeList = BadgeList::instance()->getMap();

	for (int i = 0; i < badgeList->size(); i++) {
		const Badge* badge = badgeList->get(i);

		if (badge->getTypeString() == type) {
			count++;
			lua_pushinteger(L, badge->getIndex());
			lua_rawseti(L, -2, count);
		}
	}

	return 1;
}

int DirectorManager::getGalaxyName(lua_State* L) {
	ZoneServer* zoneServer = ServerCore::getZoneServer();

	if (zoneServer == nullptr) {
		return 0;
	}

	lua_pushstring(L, zoneServer->getGalaxyName().toCharArray());

	return 1;
}

int DirectorManager::getQuestTasks(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::getQuestTasks";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	int questCrc = lua_tointeger(L, -1);

	ZoneServer* zoneServer = ServerCore::getZoneServer();

	if (zoneServer == nullptr) {
		return 0;
	}

	PlayerManager* playerManager = zoneServer->getPlayerManager();

	if (playerManager == nullptr) {
		return 0;
	}

	Reference<QuestTasks*> questTasks = playerManager->getQuestTasks(questCrc);

	if (questTasks) {
		lua_pushlightuserdata(L, questTasks.get());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int DirectorManager::broadcastToGalaxy(lua_State* L) {
	if (checkArgumentCount(L, 2) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::broadcastToGalaxy";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	CreatureObject* creature = (CreatureObject*)lua_touserdata(L, -2);
	String message = Lua::getStringParameter(L, -1);

	ZoneServer* zoneServer = ServerCore::getZoneServer();

	if (zoneServer == nullptr) {
		return 0;
	}

	ChatManager* chatManager = zoneServer->getChatManager();

	if (chatManager == nullptr)
		return 0;

	chatManager->broadcastGalaxy(creature, message);

	return 1;
}

int DirectorManager::getWorldFloor(lua_State* L) {
	if (checkArgumentCount(L, 3) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::getWorldFloor";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	ZoneServer* zoneServer = ServerCore::getZoneServer();

	if (zoneServer == nullptr)
		return 0;

	float x = lua_tonumber(L, -3);
	float y = lua_tonumber(L, -2);
	String zoneName = lua_tostring(L, -1);

	Zone* zone = zoneServer->getZone(zoneName);

	if (zone == nullptr)
		return 0;

	float z = CollisionManager::getWorldFloorCollision(x, y, zone, false);

	lua_pushnumber(L, z);

	return 1;
}

int DirectorManager::useCovertOvert(lua_State* L) {
	bool result = ConfigManager::instance()->useCovertOvertSystem();

	lua_pushboolean(L, result);

	return 1;
}

// Infinity:  Custom Hooks below

void DirectorManager::printErrorLog(lua_State* L, const String& logMessage) {
	Lua* lua = instance()->getLuaInstance();
	lua->error(logMessage);
}

//Infinity: custom
int DirectorManager::printLuaLog(lua_State* L) {  //lua.log without all the traceback nonsense
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::printLuaLog";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String logMessage = lua_tostring(L, -1);

	printErrorLog(L, logMessage);

	return 0;
}

//Infinity: custom
int DirectorManager::logCore3(lua_State* L) {  //Core3 option of the logLua hooks

	if (checkArgumentCount(L, 2) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::logCore3";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	/*	bin/scripts/utils/logger.lua
		LT_ERROR = 1
		LT_WARNING = 2
		LT_LOG = 3
		LT_INFO = 4
		LT_DEBUG = 5
		LT_TRACE = 6
	*/

	int logLevel = lua_tonumber(L, -2);
	String message = lua_tostring(L, -1);

	switch(logLevel) {
		case Logger::ERROR:
			Logger::console.error("DirectorManager: " + message);
			break;
		case Logger::WARNING:
			Logger::console.warning("DirectorManager: " + message);
			break;
		case Logger::LOG:
			Logger::console.log("DirectorManager: " + message);
			break;
		case Logger::INFO:
			Logger::console.info("DirectorManager: " + message, true);
			break;
		case Logger::DEBUG:
			Logger::console.debug("DirectorManager: " + message);
			break;
		default:
			Logger::console.info("DirectorManager: " + message);
			break;
	}

	return 0;
}

int DirectorManager::spawnActiveAreaRect(lua_State* L) {
	int numberOfArguments = lua_gettop(L);
	if (numberOfArguments != 7) {
		String err = "incorrect number of arguments passed to DirectorManager::spawnActiveAreaRect";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	uint64 cellID = lua_tointeger(L, -1);
	float y2 = lua_tonumber(L, -2);
	float x2 = lua_tonumber(L, -3);
	float y1 = lua_tonumber(L, -4);
	float x1 = lua_tonumber(L, -5);
	String script = lua_tostring(L, -6);
	String zoneID = lua_tostring(L, -7);

	ZoneServer* zoneServer = ServerCore::getZoneServer();
	Zone* zone = zoneServer->getZone(zoneID);

	if (zone == nullptr) {
		lua_pushnil(L);
		return 1;
	}

	ManagedReference<SceneObject*> object = zoneServer->createObject(script.hashCode(), 0);

	if (object != nullptr && object->isActiveArea()) {
		ActiveArea* area = object.castTo<ActiveArea*>();

		if (area == nullptr) {
			lua_pushnil(L);
			return 1;
		}

		float centerX = x1 + ((x2 - x1) / 2);
		float centerY = y1 + ((y2 - y1) / 2);

		float z = zone->getHeight(centerX, centerY);

		Locker locker(area);
		area->initializePosition(centerX, z, centerY);
		
		ManagedReference<RectangularAreaShape*> rectangularAreaShape  = new RectangularAreaShape();
		Locker shapeLocker(rectangularAreaShape);
		rectangularAreaShape->setDimensions(x1, y1, x2, y2);
		rectangularAreaShape->setAreaCenter(centerX, centerY);

		area->setAreaShape(rectangularAreaShape);

		Reference<SceneObject*> cellParent = nullptr;
		if (cellID != 0) {
			cellParent = zoneServer->getObject(cellID);

			if (cellParent == nullptr || !cellParent->isCellObject()) {
				cellID = 0;
			}
		}

		area->setCellObjectID(cellID);

		Locker zoneLocker(zone);

		zone->transferObject(area, -1, true);

		area->_setUpdated(true); //mark updated so the GC doesnt delete it while in LUA

		lua_pushlightuserdata(L, area);
	} else {
		lua_pushnil(L);
	}

	return 1;
}

int DirectorManager::generateShipDeed(lua_State* L) {
	if (checkArgumentCount(L, 3) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::generateShipDeed";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	CreatureObject* player = (CreatureObject*)lua_touserdata(L, -3);
	ShipChassisComponent* chassisBlueprint = (ShipChassisComponent*)lua_touserdata(L, -2);
	CreatureObject* chassisDealer = (CreatureObject*)lua_touserdata(L, -1);

	if (player == nullptr || chassisBlueprint == nullptr || chassisDealer == nullptr) {
		lua_pushboolean(L, false);
		return 1;
	}

	auto shipManager = ShipManager::instance();

	if (shipManager != nullptr && shipManager->createDeedFromChassis(player, chassisBlueprint, chassisDealer)) {
		lua_pushboolean(L, true);
		return 1;
	}

	lua_pushboolean(L, false);

	return 1;
}

int DirectorManager::getTerrainHeightZone(lua_State* L) {
	if (checkArgumentCount(L, 3) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::getTerrainHeightZone";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	float y = lua_tonumber(L, -1);
	float x = lua_tonumber(L, -2);
	String zoneName = lua_tostring(L, -3);

	ZoneServer* zoneServer = ServerCore::getZoneServer();
	if (zoneServer == nullptr) {
		lua_pushnil(L);
		return 1;
	}

	auto zone = zoneServer->getZone(zoneName);
	if (zone == nullptr) {
		lua_pushnil(L);
		return 1;
	}

	lua_pushnumber(L, zone->getHeight(x, y));
	return 1;
}

int DirectorManager::sellSpaceLoot(lua_State* L) {
	if (checkArgumentCount(L, 3) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::sellSpaceLoot";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	CreatureObject* player = (CreatureObject*)lua_touserdata(L, -3);
	ShipComponent* lootComponent = (ShipComponent*)lua_touserdata(L, -2);
	CreatureObject* chassisDealer = (CreatureObject*)lua_touserdata(L, -1);

	if (player == nullptr || lootComponent == nullptr || chassisDealer == nullptr) {
		return 0;
	}

	Locker lock(player);
	Locker clock(lootComponent, player);

	TransactionLog trx(chassisDealer, player, lootComponent, TrxCode::SPACELOOTSOLD, true);

	int rELevel = lootComponent->getReverseEngineeringLevel();

	StringIdChatParameter soldMsg("@space/space_loot:sold_item");
	StringBuffer itemName;
	itemName << "@space/space_item:" << lootComponent->getObjectName()->getStringID();

	int itemValue = 1000;

	if (rELevel < 10) {
		itemValue *= rELevel;
	} else {
		itemValue = 500;
	}

	soldMsg.setTO(itemName.toString());
	soldMsg.setDI(itemValue);

	// Send player sold message
	player->sendSystemMessage(soldMsg);

	trx.addState("sellValue", itemValue);

	// Give player credits to bank
	player->addBankCredits(itemValue, true);

	// Destroy the loot component
	lootComponent->destroyObjectFromWorld(true);
	lootComponent->destroyObjectFromDatabase(true);

	trx.commit();

	return 0;
}

int DirectorManager::isJtlEnabled(lua_State* L) {
	bool result = ConfigManager::instance()->isJtlEnabled();

	lua_pushboolean(L, result);

	return 1;
}

int DirectorManager::getZoneTime(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::getZoneTime";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String zoneName = lua_tostring(L, -1);

	auto zone = ServerCore::getZoneServer()->getZone(zoneName);

	if (zone == nullptr) {
		lua_pushnil(L);
		return 1;
	}

	auto planetManager = zone->getPlanetManager();

	if (planetManager == nullptr) {
		lua_pushnil(L);
		return 1;
	}

	auto terrainManager = planetManager->getTerrainManager();

	if (terrainManager == nullptr) {
		lua_pushnil(L);
		return 1;
	}

	ProceduralTerrainAppearance* pta = terrainManager->getProceduralTerrainAppearance();
	if (pta != nullptr) {
		float timeCycle = pta->getTimeCycle();
		float localTime = zone->getGalacticTime() / (timeCycle / 24 / 60 / 60);
		lua_pushnumber(L, localTime);
	} else {
		lua_pushnil(L);
	}

	return 1;
}

int DirectorManager::withCommas(lua_State* L) {

	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::withCommas";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	int number = lua_tointeger(L, -1);

	String formattedNumber = String::withCommas(number);

	lua_pushstring(L, formattedNumber.toCharArray());
 
	return 1;
}

int DirectorManager::broadcastToPvp(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::broadcastToGalaxy";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String messageString = Lua::getStringParameter(L, -1);

	auto zoneServer = ServerCore::getZoneServer();
	if (zoneServer == nullptr) {
		return 0;
	}

	if (ConfigManager::instance()->isPvpBroadcastChannelEnabled()) {

		auto chatManager = zoneServer->getChatManager();
		if (chatManager != nullptr) {

			auto pvpBroadcastRoom = chatManager->getPvpBroadcastRoom();
			if (pvpBroadcastRoom != nullptr) {
				UnicodeString message(messageString);
				UnicodeString formattedMsg(chatManager->formatMessage(message));
				BaseMessage* msg = new ChatRoomMessage("", zoneServer->getGalaxyName(), formattedMsg, pvpBroadcastRoom->getRoomID());
				pvpBroadcastRoom->broadcastMessage(msg);
			}
		}
	}

	return 1;
}

void DirectorManager::reportSharedMemory() {

#ifndef WITH_STM
	DirectorManager::instance()->rlock();
#endif

	StringBuffer msg;
	msg << endl << "------------ Director Manager Shared Memory ------------" << endl;

	msg << " hashTable size: " << "\t\t" << DirectorManager::instance()->sharedMemory->getHashTableSize() << endl;

	msg << " stringTable size: " << "\t\t" << DirectorManager::instance()->sharedMemory->getStringTableSize() << endl;

	msg << " vector3Table size: " << "\t\t" <<  DirectorManager::instance()->sharedMemory->getVector3TableSize() << endl;

	msg << " stringVectorTable size: " "\t" << DirectorManager::instance()->sharedMemory->getStringVectorTableSize() << endl;

	msg << " total sharedMemory size: " "\t" << DirectorManager::instance()->sharedMemory->getSize() << endl;

	Logger::console.error(msg.toString());

#ifndef WITH_STM
	DirectorManager::instance()->runlock();
#endif

}

int DirectorManager::getMasterScreenPlayVersion(lua_State* L) {

	uint32 version = DirectorManager::instance()->masterScreenPlayVersion.get();

	lua_pushinteger(L, version);

	return 1;
}

int DirectorManager::getPlayersInRangeOfPos(lua_State* L) {
	if (checkArgumentCount(L, 4) != 0) {
		String err = "DirectorManager::getPlayersInRangeOfPos - incorrect number of arguments";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		lua_pushnil(L);
		return 1;
	}

	String zoneName = lua_tostring(L, -4);
	float x = lua_tonumber(L, -3);
	float y = lua_tonumber(L, -2);
	float range = lua_tonumber(L, -1);

	auto zoneServer = ServerCore::getZoneServer();

	if (zoneServer == nullptr) {
		lua_pushnil(L);
		return 1;
	}

	auto zone = zoneServer->getZone(zoneName);

	if (zone == nullptr) {
		lua_pushnil(L);
		return 1;
	}

	lua_newtable(L);

	SortedVector<ManagedReference<TreeEntry*>> playerObjects;
	zone->getInRangePlayers(x, 0, y, range, &playerObjects);
	int numPlayers = 0;

	for (int i = 0; i < playerObjects.size(); ++i) {
		SceneObject* object = cast<SceneObject*>(playerObjects.get(i).get());

		if (object == nullptr || !object->isPlayerCreature()) {
			continue;
		}

		numPlayers++;
		lua_pushlightuserdata(L, object);
		lua_rawseti(L, -2, numPlayers);
	}

	return 1;
}

int DirectorManager::getCreaturesInRangeOfPos(lua_State* L) {
	if (checkArgumentCount(L, 4) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::getCreaturesInRangeOfPos";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
        lua_pushnil(L);
		return 1;
	}

	String zoneName = lua_tostring(L, -4);
	float x = lua_tonumber(L, -3);
	float y = lua_tonumber(L, -2);
	float range = lua_tonumber(L, -1);

	auto zoneServer = ServerCore::getZoneServer();

	if (zoneServer == nullptr) {
		lua_pushnil(L);
		return 1;
	}

	auto zone = zoneServer->getZone(zoneName);

	if (zone == nullptr) {
		lua_pushnil(L);
		return 1;
	}

	lua_newtable(L);

	SortedVector<ManagedReference<TreeEntry*>> closeObjects;
	zone->getInRangeObjects(x, 0, y, range, &closeObjects, true);
	int numCreatures = 0;

	for (int i = 0; i < closeObjects.size(); ++i) {
		SceneObject* object = cast<SceneObject*>(closeObjects.get(i).get());

		if (object == nullptr || !object->isCreatureObject())
			continue;

		CreatureObject* creo = object->asCreatureObject();

		if (creo == nullptr || creo->isInvisible())
			continue;

		numCreatures++;
		lua_pushlightuserdata(L, object);
		lua_rawseti(L, -2, numCreatures);
	}

	return 1;
}

Vector<String> DirectorManager::getQuestStatuses() const {

    Vector<String> keyVector;

	for (int i = questStatuses.size(); i > 0; --i) {
		const String key = questStatuses.getKey(i - 1);
   		keyVector.add(key);
	}

	return keyVector;
}

int DirectorManager::getQuestStatuses(lua_State *L) {

    lua_newtable(L);  //Create new Lua table and push it to the stack

	auto zoneServer = ServerCore::getZoneServer();
	if (zoneServer == nullptr) {
		return 1;
	}

    String galaxyName = zoneServer->getGalaxyName();

	if (galaxyName.beginsWith("SWG Infinity")) {
        Logger::console.error("DirectorManager::getQuestStatuses - you cannot run this hook on live server or TC.");
		return 1;
	}

    Vector<String> keyVector = instance()->getQuestStatuses();

    for (int i = keyVector.size(); i > 0; --i) {
        const String key = keyVector.get(i-1);
        lua_pushstring(L, key.toCharArray());
    }

    for (int j = keyVector.size(); j > 0; --j) {
        int rawSet = (j + 1) * -1;
        lua_rawseti(L, rawSet, j);
    }  

	return 1;
}

int DirectorManager::isValidName(lua_State* L) {
	bool result = false;

	String name = lua_tostring(L, -1);

	if (!name.isEmpty()) {
		auto zoneServer = ServerCore::getZoneServer();

		if (zoneServer != nullptr) {
			auto nameManager = zoneServer->getNameManager();

			if (nameManager != nullptr) {
				result = nameManager->checkNamingFilter(name) == NameManagerResult::ACCEPTED;
			}
		}
	}

	lua_pushboolean(L, result);

	return 1;
}

int DirectorManager::addCreatureToPlayerGroup(lua_State* L) {
	// Lua function: addCreatureToPlayerGroup(player, creature)
	// Adds a creature to a player's group
	// If player is ungrouped, creates a new group
	// Returns true if successful, false otherwise

	CreatureObject* creature = (CreatureObject*) lua_touserdata(L, -1);
	CreatureObject* player = (CreatureObject*) lua_touserdata(L, -2);

	if (player == nullptr || creature == nullptr) {
		lua_pushboolean(L, false);
		return 1;
	}

	bool result = GroupManager::instance()->addCreatureToPlayerGroup(player, creature);
	lua_pushboolean(L, result);

	return 1;
}

void DirectorManager::writeStringSharedMemory(const String& key, const String& data) {

    if (key.isEmpty()) {
        return;
    }

#ifndef WITH_STM
	DirectorManager::instance()->wlock();
#endif

	DirectorManager::instance()->sharedMemory->putString(key, data);

#ifndef WITH_STM
	DirectorManager::instance()->unlock();
#endif

}

void DirectorManager::deleteSharedMemory(const String& key) {
	if (key.isEmpty()) {
		return;
	}

#ifndef WITH_STM
	DirectorManager::instance()->wlock();
#endif

	DirectorManager::instance()->sharedMemory->remove(key);

#ifndef WITH_STM
	DirectorManager::instance()->unlock();
#endif
}

int DirectorManager::grantStarterShip(lua_State* L) {
	if (checkArgumentCount(L, 2) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::grantStarterShip";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	CreatureObject* player = (CreatureObject*)lua_touserdata(L, -2);
	String factionName = lua_tostring(L, -1);

	if (player == nullptr || factionName.isEmpty()) {
		return 0;
	}

	String generatedObjectTemplate = "";

	if (factionName == "neutral") {
		generatedObjectTemplate = "object/ship/player/player_prototype_hutt_light.iff";
	} else if (factionName == "rebel") {
		generatedObjectTemplate = "object/ship/player/player_prototype_z95.iff";
	} else if (factionName == "imperial") {
		generatedObjectTemplate = "object/ship/player/player_prototype_tiefighter.iff";
	} else {
		return 0;
	}

	auto zoneServer = player->getZoneServer();

	if (zoneServer == nullptr) {
		return 0;
	}

	auto zone = player->getZone();

	if (zone == nullptr) {
		return 0;
	}

	auto planetManager = zone->getPlanetManager();

	if (planetManager == nullptr) {
		return 0;
	}

	auto travelPoint = planetManager->getNearestPlanetTravelPoint(player->getWorldPosition(), 16000.f, false, true);

	if (travelPoint == nullptr) {
		return 0;
	}

	Locker lock(player);

	ManagedReference<ShipObject*> ship = ShipManager::instance()->createPlayerShip(player, generatedObjectTemplate, "", true);

	if (ship == nullptr) {
		player->error() << "Failed to generate ship object from template: " << generatedObjectTemplate;
		return 0;
	}

	// Player is locked, cross lock the ship to the player
	Locker slocker(ship, player);

	ship->setComponentMass(Components::REACTOR, 1500.f);
	ship->setReactorGenerationRate(8000.f, false);
	ship->setComponentMass(Components::ENGINE, 1500.f);
	ship->setComponentMass(Components::SHIELD0, 1500.f);
	ship->setComponentMass(Components::ARMOR0, 1500.f);
	ship->setComponentMass(Components::ARMOR1, 1500.f);
	ship->setComponentMass(Components::CAPACITOR, 1500.f);
	ship->setCapacitorMaxEnergy(2000.f, false);
	ship->setComponentMass(Components::WEAPON_START, 1500.f);

	ManagedReference<ShipControlDevice*> shipControlDevice = cast<ShipControlDevice*>(zoneServer->getObject(ship->getControlDeviceID()).get());

	if (shipControlDevice == nullptr) {
		ship->destroyObjectFromDatabase(true);
		ship->destroyObjectFromWorld(true);

		player->error() << "grantStarterShip - null control device: " << generatedObjectTemplate;

		return 0;
	}

	Locker deviceLock(shipControlDevice, player);

	shipControlDevice->setParkingLocation(travelPoint->getPointName());

	return 0;
}

int DirectorManager::drawClientPath(lua_State* L) {
	if (checkArgumentCount(L, 7) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::drawClientPath";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	SceneObject* obj = (SceneObject*) lua_touserdata(L, -7);

	float x1 = lua_tonumber(L, -6);
	float z1 = lua_tonumber(L, -5);
	float y1 = lua_tonumber(L, -4);
	float x2 = lua_tonumber(L, -3);
	float z2 = lua_tonumber(L, -2);
	float y2 = lua_tonumber(L, -1);

	CreateClientPathMessage* pathMessage = new CreateClientPathMessage();

	pathMessage->addCoordinate(x1, z1, y1);
	pathMessage->addCoordinate(x2, z2, y2);

	obj->broadcastMessage(pathMessage, true);

	return 0;
}

// Infinity: Spawn a mobile as a baby creature
int DirectorManager::spawnMobileAsBaby(lua_State* L) {
	int numberOfArguments = lua_gettop(L);
	if (numberOfArguments != 8 && numberOfArguments != 9) {
		String err = "incorrect number of arguments passed to DirectorManager::spawnMobileAsBaby";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	bool randomRespawn = false;
	uint64 parentID;
	float x, y, z, heading;
	int respawnTimer;
	String mobile, zoneid;

	if (numberOfArguments == 8) {
		parentID = lua_tointeger(L, -1);
		heading = lua_tonumber(L, -2);
		y = lua_tonumber(L, -3);
		z = lua_tonumber(L, -4);
		x = lua_tonumber(L, -5);
		respawnTimer = lua_tointeger(L, -6);
		if (!lua_isstring(L, -7)) {
			String err = "invalid template string passed to DirectorManager::spawnMobileAsBaby";
			printTraceError(L, err);
			ERROR_CODE = INCORRECT_ARGUMENTS;
			return 0;
		}
		mobile = lua_tostring(L, -7);
		zoneid = lua_tostring(L, -8);
	} else {
		randomRespawn = lua_toboolean(L, -1);
		parentID = lua_tointeger(L, -2);
		heading = lua_tonumber(L, -3);
		y = lua_tonumber(L, -4);
		z = lua_tonumber(L, -5);
		x = lua_tonumber(L, -6);
		respawnTimer = lua_tointeger(L, -7);
		if (!lua_isstring(L, -8)) {
			String err = "invalid template string passed to DirectorManager::spawnMobileAsBaby";
			printTraceError(L, err);
			ERROR_CODE = INCORRECT_ARGUMENTS;
			return 0;
		}
		mobile = lua_tostring(L, -8);
		zoneid = lua_tostring(L, -9);
	}

	ZoneServer* zoneServer = ServerCore::getZoneServer();
	Zone* zone = zoneServer->getZone(zoneid);

	if (zone == nullptr) {
		lua_pushnil(L);
		return 1;
	}

	CreatureManager* creatureManager = zone->getCreatureManager();
	CreatureObject* creature = creatureManager->spawnCreatureAsBaby(mobile.hashCode(), x, z, y, parentID, heading);

	if (creature == nullptr) {
		String err = "could not spawn mobile with template name: " + mobile;
		printTraceError(L, err);
		lua_pushnil(L);
	} else {
		Locker locker(creature);

		if (creature->isAiAgent()) {
			AiAgent* ai = cast<AiAgent*>(creature);
			ai->setRespawnTimer(respawnTimer);

			if (randomRespawn) {
				ai->setRandomRespawn(true);
			}

			ai->addCreatureFlag(ObjectFlag::BABYONLY);  //Infinity:  Set BABYONLY flag
			ai->removeCreatureFlag(ObjectFlag::ADULTONLY);  //Infinity:  Remove ADULTONLY flag if present
			ai->setAITemplate();
		}

		creature->_setUpdated(true); //mark updated so the GC doesnt delete it while in LUA
		lua_pushlightuserdata(L, creature);
	}

	return 1;
}

// Infinity: Return current galactic time (seconds) for Lua scripts
int DirectorManager::getGalacticTime(lua_State* L) {
	Zone* zone = nullptr;

	//Infinity:  Let's check 3 zones, if none of them are up, return nil
	zone = ServerCore::getZoneServer()->getZone("naboo");
	if (zone == nullptr) {
		zone = ServerCore::getZoneServer()->getZone("corellia");
	}
	if (zone == nullptr) {
		zone = ServerCore::getZoneServer()->getZone("tutorial");
	}

	if (zone == nullptr) {
		lua_pushnil(L);
	} else {
		lua_pushinteger(L, zone->getGalacticTime());
	}

	return 1;
}

// ============================================================================
// DEATH WATCH MANAGER BINDINGS
// ============================================================================

/**
 * Set Death Watch war state
 * Lua: setDwWarState(warType, config)
 * warType: 0=peace, 1=free-for-all, 2=2v2, 3=1v3, 4=four-way
 * config: configuration index (1-based for 2v2/1v3, 0 otherwise)
 * Returns: true if successful
 */
int DirectorManager::setDwWarState(lua_State* L) {
	if (checkArgumentCount(L, 2) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::setDwWarState";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		lua_pushboolean(L, false);
		return 1;
	}

	int config = lua_tointeger(L, -1);
	int warType = lua_tointeger(L, -2);

	// Validate war type range
	if (warType < 0 || warType > 4) {
		lua_pushboolean(L, false);
		return 1;
	}

	// Write to shared memory (C++ becomes source of truth)
	DirectorManager* dm = DirectorManager::instance();

#ifndef WITH_STM
	dm->wlock();
#endif

	dm->sharedMemory->put("dw:war:type", (uint64)warType);
	dm->sharedMemory->put("dw:war:config", (uint64)config);
	dm->sharedMemory->put("dw:war:initialized", (uint64)1);  // Mark as initialized

#ifndef WITH_STM
	dm->unlock();
#endif

	// Log the change
	StringBuffer logMsg;
	logMsg << "Death Watch war state updated: type=" << warType << ", config=" << config;
	Logger::console.info(logMsg.toString(), true);

	lua_pushboolean(L, true);
	return 1;
}

/**
 * Get Death Watch war state
 * Lua: warType, config = getDwWarState()
 * Returns: warType, config
 */
int DirectorManager::getDwWarState(lua_State* L) {
	DirectorManager* dm = DirectorManager::instance();

	uint64 warType = dm->readSharedMemory("dw:war:type");
	uint64 config = dm->readSharedMemory("dw:war:config");

	lua_pushinteger(L, (int)warType);
	lua_pushinteger(L, (int)config);

	return 2;
}

/**
 * Check if two Death Watch factions are currently at war
 * Lua: isAtWar = areDwFactionsAtWar(faction1, faction2)
 * faction1, faction2: Faction IDs (1=Xon, 2=Sun, 3=Krr, 4=Zan)
 * Returns: true if factions are at war
 */
int DirectorManager::areDwFactionsAtWar(lua_State* L) {
	if (checkArgumentCount(L, 2) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::areDwFactionsAtWar";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		lua_pushboolean(L, false);
		return 1;
	}

	int faction2 = lua_tointeger(L, -1);
	int faction1 = lua_tointeger(L, -2);

	auto zoneServer = ServerCore::getZoneServer();
	if (zoneServer == nullptr) {
		lua_pushboolean(L, false);
		return 1;
	}

	DeathWatchManager* dwManager = zoneServer->getDeathWatchManager();
	if (dwManager == nullptr) {
		lua_pushboolean(L, false);
		return 1;
	}

	bool atWar = dwManager->areFactionsAtWar((byte)faction1, (byte)faction2);
	lua_pushboolean(L, atWar);

	return 1;
}

/**
 * Set the Death Watch kill switch state
 * Lua: setDwKillSwitch(active)
 * active: true to disable Death Watch, false to enable
 */
int DirectorManager::setDwKillSwitch(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::setDwKillSwitch";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	bool active = lua_toboolean(L, -1);

	auto zoneServer = ServerCore::getZoneServer();
	if (zoneServer == nullptr) {
		return 0;
	}

	DeathWatchManager* dwManager = zoneServer->getDeathWatchManager();
	if (dwManager == nullptr) {
		return 0;
	}

	dwManager->setKillSwitch(active);

	return 0;
}

/**
 * Check if the Death Watch kill switch is active
 * Lua: isDwKillSwitchActive()
 * Returns: true if Death Watch is disabled (kill switch active)
 */
int DirectorManager::isDwKillSwitchActive(lua_State* L) {
	auto zoneServer = ServerCore::getZoneServer();
	if (zoneServer == nullptr) {
		lua_pushboolean(L, false);
		return 1;
	}

	DeathWatchManager* dwManager = zoneServer->getDeathWatchManager();
	if (dwManager == nullptr) {
		lua_pushboolean(L, false);
		return 1;
	}

	lua_pushboolean(L, dwManager->isKillSwitchActive());

	return 1;
}

/**
 * Set a Death Watch jetpack as invulnerable for a duration
 * Lua: setDwJetpackInvulnerable(vehicleID, durationMs)
 * vehicleID: Object ID of the jetpack
 * durationMs: Duration in milliseconds
 */
int DirectorManager::setDwJetpackInvulnerable(lua_State* L) {
	if (checkArgumentCount(L, 2) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::setDwJetpackInvulnerable";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	int durationMs = lua_tointeger(L, -1);
	uint64 vehicleID = lua_tointeger(L, -2);

	auto zoneServer = ServerCore::getZoneServer();
	if (zoneServer == nullptr) {
		return 0;
	}

	DeathWatchManager* dwManager = zoneServer->getDeathWatchManager();
	if (dwManager == nullptr) {
		return 0;
	}

	dwManager->setJetpackInvulnerable(vehicleID, durationMs);

	return 0;
}

/**
 * Check if a Death Watch jetpack is currently invulnerable
 * Lua: isInvuln = isDwJetpackInvulnerable(vehicleID)
 * vehicleID: Object ID of the jetpack
 * Returns: true if jetpack is invulnerable
 */
int DirectorManager::isDwJetpackInvulnerable(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::isDwJetpackInvulnerable";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		lua_pushboolean(L, false);
		return 1;
	}

	uint64 vehicleID = lua_tointeger(L, -1);

	auto zoneServer = ServerCore::getZoneServer();
	if (zoneServer == nullptr) {
		lua_pushboolean(L, false);
		return 1;
	}

	DeathWatchManager* dwManager = zoneServer->getDeathWatchManager();
	if (dwManager == nullptr) {
		lua_pushboolean(L, false);
		return 1;
	}

	bool isInvuln = dwManager->isJetpackInvulnerable(vehicleID);
	lua_pushboolean(L, isInvuln);

	return 1;
}

/**
 * Remove invulnerability from a Death Watch jetpack
 * Lua: removeDwJetpackInvulnerability(vehicleID)
 * vehicleID: Object ID of the jetpack
 */
int DirectorManager::removeDwJetpackInvulnerability(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::removeDwJetpackInvulnerability";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	uint64 vehicleID = lua_tointeger(L, -1);

	auto zoneServer = ServerCore::getZoneServer();
	if (zoneServer == nullptr) {
		return 0;
	}

	DeathWatchManager* dwManager = zoneServer->getDeathWatchManager();
	if (dwManager == nullptr) {
		return 0;
	}

	dwManager->removeJetpackInvulnerability(vehicleID);

	return 0;
}

/**
 * Process a Death Watch Mk4 Ground Striker hit on a victim
 * Lua: result = processDwMk4Hit(pAttacker, pVictim, weaponDamage)
 * pAttacker: Player who fired the missile
 * pVictim: Target player receiving the hit
 * weaponDamage: Base weapon damage for stun calculation
 * Returns: 1 on success, 0 if victim invalid, -1 on error
 */
int DirectorManager::processDwMk4Hit(lua_State* L) {
	if (checkArgumentCount(L, 3) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::processDwMk4Hit";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		lua_pushinteger(L, -1);
		return 1;
	}

	CreatureObject* attacker = (CreatureObject*)lua_touserdata(L, -3);
	CreatureObject* victim = (CreatureObject*)lua_touserdata(L, -2);
	int weaponDamage = lua_tointeger(L, -1);

	if (attacker == nullptr || victim == nullptr) {
		lua_pushinteger(L, -1);
		return 1;
	}

	ZoneServer* zoneServer = ServerCore::getZoneServer();
	if (zoneServer == nullptr) {
		lua_pushinteger(L, -1);
		return 1;
	}

	DeathWatchManager* dwManager = zoneServer->getDeathWatchManager();
	if (dwManager == nullptr) {
		lua_pushinteger(L, -1);
		return 1;
	}

	int result = dwManager->processMk4Hit(attacker, victim, weaponDamage);
	lua_pushinteger(L, result);

	return 1;
}

/**
 * Process a Death Watch missile hit (Mk1/Mk2)
 * Lua: damage = processDwMissileHit(pAttacker, pDefender, minDamage, maxDamage, damageType, armorPiercing, isSplash)
 * pAttacker: Player who fired the missile
 * pDefender: Target receiving the hit
 * minDamage: Minimum weapon damage
 * maxDamage: Maximum weapon damage
 * damageType: Damage type (e.g., KINETIC, ENERGY)
 * armorPiercing: Armor piercing level
 * isSplash: Boolean indicating if this is splash damage
 * Returns: Total damage dealt, or -1 on error
 */
int DirectorManager::processDwMissileHit(lua_State* L) {
	if (checkArgumentCount(L, 8) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::processDwMissileHit";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		lua_pushinteger(L, -1);
		return 1;
	}

	CreatureObject* attacker = (CreatureObject*)lua_touserdata(L, -8);
	CreatureObject* defender = (CreatureObject*)lua_touserdata(L, -7);
	int minDamage = lua_tointeger(L, -6);
	int maxDamage = lua_tointeger(L, -5);
	int damageType = lua_tointeger(L, -4);
	int armorPiercing = lua_tointeger(L, -3);
	bool isSplash = lua_toboolean(L, -2);
	String missileType = lua_tostring(L, -1);

	if (attacker == nullptr || defender == nullptr) {
		lua_pushinteger(L, -1);
		return 1;
	}

	ZoneServer* zoneServer = ServerCore::getZoneServer();
	if (zoneServer == nullptr) {
		lua_pushinteger(L, -1);
		return 1;
	}

	DeathWatchManager* dwManager = zoneServer->getDeathWatchManager();
	if (dwManager == nullptr) {
		lua_pushinteger(L, -1);
		return 1;
	}

	int damage = dwManager->processMissileHit(attacker, defender, minDamage, maxDamage, damageType, armorPiercing, isSplash, missileType);
	lua_pushinteger(L, damage);

	return 1;
}

/**
 * Calculate mitigated damage for Death Watch attacks
 * Lua: damage = getDwMitigatedDamage(pAttacker, pDefender, rawDamage, damageType, armorPiercing)
 * Returns: Mitigated damage after armor calculations
 */
int DirectorManager::getDwMitigatedDamage(lua_State* L) {
	if (checkArgumentCount(L, 5) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::getDwMitigatedDamage";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		lua_pushnumber(L, 0);
		return 1;
	}

	CreatureObject* attacker = (CreatureObject*)lua_touserdata(L, -5);
	CreatureObject* defender = (CreatureObject*)lua_touserdata(L, -4);
	float rawDamage = lua_tonumber(L, -3);
	int damageType = lua_tointeger(L, -2);
	int armorPiercing = lua_tointeger(L, -1);

	if (attacker == nullptr || defender == nullptr) {
		lua_pushnumber(L, 0);
		return 1;
	}

	ZoneServer* zoneServer = ServerCore::getZoneServer();
	if (zoneServer == nullptr) {
		lua_pushnumber(L, 0);
		return 1;
	}

	DeathWatchManager* dwManager = zoneServer->getDeathWatchManager();
	if (dwManager == nullptr) {
		lua_pushnumber(L, 0);
		return 1;
	}

	float mitigated = dwManager->calculateMitigatedDamage(attacker, defender, rawDamage, damageType, armorPiercing);
	lua_pushnumber(L, mitigated);

	return 1;
}

/**
 * Apply acid DoT to a target
 * Lua: applyDwAcidDot(pAttacker, pTarget, totalDamage, duration, tickInterval)
 */
int DirectorManager::applyDwAcidDot(lua_State* L) {
	if (checkArgumentCount(L, 5) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::applyDwAcidDot";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	CreatureObject* attacker = (CreatureObject*)lua_touserdata(L, -5);
	CreatureObject* target = (CreatureObject*)lua_touserdata(L, -4);
	int totalDamage = lua_tointeger(L, -3);
	int duration = lua_tointeger(L, -2);
	int tickInterval = lua_tointeger(L, -1);

	if (attacker == nullptr || target == nullptr) {
		return 0;
	}

	ZoneServer* zoneServer = ServerCore::getZoneServer();
	if (zoneServer == nullptr) {
		return 0;
	}

	DeathWatchManager* dwManager = zoneServer->getDeathWatchManager();
	if (dwManager == nullptr) {
		return 0;
	}

	dwManager->applyAcidDot(attacker, target, totalDamage, duration, tickInterval);

	return 0;
}

/**
 * Apply acid corrosion to target's armor
 * Lua: damage = applyDwAcidCorrosion(pAttacker, pTarget, baseDamage, chestMult, legsMult)
 * Returns: Total armor condition damage applied
 */
int DirectorManager::applyDwAcidCorrosion(lua_State* L) {
	if (checkArgumentCount(L, 5) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::applyDwAcidCorrosion";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		lua_pushinteger(L, 0);
		return 1;
	}

	CreatureObject* attacker = (CreatureObject*)lua_touserdata(L, -5);
	CreatureObject* target = (CreatureObject*)lua_touserdata(L, -4);
	int baseDamage = lua_tointeger(L, -3);
	float chestMult = lua_tonumber(L, -2);
	float legsMult = lua_tonumber(L, -1);

	if (attacker == nullptr || target == nullptr) {
		lua_pushinteger(L, 0);
		return 1;
	}

	ZoneServer* zoneServer = ServerCore::getZoneServer();
	if (zoneServer == nullptr) {
		lua_pushinteger(L, 0);
		return 1;
	}

	DeathWatchManager* dwManager = zoneServer->getDeathWatchManager();
	if (dwManager == nullptr) {
		lua_pushinteger(L, 0);
		return 1;
	}

	int damage = dwManager->applyAcidCorrosion(attacker, target, baseDamage, chestMult, legsMult);
	lua_pushinteger(L, damage);

	return 1;
}

/**
 * Clear combat serum debuffs (priority-based state clearing)
 * Lua: clearedMask = clearDwCombatSerumDebuffs(pPlayer, power, skill)
 * Returns bitmask of cleared states
 */
int DirectorManager::clearDwCombatSerumDebuffs(lua_State* L) {
	if (checkArgumentCount(L, 3) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::clearDwCombatSerumDebuffs";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		lua_pushinteger(L, 0);
		return 1;
	}

	int skill = lua_tointeger(L, -1);
	int power = lua_tointeger(L, -2);
	CreatureObject* player = (CreatureObject*)lua_touserdata(L, -3);

	if (player == nullptr) {
		lua_pushinteger(L, 0);
		return 1;
	}

	ZoneServer* zoneServer = ServerCore::getZoneServer();
	if (zoneServer == nullptr) {
		lua_pushinteger(L, 0);
		return 1;
	}

	DeathWatchManager* dwManager = zoneServer->getDeathWatchManager();
	if (dwManager == nullptr) {
		lua_pushinteger(L, 0);
		return 1;
	}

	int clearedMask = dwManager->clearCombatSerumDebuffs(player, power, skill);
	lua_pushinteger(L, clearedMask);

	return 1;
}

/**
 * Get hidden damage reduction for a defender
 * Lua: reduction = getDwHiddenDamageReduction(pDefender, attackType)
 * Returns: Damage reduction multiplier (0.0-0.40)
 */
int DirectorManager::getDwHiddenDamageReduction(lua_State* L) {
	if (checkArgumentCount(L, 2) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::getDwHiddenDamageReduction";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		lua_pushnumber(L, 0);
		return 1;
	}

	CreatureObject* defender = (CreatureObject*)lua_touserdata(L, -2);
	String attackType = lua_tostring(L, -1);

	if (defender == nullptr) {
		lua_pushnumber(L, 0);
		return 1;
	}

	ZoneServer* zoneServer = ServerCore::getZoneServer();
	if (zoneServer == nullptr) {
		lua_pushnumber(L, 0);
		return 1;
	}

	DeathWatchManager* dwManager = zoneServer->getDeathWatchManager();
	if (dwManager == nullptr) {
		lua_pushnumber(L, 0);
		return 1;
	}

	float reduction = dwManager->getHiddenDamageReduction(defender, attackType);
	lua_pushnumber(L, reduction);

	return 1;
}

/**
 * Get hidden evasion chance for a defender
 * Lua: chance = getDwHiddenEvasionChance(pDefender, attackType)
 * Returns: Evasion chance (0.0-0.25)
 */
int DirectorManager::getDwHiddenEvasionChance(lua_State* L) {
	if (checkArgumentCount(L, 2) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::getDwHiddenEvasionChance";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		lua_pushnumber(L, 0);
		return 1;
	}

	CreatureObject* defender = (CreatureObject*)lua_touserdata(L, -2);
	String attackType = lua_tostring(L, -1);

	if (defender == nullptr) {
		lua_pushnumber(L, 0);
		return 1;
	}

	ZoneServer* zoneServer = ServerCore::getZoneServer();
	if (zoneServer == nullptr) {
		lua_pushnumber(L, 0);
		return 1;
	}

	DeathWatchManager* dwManager = zoneServer->getDeathWatchManager();
	if (dwManager == nullptr) {
		lua_pushnumber(L, 0);
		return 1;
	}

	float chance = dwManager->getHiddenEvasionChance(defender, attackType);
	lua_pushnumber(L, chance);

	return 1;
}

/**
 * Get hidden special effect value
 * Lua: value = getDwHiddenSpecialEffect(pPlayer, attackType, effectName, isDefender)
 * Returns: Effect value or 0 if not found
 */
int DirectorManager::getDwHiddenSpecialEffect(lua_State* L) {
	if (checkArgumentCount(L, 4) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::getDwHiddenSpecialEffect";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		lua_pushnumber(L, 0);
		return 1;
	}

	CreatureObject* player = (CreatureObject*)lua_touserdata(L, -4);
	String attackType = lua_tostring(L, -3);
	String effectName = lua_tostring(L, -2);
	bool isDefender = lua_toboolean(L, -1);

	if (player == nullptr) {
		lua_pushnumber(L, 0);
		return 1;
	}

	ZoneServer* zoneServer = ServerCore::getZoneServer();
	if (zoneServer == nullptr) {
		lua_pushnumber(L, 0);
		return 1;
	}

	DeathWatchManager* dwManager = zoneServer->getDeathWatchManager();
	if (dwManager == nullptr) {
		lua_pushnumber(L, 0);
		return 1;
	}

	float value = dwManager->getHiddenSpecialEffect(player, attackType, effectName, isDefender);
	lua_pushnumber(L, value);

	return 1;
}

// ============================================================================
// DEATH WATCH STEALTH PROTOCOL BINDINGS
// ============================================================================

/**
 * Activate Shadow Protocol for a Black Sun player (C++)
 * Lua: success = activateShadowProtocolCpp(pPlayer)
 * pPlayer: Creature object (the player)
 * Returns: true if activated successfully
 */
int DirectorManager::activateShadowProtocolCpp(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::activateShadowProtocolCpp";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		lua_pushboolean(L, false);
		return 1;
	}

	CreatureObject* player = (CreatureObject*)lua_touserdata(L, -1);

	if (player == nullptr) {
		lua_pushboolean(L, false);
		return 1;
	}

	ZoneServer* zoneServer = ServerCore::getZoneServer();
	if (zoneServer == nullptr) {
		lua_pushboolean(L, false);
		return 1;
	}

	DeathWatchManager* dwManager = zoneServer->getDeathWatchManager();
	if (dwManager == nullptr) {
		lua_pushboolean(L, false);
		return 1;
	}

	bool success = dwManager->activateShadowProtocol(player);
	lua_pushboolean(L, success);

	return 1;
}

/**
 * Deactivate Shadow Protocol for a player (C++)
 * Lua: deactivateShadowProtocolCpp(pPlayer, broadcastUpdate)
 * pPlayer: Creature object (the player)
 * broadcastUpdate: true to broadcast PvP status update
 */
int DirectorManager::deactivateShadowProtocolCpp(lua_State* L) {
	if (checkArgumentCount(L, 2) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::deactivateShadowProtocolCpp";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	bool broadcastUpdate = lua_toboolean(L, -1);
	CreatureObject* player = (CreatureObject*)lua_touserdata(L, -2);

	if (player == nullptr) {
		return 0;
	}

	ZoneServer* zoneServer = ServerCore::getZoneServer();
	if (zoneServer == nullptr) {
		return 0;
	}

	DeathWatchManager* dwManager = zoneServer->getDeathWatchManager();
	if (dwManager == nullptr) {
		return 0;
	}

	dwManager->deactivateShadowProtocol(player, broadcastUpdate);

	return 0;
}

/**
 * Check if Shadow Protocol is active for a player (C++)
 * Lua: isActive = isShadowProtocolActiveCpp(pPlayer)
 * pPlayer: Creature object (the player)
 * Returns: true if Shadow Protocol is active
 */
int DirectorManager::isShadowProtocolActiveCpp(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::isShadowProtocolActiveCpp";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		lua_pushboolean(L, false);
		return 1;
	}

	CreatureObject* player = (CreatureObject*)lua_touserdata(L, -1);

	if (player == nullptr) {
		lua_pushboolean(L, false);
		return 1;
	}

	ZoneServer* zoneServer = ServerCore::getZoneServer();
	if (zoneServer == nullptr) {
		lua_pushboolean(L, false);
		return 1;
	}

	DeathWatchManager* dwManager = zoneServer->getDeathWatchManager();
	if (dwManager == nullptr) {
		lua_pushboolean(L, false);
		return 1;
	}

	bool isActive = dwManager->isShadowProtocolActive(player);
	lua_pushboolean(L, isActive);

	return 1;
}

/**
 * Activate Espionage Matrix for a Zann Consortium player (C++)
 * Lua: success = activateEspionageMatrixCpp(pPlayer)
 * pPlayer: Creature object (the player)
 * Returns: true if activated successfully
 */
int DirectorManager::activateEspionageMatrixCpp(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::activateEspionageMatrixCpp";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		lua_pushboolean(L, false);
		return 1;
	}

	CreatureObject* player = (CreatureObject*)lua_touserdata(L, -1);

	if (player == nullptr) {
		lua_pushboolean(L, false);
		return 1;
	}

	ZoneServer* zoneServer = ServerCore::getZoneServer();
	if (zoneServer == nullptr) {
		lua_pushboolean(L, false);
		return 1;
	}

	DeathWatchManager* dwManager = zoneServer->getDeathWatchManager();
	if (dwManager == nullptr) {
		lua_pushboolean(L, false);
		return 1;
	}

	bool success = dwManager->activateEspionageMatrix(player);
	lua_pushboolean(L, success);

	return 1;
}

/**
 * Deactivate Espionage Matrix for a player (C++)
 * Lua: deactivateEspionageMatrixCpp(pPlayer, broadcastUpdate)
 * pPlayer: Creature object (the player)
 * broadcastUpdate: true to broadcast PvP status update
 */
int DirectorManager::deactivateEspionageMatrixCpp(lua_State* L) {
	if (checkArgumentCount(L, 2) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::deactivateEspionageMatrixCpp";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	bool broadcastUpdate = lua_toboolean(L, -1);
	CreatureObject* player = (CreatureObject*)lua_touserdata(L, -2);

	if (player == nullptr) {
		return 0;
	}

	ZoneServer* zoneServer = ServerCore::getZoneServer();
	if (zoneServer == nullptr) {
		return 0;
	}

	DeathWatchManager* dwManager = zoneServer->getDeathWatchManager();
	if (dwManager == nullptr) {
		return 0;
	}

	dwManager->deactivateEspionageMatrix(player, broadcastUpdate);

	return 0;
}

/**
 * Check if Espionage Matrix is active for a player (C++)
 * Lua: isActive = isEspionageMatrixActiveCpp(pPlayer)
 * pPlayer: Creature object (the player)
 * Returns: true if Espionage Matrix is active
 */
int DirectorManager::isEspionageMatrixActiveCpp(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::isEspionageMatrixActiveCpp";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		lua_pushboolean(L, false);
		return 1;
	}

	CreatureObject* player = (CreatureObject*)lua_touserdata(L, -1);

	if (player == nullptr) {
		lua_pushboolean(L, false);
		return 1;
	}

	ZoneServer* zoneServer = ServerCore::getZoneServer();
	if (zoneServer == nullptr) {
		lua_pushboolean(L, false);
		return 1;
	}

	DeathWatchManager* dwManager = zoneServer->getDeathWatchManager();
	if (dwManager == nullptr) {
		lua_pushboolean(L, false);
		return 1;
	}

	bool isActive = dwManager->isEspionageMatrixActive(player);
	lua_pushboolean(L, isActive);

	return 1;
}

/**
 * Check if a target is hidden from an observer due to stealth protocol (C++)
 * Lua: isHidden = isTargetHiddenByStealthProtocolCpp(pObserver, pTarget)
 * pObserver: Creature object (the one trying to detect)
 * pTarget: Creature object (the one being searched for)
 * Returns: true if target is hidden
 */
int DirectorManager::isTargetHiddenByStealthProtocolCpp(lua_State* L) {
	if (checkArgumentCount(L, 2) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::isTargetHiddenByStealthProtocolCpp";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		lua_pushboolean(L, false);
		return 1;
	}

	CreatureObject* target = (CreatureObject*)lua_touserdata(L, -1);
	CreatureObject* observer = (CreatureObject*)lua_touserdata(L, -2);

	if (observer == nullptr || target == nullptr) {
		lua_pushboolean(L, false);
		return 1;
	}

	ZoneServer* zoneServer = ServerCore::getZoneServer();
	if (zoneServer == nullptr) {
		lua_pushboolean(L, false);
		return 1;
	}

	DeathWatchManager* dwManager = zoneServer->getDeathWatchManager();
	if (dwManager == nullptr) {
		lua_pushboolean(L, false);
		return 1;
	}

	bool isHidden = dwManager->isTargetHiddenByStealthProtocol(observer, target);
	lua_pushboolean(L, isHidden);

	return 1;
}

/**
 * Broadcast a player's PvP status to ALL online Death Watch players (C++)
 * Lua: broadcastPvpStatusToAllDWCpp(pPlayer)
 * pPlayer: Creature object (the player whose status changed)
 */
int DirectorManager::broadcastPvpStatusToAllDWCpp(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::broadcastPvpStatusToAllDWCpp";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	CreatureObject* player = (CreatureObject*)lua_touserdata(L, -1);

	if (player == nullptr) {
		return 0;
	}

	ZoneServer* zoneServer = ServerCore::getZoneServer();
	if (zoneServer == nullptr) {
		return 0;
	}

	DeathWatchManager* dwManager = zoneServer->getDeathWatchManager();
	if (dwManager == nullptr) {
		return 0;
	}

	dwManager->broadcastPvpStatusToAllDW(player);

	return 0;
}

/**
 * Refresh a player's view of ALL other Death Watch players' PvP status (C++)
 * Lua: refreshPvpStatusForPlayerCpp(pPlayer)
 * pPlayer: Creature object (the player who needs updates)
 */
int DirectorManager::refreshPvpStatusForPlayerCpp(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::refreshPvpStatusForPlayerCpp";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	CreatureObject* player = (CreatureObject*)lua_touserdata(L, -1);

	if (player == nullptr) {
		return 0;
	}

	ZoneServer* zoneServer = ServerCore::getZoneServer();
	if (zoneServer == nullptr) {
		return 0;
	}

	DeathWatchManager* dwManager = zoneServer->getDeathWatchManager();
	if (dwManager == nullptr) {
		return 0;
	}

	dwManager->refreshPvpStatusForPlayer(player);

	return 0;
}

// ============================================================================
// DEATH WATCH RADAR STEALTH LUA BINDINGS
// ============================================================================

/**
 * Activate radar stealth for a player
 * Lua: activateRadarStealthCpp(pPlayer)
 * Returns: 0 = success, -1 = not DW, -2 = players nearby, -3 = already active
 */
int DirectorManager::activateRadarStealthCpp(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::activateRadarStealthCpp";
		printTraceError(L, err);

		ERROR_CODE = INCORRECT_ARGUMENTS;
		lua_pushinteger(L, -99);
		return 1;
	}

	CreatureObject* player = (CreatureObject*)lua_touserdata(L, -1);

	if (player == nullptr) {
		lua_pushinteger(L, -99);
		return 1;
	}

	ZoneServer* zoneServer = ServerCore::getZoneServer();
	if (zoneServer == nullptr) {
		lua_pushinteger(L, -99);
		return 1;
	}

	DeathWatchManager* dwManager = zoneServer->getDeathWatchManager();
	if (dwManager == nullptr) {
		lua_pushinteger(L, -99);
		return 1;
	}

	int result = dwManager->activateRadarStealth(player);
	lua_pushinteger(L, result);

	return 1;
}

/**
 * Activate radar stealth with explicit duration
 * Lua: activateRadarStealthWithDurationCpp(pPlayer, durationMs, bypassChecks)
 * Returns: 0 = success, -1 = not DW, -2 = players nearby, -3 = already active
 */
int DirectorManager::activateRadarStealthWithDurationCpp(lua_State* L) {
	if (checkArgumentCount(L, 3) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::activateRadarStealthWithDurationCpp";
		printTraceError(L, err);

		ERROR_CODE = INCORRECT_ARGUMENTS;
		lua_pushinteger(L, -99);
		return 1;
	}

	bool bypassChecks = lua_toboolean(L, -1);
	int durationMs = lua_tointeger(L, -2);
	CreatureObject* player = (CreatureObject*)lua_touserdata(L, -3);

	if (player == nullptr) {
		lua_pushinteger(L, -99);
		return 1;
	}

	ZoneServer* zoneServer = ServerCore::getZoneServer();
	if (zoneServer == nullptr) {
		lua_pushinteger(L, -99);
		return 1;
	}

	DeathWatchManager* dwManager = zoneServer->getDeathWatchManager();
	if (dwManager == nullptr) {
		lua_pushinteger(L, -99);
		return 1;
	}

	int result = dwManager->activateRadarStealthWithDuration(player, durationMs, bypassChecks);
	lua_pushinteger(L, result);

	return 1;
}

/**
 * Deactivate radar stealth for a player
 * Lua: deactivateRadarStealthCpp(pPlayer, reason)
 * reason: 0=manual, 1=expired, 2=zonechange, 3=detected
 */
int DirectorManager::deactivateRadarStealthCpp(lua_State* L) {
	if (checkArgumentCount(L, 2) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::deactivateRadarStealthCpp";
		printTraceError(L, err);

		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	int reason = lua_tointeger(L, -1);
	CreatureObject* player = (CreatureObject*)lua_touserdata(L, -2);

	if (player == nullptr) {
		return 0;
	}

	ZoneServer* zoneServer = ServerCore::getZoneServer();
	if (zoneServer == nullptr) {
		return 0;
	}

	DeathWatchManager* dwManager = zoneServer->getDeathWatchManager();
	if (dwManager == nullptr) {
		return 0;
	}

	dwManager->deactivateRadarStealth(player, reason);

	return 0;
}

/**
 * Check if radar stealth is active for a player
 * Lua: isRadarStealthActiveCpp(pPlayer)
 * Returns: boolean
 */
int DirectorManager::isRadarStealthActiveCpp(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::isRadarStealthActiveCpp";
		printTraceError(L, err);

		ERROR_CODE = INCORRECT_ARGUMENTS;
		lua_pushboolean(L, false);
		return 1;
	}

	CreatureObject* player = (CreatureObject*)lua_touserdata(L, -1);

	if (player == nullptr) {
		lua_pushboolean(L, false);
		return 1;
	}

	ZoneServer* zoneServer = ServerCore::getZoneServer();
	if (zoneServer == nullptr) {
		lua_pushboolean(L, false);
		return 1;
	}

	DeathWatchManager* dwManager = zoneServer->getDeathWatchManager();
	if (dwManager == nullptr) {
		lua_pushboolean(L, false);
		return 1;
	}

	bool isActive = dwManager->isRadarStealthActive(player);
	lua_pushboolean(L, isActive);

	return 1;
}

/**
 * Get remaining radar stealth duration in milliseconds
 * Lua: getRadarStealthRemainingMsCpp(pPlayer)
 * Returns: int (milliseconds remaining, 0 if not active)
 */
int DirectorManager::getRadarStealthRemainingMsCpp(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::getRadarStealthRemainingMsCpp";
		printTraceError(L, err);

		ERROR_CODE = INCORRECT_ARGUMENTS;
		lua_pushinteger(L, 0);
		return 1;
	}

	CreatureObject* player = (CreatureObject*)lua_touserdata(L, -1);

	if (player == nullptr) {
		lua_pushinteger(L, 0);
		return 1;
	}

	ZoneServer* zoneServer = ServerCore::getZoneServer();
	if (zoneServer == nullptr) {
		lua_pushinteger(L, 0);
		return 1;
	}

	DeathWatchManager* dwManager = zoneServer->getDeathWatchManager();
	if (dwManager == nullptr) {
		lua_pushinteger(L, 0);
		return 1;
	}

	int remaining = dwManager->getRadarStealthRemainingMs(player);
	lua_pushinteger(L, remaining);

	return 1;
}

/**
 * Calculate radar stealth duration based on player's skills
 * Lua: calculateRadarStealthDurationCpp(pPlayer)
 * Returns: int (duration in milliseconds)
 */
int DirectorManager::calculateRadarStealthDurationCpp(lua_State* L) {
	if (checkArgumentCount(L, 1) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::calculateRadarStealthDurationCpp";
		printTraceError(L, err);

		ERROR_CODE = INCORRECT_ARGUMENTS;
		lua_pushinteger(L, 0);
		return 1;
	}

	CreatureObject* player = (CreatureObject*)lua_touserdata(L, -1);

	if (player == nullptr) {
		lua_pushinteger(L, 0);
		return 1;
	}

	ZoneServer* zoneServer = ServerCore::getZoneServer();
	if (zoneServer == nullptr) {
		lua_pushinteger(L, 0);
		return 1;
	}

	DeathWatchManager* dwManager = zoneServer->getDeathWatchManager();
	if (dwManager == nullptr) {
		lua_pushinteger(L, 0);
		return 1;
	}

	int duration = dwManager->calculateRadarStealthDuration(player);
	lua_pushinteger(L, duration);

	return 1;
}


int DirectorManager::getSpawnPointInCell(lua_State* L) {
	if (checkArgumentCount(L, 4) != 0) {
		String err = "incorrect number of arguments passed to DirectorManager::getSpawnPointInCell";
		printTraceError(L, err);
		ERROR_CODE = INCORRECT_ARGUMENTS;
		return 0;
	}

	String zoneName = lua_tostring(L, -4);
	uint64 buildingID = lua_tointeger(L, -3);
	uint64 cellID = lua_tointeger(L, -2);
	float radius = lua_tonumber(L, -1);

	ZoneServer* zoneServer = ServerCore::getZoneServer();
	if (zoneServer == nullptr) {
		instance()->error("ZoneServer is nullptr in DirectorManager::getSpawnPointInCell");
		return 0;
	}

	Zone* zone = zoneServer->getZone(zoneName);
	if (zone == nullptr) {
		instance()->error("Zone is nullptr in DirectorManager::getSpawnPointInCell (" + zoneName + ")");
		return 0;
	}

	ManagedReference<SceneObject*> buildingObj = zoneServer->getObject(buildingID);
	if (buildingObj == nullptr || !buildingObj->isBuildingObject()) {
		instance()->error("Invalid building ID in DirectorManager::getSpawnPointInCell");
		return 0;
	}

	BuildingObject* building = buildingObj.castTo<BuildingObject*>();
	if (building == nullptr) {
		instance()->error("Building cast failed in DirectorManager::getSpawnPointInCell");
		return 0;
	}

	ManagedReference<SceneObject*> cellObj = zoneServer->getObject(cellID);
	if (cellObj == nullptr || !cellObj->isCellObject()) {
		instance()->error("Invalid cell ID in DirectorManager::getSpawnPointInCell");
		return 0;
	}

	CellObject* cell = cellObj.castTo<CellObject*>();
	if (cell == nullptr) {
		instance()->error("Cell cast failed in DirectorManager::getSpawnPointInCell");
		return 0;
	}

	if (cell->getParent().get() != building) {
		instance()->error("Cell does not belong to specified building in DirectorManager::getSpawnPointInCell");
		return 0;
	}

	float angle = System::frandom() * 2.0f * M_PI;
	float r = sqrt(System::frandom()) * radius;

	float x = r * cos(angle);
	float y = r * sin(angle);
	float z = 0.0f;

	lua_newtable(L);
	lua_pushnumber(L, x);
	lua_pushnumber(L, z);
	lua_pushnumber(L, y);
	lua_rawseti(L, -4, 3);
	lua_rawseti(L, -3, 2);
	lua_rawseti(L, -2, 1);

	return 1;
}
