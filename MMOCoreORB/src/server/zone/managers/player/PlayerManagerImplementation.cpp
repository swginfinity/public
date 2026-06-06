/*
 * PlayerManagerImplementation.cpp
 *
 *  Created on: 18/07/2009
 *      Author: TheAnswer
 */

#include "server/zone/managers/player/PlayerManager.h"
#include <utility>
#include <mutex>

#ifdef WITH_SWGREALMS_API
#include "server/login/SWGRealmsAPI.h"
#endif // WITH_SWGREALMS_API

#include "server/zone/packets/charcreation/ClientCreateCharacterCallback.h"
#include "server/zone/packets/charcreation/ClientCreateCharacterFailed.h"
#include "server/zone/ZoneServer.h"
#include "server/zone/ZoneProcessServer.h"
#include "server/zone/managers/name/NameManager.h"
#include "templates/manager/TemplateManager.h"
#include "server/zone/managers/object/ObjectManager.h"
#include "server/zone/managers/faction/FactionManager.h"
#include "server/zone/managers/frs/FrsManager.h"
#include "server/db/ServerDatabase.h"
#include "server/chat/ChatManager.h"
#include "server/zone/packets/chat/ChatRoomMessage.h"
#include "server/zone/managers/objectcontroller/ObjectController.h"
#include "server/zone/managers/combat/CombatManager.h"
#include "server/zone/managers/skill/Performance.h"
#include "server/zone/managers/collision/CollisionManager.h"
#include "server/zone/objects/intangible/VehicleControlDevice.h"
#include "server/zone/objects/tangible/threat/ThreatMap.h"
#include "server/zone/objects/creature/VehicleObject.h"
#include "server/login/packets/ErrorMessage.h"
#include "server/zone/packets/player/LogoutMessage.h"
#include "server/zone/objects/player/sessions/TradeSession.h"
#include "server/zone/objects/player/sessions/ProposeUnitySession.h"
#include "server/zone/objects/player/sessions/VeteranRewardSession.h"
#include "templates/params/OptionBitmask.h"
#include "server/zone/managers/player/JukeboxSong.h"
#include "server/zone/managers/player/QuestInfo.h"

#include "server/zone/objects/intangible/ShipControlDevice.h"
#include "server/zone/objects/group/GroupObject.h"
#include "server/zone/objects/building/BuildingObject.h"
#include "templates/building/CloningBuildingObjectTemplate.h"
#include "server/zone/objects/player/PlayerObject.h"
#include "server/zone/objects/tangible/wearables/ArmorObject.h"
#include "server/zone/objects/tangible/weapon/WeaponObject.h"
#include "server/zone/objects/tangible/wearables/WearableObject.h"

#include "server/zone/objects/player/events/PlayerIncapacitationRecoverTask.h"
#include "server/zone/objects/creature/events/ProposeUnityExpiredTask.h"
#include "server/zone/objects/player/events/ForceMeditateTask.h"
#include "server/zone/objects/player/events/MeditateTask.h"
#include "server/zone/objects/player/events/LogoutTask.h"
#include "server/zone/objects/player/sessions/EntertainingSession.h"
#include "templates/building/CloneSpawnPoint.h"
#include "server/zone/objects/player/sui/messagebox/SuiMessageBox.h"
#include "server/zone/objects/player/sui/listbox/SuiListBox.h"
#include "server/zone/objects/cell/CellObject.h"
#include "server/zone/managers/skill/SkillManager.h"
#include "server/zone/objects/player/FactionStatus.h"
#include "server/zone/managers/planet/PlanetManager.h"

#include "server/zone/packets/trade/AbortTradeMessage.h"
#include "server/zone/packets/trade/AcceptTransactionMessage.h"
#include "server/zone/packets/trade/UnAcceptTransactionMessage.h"
#include "server/zone/packets/trade/AddItemMessage.h"
#include "server/zone/packets/trade/TradeCompleteMessage.h"
#include "server/zone/packets/trade/GiveMoneyMessage.h"
#include "server/zone/packets/chat/ChatSystemMessage.h"
#include "server/zone/packets/tangible/TangibleObjectDeltaMessage3.h"
#include "server/zone/packets/player/PlayMusicMessage.h"
#include "server/zone/packets/object/StartingLocationListMessage.h"

#include "server/zone/objects/region/CityRegion.h"
#include "server/zone/managers/director/DirectorManager.h"
#include "server/zone/objects/player/sui/callbacks/CloningRequestSuiCallback.h"
#include "server/zone/objects/tangible/tool/CraftingStation.h"
#include "server/zone/objects/tangible/tool/CraftingTool.h"

#include "server/zone/Zone.h"
#include "server/zone/managers/player/creation/PlayerCreationManager.h"
#include "server/ServerCore.h"
#include "server/login/account/Account.h"

#include "server/zone/objects/player/sui/callbacks/PlayerTeachSuiCallback.h"
#include "server/zone/objects/player/sui/callbacks/PlayerTeachConfirmSuiCallback.h"
#include "server/zone/objects/player/sui/callbacks/ProposeUnitySuiCallback.h"
#include "server/zone/objects/player/sui/callbacks/SelectUnityRingSuiCallback.h"
#include "server/zone/objects/player/sui/callbacks/SelectVeteranRewardSuiCallback.h"
#include "server/zone/objects/player/sui/callbacks/ConfirmVeteranRewardSuiCallback.h"
#include "server/zone/objects/player/sui/callbacks/ConfirmDivorceSuiCallback.h"

#include "server/zone/managers/stringid/StringIdManager.h"
#include "server/zone/objects/creature/buffs/PowerBoostBuff.h"
//#include "server/zone/objects/creature/buffs/ForceWeakenDebuff.h"  //Infinity:  Not used
#include "server/zone/objects/creature/ai/Creature.h"
#include "server/zone/objects/creature/ai/NonPlayerCreatureObject.h"
#include "server/zone/objects/creature/events/DespawnCreatureTask.h"
#include "server/zone/objects/creature/ai/AiAgent.h"
#include "server/zone/managers/gcw/GCWManager.h"
#include "server/zone/objects/intangible/PetControlDevice.h"
#include "server/zone/managers/creature/PetManager.h"
#include "server/zone/objects/creature/events/BurstRunNotifyAvailableEvent.h"
#include "server/zone/objects/creature/ai/DroidObject.h"
#include "server/zone/objects/tangible/components/droid/DroidPlaybackModuleDataComponent.h"
#include "server/zone/objects/player/badges/Badge.h"
#include "server/zone/objects/building/TutorialBuildingObject.h"
#include "server/zone/managers/frs/FrsManager.h"
#include "server/zone/objects/player/events/OnlinePlayerLogTask.h"
#include <sys/stat.h>
#include "server/zone/objects/transaction/TransactionLog.h"
#include "server/zone/objects/creature/commands/TransferItemMiscCommand.h"
#include "templates/crcstringtable/CrcStringTable.h"
// Infinity: POB ship permission support
#include "server/zone/objects/ship/PobShipObject.h"
// Infinity: Space XP distribution
#include "server/zone/objects/ship/ai/ShipAiAgent.h"
#include "server/zone/objects/tangible/deed/ship/ShipDeed.h"
#include "server/zone/packets/object/transform/Transform.h"

#include "server/zone/managers/statistics/StatisticsManager.h"

//Infinity:  Custom Includes
#include "server/zone/objects/creature/events/RocketBootsNotifyAvailableEvent.h"
#include "server/zone/objects/creature/events/DashNotifyAvailableEvent.h"
#include "server/zone/objects/creature/buffs/PrivateSkillMultiplierBuff.h"
#include "server/zone/objects/player/events/StoreSpawnedChildrenTask.h"
#include "server/zone/managers/death_watch/DeathWatchManager.h"
// basic file operations
#include <iostream>
#include <fstream>

//BH Code Additions
#include "server/zone/managers/visibility/VisibilityManager.h"
#include "server/zone/objects/player/sui/callbacks/BountyHuntSuiCallback.h"
#include "server/zone/objects/player/sui/inputbox/SuiInputBox.h"


// #define DEBUG_SPEED_HACK

// Infinity: Speed hack detection defaults (overridable via Core3.PlayerManager in config.lua)
static constexpr uint32 DEFAULT_SPEED_HACK_DELTA_CHECK = 100;
static constexpr float DEFAULT_SPEED_HACK_ERROR_TOLERANCE = 1.05f;

PlayerManagerImplementation::PlayerManagerImplementation(ZoneServer* zoneServer, ZoneProcessServer* impl, bool trackOnlineUsers) : Logger("PlayerManager") {
	playerLoggerFilename = "log/player.log";
	playerLoggerLines = ConfigManager::instance()->getMaxLogLines();
	playerLogger.setLoggingName("PlayerLogger");
	playerLogger.setFileLogger(playerLoggerFilename, true);

	server = zoneServer;
	processor = impl;

	nameMap = new CharacterNameMap();

	DirectorManager::instance()->getLuaInstance()->runFile("scripts/screenplays/checklnum.lua");

	loadLuaConfig();
	loadStartingLocations();
	loadQuestInfo();
	loadQuestCrcTable();
	loadPermissionLevels();
	loadXpBonusList();

	onlinePlayersLogOnSessionChange = false;

	setGlobalLogging(true);
	setLogging(false);

#ifndef WITH_SWGREALMS_API
	if (ServerCore::truncateDatabases()) {
		try {
			const static String query = "TRUNCATE TABLE characters";

			Reference<ResultSet*> res = ServerDatabase::instance()->executeQuery(query);

			info("characters table truncated", true);
		} catch (const Exception& e) {
			error(e.getMessage());
		}
	}
#endif // !WITH_SWGREALMS_API

	loadNameMap();

	if (ServerCore::hasArgument("listadmins")) {
		info("generating admin list...", true);

		auto admins = generateAdminList();

		for (auto& entry : admins) {
			auto name = entry.getKey();
			auto level = entry.getValue();

			info(true) << "Player: " << name << " level: " << level;
		}
	}

	int onlineLogSeconds = ConfigManager::instance()->getOnlineLogSeconds();

	if (onlineLogSeconds > 0 && trackOnlineUsers) {
		onlinePlayerLogSum = 0;

		Core::getTaskManager()->executeTask([=] () {
			rescheduleOnlinePlayerLogTask(onlineLogSeconds);
		}, "startOnlinePlayerLogTask");
	}
}

void PlayerManagerImplementation::stopOnlinePlayerLogTask() {
	auto onlinePlayerLogTask = this->onlinePlayerLogTask.get();

	if (onlinePlayerLogTask != nullptr) {
		onlinePlayerLogTask->cancel();
	}
}

bool PlayerManagerImplementation::rescheduleOnlinePlayerLogTask(int logSecs) {
	auto onlinePlayerLogTask = this->onlinePlayerLogTask.get();

	if (logSecs <= -1) {
		if (onlinePlayerLogTask != nullptr) {
			onlinePlayerLogTask->cancel();
			this->onlinePlayerLogTask = onlinePlayerLogTask = nullptr;
		}
		info("Loging online players disabled.", true);
		return true;
	}

	if (logSecs < 10) {
		error() << "rescheduleOnlinePlayerLogTask: attempt to set log schedule too low: " << logSecs;
		return false;
	}

	if (onlinePlayerLogTask == nullptr) {
		this->onlinePlayerLogTask = onlinePlayerLogTask = new OnlinePlayerLogTask();
	} else {
		onlinePlayerLogTask->cancel();
	}

	onlinePlayersLogOnSessionChange = ConfigManager::instance()->getBool("Core3.LogOnlineOnSessionChange", true);

	onlinePlayerLogTask->schedulePeriodic(0, logSecs * 1000);

	info(true) << "Logging online players every " << logSecs << " seconds.";

	return true;
}

bool PlayerManagerImplementation::createPlayer(ClientCreateCharacterCallback* callback) {
	PlayerCreationManager* pcm = PlayerCreationManager::instance();
	return pcm->createCharacter(callback);
}

void PlayerManagerImplementation::loadLuaConfig() {
	info("Loading configuration script.");

	Lua* lua = new Lua();
	lua->init();

	lua->runFile("scripts/managers/player_manager.lua");

	allowSameAccountPvpRatingCredit = lua->getGlobalInt("allowSameAccountPvpRatingCredit");
	onlineCharactersPerAccount = lua->getGlobalInt("onlineCharactersPerAccount");
	performanceBuff = lua->getGlobalInt("performanceBuff");
	medicalBuff = lua->getGlobalInt("medicalBuff");
	performanceDuration = lua->getGlobalInt("performanceDuration");
	medicalDuration = lua->getGlobalInt("medicalDuration");

    groupExpMultiplier = lua->getGlobalFloat("groupExpMultiplier");
    if (groupExpMultiplier == 0.0f) groupExpMultiplier = 1.0f;

    globalExpMultiplier = lua->getGlobalFloat("globalExpMultiplier");
    if (globalExpMultiplier == 0.0f) globalExpMultiplier = 1.0f;

    JediExpMultiplier = lua->getGlobalFloat("JediExpMultiplier");  // Infinity: Custom
    if (JediExpMultiplier == 0.0f) JediExpMultiplier = 1.0f;

    FRSExpMultiplier = lua->getGlobalFloat("FRSExpMultiplier");  // Infinity: Custom
    if (FRSExpMultiplier == 0.0f) FRSExpMultiplier = 1.0f;

    stadiumFrsMultiplier = lua->getGlobalFloat("stadiumFrsMultiplier");  // Infinity: Custom — scalar applied to FRS award when killer/victim are inside the stadium (player_manager.lua). 0.0 (or omitted) = arena mode: stadium kills award no FRS to killer/group and deduct no FRS from victim. 0.75 = legacy 25% reduction. 1.0 = no penalty. Runtime override: /server playermanager setStadiumFrsMultiplier <float>. No coercion — the configured value (default 0.0) wins; GH #1624.

	baseStoredCreaturePets = lua->getGlobalInt("baseStoredCreaturePets");
	baseStoredFactionPets = lua->getGlobalInt("baseStoredFactionPets");
	baseStoredDroids = lua->getGlobalInt("baseStoredDroids");
	baseStoredVehicles = lua->getGlobalInt("baseStoredVehicles");
	baseStoredShips = lua->getGlobalInt("baseStoredShips");

	veteranRewardAdditionalMilestones = lua->getGlobalInt("veteranRewardAdditionalMilestones");

	LuaObject rewardMilestonesLua = lua->getGlobalObject("veteranRewardMilestones");
	for (int i = 1; i <= rewardMilestonesLua.getTableSize(); ++i) {
		veteranRewardMilestones.add(rewardMilestonesLua.getIntAt(i));
	}
	rewardMilestonesLua.pop();

	LuaObject rewardsListLua = lua->getGlobalObject("veteranRewards");
	int size = rewardsListLua.getTableSize();

	lua_State* L = rewardsListLua.getLuaState();

	for (int i = 0; i < size; ++i) {
		lua_rawgeti(L, -1, i + 1);
		LuaObject a(L);

		VeteranReward reward;
		reward.parseFromLua(&a);
		veteranRewards.add(reward);

		a.pop();
	}

	rewardsListLua.pop();

	info(true) << "Loaded " << veteranRewards.size() << " veteran rewards.";

	LuaObject jboxSongs = lua->getGlobalObject("jukeboxSongs");

	if (jboxSongs.isValidTable()) {
		for (int i = 1; i <= jboxSongs.getTableSize(); ++i) {
			LuaObject songData = jboxSongs.getObjectAt(i);

			if (songData.isValidTable()) {
				String songStringID = songData.getStringAt(1);
				String songPath = songData.getStringAt(2);

				Reference<JukeboxSong*> data = new JukeboxSong(songStringID, songPath);

				jukeboxSongs.add(data);
			}

			songData.pop();
		}

	}

	jboxSongs.pop();

	delete lua;
	lua = nullptr;
}

void PlayerManagerImplementation::loadStartingLocations() {
	info("Loading starting locations.");

	IffStream* iffStream = TemplateManager::instance()->openIffFile("datatables/creation/starting_locations.iff");

	if (iffStream == nullptr) {
		info("Couldn't load starting locations.", true);
		return;
	}

	startingLocationList.parseFromIffStream(iffStream);

	delete iffStream;

	info(true) << "Loaded " << startingLocationList.getTotalLocations() << " starting locations.";
}

void PlayerManagerImplementation::loadXpBonusList() {
	IffStream* iffStream = TemplateManager::instance()->openIffFile("datatables/xp/species.iff");

	if (iffStream == nullptr) {
		info("Couldn't load species xp bonuses.", true);
		return;
	}

	DataTableIff dtiff;
	dtiff.readObject(iffStream);

	delete iffStream;

	for (int i = 0; i < dtiff.getTotalColumns(); i++) {
		VectorMap<String, int> bonusList;
		String speciesName = dtiff.getColumnNameByIndex(i);

		for (int j = 0; j < dtiff.getTotalRows(); j++) {
			DataTableRow* row = dtiff.getRow(j);
			String columnData = "";
			row->getCell(i)->getValue(columnData);

			if (columnData != "") {
				StringTokenizer callbackString(columnData);
				callbackString.setDelimeter(":");

				String xpType = "";
				int bonusMod = 0;

				callbackString.getStringToken(xpType);
				bonusMod = callbackString.getIntToken();

				if (xpType == "" or bonusMod == 0)
					continue;

				bonusList.put(xpType, bonusMod);
			}
		}

		if (bonusList.size() > 0)
			xpBonusList.put(speciesName, bonusList);
	}

	info(true) << "Loaded xp bonuses for " << xpBonusList.size() << " species.";
}

void PlayerManagerImplementation::loadQuestInfo() {
	TemplateManager* templateManager = TemplateManager::instance();

	IffStream* iffStream = templateManager->openIffFile("datatables/player/quests.iff");

	if (iffStream == nullptr) {
		info("quests.iff could not be found.", true);
		return;
	}

	DataTableIff dtable;
	dtable.readObject(iffStream);

	delete iffStream;

	for (int i = 0; i < dtable.getTotalRows(); ++i) {
		DataTableRow* row = dtable.getRow(i);

		QuestInfo* quest = new QuestInfo();
		quest->parseDataTableRow(row);
		questInfo.add(quest);
	}

	info(true) << "Loaded " << questInfo.size() << " quests.";
}

void PlayerManagerImplementation::loadQuestCrcTable() {
	TemplateManager* templateManager = TemplateManager::instance();

	IffStream* iffStream = templateManager->openIffFile("misc/quest_crc_string_table.iff");

	if (iffStream == nullptr) {
		info("quest_crc_string_table.iff could not be found.", true);
		return;
	}

	CrcStringTable stringTable;
	stringTable.readObject(iffStream);

	delete iffStream;

	questCrcTable = stringTable.getTableData();

	info(true) << "Loaded " << questCrcTable.size() << " quests crc's.";
}

void PlayerManagerImplementation::loadPermissionLevels() {
	try {
		permissionLevelList = PermissionLevelList::instance();
		permissionLevelList->loadLevels();
	} catch(const Exception& e) {
		error("Couldn't load permission levels.");
		error(e.getMessage());
	}
}

void PlayerManagerImplementation::finalize() {
	stopOnlinePlayerLogTask();

	nameMap = nullptr;

	permissionLevelList->removeAll();
	permissionLevelList = nullptr;

	jukeboxSongs.removeAll();

	questInfo.removeAll();
}

void PlayerManagerImplementation::loadNameMap() {
	info("loading character names");

#ifndef WITH_SWGREALMS_API
	try {
		String query = "SELECT character_oid, firstname FROM characters where character_oid > 16777216 and galaxy_id = " + String::valueOf(server->getGalaxyID()) + " order by character_oid asc";

		UniqueReference<ResultSet*> res(ServerDatabase::instance()->executeQuery(query));

		while (res->next()) {
			uint64 oid = res->getUnsignedLong(0);
			String firstName = res->getString(1);

			if (!nameMap->put(firstName.toLowerCase(), oid)) {
				error("error coliding name:" + firstName.toLowerCase());
			}
		}

	} catch (const Exception& e) {
		fatal(e.getMessage());
	}
#else // WITH_SWGREALMS_API
	auto swgRealmsAPI = SWGRealmsAPI::instance();
	if (swgRealmsAPI != nullptr) {
		String errorMessage;
		if (!swgRealmsAPI->loadCharacterNamesBlocking(server->getGalaxyID(), *nameMap, errorMessage)) {
			error("Failed to load character names via API: " + errorMessage);
		}
	}
#endif // WITH_SWGREALMS_API

	info(true) << "loaded " << nameMap->size() << " character names in memory";
}

void PlayerManagerImplementation::writePlayerLogEntry(JSONSerializationType& logEntry) {
	FileWriter* logFile = playerLogger.getFileLogger();
	StringBuffer logLine;

	logLine << logEntry.dump().c_str() << "\n";

	Locker lock(&playerLoggerMutex);

	(*logFile) << logLine;

	logFile->flush();

	// Check for log rotation
	if (--playerLoggerLines <= 0) {
		playerLoggerLines = ConfigManager::instance()->getMaxLogLines();

		Time now;
		StringBuffer archiveFilename;
		archiveFilename << "log/player-" << now.getMiliTime() << ".log";

		// If the rename failed its ok because we open with append below
		int err = std::rename(playerLoggerFilename.toCharArray(), archiveFilename.toString().toCharArray());

		// report failure if any
		if (err != 0)
			error() << "Failed to archive player log to " << archiveFilename.toString() << " err = " << err;

		playerLogger.setFileLogger(playerLoggerFilename, true);
	}
}

JSONSerializationType PlayerManagerImplementation::basePlayerLogEntry(CreatureObject* creature, PlayerObject* ghost) {
	JSONSerializationType logEntry;
	Time time;

	logEntry["@timestamp"] = time.getFormattedTimeFull().toCharArray();
	logEntry["time_msecs"] = time.getMiliTime();

	Thread* currentThread = Thread::getCurrentThread();

	if (currentThread != nullptr)
		logEntry["thread"] = currentThread->getName();

	if (creature != nullptr) {
		Locker locker(creature);
		logEntry["oid"] = creature->getObjectID();
		logEntry["first_name"] = creature->getFirstName();
	} else {
		logEntry["oid"] = ghost != nullptr ? ghost->getObjectID() : 0;
		logEntry["first_name"] = "-ghost-";
	}

	if (ghost != nullptr) {
		Locker locker(ghost);
		logEntry["account_id"] = ghost->getAccountID();
		logEntry["played_ms"] = ghost->getPlayedMiliSecs();
		logEntry["session_ms"] = ghost->getSessionMiliSecs();
	}

	logEntry["uptime_secs"] = playerLogger.getElapsedTime();

	return logEntry;
}

void PlayerManagerImplementation::writePlayerLog(CreatureObject* creature, PlayerObject* ghost, const String& msg, int logLevelType) {
	if (logLevelType > ghost->getLogLevel())
		return;

	JSONSerializationType logEntry = basePlayerLogEntry(creature, ghost);

	logEntry["type"] = "log";
	logEntry["log"] = msg;
	logEntry["log_level"] = playerLogger.getLogType((Logger::LogLevel)logLevelType);
	logEntry["log_tag"] = playerLogger.getLoggingName();

	// Add additional info on error
	if (logLevelType == Logger::LogLevel::ERROR) {
		logEntry["worldPositionX"] = (int)creature->getWorldPositionX();
		logEntry["worldPositionZ"] = (int)creature->getWorldPositionZ();
		logEntry["worldPositionY"] = (int)creature->getWorldPositionY();

		if (creature != nullptr) {
			ManagedReference<SceneObject*> parent = creature->getParent().get();

			if (parent != nullptr) {
				logEntry["parentOID"] = parent->getObjectID();

				if (parent->isCellObject()) {
					logEntry["positionX"] = (int)creature->getPositionX();
					logEntry["positionZ"] = (int)creature->getPositionZ();
					logEntry["positionY"] = (int)creature->getPositionY();
				}
			}

			auto zone = creature->getZone();

			logEntry["zone"] = zone != nullptr ? zone->getZoneName() : "null";
		}
	}

	writePlayerLogEntry(logEntry);
}

void PlayerManagerImplementation::writePlayerLog(PlayerObject* ghost, const String& msg, int logLevelType) {
	if (ghost == nullptr)
		return;

	auto parentSceneO = ghost->getParent().get();

	if (parentSceneO == nullptr || !parentSceneO->isCreatureObject())
		return;

	CreatureObject* player = parentSceneO->asCreatureObject();

	if (player == nullptr)
		return;

	PlayerManagerImplementation::writePlayerLog(player, ghost, msg, logLevelType);
}

void PlayerManagerImplementation::writePlayerLog(CreatureObject* creature, const String& msg, int logLevelType) {
	if (creature == nullptr)
		return;

	Reference<PlayerObject*> ghost = creature->getPlayerObject();

	if (ghost == nullptr)
		return;

	PlayerManagerImplementation::writePlayerLog(creature, ghost, msg, logLevelType);
}

int PlayerManagerImplementation::getPlayerQuestID(const String& name) {
	for (int i = 0; i < questInfo.size(); ++i) {
		QuestInfo* quest = questInfo.get(i);

		if (quest != nullptr && quest->getQuestName().hashCode()== name.hashCode())
			return i;
	}

	return -1;
}

QuestTasks* PlayerManagerImplementation::getQuestTasks(const unsigned int questCrc) {
	Locker locker(&questTaskMutex);

	if (!questCrcTable.contains(questCrc)) {
		error("QuestTasks with CRC 0x" + String::hexvalueOf(questCrc) + " not found.");
		return nullptr;
	} else {
		if (!questTasksCache.contains(questCrc)) {
			String questPath = "datatables/questtask/" + questCrcTable.get(questCrc) + ".iff";

			TemplateManager* templateManager = TemplateManager::instance();

			IffStream* iffStream = templateManager->openIffFile(questPath);

			if (iffStream == nullptr) {
				error("QuestTask at " + questPath + " cannot be found.");
				return nullptr;
			}

			DataTableIff dtable;
			dtable.readObject(iffStream);

			delete iffStream;

			QuestTasks* questTasks = new QuestTasks();
			try {
				questTasks->parseDataTable(dtable);

				questTasksCache.put(questCrc, questTasks);
			}
			catch (UnknownDatatableException& e) {
				error() << "Parsing of " << questPath << " - " << e.getMessage();
				return nullptr;
			}
		}

		Reference<QuestTasks*> questTasks = questTasksCache.get(questCrc);
		return questTasks;
	}
}

String PlayerManagerImplementation::getPlayerQuestParent(int questID) {
	QuestInfo* quest = questInfo.get(questID);

	if (quest != nullptr)
		return quest->getQuestParent();
	else
		return "";
}

bool PlayerManagerImplementation::existsName(const String& name) {
	bool res = false;

	try {
		res = nameMap->containsKey(name.toLowerCase());
	} catch (DatabaseException& e) {
		error(e.getMessage());
	}

	return res;
}

bool PlayerManagerImplementation::existsPlayerCreatureOID(uint64 oid) {
	return nameMap->containsOID(oid);
}

bool PlayerManagerImplementation::kickUser(const String& name, const String& admin, String& reason, bool doBan) {
	Reference<CreatureObject*> player = getPlayer(name);

	if (player == nullptr)
		return false;

	Reference<CreatureObject*> adminPlayer = getPlayer(admin);

	if (adminPlayer == nullptr)
		return false;

	info(true) << admin << (doBan ? " kickbanned " : " kicked ") << name << " for '" << reason << "'";

	StringBuffer buf;
	buf << "You have been kicked by " << admin << " for '" << reason << "'";
	auto kickMessage = buf.toString();

	player->sendSystemMessage(kickMessage);

	ErrorMessage* errmsg = new ErrorMessage(admin, kickMessage, 0);
	player->sendMessage(errmsg);

	Reference<PlayerObject*> ghost = player->getSlottedObject("ghost").castTo<PlayerObject*>();

	if (ghost != nullptr)
		ghost->setLoggingOut();

	player->sendMessage(new LogoutMessage());

	Core::getTaskManager()->scheduleTask([
			this, playerCreo = Reference<CreatureObject*>(player), adminCreo = Reference<CreatureObject*>(adminPlayer), reason, doBan] {

		Reference<ZoneClientSession*> session = playerCreo->getClient();

		if (session != nullptr)
			session->disconnect(true);

		Reference<PlayerObject*> ghost = playerCreo->getSlottedObject("ghost").castTo<PlayerObject*>();

		if (ghost == nullptr)
			return;

		Reference<Account*> account = ghost->getAccount();

		if (account == nullptr)
			return;

		if (doBan) {
			Reference<PlayerObject*> adminGhost = adminCreo->getSlottedObject("ghost").castTo<PlayerObject*>();

			if (adminGhost != nullptr) {
				String banMessage = banAccount(adminGhost, account, 60 * 10, reason);

				adminCreo->sendSystemMessage(banMessage);
			}
		} else {
			adminCreo->sendSystemMessage("Kicked " + playerCreo->getFirstName() + " on account " + account->getUsername() + ".");
		}
	}, "kickUserTask", 500);

	return true;
}

Reference<CreatureObject*> PlayerManagerImplementation::getPlayer(const String& name) {
	uint64 oid = 0;

	try {
		oid = nameMap->get(name.toLowerCase());
	} catch (ArrayIndexOutOfBoundsException& ex) {
		// `oid` is initialized with zero so the method will return nullptr after unlocking.
		error("Didn't find player " +  name);
	}

	if (oid == 0)
		return nullptr;

	Reference<SceneObject*> obj = server->getObject(oid);

	if (obj == nullptr || !obj->isPlayerCreature())
		return nullptr;

	return obj.castTo<CreatureObject*>();
}

uint64 PlayerManagerImplementation::getObjectID(const String& name) {
	uint64 oid = 0;

	oid = nameMap->get(name.toLowerCase());

	return oid;
}

String PlayerManagerImplementation::getPlayerName(uint64 oid) {
	return nameMap->get(oid);
}

bool PlayerManagerImplementation::checkPlayerName(ClientCreateCharacterCallback* callback) {
	auto client = callback->getClient();

	NameManager* nm = processor->getNameManager();
	BaseMessage* msg = nullptr;

	String firstName;

	UnicodeString unicodeName;
	callback->getCharacterName(unicodeName);

	String name = unicodeName.toString();

	//Get the firstname
	int idx = name.indexOf(" ");
	if (idx != -1)
		firstName = name.subString(0, idx);
	else
		firstName = name;

	//Does this name already exist?
	if (nameMap->containsKey(firstName.toLowerCase())) {
		msg = new ClientCreateCharacterFailed("name_declined_in_use");
		client->sendMessage(msg);

		return false;
	}

	//Check to see if name is valid
	int res = nm->validateName(name, callback->getSpecies());

	if (res != NameManagerResult::ACCEPTED) {
		switch (res) {
		case NameManagerResult::DECLINED_EMPTY:
			msg = new ClientCreateCharacterFailed("name_declined_empty");
			break;
		case NameManagerResult::DECLINED_DEVELOPER:
			msg = new ClientCreateCharacterFailed("name_declined_developer");
			break;
		case NameManagerResult::DECLINED_FICT_RESERVED:
			msg = new ClientCreateCharacterFailed("name_declined_fictionally_reserved");
			break;
		case NameManagerResult::DECLINED_PROFANE:
			msg = new ClientCreateCharacterFailed("name_declined_profane");
			break;
		case NameManagerResult::DECLINED_RACE_INAPP:
			msg = new ClientCreateCharacterFailed("name_declined_racially_inappropriate");
			break;
		case NameManagerResult::DECLINED_SYNTAX:
			msg = new ClientCreateCharacterFailed("name_declined_syntax");
			break;
		case NameManagerResult::DECLINED_RESERVED:
			msg = new ClientCreateCharacterFailed("name_declined_reserved");
			break;
		default:
			msg = new ClientCreateCharacterFailed("name_declined_retry");
			break;
		}

		client->sendMessage(msg); //Name failed filters
		return false;
	}

	return true;
}

String PlayerManagerImplementation::setFirstName(CreatureObject* creature, const String& newFirstName, bool skipVerify) {
    if (creature == nullptr)
		return "nullptr creature specified";

	if (!creature->isPlayerCreature())
		return "Can only set FirstName on players.";

	ManagedReference<PlayerObject*> ghost = creature->getPlayerObject();

	if (ghost == nullptr)
		return "missing ghost";

	if (newFirstName.isEmpty())
		return "Empty first name is now allowed";

	if (existsName(newFirstName))
		return "That name is already in use";

	Locker locker(creature, ghost);

	if (!skipVerify) {
		auto nameManager = processor->getNameManager();

		int result = nameManager->validateName(newFirstName, creature->getSpecies());

		switch (result) {
		case NameManagerResult::ACCEPTED:
			break;
		case NameManagerResult::DECLINED_EMPTY:
			return "First names may not be empty.";
			break;
		case NameManagerResult::DECLINED_RACE_INAPP:
			return "That name is inappropriate for the player's species.";
			break;
		case NameManagerResult::DECLINED_PROFANE:
			return "That name is profane.";
			break;
		case NameManagerResult::DECLINED_DEVELOPER:
			return "That is a developer's name.";
			break;
		case NameManagerResult::DECLINED_FICT_RESERVED:
			return "That name is a reserved fictional name.";
			break;
		case NameManagerResult::DECLINED_RESERVED:
			return "That name is reserved.";
			break;
		case NameManagerResult::DECLINED_SYNTAX:
			return "That name contains invalid syntax.";
			break;
		}
	}

	String oldFirstName = creature->getFirstName();
	String oldLastName = creature->getLastName();
	String newFullName = newFirstName;

	if (!oldLastName.isEmpty())
		newFullName = newFirstName + " " + oldLastName;

	creature->setCustomObjectName(newFullName, true);

	// If staff fix their staff tags
	if (ghost->hasGodMode())
		updatePermissionName(creature, ghost->getAdminLevel());

	auto chatManager = server->getChatManager();
	chatManager->removePlayer(oldFirstName);
	chatManager->addPlayer(creature);

	removePlayer(oldFirstName);
	addPlayer(creature);

	// Remove the old name from other people's friends lists
	ghost->removeAllReverseFriends(oldFirstName);

#ifndef WITH_SWGREALMS_API
	// Update mysql characters table
	String characterFirstName = creature->getFirstName();
	Database::escapeString(characterFirstName);

	int galaxyID = server->getGalaxyID();

	StringBuffer charDirtyQuery;
	charDirtyQuery
			<< "UPDATE `characters_dirty` SET `firstname` = '"  << characterFirstName
			<< "' WHERE `character_oid` = '" << creature->getObjectID()
			<< "' AND `galaxy_id` = '" << galaxyID << "'";

	ServerDatabase::instance()->executeStatement(charDirtyQuery);

	StringBuffer charQuery;
	charQuery
			<< "UPDATE `characters` SET `firstname` = '"  << characterFirstName
			<< "' WHERE `character_oid` = '" << creature->getObjectID()
			<< "' AND `galaxy_id` = '" << galaxyID << "'";

	ServerDatabase::instance()->executeStatement(charQuery);
#else // WITH_SWGREALMS_API
	auto swgRealmsAPI = SWGRealmsAPI::instance();
	String errorMessage;

	if (!swgRealmsAPI->updateCharacterFirstNameBlocking(
			creature->getObjectID(),
			server->getGalaxyID(),
			creature->getFirstName(),
			errorMessage)) {
		error("Failed to update character firstname: " + errorMessage);
		return "API error: " + errorMessage;
	}
#endif // WITH_SWGREALMS_API

	// Success, return empty string
	return "";
}

String PlayerManagerImplementation::setLastName(CreatureObject* creature, const String& newLastName, bool skipVerify) {
	if (creature == nullptr)
		return "nullptr creature specified";

	if (!creature->isPlayerCreature())
		return "Can only set LastName on players.";

	ManagedReference<PlayerObject*> ghost = creature->getPlayerObject();

	if (ghost == nullptr)
		return "missing ghost";

	Locker locker(creature, ghost);

	String oldFirstName = creature->getFirstName();
	String newFullName = oldFirstName;

	if (!newLastName.isEmpty())
		newFullName = oldFirstName + " " + newLastName;

	if (!skipVerify) {
		auto nameManager = processor->getNameManager();

		int result = nameManager->validateName(newFullName, creature->getSpecies());

		switch (result) {
		case NameManagerResult::ACCEPTED:
			break;
		case NameManagerResult::DECLINED_RACE_INAPP:
			return "That name is inappropriate for the player's species.";
			break;
		case NameManagerResult::DECLINED_PROFANE:
			return "That name is profane.";
			break;
		case NameManagerResult::DECLINED_DEVELOPER:
			return "That is a developer's name.";
			break;
		case NameManagerResult::DECLINED_FICT_RESERVED:
			return "That name is a reserved fictional name.";
			break;
		case NameManagerResult::DECLINED_RESERVED:
			return "That name is reserved.";
			break;
		case NameManagerResult::DECLINED_SYNTAX:
			return "That name contains invalid syntax.";
			break;
		}
	}

	creature->setCustomObjectName(newFullName, true);

	// If staff fix their staff tags
	if (ghost->hasGodMode())
		updatePermissionName(creature, ghost->getAdminLevel());

	// Update mysql characters table
	int galaxyID = server->getGalaxyID();

#ifndef WITH_SWGREALMS_API
	String characterLastName = creature->getLastName();
	Database::escapeString(characterLastName);

	StringBuffer charDirtyQuery;
	charDirtyQuery
			<< "UPDATE `characters_dirty` SET `surname` = '"  << characterLastName
			<< "' WHERE `character_oid` = '" << creature->getObjectID()
			<< "' AND `galaxy_id` = '" << galaxyID << "'";

	ServerDatabase::instance()->executeStatement(charDirtyQuery);

	StringBuffer charQuery;
	charQuery
			<< "UPDATE `characters` SET `surname` = '"  << characterLastName
			<< "' WHERE `character_oid` = '" << creature->getObjectID()
			<< "' AND `galaxy_id` = '" << galaxyID << "'";

	ServerDatabase::instance()->executeStatement(charQuery);
#else // WITH_SWGREALMS_API
	auto swgRealmsAPI = SWGRealmsAPI::instance();
	String errorMessage;

	if (!swgRealmsAPI->updateCharacterSurNameBlocking(
			creature->getObjectID(),
			galaxyID,
			creature->getLastName(),
			errorMessage)) {
		error("Failed to update character surname: " + errorMessage);
		return "API error: " + errorMessage;
	}
#endif // WITH_SWGREALMS_API

	// Success, return empty string
	return "";
}

void PlayerManagerImplementation::createTutorialBuilding(CreatureObject* player) {
	// Infinity: Clear tutorialStarted flag so TutorialScreenPlay::start() respawns NPCs
	// in the new building. Without this, logging back in within the same server session
	// after the building is destroyed leaves the player in an empty hall (#1470).
	DirectorManager::instance()->deleteSharedMemory(String::valueOf(player->getObjectID()) + ":tutorialStarted");

	auto zone = server->getZone("tutorial");

	if (zone == nullptr) {
		// Infinity: Tutorial zone disabled — fall back to first available starting location
		warning() << "[CharCreate] Tutorial zone is NULL — falling back to starting location";

		for (int i = 0; i < startingLocationList.size(); ++i) {
			StartingLocation* startLoc = &startingLocationList.get(i);

			if (startLoc == nullptr) {
				continue;
			}

			auto fallbackZone = server->getZone(startLoc->getZoneName());

			if (fallbackZone != nullptr) {
				warning() << "[CharCreate] Fallback to " << startLoc->getZoneName() << " at " << startLoc->getX() << "," << startLoc->getZ() << "," << startLoc->getY();
				player->switchZone(startLoc->getZoneName(), startLoc->getX(), startLoc->getZ(), startLoc->getY(), startLoc->getCell());
				player->updateToDatabase();
				return;
			}
		}

		error() << "[CharCreate] FAILED — tutorial zone and all starting location zones disabled.";
		return;
	}

	Reference<TutorialBuildingObject*> tutorial = server->createObject(STRING_HASHCODE("object/building/general/newbie_hall.iff"), 0).castTo<TutorialBuildingObject*>();

	if (tutorial == nullptr) {
		error() << "[CharCreate] FAILED — unable to create newbie_hall building";
		return;
	}

	Locker tutClocker(tutorial, player);

	tutorial->createCellObjects();
	tutorial->setPublicStructure(true);
	tutorial->setTutorialOwnerID(player->getObjectID());

	tutorial->initializePosition(System::random(5000), 0, System::random(5000));

	if (!zone->transferObject(tutorial, -1)) {
		warning() << "[CharCreate] FAILED — zone->transferObject returned false for tutorial building";
		tutorial->destroyObjectFromWorld(true);

		return;
	}

	// Infinity: Use spawningPoints from newbie_hall.lua — cell 5 at (7.61, -7.0, -56.48)
	player->initializePosition(7.61262f, -6.99999f, -56.4838f);

	SceneObject* tutorialCell = tutorial->getCell(5);

	if (tutorialCell == nullptr) {
		warning() << "[CharCreate] FAILED — tutorial->getCell(5) returned null, total cells: " << tutorial->getTotalCellNumber();
		tutorial->destroyObjectFromWorld(true);

		return;
	}

	tutClocker.release();

	auto ghost = player->getPlayerObject();

	if (ghost != nullptr) {
		ghost->setTutorialParticpant();
	}

	uint64 cellID = tutorialCell->getObjectID();

	player->switchZone("tutorial", 7.61262f, -6.99999f, -56.4838f, cellID);

	player->updateToDatabase();
}

void PlayerManagerImplementation::insertIntoSkippedTutorialBuilding(CreatureObject* player) {
	// Infinity: Skip-tutorial players go to the first starting location from starting_locations.iff.
	// The previous approach created a per-player building with a travel terminal, but the terminal
	// was non-functional: getPlanetTravelPoint() returns null for the tutorial zone, so the terminal
	// silently failed and players were stuck in the tutorial zone forever (ticket-5858).

	if (startingLocationList.getTotalLocations() > 0) {
		StartingLocation* startLoc = &startingLocationList.get(0);

		player->switchZone(startLoc->getZoneName(), startLoc->getX(), startLoc->getZ(), startLoc->getY(), startLoc->getCell());
	} else {
		error() << "[CharCreate] insertIntoSkippedTutorialBuilding — no starting locations loaded, cannot place player " << player->getObjectID();
		return;
	}

	player->updateToDatabase();
}

uint8 PlayerManagerImplementation::calculateIncapacitationTimer(CreatureObject* playerCreature, int condition) {
	// Switch the sign of the value
	int value = -condition;

	if (value < 5)
		return 5;

	int recoveryTime = (value / 5); // In seconds - 3 seconds is recoveryEvent timer

	// Recovery time cannot be higher than 60 seconds.
	recoveryTime = (recoveryTime > 60) ? 60 : recoveryTime;

	//Check for incap recovery food buff - overrides recovery time gate.
	if (playerCreature->hasBuff(STRING_HASHCODE("food.incap_recovery"))) {
		Buff* buff = playerCreature->getBuff(STRING_HASHCODE("food.incap_recovery"));

		if (buff != nullptr) {
			float percent = buff->getSkillModifierValue("incap_recovery");

			recoveryTime = round(recoveryTime * ((100.0f - percent) / 100.0f));

			StringIdChatParameter message("combat_effects", "incap_recovery");
			message.setDI(recoveryTime);
			playerCreature->sendSystemMessage(message); // Incapacitation recovery time reduced by %DI%.

			Locker buffLock(buff, playerCreature);
			playerCreature->removeBuff(buff);
		}
	}

	return recoveryTime;
}

int PlayerManagerImplementation::notifyDestruction(TangibleObject* destructor, TangibleObject* destructedObject, int condition, bool isCombatAction) {
	fatal(destructor, "destructor cant be nullptr.");

	if (!destructedObject->isPlayerCreature())
		return 1;

	CreatureObject* playerCreature = cast<CreatureObject*>(destructedObject);
	if (playerCreature == nullptr)
		return 1;

	if ((playerCreature->isIncapacitated() && !(playerCreature->isFeigningDeath())) || playerCreature->isDead())
		return 1;

	if (playerCreature->isRidingMount()) {
		playerCreature->updateCooldownTimer("mount_dismount", 0);
		playerCreature->executeObjectControllerAction(STRING_HASHCODE("dismount"));
	}

	ManagedReference<PlayerObject*> ghost = playerCreature->getPlayerObject();

	if (ghost == nullptr)
		return 1;

	ghost->addIncapacitationTime();

	const DeltaVector<ManagedReference<SceneObject*> >* defenderList = destructor->getDefenderList();

	bool isDefender = false;

	if (defenderList->contains(destructedObject)) {
		isDefender = true;
		destructor->removeDefender(destructedObject);
	}

	if (defenderList->size() == 0) {
		destructor->clearCombatState(false);
	}

	int pvpDeath = ((destructor->isPlayerCreature() && destructor != destructedObject) || destructor->isPet()) ? 1 : 0;   //Infinity:  Not a PVP death if you kill yourself through stupidity

	if (pvpDeath) {  //Infinity:  Let's log incaps by Bounty Hunters to catch the cheaters who are letting Jedi get the DB to stay off terms

		ManagedReference<CreatureObject*> destructorCreature = destructor->asCreatureObject();	

		if (destructorCreature != nullptr && destructorCreature->isPet()) {
			destructorCreature = destructorCreature->getLinkedCreature().get();
		}

		if (destructorCreature != nullptr && destructorCreature->hasBountyMissionFor(playerCreature)) {
			info("Bounty Hunter " + destructorCreature->getDisplayedName() + " incapped player target " + playerCreature->getDisplayedName() + ".", true);
		}
	}

	if (ghost->getIncapacitationCounter() >= 3) {
		killPlayer(destructor, playerCreature, pvpDeath, isCombatAction);
	}
	else {
		playerCreature->setPosture(CreaturePosture::INCAPACITATED, true, true);
		playerCreature->clearCombatState(false);
		playerCreature->clearState(CreatureState::FEIGNDEATH); // We got incapped for real - Remove the state so we can be DB'd

		uint8 incapTime = calculateIncapacitationTimer(playerCreature, condition);
		playerCreature->setIncapacitationTimer((uint32) incapTime, true);

		Reference<Task*> oldTask = playerCreature->getPendingTask("incapacitationRecovery");

		if (oldTask != nullptr && oldTask->isScheduled()) {
			oldTask->cancel();
			playerCreature->removePendingTask("incapacitationRecovery");
		}

		Reference<Task*> task = new PlayerIncapacitationRecoverTask(playerCreature, false);
		playerCreature->addPendingTask("incapacitationRecovery", task, incapTime * 1000);

		StringIdChatParameter toVictim;

		toVictim.setStringId("base_player", "prose_victim_incap");
		toVictim.setTT(destructor->getDisplayedName());

		playerCreature->sendSystemMessage(toVictim);

		if (destructor->isPlayerCreature()) {
			ManagedReference<CreatureObject*> destructorCreature = destructor->asCreatureObject();

			PlayerObject* attackerGhost = destructorCreature->getPlayerObject();
			PlayerObject* victimGhost = playerCreature->getPlayerObject();

			if (attackerGhost != nullptr && victimGhost != nullptr) {

				StringIdChatParameter toKiller;

				toKiller.setStringId("base_player", "prose_target_incap");
				toKiller.setTT(playerCreature->getDisplayedName());

				destructorCreature->sendSystemMessage(toKiller);
			}
		}
	}

	if (playerCreature->isWatching()) {
		uint64 watchID = playerCreature->getWatchToID();
		stopWatch(playerCreature, watchID, false);
	} else if (playerCreature->isListening()) {
		uint64 listenID = playerCreature->getListenID();
		stopListen(playerCreature, listenID, false);
	}

	ThreatMap* destructorThreatMap = destructor->getThreatMap();

	if (destructorThreatMap != nullptr) {
		for (int i = 0; i < destructorThreatMap->size(); i++) {
			TangibleObject* destructedTano = destructorThreatMap->elementAt(i).getKey();

			if (destructedTano == destructedObject) {
				destructorThreatMap->remove(i);
			}
		}
	}

	return 0;
}

void PlayerManagerImplementation::killPlayer(TangibleObject* attacker, CreatureObject* player, int typeofdeath, bool isCombatAction) {
	if (player == nullptr) {
		error() << __FILE__ << ":" << __FUNCTION__ << "()" << " player is nullptr";
		return;
	}

	if (attacker == nullptr) { //Infinity
		error() << __FILE__ << ":" << __FUNCTION__ << "()" << " attacker is nullptr";
		return;
	}

	if (player->isDead()) {
		error() << __FILE__ << ":" << __FUNCTION__ << "()" << " player is already dead";	
		return;
	}
	
	if (player->getTimeOfDeath()->miliDifference() < 1000) {
		error() << __FILE__ << ":" << __FUNCTION__ << "()" << " players can't die more than once per second";	
		return;
	}

	player->updateTimeOfDeath();	//Doing this upfront to prevent multiple simultaneous deaths

	StringIdChatParameter stringId;

	ManagedReference<PlayerObject*> ghost = player->getPlayerObject();  //Infinity:  Let's get ghost up front it is used everywhere

	//Infinity:  Let's grab the owner right now if attacker is pet
	ManagedReference<CreatureObject*> attackerCreature = attacker->asCreatureObject();
	if (attackerCreature != nullptr && attackerCreature->isPet()) {
		ManagedReference<CreatureObject*> owner = attackerCreature->getLinkedCreature().get();
		if (owner != nullptr && owner->isPlayerCreature()) {
			attackerCreature = owner;
		}
	}

	if (!attacker->isPlayerCreature() || (attackerCreature != nullptr && !CombatManager::instance()->areInDuel(attackerCreature, player))) {
		TransactionLog trx(attacker, player, TrxCode::PLAYERDIED);
		trx.addState("isCombatAction", isCombatAction);
		trx.addState("typeofdeath", typeofdeath);
		trx.addState("attackerFaction", attacker->getFaction());
		trx.addState("attackerIsPlayerCreature", attacker->isPlayerCreature());
		trx.addState("attackerIsPet", attacker->isPet());
		trx.addState("playerFaction", player->getFaction());
		trx.addState("playerIsRidingMount", player->isRidingMount());
		trx.addState("playerPosture", CreaturePosture::postureToString(player->getPosture()));

		if (ghost != nullptr) {
			trx.addState("playerCloningFacilityID", ghost->getCloningFacility());
			trx.addState("playerIsJedi", ghost->isJedi());
			trx.addState("playerIsLightJedi", ghost->isJediLight());
			trx.addState("playerIsDarkJedi", ghost->isJediDark());
			trx.addState("playerPvpRating", ghost->getPvpRating());
		}

		if (attacker->isPlayerCreature()) {

			if (attackerCreature != nullptr) {
				trx.addState("attackerIsHuntingPlayer", attackerCreature->hasBountyMissionFor(player));
				trx.addState("playerIsHuntingAttacker", player->hasBountyMissionFor(attackerCreature));

				ManagedReference<PlayerObject*> attackerGhost = attackerCreature->getPlayerObject();

				if (attackerGhost != nullptr) {
					trx.addState("attackerIsJedi", attackerGhost->isJedi());
					trx.addState("attackerIsLightJedi", attackerGhost->isJediLight());
					trx.addState("attackerIsDarkJedi", attackerGhost->isJediDark());
					trx.addState("attackerPvpRating", attackerGhost->getPvpRating());
				}
			}
		}
	}

	if (player->isRidingMount()) {
		player->updateCooldownTimer("mount_dismount", 0);
		player->executeObjectControllerAction(STRING_HASHCODE("dismount"));
	}

	if (typeofdeath == 1 && ghost != nullptr) { //Infinity:   1 = PVP death, store all active pets

		Vector<ManagedReference<ControlDevice*> > petsToStore;
		for (int i = 0; i < ghost->getActivePetsSize(); i++) {
			ManagedReference<AiAgent*> pet = ghost->getActivePet(i);
			if (pet != nullptr) {
				ManagedReference<PetControlDevice*> controlDevice = pet->getControlDevice().get().castTo<PetControlDevice*>();
				if (controlDevice != nullptr) {
					petsToStore.add(controlDevice.castTo<ControlDevice*>());
				}
			}
		}

		Reference<StoreSpawnedChildrenTask*> task = new StoreSpawnedChildrenTask(player, std::move(petsToStore));
		task->execute();
	}

	player->clearCombatState(true);

	player->clearDots();

	player->setPosture(CreaturePosture::DEAD, !isCombatAction, !isCombatAction);

	sendActivateCloneRequest(player, typeofdeath);

	stringId.setStringId("base_player", "prose_victim_dead");
	stringId.setTT(attacker->getDisplayedName());
	player->sendSystemMessage(stringId);

	bool playerBuffed = player->hasBuff(BuffCRC::MEDICAL_ENHANCE_HEALTH);

	player->clearBuffs(true, false);

	if (ghost != nullptr) {
		ghost->resetIncapacitationTimes();
		if (ghost->hasTef()) {
			ghost->schedulePvpTefRemovalTask(true, true, true);
		}
	}

	//Player Bounties
	bool bountyOption = false;

	if (typeofdeath == 1 && attackerCreature != nullptr && attackerCreature != player) {//Only enable player bounty if PVP Death and player didn't kill themselves

		auto attackerGhost = attackerCreature->getPlayerObject();

		if (attackerGhost != nullptr && !attackerGhost->hasGodMode()) {

			String killerName = attackerCreature->getFirstName();

			if (ghost != nullptr) {
				bountyOption = true;
				Time currentTime;
				uint64 currentTimeMili = currentTime.getMiliTime();
				ghost->setScreenPlayData("pvp", "last_death_timestamp", String::valueOf(currentTimeMili));
				ghost->setScreenPlayData("pvp", "last_killer", killerName);
			}

			ManagedReference<SuiInputBox*> box = new SuiInputBox(player, SuiWindowType::OBJECT_NAME);
			box->setPromptTitle("You have died in PVP");
			box->setPromptText("Place a bounty on your killer, " + killerName + ". Enter an amount between 25,000 and 2,500,000 credits. The Bounty Hunter Guild will take 20% for its fee and your target will be added to our boards immediately.");
			box->setMaxInputSize(128);
			box->setCancelButton(true, "@no");
			box->setOkButton(true, "@yes");
			box->setUsingObject(player);
			box->setCallback(new BountyHuntSuiCallback(player->getZoneServer()));
			ghost->addSuiBox(box);
			player->sendMessage(box->generateMessage());
		}
	}

	ThreatMap* threatMap = player->getThreatMap();

	if (attackerCreature != nullptr && attackerCreature->isPlayerCreature() && attackerCreature != player) {  //Only do PVP faction points and broadcast checks if we have a valid player creature, and not killing yourself

		if (attacker->getFaction() != 0) {
			if (!CombatManager::instance()->areInDuel(attackerCreature, player)) {
				FactionManager::instance()->awardPvpFactionPoints(attackerCreature, player);
			}
		}

		ThreatMap* attackerThreatMap = attackerCreature->getThreatMap();
		if (attackerThreatMap != nullptr) {
			info("PVPDEATH:  Total damage done to attacker " + attackerCreature->getDisplayedName() + " by victim " + player->getDisplayedName() + ": " + String::valueOf(attackerThreatMap->getTotalDamageByAttacker(player)), true);
		}

		if (ConfigManager::instance()->isPvpBroadcastChannelEnabled() && ghost != nullptr) {
			auto zoneServer = player->getZoneServer();

			if (zoneServer != nullptr) {
				ChatManager* chatManager = zoneServer->getChatManager();

				if (chatManager != nullptr) {
	
					String rebelPrefix = "Rebel Insurgent < ";
					String imperialPrefix = "Imperial Agent < ";
					String civilianPrefix = "Civilian < ";
					String lightJediPrefix = "Light Jedi < ";
					String darkJediPrefix = "Dark Jedi < ";
					String neutralJediPrefix = "Jedi < ";

					String rebelColor = "\r\\#33CCFF";
					String imperialColor = "\r\\#FF0000";
					String civilianColor = "\r\\#00FF21";
		
					// Destructed player
					String victimName = player->getDisplayedName();
					String victimPrefix, victimColor = "";
					uint32 victimFaction = player->getFaction();

					if (player->hasSkill("force_rank_light_novice")) {
						victimPrefix = lightJediPrefix;
						victimColor = rebelColor;
					}
					else if (player->hasSkill("force_rank_dark_novice")) {
						victimPrefix = darkJediPrefix;
						victimColor = imperialColor;
					}
					else {   //Non-FRS Jedi or Normie
						if ((player->hasSkill("force_title_jedi_rank_02"))) {
							victimPrefix = neutralJediPrefix;
						}
						else { //Normie
							if (victimFaction == Factions::FACTIONREBEL)
								victimPrefix = rebelPrefix;
							else if (victimFaction == Factions::FACTIONIMPERIAL)
								victimPrefix = imperialPrefix;
							else
								victimPrefix = civilianPrefix;
						}

						if (victimFaction == Factions::FACTIONREBEL)
							victimColor = rebelColor;
						else if (victimFaction == Factions::FACTIONIMPERIAL)
							victimColor = imperialColor;
						else
							victimColor = civilianColor;
					}

					// Attacker player
					String attackerName = attackerCreature->getDisplayedName();
					String attackerPrefix, attackerColor = "";
					uint32 attackerFaction = attackerCreature->getFaction();

					ManagedReference<PlayerObject*> attackerGhost = attackerCreature->getPlayerObject();
					bool godModeAttacker = false;
					if (attackerGhost != nullptr) {
						godModeAttacker = attackerGhost->hasGodMode();
					}

					if ((attackerCreature->hasSkill("force_rank_light_novice"))) {
						attackerPrefix = lightJediPrefix;
						attackerColor = rebelColor;
					}
					else if ((attackerCreature->hasSkill("force_rank_dark_novice"))) {
						attackerPrefix = darkJediPrefix;
						attackerColor = imperialColor;
					}
					else {   //Non-FRS Jedi or Normie
						if (attackerCreature->hasSkill("force_title_jedi_rank_02")) {
							attackerPrefix = neutralJediPrefix;
						}
						else { //Normie
							if (attackerFaction == Factions::FACTIONREBEL)
								attackerPrefix = rebelPrefix;
							else if (attackerFaction == Factions::FACTIONIMPERIAL)
								attackerPrefix = imperialPrefix;
							else
								attackerPrefix = civilianPrefix;
						}

						if (attackerFaction == Factions::FACTIONREBEL)
							attackerColor = rebelColor;
						else if (attackerFaction == Factions::FACTIONIMPERIAL)
							attackerColor = imperialColor;
						else
							attackerColor = civilianColor;
					}

					bool areInDuel = CombatManager::instance()->areInDuel(attackerCreature, player);

					String type = ".";

					if (areInDuel)
						type = "in a duel.";
					else if (attackerFaction != victimFaction)
						type = "in GCW combat.";

					if (godModeAttacker) {
						if (attackerCreature->getGender() == 0)
							type = "because he could.";
						else 
							type = "because she could.";
					}

					StringBuffer broadcastMsg, logMsg;
					broadcastMsg << "\r\\#FFFFFF" << attackerPrefix << attackerColor << attackerName << "\r\\#FFFFFF" << " > has defeated " << victimPrefix << victimColor << victimName << "\r\\#FFFFFF" << " > " << type;

					String victimZoneName, attackerZoneName = "ERROR";
					auto zone = player->getZone();
					if (zone != nullptr) {
						victimZoneName = zone->getZoneName();
						victimZoneName[0] = toupper(victimZoneName[0]);
					}
					zone = attackerCreature->getZone();
					if (zone != nullptr) {
						attackerZoneName = zone->getZoneName();
						attackerZoneName[0] = toupper(attackerZoneName[0]);
					}

					logMsg << attackerPrefix << attackerName << " > has defeated " << victimPrefix << victimName << " > " << type << "  ";
					String petName = attacker->isPet() ? " [" + attacker->getDisplayedName() +  "]" : "";
					logMsg << "Killer zone = " << attackerZoneName << ".  Killer " << attackerName << petName << " coordinates (x=" << (int)attacker->getWorldPositionX() << ", y=";
					logMsg << (int)attacker->getWorldPositionY() << ", z=" << (int)attacker->getWorldPositionZ() << ", cellID = " << attacker->getParentID() << ").  ";
					logMsg << "Victim zone = " << victimZoneName << ".  Victim " << victimName << " coordinates (x=" << (int)player->getWorldPositionX() << ", y=";
					logMsg << (int)player->getWorldPositionY() << ", z=" << (int)player->getWorldPositionZ() << ", cellID = " << player->getParentID() << ").  ";
					logMsg << "Victim buffed = " << playerBuffed << ".";
					// Infinity: warning() lands at LogFileLevel=3 (info() doesn't),
					// so PvP kills now appear in core3.log alongside the chatroom
					// broadcast — closes the silent-PvP-fight-club gap.
					warning(logMsg);

					// Infinity: Persist the PvP kill in the trx log so anti-cheat
					// can detect fight-club farming patterns (same attacker/victim
					// pair repeated). The existing TrxCode::COMBATSTATS path in
					// disseminateExperience() only fires for NPC kills, so without
					// this entry PvP kills would be invisible to trx queries.
					if (attackerCreature->isPlayerCreature()) {
						TransactionLog pvpTrx(attackerCreature, player, TrxCode::COMBATSTATS);
						pvpTrx.addState("combatIsPet", attacker->isPet());
						pvpTrx.addState("combatIsPvp", true);
						pvpTrx.addState("combatType", type);   // "in a duel.", "in GCW combat.", or "."
						pvpTrx.addState("combatAreInDuel", areInDuel);
						pvpTrx.addState("combatAttackerFaction", attackerFaction);
						pvpTrx.addState("combatVictimFaction", victimFaction);
						pvpTrx.addState("combatAttackerZone", attackerZoneName);
						pvpTrx.addState("combatVictimZone", victimZoneName);
						pvpTrx.addState("combatVictimBuffed", playerBuffed);
						pvpTrx.addState("combatGodModeAttacker", godModeAttacker);
						pvpTrx.addState("combatPlayerLevel", calculatePlayerLevel(attackerCreature));
					}

					UnicodeString message(broadcastMsg.toString());
					UnicodeString formattedMsg(chatManager->formatMessage(message));

					ManagedReference<ChatRoom*> pvpBroadcastRoom = chatManager->getPvpBroadcastRoom();
					if (pvpBroadcastRoom != nullptr) {
						BaseMessage* msg = new ChatRoomMessage("", server->getGalaxyName(), formattedMsg, pvpBroadcastRoom->getRoomID());
						pvpBroadcastRoom->broadcastMessage(msg);
					}
				}
			}
		}

		if (attackerCreature->isRebel()) {
			attackerCreature->playEffect("clienteffect/holoemote_rebel.cef", "head");
		}

		if (attackerCreature->isImperial()) {
			attackerCreature->playEffect("clienteffect/holoemote_imperials.cef", "head");
		}

		calculateFRS(attackerCreature, player);  //Infinity:  Have player attacker and player victim, calculate FRS

		//Infinity:  Moved this doPvpDeathRatingUpdate into same if block as calculateFRS
		ManagedReference<CreatureObject*> playerRef = player->asCreatureObject();

		stringId.setStringId("base_player", "prose_target_dead");
		stringId.setTT(player->getDisplayedName());
		playerRef->sendSystemMessage(stringId);

		Reference<ThreatMap*> copyThreatMap = new ThreatMap(*threatMap);

		Core::getTaskManager()->scheduleTask([=] () {
			if (playerRef != nullptr) {
				Locker locker(playerRef);
				doPvpDeathRatingUpdate(playerRef, copyThreatMap);
			}
		}, "PvpDeathRatingUpdateLambda", 1000);

	}	

	CombatManager::instance()->freeDuelList(player, false);

	if (player->isGrouped()) {
		ManagedReference<GroupObject*> group = player->getGroup();

		if (group != nullptr) {
			StringIdChatParameter stringId;
			stringId.setStringId("group", "notify_death"); // [GROUP] %TU has died.

			stringId.setTU(player->getDisplayedName());

			for (int i = 0; i < group->getGroupSize(); i++) {
				auto groupMember = group->getGroupMember(i);

				if (groupMember == nullptr || !groupMember->isPlayerCreature()) {
					continue;
                }

				groupMember->sendSystemMessage(stringId);
			}
		}
	}

	// Infinity: Death Watch PvP kill handling + get highest damage player for PLAYERKILLED observer
	// Both need highestDamagePlayer before the threat map is cleared below
	auto highestDamagePlayer = threatMap->getHighestDamagePlayer();
	auto effectiveKiller = (highestDamagePlayer != nullptr) ? highestDamagePlayer : attackerCreature.get();
	if (effectiveKiller != nullptr && effectiveKiller->isPlayerCreature()) {
		auto dwManager = server->getDeathWatchManager();
		if (dwManager != nullptr) {
			dwManager->handlePlayerKill(effectiveKiller, player);
		}
	}

	// Pass last-hit killer as arg1, highest damage player's ID as arg2
	uint64 highestDamagePlayerID = (highestDamagePlayer != nullptr) ? highestDamagePlayer->getObjectID() : attacker->getObjectID();
	player->notifyObservers(ObserverEventType::PLAYERKILLED, attacker, highestDamagePlayerID);


	threatMap->removeAll(true);
	player->dropFromDefenderLists();
	player->setTargetID(0, true);

	if (bountyOption) {
		player->sendSystemMessage(attackerCreature->getFirstName() + " has killed you in PVP. Use the /setBounty command in the next 30 minutes to place a bounty on your killer.");
	}

}

void PlayerManagerImplementation::sendActivateCloneRequest(CreatureObject* player, int typeofdeath) {
	if (player == nullptr) {
		error() << "Player is null in sendActivateCloneRequest";
		return;
	}

	PlayerObject* ghost = player->getPlayerObject();

	if (ghost == nullptr) {
		return;
	}

	Zone* zone = player->getZone();

	if (zone == nullptr) {
		return;
	}

	Vector3 playerPos = player->getWorldPosition();

	// Handle players death in space
	if (zone->isSpaceZone()) {
		auto launchZoneName = ghost->getSpaceLaunchZone();

		zone = server->getZone(launchZoneName);

		if (zone == nullptr || zone->isSpaceZone()) {
			return;
		}

		playerPos = ghost->getSpaceLaunchLocation();
	}

	// Death Watch Safe Cloning - intercept for DW members
	ManagedReference<DeathWatchManager*> dwManager = server->getDeathWatchManager();
	if (dwManager != nullptr && dwManager->shouldInterceptCloning(player)) {
		dwManager->sendDeathWatchCloneRequest(player, typeofdeath);
		return;
	}

	ghost->removeSuiBoxType(SuiWindowType::CLONE_REQUEST);

	ManagedReference<SuiListBox*> cloneMenu = new SuiListBox(player, SuiWindowType::CLONE_REQUEST);

	if (cloneMenu == nullptr) {
		return;
    }

	cloneMenu->setCallback(new CloningRequestSuiCallback(server, typeofdeath));
	cloneMenu->setPromptTitle("@base_player:revive_title");

	uint64 preDesignatedFacilityOid = ghost->getCloningFacility();

	ManagedReference<SceneObject*> preDesignatedFacility = server->getObject(preDesignatedFacilityOid);
	String predesignatedName = "None";

	//Get the name of the pre-designated facility
	if (preDesignatedFacility != nullptr) {
		ManagedReference<CityRegion*> cityRegion = preDesignatedFacility->getCityRegion().get();

		// We now allow pre-designated facility to be chosen galaxy-wide if PVP death
		if (preDesignatedFacility->getZone() != zone && typeofdeath == 0) {
			predesignatedName = "off-planet (unavailable)";
		}
        else if (cityRegion != nullptr) {
			predesignatedName = cityRegion->getRegionDisplayedName();
		}
        else {
			predesignatedName = preDesignatedFacility->getDisplayedName();
		}
	}

	ManagedReference<SceneObject*> closestCloning = nullptr;
	String closestName = "None";

	SortedVector<ManagedReference<SceneObject*> > locations = zone->getPlanetaryObjectList("cloningfacility");

	int checkDistanceSq = 16384 * 16384;  //Infinity:  Updated default distance

	//Infinity:  Lookup terrain size
	auto planetManager = zone->getPlanetManager();

	if (planetManager != nullptr) {
		auto terrainManager = planetManager->getTerrainManager();

		if (terrainManager != nullptr) {
			int zoneSize = terrainManager->getSize();
			checkDistanceSq = zoneSize * zoneSize;
		}
	}

	for (int j = 0; j < locations.size(); j++) {
		ManagedReference<SceneObject*> location = locations.get(j);

		if (location == nullptr) {
			continue;
        }

		ManagedReference<CityRegion*> clonerCityRegion = location->getCityRegion().get();

		if (!isValidClosestCloner(player, location, clonerCityRegion)) {
			continue;
        }

		String name = "";

		if (clonerCityRegion != nullptr) {
			name = clonerCityRegion->getRegionDisplayedName();
		} else {
			name = location->getDisplayedName();
		}

		Vector3 clonerLocation = location->getWorldPosition();
		int distanceSq = clonerLocation.squaredDistanceTo(playerPos);

		if (distanceSq < checkDistanceSq) {
			// Update distance to closest cloner to check against
			checkDistanceSq = distanceSq;

			// Store name and cloner object
			closestName = name;
			closestCloning = location;
		}
	}

	StringBuffer promptText;
	promptText << "Closest:\t\t " << closestName << "\n"
			<< "Pre-Designated: " << predesignatedName << "\n"
			<< "Cash Balance:\t " << player->getCashCredits() << "\n\n"
			<< "Select the desired option and click OK.";

	cloneMenu->setPromptText(promptText.toString());

	if (closestCloning != nullptr)
		cloneMenu->addMenuItem("@base_player:revive_closest", closestCloning->getObjectID());

	if (preDesignatedFacility != nullptr && (preDesignatedFacility->getZone() == zone || typeofdeath == 1))    //We now allow pre-designated facility to be chosen galaxy-wide for PVP death only
		cloneMenu->addMenuItem("@base_player:revive_bind", preDesignatedFacility->getObjectID());

	for (int i = 0; i < locations.size(); i++) {
		ManagedReference<SceneObject*> loc = locations.get(i);

		if (loc == nullptr)
			continue;

		CloningBuildingObjectTemplate* cbot = cast<CloningBuildingObjectTemplate*>(loc->getObjectTemplate());

		if (cbot == nullptr)
			continue;

		if (cbot->getFacilityType() == CloningBuildingObjectTemplate::JEDI_ONLY && player->hasSkill("force_title_jedi_rank_01")) {
			String name = "Force Shrine (" + String::valueOf((int)loc->getWorldPositionX()) + ", " + String::valueOf((int)loc->getWorldPositionY()) + ")";
			cloneMenu->addMenuItem(name, loc->getObjectID());
		} else if ((cbot->getFacilityType() == CloningBuildingObjectTemplate::LIGHT_JEDI_ONLY && player->hasSkill("force_rank_light_novice")) ||
				(cbot->getFacilityType() == CloningBuildingObjectTemplate::DARK_JEDI_ONLY && player->hasSkill("force_rank_dark_novice"))) {
			FrsManager* frsManager = server->getFrsManager();

			if (frsManager != nullptr && frsManager->isFrsEnabled()) {
				String name = "Jedi Enclave (" + String::valueOf((int)loc->getWorldPositionX()) + ", " + String::valueOf((int)loc->getWorldPositionY()) + ")";
				cloneMenu->addMenuItem(name, loc->getObjectID());
			}
		}
	}

	ghost->addSuiBox(cloneMenu);
	player->sendMessage(cloneMenu->generateMessage());
}

bool PlayerManagerImplementation::isValidClosestCloner(CreatureObject* player, SceneObject* cloner, CityRegion* cityRegion) {
	if (player == nullptr || cloner == nullptr)
		return false;

	if (cityRegion != nullptr && cityRegion->isBanned(player->getObjectID()))
		return false;

	CloningBuildingObjectTemplate* cbot = cast<CloningBuildingObjectTemplate*>(cloner->getObjectTemplate());

	if (cbot == nullptr)
		return false;

	if (cbot->getFacilityType() == CloningBuildingObjectTemplate::FACTION_IMPERIAL && player->getFaction() != Factions::FACTIONIMPERIAL)
		return false;

	if (cbot->getFacilityType() == CloningBuildingObjectTemplate::FACTION_REBEL && player->getFaction() != Factions::FACTIONREBEL)
		return false;

	if (cbot->isJediCloner())
		return false;

	return true;
}

void PlayerManagerImplementation::sendPlayerToCloner(CreatureObject* player, uint64 clonerID, int typeofdeath) {
	ManagedReference<SceneObject*> cloner = server->getObject(clonerID);

	if (cloner == nullptr) {
		error("Cloning structure is null");
		return;
	}

	PlayerObject* ghost = player->getPlayerObject();

	if (ghost == nullptr)	{
		error("The player to be cloned is null");
		return;
	}

	CloningBuildingObjectTemplate* cbot = cast<CloningBuildingObjectTemplate*>(cloner->getObjectTemplate());

	if (cbot == nullptr) {
		error("Not a cloning building template.");
		return;
	}

	CloneSpawnPoint* clonePoint = cbot->getRandomSpawnPoint();

	if (clonePoint == nullptr) {
		error("clone point null");
		return;
	}

	Coordinate* coordinate = clonePoint->getCoordinate();
	Quaternion* direction = clonePoint->getDirection();

	int cellID = clonePoint->getCellID();
	SceneObject* cell = nullptr;

	if (cellID != 0) {
		BuildingObject* cloningBuilding = cloner.castTo<BuildingObject*>();

		if (cloningBuilding == nullptr)  {
			error("Cloning building is null");
			return;
		}

		cell = cloningBuilding->getCell(cellID);

		if (cell == nullptr) {
			StringBuffer msg;
			msg << "null cell for cellID " << cellID << " in building: " << cbot->getFullTemplateString();
			error(msg.toString());
			return;
		}
	}

	Zone* zone = player->getZone();

	if (zone == nullptr) {
		error() << player->getDisplayedName() << " ID: " << player->getObjectID() << " - Failed to activate clone due to null zone. Chosen Cloning Facility ID: " << cloner->getObjectID();
		return;
	}

	// Handle players death in space
	if (zone->isSpaceZone()) {
		auto launchZoneName = ghost->getSpaceLaunchZone();

		zone = server->getZone(launchZoneName);

		if (zone == nullptr || zone->isSpaceZone()) {
			error() << player->getDisplayedName() << " ID: " << player->getObjectID() << " - Failed to activate clone on Ground Zone. Chosen Cloning Facility ID: " << cloner->getObjectID();
			return;
		}
	}

	ghost->setCloning(true);

	if (cellID == 0)
		player->switchZone(zone->getZoneName(), cloner->getWorldPositionX() + coordinate->getPositionX(), cloner->getWorldPositionZ() + coordinate->getPositionZ(), cloner->getWorldPositionY() + coordinate->getPositionY(), 0);
	else
		player->switchZone(zone->getZoneName(), coordinate->getPositionX(), coordinate->getPositionZ(), coordinate->getPositionY(), cell->getObjectID());

	uint64 preDesignatedFacilityOid = ghost->getCloningFacility();
	ManagedReference<SceneObject*> preDesignatedFacility = server->getObject(preDesignatedFacilityOid);

	// Infinity Custom: preDesignatedWasSelected int is used for PLAYERCLONED observer arg2
	int preDesignatedWasSelected = 1;

	if (preDesignatedFacility == nullptr || preDesignatedFacility != cloner) {
		player->addWounds(CreatureAttribute::HEALTH, 100, true, false);
		player->addWounds(CreatureAttribute::ACTION, 100, true, false);
		player->addWounds(CreatureAttribute::MIND, 100, true, false);
		player->addShockWounds(100, true);

		preDesignatedWasSelected = 0;
	}

	if (ConfigManager::instance()->useCovertOvertSystem()) {
		if ((player->getFactionStatus() == FactionStatus::OVERT) && !player->hasSkill("force_rank_light_novice") && !player->hasSkill("force_rank_dark_novice") && (cbot->getFacilityType() != CloningBuildingObjectTemplate::FACTION_IMPERIAL) && (cbot->getFacilityType() != CloningBuildingObjectTemplate::FACTION_REBEL))
			player->setFactionStatus(FactionStatus::COVERT);
	} else {
		if (player->getFactionStatus() != FactionStatus::ONLEAVE && cbot->getFacilityType() != CloningBuildingObjectTemplate::FACTION_IMPERIAL && cbot->getFacilityType() != CloningBuildingObjectTemplate::FACTION_REBEL && !player->hasSkill("force_title_jedi_rank_03"))
			player->setFactionStatus(FactionStatus::ONLEAVE);
	}

	SortedVector<ManagedReference<SceneObject*> > insurableItems = getInsurableItems(player, false);

	// Decay
	if (typeofdeath == 0 && insurableItems.size() > 0) {

		ManagedReference<SuiListBox*> suiCloneDecayReport = new SuiListBox(player, SuiWindowType::CLONE_REQUEST_DECAY, SuiListBox::HANDLESINGLEBUTTON);
		suiCloneDecayReport->setPromptTitle("DECAY REPORT");
		suiCloneDecayReport->setPromptText("The following report summarizes the status of your items after the decay event.");
		suiCloneDecayReport->addMenuItem("\\#00FF00DECAYED ITEMS");

		for (int i = 0; i < insurableItems.size(); i++) {
			SceneObject* item = insurableItems.get(i);

			if (item != nullptr && item->isTangibleObject()) {
				ManagedReference<TangibleObject*> obj = cast<TangibleObject*>(item);

				Locker clocker(obj, player);

				if (obj->getOptionsBitmask() & OptionBitmask::INSURED) {
					//1% Decay for insured items
					obj->inflictDamage(obj, 0, 0.01 * obj->getMaxCondition(), true, true);
					//Set uninsured
					uint32 bitmask = obj->getOptionsBitmask() - OptionBitmask::INSURED;
					obj->setOptionsBitmask(bitmask);
				} else {
					//5% Decay for uninsured items
					obj->inflictDamage(obj, 0, 0.05 * obj->getMaxCondition(), true, true);
				}

				// Calculate condition percentage for decay report
				int max = obj->getMaxCondition();
				int min = max - obj->getConditionDamage();
				int condPercentage = ( min / (float)max ) * 100.0f;
				String line = " - " + obj->getDisplayedName() + " (@"+String::valueOf(condPercentage)+"%)";

				suiCloneDecayReport->addMenuItem(line, item->getObjectID());
			}
		}

		ghost->addSuiBox(suiCloneDecayReport);
		player->sendMessage(suiCloneDecayReport->generateMessage());

	}

	if (ConfigManager::instance()->getBool("Core3.PlayerManager.WipeFillingOnClone", false)) {
		ghost->setFoodFilling(0);
		ghost->setDrinkFilling(0);
	}

	Reference<Task*> task = new PlayerIncapacitationRecoverTask(player, true);
	task->schedule(3 * 1000);

	// Infinity Custom: arg2 of the observer = 1 if pre-designated cloning destination was selected, 0 if not
	player->notifyObservers(ObserverEventType::PLAYERCLONED, player, preDesignatedWasSelected);

	// Jedi experience loss.
	if (player->hasSkill("force_title_jedi_rank_02")) {//Going to check padawan box only, not JediState
		int jediXpCap = ghost->getXpCap("jedi_general");
		int xpLoss = (int)(jediXpCap * -0.05);
		int curExp = ghost->getExperience("jedi_general");

		int negXpCap = -10000000; // Cap on negative jedi experience

		if ((curExp + xpLoss) < negXpCap)
			xpLoss = negXpCap - curExp;

		awardExperience(player, "jedi_general", xpLoss, true, 1.0f, false);  //Infinity:  Don't use XP modifiers
		StringIdChatParameter message("base_player","prose_revoke_xp");
		message.setDI(xpLoss * -1);
		message.setTO("exp_n", "jedi_general");
		player->sendSystemMessage(message);
	}
}

void PlayerManagerImplementation::ejectPlayerFromBuilding(CreatureObject* player) {
	Zone* zone = player->getZone();

	if (zone == nullptr)
		return;

	ManagedReference<SceneObject*> parent = player->getParent().get();

	if (parent == nullptr || !parent->isCellObject())
		return;

	ManagedReference<CellObject*> cell = cast<CellObject*>(parent.get());

	if (cell == nullptr)
		return;

	ManagedReference<BuildingObject*> building = cell->getParent().get().castTo<BuildingObject*>();

	if (building == nullptr)
		return;

	if (building->hasTemplateEjectionPoint()) {
		Vector3 ejectionPoint = building->getEjectionPoint();
		player->switchZone(zone->getZoneName(), ejectionPoint.getX(), ejectionPoint.getZ(), ejectionPoint.getY(), 0);
	}
}

void PlayerManagerImplementation::disseminateExperience(TangibleObject* destructedObject, ThreatMap* threatMap, SynchronizedVector<ManagedReference<CreatureObject*> >* spawnedCreatures,Zone* lairZone) {
	uint32 totalDamage = threatMap->getTotalDamage();

	if (totalDamage == 0) {
		threatMap->removeAll();
		return;
	}

	VectorMap<ManagedReference<CreatureObject*>, int> slExperience;
	slExperience.setAllowOverwriteInsertPlan();
	slExperience.setNullValue(0);

	float gcwBonus = 1.0f;
	uint32 winningFaction = -1;
	int baseXp = 0;

	Zone* zone = lairZone;

	if (zone == nullptr) {
		zone = destructedObject->getZone();
	}

	if (zone != nullptr) {
		GCWManager* gcwMan = zone->getGCWManager();

		if (gcwMan != nullptr) {
			gcwBonus += (gcwMan->getGCWXPBonus() / 100.0f);
			winningFaction = gcwMan->getWinningFaction();
		}
	}

	if (!destructedObject->isCreatureObject() && spawnedCreatures != nullptr) {
		ManagedReference<AiAgent*> ai = nullptr;

		for (int i = 0; i < spawnedCreatures->size(); i++) {
			ai = cast<AiAgent*>(spawnedCreatures->get(i).get());

			if (ai != nullptr) {
				Creature* creature = cast<Creature*>(ai.get());

				if (creature != nullptr && creature->isBaby())
					continue;
				else
					break;
			}
		}

		if (ai != nullptr)
			baseXp = (ai->getFactionString().toLowerCase() == "rebel" || ai->getFactionString().toLowerCase() == "imperial") ? 0 : ai->getBaseXp();

	} else {
		ManagedReference<AiAgent*> ai = cast<AiAgent*>(destructedObject);

		if (ai != nullptr)
			baseXp = (ai->getFactionString().toLowerCase() == "rebel" || ai->getFactionString().toLowerCase() == "imperial") ? 0 : ai->getBaseXp();
	}

	Vector<uint64> playerList;

	for (int i = 0; i < threatMap->size(); ++i) {
		TangibleObject* attacker = threatMap->elementAt(i).getKey();

		if (attacker == nullptr)
			continue;

		if (attacker->isPet()) {
			CreatureObject* attackerCreo = attacker->asCreatureObject();

			if (attackerCreo == nullptr)
				continue;

			CreatureObject* owner = attackerCreo->getLinkedCreature().get();

			if (owner == nullptr)
				continue;

			uint64 attackerID = owner->getObjectID();

			if (!playerList.contains(attackerID))
				playerList.add(attackerID);
		} else {
			if (!attacker->isPlayerCreature())
				continue;

			uint64 attackerID = attacker->getObjectID();

			if (!playerList.contains(attackerID))
				playerList.add(attackerID);
		}
	}

	int playerHitCount = playerList.size();

	for (int i = 0; i < threatMap->size(); ++i) {
		ThreatMapEntry* entry = &threatMap->elementAt(i).getValue();
		TangibleObject* attacker = threatMap->elementAt(i).getKey();

		if (entry == nullptr || attacker == nullptr) {
			continue;
		}

		if (attacker->isPet()) {
			if (!attacker->isCreatureObject()) {
				continue;
			}

			if (destructedObject->isAiAgent() && destructedObject->asAiAgent()->isEventMob())
				continue;

			CreatureObject* attackerCreo = attacker->asCreatureObject();

			if (attackerCreo == nullptr) {
				continue;
			}

			PetControlDevice* pcd = attackerCreo->getControlDevice().get().castTo<PetControlDevice*>();

			// only creature pets will award exp, so discard anything else
			if (pcd == nullptr || pcd->getPetType() != PetManager::CREATUREPET) {
				continue;
			}

			CreatureObject* owner = attackerCreo->getLinkedCreature().get();

			if (owner == nullptr || !owner->isPlayerCreature()) {
				continue;
			}

			Locker crossLocker(owner, destructedObject);

			PlayerObject* ownerGhost = owner->getPlayerObject();

			if (ownerGhost == nullptr || !owner->hasSkill("outdoors_creaturehandler_novice") || !destructedObject->isInRange(owner, 80)) {
				continue;
			}

			TransactionLog trx(owner, destructedObject, TrxCode::COMBATSTATS, attackerCreo);
			trx.addState("combatIsPet", true);
			trx.addState("combatTotalDamage", entry->getTotalDamage());
			trx.addState("combatTotalNonAggroDamage", entry->getNonAggroDamage());
			trx.addState("combatTotalHeal", entry->getHeal());
			trx.addState("combatTotalSeconds", entry->getDurationSeconds());
			trx.addState("combatDPS", entry->getDPS());
			trx.addState("combatPlayerLevel", calculatePlayerLevel(owner));
			trx.addState("combatPetLevel", calculatePlayerLevel(attackerCreo));

			int totalPets = 1;

			for (int i = 0; i < ownerGhost->getActivePetsSize(); i++) {
				ManagedReference<AiAgent*> object = ownerGhost->getActivePet(i);

				if (object != nullptr && object->isCreature()) {
					if (object == attacker)
						continue;

					PetControlDevice* petControlDevice = object->getControlDevice().get().castTo<PetControlDevice*>();
					if (petControlDevice != nullptr && petControlDevice->getPetType() == PetManager::CREATUREPET)
						totalPets++;
				}
			}

			// TODO: Find a more correct CH xp formula
			float levelRatio = (float)destructedObject->getLevel() / (float)attacker->getLevel();

			float xpAmount = levelRatio * 500.f;

			if (levelRatio <= 0.5) {
				xpAmount = 1;
			} else {
				xpAmount = Math::min(xpAmount, (float)attacker->getLevel() * 50.f);
				xpAmount /= totalPets;

				if (winningFaction != Factions::FACTIONNEUTRAL && winningFaction == attacker->getFaction())
					xpAmount *= gcwBonus;
			}

			trx.addState("combatTotalPets", totalPets);

			awardExperience(owner, "creaturehandler", xpAmount);
		} else if (attacker->isPlayerCreature()) {
			if (!(attacker->getZone() == zone && destructedObject->isInRangeZoneless(attacker, 80))) {
				continue;
			}

			CreatureObject* attackerCreo = attacker->asCreatureObject();

			if (attackerCreo == nullptr) {
				continue;
			}

			ManagedReference<GroupObject*> group = attackerCreo->getGroup();

			uint32 combatXp = 0;
			uint32 jediXp = 0;

			Locker crossLocker(attackerCreo, destructedObject);

			uint32 playerTotal = 0;
			for (int v = 0; v < entry->size(); ++v) {
				uint32 weapDamage = entry->elementAt(v).getValue();
				playerTotal += weapDamage;
			}

			TransactionLog trx(attackerCreo, destructedObject, TrxCode::COMBATSTATS);
			trx.addState("combatIsPet", false);
			trx.addState("combatTotalDamage", entry->getTotalDamage());
			trx.addState("combatTotalNonAggroDamage", entry->getNonAggroDamage());
			trx.addState("combatTotalHeal", entry->getHeal());
			trx.addState("combatTotalSeconds", entry->getDurationSeconds());
			trx.addState("combatDPS", entry->getDPS());
			trx.addState("combatPlayerLevel", calculatePlayerLevel(attackerCreo));

			if (group != nullptr) {
				trx.addState("combatGroupID", group->getObjectID());
				trx.addState("combatGroupSize", group->getGroupSize());
				trx.addState("combatGroupLevel", group->getGroupLevel());
				trx.addState("combatGroupFactionPetLevel", group->getFactionPetLevel());
			}

			for (int j = 0; j < entry->size(); ++j) {
				uint32 damage = entry->elementAt(j).getValue();
				String xpType = entry->elementAt(j).getKey();

				float xpAmount = baseXp;
				int playerLevel = calculatePlayerLevel(attackerCreo, xpType);

				//xpAmount *= (float) damage / totalDamage;  //Infinity:  Don't split XP based on damage.   

				if (playerTotal == 0) {	//Player did no damage, so no XP will be awarded
					continue;
				}

				xpAmount *= (float) damage / playerTotal;  //Infinity: add in weapon damage split

				//Cap xp based on level
				xpAmount = Math::min(xpAmount, playerLevel * 300.f);

				//Apply group bonus if in group
				if (group != nullptr)
					xpAmount *= groupExpMultiplier;

				if (winningFaction != Factions::FACTIONNEUTRAL && winningFaction == attackerCreo->getFaction())
					xpAmount *= gcwBonus;

				//Infinity:  Reworked all this logic to simplify
				if (xpType == "jedi_general") {
					jediXp = Math::min(xpAmount, 7500.0f) * JediExpMultiplier;
					awardExperience(attackerCreo, xpType, jediXp, true, 1.0f, false);  //Award individual experience (Jedi).  No XP modifiers apply.
				}
				else {
					combatXp += xpAmount;   //Add combat XP for all other combat XP types

					if (xpType == "dotDMG") { //DOTs count for combat XP, but not weapon XP
						continue;
					}

					if (xpType == "mandalorian" && !attackerCreo->hasSkill("mandalorian_rank_5")) { //Only award Mando XP if they have rank_5
						continue;
					}

					if (xpType == "combat_general") {
						awardExperience(attackerCreo, xpType, xpAmount, true, 1.0f, false);  //Don't use global XP multiplier on grenade damage
					}
					else {
						awardExperience(attackerCreo, xpType, xpAmount); //Award individual weapon experience (non-Jedi)
					}
				}
			}

			//Infinity:  Don't award if zero
			if (combatXp > 0)
				awardExperience(attackerCreo, "combat_general", combatXp, true, 0.25f);

			//Check if the group leader is a squad leader
			if (group == nullptr)
				continue;

			//Calculate squad leader XP based on greater of Jedi XP or Combat XP
			float selectedXp = Math::max(combatXp, jediXp);

			//Calculate squad leader group size experience @ 10% person + combat experience which is 10% of the variable
			float squadXp = (selectedXp * 0.1f) + (selectedXp * 0.1f * group->getGroupSize());

			crossLocker.release();

			ManagedReference<CreatureObject*> groupLeader = group->getLeader();

			if (groupLeader == nullptr || !groupLeader->isPlayerCreature())
				continue;

			Locker squadLock(groupLeader, destructedObject);

			trx.addState("squadLeader", groupLeader->getObjectID());
			trx.addState("squadLeaderDistance", groupLeader->getDistanceTo(attackerCreo));

			//If he is a squad leader, and is in range of this player, then add the combat exp for him to use.
			//Removed distance check to keep current functionality. Attacker was previously comparing its location to itself
			if (groupLeader->hasSkill("outdoors_squadleader_novice")) {
				int v = slExperience.get(groupLeader) + squadXp;
				slExperience.put(groupLeader, v);
			}
		}
	}

	// Send out squad leader experience.
	for (int i = 0; i < slExperience.size(); ++i) {
		VectorMapEntry<ManagedReference<CreatureObject*>, int>* entry = &slExperience.elementAt(i);
		CreatureObject* leader = entry->getKey();

		if (leader == nullptr)
			continue;

		Locker clock(leader, destructedObject);

		awardExperience(leader, "squadleader", entry->getValue() * 2.f);
	}

	threatMap->removeAll();
}

void PlayerManagerImplementation::disseminateSpaceExperience(ShipAiAgent* destructedObject, ThreatMap* threatMap) {
	if (destructedObject == nullptr || threatMap == nullptr) {
		threatMap->removeAll();
		return;
	}

	uint32 totalDamage = threatMap->getTotalDamage();

	if (totalDamage == 0) {
		threatMap->removeAll();
		return;
	}

	auto zoneServer = ServerCore::getZoneServer();

	if (zoneServer == nullptr) {
		threatMap->removeAll();
		return;
	}

	uint32 shipDifficulty = destructedObject->getShipDifficulty().hashCode();
	String factionName = destructedObject->getShipFactionString();
	uint32 shipTypeHash = destructedObject->getShipType().hashCode();

	float experienceReward = destructedObject->getExperienceValue();

	int imperialReward = destructedObject->getImperialFactionReward();
	int rebelReward = destructedObject->getRebelFactionReward();

	// All experience should be random with the exception of ISD and Corvette
	if (shipTypeHash != STRING_HASHCODE("star_destroyer") && shipTypeHash != STRING_HASHCODE("corvette")) {
		int bonus = experienceReward * 0.15f;
		experienceReward += System::random(bonus);
	}

	// info(true) << "disseminateSpaceExperience -- for: " << destructedObject->getDisplayedName() << " Ship Diificulty: " << destructedObject->getShipDifficulty() << " Hash: " << shipDifficulty;
	// info(true) << "Anticipated XP: " << experienceReward;

	// Get destructed ships faction
	uint32 destructedFaction = destructedObject->getFaction();

	for (int i = 0; i < threatMap->size(); ++i) {
		ThreatMapEntry* entry = &threatMap->elementAt(i).getValue();
		TangibleObject* attacker = threatMap->elementAt(i).getKey();

		if (entry == nullptr || attacker == nullptr || !attacker->isPlayerShip()) {
			continue;
		}

		if (entry->getTotalDamage() < 0.5f) {
			continue;
		}

		auto playerShip = attacker->asShipObject();

		if (playerShip == nullptr) {
			continue;
		}

		Locker shipLock(playerShip, destructedObject);

		// Range check
		if (!destructedObject->isInRange3dZoneless(playerShip, ZoneServer::SPACECLOSEOBJECTRANGE)) {
			continue;
		}

		auto playersOnBoard = playerShip->getPlayersOnBoard();
		int totalPlayers = playersOnBoard.size();

		// Experience divided among players on ship
		float shipExperience = (experienceReward / totalPlayers);

		for (int i = 0; i < totalPlayers; ++i) {
			auto shipMemberID = playersOnBoard.get(i);
			auto shipMember = cast<CreatureObject*>(zoneServer->getObject(shipMemberID).get());

			if (shipMember == nullptr) {
				continue;
			}

			auto ghost = shipMember->getPlayerObject();

			if (ghost == nullptr) {
				continue;
			}

			Locker playLock(shipMember, playerShip);

			// Award Faction Points to overt players
			if (shipMember->getFactionStatus() == FactionStatus::OVERT && (imperialReward != 0 || rebelReward != 0)) {
				FactionManager* factionManager = FactionManager::instance();
				factionManager->awardSpaceFactionPoints(shipMember, shipTypeHash, factionName, shipDifficulty, totalPlayers, imperialReward, rebelReward);
			}

			if (shipMember->hasSkill("pilot_neutral_master")) {
				awardExperience(shipMember, "prestige_pilot", shipExperience, true, 1.f);
			} else if (shipMember->hasSkill("pilot_rebel_navy_master")) {
				awardExperience(shipMember, "prestige_rebel", shipExperience, true, 1.f);
			} else if (shipMember->hasSkill("pilot_imperial_navy_master")) {
				awardExperience(shipMember, "prestige_imperial", shipExperience, true, 1.f);
			} else {
				float aceMultiplier = 0.0f;

				/*	130 = "pilot_rebel_navy_corellia"	"...has become a Arkon's Havoc Squadron Ace Pilot."
					131 = "pilot_rebel_navy_naboo"	"...has become a Vortex Ace Pilot."
					132 = "pilot_rebel_navy_tatooine"	"...has become a Crimson Phoenix Ace Pilot."

					133 = "pilot_imperial_navy_corellia"	"...has become a Black Epsilon Ace Pilot."
					134 = "pilot_imperial_navy_naboo"	"...has become an Imperial Inquisition Ace Pilot."
					135 = "pilot_imperial_navy_tatooine"	"...has become a Storm Squadron Ace Pilot."

					136 = "pilot_neutral_corellia"	"...has become a Corellian Security Forces Ace Pilot."
					137 = "pilot_neutral_naboo"	"...has become a Royal Security Forces Ace Pilot."
					138 = "pilot_neutral_tatooine"	"...has become a Smugglers Alliance Ace Pilot."		*/

				Vector<Vector<uint32>> aceBadges = {{130, 131, 132}, {133, 134, 135}, {136, 137, 138}};

				for (int i = 0; i < aceBadges.size(); i++) {
					auto badgeVec = aceBadges.get(i);

					for (int j = 0; j < 3; j++) {
						auto badge = badgeVec.get(j);

						if (ghost->hasBadge(badge)) {
							aceMultiplier += 1.f;
							break;
						}
					}
				}

				// info(true) << "Awarding XP to: " << shipMember->getDisplayedName() << " Ace Multiplier: " << aceMultiplier << " XP Amount: " << shipExperience;

				awardExperience(shipMember, "space_combat_general", shipExperience, true, 1.f);

				if (aceMultiplier > 0) {
					awardExperience(shipMember, "space_combat_general", shipExperience, true, aceMultiplier, true, true);
				}
			}
		}
	}

	// Lastly, clear the threat map
	threatMap->removeAll();
}

bool PlayerManagerImplementation::checkEncumbrancies(CreatureObject* player, ArmorObject* armor) {
	int strength = player->getHAM(CreatureAttribute::STRENGTH);
	int constitution = player->getHAM(CreatureAttribute::CONSTITUTION);
	int quickness = player->getHAM(CreatureAttribute::QUICKNESS);
	int stamina = player->getHAM(CreatureAttribute::STAMINA);
	int focus = player->getHAM(CreatureAttribute::FOCUS);
	int willpower = player->getHAM(CreatureAttribute::WILLPOWER);

	int healthEncumb = armor->getHealthEncumbrance();
	int actionEncumb = armor->getActionEncumbrance();
	int mindEncumb = armor->getMindEncumbrance();

	if (healthEncumb <= 0 && actionEncumb <= 0 && mindEncumb <= 0)
		return true;

	if (healthEncumb >= strength || healthEncumb >= constitution ||
			actionEncumb >= quickness || actionEncumb >= stamina ||
			mindEncumb >= focus || mindEncumb >= willpower) {
		player->sendSystemMessage("@system_msg:equip_armor_fail"); // You are not healthy enough to wear this armor!

		if (healthEncumb >= strength) {
			int statStr = (healthEncumb - strength) + 1;
			StringIdChatParameter params("@system_msg:equip_armor_fail_prose"); // You need %DI more %TT to wear this armor.
			params.setDI(statStr);
			params.setTT("@att_n:strength");
			player->sendSystemMessage(params);
		}

		if (healthEncumb >= constitution) {
			int statCon = (healthEncumb - constitution) + 1;
			StringIdChatParameter params("@system_msg:equip_armor_fail_prose");
			params.setDI(statCon);
			params.setTT("@att_n:constitution");
			player->sendSystemMessage(params);
		}

		if (actionEncumb >= quickness) {
			int statQuick = (actionEncumb - quickness) + 1;
			StringIdChatParameter params("@system_msg:equip_armor_fail_prose");
			params.setDI(statQuick);
			params.setTT("@att_n:quickness");
			player->sendSystemMessage(params);
		}

		if (actionEncumb >= stamina) {
			int statStam = (actionEncumb - stamina) + 1;
			StringIdChatParameter params("@system_msg:equip_armor_fail_prose");
			params.setDI(statStam);
			params.setTT("@att_n:stamina");
			player->sendSystemMessage(params);
		}

		if (mindEncumb >= focus) {
			int statFoc = (mindEncumb - focus) + 1;
			StringIdChatParameter params("@system_msg:equip_armor_fail_prose");
			params.setDI(statFoc);
			params.setTT("@att_n:focus");
			player->sendSystemMessage(params);
		}

		if (mindEncumb >= willpower) {
			int statWill = (mindEncumb - willpower) + 1;
			StringIdChatParameter params("@system_msg:equip_armor_fail_prose");
			params.setDI(statWill);
			params.setTT("@att_n:willpower");
			player->sendSystemMessage(params);
		}

		return false;
	}
	else
		return true;
}

void PlayerManagerImplementation::applyEncumbrancies(CreatureObject* player, ArmorObject* armor) {
	int healthEncumb = Math::max(0, armor->getHealthEncumbrance());
	int actionEncumb = Math::max(0, armor->getActionEncumbrance());
	int mindEncumb = Math::max(0, armor->getMindEncumbrance());

	player->addEncumbrance(CreatureEncumbrance::HEALTH, healthEncumb, true);
	player->addEncumbrance(CreatureEncumbrance::ACTION, actionEncumb, true);
	player->addEncumbrance(CreatureEncumbrance::MIND, mindEncumb, true);

	player->inflictDamage(player, CreatureAttribute::STRENGTH, healthEncumb, true);
	player->addMaxHAM(CreatureAttribute::STRENGTH, -healthEncumb, true);

	player->inflictDamage(player, CreatureAttribute::CONSTITUTION, healthEncumb, true);
	player->addMaxHAM(CreatureAttribute::CONSTITUTION, -healthEncumb, true);

	player->inflictDamage(player, CreatureAttribute::QUICKNESS, actionEncumb, true);
	player->addMaxHAM(CreatureAttribute::QUICKNESS, -actionEncumb, true);

	player->inflictDamage(player, CreatureAttribute::STAMINA, actionEncumb, true);
	player->addMaxHAM(CreatureAttribute::STAMINA, -actionEncumb, true);

	player->inflictDamage(player, CreatureAttribute::FOCUS, mindEncumb, true);
	player->addMaxHAM(CreatureAttribute::FOCUS, -mindEncumb, true);

	player->inflictDamage(player, CreatureAttribute::WILLPOWER, mindEncumb, true);
	player->addMaxHAM(CreatureAttribute::WILLPOWER, -mindEncumb, true);
}

void PlayerManagerImplementation::removeEncumbrancies(CreatureObject* player, ArmorObject* armor) {
	int healthEncumb = Math::max(0, armor->getHealthEncumbrance());
	int actionEncumb = Math::max(0, armor->getActionEncumbrance());
	int mindEncumb = Math::max(0, armor->getMindEncumbrance());

	player->addEncumbrance(CreatureEncumbrance::HEALTH, -healthEncumb, true);
	player->addEncumbrance(CreatureEncumbrance::ACTION, -actionEncumb, true);
	player->addEncumbrance(CreatureEncumbrance::MIND, -mindEncumb, true);

	player->addMaxHAM(CreatureAttribute::STRENGTH, healthEncumb, true);
	player->healDamage(player, CreatureAttribute::STRENGTH, healthEncumb, true);

	player->addMaxHAM(CreatureAttribute::CONSTITUTION, healthEncumb, true);
	player->healDamage(player, CreatureAttribute::CONSTITUTION, healthEncumb, true);

	player->addMaxHAM(CreatureAttribute::QUICKNESS, actionEncumb, true);
	player->healDamage(player, CreatureAttribute::QUICKNESS, actionEncumb, true);

	player->addMaxHAM(CreatureAttribute::STAMINA, actionEncumb, true);
	player->healDamage(player, CreatureAttribute::STAMINA, actionEncumb, true);

	player->addMaxHAM(CreatureAttribute::FOCUS, mindEncumb, true);
	player->healDamage(player, CreatureAttribute::FOCUS, mindEncumb, true);

	player->addMaxHAM(CreatureAttribute::WILLPOWER, mindEncumb, true);
	player->healDamage(player, CreatureAttribute::WILLPOWER, mindEncumb, true);
}

void PlayerManagerImplementation::awardBadge(PlayerObject* ghost, uint32 badgeId) {
	const Badge* badge = BadgeList::instance()->get(badgeId);
	if (ghost != nullptr && badge != nullptr)
		awardBadge(ghost, badge);
}

void PlayerManagerImplementation::awardBadge(PlayerObject* ghost, const Badge* badge) {
	if (ghost == nullptr) {
		error("ghost is nullptr");
		return;
	}
	if (badge == nullptr) {
		ghost->error("Failed to award null badge.");
		return;
	}

	StringIdChatParameter stringId("badge_n", "");
	stringId.setTO("badge_n", badge->getKey());

	ManagedReference<CreatureObject*> player = dynamic_cast<CreatureObject*>(ghost->getParent().get().get());

    if (player == nullptr) {
		error("player is nullptr");
		return;
	}

	const unsigned int badgeId = badge->getIndex();
	if (ghost->hasBadge(badgeId)) {
		stringId.setStringId("badge_n", "prose_hasbadge");
		player->sendSystemMessage(stringId);
		return;
	}

	ghost->setBadge(badgeId);
	stringId.setStringId("badge_n", "prose_grant");
	player->sendSystemMessage(stringId);

	if (badge->getHasMusic()) {
		String music = badge->getMusic();
		PlayMusicMessage* musicMessage = new PlayMusicMessage(music);
		player->sendMessage(musicMessage);
	}

	player->notifyObservers(ObserverEventType::BADGEAWARDED, player, badgeId);

    //
    //Infinity:  Added validateBadges function to call on award, revoke, and login
    //

    validateBadges(ghost, true);

}

void PlayerManagerImplementation::setExperienceMultiplier(float newGlobalMultiplier) {
	globalExpMultiplier = newGlobalMultiplier;
}

/*
 * Award experience to a player.
 * Ex.
	PlayerManager* playerManager = server->getPlayerManager();
	playerManager->awardExperience(playerCreature, "resource_harvesting_inorganic", 500);
 *
 */
int PlayerManagerImplementation::awardExperience(CreatureObject* player, const String& xpType, int amount, bool sendSystemMessage, float localMultiplier, bool applyModifiers, bool spaceBonus) {
	PlayerObject* playerObject = player->getPlayerObject();

	if (playerObject == nullptr)
		return 0;

	TransactionLog trx(TrxCode::EXPERIENCE, player);

	float speciesModifier = 1.f;

	/* Infinity:  Not going to use species XP modifiers
	if (amount > 0) {
		speciesModifier = getSpeciesXpModifier(player->getSpeciesName(), xpType);
	}
	*/

	float buffMultiplier = 1.f;

	if (player->hasBuff(BuffCRC::FOOD_XP_INCREASE) && !player->containsActiveSession(SessionFacadeType::CRAFTING))
		buffMultiplier += player->getSkillModFromBuffs("xp_increase") / 100.f;

	int xp = 0;

	trx.addState("applyModifiers", applyModifiers);

	if (applyModifiers) {
		trx.addState("speciesModifier", speciesModifier);
		trx.addState("buffMultiplier", buffMultiplier);
		trx.addState("localMultiplier", localMultiplier);
		trx.addState("globalExpMultiplier", globalExpMultiplier);
		trx.addState("FRSExpMultiplier", FRSExpMultiplier);   //Infinity

		if (xpType == "force_rank_xp") { //Infinity
			xp = playerObject->addExperience(trx, xpType, (int) (amount * FRSExpMultiplier));
		} else {
			xp = playerObject->addExperience(trx, xpType, (int) (amount * speciesModifier * buffMultiplier * localMultiplier * globalExpMultiplier));
		}
	} else {
		xp = playerObject->addExperience(trx, xpType, (int)amount);
	}

	player->notifyObservers(ObserverEventType::XPAWARDED, player, xp);

	if (sendSystemMessage) {
		if (xp > 0) {
			if (spaceBonus) {
				StringIdChatParameter message("base_player","prose_grant_xp_bonus");
				message.setDI(xp);
				message.setTO("exp_n", xpType);
				player->sendSystemMessage(message);
			} else {
				StringIdChatParameter message("base_player","prose_grant_xp");
				message.setDI(xp);
				message.setTO("exp_n", xpType);
				player->sendSystemMessage(message);
			}
		}
		if (xp > 0 && playerObject->hasCappedExperience(xpType)) {
			StringIdChatParameter message("base_player", "prose_hit_xp_cap"); //You have achieved your current limit for %TO experience.
			message.setTO("exp_n", xpType);
			player->sendSystemMessage(message);
		}
	}

	return xp;
}

void PlayerManagerImplementation::sendLoginMessage(CreatureObject* creature) {
	String motd = server->getLoginMessage();

	ChatSystemMessage* csm = new ChatSystemMessage(UnicodeString(motd), ChatSystemMessage::DISPLAY_CHATONLY);
	creature->sendMessage(csm);
}

void PlayerManagerImplementation::resendLoginMessageToAll() {
	ChatManager* chatManager = server->getChatManager();

	if (chatManager != nullptr) {
		String motd = server->getLoginMessage();

		ChatSystemMessage* csm = new ChatSystemMessage(UnicodeString(motd), ChatSystemMessage::DISPLAY_CHATONLY);
		chatManager->broadcastMessage(csm);
	}
}

void PlayerManagerImplementation::handleAbortTradeMessage(CreatureObject* player) {
	if (player == nullptr)
		return;

	// Clear trade target on player
	player->setTradeTargetID(0);

	ManagedReference<TradeSession*> tradeContainer = player->getActiveSession(SessionFacadeType::TRADE).castTo<TradeSession*>();

	AbortTradeMessage* msg = new AbortTradeMessage();

	if (tradeContainer == nullptr) {
		player->sendMessage(msg->clone());
		delete msg;

		return;
	}

	uint64 targetID = tradeContainer->getTradeTargetPlayer();
	ManagedReference<SceneObject*> object = server->getObject(targetID);

	if (object != nullptr) {
		CreatureObject* receiver = object->asCreatureObject();

		if (receiver != nullptr) {
			Locker clocker(receiver, player);

			ManagedReference<TradeSession*> receiverContainer = receiver->getActiveSession(SessionFacadeType::TRADE).castTo<TradeSession*>();

			if (receiverContainer != nullptr && receiverContainer->getTradeTargetPlayer() == player->getObjectID()) {
				receiver->dropActiveSession(SessionFacadeType::TRADE);
				receiver->sendMessage(msg->clone());

				// clear receivers trade target ID
				receiver->setTradeTargetID(0);
			}

			clocker.release();
		}
	}

	player->sendMessage(msg->clone());

	delete msg;

	player->dropActiveSession(SessionFacadeType::TRADE);
}

void PlayerManagerImplementation::handleAddItemToTradeWindow(CreatureObject* player, uint64 itemID) {
	if (player == nullptr)
		return;

	ManagedReference<TradeSession*> tradeContainer = player->getActiveSession(SessionFacadeType::TRADE).castTo<TradeSession*>();

	if (tradeContainer == nullptr)
		return;

	// First Verify Target is Player
	uint64 targetID = tradeContainer->getTradeTargetPlayer();
	ManagedReference<SceneObject*> object = server->getObject(targetID);

	if (object == nullptr || !object->isPlayerCreature())
		return;

	CreatureObject* receiver = object->asCreatureObject();

	ManagedReference<SceneObject*> objectToTrade = server->getObject(itemID);

	if (objectToTrade == nullptr || !objectToTrade->isASubChildOf(player) || !objectToTrade->checkContainerPermission(player, ContainerPermissions::MOVECONTAINER)) {
		player->sendSystemMessage("@container_error_message:container26");
		handleAbortTradeMessage(player);
		return;
	}

	if (objectToTrade->isNoTrade()) {
		player->sendSystemMessage("@container_error_message:container26");
		handleAbortTradeMessage(player);
		return;
	}

	// Containers containing notrade items...
	if (objectToTrade->containsNoTradeObjectRecursive()) {
		player->sendSystemMessage("@container_error_message:container26");
		handleAbortTradeMessage(player);
		return;
	}

	if (objectToTrade->isControlDevice()) {
		Reference<ControlDevice*> controlDevice = cast<ControlDevice*>(objectToTrade.get());
		Reference<TangibleObject*> controlledObject = controlDevice->getControlledObject();

		if (controlledObject != nullptr) {
			Locker crossLocker(controlledObject, player);

			controlDevice->storeObject(player, true);
		}
	}

	tradeContainer->addTradeItem(objectToTrade);

	SceneObject* inventory = player->getInventory();

	if (inventory != nullptr)
		inventory->sendWithoutContainerObjectsTo(receiver);

	objectToTrade->sendTo(receiver, true);

	AddItemMessage* msg = new AddItemMessage(itemID);

	if (msg != nullptr)
		receiver->sendMessage(msg);
}

void PlayerManagerImplementation::handleGiveMoneyMessage(CreatureObject* player, uint32 value) {
	if (player == nullptr)
		return;

	int currentMoney = player->getCashCredits();

	if (value > currentMoney)
		value = currentMoney;

	ManagedReference<TradeSession*> tradeContainer = player->getActiveSession(SessionFacadeType::TRADE).castTo<TradeSession*>();

	if (tradeContainer == nullptr)
		return;

	tradeContainer->setMoneyToTrade(value);

	uint64 targetID = tradeContainer->getTradeTargetPlayer();
	ManagedReference<SceneObject*> object = server->getObject(targetID);

	if (object == nullptr || !object->isPlayerCreature())
		return;

	CreatureObject* receiver = object->asCreatureObject();

	if (receiver == nullptr)
		return;

	GiveMoneyMessage* msg = new GiveMoneyMessage(value);

	if (msg == nullptr)
		return;

	receiver->sendMessage(msg);
}

void PlayerManagerImplementation::handleAcceptTransactionMessage(CreatureObject* player) {
	Locker _locker(player);

	ManagedReference<TradeSession*> tradeContainer = player->getActiveSession(SessionFacadeType::TRADE).castTo<TradeSession*>();

	if (tradeContainer == nullptr)
		return;

	tradeContainer->setAcceptedTrade(true);

	uint64 targID = tradeContainer->getTradeTargetPlayer();
	ManagedReference<SceneObject*> obj = server->getObject(targID);

	if (obj != nullptr && obj->isPlayerCreature()) {
		CreatureObject* receiver = cast<CreatureObject*>(obj.get());

		AcceptTransactionMessage* msg = new AcceptTransactionMessage();
		receiver->sendMessage(msg);
	}
}

void PlayerManagerImplementation::handleUnAcceptTransactionMessage(CreatureObject* player) {
	Locker _locker(player);

	ManagedReference<TradeSession*> tradeContainer = player->getActiveSession(SessionFacadeType::TRADE).castTo<TradeSession*>();

	if (tradeContainer == nullptr)
		return;

	tradeContainer->setAcceptedTrade(false);

	uint64 targID = tradeContainer->getTradeTargetPlayer();
	ManagedReference<SceneObject*> obj = server->getObject(targID);

	if (obj != nullptr && obj->isPlayerCreature()) {
		CreatureObject* receiver = cast<CreatureObject*>(obj.get());

		UnAcceptTransactionMessage* msg = new UnAcceptTransactionMessage();
		receiver->sendMessage(msg);
	}

}

bool PlayerManagerImplementation::checkTradeItems(CreatureObject* player, CreatureObject* receiver) {
	// Pre: arg1 && arg2 pre-locked

	PlayerObject* ghost = player->getPlayerObject();
	PlayerObject* targetGhost = receiver->getPlayerObject();

	ManagedReference<TradeSession*> tradeContainer = player->getActiveSession(SessionFacadeType::TRADE).castTo<TradeSession*>();
	ManagedReference<TradeSession*> receiverContainer = receiver->getActiveSession(SessionFacadeType::TRADE).castTo<TradeSession*>();

	if (tradeContainer == nullptr || receiverContainer == nullptr)
		return false;

	if (tradeContainer->getTradeTargetPlayer() != receiver->getObjectID())
		return false;

	if (receiverContainer->getTradeTargetPlayer() != player->getObjectID())
		return false;

	SceneObject* playerInventory = player->getInventory();
	SceneObject* receiverInventory = receiver->getInventory();

	SceneObject* playerDatapad = player->getSlottedObject("datapad");
	SceneObject* receiverDatapad = receiver->getSlottedObject("datapad");

	int playerTanos = 0;
	int playerItnos = 0;
	int recieverTanos = 0;
	int recieverItnos = 0;
	int playerCreaturePetsTraded = 0;
	int receiverCreaturePetsTraded = 0;
	int playerFactionPetsTraded = 0;
	int receiverFactionPetsTraded = 0;
	int playerDroidsTraded = 0;
	int receiverDroidsTraded = 0;
	int playerVehiclesTraded = 0;
	int receiverVehiclesTraded = 0;
	int playerShipsTraded = 0;
	int receiverShipsTraded = 0;

	for (int i = 0; i < tradeContainer->getTradeSize(); ++i) {
		ManagedReference<SceneObject*> scene = tradeContainer->getTradeItem(i);

		if (scene->isNoTrade())
			return false;

		if (scene->isTangibleObject()) {

			String err;
			if (receiverInventory->canAddObject(scene, -1, err) != 0)
				return false;

			if (!playerInventory->hasObjectInContainer(scene->getObjectID()))
				return false;

			if (scene->getObjectTemplate()->getTemplateFileName() == "eta1_ship_usable") {
				return false;
			}		

			recieverTanos++;

		} else if (scene->isIntangibleObject()) {

			String err;
			if (receiverDatapad->canAddObject(scene, -1, err) != 0)
				return false;

			if (!playerDatapad->hasObjectInContainer(scene->getObjectID()))
				return false;

			if (scene->isPetControlDevice()) {
				PetControlDevice* petControlDevice = cast<PetControlDevice*>(scene.get());

				if (petControlDevice->getPetType() == PetManager::CREATUREPET) {
					if (!petControlDevice->canBeTradedTo(player, receiver, receiverCreaturePetsTraded))
						return false;

					receiverCreaturePetsTraded++;
				} else if (petControlDevice->getPetType() == PetManager::FACTIONPET) {
					if (!petControlDevice->canBeTradedTo(player, receiver, receiverFactionPetsTraded))
						return false;

					receiverFactionPetsTraded++;
				} else if (petControlDevice->getPetType() == PetManager::DROIDPET) {
					if (!petControlDevice->canBeTradedTo(player, receiver, receiverDroidsTraded))
						return false;

					receiverDroidsTraded++;
				} else if (petControlDevice->getPetType() == PetManager::HELPERDROIDPET) {
					return false;
				}
			} else if (scene->isVehicleControlDevice()) {
				VehicleControlDevice* vehicleControlDevice = cast<VehicleControlDevice*>(scene.get());

				if (!vehicleControlDevice->canBeTradedTo(player, receiver, receiverVehiclesTraded))
					return false;

				receiverVehiclesTraded++;
			} else if (scene->isShipControlDevice()) {
				ShipControlDevice* shipControlDevice = cast<ShipControlDevice*>(scene.get());

				if (!shipControlDevice->canBeTradedTo(player, receiver, receiverShipsTraded))
					return false;

				receiverShipsTraded++;
			}

			recieverItnos++;

		} else {
			return false;
		}
	}

	for (int i = 0; i < receiverContainer->getTradeSize(); ++i) {
		ManagedReference<SceneObject*> scene = receiverContainer->getTradeItem(i);

		if (scene->isNoTrade())
			return false;

		if (scene->isTangibleObject()) {

			String err;
			if (playerInventory->canAddObject(scene, -1, err) != 0)
				return false;

			if (!receiverInventory->hasObjectInContainer(scene->getObjectID()))
				return false;

			playerTanos++;

		} else if (scene->isIntangibleObject()) {

			String err;
			if (playerDatapad->canAddObject(scene, -1, err) != 0)
				return false;

			if (!receiverDatapad->hasObjectInContainer(scene->getObjectID()))
				return false;

			if (scene->isPetControlDevice()) {
				PetControlDevice* petControlDevice = cast<PetControlDevice*>(scene.get());

				if (petControlDevice->getPetType() == PetManager::CREATUREPET) {
					if (!petControlDevice->canBeTradedTo(receiver, player, playerCreaturePetsTraded))
						return false;

					playerCreaturePetsTraded++;
				} else if (petControlDevice->getPetType() == PetManager::FACTIONPET) {
					if (!petControlDevice->canBeTradedTo(receiver, player, playerFactionPetsTraded))
						return false;

					playerFactionPetsTraded++;
				} else if (petControlDevice->getPetType() == PetManager::DROIDPET) {
					if (!petControlDevice->canBeTradedTo(receiver, player, playerDroidsTraded))
						return false;

					playerDroidsTraded++;
				}
			} else if (scene->isVehicleControlDevice()) {
				VehicleControlDevice* vehicleControlDevice = cast<VehicleControlDevice*>(scene.get());

				if (!vehicleControlDevice->canBeTradedTo(receiver, player, playerVehiclesTraded))
					return false;

				playerVehiclesTraded++;
			} else if (scene->isShipControlDevice()) {
				ShipControlDevice* shipControlDevice = cast<ShipControlDevice*>(scene.get());

				if (!shipControlDevice->canBeTradedTo(receiver, player, playerShipsTraded))
					return false;

				playerShipsTraded++;
			}

			playerItnos++;

		} else {
			return false;
		}
	}

	if (recieverTanos != 0 && (receiverInventory->getContainerObjectsSize() + recieverTanos >= receiverInventory->getContainerVolumeLimit())) {
		player->sendSystemMessage("@container_error_message:container19");
		receiver->sendSystemMessage("@container_error_message:container19");
		return false;
	}

	if (playerTanos != 0 && (playerInventory->getContainerObjectsSize() + playerTanos >= playerInventory->getContainerVolumeLimit())) {
		player->sendSystemMessage("@container_error_message:container19");
		receiver->sendSystemMessage("@container_error_message:container19");
		return false;
	}

	if (receiverDatapad->getContainerObjectsSize() + recieverItnos >= receiverDatapad->getContainerVolumeLimit()) {
		player->sendSystemMessage("@container_error_message:container19");
		receiver->sendSystemMessage("@container_error_message:container19");
		return false;
	}

	if (playerDatapad->getContainerObjectsSize() + playerItnos >= playerDatapad->getContainerVolumeLimit()) {
		player->sendSystemMessage("@container_error_message:container19");
		receiver->sendSystemMessage("@container_error_message:container19");
		return false;
	}

	int playerMoneyToTrade = tradeContainer->getMoneyToTrade();

	if (playerMoneyToTrade < 0)
		return false;

	if (playerMoneyToTrade > player->getCashCredits())
		return false;

	int receiverMoneyToTrade = receiverContainer->getMoneyToTrade();

	if (receiverMoneyToTrade < 0)
		return false;

	if (receiverMoneyToTrade > receiver->getCashCredits())
		return false;

	if (player->getDistanceTo(receiver) >= 15.f) {
		player->sendSystemMessage("You are too far to trade");
		receiver->sendSystemMessage("You are too far to trade");
		return false;
	}

	return true;
}

void PlayerManagerImplementation::handleVerifyTradeMessage(CreatureObject* player) {
	if (player == nullptr)
		return;

	ManagedReference<ObjectController*> objectController = server->getObjectController();

	if (objectController == nullptr)
		return;

	ManagedReference<TradeSession*> tradeContainer = player->getActiveSession(SessionFacadeType::TRADE).castTo<TradeSession*>();

	if (tradeContainer == nullptr) {
		return;
	}

	// Get a trx group to trace all trx's in this session
	auto trxGroup = TransactionLog::getNewTrxGroup();

	tradeContainer->setVerifiedTrade(true);

	uint64 targetID = tradeContainer->getTradeTargetPlayer();
	ManagedReference<SceneObject*> object = server->getObject(targetID);

	if (object == nullptr || !object->isPlayerCreature())
		return;

	CreatureObject* receiver = object->asCreatureObject();

	if (receiver == nullptr)
		return;

	Locker clocker(receiver, player);

	ManagedReference<TradeSession*> receiverTradeContainer = receiver->getActiveSession(SessionFacadeType::TRADE).castTo<TradeSession*>();

	if (receiverTradeContainer == nullptr) {
		tradeContainer->setVerifiedTrade(false);
		return;
	}

	if (!checkTradeItems(player, receiver)) {
		clocker.release();

		handleAbortTradeMessage(player);

		return;
	}

	if (receiverTradeContainer->hasVerifiedTrade()) {
		SceneObject* receiverInventory = receiver->getInventory();
		SceneObject* receiverDatapad = receiver->getSlottedObject("datapad");

		for (int i = 0; i < tradeContainer->getTradeSize(); ++i) {
			ManagedReference<SceneObject*> item = tradeContainer->getTradeItem(i);

			TransactionLog trx(player, receiver, item, TrxCode::PLAYERTRADE);
			trx.setTrxGroup(trxGroup);

			if (item->isTangibleObject()) {
				if (objectController->transferObject(item, receiverInventory, -1, true))
					item->sendDestroyTo(player);
			} else {
				if (objectController->transferObject(item, receiverDatapad, -1, true))
					item->sendDestroyTo(player);
			}
		}

		SceneObject* playerInventory = player->getInventory();
		SceneObject* playerDatapad = player->getSlottedObject("datapad");

		for (int i = 0; i < receiverTradeContainer->getTradeSize(); ++i) {
			ManagedReference<SceneObject*> item = receiverTradeContainer->getTradeItem(i);

			TransactionLog trx(receiver, player, item, TrxCode::PLAYERTRADE);
			trx.setTrxGroup(trxGroup);

			if (item->isTangibleObject()) {
				if (objectController->transferObject(item, playerInventory, -1, true)) {
					item->sendDestroyTo(receiver);
				} else {
					trx.errorMessage() << "transferObject failed";
				}
			} else {
				if (objectController->transferObject(item, playerDatapad, -1, true)) {
					item->sendDestroyTo(receiver);
				} else {
					trx.errorMessage() << "transferObject failed";
				}
			}
		}

		uint32 giveMoney = tradeContainer->getMoneyToTrade();
		uint32 getMoney = receiverTradeContainer->getMoneyToTrade();

		TradeCompleteMessage* msg = new TradeCompleteMessage();
		receiver->sendMessage(msg->clone());
		player->sendMessage(msg->clone());
		delete msg;

		if (giveMoney > 0) {
			TransactionLog trx(player, receiver, TrxCode::PLAYERTRADE, giveMoney, true);
			trx.setTrxGroup(trxGroup);
			player->subtractCashCredits(giveMoney);
			receiver->addCashCredits(giveMoney);
		}

		if (getMoney > 0) {
			TransactionLog trx(receiver, player, TrxCode::PLAYERTRADE, getMoney, true);
			trx.setTrxGroup(trxGroup);
			receiver->subtractCashCredits(getMoney);
			player->addCashCredits(getMoney);
		}

		tradeContainer->clearTradeContainer();
		receiverTradeContainer->clearTradeContainer();

		receiver->dropActiveSession(SessionFacadeType::TRADE);
		player->dropActiveSession(SessionFacadeType::TRADE);

		receiver->setTradeTargetID(0);
		player->setTradeTargetID(0);
	}
}

int PlayerManagerImplementation::notifyObserverEvent(uint32 eventType, Observable* observable, ManagedObject* arg1, int64 arg2) {

	if (eventType == ObserverEventType::POSTURECHANGED) {
		CreatureObject* creature = cast<CreatureObject*>(observable);

		if (creature == nullptr)
			return 1;

		if (creature->hasState(CreatureState::ALERT)) { // This can apply to TKA AND Jedi meditate since they share the same sysmsgs / moods.
			creature->sendSystemMessage("@teraskasi:med_end");
			creature->setMood(creature->getMoodID(), true);
			creature->clearState(CreatureState::ALERT, true);

			// Check POSTERCHANGE on Meditate...
			Reference<MeditateTask*> meditateTask = creature->getPendingTask("meditate").castTo<MeditateTask*>();
			if (meditateTask != nullptr) {
				creature->removePendingTask("meditate");

				if (meditateTask->isScheduled())
					meditateTask->cancel();
			}

			// Check POSTERCHANGE on Force Meditate...
			Reference<ForceMeditateTask*> fmeditateTask = creature->getPendingTask("forcemeditate").castTo<ForceMeditateTask*>( );

			if (fmeditateTask != nullptr) {
				creature->removePendingTask("forcemeditate");

				if (fmeditateTask->isScheduled())
					fmeditateTask->cancel();
			}
		}

		// Check POSTURECHANGED disrupting Logout...
		Reference<LogoutTask*> logoutTask = creature->getPendingTask("logout").castTo<LogoutTask*>();
		if (logoutTask != nullptr && !creature->isSitting()) {
            StringIdChatParameter msg("logout", "must_be_sitting");
			creature->sendSystemMessage(msg); // You must be sitting in order to log out safely..
			logoutTask->cancelLogout();
		}

		return 1;
	}

	return 0;
}

void PlayerManagerImplementation::sendBattleFatigueMessage(CreatureObject* player, CreatureObject* target) {
	uint32 targetBattleFatigue = target->getShockWounds();

	if (player == target) {
		if (targetBattleFatigue >= 250 && targetBattleFatigue < 500) {
			player->sendSystemMessage("@healing:shock_effect_low");
		} else if (targetBattleFatigue >= 500 && targetBattleFatigue < 750) {
			player->sendSystemMessage("@healing:shock_effect_medium");
		} else if (targetBattleFatigue >= 750) {
			player->sendSystemMessage("@healing:shock_effect_high");
		}
	} else if (target->isPlayerCreature()) {
		if (targetBattleFatigue >= 250 && targetBattleFatigue < 500) {
			target->sendSystemMessage("@healing:shock_effect_low_target");
		} else if (targetBattleFatigue >= 500 && targetBattleFatigue < 750) {
			target->sendSystemMessage("@healing:shock_effect_medium_target");
		} else if (targetBattleFatigue >= 750) {
			target->sendSystemMessage("@healing:shock_effec_high_target");
		}
	}
}

int PlayerManagerImplementation::healEnhance(CreatureObject* enhancer, CreatureObject* patient, byte attribute, int buffvalue, float duration, int absorption) {
	String buffname = "medical_enhance_" + BuffAttribute::getName(attribute);
	uint32 buffcrc = buffname.hashCode();
	uint32 buffdiff = buffvalue;

	//If a stronger buff already exists, then we don't buff the patient.
	if (patient->hasBuff(buffcrc)) {
		Buff* buff = patient->getBuff(buffcrc);

		if (buff != nullptr) {
			int value = buff->getAttributeModifierValue(attribute);

			if (BuffAttribute::isProtection(attribute))
				value = buff->getSkillModifierValue(BuffAttribute::getProtectionString(attribute));

			if (value > buffvalue)
				return 0;

			buffdiff -= value;
		}
	}

	// Twi'lek race receives a 10% duration bonus
	if (patient->getSpeciesName() == "twilek") {
		duration *= 1.10f;
	}

	Reference<Buff*> buff = new Buff(patient, buffcrc, duration, BuffType::MEDICAL);

	Locker locker(buff);

	buff->setBuffName(buffname); //Infinity:  

	if (BuffAttribute::isProtection(attribute)) {
		buff->setSkillModifier(BuffAttribute::getProtectionString(attribute), buffvalue);

		if (absorption > 0)
			buff->setSkillModifier(BuffAttribute::getAbsorptionString(attribute), absorption);
	} else {
		buff->setAttributeModifier(attribute, buffvalue);
		buff->setFillAttributesOnBuff(true);
	}

	patient->addBuff(buff);

	enhancer->notifyObservers(ObserverEventType::ENHANCINGPERFORMED, patient, buffdiff);

	return buffdiff;
}

void PlayerManagerImplementation::stopListen(CreatureObject* creature, uint64 entertainerID, bool doSendPackets, bool forced, bool doLock, bool outOfRange) {
	Locker locker(creature);

	uint64 listenID = creature->getListenID();

	// If the player selected "Stop listening" by using a radial menu created on a
	// musician other than the one that they are currently listening to then change
	// entertainerID to their listenID so that the player can still stop listening.
	if (entertainerID != listenID && listenID != 0 && creature->isListening()) {
		entertainerID = listenID;
	}

	ManagedReference<SceneObject*> object = server->getObject(entertainerID);

	if (object == nullptr)
		return;

	if (object->isDroidObject()) {
		creature->setMood(creature->getMoodID());
		if (doSendPackets) {
			creature->setListenToID(0, true);
			creature->setMoodString(creature->getMoodString(), true);
		}
		if (creature->isPlayerCreature()) {
			CreatureObject* player = cast<CreatureObject*>( creature);
			StringIdChatParameter stringID;
			if (forced) {
				stringID.setTU(entertainerID);
				stringID.setStringId("performance", "music_stop_other"); // "%TU stops playing."
				player->sendSystemMessage(stringID);
				return;
			} else if (outOfRange) {
				// The correct string id is @performance:music_listen_out_of_range ("You stop listening to %TT because %OT is too far away.")
				// but %OT will get replaced by him/her which gives an incorrect message.
				StringBuffer msg;
				msg << "You stop listening to " << object->getDisplayedName() << " because they are too far away.";
				player->sendSystemMessage(msg.toString());
				return;
			} else {
				player->sendSystemMessage("@performance:music_listen_stop_self"); // "You stop listening."
				return;
			}
		}
		return;
	}

	if (!object->isPlayerCreature()) {
		creature->sendSystemMessage("@performance:music_listen_npc"); // "You cannot /listen to NPCs."
		return;
	}

	CreatureObject* entertainer = cast<CreatureObject*>( object.get());

	if (entertainer == creature)
		return;

	String entName;
	ManagedReference<EntertainingSession*> esession;

	if (entertainer != nullptr) {
		Locker clocker(entertainer, creature);

		entName = entertainer->getFirstName();

		ManagedReference<Facade*> session = entertainer->getActiveSession(SessionFacadeType::ENTERTAINING);

		if (session != nullptr) {
			esession = dynamic_cast<EntertainingSession*>(session.get());

			if (esession != nullptr) {
				esession->activateEntertainerBuff(creature, PerformanceType::MUSIC);

				esession->removePatron(creature);
			}
		}

		clocker.release();
	}

	creature->setMood(creature->getMoodID());

	if (doSendPackets && esession != nullptr)
		esession->sendEntertainmentUpdate(creature, 0, creature->getMoodString());

	if (creature->isPlayerCreature() && entertainer != nullptr) {
		CreatureObject* player = cast<CreatureObject*>( creature);

		StringIdChatParameter stringID;

		if (forced) {
			stringID.setTU(entertainerID);
			stringID.setStringId("performance", "music_stop_other"); // "%TU stops playing."

			player->sendSystemMessage(stringID);
		} else if (outOfRange) {
			// The correct string id is @performance:music_listen_out_of_range ("You stop listening to %TT because %OT is too far away.")
			// but %OT will get replaced by him/her which gives an incorrect message.
			StringBuffer msg;
			msg << "You stop listening to " << entertainer->getFirstName() << " because they are too far away.";
			player->sendSystemMessage(msg.toString());
		} else {
			player->sendSystemMessage("@performance:music_listen_stop_self"); // "You stop listening."
		}

		ManagedReference<PlayerObject*> entPlayer = entertainer->getPlayerObject();
		if (entPlayer != nullptr && entPlayer->getPerformanceBuffTarget() == player->getObjectID())
			entPlayer->setPerformanceBuffTarget(0);
	}
	//esession->setEntertainerBuffDuration(creature, PerformanceType::MUSIC, 0.0f); // reset
	//esession->setEntertainerBuffStrength(creature, PerformanceType::MUSIC, 0.0f);
	creature->info("stopped listening [" + entName + "]");

	creature->setListenToID(0, true);
}


void PlayerManagerImplementation::stopWatch(CreatureObject* creature, uint64 entertainerID, bool doSendPackets, bool forced, bool doLock, bool outOfRange) {
	Locker locker(creature);

	uint64 watchID = creature->getWatchToID();

	// If the player selected "Stop watching" by using a radial menu created on a
	// dancer other than the one that they are currently watching then change
	// entertainerID to their watchID so that the player can still stop watching.
	if (entertainerID != watchID && watchID != 0 && creature->isWatching()) {
		entertainerID = watchID;
	}

	ManagedReference<SceneObject*> object = server->getObject(entertainerID);

	if (object == nullptr)
		return;

	if (!object->isPlayerCreature()) {
		creature->sendSystemMessage("@performance:dance_watch_npc"); // "You cannot /watch NPCs."
		return;
	}

	CreatureObject* entertainer = cast<CreatureObject*>( object.get());

	if (entertainer == creature)
		return;

	ManagedReference<EntertainingSession*> esession = nullptr;

	String entName;
	if (entertainer != nullptr) {
		Locker clocker(entertainer, creature);

		entName = entertainer->getFirstName();

		ManagedReference<Facade*> session = entertainer->getActiveSession(SessionFacadeType::ENTERTAINING);

		if (session != nullptr) {
			esession = dynamic_cast<EntertainingSession*>(session.get());

			if (esession != nullptr) {
				esession->activateEntertainerBuff(creature, PerformanceType::DANCE);

				esession->removePatron(creature);
			}
		}

		clocker.release();
	}

	creature->setMood(creature->getMoodID());

	if (doSendPackets && esession != nullptr)
		esession->sendEntertainmentUpdate(creature, 0, creature->getMoodString());

	//System Message.
	if (creature->isPlayerCreature() && entertainer != nullptr) {
		CreatureObject* player = cast<CreatureObject*>( creature);

		StringIdChatParameter stringID;

		if (forced) {
			stringID.setTU(entertainerID);
			stringID.setStringId("performance", "dance_stop_other"); // %TU stops dancing.

			player->sendSystemMessage(stringID);
		} else if (outOfRange) {
			// The correct string id is @performance:dance_watch_out_of_range ("You stop watching %TT because %OT is too far away.")
			// but %OT will get replaced by him/her which gives an incorrect message.
			StringBuffer msg;
			msg << "You stop watching " << entertainer->getFirstName() << " because they are too far away.";
			player->sendSystemMessage(msg.toString());
		} else {
			player->sendSystemMessage("@performance:dance_watch_stop_self"); //"You stop watching."
		}

		ManagedReference<PlayerObject*> entPlayer = entertainer->getPlayerObject();
		if (entPlayer != nullptr && entPlayer->getPerformanceBuffTarget() == player->getObjectID())
			entPlayer->setPerformanceBuffTarget(0);
	}


	//esession->setEntertainerBuffDuration(creature, PerformanceType::DANCE, 0.0f); // reset
	//esession->setEntertainerBuffStrength(creature, PerformanceType::DANCE, 0.0f);
	creature->info("stopped watching [" + entName + "]");

	creature->setWatchToID(0);
	/*doWatching = false;
	watchID = 0;*/
}

void PlayerManagerImplementation::startWatch(CreatureObject* creature, uint64 entertainerID) {
	Locker locker(creature);

	uint64 watchID = creature->getWatchToID();

	if (watchID == entertainerID)
		return;

	ManagedReference<SceneObject*> object = server->getObject(entertainerID);

	if (object == nullptr)
		return;

	if (!object->isPlayerCreature()) {
		creature->sendSystemMessage("@performance:dance_watch_npc"); // "You can not /watch NPCs."
		return;
	}

	if (!CollisionManager::checkLineOfSight(object, creature)) {
		creature->sendSystemMessage("@healing:no_line_of_sight"); // You cannot see your target.
		return;
	}

	CreatureObject* entertainer = cast<CreatureObject*>(object.get());

	if (entertainer == nullptr || creature == entertainer)
		return;

	Locker clocker(entertainer, creature);

	if (creature->isDancing() || creature->isPlayingMusic()) {
		StringIdChatParameter stringID;
		stringID.setStringId("cmd_err", "locomotion_prose"); // "You cannot %TO while %TU."
		stringID.setTO("/watch");
		stringID.setTU("@locomotion_n:skillanimating"); // "Skill Animating"
		creature->sendSystemMessage(stringID);

		return;
	} else if (!entertainer->isDancing()) {
		StringIdChatParameter stringID;
		stringID.setStringId("performance", "dance_watch_not_dancing"); // "%TT is not dancing."
		stringID.setTT(entertainerID);
		creature->sendSystemMessage(stringID);
		return;
	}

	ManagedReference<EntertainingSession*> entertainingSession = entertainer->getActiveSession(SessionFacadeType::ENTERTAINING).castTo<EntertainingSession*>();

	if (entertainingSession == nullptr)
		return;

	if (creature->isWatching()) {
		stopWatch(creature, watchID, false);
	}

	entertainingSession->sendEntertainmentUpdate(creature, entertainerID, "entertained");
	entertainingSession->addPatron(creature);

	entertainer->notifyObservers(ObserverEventType::WASWATCHED, creature);

	StringIdChatParameter stringID;
	stringID.setStringId("performance", "dance_watch_self"); // You start watching %TT.
	stringID.setTT(entertainerID);
	creature->sendSystemMessage(stringID);

	creature->info("started watching [" + entertainer->getCustomObjectName().toString() + "]");

	creature->setWatchToID(entertainer->getObjectID());
	//watchID =  entertainerID;
}

void PlayerManagerImplementation::startListen(CreatureObject* creature, uint64 entertainerID) {
	Locker locker(creature);

	ManagedReference<SceneObject*> object = server->getObject(entertainerID);
	uint64 listenID = creature->getListenID();

	if (listenID == entertainerID)
		return;

	if (object == nullptr)
		return;

	if (!CollisionManager::checkLineOfSight(object, creature)) {
		creature->sendSystemMessage("@healing:no_line_of_sight"); // You cannot see your target.
		return;
	}

	// Droid playback handling
	if (object->isDroidObject()) {
		DroidObject* droid = cast<DroidObject*>( object.get());
		if (droid == nullptr) {
			creature->sendSystemMessage("@performance:music_listen_npc"); // "You cannot /listen to NPCs."
			return;
		}
		auto bmodule = droid->getModule("playback_module");
		if (bmodule != nullptr) {
			DroidPlaybackModuleDataComponent* module = cast<DroidPlaybackModuleDataComponent*>(bmodule.get());
			if (module != nullptr) {
				if (creature->isDancing() || creature->isPlayingMusic()) {
					StringIdChatParameter stringID;
					stringID.setStringId("cmd_err", "locomotion_prose"); // "You cannot %TO while %TU."
					stringID.setTO("/listen");
					stringID.setTU("@locomotion_n:skillanimating"); // "Skill Animating"
					creature->sendSystemMessage(stringID);
					return;
				}

				if (module->isPlayingMusic()) {
					// the droid is playing so we can do something
					if (creature->isListening()) {
						stopListen(creature, listenID, false);
					}

					StringIdChatParameter stringID;
					stringID.setTT(entertainerID);
					stringID.setStringId("performance", "music_listen_self"); // "You start listening to %TT."
					creature->sendSystemMessage(stringID);

					creature->setListenToID(entertainerID, true);
					String str = server->getChatManager()->getMoodAnimation("entertained");
					creature->setMoodString(str, true);
					creature->setListenToID(droid->getObjectID());
					module->addListener(creature->getObjectID());
					return;
				} else {
					StringIdChatParameter stringID;
					stringID.setTT(entertainerID);
					stringID.setStringId("performance", "music_listen_not_playing"); // %TT is not playing a song.
					creature->sendSystemMessage(stringID);
					return;
				}
			} else {
				creature->sendSystemMessage("@performance:music_listen_npc"); // "You cannot /listen to NPCs."
			}
		} else {
			creature->sendSystemMessage("@performance:music_listen_npc"); // "You cannot /listen to NPCs."
		}
		return;
	}

	if (!object->isPlayerCreature()) {
		creature->sendSystemMessage("@performance:music_listen_npc"); // "You cannot /listen to NPCs."
		return;
	}

	CreatureObject* entertainer = cast<CreatureObject*>( object.get());

	if (creature == entertainer)
		return;

	Locker clocker(entertainer, creature);

	if (creature->isDancing() || creature->isPlayingMusic()) {
		StringIdChatParameter stringID;
		stringID.setStringId("cmd_err", "locomotion_prose"); // "You cannot %TO while %TU."
		stringID.setTO("/listen");
		stringID.setTU("@locomotion_n:skillanimating"); // "Skill Animating"
		creature->sendSystemMessage(stringID);
		return;
	} else if (!entertainer->isPlayingMusic()) {
		StringIdChatParameter stringID;
		stringID.setTT(entertainerID);
		stringID.setStringId("performance", "music_listen_not_playing"); // %TT is not playing a song.
		creature->sendSystemMessage(stringID);

		return;
	}

	ManagedReference<Facade*> facade = entertainer->getActiveSession(SessionFacadeType::ENTERTAINING);

	if (facade == nullptr)
		return;

	EntertainingSession* entertainingSession = dynamic_cast<EntertainingSession*>(facade.get());

	if (entertainingSession == nullptr)
		return;

	if (creature->isListening()) {
		stopListen(creature, listenID, false);
	}

	//sendEntertainmentUpdate(entertainerID, "entertained");

	entertainingSession->sendEntertainmentUpdate(creature, entertainerID, "entertained");
	entertainingSession->addPatron(creature);

	entertainer->notifyObservers(ObserverEventType::WASLISTENEDTO, creature);

	//creature->addWatcher(_this);

	StringIdChatParameter stringID;
	stringID.setTT(entertainerID);
	stringID.setStringId("performance", "music_listen_self"); // "You start listening to %TT."
	creature->sendSystemMessage(stringID);

	//setEntertainerBuffDuration(PerformanceType::DANCE, 0.0f);
	//setEntertainerBuffStrength(PerformanceType::DANCE, 0.0f);

	creature->info("started listening to [" + entertainer->getCustomObjectName().toString() + "]");

	creature->setListenToID(entertainer->getObjectID());
	//watchID =  entertainerID;
}


SceneObject* PlayerManagerImplementation::getInRangeStructureWithAdminRights(CreatureObject* creature, uint64 targetID) {
	ZoneServer* zoneServer = server;

	//First we need to check if the target. if it's not right...
	//Check the building they are standing in, if it's not right...
	//Find the nearest installation.

	ManagedReference<SceneObject*> obj = nullptr;

	if (targetID != 0) {
		obj = zoneServer->getObject(targetID);

		if (obj != nullptr && obj->isStructureObject() && (cast<StructureObject*>(obj.get()))->isOnAdminList(creature))
			return obj.get();
	}

	ManagedReference<SceneObject*> rootParent = creature->getRootParent();

	if (rootParent != nullptr) {
		// Structure Object
		if (rootParent->isStructureObject()) {
			StructureObject* structure = cast<StructureObject*>(rootParent.get());

			if (structure!= nullptr && structure->isOnAdminList(creature))
				return rootParent;
		// Infinity: POB Ship
		} else if (rootParent->isPobShip()) {
			PobShipObject* pobShip = cast<PobShipObject*>(rootParent.get());

			if (pobShip != nullptr && pobShip->isOnAdminList(creature))
				return rootParent;
		}
	}

	StructureObject* structure = nullptr;
	float distance = 16000;

	Zone* zone = creature->getZone();

	if (zone == nullptr) {
		return nullptr;
	}

	//We need to search nearby for an installation that belongs to the player.
	Locker _locker(zone);

	CloseObjectsVector* closeObjs = (CloseObjectsVector*)creature->getCloseObjects();
	SortedVector<TreeEntry*> closeObjects;
	closeObjs->safeCopyReceiversTo(closeObjects, CloseObjectsVector::STRUCTURETYPE);

	for (int i = 0; i < closeObjects.size(); ++i) {
		ManagedReference<SceneObject*> tObj = cast<SceneObject*>( closeObjects.get(i));

		if (tObj != nullptr) {
			if (tObj->isStructureObject()) {
				float dist = tObj->getDistanceTo(creature);

				StructureObject* structureObject = cast<StructureObject*>( tObj.get());

				if (structureObject->isOnAdminList(creature) && dist < distance) {
					structure = structureObject;
					distance = dist;
				}
			}
		}
	}

	if (distance < 25)
		return structure;

	return nullptr;
}

StructureObject* PlayerManagerImplementation::getInRangeOwnedStructure(CreatureObject* creature, float range) {
	Locker _lock(creature);

	ManagedReference<PlayerObject*> ghost = creature->getPlayerObject();

	if (ghost == nullptr)
		return nullptr;

	ManagedReference<StructureObject*> closestStructure = nullptr;
	float closestDistance = 16000.f;

	for (int i = 0; i < ghost->getTotalOwnedStructureCount(); ++i) {
		uint64 oid = ghost->getOwnedStructure(i);

		ManagedReference<StructureObject*> structure = (ghost->getZoneServer()->getObject(oid)).castTo<StructureObject*>();

		Locker _slock(structure, creature);

		if (creature->getZone() != structure->getZone())
			continue;

		float distance = structure->getDistanceTo(creature);

		if (distance <= range && distance < closestDistance) {
			closestStructure = structure;
			closestDistance = distance;
		}
	}

	return closestStructure;
}

void PlayerManagerImplementation::updatePermissionLevel(CreatureObject* targetPlayer, int permissionLevel) {

	if (targetPlayer == nullptr) {
		return;
	}

	Locker locker(targetPlayer);
	ManagedReference<PlayerObject*> ghost = targetPlayer->getPlayerObject();

	if (ghost == nullptr) {
		return;
	}

	SkillManager* skillManager = server->getSkillManager();

	int currentPermissionLevel = ghost->getAdminLevel();

	/*Temporarily removed so that we can update admin levels immediately
	if (currentPermissionLevel == permissionLevel)
		return;*/

	if (currentPermissionLevel != 0) {
		Vector<String>* skillsToBeRemoved = permissionLevelList->getPermissionSkills(currentPermissionLevel);
		if (skillsToBeRemoved != nullptr) {
			for (int i = 0; i < skillsToBeRemoved->size(); i++) {
				const String& skill = skillsToBeRemoved->get(i);
				targetPlayer->sendSystemMessage("Staff skill revoked: " + skill);
				skillManager->surrenderSkill(skill, targetPlayer, true);
			}
		}
	}

	ghost->setAdminLevel(permissionLevel);

	if (permissionLevel != 0) {
		Vector<String>* skillsToBeAdded = permissionLevelList->getPermissionSkills(permissionLevel);
		if (skillsToBeAdded != nullptr) {
			for (int i = 0; i < skillsToBeAdded->size(); ++i) {
				const String& skill = skillsToBeAdded->get(i);
				targetPlayer->sendSystemMessage("Staff skill granted: " + skill);
				skillManager->awardSkill(skill, targetPlayer, false, true, true);
			}
		}
	}

	updatePermissionName(targetPlayer, permissionLevel);
}

void PlayerManagerImplementation::updatePermissionName(CreatureObject* player, int permissionLevel) {
	ManagedReference<PlayerObject*> ghost = player->getPlayerObject();
	int priviledgeFlag = permissionLevelList->getPriviledgeFlag(permissionLevel);

	ghost->setPriviledgeFlag(priviledgeFlag);
	//Send deltas
	if (player->isOnline()) {
		UnicodeString tag = permissionLevelList->getPermissionTag(permissionLevel);

		TangibleObjectDeltaMessage3* tanod3 = new TangibleObjectDeltaMessage3(player);
		tanod3->updateCustomName(player->getDisplayedName(), tag);
		tanod3->close();
		player->broadcastMessage(tanod3, true);

		ghost->updateInRangeBuildingPermissions();

		/*PlayerObjectDeltaMessage6* playd6 = new PlayerObjectDeltaMessage6(ghost);
			playd6->setAdminLevel(adminLevel);
			playd6->close();
			player->broadcastMessage(playd6, true);*/
	}
}

void PlayerManagerImplementation::updateSwimmingState(CreatureObject* player, float newZ, IntersectionResults* intersections, CloseObjectsVector* closeObjectsVector) {
	player->notifySelfPositionUpdate();

	auto parent = player->getParent().get();

	if (parent != nullptr && !parent->isVehicleObject() && !parent->isMount()) {
		return;
	}

	CreatureObject* mount = nullptr;

	if (parent != nullptr && parent->isMount()) {
		mount = parent->asCreatureObject();
	}

	Zone* zone = player->getZone();

	if (zone == nullptr) {
		// player->info("No zone.", true);
		return;
	}

	PlanetManager* planetManager = zone->getPlanetManager();

	if (planetManager == nullptr) {
		// player->info("No planet manager.", true);
		return;
	}

	TerrainManager* terrainManager = planetManager->getTerrainManager();

	if (terrainManager == nullptr) {
		// player->info("No terrain manager.", true);
		return;
	}


	float landHeight = zone->getHeight(player->getPositionX(), player->getPositionY());
	float waterHeight = landHeight;
	bool waterIsDefined = terrainManager->getWaterHeight(player->getPositionX(), player->getPositionY(), waterHeight);

	if (waterIsDefined && (waterHeight > landHeight) && (landHeight + player->getSwimHeight() - waterHeight < 0.2)) {
		//Water level is higher than the terrain level and is deep enough for the player to be swimming.
		//Check if the player is on a bridge or other terrain above the water level.
		//SortedVector<IntersectionResult> intersections;
		Reference<IntersectionResults*> ref;

		if (intersections == nullptr) {
			ref = intersections = new IntersectionResults();

			CollisionManager::getWorldFloorCollisions(player->getPositionX(), player->getPositionY(), zone, intersections, closeObjectsVector);
		}

		for (int i = 0; i < intersections->size(); i++) {
			if (fabs(16384 - intersections->get(i).getIntersectionDistance() - newZ) < 0.2) {
				//Player is on terrain above the water.
				player->clearState(CreatureState::SWIMMING);
				return;
			}
		}

		// Player is in the water.
		player->setState(CreatureState::SWIMMING);

		if (mount != nullptr) {
			Locker clock(mount, player);

			mount->setState(CreatureState::SWIMMING);
		}

		return;
	}

	// Terrain is above water level.
	player->clearState(CreatureState::SWIMMING);

	if (mount != nullptr) {
		Locker clock(mount, player);

		mount->clearState(CreatureState::SWIMMING);
	}
}

bool PlayerManagerImplementation::checkPlayerSpeedTest(CreatureObject* player, SceneObject* parent, float parsedSpeed, ValidatedPosition* lastValidPosition, const Vector3& lastValidatedWorldPosition, const Vector3& newWorldPosition, float errorMultiplier) {
	float allowedSpeedMod = player->getSpeedMultiplierMod();
	float allowedSpeedBase = player->getRunSpeed();

	SpeedMultiplierModChanges* changeBuffer = player->getSpeedMultiplierModChanges();

	Vector3 lastValidVec = lastValidPosition->getPosition();
	uint64 lastValidParentParentID = lastValidPosition->getParent();

	if (parent != nullptr) {
		if (parent->isVehicleObject()) {
			VehicleObject* vehicle = cast<VehicleObject*>(parent);

			allowedSpeedMod = vehicle->getSpeedMultiplierMod();
			allowedSpeedBase = vehicle->getRunSpeed();
		} else if (parent->isMount()) {
			Creature* mount = cast<Creature*>(parent);

			allowedSpeedMod = mount->getSpeedMultiplierMod();

			PetManager* petManager = server->getPetManager();

			if (petManager != nullptr) {
				allowedSpeedBase = petManager->getMountedRunSpeed(mount);
			}
		}
	}

	float maxAllowedSpeed = allowedSpeedMod * allowedSpeedBase;
	float maxSpeedVariable = (maxAllowedSpeed * errorMultiplier);

#ifdef DEBUG_SPEED_HACK
	auto speedMsg = player->info(true);
	speedMsg << "checkPlayerSpeedTest -- parsedSpeed: " << parsedSpeed << " Max Allowed Speed: " << maxSpeedVariable << " Error Multiplier: " << errorMultiplier << endl;
	speedMsg << "checkPlayerSpeedTest -- Player Run Speed: " << allowedSpeedBase << " Player Run Modifier: " << allowedSpeedMod;
	speedMsg.flush();
#endif // DEBUG_SPEED_HACK

	if (parsedSpeed > maxSpeedVariable) {
		// Infinity: shuttle disembark teleports bypass the speed budget by
		// design — skip the OVER log if the player is in the travel-immunity
		// window set by BoardShuttleCommand.
		if (!player->checkCooldownRecovery("travelImmunity")) {
			return true;
		}

		// Infinity: Log-only mode — no enforcement until thresholds are dialed in.
		// Do NOT reject movement or teleport players. Just log for analysis.
		auto zone = player->getZoneUnsafe();
		String zoneName = zone != nullptr ? zone->getZoneName() : "unknown";
		String cellInfo = "none";
		ManagedReference<SceneObject*> parent = player->getParent().get();
		if (parent != nullptr) {
			cellInfo = String::valueOf(parent->getObjectID());
			SceneObject* root = parent->getRootParent();
			if (root != nullptr && root->getObjectTemplate() != nullptr)
				cellInfo = cellInfo + " building=" + root->getObjectTemplate()->getFullTemplateString();
		}
		float wx = player->getWorldPositionX();
		float wy = player->getWorldPositionY();

		if (changeBuffer->size() == 0) {
			player->warning() << "[SpeedCheck] OVER — " << player->getDisplayedName() << " allowed: " << maxSpeedVariable << " parsed: " << parsedSpeed << " (no speed changes)"
				<< " zone=" << zoneName << " cell=" << cellInfo << " pos=(" << wx << "," << wy << ")";
		} else {
			SpeedModChange* latestChange = &changeBuffer->get(changeBuffer->size() - 1);
			const Time* timeStamp = &latestChange->getTimeStamp();

			bool bufferExplains = false;
			for (int i = 0; i < changeBuffer->size(); ++i) {
				SpeedModChange* change = &changeBuffer->get(i);
				float allowed = allowedSpeedBase * change->getNewSpeed() * errorMultiplier;

				if (allowed >= parsedSpeed) {
					bufferExplains = true;
					break;
				}
			}

			if (!bufferExplains) {
				player->warning() << "[SpeedCheck] OVER — " << player->getDisplayedName() << " allowed: " << maxSpeedVariable << " parsed: " << parsedSpeed << " bufferSize: " << changeBuffer->size() << " staleness: " << timeStamp->miliDifference() << "ms"
					<< " zone=" << zoneName << " cell=" << cellInfo << " pos=(" << wx << "," << wy << ")";
			}
		}

		return true; // Infinity: accept all movement, log only
	}

#ifdef DEBUG_SPEED_HACK
	player->info() << "checkPlayerSpeedTest -- PASSED";
#endif // DEBUG_SPEED_HACK

	return true;
}

int PlayerManagerImplementation::checkSpeedHackTests(CreatureObject* player, PlayerObject* ghost, ValidatedPosition* lastValidPosition, const Vector3& newPosition, uint32 newStamp, float floorZ, SceneObject* newParent) {
	if (player == nullptr || ghost == nullptr) {
		return Transform::INVALID_POSITION;
	}

	// newStamp - stamp;
	uint32 deltaTime = ghost->getServerMovementTimeDelta();

	// Infinity: Configurable delta check interval
	uint32 deltaCheck = ConfigManager::instance()->getInt("Core3.PlayerManager.SpeedHackDeltaCheck", DEFAULT_SPEED_HACK_DELTA_CHECK);

	if (deltaTime < deltaCheck) {
#ifdef DEBUG_SPEED_HACK
		player->info(true)  << "checkSpeedHackTests -- Returning Transform::TIME_VALIDATED due to being below DELTA_SPEED_CHECK -- Delta Time: " << deltaTime;
#endif // DEBUG_SPEED_HACK

		return Transform::TIME_VALIDATED;
	}

	// Infinity: Class 3 stale-buffer guard. When deltaTime is huge (logout +
	// re-login, zone transition, link-dead resume), lastValidPosition is from
	// a prior session; computing speed against it produces bogus [SpeedCheck]
	// OVER hits in the hundreds-to-thousands m/s range. Return FULL_VALIDATED
	// so the caller (DataTransform::updatePosition) re-anchors validPosition
	// to the current packet for a fresh reference on the next frame.
	static const uint32 STALE_BUFFER_MS = 30000;
	if (deltaTime > STALE_BUFFER_MS) {
		return Transform::FULL_VALIDATED;
	}

	uint32 stamp = ghost->getClientLastMovementStamp();

	if (stamp > newStamp) {
#ifdef DEBUG_SPEED_HACK
		player->info(true)  << "checkSpeedHackTests -- FAILED -- older client movement stamp received";
#endif // DEBUG_SPEED_HACK
		return Transform::INVALID_POSITION;
	}

	auto newWorldPosition(newPosition);
	auto lastValidatedWorldPosition = lastValidPosition->getWorldPosition(server);

	if (newParent != nullptr) {
		ManagedReference<SceneObject*> root = newParent->getRootParent();

		// Infinity: Ship interiors are valid movement parents (like buildings)
		if (!root->isBuildingObject() && !root->isShipObject()) {
#ifdef DEBUG_SPEED_HACK
			player->info()  << "checkSpeedHackTests -- FAILED due to improper root parent type.";
#endif // DEBUG_SPEED_HACK
			return Transform::INVALID_POSITION;
		}

		float newX = newPosition.getX();
		float newY = newPosition.getY();

		float length = Math::sqrt(newX * newX + newY * newY);
		float angle = root->getDirection()->getRadians() + atan2(newX, newY);

		newWorldPosition.setX(root->getPositionX() + (sin(angle) * length));
		newWorldPosition.setY(root->getPositionY() + (cos(angle) * length));
		newWorldPosition.setZ(root->getPositionZ() + newPosition.getZ());

#ifdef DEBUG_SPEED_HACK
		player->info(true) << "checkSpeedHackTests -- Parent Transform with newWorldPosition: " << newWorldPosition.toString() << " Validated World Position: " << lastValidatedWorldPosition.toString() << " Distance Length = " << length;
#endif // DEBUG_SPEED_HACK
	} else {
		newWorldPosition.setZ(0);
		lastValidatedWorldPosition.setZ(0.f);

#ifdef DEBUG_SPEED_HACK
		player->info(true) << "checkSpeedHackTests -- Non-Parent Transform newWorldPosition: " << newWorldPosition.toString() << " Validated World Position: " << lastValidatedWorldPosition.toString();
#endif // DEBUG_SPEED_HACK
	}

	float movementDistance = newWorldPosition.distanceTo2d(lastValidatedWorldPosition);

	// Infinity: Refresh the latest speed mod change timestamp so mounted speed
	// entries don't go stale after 2 seconds and cause false speed hack rejects.
	// Must happen BEFORE the speed test — otherwise a failed check prevents the
	// refresh, and every subsequent check also fails due to staleness (death spiral).
	SpeedMultiplierModChanges* changeBuffer = player->getSpeedMultiplierModChanges();

	if (changeBuffer != nullptr && changeBuffer->size() > 0) {
		SpeedModChange& latestChange = changeBuffer->get(changeBuffer->size() - 1);
		float currentMod = latestChange.getNewSpeed();
		changeBuffer->set(changeBuffer->size() - 1, SpeedModChange(currentMod));
	}

	if (movementDistance > 1.f && !ghost->isPrivileged()) {
		// Infinity: Grace period during mount/dismount transition — the client moves at
		// the new speed before the server parent is updated, causing false speed rejects.
		if (!player->checkCooldownRecovery("mount_dismount")) {
			return Transform::FULL_VALIDATED;
		}

		// Infinity: Grace period after shuttle/teleport travel. The arrival position is
		// already re-anchored into validPosition by every travelImmunity setter
		// (BoardShuttleCommand + LuaSceneObject switchZone/teleport all route through
		// PlayerZoneComponent::switchZone/teleport -> updateLastValidatedPosition). Return
		// TIME_VALIDATED (accept the move, do NOT re-anchor) instead of FULL_VALIDATED so a
		// stale in-flight client transform still reporting the pre-travel (departure)
		// position can't overwrite that arrival anchor. The old FULL_VALIDATED here
		// re-anchored validPosition to the client packet (DataTransform.h), so a lagging
		// departure transform during the immunity window clobbered the arrival anchor; once
		// immunity expired the first true arrival transform computed an impossible
		// departure->arrival speed and fired a false [SpeedCheck] OVER (ticket-6179,
		// chained-travel FP).
		if (!player->checkCooldownRecovery("travelImmunity")) {
			return Transform::TIME_VALIDATED;
		}

		float speed = movementDistance / ((float)deltaTime / 1000.f);

#ifdef DEBUG_SPEED_HACK
		player->info(true) << "checkSpeedHackTests -- Next Position Distance: " << movementDistance << " Speed: " << speed << " Delta Time: " << deltaTime;
#endif // DEBUG_SPEED_HACK

		ManagedReference<SceneObject*> parent = player->getParent().get();

		// Infinity: Configurable error tolerance
		float baseTolerance = ConfigManager::instance()->getFloat("Core3.PlayerManager.SpeedHackErrorTolerance", DEFAULT_SPEED_HACK_ERROR_TOLERANCE);
		float errorVariable = baseTolerance + Math::min(0.05f, (speed * 0.00103f));


		// Run speed tests
		if (!checkPlayerSpeedTest(player, parent, speed, lastValidPosition, lastValidatedWorldPosition, newWorldPosition, errorVariable)) {
			return Transform::INVALID_POSITION;
		}
	}

	if (ghost->isOnLoadScreen()) {
		ghost->setOnLoadScreen(false);
	}

	ghost->incrementSessionMovement(movementDistance);

#ifdef DEBUG_SPEED_HACK
	player->info(true) << "checkSpeedHackTests -- PASSED -- Distance: " << movementDistance;
#endif // DEBUG_SPEED_HACK

	return Transform::FULL_VALIDATED;
}

void PlayerManagerImplementation::lootAll(CreatureObject* player, CreatureObject* ai) {
	Locker locker(ai, player);

	if (!ai->isDead() || player->isDead())
		return;

	SceneObject* creatureInventory = ai->getInventory();

	if (creatureInventory == nullptr)
		return;

	auto trxGroup = TransactionLog::getNewTrxGroup();

	int cashCredits = ai->getCashCredits();

	if (cashCredits > 0) {
		int luck = player->getSkillMod("force_luck");

		if (luck > 0)
			cashCredits += (cashCredits * luck) / 20;

		{
			TransactionLog trx(ai, player, TrxCode::NPCLOOTCLAIM, cashCredits, true);
			trx.setTrxGroup(trxGroup);
			trx.addState("srcDisplayedName", ai->getDisplayedName());
			player->addCashCredits(cashCredits, true);
			ai->clearCashCredits();
		}

		StringIdChatParameter param("base_player", "prose_coin_loot"); //You loot %DI credits from %TT.
		param.setDI(cashCredits);
		param.setTT(ai->getObjectID());

		player->sendSystemMessage(param);
	}

	ai->notifyObservers(ObserverEventType::LOOTCREATURE, player, 0);

	SceneObject* playerInventory = player->getInventory();

	if (playerInventory == nullptr)
		return;

	if (creatureInventory->getContainerObjectsSize() < 1) {
		rescheduleCorpseDestruction(player, ai);
		return;
	}

	for (int i = creatureInventory->getContainerObjectsSize() - 1; i >= 0; --i) { //Infinity:  check size each time
		SceneObject* object = creatureInventory->getContainerObject(i);

		TransactionLog trx(ai, player, object, TrxCode::NPCLOOTCLAIM);
		trx.setTrxGroup(trxGroup);

		TransferItemMiscCommand::doTransferItemMisc(player, object, playerInventory, -1, trx, 24.5f);  //Increase distance for lootArea to 24 meters
	}

	if (creatureInventory->getContainerObjectsSize() <= 0) {
		player->sendSystemMessage("@base_player:corpse_looted"); //You have completely looted the corpse of all items.

		rescheduleCorpseDestruction(player, ai);
	}
}

void PlayerManagerImplementation::sendStartingLocationsTo(CreatureObject* player) {
	StartingLocationListMessage* slm = new StartingLocationListMessage(player);
	startingLocationList.insertToMessage(slm);

	player->sendMessage(slm);
}

StartingLocation* PlayerManagerImplementation::getStartingLocation(const String& city) {
	for (int i = 0; i < startingLocationList.size(); ++i) {
		StartingLocation* loc = &startingLocationList.get(i);

		if (loc == nullptr) {
			continue;
		}

		if (loc->getLocation() == city) {
			return loc;
		}
	}

	return nullptr;
}

StartingLocation* PlayerManagerImplementation::getFirstAvailableStartingLocation() {
	for (int i = 0; i < startingLocationList.size(); ++i) {
		StartingLocation* loc = &startingLocationList.get(i);

		if (loc == nullptr) {
			continue;
		}

		auto zone = server->getZone(loc->getZoneName());

		if (zone != nullptr) {
			return loc;
		}
	}

	return nullptr;
}

void PlayerManagerImplementation::addInsurableItemsRecursive(SceneObject* obj, SortedVector<ManagedReference<SceneObject*> >* items, bool onlyInsurable) {
	for (int j = 0; j < obj->getContainerObjectsSize(); j++) {
		SceneObject* object = obj->getContainerObject(j);

		if (!object->isTangibleObject())
			continue;

		TangibleObject* item = cast<TangibleObject*>( object);

		if (item == nullptr || item->hasAntiDecayKit() || item->isJediRobe() || item->isUnionRing() || !item->isInsurable())
			continue;

		if (!(item->getOptionsBitmask() & OptionBitmask::INSURED) && (item->isArmorObject() || item->isWearableObject())) {
			items->put(item);
		} else if ((item->getOptionsBitmask() & OptionBitmask::INSURED) && (item->isArmorObject() || item->isWearableObject()) && !onlyInsurable) {
			items->put(item);
		}

		if (object->isContainerObject())
			addInsurableItemsRecursive(object, items, onlyInsurable);
	}
}

SortedVector<ManagedReference<SceneObject*> > PlayerManagerImplementation::getInsurableItems(CreatureObject* player, bool onlyInsurable) {
	SortedVector<ManagedReference<SceneObject*> > insurableItems;
	insurableItems.setNoDuplicateInsertPlan();

	if (player == nullptr)
		return insurableItems;

	SceneObject* datapad = player->getSlottedObject("datapad");
	SceneObject* defweapon = player->getSlottedObject("default_weapon");
	SceneObject* bank = player->getSlottedObject("bank");

	for (int i = 0; i < player->getSlottedObjectsSize(); ++i) {
		SceneObject* container = player->getSlottedObject(i);

		if (container == datapad || container == nullptr || container == bank || container == defweapon)
			continue;

		if (container->isTangibleObject()) {
			TangibleObject* item = cast<TangibleObject*>( container);

			if (item == nullptr || item->hasAntiDecayKit() || item->isJediRobe() || item->isUnionRing() || !item->isInsurable())
				continue;

			if (!(item->getOptionsBitmask() & OptionBitmask::INSURED) && (item->isArmorObject() || item->isWearableObject())) {
				insurableItems.put(item);
			} else if ((item->getOptionsBitmask() & OptionBitmask::INSURED) && (item->isArmorObject() || item->isWearableObject()) && !onlyInsurable) {
				insurableItems.put(item);
			}
		}

		addInsurableItemsRecursive(container, &insurableItems, onlyInsurable);
	}

	return insurableItems;
}

SortedVector<ManagedReference<SceneObject*> > PlayerManagerImplementation::getInventoryItemsOfType(CreatureObject* player, int mask) {
	SortedVector<ManagedReference<SceneObject*> > matchedItems;
	matchedItems.setNoDuplicateInsertPlan();

	if (player == nullptr)
		return matchedItems;

	SceneObject* inventory = player->getInventory();

	if ( inventory == nullptr )
		return matchedItems;

	for (int j = 0; j < inventory->getContainerObjectsSize(); j++) {
		SceneObject* item = inventory->getContainerObject(j);

		if (!item->isTangibleObject())
			continue;

		TangibleObject* tano = cast<TangibleObject*>( item);

		if (tano == nullptr )
			continue;

		if ( (tano -> getGameObjectType()) & mask ) {
			matchedItems.put(tano);
		}
	}

	return matchedItems;
}

int PlayerManagerImplementation::calculatePlayerLevel(CreatureObject* player) {

	ManagedReference<WeaponObject*> weapon = player->getWeapon();

	if (weapon == nullptr) {
		player->error("player with nullptr weapon");

		return 0;
	}

	String weaponType = weapon->getWeaponType();
	int skillMod = player->getSkillMod("private_" + weaponType + "_combat_difficulty");

	if (player->getPlayerObject() != nullptr && player->getPlayerObject()->isJedi() && weapon->isJediWeapon())
		skillMod += player->getSkillMod("private_jedi_difficulty");

	int level = Math::min(25, skillMod / 100 + 1);

	return level;
}

int PlayerManagerImplementation::calculatePlayerLevel(CreatureObject* player, String& xpType) {
	if (xpType.isEmpty() || xpType == "jedi_general" || xpType == "mandalorian")
		return calculatePlayerLevel(player);

	String weaponType;
	if (xpType.contains("onehand"))
		weaponType = "onehandmelee";
	else if (xpType.contains("polearm"))
		weaponType = "polearm";
	else if (xpType.contains("twohand"))
		weaponType = "twohandmelee";
	else if (xpType.contains("unarmed"))
		weaponType = "unarmed";
	else if (xpType.contains("carbine"))
		weaponType = "carbine";
	else if (xpType.contains("pistol"))
		weaponType = "pistol";
	else if (xpType.contains("rifle"))
		weaponType = "rifle";
	else
		weaponType = "heavyweapon";

	int level = Math::min(25, player->getSkillMod("private_" + weaponType + "_combat_difficulty") / 100 + 1);

	return level;
}

CraftingStation* PlayerManagerImplementation::getNearbyCraftingStation(CreatureObject* player, int type) {

	ManagedReference<Zone*> zone = player->getZone();

	if (zone == nullptr)
		return nullptr;

	ManagedReference<ZoneServer*> server = zone->getZoneServer();

	if (server == nullptr)
		return nullptr;

	ManagedReference<CraftingStation*> station = nullptr;

	//Locker locker(zone);

	CloseObjectsVector* vec = (CloseObjectsVector*) player->getCloseObjects();
	SortedVector<TreeEntry*> closeObjects(vec->size(), 10);
	vec->safeCopyTo(closeObjects);

	for (int i = 0; i < closeObjects.size(); ++i) {
		SceneObject* scno = static_cast<SceneObject*> (closeObjects.get(i));
		if (scno->isCraftingStation() && (fabs(scno->getPositionZ() - player->getPositionZ()) < 7.0f) && player->isInRange(scno, 7.0f)) {

			station = server->getObject(scno->getObjectID()).castTo<CraftingStation*>();

			if (station == nullptr)
				continue;

			ManagedReference<SceneObject*> parent = station->getParent().get();

			if (parent != nullptr && !parent->isCellObject())
				continue;

			if (type == station->getStationType() || (type
					== CraftingTool::JEDI && station->getStationType()
					== CraftingTool::WEAPON)) {

				return station;
			}
		}
		// dont check Z axis here just check in range call. z axis check for some reason returns a huge number when checking a mob standing on you.
		// in range should be sufficient
		if (scno->isDroidObject() && player->isInRange(scno, 7.0f)) {
			// check the droids around
			DroidObject* droid = cast<DroidObject*>(scno);
			if (droid == nullptr) {
				continue;
			}
			// only the player can benefit from their own droid
			if (droid->getLinkedCreature() != player) {
				continue;
			}
			// check the droid
			station = droid->getCraftingStation(type);

			// As per Mantis 8285 the droid does not need to be charged to use the crafting station - H
			if (station != nullptr/* && droid->hasPower()*/) {
				return station;
			}
		}
	}

	return nullptr;
}

void PlayerManagerImplementation::finishHologrind(CreatureObject* player) {

	ManagedReference<PlayerObject*> ghost = player->getPlayerObject();

	if (ghost->hasSuiBoxWindowType(SuiWindowType::HOLOGRIND_UNLOCK))
		return;

	ManagedReference<SuiMessageBox*> box = new SuiMessageBox(player, SuiWindowType::NONE);
	box->setPromptTitle("@quest/force_sensitive/intro:force_sensitive"); // You feel a tingle in the Force.
	box->setPromptText("Perhaps you should meditate somewhere alone...");

	ghost->addSuiBox(box);
	player->sendMessage(box->generateMessage());

	SkillManager::instance()->awardSkill("force_title_jedi_novice", player, true, false, true);

	ghost->setJediState(1);

}

String PlayerManagerImplementation::banAccount(PlayerObject* admin, Account* account, uint32 seconds, const String& reason) {

	if (admin == nullptr || !admin->isPrivileged())
		return "";

	if (account == nullptr)
		return "Account Not Found";

	String escapedReason = reason;
	Database::escapeString(escapedReason);

#ifndef WITH_SWGREALMS_API
	try {
		StringBuffer query;
		query << "INSERT INTO account_bans values (NULL, " << account->getAccountID() << ", " << admin->getAccountID() << ", now(), " << (uint64)time(0) + seconds << ", '" << escapedReason << "');";

		ServerDatabase::instance()->executeStatement(query);
	} catch(Exception& e) {
		return "Exception banning account: " + e.getMessage();
	}

	Locker locker(account);
	account->setBanReason(reason);
	account->setBanExpires(time(0) + seconds);
	account->setBanAdmin(admin->getAccountID());
#else // WITH_SWGREALMS_API
	// SWGRealms API implementation
	String errorMessage;
	auto swgRealmsAPI = SWGRealmsAPI::instance();
	uint64 expiresTimestamp = (uint64)time(0) + seconds;

	if (swgRealmsAPI != nullptr && swgRealmsAPI->banAccountBlocking(
			account->getAccountID(), admin->getAccountID(), expiresTimestamp, escapedReason, errorMessage)) {
		// API ban succeeded - update local account object
		Locker locker(account);
		account->setBanReason(reason);
		account->setBanExpires(expiresTimestamp);
		account->setBanAdmin(admin->getAccountID());
	} else {
		error() << "SWGRealms API banAccountBlocking failed for accountID " << account->getAccountID()
			<< ": " << errorMessage << " (fail-closed, NOT falling back to MySQL)";
		return "Failed to ban account: " + errorMessage;
	}
#endif // WITH_SWGREALMS_API

	StringBuffer banResult;

	Time expireTime;

	expireTime.addMiliTime((uint64)seconds * 1000);

	banResult << "Account \"" + account->getUsername() + "\" successfully banned until " << expireTime.getFormattedTime() + " server time";

	try {
		Reference<const CharacterList*> characters = account->getCharacterList();

		for (int i = 0; i < characters->size(); ++i) {
			const CharacterListEntry* entry = &characters->get(i);
			if (entry->getGalaxyID() == server->getGalaxyID()) {
				ManagedReference<CreatureObject*> player = getPlayer(entry->getFirstName());

				if (player != nullptr) {
					clearOwnedStructuresPermissions(player);

					if (player->isOnline()) {
						player->sendMessage(new LogoutMessage());

						Reference<ZoneClientSession*> session = player->getClient();

						if (session != nullptr)
							session->disconnect(true);
					}
				}
			}
		}
	} catch(Exception& e) {
		banResult << ", error kicking characters: " + e.getMessage();
	}

	banResult << ".";

	return banResult.toString();
}

String PlayerManagerImplementation::unbanAccount(PlayerObject* admin, Account* account, const String& reason) {

	if (admin == nullptr || !admin->isPrivileged())
		return "";

	if (account == nullptr)
		return "Account Not Found";

	String escapedReason = reason;
	Database::escapeString(escapedReason);

#ifndef WITH_SWGREALMS_API
	try {
		StringBuffer query;
		query << "UPDATE account_bans SET expires = UNIX_TIMESTAMP(), reason = '" << escapedReason << "'  WHERE account_id = " << account->getAccountID() << " and expires > UNIX_TIMESTAMP();";

		ServerDatabase::instance()->executeStatement(query);
	} catch(Exception& e) {
		return "Exception unbanning account: " + e.getMessage();
	}

	Locker locker(account);
	account->setBanExpires(System::getMiliTime());
	account->setBanReason(reason);

	return "Account Successfully Unbanned";
#else // WITH_SWGREALMS_API
	// SWGRealms API implementation
	String errorMessage;
	auto swgRealmsAPI = SWGRealmsAPI::instance();

	if (swgRealmsAPI != nullptr && swgRealmsAPI->unbanAccountBlocking(
			account->getAccountID(), escapedReason, errorMessage)) {
		// API unban succeeded - update local account object
		Locker locker(account);
		account->setBanExpires(System::getMiliTime());
		account->setBanReason(reason);

		return "Account Successfully Unbanned";
	}

	// API failed
	error() << "SWGRealms API unbanAccountBlocking failed for accountID " << account->getAccountID()
		<< ": " << errorMessage << " (fail-closed, NOT falling back to MySQL)";
	return "Failed to unban account: " + errorMessage;
#endif // WITH_SWGREALMS_API
}

#ifndef WITH_SWGREALMS_API
String PlayerManagerImplementation::banFromGalaxy(PlayerObject* admin, Account* account, const uint32 galaxy, uint32 seconds, const String& reason) {

	if (admin == nullptr || !admin->isPrivileged())
		return "";

	if (account == nullptr)
		return "Account Not Found";

	String escapedReason = reason;
	Database::escapeString(escapedReason);

	try {
		StringBuffer query;
		query << "INSERT INTO galaxy_bans values (NULL, " << account->getAccountID() << ", " << admin->getAccountID() << "," << galaxy << ", now()," << (uint64)time(0) + seconds << ", '" << escapedReason << "');";

		ServerDatabase::instance()->executeStatement(query);
	} catch(Exception& e) {
		return "Exception banning from galaxy: " + e.getMessage();
	}

	Locker locker(account);

	Time current;
	Time expires;

	expires.addMiliTime(seconds*10000);

	Reference<GalaxyBanEntry*> ban = new GalaxyBanEntry();

	ban->setAccountID(account->getAccountID());
	ban->setBanAdmin(admin->getAccountID());
	ban->setGalaxyID(galaxy);

	ban->setCreationDate(current);

	ban->setBanExpiration(expires);

	ban->setBanReason(reason);

	account->addGalaxyBan(ban, galaxy);

	try {

		if (server->getGalaxyID() == galaxy) {
			Reference<const CharacterList*> characters = account->getCharacterList();

			for (int i = 0; i < characters->size(); ++i) {
				const CharacterListEntry* entry = &characters->get(i);
				if (entry->getGalaxyID() == galaxy) {

					ManagedReference<CreatureObject*> player = getPlayer(entry->getFirstName());
					if (player != nullptr) {
						clearOwnedStructuresPermissions(player);

						if (player->isOnline()) {
							player->sendMessage(new LogoutMessage());

							ManagedReference<ZoneClientSession*> session = player->getClient();

							if (session != nullptr)
								session->disconnect(true);
						}
					}
				}
			}
		} else {
			return "Successfully Banned from Galaxy, but cannot kick characters because Galaxy is not your current galaxy.";
		}
	} catch(Exception& e) {
		return "Successfully Banned from Galaxy, but error kicking characters. " + e.getMessage();
	}

	return "Successfully Banned from Galaxy";
}
#else // WITH_SWGREALMS_API
String PlayerManagerImplementation::banFromGalaxy(PlayerObject* admin, Account* account, const uint32 galaxy, uint32 seconds, const String& reason) {

	if (admin == nullptr || !admin->isPrivileged())
		return "";

	if (account == nullptr)
		return "Account Not Found";

	String escapedReason = reason;
	Database::escapeString(escapedReason);

	// Use SWGRealms API
	String errorMessage;
	auto swgRealmsAPI = SWGRealmsAPI::instance();
	uint64 expiresTimestamp = (uint64)time(0) + seconds;

	if (swgRealmsAPI != nullptr && swgRealmsAPI->banFromGalaxyBlocking(
			account->getAccountID(), galaxy, admin->getAccountID(), expiresTimestamp, escapedReason, errorMessage)) {

		// API ban succeeded - update local account object
		Locker locker(account);

		Time current;
		Time expires;
		expires.addMiliTime(seconds*10000);

		Reference<GalaxyBanEntry*> ban = new GalaxyBanEntry();
		ban->setAccountID(account->getAccountID());
		ban->setBanAdmin(admin->getAccountID());
		ban->setGalaxyID(galaxy);
		ban->setCreationDate(current);
		ban->setBanExpiration(expires);
		ban->setBanReason(reason);

		account->addGalaxyBan(ban, galaxy);

		// Kick characters if on current galaxy
		try {
			if (server->getGalaxyID() == galaxy) {
				Reference<const CharacterList*> characters = account->getCharacterList();

				for (int i = 0; i < characters->size(); ++i) {
					const CharacterListEntry* entry = &characters->get(i);
					if (entry->getGalaxyID() == galaxy) {
						ManagedReference<CreatureObject*> player = getPlayer(entry->getFirstName());
						if (player != nullptr) {
							clearOwnedStructuresPermissions(player);

							if (player->isOnline()) {
								player->sendMessage(new LogoutMessage());
								ManagedReference<ZoneClientSession*> session = player->getClient();
								if (session != nullptr)
									session->disconnect(true);
							}
						}
					}
				}
			} else {
				return "Successfully Banned from Galaxy, but cannot kick characters because Galaxy is not your current galaxy.";
			}
		} catch(Exception& e) {
			return "Successfully Banned from Galaxy, but error kicking characters. " + e.getMessage();
		}

		return "Successfully Banned from Galaxy";
	}

	// API failed
	error() << "SWGRealms API banFromGalaxyBlocking failed for accountID " << account->getAccountID()
			<< ", galaxy " << galaxy << ": " << errorMessage;
	return "Failed to ban from galaxy: " + errorMessage;
}
#endif // WITH_SWGREALMS_API

#ifndef WITH_SWGREALMS_API
String PlayerManagerImplementation::unbanFromGalaxy(PlayerObject* admin, Account* account, const uint32 galaxy, const String& reason) {

	if (admin == nullptr || !admin->isPrivileged())
		return "";

	if (account == nullptr)
		return "Account Not Found";

	String escapedReason = reason;
	Database::escapeString(escapedReason);

	try {
		StringBuffer query;

		query << "UPDATE galaxy_bans SET expires = UNIX_TIMESTAMP(), reason = '" << escapedReason << "' WHERE account_id = " <<  account->getAccountID() << " and galaxy_id = " << galaxy << " and expires > UNIX_TIMESTAMP();";

		ServerDatabase::instance()->executeStatement(query);
	} catch(Exception& e) {
		return "Exception unbanning from galaxy: " + e.getMessage();
	}

	Locker locker(account);
	account->removeGalaxyBan(galaxy);

	return "Successfully Unbanned from Galaxy";
}
#else // WITH_SWGREALMS_API
String PlayerManagerImplementation::unbanFromGalaxy(PlayerObject* admin, Account* account, const uint32 galaxy, const String& reason) {

	if (admin == nullptr || !admin->isPrivileged())
		return "";

	if (account == nullptr)
		return "Account Not Found";

	String escapedReason = reason;
	Database::escapeString(escapedReason);

	// Use SWGRealms API
	String errorMessage;
	auto swgRealmsAPI = SWGRealmsAPI::instance();

	if (swgRealmsAPI != nullptr && swgRealmsAPI->unbanFromGalaxyBlocking(
			account->getAccountID(), galaxy, escapedReason, errorMessage)) {

		// API unban succeeded - update local account object
		Locker locker(account);
		account->removeGalaxyBan(galaxy);

		return "Successfully Unbanned from Galaxy";
	}

	// API failed
	error() << "SWGRealms API unbanFromGalaxyBlocking failed for accountID " << account->getAccountID()
			<< ", galaxy " << galaxy << ": " << errorMessage;
	return "Failed to unban from galaxy: " + errorMessage;
}
#endif // WITH_SWGREALMS_API

String PlayerManagerImplementation::banCharacter(PlayerObject* admin, Account* account, const String& name, const uint32 galaxyID, uint32 seconds, const String& reason) {

	if (admin == nullptr || !admin->isPrivileged())
		return "";

	if (account == nullptr)
		return "Account Not Found";

#ifndef WITH_SWGREALMS_API
	String escapedReason = reason;
	Database::escapeString(escapedReason);

	String escapedName = name;
	Database::escapeString(escapedName);

	try {
		StringBuffer query;
		query << "INSERT INTO character_bans values (NULL, " << account->getAccountID() << ", " << admin->getAccountID() << ", " << galaxyID << ", '" << escapedName << "', " <<  "now(), UNIX_TIMESTAMP() + " << seconds << ", '" << escapedReason << "');";

		ServerDatabase::instance()->executeStatement(query);
	} catch(Exception& e) {
		return "Exception banning character: " + e.getMessage();
	}
#else // WITH_SWGREALMS_API
	auto swgRealmsAPI = SWGRealmsAPI::instance();
	if (swgRealmsAPI == nullptr) {
		return "SWGRealms API not available";
	}

	uint64 expiresTimestamp = time(nullptr) + seconds;
	String errorMessage;

	if (!swgRealmsAPI->banCharacterBlocking(account->getAccountID(), galaxyID, name, admin->getAccountID(),
	                                         expiresTimestamp, reason, errorMessage)) {
		return "Exception banning character: " + errorMessage;
	}
#endif // WITH_SWGREALMS_API

	Locker locker(account);

	Reference<const CharacterList*> characters = account->getCharacterList();

	for (int i = 0; i<characters->size(); i++) {
		CharacterListEntry& entry = characters->get(i);

		if (entry.getFirstName() == name && entry.getGalaxyID() == galaxyID) {
			Time expires;
			expires.addMiliTime(seconds*1000);

			entry.setBanReason(reason);
			entry.setBanAdmin(admin->getAccountID());
			entry.setBanExpiration(expires);
		}
	}

	locker.release();

	try {
		if (server->getGalaxyID() == galaxyID) {
			ManagedReference<CreatureObject*> player = getPlayer(name);
			if (player != nullptr) {
				clearOwnedStructuresPermissions(player);

				if (player->isOnline()) {
					player->sendMessage(new LogoutMessage());

					ManagedReference<ZoneClientSession*> session = player->getClient();

					if (session != nullptr)
						session->disconnect(true);
				}
			}
		} else {
			return "Character Successfully Banned, but cannot kick character because it's on a different galaxy.";
		}
	} catch(Exception& e) {
		return "Character Successfully Banned, but error kicking Character. " + e.getMessage();
	}

	return "Character Successfully Banned";
}

String PlayerManagerImplementation::unbanCharacter(PlayerObject* admin, Account* account, const String& name, const uint32 galaxyID, const String& reason) {

	if (admin == nullptr || !admin->isPrivileged())
		return "";

	if (account == nullptr)
		return "Account Not Found";

#ifndef WITH_SWGREALMS_API
	String escapedReason = reason;
	Database::escapeString(escapedReason);

	String escapedName = name;
	Database::escapeString(escapedName);

	try {
		StringBuffer query;
		query << "UPDATE character_bans SET expires = UNIX_TIMESTAMP(), reason = '" << escapedReason << "' WHERE account_id = " <<  account->getAccountID() << " AND galaxy_id = " << galaxyID << " and name =  '" << escapedName << "' and expires > UNIX_TIMESTAMP();";

		ServerDatabase::instance()->executeStatement(query);
	} catch(Exception& e) {
		return "Exception banning character: " + e.getMessage();
	}
#else // WITH_SWGREALMS_API
	auto swgRealmsAPI = SWGRealmsAPI::instance();
	if (swgRealmsAPI == nullptr) {
		return "SWGRealms API not available";
	}

	String errorMessage;

	if (!swgRealmsAPI->unbanCharacterBlocking(account->getAccountID(), galaxyID, name, reason, errorMessage)) {
		return "Exception unbanning character: " + errorMessage;
	}
#endif // WITH_SWGREALMS_API

	Locker locker(account);
	CharacterListEntry *entry = account->getCharacterBan(galaxyID, name);

	if (entry != nullptr) {
		Time now;
		entry->setBanExpiration(now);
		entry->setBanReason(reason);
	}

	return "Character Successfully Unbanned";
}

void PlayerManagerImplementation::clearOwnedStructuresPermissions(CreatureObject* player) {
	PlayerObject* ghost = player->getPlayerObject();

	if (ghost == nullptr) {
		return;
	}

	for (int i = 0; i < ghost->getTotalOwnedStructureCount(); i++) {
		uint64 structureID = ghost->getOwnedStructure(i);

		ManagedReference<StructureObject*> structure = server->getObject(structureID).castTo<StructureObject*>();

		if (structure == nullptr) {
			continue;
		}

		structure->revokeAllPermissions();
	}
}

void PlayerManagerImplementation::fixHAM(CreatureObject* player) {
	Locker locker(player);

	try {
		const BuffList* buffs = player->getBuffList();

		VectorMap<byte, int> attributeValues;
		attributeValues.setNullValue(0);
		attributeValues.setAllowOverwriteInsertPlan();

		ManagedReference<Buff*> powerBoost;

		//check buffs
		for (int i = 0; i < buffs->getBuffListSize(); ++i) {
			ManagedReference<Buff*> buff = buffs->getBuffByIndex(i);

			PowerBoostBuff* power = dynamic_cast<PowerBoostBuff*>(buff.get());

			if (power != nullptr) {
				powerBoost = power;
				continue;
			}

			const VectorMap<byte, int>* attributeModifiers = buff->getAttributeModifiers();

			for (int j = 0; j < attributeModifiers->size(); ++j) {
				byte modifier = attributeModifiers->elementAt(j).getKey();
				int val = attributeModifiers->elementAt(j).getValue();

				attributeValues.put(modifier, attributeValues.get(modifier) + val);
			}
		}

		//Infinity:  Let's cancel any weaken debuffs
		if (player->hasBuff(BuffCRC::JEDI_FORCEWEAKEN1))
			player->removeBuff(BuffCRC::JEDI_FORCEWEAKEN1);

		if (player->hasBuff(BuffCRC::JEDI_FORCEWEAKEN2))
			player->removeBuff(BuffCRC::JEDI_FORCEWEAKEN2);

		if (player->hasBuff(BuffCRC::JEDI_FORCEWEAKEN3))
			player->removeBuff(BuffCRC::JEDI_FORCEWEAKEN3);

		if (powerBoost != nullptr) {
			Locker buffLocker(powerBoost);

			player->removeBuff(powerBoost);
		}

		int encumbranceType = -1;

		for (int i = 0; i < 9; ++i) {
			int maxModifier = attributeValues.get((byte)i);
			int baseHam = player->getBaseHAM(i);
			int max = player->getMaxHAM(i);

			int calculated = baseHam + maxModifier;

			if (i % 3 == 0) {
				++encumbranceType;
			} else {
				calculated -= player->getEncumbrance(encumbranceType);
			}

			//info("attribute: " + CreatureAttribute::getName(i, true) + " max = " + String::valueOf(max) + " calculatedMax = " + String::valueOf(calculated), true);

			if (calculated != max && calculated > 1) {
				if (player->getHAM(i) > calculated)
					player->setHAM(i, calculated, false);

				player->setMaxHAM(i, calculated, false);
			}
		}
	} catch (const Exception& e) {
		error(e.getMessage());
	}
}

void PlayerManagerImplementation::fixBuffSkillMods(CreatureObject* player) {
	Locker locker(player);

	try {
		GroupObject* grp = player->getGroup();
		if (grp != nullptr)
			GroupManager::instance()->leaveGroup(grp, player);

		Reference<Buff*> buff = player->getBuff(STRING_HASHCODE("squadleader"));
		if (buff != nullptr) {
			Locker locker(buff);
			player->removeBuff(buff);
		}

		if (player->getSkillModList() == nullptr)
			return;

		Locker smodsGuard(player->getSkillModMutex());

		SkillModGroup* smodGroup = player->getSkillModList()->getSkillModGroup(SkillModManager::BUFF);
		smodGroup->removeAll();

		smodsGuard.release();

		const BuffList* buffs = player->getBuffList();

		for (int i = 0; i < buffs->getBuffListSize(); i++) {
			ManagedReference<Buff*> buff = buffs->getBuffByIndex(i);

			Locker clocker(buff, player);

			buff->setModsApplied(true);

			buff->applySkillModifiers();
		}

		if (grp != nullptr && grp->getLeader() != nullptr) {
			player->updateGroupInviterID(grp->getLeader()->getObjectID());
			GroupManager::instance()->joinGroup(player);
		}
	} catch (const Exception& e) {
		error(e.getMessage());
	}
}

bool PlayerManagerImplementation::promptTeachableSkills(CreatureObject* teacher, SceneObject* target) {
	if (target == nullptr || !target->isPlayerCreature()) {
		teacher->sendSystemMessage("@teaching:no_target"); //Whom do you want to teach?
		return false;
	}

	if (target == teacher) {
		teacher->sendSystemMessage("@teaching:no_teach_self"); //You cannot teach yourself.
		return false;
	}

	Locker _lock(target, teacher);

	//We checked if they had the player object in slot with isPlayerCreature
	CreatureObject* student = cast<CreatureObject*>(target);

	if (teacher->getGroup() == nullptr || student->getGroup() != teacher->getGroup()) {
		StringIdChatParameter params("teaching", "not_in_same_group"); //You must be within the same group as %TT in order to teach.
		params.setTT(student->getDisplayedName());
		teacher->sendSystemMessage(params);
		return false;
	}

	if (student->isDead()) {
		StringIdChatParameter params("teaching", "student_dead"); //%TT does not feel like being taught right now.
		params.setTT(student->getDisplayedName());
		teacher->sendSystemMessage(params);
		return false;
	}

	if (!student->isInRange(teacher, 32.f)) {
		teacher->sendSystemMessage("@teaching:student_too_far"); // Your student must be nearby in order to teach.
		return false;
	}

	ManagedReference<PlayerObject*> studentGhost = student->getPlayerObject();

	//Do they have an outstanding teaching offer?
	if (studentGhost->hasSuiBoxWindowType(SuiWindowType::TEACH_OFFER)) {
		StringIdChatParameter params("teaching", "student_has_offer_to_learn"); //%TT already has an offer to learn.
		params.setTT(student->getDisplayedName());
		teacher->sendSystemMessage(params);
		return false;
	}

	SortedVector<String> skills = getTeachableSkills(teacher, student);

	if (skills.size() <= 0) {
		StringIdChatParameter params("teaching", "no_skills_for_student"); //You have no skills that  %TT can currently learn."
		params.setTT(student->getDisplayedName());
		teacher->sendSystemMessage(params);
		return false;
	}

	ManagedReference<SuiListBox*> listbox = new SuiListBox(teacher, SuiWindowType::TEACH_SKILL);
	listbox->setUsingObject(student);
	listbox->setForceCloseDistance(32.f);
	listbox->setPromptTitle("SELECT SKILL");
	listbox->setPromptText("Select a skill to teach.");
	listbox->setCancelButton(true, "@cancel");

	for (int i = 0; i < skills.size(); ++i) {
		const String& skill = skills.get(i);
		listbox->addMenuItem("@skl_n:" + skill, skill.hashCode());
	}

	listbox->setCallback(new PlayerTeachSuiCallback(server));

	if (teacher->isPlayerCreature()) {
		ManagedReference<PlayerObject*> teacherGhost = teacher->getPlayerObject();

		teacherGhost->addSuiBox(listbox);
	}

	teacher->sendMessage(listbox->generateMessage());

	return true;
}

bool PlayerManagerImplementation::offerTeaching(CreatureObject* teacher, CreatureObject* student, Skill* skill) {
	ManagedReference<PlayerObject*> studentGhost = student->getPlayerObject();

	//Do they have an outstanding teaching offer?
	if (studentGhost->hasSuiBoxWindowType(SuiWindowType::TEACH_OFFER)) {
		StringIdChatParameter params("teaching", "student_has_offer_to_learn"); //%TT already has an offer to learn.
		params.setTT(student->getDisplayedName());
		teacher->sendSystemMessage(params);
		return false;
	}

	ManagedReference<SuiMessageBox*> suibox = new SuiMessageBox(teacher, SuiWindowType::TEACH_OFFER);
	suibox->setUsingObject(teacher);
	suibox->setForceCloseDistance(32.f);
	suibox->setPromptTitle("@sui:swg"); //Star Wars Galaxies

	StringIdManager* sidman = StringIdManager::instance();

	String sklname = sidman->getStringId(String("@skl_n:" + skill->getSkillName()).hashCode()).toString();
	String expname = sidman->getStringId(String("@exp_n:" + skill->getXpType()).hashCode()).toString();

	StringBuffer prompt;
	prompt << teacher->getDisplayedName()
															<< " has offered to teach you " << sklname << " (" << skill->getXpCost()
															<< " " << expname  << " experience cost).";

	suibox->setPromptText(prompt.toString());
	suibox->setCallback(new PlayerTeachConfirmSuiCallback(server, skill));

	suibox->setOkButton(true, "@yes");
	suibox->setCancelButton(true, "@no");

	studentGhost->addSuiBox(suibox);
	student->sendMessage(suibox->generateMessage());

	StringIdChatParameter params("teaching", "offer_given"); //You offer to teach %TT %TO.
	params.setTT(student->getDisplayedName());
	params.setTO("@skl_n:" + skill->getSkillName());
	teacher->sendSystemMessage(params);

	return true;
}

bool PlayerManagerImplementation::acceptTeachingOffer(CreatureObject* teacher, CreatureObject* student, Skill* skill) {
	if (teacher->getGroup() == nullptr || student->getGroup() != teacher->getGroup()) {
		StringIdChatParameter params("teaching", "not_in_same_group"); //You must be within the same group as %TT in order to teach.
		params.setTT(student->getDisplayedName());
		teacher->sendSystemMessage(params);
		return false;
	}

	//Check to see if the teacher still has the skill and the student can still learn the skill.
	SkillManager* skillManager = SkillManager::instance();

	if (!student->isInRange(teacher, 32.f)) {
		StringIdChatParameter params("teaching", "teacher_too_far_target"); //You are too far away from %TT to learn.
		params.setTT(teacher->getDisplayedName());
		student->sendSystemMessage(params);

		params.setStringId("teaching", "student_too_far_target");
		params.setTT(student->getDisplayedName()); //You are too far away from %TT to teach.
		teacher->sendSystemMessage(params);
		return false;
	}

	if (teacher->hasSkill(skill->getSkillName()) && skillManager->awardSkill(skill->getSkillName(), student, true, false, false)) {
		StringIdChatParameter params("teaching", "student_skill_learned"); //You learn %TO from %TT.
		params.setTO("@skl_n:" + skill->getSkillName());
		params.setTT(teacher->getDisplayedName());
		student->sendSystemMessage(params);

		params.setStringId("teaching", "teacher_skill_learned"); //%TT learns %TO from you.
		params.setTT(student->getDisplayedName());
		teacher->sendSystemMessage(params);

		if (skillManager->isApprenticeshipEnabled() && !skill->getSkillName().endsWith("novice")) {
			int exp = 10 + (skill->getTotalChildren() * 10);

			StringIdChatParameter params("teaching", "experience_received"); //You have received %DI Apprenticeship experience.
			params.setDI(exp);
			teacher->sendSystemMessage(params);

			awardExperience(teacher, "apprenticeship", exp, false, 1.0f, false);
		}
	} else {
		student->sendSystemMessage("@teaching:learning_failed"); //Learning failed.
		teacher->sendSystemMessage("@teaching:teaching_failed"); //Teaching failed.
		return false;
	}

	return true;
}

SortedVector<String> PlayerManagerImplementation::getTeachableSkills(CreatureObject* teacher, CreatureObject* student) {
	SortedVector<String> skills;
	skills.setNoDuplicateInsertPlan();

	const SkillList* skillList = teacher->getSkillList();

	SkillManager* skillManager = SkillManager::instance();

	static const Vector<String> restrictedPrefixes =
	{
        "admin",
        "big_game",
        "combat_death_watch",
        "combat_demolition",
        "etyyy_creature_handler",
        "force_rank",
        "force_sensitive",
        "force_title",
        "gladiator",
        "mandalorian",
        "stadium" 
    };

	for (int i = 0; i < skillList->size(); ++i) {
		const Skill* skill = skillList->get(i);

		const auto& skillName = skill->getSkillName();

		bool hasRestrictedPrefix = false;
		for (const auto& prefix : restrictedPrefixes) {
			if (skillName.beginsWith(prefix)) {
				hasRestrictedPrefix = true;
				break;
			}
		}

		if (!hasRestrictedPrefix && skillManager->canLearnSkill(skillName, student, false)) {
			skills.put(skillName);
		}
	}

	return skills;
}

void PlayerManagerImplementation::decreaseOnlineCharCount(ZoneClientSession* client) {
	Locker locker(&onlineMapMutex);

	auto server = ServerCore::getZoneServer();
	uint32 accountId = client->getAccountID();

	if (!onlineZoneClientMap.containsKey(accountId)) {
		info("decreaseOnlineCharCount missing account " + String::valueOf(accountId) + " in onlineZoneClientMap");
		onlineZoneClientMap.accountLoggedOut(client->getIPAddress(), accountId, server != nullptr ? server->getGalaxyID() : 0);
		return;
	}

	Vector<Reference<ZoneClientSession*> > clients = onlineZoneClientMap.get(accountId);

	for (int i = 0; i < clients.size(); ++i)
		if (clients.get(i) == client) {
			clients.remove(i);

			break;
		}

	if (clients.size() == 0) {
		onlineZoneClientMap.remove(accountId);
		onlineZoneClientMap.accountLoggedOut(client->getIPAddress(), accountId, server != nullptr ? server->getGalaxyID() : 0);
	} else
		onlineZoneClientMap.put(accountId, clients);

	locker.release();
}

void PlayerManagerImplementation::proposeUnity( CreatureObject* askingPlayer, CreatureObject* respondingPlayer, SceneObject* askingPlayerRing) {
	if (!askingPlayer->isPlayerCreature()) {
		return;
	}

	// Check if target is self
	if (askingPlayer == respondingPlayer) {
		askingPlayer->sendSystemMessage("@unity:bad_target"); // "You must have a valid player target to Propose Unity."
		return;
	}

	// Check if target is a player
	if (!respondingPlayer->isPlayerCreature()) {
		askingPlayer->sendSystemMessage("@unity:bad_target"); // "You must have a valid player target to Propose Unity."
		return;
	}

	Reference<PlayerObject*> askingGhost = askingPlayer->getPlayerObject();
	Reference<PlayerObject*> respondingGhost = respondingPlayer->getPlayerObject();
	if (askingGhost == nullptr || respondingGhost == nullptr) {
		return;
	}

	// Check if askingPlayer is married
	if (askingGhost->isMarried()) {
		StringIdChatParameter errAskerMarried;
		errAskerMarried.setStringId("unity", "prose_already_married"); // "You cannot propose unity. You are already united with %TO."
		errAskerMarried.setTO( askingGhost->getSpouseName() );
		askingPlayer->sendSystemMessage(errAskerMarried );
		return;
	}

	// Check if respondingPlayer is married
	if (respondingGhost->isMarried()) {
		askingPlayer->sendSystemMessage("@unity:target_married"); // "You cannot propose unity to someone who is already united."
		return;
	}

	// Check distance
	if (!respondingPlayer->isInRange(askingPlayer, 15.0)) {
		askingPlayer->sendSystemMessage("@unity:out_of_range"); // "Your target is too far away to properly propose!"
		return;
	}

	if (!askingPlayer->isFacingObject(respondingPlayer)) {
		askingPlayer->sendSystemMessage("@unity:bad_facing"); // "You must be facing your target to properly propose!"
		return;
	}

	// Check if asking player has a proposal outstanding
	if (askingPlayer->getActiveSession(SessionFacadeType::PROPOSEUNITY) != nullptr) {
		askingPlayer->sendSystemMessage("But you already have an outstanding unity proposal");
		return;
	}

	// Check if responding player has a proposal outstanding
	if (respondingPlayer->getActiveSession(SessionFacadeType::PROPOSEUNITY) != nullptr) {
		askingPlayer->sendSystemMessage("@unity:target_proposed"); // "Your proposal target is already engaged in a unity proposal."
		return;
	}

	//
	// All checks passed
	//

	if (askingPlayer->getPosture() != CreaturePosture::CROUCHED) {
		askingPlayer->setPosture(CreaturePosture::CROUCHED);
	}

	Locker rlocker( askingPlayer, respondingPlayer );

	// Initialize session
	ManagedReference<ProposeUnitySession*> askerUnitySession =
			new ProposeUnitySession( askingPlayer->getObjectID(), respondingPlayer->getObjectID(), askingPlayerRing->getObjectID() );
	askingPlayer->addActiveSession(SessionFacadeType::PROPOSEUNITY, askerUnitySession);

	ManagedReference<ProposeUnitySession*> responderUnitySession =
			new ProposeUnitySession( askingPlayer->getObjectID(), respondingPlayer->getObjectID(), askingPlayerRing->getObjectID() );
	respondingPlayer->addActiveSession(SessionFacadeType::PROPOSEUNITY, responderUnitySession);

	// Submit timeout task
	Reference<Task*> askerExpiredTask = new ProposeUnityExpiredTask( askingPlayer );
	askingPlayer->addPendingTask("propose_unity", askerExpiredTask, 60000); // 1 min

	Reference<Task*> responderExpiredTask = new ProposeUnityExpiredTask( respondingPlayer );
	respondingPlayer->addPendingTask("propose_unity", responderExpiredTask, 60000); // 1 min

	// Build and send proposal window
	ManagedReference<SuiMessageBox*> suiBox = new SuiMessageBox(respondingPlayer, SuiWindowType::PROPOSE_UNITY);
	suiBox->setCallback(new ProposeUnitySuiCallback(server));
	suiBox->setPromptTitle("@unity:accept_title"); // "Accept Unity Proposal?"
	suiBox->setPromptText( askingPlayer->getCreatureName().toString() + " is proposing unity to you. Do you wish to accept?" );
	suiBox->setUsingObject( askingPlayer );
	suiBox->setCancelButton(true, "@no");
	suiBox->setOkButton(true, "@yes");

	respondingGhost->addSuiBox(suiBox);
	respondingPlayer->sendMessage(suiBox->generateMessage());

	// Send message to asking player
	StringIdChatParameter proposalSent;
	proposalSent.setStringId("unity", "prose_propose"); // "You propose unity to %TO."
	proposalSent.setTO( respondingPlayer->getFirstName() );
	askingPlayer->sendSystemMessage(proposalSent );

}

void PlayerManagerImplementation::denyUnity( CreatureObject* respondingPlayer) {

	if (respondingPlayer == nullptr )
		return;

	// Check session
	ManagedReference<ProposeUnitySession*> proposeUnitySession = respondingPlayer->getActiveSession(SessionFacadeType::PROPOSEUNITY).castTo<ProposeUnitySession*>();
	if (proposeUnitySession == nullptr) {
		respondingPlayer->sendSystemMessage("@unity:expire_target"); // "The unity proposal extended to you has expired."
		return;
	}

	// Pull asking player
	uint64 targID = proposeUnitySession->getAskingPlayer();
	ManagedReference<SceneObject*> obj = server->getObject(targID);
	if (obj == nullptr || !obj->isPlayerCreature()) {
		respondingPlayer->sendSystemMessage("@unity:wed_error"); // "An error has occurred during the unity process."
		return;
	}

	CreatureObject* askingPlayer = cast<CreatureObject*>( obj.get());
	Locker alocker( askingPlayer, respondingPlayer );
	askingPlayer->sendSystemMessage("@unity:declined"); // "Your unity proposal has been declined.")
	respondingPlayer->sendSystemMessage("@unity:decline"); // "You decline the unity proposal.")

	// Remove session
	cancelProposeUnitySession(respondingPlayer, askingPlayer);

}

void PlayerManagerImplementation::acceptUnity( CreatureObject* respondingPlayer) {

	if (respondingPlayer == nullptr )
		return;

	// Check session
	ManagedReference<ProposeUnitySession*> proposeUnitySession = respondingPlayer->getActiveSession(SessionFacadeType::PROPOSEUNITY).castTo<ProposeUnitySession*>();
	if (proposeUnitySession == nullptr) {
		respondingPlayer->sendSystemMessage("@unity:expire_target"); // "The unity proposal extended to you has expired."
		return;
	}

	// Pull asking player
	uint64 targID = proposeUnitySession->getAskingPlayer();
	ManagedReference<SceneObject*> obj = server->getObject(targID);
	if (obj == nullptr || !obj->isPlayerCreature()) {
		respondingPlayer->sendSystemMessage("@unity:wed_error"); // "An error has occurred during the unity process."
		return;
	}

	CreatureObject* askingPlayer = cast<CreatureObject*>( obj.get());
	Locker alocker( askingPlayer, respondingPlayer );

	// Check distance
	if (!respondingPlayer->isInRange(askingPlayer, 15.0)) {
		askingPlayer->sendSystemMessage("@unity:wed_oor"); // "You must remain within 15 meters during the unity process for it to complete."
		respondingPlayer->sendSystemMessage("@unity:wed_oor"); // "You must remain within 15 meters during the unity process for it to complete."
		cancelProposeUnitySession(respondingPlayer, askingPlayer);
		return;
	}

	// Check for a ring in player's inventory
	ManagedReference<SceneObject*> inventory = respondingPlayer->getInventory();
	if (inventory == nullptr) {
		respondingPlayer->sendSystemMessage("@unity:wed_error"); // "An error has occurred during the unity process."
		askingPlayer->sendSystemMessage("@unity:wed_error"); // "An error has occurred during the unity process."
		cancelProposeUnitySession(respondingPlayer, askingPlayer);
		return;
	}

	ManagedReference<WearableObject*> wearable = nullptr;
	Vector<WearableObject*> rings;

	// Check proposal target for unequipped applicable rings
	for (int i = 0; i < inventory->getContainerObjectsSize(); i++) {
		wearable = cast<WearableObject*>(inventory->getContainerObject(i).get());

		if (wearable != nullptr && wearable->getGameObjectType() == SceneObjectType::RING && !wearable->isNoTrade()) {
			rings.add(wearable);
		}
	}

	// Check equipped items for applicable rings
	VectorMap<String, ManagedReference<SceneObject* > > slotted;
	respondingPlayer->getSlottedObjects(slotted);

	for (int i = 0; i < slotted.size(); ++i) {
		SceneObject* object = slotted.get(i);

		if (object != nullptr && object->isWearableObject() && object->getGameObjectType() == SceneObjectType::RING && !object->isNoTrade()) {
			wearable = cast<WearableObject*>(object);

			if (wearable != nullptr) {
				rings.add(wearable);
			}
		}
	}

	// No ring found
	if (rings.size() <= 0) {
		askingPlayer->sendSystemMessage("@unity:accept_fail"); // "Your proposal target has no ring to offer in return."
		respondingPlayer->sendSystemMessage("@unity:no_ring"); // "You cannot accept a unity proposal without a ring to offer."
		cancelProposeUnitySession(respondingPlayer, askingPlayer);
		return;
	} else {
		// Build and send list box for ring selection
		ManagedReference<SuiListBox*> box = new SuiListBox(respondingPlayer, SuiWindowType::SELECT_UNITY_RING, SuiListBox::HANDLETWOBUTTON);
		box->setCallback(new SelectUnityRingSuiCallback(server));
		box->setPromptText("@unity:ring_prompt"); // "Select the ring you would like to offer, in return, for your unity."
		box->setPromptTitle("Select Unity Ring");
		box->setOkButton(true, "@ok");
		box->setCancelButton(true, "@cancel");

		for (int i = 0; i < rings.size(); i++) {
			WearableObject* ring = rings.get(i);

			if (ring != nullptr) {
				String itemName = ring->getDisplayedName();
				box->addMenuItem(itemName, ring->getObjectID());
			}
		}

		box->setUsingObject(respondingPlayer);
		respondingPlayer->getPlayerObject()->addSuiBox(box);
		respondingPlayer->sendMessage(box->generateMessage());
	}
}

void PlayerManagerImplementation::completeUnity( CreatureObject* respondingPlayer, unsigned long long respondingPlayerRingId) {
	if (respondingPlayer == nullptr )
		return;

	// Check session
	ManagedReference<ProposeUnitySession*> proposeUnitySession = respondingPlayer->getActiveSession(SessionFacadeType::PROPOSEUNITY).castTo<ProposeUnitySession*>();
	if (proposeUnitySession == nullptr) {
		respondingPlayer->sendSystemMessage("@unity:expire_target"); // "The unity proposal extended to you has expired."
		return;
	}

	// Pull asking player
	uint64 targID = proposeUnitySession->getAskingPlayer();
	ManagedReference<SceneObject*> obj = server->getObject(targID);
	if (obj == nullptr || !obj->isPlayerCreature()) {
		respondingPlayer->sendSystemMessage("@unity:wed_error"); // "An error has occurred during the unity process."
		return;
	}

	CreatureObject* askingPlayer = cast<CreatureObject*>( obj.get());
	Locker alocker( askingPlayer, respondingPlayer );

	// Check distance
	if (!respondingPlayer->isInRange(askingPlayer, 15.0)) {
		askingPlayer->sendSystemMessage("@unity:wed_oor"); // "You must remain within 15 meters during the unity process for it to complete."
		respondingPlayer->sendSystemMessage("@unity:wed_oor"); // "You must remain within 15 meters during the unity process for it to complete."
		cancelProposeUnitySession(respondingPlayer, askingPlayer);
		return;
	}

	// Check if Asking and Responding players Inventories are null
	ManagedReference<SceneObject*> respondingPlayerInventory = respondingPlayer->getInventory();
	ManagedReference<SceneObject*> askingPlayerInventory = askingPlayer->getInventory();

	if (respondingPlayerInventory == nullptr || askingPlayerInventory == nullptr) {
		respondingPlayer->sendSystemMessage("@unity:wed_error"); // "An error has occurred during the unity process."
		askingPlayer->sendSystemMessage("@unity:wed_error"); // "An error has occurred during the unity process."
		cancelProposeUnitySession(respondingPlayer, askingPlayer);
		return;
	}

	ManagedReference<WearableObject*> wearable = nullptr;
	ManagedReference<WearableObject*> responderRing = nullptr;

	// Find responder's ring -- unequipped items
	for (int i = 0; i < respondingPlayerInventory->getContainerObjectsSize(); i++) {
		wearable = cast<WearableObject*>(respondingPlayerInventory->getContainerObject(i).get());
		if (wearable != nullptr && wearable->getObjectID() == respondingPlayerRingId) {
			responderRing = wearable;
			break;
		}
	}

	// If responderRing is null, check equipped items
	if (responderRing == nullptr) {
		VectorMap<String, ManagedReference<SceneObject* > > slotted;
		respondingPlayer->getSlottedObjects(slotted);

		for (int i = 0; i < slotted.size(); ++i) {
			SceneObject* responderObject = slotted.get(i);

			if (responderObject != nullptr && responderObject->isWearableObject() && responderObject->getGameObjectType() == SceneObjectType::RING) {
				wearable = cast<WearableObject*>(responderObject);

				if (wearable != nullptr && wearable->getObjectID() == respondingPlayerRingId) {
					responderRing = wearable;
					break;
				}
			}
		}
	}

	// Find asker's ring -- unequipped items
	wearable = nullptr;
	ManagedReference<WearableObject*> askerRing = nullptr;
	for (int i = 0; i < askingPlayerInventory->getContainerObjectsSize(); i++) {
		wearable = cast<WearableObject*>(askingPlayerInventory->getContainerObject(i).get());
		if (wearable != nullptr && wearable->getObjectID() == proposeUnitySession->getAskingPlayerRing()) {
			askerRing = wearable;
			break;
		}
	}

	// If askerRing is null, check equipped items
	if (askerRing == nullptr) {
		VectorMap<String, ManagedReference<SceneObject* > > slotted;
		askingPlayer->getSlottedObjects(slotted);

		for (int i = 0; i < slotted.size(); ++i) {
			SceneObject* askingObject = slotted.get(i);

			if (askingObject != nullptr && askingObject->isWearableObject() && askingObject->getGameObjectType() == SceneObjectType::RING) {
				wearable = cast<WearableObject*>(askingObject);

				if (wearable != nullptr && wearable->getObjectID() == proposeUnitySession->getAskingPlayerRing()) {
					askerRing = wearable;
					break;
				}
			}
		}
	}


	// Rings not found
	if (responderRing == nullptr) {
		askingPlayer->sendSystemMessage("@unity:accept_fail"); // "Your proposal target has no ring to offer in return."
		respondingPlayer->sendSystemMessage("@unity:no_ring"); // "You cannot accept a unity proposal without a ring to offer."
		cancelProposeUnitySession(respondingPlayer, askingPlayer);
		return;
	} else if (askerRing == nullptr) {
		respondingPlayer->sendSystemMessage("@unity:accept_fail"); // "Your proposal target has no ring to offer in return."
		askingPlayer->sendSystemMessage("@unity:no_ring"); // "You cannot accept a unity proposal without a ring to offer."
		cancelProposeUnitySession(respondingPlayer, askingPlayer);
		return;
	}

	// Exchange rings, set Insured and set as Union Ring
	ManagedReference<ObjectController*> objectController = server->getObjectController();
	if (objectController->transferObject(askerRing, respondingPlayerInventory, -1, true, true)) { // Allow overflow
		Locker aringlock(askerRing);
		askerRing->sendDestroyTo(askingPlayer);

		respondingPlayerInventory->broadcastObject(askerRing, true);

		askerRing->setOptionsBitmask(OptionBitmask::INSURED);
		askerRing->setUnionRing(true);
	}

	if (objectController->transferObject(responderRing, askingPlayerInventory, -1, true, true)) { // Allow overflow
		Locker rringlock(responderRing);
		responderRing->sendDestroyTo(respondingPlayer);

		askingPlayerInventory->broadcastObject(responderRing, true);

		responderRing->setOptionsBitmask(OptionBitmask::INSURED);
		responderRing->setUnionRing(true);
	}

	// Set married
	String respondingPlayerName = respondingPlayer->getFirstName();
	String askingPlayerName = askingPlayer->getFirstName();
	askingPlayer->getPlayerObject()->setSpouseName( respondingPlayerName );
	respondingPlayer->getPlayerObject()->setSpouseName( askingPlayerName );

	// Send obligatory congratulations
	StringIdChatParameter congratsAsker;
	congratsAsker.setStringId("unity", "prose_wed_complete"); //  "Your union with %TT is complete."
	congratsAsker.setTT( respondingPlayer->getFirstName() );
	askingPlayer->sendSystemMessage(congratsAsker );

	StringIdChatParameter congratsResponder;
	congratsResponder.setStringId("unity", "prose_wed_complete"); //  "Your union with %TT is complete."
	congratsResponder.setTT( askingPlayer->getFirstName() );
	respondingPlayer->sendSystemMessage(congratsResponder );

	// Remove session
	cancelProposeUnitySession(respondingPlayer, askingPlayer);

}

void PlayerManagerImplementation::cancelProposeUnitySession(CreatureObject* respondingPlayer, CreatureObject* askingPlayer) {
	askingPlayer->dropActiveSession(SessionFacadeType::PROPOSEUNITY);
	respondingPlayer->dropActiveSession(SessionFacadeType::PROPOSEUNITY);
	askingPlayer->removePendingTask( "propose_unity" );
	respondingPlayer->removePendingTask( "propose_unity" );
}

void PlayerManagerImplementation::promptDivorce(CreatureObject* player) {
	if (player == nullptr || !player->isPlayerCreature())
		return;

	// Check if player is married
	PlayerObject* playerGhost = player->getPlayerObject();

	if (playerGhost == nullptr)
		return;

	if (!playerGhost->isMarried()) {
		player->sendSystemMessage("You are not united with anyone!");
		return;
	}

	// Build and confirmation window
	ManagedReference<SuiMessageBox*> suiBox = new SuiMessageBox(player, SuiWindowType::CONFIRM_DIVORCE);
	suiBox->setCallback(new ConfirmDivorceSuiCallback(server));
	suiBox->setPromptTitle("Confirm Divorce?");
	suiBox->setPromptText("Do you wish to nullify your unity with " + playerGhost->getSpouseName() + "?");
	suiBox->setCancelButton(true, "@no");
	suiBox->setOkButton(true, "@yes");

	playerGhost->addSuiBox(suiBox);
	player->sendMessage(suiBox->generateMessage());
}

void PlayerManagerImplementation::grantDivorce(CreatureObject* player) {
	if (player == nullptr || !player->isPlayerCreature())
		return;

	// Check if player is married
	PlayerObject* playerGhost = player->getPlayerObject();

	if (playerGhost == nullptr || !playerGhost->isMarried())
		return;

	// Get & check players inventory
	ManagedReference<SceneObject*> playerInventory = player->getInventory();

	if (playerInventory == nullptr) {
		return;
	}

	// Check for unionRing and set false
	ManagedReference<WearableObject*> wearable = nullptr;
	ManagedReference<WearableObject*> ring = nullptr;

	// Unequipped items
	for (int i = 0; i < playerInventory->getContainerObjectsSize(); i++) {
		wearable = cast<WearableObject*>(playerInventory->getContainerObject(i).get());
		if (wearable != nullptr && wearable->isUnionRing()) {
			ring = wearable;
			break;
		}
	}

	// If ring is null, check equipped items
	if (ring == nullptr) {
		VectorMap<String, ManagedReference<SceneObject* > > slotted;
		player->getSlottedObjects(slotted);

		for (int i = 0; i < slotted.size(); ++i) {
			SceneObject* askingObject = slotted.get(i);

			if (askingObject != nullptr && askingObject->isWearableObject() && askingObject->getGameObjectType() == SceneObjectType::RING) {
				wearable = cast<WearableObject*>(askingObject);

				if (wearable != nullptr && wearable->isUnionRing()) {
					ring = wearable;
					break;
				}
			}
		}
	}

	if (ring != nullptr) {
		Locker lock(ring);
		ring->setUnionRing(false);
	}

	// Find spouse
	CreatureObject* spouse = getPlayer(playerGhost->getSpouseName());

	StringIdChatParameter msg;
	msg.setStringId("unity", "prose_end_unity"); // "Your union with %TO has ended."

	// Remove spouse name from both players
	if (spouse != nullptr && spouse->isPlayerCreature()) {
		Locker slocker(spouse, player);

		PlayerObject* spouseGhost = spouse->getPlayerObject();

		if (spouseGhost != nullptr)
			spouseGhost->removeSpouse();

		playerGhost->removeSpouse();

		msg.setTO(player->getFirstName());
		spouse->sendSystemMessage(msg);

		msg.setTO(spouse->getFirstName());
		player->sendSystemMessage(msg);

		// Get spouses inventory
		ManagedReference<SceneObject*> spouseInventory = spouse->getInventory();

		if (spouseInventory != nullptr) {
			ring = nullptr;
			wearable = nullptr;

			// Check for Union Ring
			for (int i = 0; i < spouse->getContainerObjectsSize(); i++) {
				wearable = cast<WearableObject*>(spouseInventory->getContainerObject(i).get());
				if (wearable != nullptr && wearable->isUnionRing()) {
					ring = wearable;
					break;
				}
			}

			// If ring is null, check equipped items
			if (ring == nullptr) {
				VectorMap<String, ManagedReference<SceneObject* > > slotted;
				spouse->getSlottedObjects(slotted);

				for (int i = 0; i < slotted.size(); ++i) {
					SceneObject* askingObject = slotted.get(i);

					if (askingObject != nullptr && askingObject->isWearableObject() && askingObject->getGameObjectType() == SceneObjectType::RING) {
						wearable = cast<WearableObject*>(askingObject);

						if (wearable != nullptr && wearable->isUnionRing()) {
							ring = wearable;
							break;
						}
					}
				}
			}

			if (ring != nullptr) {
				Locker srlock(ring);
				ring->setUnionRing(false);
			}
		}

	} else {
		// If spouse player is null (perhaps it's been deleted), we can still remove the spouse from the current player
		msg.setTO(playerGhost->getSpouseName());
		player->sendSystemMessage(msg);

		playerGhost->removeSpouse();
	}
}

void PlayerManagerImplementation::claimVeteranRewards(CreatureObject* player) {
	if (player == nullptr || !player->isPlayerCreature() )
		return;

	PlayerObject* ghost = player->getPlayerObject();

	// Get account
	ManagedReference<Account*> account = ghost->getAccount();

	if (account == nullptr )
		return;

	// Send message with current account age
	StringIdChatParameter timeActiveMsg;
	timeActiveMsg.setStringId("veteran", "self_time_active"); // You have %DI days logged for veteran rewards.
	timeActiveMsg.setDI( account->getAgeInDays() );
	player->sendSystemMessage(timeActiveMsg );

	// Verify player is eligible for a reward
	int milestone = getEligibleMilestone(ghost, account);

	if (milestone < 0) {
		player->sendSystemMessage("@veteran:not_eligible"); // You are not currently eligible for a veteran reward.
		return;
	}

	// Verify player is not already choosing a reward
	if (player->getActiveSession(SessionFacadeType::VETERANREWARD) != nullptr) {
		player->sendSystemMessage("You are already attempting to claim a veteran reward." );
		return;
	}

	// Create session
	ManagedReference<VeteranRewardSession*> rewardSession =	new VeteranRewardSession( milestone );
	player->addActiveSession(SessionFacadeType::VETERANREWARD, rewardSession);

	// Build and SUI list box of rewards
	ManagedReference<SuiListBox*> box = new SuiListBox(player, SuiWindowType::SELECT_VETERAN_REWARD, SuiListBox::HANDLETWOBUTTON);

	box->setCallback(new SelectVeteranRewardSuiCallback(server));
	box->setPromptText("@veteran_new:choice_description" ); // You may choose one of the items listed below. This item will be placed in your inventory.
	box->setPromptTitle("@veteran_new:item_grant_box_title"); // Reward
	box->setOkButton(true, "@ok");
	box->setCancelButton(true, "@cancel");

	for (int i = 0; i < veteranRewards.size(); i++) {
		// Any rewards at or below current milestone are eligible
		VeteranReward reward = veteranRewards.get(i);

		//info(true) << "Reward: " << reward.getTemplateFile() << " Player Milestone: " << milestone << " Reward Milestone: " << reward.getMilestone();

		if (reward.getMilestone() > milestone)
			continue;

		// Filter out one-time rewards already claimed
		if (reward.isOneTime() && ghost->hasChosenVeteranReward(reward.getTemplateFile())) {
			continue;
		}

		SharedObjectTemplate* rewardTemplate = TemplateManager::instance()->getTemplate(reward.getTemplateFile().hashCode());

		if (rewardTemplate != nullptr) {
			if (reward.getDescription().isEmpty()) {
				box->addMenuItem(rewardTemplate->getDetailedDescription(), i);
			} else {
				box->addMenuItem(reward.getDescription(), i);
			}
		}
	}

	box->setUsingObject(nullptr);
	ghost->addSuiBox(box);
	player->sendMessage(box->generateMessage());
}

void PlayerManagerImplementation::cancelVeteranRewardSession(CreatureObject* player) {
	player->dropActiveSession(SessionFacadeType::VETERANREWARD);
}

void PlayerManagerImplementation::confirmVeteranReward(CreatureObject* player, int itemIndex) {
	if (player == nullptr || !player->isPlayerCreature()) {
		return;
	}

	if (itemIndex < 0 || itemIndex >= veteranRewards.size()) {
		player->sendSystemMessage("@veteran:reward_error"); //	The reward could not be granted.
		cancelVeteranRewardSession(player);
		return;
	}

	// Get account
	PlayerObject* ghost = player->getPlayerObject();
	ManagedReference<Account*> account = ghost->getAccount();
	if (account == nullptr) {
		player->sendSystemMessage("@veteran:reward_error"); //	The reward could not be granted.
		cancelVeteranRewardSession(player);
		return;
	}

	// Check session
	ManagedReference<VeteranRewardSession*> rewardSession = player->getActiveSession(SessionFacadeType::VETERANREWARD).castTo<VeteranRewardSession*>();

	if (rewardSession == nullptr) {
		player->sendSystemMessage("@veteran:reward_error"); //	The reward could not be granted.
		return;
	}

	VeteranReward reward = veteranRewards.get(itemIndex);

	if (reward.isOneTime() && ghost->hasChosenVeteranReward(reward.getTemplateFile())) {
		player->sendSystemMessage("@veteran:reward_error"); //	The reward could not be granted.
		cancelVeteranRewardSession(player);
		return;
	}

	rewardSession->setSelectedRewardIndex(itemIndex);

	// Generate confirmation dialog if item is one-time.  Otherwise, just generate it.
	if (reward.isOneTime()) {
		ManagedReference<SuiMessageBox*> suibox = new SuiMessageBox(player, SuiWindowType::CONFIRM_VETERAN_REWARD);
		suibox->setPromptTitle("@veteran_new:unique_are_you_sure_box_title"); // Reward
		suibox->setPromptText( "@veteran_new:item_unique_are_you_sure"); // The item you are selecting can only be selected as a reward item once for the the lifetime of your account. Are you sure you wish to continue selecting this item?
		suibox->setCallback(new ConfirmVeteranRewardSuiCallback(server));
		suibox->setOkButton(true, "@yes");
		suibox->setCancelButton(true, "@no");

		ghost->addSuiBox(suibox);
		player->sendMessage(suibox->generateMessage());
	} else {
		generateVeteranReward(player);
	}
}

void PlayerManagerImplementation::generateVeteranReward(CreatureObject* player) {
	if (player == nullptr || !player->isPlayerCreature()) {
		return;
	}

	// Get account
	auto ghost = player->getPlayerObject();

	ManagedReference<Account*> account = ghost->getAccount();

	if (account == nullptr) {
		player->sendSystemMessage("@veteran:reward_error"); //	The reward could not be granted.
		cancelVeteranRewardSession(player);
		return;
	}

	// Check session
	ManagedReference<VeteranRewardSession*> rewardSession = player->getActiveSession(SessionFacadeType::VETERANREWARD).castTo<VeteranRewardSession*>();

	if (rewardSession == nullptr) {
		player->sendSystemMessage("@veteran:reward_error"); //	The reward could not be granted.
		return;
	}

	// Final check to see if milestone has already been claimed on any of the player's characters
	// (prevent claiming while multi-logged)

	bool milestoneClaimed = false;

	if (!ghost->getChosenVeteranReward(rewardSession->getMilestone()).isEmpty()) {
		milestoneClaimed = true;
	}

	if (milestoneClaimed) {
		player->sendSystemMessage("@veteran:reward_error"); //	The reward could not be granted.
		cancelVeteranRewardSession(player);
		return;
	}

	// Generate item
	SceneObject* inventory = player->getInventory();

	if (inventory == nullptr) {
		player->sendSystemMessage("@veteran:reward_error"); //	The reward could not be granted.
		cancelVeteranRewardSession(player);
		return;
	}

	VeteranReward reward = veteranRewards.get(rewardSession->getSelectedRewardIndex());
	Reference<SceneObject*> rewardSceno = server->createObject(reward.getTemplateFile().hashCode(), 1);

	if (rewardSceno == nullptr) {
		player->sendSystemMessage("@veteran:reward_error"); //	The reward could not be granted.
		cancelVeteranRewardSession(player);
		return;
	}

	{
		TransactionLog trx(TrxCode::VETERANREWARD, player, rewardSceno);

		// Transfer to player
		if (!inventory->transferObject(rewardSceno, -1, false, true)) { // Allow overflow
			trx.abort() << "Failed to transfer to player inventory";
			player->sendSystemMessage("@veteran:reward_error"); //	The reward could not be granted.
			rewardSceno->destroyObjectFromDatabase(true);
			cancelVeteranRewardSession(player);
			return;
		}
	}

	inventory->broadcastObject(rewardSceno, true);
	player->sendSystemMessage("@veteran:reward_given");  // Your reward has been placed in your inventory.

	// Record reward in all characters registered to the account
	GalaxyAccountInfo* accountInfo = account->getGalaxyAccountInfo(player->getZoneServer()->getGalaxyName());

	// sorosuub_space_yacht_deed
	if (reward.isJtlReward()) {
		accountInfo->addChosenVeteranReward(1, reward.getTemplateFile());
	} else {
		accountInfo->addChosenVeteranReward(rewardSession->getMilestone(), reward.getTemplateFile());
	}

	cancelVeteranRewardSession(player);

	// If player is eligible for another reward, kick off selection
	if (getEligibleMilestone(ghost, account ) >= 0) {
		player->enqueueCommand(STRING_HASHCODE("claimveteranreward"), 0, 0, "");
	}
}

int PlayerManagerImplementation::getEligibleMilestone(PlayerObject *ghost, Account* account) {
	if (account == nullptr || ghost == nullptr )
		return -1;

	int accountAge = account->getAgeInDays();
	int milestone = -1;

	// Return -1 if account age is less than the first milestone
	if (accountAge < veteranRewardMilestones.get(0)) {
		return -1;
	}

	// Return the first milestone for which the player is eligible and has not already claimed
	for (int i = 0; i < veteranRewardMilestones.size(); i++) {
		milestone = veteranRewardMilestones.get(i);

		if (accountAge >= milestone && ghost->getChosenVeteranReward(milestone).isEmpty()) {
			return milestone;
		}
	}

	// They've claimed all of the established milestones, see if they're eligible for an additional one
	milestone += veteranRewardAdditionalMilestones;

	while (accountAge >= milestone) {
		if (ghost->getChosenVeteranReward(milestone).isEmpty()) {
			return milestone;
		}

		milestone += veteranRewardAdditionalMilestones;
	}

	// Not eligible for any milestones
	return -1;
}

int PlayerManagerImplementation::getFirstIneligibleMilestone(PlayerObject *playerGhost, Account* account) {
	if (account == nullptr || playerGhost == nullptr )
		return -1;

	int accountAge = account->getAgeInDays();
	int milestone = -1;

	// Return the first milestone the player has not already claimed
	for (int i = 0; i < veteranRewardMilestones.size(); i++) {
		milestone = veteranRewardMilestones.get(i);
		if (accountAge < milestone) {
			return milestone;
		}
	}

	// Check additional milestones if all established ones have been claimed
	while (accountAge >= milestone) {
		milestone += veteranRewardAdditionalMilestones;
	}

	return milestone;
}

bool PlayerManagerImplementation::increaseOnlineCharCountIfPossible(ZoneClientSession* client) {
	Locker locker(&onlineMapMutex);

	auto server = ServerCore::getZoneServer();
	uint32 accountId = client->getAccountID();

	if (!onlineZoneClientMap.containsKey(accountId)) {
		Vector<Reference<ZoneClientSession*> > clients;
		clients.add(client);

		onlineZoneClientMap.put(accountId, clients);

		locker.release();

		onlineZoneClientMap.accountLoggedIn(client->getIPAddress(), accountId, server != nullptr ? server->getGalaxyID() : 0);

		return true;
	}

	Vector<Reference<ZoneClientSession*> > clients = onlineZoneClientMap.get(accountId);

	int onlineCount = 0;

	for (int i = 0; i < clients.size(); ++i) {
		ZoneClientSession* session = clients.get(i);

		if (session == nullptr)
			continue;

		ManagedReference<CreatureObject*> player = session->getPlayer();

		if (player != nullptr) {
			Reference<PlayerObject*> ghost = player->getPlayerObject();

			if (ghost != nullptr && ghost->getAdminLevel() > 0)
				continue;
			else if (player->getClient() == session)
				++onlineCount;
		}
	}

	if (onlineCount >= onlineCharactersPerAccount)
		return false;

	clients.add(client);

	onlineZoneClientMap.put(accountId, clients);

	locker.release();

	return true;
}

/*
int PlayerManagerImplementation::getOnlineCharCount(unsigned int accountId) {
	//onlineMapMutex.rlock()
	return 0;
}
 */

void PlayerManagerImplementation::disconnectAllPlayers() {
	Locker locker(&onlineMapMutex);

	info(true) << "Disconnecting " << onlineZoneClientMap.size() << " players.";

	Time now, last_rpt;
	Timer profile;
	int countDisconnected = 0;

	profile.start();
	HashTableIterator<uint32, Vector<Reference<ZoneClientSession*> > > iter = onlineZoneClientMap.iterator();

	while (iter.hasNext()) {
		Vector<Reference<ZoneClientSession*> > clients = iter.next();

		for (int i = 0; i < clients.size(); i++) {
			ZoneClientSession* session = clients.get(i);

			if (session != nullptr) {
				CreatureObject* player = session->getPlayer();

				if (player != nullptr) {
					PlayerObject* ghost = player->getPlayerObject();

					if (ghost != nullptr) {
						Locker plocker(player);
						ghost->setLinkDead(true);
						ghost->disconnect(true, true);
						++countDisconnected;
					}
				}
			}

			now.updateToCurrentTime();
			int delta = now.getTime() - last_rpt.getTime();

			if (delta > 5) {
				last_rpt.updateToCurrentTime();
				auto elapsedMs = profile.elapsedToNow() / 1000000;
				auto ps = countDisconnected / (elapsedMs / 1000.0f);
				info(true) << "Disconnected " << commas << countDisconnected << " players (" << ps << "/s)";
			}
		}
	}

	auto elapsedMs = Math::max((uint64)1, profile.stopMs());
	auto ps = countDisconnected / (elapsedMs / 1000.0f);
	info(true) << "Finished disconnecting " << commas << countDisconnected << " players (" << ps << "/s)";
}

bool PlayerManagerImplementation::shouldRescheduleCorpseDestruction(CreatureObject* player, CreatureObject* ai) {
	if (player == nullptr || ai == nullptr)
		return true;

	if (!player->isPlayerCreature()) {
		return true;
	}

	Locker locker(player, ai);

	if (ai->isNonPlayerCreatureObject()) {
		NonPlayerCreatureObject *npc = dynamic_cast<NonPlayerCreatureObject*>(ai);

		if (!npc->hasLoot() && npc->getCashCredits() < 1 && npc->getBankCredits() < 1) {
			return true;
		}
	} else if (ai->isCreature()) {
		Creature * creature = dynamic_cast<Creature*>(ai);

		if (creature->hasLoot() || creature->getCashCredits() > 0 || creature->getBankCredits() > 0)
			return false;

		return !(creature->hasSkillToHarvestMe(player) || canGroupMemberHarvestCorpse(player, creature));

	}

	return false;
}

bool PlayerManagerImplementation::canGroupMemberHarvestCorpse(CreatureObject* player, Creature* creature) {

	if (!player->isGrouped())
		return false;

	ManagedReference<GroupObject*> group = player->getGroup();
	int groupSize = group->getGroupSize();

	for (int i = 0; i < groupSize; i++) {
		ManagedReference<CreatureObject*> groupMember = group->getGroupMember(i);

		if (player->getObjectID() == groupMember->getObjectID())
			continue;

		if (creature->isInRange(groupMember, 128.0f) && creature->hasSkillToHarvestMe(groupMember)) {
			return true;
		}
	}

	return false;
}

void PlayerManagerImplementation::rescheduleCorpseDestruction(CreatureObject* player, CreatureObject* ai) {

	//If the looting player or no group members in the area can harvest then despawn immediately
	if (shouldRescheduleCorpseDestruction(player, ai)) {
		Reference<DespawnCreatureTask*> despawn = ai->getPendingTask("despawn").castTo<DespawnCreatureTask*>();
		if (despawn != nullptr) {
			despawn->cancel();
			despawn->reschedule(1000);
		}

	}
}


void PlayerManagerImplementation::getCleanupCharacterCount() {
	info("**** GETTING CHARACTER CLEANUP INFORMATION ***",true);

	ObjectDatabase* thisDatabase = ObjectDatabaseManager::instance()->loadObjectDatabase("sceneobjects", true, 0xFFFF, false);

	if (thisDatabase == nullptr)
		return;

	ObjectInputStream objectData(2000);
	ObjectDatabaseIterator iterator(thisDatabase);

	uint64 objectID;

	String className;
	uint64 deletedCount = 0;
	uint64 playerCount = 0;

	ZoneServer* server = ServerCore::getZoneServer();

	if (server == nullptr) {
		error("nullptr ZoneServer in character cleanup");
		return;
	}

	int galaxyID = server->getGalaxyID();

	while(iterator.getNextKeyAndValue(objectID, &objectData)) {
		if (Serializable::getVariable<String>(STRING_HASHCODE("_className"), &className, &objectData)) {
			if (className == "CreatureObject") {
				playerCount++;

				if (shouldDeleteCharacter(objectID, galaxyID)) {
					deletedCount++;
					info("DELETE CHARACTER " + String::valueOf(objectID),true);
				}

			}
		}
		objectData.reset();
	}

	StringBuffer deletedMessage;
	deletedMessage << "TOTAL CHARACTERS " << " TO BE DELETED " << String::valueOf(deletedCount);
	info("TOTAL CHARACTERS IN OBJECT DB: " + String::valueOf(playerCount),true);
	info(deletedMessage.toString(),true);
}

void PlayerManagerImplementation::cleanupCharacters() {

	info("**** PERFORMING CHARACTER CLEANUP ***",true);

	ObjectDatabase* thisDatabase = ObjectDatabaseManager::instance()->loadObjectDatabase("sceneobjects", true, 0xFFFF, false);

	if (thisDatabase == nullptr)
		return;

	ObjectInputStream objectData(2000);
	ObjectDatabaseIterator iterator(thisDatabase);

	uint64 objectID;

	String className;
	uint64 deletedCount = 0;
	uint64 playerCount = 0;

	ZoneServer* server = ServerCore::getZoneServer();

	if (server == nullptr) {
		error("nullptr ZoneServer in character cleanup");
		return;
	}

	int galaxyID = server->getGalaxyID();

	while(iterator.getNextKeyAndValue(objectID, &objectData) && deletedCount < 400) {
		if (Serializable::getVariable<String>(STRING_HASHCODE("_className"), &className, &objectData)) {
			if (className == "CreatureObject") {
				playerCount++;

				if (shouldDeleteCharacter(objectID, galaxyID)) {

					ManagedReference<CreatureObject*> object = Core::getObjectBroker()->lookUp(objectID).castTo<CreatureObject*>();

					if (object == nullptr) {
						info("OBJECT nullptr when getting object " + String::valueOf(objectID),true);
					}else if (object->isPlayerCreature()) {

						deletedCount++;
						info("DELETING CHARACTER: " + String::valueOf(objectID)+ " NAME: " +  object->getFirstName() + " " + object->getLastName() ,true);
						Locker _lock(object);

						ManagedReference<ZoneClientSession*> client = object->getClient();

						if (client != nullptr)
							client->disconnect();

						object->destroyObjectFromWorld(false); //Don't need to send destroy to the player - they are being disconnected.
						object->destroyPlayerCreatureFromDatabase(true);

					}

				}

			}

		}

		objectData.reset();
	}

	StringBuffer deletedMessage;
	deletedMessage << "TOTAL CHARACTERS";
	deletedMessage << " DELETED FROM OBJECTDB: " << String::valueOf(deletedCount);
	info(deletedMessage.toString(),true);

}

bool PlayerManagerImplementation::shouldDeleteCharacter(uint64 characterID, int galaxyID) {
#ifndef WITH_SWGREALMS_API
	const String query = "SELECT * FROM characters WHERE character_oid = " + String::valueOf(characterID) + " AND galaxy_id = " + String::valueOf(galaxyID);

	try {
		UniqueReference<ResultSet*> result(ServerDatabase::instance()->executeQuery(query));

		if (result == nullptr) {
			error("ERROR WHILE LOOKING UP CHARACTER IN SQL TABLE");
		} else if (result->getRowsAffected() > 1) {

			error("More than one character with oid = " + String::valueOf(characterID) + " in galaxy " + String::valueOf(galaxyID));
			return false;

		} else if (result->getRowsAffected() == 0) {
			return true;
		}

		return false;
	} catch (const DatabaseException& err) {
		error() << "database error " << err.getMessage();
		return false;
	}
#else // WITH_SWGREALMS_API
	auto swgRealmsAPI = SWGRealmsAPI::instance();
	if (swgRealmsAPI == nullptr) {
		return false;  // Can't verify, don't delete
	}

	String errorMessage;
	auto character = swgRealmsAPI->getCharacterBlocking(characterID, galaxyID, errorMessage);

	// Return true (delete) if character not found (orphan in BerkeleyDB)
	return character.is_null() || character.size() == 0;
#endif // WITH_SWGREALMS_API
}

bool PlayerManagerImplementation::doBurstRun(CreatureObject* player, float hamModifier, float cooldownModifier) {
	if (player == nullptr)
		return false;

	if (player->isRidingMount()) {
		player->sendSystemMessage("@cbt_spam:no_burst"); // You cannot burst-run while mounted on a creature or vehicle.
		return false;
	}

	if (player->hasBuff(STRING_HASHCODE("gallop")) || player->hasBuff(STRING_HASHCODE("dash")) || player->hasBuff(STRING_HASHCODE("burstrun")) ||
	 		player->hasBuff(STRING_HASHCODE("retreat")) || player->hasBuff(STRING_HASHCODE("rocketboots")) || player->hasBuff(STRING_HASHCODE("luaSetSpeed"))) {
		player->sendSystemMessage("You cannot use Burst Run while another speed buff is active.");
		return false;
	}

	if(player->hasBuff(BuffCRC::JEDI_FORCE_RUN_1) || player->hasBuff(BuffCRC::JEDI_FORCE_RUN_2) || player->hasBuff(BuffCRC::JEDI_FORCE_RUN_3)) {
		player->sendSystemMessage("You cannot use Burst Run while Force run is active.");
		return false;
	}

	Zone* zone = player->getZone();

	if (zone == nullptr) {
		return false;
	}

	if (zone->getZoneName() == "dungeon1") {
		player->sendSystemMessage("@combat_effects:burst_run_space_dungeon"); // The artificial gravity makes burst running impossible here.
		return false;
	}

	if (!player->checkCooldownRecovery("burstrun")) {
		player->sendSystemMessage("@combat_effects:burst_run_wait"); //You are too tired to Burst Run.
		return false;
	}

	uint32 crc = STRING_HASHCODE("burstrun");
	float hamCost = 100.0f;
	float duration = 30;
	float cooldown = 300;

	float burstRunMod = (float) player->getSkillMod("burst_run");
	hamModifier += (burstRunMod / 100.f);

	if (hamModifier > 1.0f) {
		hamModifier = 1.0f;
	}

	float hamReduction = 1.f - hamModifier;

	int healthCost = (int) (player->calculateCostAdjustment(CreatureAttribute::STRENGTH, hamCost) * hamReduction);
	int actionCost = (int) (player->calculateCostAdjustment(CreatureAttribute::QUICKNESS, hamCost) * hamReduction);
	int mindCost = (int) (player->calculateCostAdjustment(CreatureAttribute::FOCUS, hamCost) * hamReduction);

	if (player->getHAM(CreatureAttribute::HEALTH) <= healthCost || player->getHAM(CreatureAttribute::ACTION) <= actionCost || player->getHAM(CreatureAttribute::MIND) <= mindCost) {
		player->sendSystemMessage("@combat_effects:burst_run_wait"); // You are too tired to Burst Run.
		return false;
	}

	player->inflictDamage(player, CreatureAttribute::HEALTH, healthCost, true);
	player->inflictDamage(player, CreatureAttribute::ACTION, actionCost, true);
	player->inflictDamage(player, CreatureAttribute::MIND, mindCost, true);

	if (cooldownModifier > 1.0f) {
		cooldownModifier = 1.0f;
	}

	float coodownReduction = 1.f - cooldownModifier;
	cooldown *= coodownReduction;
	int newCooldown = (int) cooldown;

	StringIdChatParameter startStringId("cbt_spam", "burstrun_start_single");
	StringIdChatParameter modifiedStartStringId("combat_effects", "instant_burst_run");
	StringIdChatParameter endStringId("cbt_spam", "burstrun_stop_single");

	ManagedReference<Buff*> buff = new Buff(player, crc, duration, BuffType::SKILL);

	Locker locker(buff);

	buff->setSpeedMultiplierMod(1.822f);
	buff->setAccelerationMultiplierMod(1.822f);

	if (cooldownModifier == 0.f)
		buff->setStartMessage(startStringId);
	else
		buff->setStartMessage(modifiedStartStringId);

	buff->setEndMessage(endStringId);

	StringIdChatParameter startSpam("cbt_spam", "burstrun_start");
	StringIdChatParameter endSpam("cbt_spam", "burstrun_stop");
	buff->setStartSpam(startSpam);
	buff->setEndSpam(endSpam);
	buff->setBroadcastSpam(true);

	player->addBuff(buff);

	player->updateCooldownTimer("burstrun", (newCooldown + duration) * 1000);

	Reference<BurstRunNotifyAvailableEvent*> task = new BurstRunNotifyAvailableEvent(player);
	player->addPendingTask("burst_run_notify", task, (newCooldown + duration) * 1000);

	return true;
}

bool PlayerManagerImplementation::doEnhanceCharacter(uint32 crc, CreatureObject* player, int amount, int duration, int buffType, uint8 attribute) {
	if (player == nullptr)
		return false;

	//Infinity:  Overwrite buffs if higher
	if (player->hasBuff(crc)) {
		Buff* existingbuff = player->getBuff(crc);

		if (existingbuff != nullptr) {
			int existingAmount = existingbuff->getAttributeModifierValue(attribute);

			if (existingAmount > amount)
				return false;
		}
	}

	ManagedReference<Buff*> buff = new Buff(player, crc, duration, buffType);

	Locker locker(buff);

	String name = BuffCRC::getBuffNameFromCRC(crc);
	if (name != "Unknown CRC") {
		buff->setBuffName(name);
	}

	buff->setAttributeModifier(attribute, amount);
	player->addBuff(buff);

	return true;
}

void PlayerManagerImplementation::enhanceCharacter(CreatureObject* player) {
	if (player == nullptr)
		return;

	bool message = true;

	message = message && doEnhanceCharacter(BuffCRC::MEDICAL_ENHANCE_HEALTH, player, medicalBuff, medicalDuration, BuffType::MEDICAL, 0); // medical_enhance_health
	message = message && doEnhanceCharacter(BuffCRC::MEDICAL_ENHANCE_STRENGTH, player, medicalBuff, medicalDuration, BuffType::MEDICAL, 1); // medical_enhance_strength
	message = message && doEnhanceCharacter(BuffCRC::MEDICAL_ENHANCE_CONSTITUTION, player, medicalBuff, medicalDuration, BuffType::MEDICAL, 2); // medical_enhance_constitution
	message = message && doEnhanceCharacter(BuffCRC::MEDICAL_ENHANCE_ACTION, player, medicalBuff, medicalDuration, BuffType::MEDICAL, 3); // medical_enhance_action
	message = message && doEnhanceCharacter(BuffCRC::MEDICAL_ENHANCE_QUICKNESS, player, medicalBuff, medicalDuration, BuffType::MEDICAL, 4); // medical_enhance_quickness
	message = message && doEnhanceCharacter(BuffCRC::MEDICAL_ENHANCE_STAMINA, player, medicalBuff, medicalDuration, BuffType::MEDICAL, 5); // medical_enhance_stamina

	message = message && doEnhanceCharacter(BuffCRC::PERFORMANCE_ENHANCE_DANCE_MIND, player, performanceBuff, performanceDuration, BuffType::PERFORMANCE, 6); // performance_enhance_dance_mind
	message = message && doEnhanceCharacter(BuffCRC::PERFORMANCE_ENHANCE_MUSIC_FOCUS, player, performanceBuff, performanceDuration, BuffType::PERFORMANCE, 7); // performance_enhance_music_focus
	message = message && doEnhanceCharacter(BuffCRC::PERFORMANCE_ENHANCE_MUSIC_WILLPOWER, player, performanceBuff, performanceDuration, BuffType::PERFORMANCE, 8); // performance_enhance_music_willpower

	if (message && player->isPlayerCreature())
		player->sendSystemMessage("An unknown force strengthens you for battles yet to come.");
}

void PlayerManagerImplementation::sendAdminJediList(CreatureObject* player) {
	Reference<ObjectManager*> objectManager = player->getZoneServer()->getObjectManager();

	HashTable<String, uint64> names = nameMap->getNames();
	HashTableIterator<String, uint64> iter = names.iterator();

	VectorMap<UnicodeString, int> players;
	uint32 a = STRING_HASHCODE("SceneObject.slottedObjects");
	uint32 b = STRING_HASHCODE("SceneObject.customName");
	uint32 c = STRING_HASHCODE("PlayerObject.jediState");

	while (iter.hasNext()) {
		uint64 creoId = iter.next();
		VectorMap<String, uint64> slottedObjects;
		UnicodeString playerName;
		int state = -1;

		objectManager->getPersistentObjectsSerializedVariable<VectorMap<String, uint64> >(a, &slottedObjects, creoId);
		objectManager->getPersistentObjectsSerializedVariable<UnicodeString>(b, &playerName, creoId);

		uint64 ghostId = slottedObjects.get("ghost");

		if (ghostId == 0) {
			continue;
		}

		objectManager->getPersistentObjectsSerializedVariable<int>(c, &state, ghostId);

		if (state > 1) {
			players.put(playerName, state);
		}
	}

	ManagedReference<SuiListBox*> listBox = new SuiListBox(player, SuiWindowType::ADMIN_JEDILIST);
	listBox->setPromptTitle("Jedi List");
	listBox->setPromptText("This is a list of all characters with a jedi state of 2 or greater (Name - State).");
	listBox->setCancelButton(true, "@cancel");

	for (int i = 0; i < players.size(); i++) {
		listBox->addMenuItem(players.elementAt(i).getKey().toString() + " - " + String::valueOf(players.get(i)));
	}

	Locker locker(player);

	player->getPlayerObject()->closeSuiWindowType(SuiWindowType::ADMIN_JEDILIST);

	player->getPlayerObject()->addSuiBox(listBox);
	player->sendMessage(listBox->generateMessage());
}

// FRS List
void PlayerManagerImplementation::sendAdminFRSList(CreatureObject* player) {

	auto zoneServer = ServerCore::getZoneServer();
	if (zoneServer == nullptr) {
		error("zoneServer is nullptr");
		return;
	}

	auto frsManager = zoneServer->getFrsManager();
	if (frsManager == nullptr) {
		error("frsManager is nullptr");
		return;
	}

	Vector<uint64> frsJediList = frsManager->getFullPlayerList();

	ManagedReference<SuiListBox*> listBox = new SuiListBox(player, SuiWindowType::ADMIN_FRSLIST);
	listBox->setPromptTitle("Force Ranking System List");
	listBox->setPromptText("This is a list of all characters within the Force Ranking System (Name - Council - Rank).");
	listBox->setCancelButton(true, "@cancel");

	std::ofstream frsFile;
  	frsFile.open("log/frsjedi.log");
	frsFile << "Force Ranking System List\n";
	frsFile << "This is a list of all characters within the Force Ranking System (Name|Council|Rank|LastOn).\n";

	for (int i = 0; i < frsJediList.size(); i++) {

		uint64 playerID = frsJediList.get(i);
		ManagedReference<SceneObject*> sceneObject = zoneServer->getObject(playerID);
		if (sceneObject == nullptr)
			continue;
		
		ManagedReference<CreatureObject*> playerCreo = sceneObject->asCreatureObject();
		if (playerCreo == nullptr)
			continue;

		ManagedReference<PlayerObject*> ghost = playerCreo->getPlayerObject();
		if (ghost == nullptr)
			continue;

		int jediState = ghost->getJediState();

		FrsData* playerData = ghost->getFrsData();
		if (playerData == nullptr)
			continue;

		int playerCouncil = playerData->getCouncilType();
		int rank = playerData->getRank();

		String council = "Error";
		if (playerCouncil == 1)
			council = "Light";
		else if (playerCouncil == 2)
			council = "Dark";

		StringBuffer msg;

		msg << playerCreo->getFirstName() << " - " << council << " - " << rank;
		listBox->addMenuItem(msg.toString());
		frsFile << playerCreo->getFirstName().toCharArray() << " - " << council.toCharArray() << " - " << rank << " - " << ghost->getLastLogout()->getFormattedTime().toCharArray() << "\n";

	}
	frsFile.close();

	Locker locker(player);

	player->getPlayerObject()->closeSuiWindowType(SuiWindowType::ADMIN_FRSLIST);

	player->getPlayerObject()->addSuiBox(listBox);
	player->sendMessage(listBox->generateMessage());
}

VectorMap<String, int> PlayerManagerImplementation::generateAdminList() {
	static Mutex guard; //only one thread cann run this

	Locker locker(&guard);

	VectorMap<String, int> players;
	HashTable<String, uint64> names = nameMap->getNames();
	HashTableIterator<String, uint64> iter = names.iterator();

	auto objectManager = server->getObjectManager();
	auto taskManager = Core::getTaskManager();

	constexpr const int objectsPerTask = 500;

	static SynchronizedVectorMap<String, int> sharedMap;
	static AtomicInteger totalObjects;

	totalObjects = names.size();

	Vector<VectorMapEntry<String, uint64>> currentObjects;

	int count = 0;
	static TaskQueue* customQueue = [taskManager] () { return taskManager->initializeCustomQueue("AdminListThreads", 10); } (); //only once

	while (iter.hasNext()) {
		uint64 oid;
		String playerName;
		iter.getNextKeyAndValue(playerName, oid);
		++count;

		currentObjects.emplace(std::move(playerName), std::move(oid));

		if (currentObjects.size() >= objectsPerTask || count >= names.size()) {
			taskManager->executeTask([currentObjects, objectManager]() {
				for (const auto& obj : currentObjects) {
					try {
						VectorMap<String, uint64> slottedObjects;
						int state = 0;

						objectManager->getPersistentObjectsSerializedVariable<VectorMap<String, uint64> >(STRING_HASHCODE("SceneObject.slottedObjects"), &slottedObjects, obj.getValue());

						uint64 ghostId = slottedObjects.get("ghost");

						if (ghostId == 0) {
							totalObjects.decrement();
							continue;
						}

						objectManager->getPersistentObjectsSerializedVariable<int>(STRING_HASHCODE("PlayerObject.adminLevel"), &state, ghostId);

						if (state != 0) {
							sharedMap.put(obj.getKey(), state);
						}
					} catch (...) {
						Logger::console.error("unreported exception caught in GetAdminPlayerObjectTask");
					}

					totalObjects.decrement();
				}

			}, "GetAdminPlayerObjectTask", "AdminListThreads");

			currentObjects.removeAll(objectsPerTask, 50);
		}
	}

	taskManager->waitForQueueToFinish("AdminListThreads");

	players = sharedMap.getMapUnsafe();

	sharedMap.removeAll();
	totalObjects = 0;

	return players;
}

void PlayerManagerImplementation::sendAdminList(CreatureObject* player) {
	VectorMap<String, int> players = generateAdminList();

	ManagedReference<SuiListBox*> listBox = new SuiListBox(player, SuiWindowType::ADMIN_LIST);
	listBox->setPromptTitle("Admin List");
	listBox->setPromptText("This is a list of all characters with a admin level of 1 or greater (Name - State).");
	listBox->setCancelButton(true, "@cancel");

	for (int i = 0; i < players.size(); i++) {
		listBox->addMenuItem(players.elementAt(i).getKey() + " - " + String::valueOf(players.get(i)));
	}

	Locker locker(player);

	player->getPlayerObject()->closeSuiWindowType(SuiWindowType::ADMIN_LIST);

	player->getPlayerObject()->addSuiBox(listBox);
	player->sendMessage(listBox->generateMessage());
}

void PlayerManagerImplementation::doPvpDeathRatingUpdate(CreatureObject* player, ThreatMap* threatMap) {
	PlayerObject* ghost = player->getPlayerObject();

	if (ghost == nullptr)
		return;

	uint32 totalDamage = threatMap->getTotalDamage();

	if (totalDamage == 0)
		return;

	int defenderPvpRating = ghost->getPvpRating();
	int victimRatingTotalDelta = 0;
	ManagedReference<CreatureObject*> highDamageAttacker = nullptr;
	uint32 highDamageAmount = 0;

	bool throttleOnly = true;
	bool accountVictimList = ConfigManager::instance()->getBool("PlayerManager.accountVictimList", false);

	for (int i = 0; i < threatMap->size(); ++i) {
		ThreatMapEntry* entry = &threatMap->elementAt(i).getValue();
		TangibleObject* attacker = threatMap->elementAt(i).getKey();

		if (entry == nullptr || attacker == nullptr || attacker == player || !attacker->isPlayerCreature())
			continue;

		CreatureObject* attackerCreo = attacker->asCreatureObject();

		if (attackerCreo == nullptr) {
			continue;
		}

		PlayerObject* attackerGhost = attackerCreo->getPlayerObject();

		if (attackerGhost == nullptr)
			continue;

		Locker crossLock(attackerCreo, player);

		if (!allowSameAccountPvpRatingCredit && ghost->getAccountID() == attackerGhost->getAccountID())
			continue;

		if (entry->getTotalDamage() <= 0)
			continue;

		if (!player->isInRange(attacker, 80.f))
			continue;

		int curAttackerRating = attackerGhost->getPvpRating();

		if (highDamageAmount == 0 || entry->getTotalDamage() > highDamageAmount) {
			highDamageAmount = entry->getTotalDamage();
			highDamageAttacker = attackerCreo;
		}

		uint64 victimId = player->getObjectID();

		if (accountVictimList) {
			victimId = ghost->getAccountID();
		}

		if (attackerGhost->hasOnVictimList(victimId)) {
			String stringFile;

			if (attackerCreo->getSpecies() == CreatureObject::TRANDOSHAN)
				stringFile = "rating_throttle_trandoshan_winner";
			else
				stringFile = "rating_throttle_winner";

			StringIdChatParameter toAttacker;
			toAttacker.setStringId("pvp_rating", stringFile);
			toAttacker.setTT(player->getFirstName());
			toAttacker.setTU(attackerCreo->getObjectID());
			toAttacker.setDI(curAttackerRating);

			attackerCreo->sendSystemMessage(toAttacker);
			continue;
		}

		if (defenderPvpRating <= PlayerObject::PVP_RATING_FLOOR) {
			String stringFile;
			if (attackerCreo->getSpecies() == CreatureObject::TRANDOSHAN)
				stringFile = "rating_floor_trandoshan_winner";
			else
				stringFile = "rating_floor_winner";

			StringIdChatParameter toAttacker;
			toAttacker.setStringId("pvp_rating", stringFile);
			toAttacker.setTT(player->getFirstName());
			toAttacker.setDI(curAttackerRating);

			attackerCreo->sendSystemMessage(toAttacker);
		}

		float damageContribution = (float) entry->getTotalDamage() / totalDamage;

		attackerGhost->addToVictimList(victimId);
		throttleOnly = false;

		if (defenderPvpRating > PlayerObject::PVP_RATING_FLOOR) {
			int attackerRatingDelta = 20 + ((defenderPvpRating - curAttackerRating) / 25);
			int victimRatingDelta = -20 + ((curAttackerRating - defenderPvpRating) / 25);

			if (attackerRatingDelta > 40)
				attackerRatingDelta = 40;
			else if (attackerRatingDelta < 0)
				attackerRatingDelta = 0;

			if (victimRatingDelta < -40)
				victimRatingDelta = -40;
			else if (victimRatingDelta > 0)
				victimRatingDelta = 0;

			attackerRatingDelta *= damageContribution;
			victimRatingDelta *= damageContribution;

			victimRatingTotalDelta += victimRatingDelta;
			int newRating = curAttackerRating + attackerRatingDelta;

			attackerGhost->setPvpRating(newRating);

			crossLock.release();

			String stringFile;

			int randNum = System::random(2) + 1;
			if (attackerCreo->getSpecies() == CreatureObject::TRANDOSHAN)
				stringFile = "trandoshan_win" + String::valueOf(randNum);
			else
				stringFile = "win" + String::valueOf(randNum);

			StringIdChatParameter toAttacker;
			toAttacker.setStringId("pvp_rating", stringFile);
			toAttacker.setTT(player->getFirstName());
			toAttacker.setDI(newRating);

			attackerCreo->sendSystemMessage(toAttacker);
		}
	}

	if (highDamageAttacker == nullptr)
		return;

	if (defenderPvpRating <= PlayerObject::PVP_RATING_FLOOR) {
		String stringFile;
		if (player->getSpecies() == CreatureObject::TRANDOSHAN)
			stringFile = "rating_floor_trandoshan_loser";
		else
			stringFile = "rating_floor_victim";

		StringIdChatParameter toVictim;
		toVictim.setStringId("pvp_rating", stringFile);
		toVictim.setTT(highDamageAttacker->getFirstName());
		toVictim.setDI(defenderPvpRating);

		player->sendSystemMessage(toVictim);
	} else if (victimRatingTotalDelta != 0) {
		int newDefenderRating = defenderPvpRating + victimRatingTotalDelta;

		if (newDefenderRating < PlayerObject::PVP_RATING_FLOOR)
			newDefenderRating = PlayerObject::PVP_RATING_FLOOR;

		ghost->setPvpRating(newDefenderRating);

		String stringFile;

		int randNum = System::random(2) + 1;
		if (player->getSpecies() == CreatureObject::TRANDOSHAN)
			stringFile = "trandoshan_killed" + String::valueOf(randNum);
		else
			stringFile = "killed" + String::valueOf(randNum);

		StringIdChatParameter toVictim;
		toVictim.setStringId("pvp_rating", stringFile);
		toVictim.setTT(highDamageAttacker->getFirstName());
		toVictim.setDI(newDefenderRating);

		player->sendSystemMessage(toVictim);
	} else if (throttleOnly) {
		String stringFile;

		if (player->getSpecies() == CreatureObject::TRANDOSHAN)
			stringFile = "rating_throttle_trandoshan_loser";
		else
			stringFile = "rating_throttle_loser";

		StringIdChatParameter toVictim;
		toVictim.setStringId("pvp_rating", stringFile);
		toVictim.setTT(highDamageAttacker->getFirstName());
		toVictim.setDI(defenderPvpRating);

		player->sendSystemMessage(toVictim);
	}
}

float PlayerManagerImplementation::getSpeciesXpModifier(const String& species, const String& xpType) {
	int bonus = xpBonusList.get(species).get(xpType);

	if (bonus == -1)
		bonus = xpBonusList.get(species).get("all");

	if (bonus == -1)
		return 1.f;

	return (100.f + bonus) / 100.f;
}

void PlayerManagerImplementation::unlockFRSForTesting(CreatureObject* player, int councilType) {
	PlayerObject* ghost = player->getPlayerObject();

	if (ghost == nullptr)
		return;

	if (player->hasSkill("force_rank_light_novice") || player->hasSkill("force_rank_dark_novice")) {
		player->sendSystemMessage("You already have FRS skills. You must drop them before using this feature again.");
		return;
	}

	SkillManager* skillManager = SkillManager::instance();

	int glowyBadgeIds[] = { 12, 14, 15, 16, 17, 19, 20, 21, 23, 30, 38, 39, 71, 105, 106, 107 };

	for (int i = 0; i < 16; i++) {
		ghost->awardBadge(glowyBadgeIds[i]);
	}

	SkillManager::instance()->surrenderAllSkills(player, true, false);

	Lua* lua = DirectorManager::instance()->getLuaInstance();

	Reference<LuaFunction*> luaFrsTesting = lua->createFunction("FsIntro", "completeVillageIntroFrog", 0);
	*luaFrsTesting << player;

	luaFrsTesting->callFunction();

	String branches[] = {
			"force_sensitive_combat_prowess_ranged_accuracy",
			"force_sensitive_combat_prowess_ranged_speed",
			"force_sensitive_combat_prowess_melee_accuracy",
			"force_sensitive_combat_prowess_melee_speed",
			"force_sensitive_enhanced_reflexes_ranged_defense",
			"force_sensitive_enhanced_reflexes_melee_defense"
		};

	for (int i = 0; i < 6; i++) {
		String branch = branches[i];
		player->setScreenPlayState("VillageUnlockScreenPlay:" + branch, 2);
		skillManager->awardSkill(branch + "_04", player, true, true, true);
	}

	luaFrsTesting = lua->createFunction("FsOutro", "completeVillageOutroFrog", 0);
	*luaFrsTesting << player;

	luaFrsTesting->callFunction();

	luaFrsTesting = lua->createFunction("JediTrials", "completePadawanForTesting", 0);
	*luaFrsTesting << player;

	luaFrsTesting->callFunction();

	skillManager->awardSkill("force_discipline_light_saber_master", player, true, true, true);
	skillManager->awardSkill("force_discipline_enhancements_master", player, true, true, true);
	skillManager->awardSkill("force_discipline_healing_damage_04", player, true, true, true);
	skillManager->awardSkill("force_discipline_healing_states_04", player, true, true, true);

	luaFrsTesting = lua->createFunction("JediTrials", "completeKnightForTesting", 0);
	*luaFrsTesting << player;
	*luaFrsTesting << councilType;

	luaFrsTesting->callFunction();
}

Vector<uint64> PlayerManagerImplementation::getOnlinePlayerList() {
	Vector<uint64> playerList;

	Locker locker(&onlineMapMutex);

	HashTableIterator<uint32, Vector<Reference<ZoneClientSession*> > > iter = onlineZoneClientMap.iterator();

	while (iter.hasNext()) {
		Vector<Reference<ZoneClientSession*> > clients = iter.next();

		for (int i = 0; i < clients.size(); i++) {
			ZoneClientSession* session = clients.get(i);

			if (session != nullptr) {
				CreatureObject* player = session->getPlayer();

				if (player != nullptr) {
					playerList.add(player->getObjectID());
				}
			}
		}
	}

	return playerList;
}

void PlayerManagerImplementation::logOnlinePlayers(bool onlyWho) {
	int countOnline = 0;
	int countAccounts = 0;
	int countPlayers = 0;
	int countnullptrClient = 0;
	int countnullptrCreature = 0;
	int countnullptrGhost = 0;

	JSONSerializationType logClients;

	Locker locker(&onlineMapMutex);

	int countDistinctIPs = onlineZoneClientMap.getDistinctIps();

	auto iter = onlineZoneClientMap.iterator();

	while (iter.hasNext()) {
		countAccounts++;

		auto clients = iter.next();

		for (int i = 0;i < clients.size();i++) {
			auto client = clients.get(i);

			if (client == nullptr) {
				countnullptrClient++;
				continue;
			}

			JSONSerializationType logClient;

			logClient["accountID"] = client->getAccountID();
			logClient["ip"] = client->getIPAddress();

			Reference<CreatureObject*> creature = client->getPlayer();

			if (creature != nullptr) {
				countPlayers++;

				logClient["oid"] = creature->getObjectID();
				logClient["firstName"] = creature->getFirstName();

				if (creature->isInvisible())
					logClient["invisible"] = true;

				Reference<PlayerObject*> ghost = creature->getPlayerObject();

				if (ghost != nullptr) {
					Locker lock(ghost);

					logClient["playedSeconds"] = (int)(ghost->getPlayedMiliSecs() / 1000);
					logClient["sessionSeconds"] = (int)(ghost->getSessionMiliSecs() / 1000);
					logClient["totalMovement"] = ghost->getSessionTotalMovement();

					auto admin_level = ghost->getAdminLevel();

					if (admin_level > 0 && ghost->hasAbility("admin"))
						logClient["admin_level"] = admin_level;

					if (ghost->isOnline()) {
						countOnline++;

						auto zone = creature->getZone();

						if (zone != nullptr) {
							auto worldPosition = creature->getWorldPosition();

							logClient["worldPositionX"] = (int)worldPosition.getX();
							logClient["worldPositionZ"] = (int)worldPosition.getZ();
							logClient["worldPositionY"] = (int)worldPosition.getY();
							logClient["zone"] = zone->getZoneName();
						}
					} else {
						logClient["isOnline"] = false;
					}

					if (ghost->isAFK())
						logClient["isAFK"] = true;
				} else {
					countnullptrGhost++;
				}
			} else {
				countnullptrCreature++;
				logClient["isNullCreature"] = true;
				logClient["isNullSession"] = client->getSession() == nullptr;
				// TODO - Periodic cleanup of these?
			}

			logClients.push_back(logClient);
		}
	}

	locker.release();

	JSONSerializationType logEntry;
	Time now;

	logEntry["@timestamp"] = now.getFormattedTimeFull().toCharArray();
	logEntry["timeMSecs"] = now.getMiliTime();
	logEntry["clients"] = logClients.size() > 0 ? logClients : nlohmann::json::array();
	logEntry["uptime_secs"] = Logger::getElapsedTime();

	auto server = ServerCore::getZoneServer();

	if (server != nullptr) {
		logEntry["uptime"] = (int)(server->getStartTimestamp()->miliDifference(now) / 1000);

		if (server->isServerLocked())
			logEntry["isServerLocked"] = true;

		if (server->isServerLoading())
			logEntry["isServerLoading"] = true;

		if (server->isServerShuttingDown())
			logEntry["isServerShuttingDown"] = true;
	}

	logEntry["countAccounts"] = countAccounts;
	logEntry["countPlayers"] = countPlayers;
	logEntry["countDistinctIPs"] = countDistinctIPs;

	if (countOnline != countPlayers)
		logEntry["countOnline"] = countOnline;

	if (countnullptrClient > 0)
		logEntry["countnullptrClient"] = countnullptrClient;

	if (countnullptrCreature > 0)
		logEntry["countnullptrCreature"] = countnullptrCreature;

	if (countnullptrGhost > 0)
		logEntry["countnullptrGhost"] = countnullptrGhost;

	StringBuffer logLine;

	logLine << logEntry.dump().c_str() << "\n";

	// Write who file
	try {
		// Write a new "current status" file
		File file("log/who.json.next");
		FileWriter logFile(&file, false);

		logFile << logLine;

		logFile.close();

		// Update current status file
#ifdef PLATFORM_WIN
		std::remove("log/who.json");
#endif
		int err = std::rename("log/who.json.next", "log/who.json");

		if (err != 0) {
			error() << "Failed to rename log/who.json.next to log/who.json err = " << err;
		}
	} catch (const Exception& e) {
		error() << "logOnlinePlayers failed to write log/who.json: " << e.getMessage();
	}

	if (onlyWho)
		return;

	Locker logfileLock(&onlinePlayerLogMutext);

	String fileName = "log/online-players.log";

	struct stat st_log;

	// Check for rollover
	if (stat(fileName.toCharArray(), &st_log) == 0) {
		if (st_log.st_size >= ConfigManager::instance()->getOnlineLogSize()) {
			StringBuffer archiveFilename;
			archiveFilename << "log/online-players-" << now.getMiliTime() << ".log";

			// If the rename failed its ok because we open with append below
			int err = std::rename(fileName.toCharArray(), archiveFilename.toString().toCharArray());

			if (err != 0)
				error() << "Failed to archive online-players to " << archiveFilename.toString() << " err = " << err;
		}
	}

	try {
		// Append log file with this entry
		File file(fileName);
		FileWriter logFile(&file, true);

		logFile << logLine;

		logFile.close();

		logfileLock.release();

		int LogSum = countOnline + countAccounts + countPlayers + countnullptrClient + countnullptrCreature + countnullptrGhost + countDistinctIPs;

		auto statisticsManager = StatisticsManager::instance();

		if (statisticsManager != nullptr) {
			statisticsManager->setAccountsCount(countAccounts);
			statisticsManager->setOnlineCount(countOnline);
			statisticsManager->setDistinctIPsCount(countDistinctIPs);
		}

		// Throttle to no more often than once per 20s and only if something to report
		if (lastOnlinePlayerLogMsg.miliDifference() >= 30000 && LogSum != onlinePlayerLogSum) { //Infinity:   5 secs to 30 secs
			StringBuffer logMsg;

			logMsg << "Logged " << countOnline << " players (" << countAccounts << " accounts, " << countDistinctIPs << " distinct IPs) to " << fileName;

			if (countnullptrClient > 0)
				logMsg << "; " << countnullptrClient << " null clients";

			if (countnullptrCreature > 0)
				logMsg << "; " << countnullptrCreature << " clients without a creature";

			if (countnullptrGhost > 0)
				logMsg << "; " << countnullptrGhost << " creatures without a player object";

			logMsg << ".";

			info(logMsg.toString(), true);
			lastOnlinePlayerLogMsg.updateToCurrentTime();
			onlinePlayerLogSum = LogSum;
		}
	} catch (const Exception& e) {
		error() << "logOnlinePlayers failed to write " << fileName << ": " << e.getMessage();
	}
}

void PlayerManagerImplementation::iteratePlayerNames(const PlayerNameIterator& iterator) {
	auto names = nameMap->getNames();
	auto iter = names.iterator();

	while (iter.hasNext()) {
		String name;
		uint64 oid;
		iter.getNextKeyAndValue(name, oid);
		iterator(name, oid);
	}
}

//Infinity:  Custom functions below
bool PlayerManagerImplementation::doDash(CreatureObject* player, float hamModifier, float cooldownModifier) {
	if (player == nullptr)
		return false;

	if (player->isRidingMount()) {
		player->sendSystemMessage("You cannot use dash while mounted.");
		return false;
	}

	if (player->hasBuff(STRING_HASHCODE("gallop")) || player->hasBuff(STRING_HASHCODE("dash")) || player->hasBuff(STRING_HASHCODE("burstrun")) ||
	 		player->hasBuff(STRING_HASHCODE("retreat")) || player->hasBuff(STRING_HASHCODE("rocketboots")) || player->hasBuff(STRING_HASHCODE("luaSetSpeed"))) {
		player->sendSystemMessage("You cannot use Dash while another speed buff is active.");
		return false;
	}

	if(player->hasBuff(BuffCRC::JEDI_FORCE_RUN_1) || player->hasBuff(BuffCRC::JEDI_FORCE_RUN_2) || player->hasBuff(BuffCRC::JEDI_FORCE_RUN_3)) {
		player->sendSystemMessage("You cannot use Dash while Force run is active.");
		return false;
	}

	Zone* zone = player->getZone();

	if (zone == nullptr) {
		return false;
	}

	if (!player->checkCooldownRecovery("dash")) {
		player->sendSystemMessage("You are too tired to dash."); //You are too tired to Burst Run.
		return false;
	}

	uint32 crc = STRING_HASHCODE("dash");
	float hamCost = 100.0f;
	float duration = 4;
	float cooldown = 30;

	float burstRunMod = (float) player->getSkillMod("burst_run");
	hamModifier += (burstRunMod / 100.f);

	if (hamModifier > 1.0f) {
		hamModifier = 1.0f;
	}

	float hamReduction = 1.f - hamModifier;
	hamCost *= hamReduction;
	int newHamCost = (int) hamCost;

	if (cooldownModifier > 1.0f) {
		cooldownModifier = 1.0f;
	}

	int newCooldown = (int) cooldown;

	if (player->getHAM(CreatureAttribute::HEALTH) <= newHamCost || player->getHAM(CreatureAttribute::ACTION) <= newHamCost || player->getHAM(CreatureAttribute::MIND) <= newHamCost) {
		player->sendSystemMessage("You are too weak to burst run."); // You are too tired to Burst Run.
		return false;
	}

	player->inflictDamage(player, CreatureAttribute::HEALTH, newHamCost, true);
	player->inflictDamage(player, CreatureAttribute::ACTION, newHamCost, true);
	player->inflictDamage(player, CreatureAttribute::MIND, newHamCost, true);

	StringIdChatParameter startStringId("cbt_spam", "burstrun_start_single");
	StringIdChatParameter modifiedStartStringId("combat_effects", "instant_burst_run");
	StringIdChatParameter endStringId("cbt_spam", "burstrun_stop_single");


	ManagedReference<Buff*> buff = new Buff(player, crc, duration, BuffType::SKILL);

	Locker locker(buff);

	buff->setSpeedMultiplierMod(2.222f);
	buff->setAccelerationMultiplierMod(2.222f);

	if (cooldownModifier == 0.f)
		buff->setStartMessage(startStringId);
	else
		buff->setStartMessage(modifiedStartStringId);

	buff->setEndMessage(endStringId);

	StringIdChatParameter startSpam("cbt_spam", "dash_start");
	StringIdChatParameter endSpam("cbt_spam", "dash_stop");
	buff->setStartSpam(startSpam);
	buff->setEndSpam(endSpam);
	buff->setBroadcastSpam(true);

	player->addBuff(buff);

	player->updateCooldownTimer("dash", (newCooldown + duration) * 1000);

	Reference<DashNotifyAvailableEvent*> task = new DashNotifyAvailableEvent(player);
	player->addPendingTask("dash_notify", task, (newCooldown + duration) * 1000);

	return true;
}

bool PlayerManagerImplementation::doRocketBoots(CreatureObject* player, float hamModifier, float cooldownModifier) {
	if (player == nullptr)
		return false;

	if (player->isRidingMount()) {
		player->sendSystemMessage("You cannot use rocket boots while mounted.");
		return false;
	}

	if (player->hasBuff(STRING_HASHCODE("rocketboots"))) { // Toggle off Rocketboots if active
		player->removeBuff(STRING_HASHCODE("rocketboots"));
		return true;
	}

	if (player->hasBuff(STRING_HASHCODE("gallop")) || player->hasBuff(STRING_HASHCODE("dash")) || player->hasBuff(STRING_HASHCODE("burstrun")) ||
	 		player->hasBuff(STRING_HASHCODE("retreat"))) {
		player->sendSystemMessage("You cannot use rocket boots while another speed buff is active.");
		return false;
	}

	if(player->hasBuff(BuffCRC::JEDI_FORCE_RUN_1) || player->hasBuff(BuffCRC::JEDI_FORCE_RUN_2) || player->hasBuff(BuffCRC::JEDI_FORCE_RUN_3)) {
		player->sendSystemMessage("You cannot use rocket boots while Force run is active.");
		return false;
	}

	auto zone = player->getZone();

	if (zone == nullptr) {
		return false;
	}

	if (!player->checkCooldownRecovery("rocketboots")) {
		player->sendSystemMessage("Your rocket boots have not yet recharged.");
		return false;
	}

	uint32 crc = STRING_HASHCODE("rocketboots");
	float duration = 30;
	float cooldown = 300;

	StringIdChatParameter startStringId("cbt_spam", "rocketboots_start_single");
	StringIdChatParameter modifiedStartStringId("combat_effects", "instant_rocket_boots");
	StringIdChatParameter endStringId("cbt_spam", "rocketboots_stop_single");

	int newCooldown = (int) cooldown;

	//Infinity:   Create Rocketboots speed buff
	ManagedReference<Buff*> buff = new Buff(player, crc, duration, BuffType::OTHER);
	Locker locker(buff);

	buff->setSpeedMultiplierMod(1.822f);
	buff->setAccelerationMultiplierMod(1.822f);
	buff->setStartMessage(startStringId);
	buff->setEndMessage(endStringId);
	StringIdChatParameter startSpam("cbt_spam", "rocketboots_start");
	StringIdChatParameter endSpam("cbt_spam", "rocketboots_stop");
	buff->setStartSpam(startSpam);
	buff->setEndSpam(endSpam);
	buff->setBroadcastSpam(true);
	player->addBuff(buff);
	locker.release();

	player->playEffect("clienteffect/ig88_bomb_droid_explode.cef", "");

	//Infinity:  Create Rocketboots damage divisor buff
	ManagedReference<PrivateSkillMultiplierBuff*> multBuff = new PrivateSkillMultiplierBuff(player, STRING_HASHCODE("rocketboots.damage"), duration, BuffType::OTHER);
	Locker mlocker(multBuff);
	multBuff->setSkillModifier("private_damage_divisor_rocketboots", 2);
	player->addBuff(multBuff);
	mlocker.release();

	Locker blocker(buff);
	buff->addSecondaryBuffCRC(STRING_HASHCODE("rocketboots.damage"));
	blocker.release();

	player->updateCooldownTimer("rocketboots", (newCooldown + duration) * 1000);

	Reference<RocketBootsNotifyAvailableEvent*> task = new RocketBootsNotifyAvailableEvent(player);
	player->addPendingTask("rocket_boots_notify", task, (newCooldown + duration) * 1000);

	return true;
}

void PlayerManagerImplementation::gmenhanceCharacter(CreatureObject* player, int buffAmount) {
	if (player == nullptr)
		return;

	bool message = true;

	int entbuffAmount = buffAmount * 0.65;

	// Buff for the amount specified in the command for 3 hours

	message = message && doEnhanceCharacter(BuffCRC::MEDICAL_ENHANCE_HEALTH, player, buffAmount, 12600, BuffType::MEDICAL, 0); // medical_enhance_health
	message = message && doEnhanceCharacter(BuffCRC::MEDICAL_ENHANCE_STRENGTH, player, buffAmount, 12600, BuffType::MEDICAL, 1); // medical_enhance_strength
	message = message && doEnhanceCharacter(BuffCRC::MEDICAL_ENHANCE_CONSTITUTION, player, buffAmount, 12600, BuffType::MEDICAL, 2); // medical_enhance_constitution
	message = message && doEnhanceCharacter(BuffCRC::MEDICAL_ENHANCE_ACTION, player, buffAmount, 12600, BuffType::MEDICAL, 3); // medical_enhance_action
	message = message && doEnhanceCharacter(BuffCRC::MEDICAL_ENHANCE_QUICKNESS, player, buffAmount, 12600, BuffType::MEDICAL, 4); // medical_enhance_quickness
	message = message && doEnhanceCharacter(BuffCRC::MEDICAL_ENHANCE_STAMINA, player, buffAmount, 12600, BuffType::MEDICAL, 5); // medical_enhance_stamina

	message = message && doEnhanceCharacter(BuffCRC::PERFORMANCE_ENHANCE_DANCE_MIND, player, entbuffAmount, 12600, BuffType::PERFORMANCE, 6); // performance_enhance_dance_mind
	message = message && doEnhanceCharacter(BuffCRC::PERFORMANCE_ENHANCE_MUSIC_FOCUS, player, entbuffAmount, 12600, BuffType::PERFORMANCE, 7); // performance_enhance_music_focus
	message = message && doEnhanceCharacter(BuffCRC::PERFORMANCE_ENHANCE_MUSIC_WILLPOWER, player, entbuffAmount, 12600, BuffType::PERFORMANCE, 8); // performance_enhance_music_willpower

	if (message && player->isPlayerCreature())
		player->sendSystemMessage("A GM has enhanced your character's health and mind pools.");
}

void PlayerManagerImplementation::regainEnhanceCharacter(CreatureObject* player, int buffAmount, int duration) {
	if (player == nullptr)
		return;

	bool message = true;

	int entbuffAmount = buffAmount * 0.65;

	// Buff for the amount specified in the command for 3 hours

	message = message && doEnhanceCharacter(BuffCRC::MEDICAL_ENHANCE_HEALTH, player, buffAmount, duration, BuffType::MEDICAL, 0); // medical_enhance_health
	message = message && doEnhanceCharacter(BuffCRC::MEDICAL_ENHANCE_STRENGTH, player, buffAmount, duration, BuffType::MEDICAL, 1); // medical_enhance_strength
	message = message && doEnhanceCharacter(BuffCRC::MEDICAL_ENHANCE_CONSTITUTION, player, buffAmount, duration, BuffType::MEDICAL, 2); // medical_enhance_constitution
	message = message && doEnhanceCharacter(BuffCRC::MEDICAL_ENHANCE_ACTION, player, buffAmount, duration, BuffType::MEDICAL, 3); // medical_enhance_action
	message = message && doEnhanceCharacter(BuffCRC::MEDICAL_ENHANCE_QUICKNESS, player, buffAmount, duration, BuffType::MEDICAL, 4); // medical_enhance_quickness
	message = message && doEnhanceCharacter(BuffCRC::MEDICAL_ENHANCE_STAMINA, player, buffAmount, duration, BuffType::MEDICAL, 5); // medical_enhance_stamina

	message = message && doEnhanceCharacter(BuffCRC::PERFORMANCE_ENHANCE_DANCE_MIND, player, entbuffAmount, duration, BuffType::PERFORMANCE, 6); // performance_enhance_dance_mind
	message = message && doEnhanceCharacter(BuffCRC::PERFORMANCE_ENHANCE_MUSIC_FOCUS, player, entbuffAmount, duration, BuffType::PERFORMANCE, 7); // performance_enhance_music_focus
	message = message && doEnhanceCharacter(BuffCRC::PERFORMANCE_ENHANCE_MUSIC_WILLPOWER, player, entbuffAmount, duration, BuffType::PERFORMANCE, 8); // performance_enhance_music_willpower

	if (message && player->isPlayerCreature())
		player->sendSystemMessage("You use the force to enhanced your character's health and mind pools.");
}

bool PlayerManagerImplementation::enhanceCreature(CreatureObject* player, int medicalBuffAmount, int performanceBuffAmount, int durationSecs) {

	if (player == nullptr)
		return false;

	// Buff for the specified amounts and hours

	bool buffed = false;

    if (medicalBuffAmount > 0 && durationSecs > 0) {
		buffed = doEnhanceCharacter(BuffCRC::MEDICAL_ENHANCE_HEALTH, player, medicalBuffAmount, durationSecs, BuffType::MEDICAL, 0) || buffed; // medical_enhance_health
		buffed = doEnhanceCharacter(BuffCRC::MEDICAL_ENHANCE_STRENGTH, player, medicalBuffAmount, durationSecs, BuffType::MEDICAL, 1) || buffed; // medical_enhance_strength
		buffed = doEnhanceCharacter(BuffCRC::MEDICAL_ENHANCE_CONSTITUTION, player, medicalBuffAmount, durationSecs, BuffType::MEDICAL, 2) || buffed; // medical_enhance_constitution
		buffed = doEnhanceCharacter(BuffCRC::MEDICAL_ENHANCE_ACTION, player, medicalBuffAmount, durationSecs, BuffType::MEDICAL, 3) || buffed; // medical_enhance_action
	    buffed = doEnhanceCharacter(BuffCRC::MEDICAL_ENHANCE_QUICKNESS, player, medicalBuffAmount, durationSecs, BuffType::MEDICAL, 4) || buffed; // medical_enhance_quickness
	    buffed = doEnhanceCharacter(BuffCRC::MEDICAL_ENHANCE_STAMINA, player, medicalBuffAmount, durationSecs, BuffType::MEDICAL, 5) || buffed; // medical_enhance_stamina
    }

    if (performanceBuffAmount > 0 && durationSecs > 0) {
	    buffed = doEnhanceCharacter(BuffCRC::PERFORMANCE_ENHANCE_DANCE_MIND, player, performanceBuffAmount, durationSecs, BuffType::PERFORMANCE, 6) || buffed; // performance_enhance_dance_mind
	    buffed = doEnhanceCharacter(BuffCRC::PERFORMANCE_ENHANCE_MUSIC_FOCUS, player, performanceBuffAmount, durationSecs, BuffType::PERFORMANCE, 7) || buffed; // performance_enhance_music_focus
	    buffed = doEnhanceCharacter(BuffCRC::PERFORMANCE_ENHANCE_MUSIC_WILLPOWER, player, performanceBuffAmount, durationSecs, BuffType::PERFORMANCE, 8) || buffed; // performance_enhance_music_willpower
    }

	return buffed;

}

/*
void PlayerManagerImplementation::setFRSExperienceMultiplier(float newFRSMultiplier) {
	FRSExpMultiplier = newFRSMultiplier;
}
*/

void PlayerManagerImplementation::revokeBadge(PlayerObject* ghost, uint32 badgeId) {
	const Badge* badge = BadgeList::instance()->get(badgeId);
	if (ghost != nullptr && badge != nullptr)
		revokeBadge(ghost, badge);
}

void PlayerManagerImplementation::revokeBadge(PlayerObject* ghost, const Badge* badge) {
	if (ghost == nullptr) {
		error("ghost is nullptr");
		return;
	}
	if (badge == nullptr) {
		ghost->error("Failed to revoke null badge.");
		return;
	}

	StringIdChatParameter stringId("badge_n", "");
	stringId.setTO("badge_n", badge->getKey());

	ManagedReference<CreatureObject*> player = dynamic_cast<CreatureObject*>(ghost->getParent().get().get());
	const unsigned int badgeId = badge->getIndex();
	if (ghost->hasBadge(badgeId)) {
		stringId.setStringId("badge_n", "prose_revoke");
		player->sendSystemMessage(stringId);
        ghost->revokeBadge(badgeId);
	}

    validateBadges(ghost, true);

}

// =============================================================================================
// Infinity: FRS (Force Ranking System) Kill Calculation - Refactored for Async Performance
// =============================================================================================
//
// DESIGN OVERVIEW:
//   The FRS calculation determines how much Force Ranking XP to award/deduct after a PvP kill.
//   It must analyze threat maps, group compositions, power ratios, and numerous eligibility rules.
//
//   OLD APPROACH (synchronous):
//     The entire calculation ran on the combat thread at kill time, iterating the killer's group
//     3 separate times and holding locks throughout ~800 lines of processing.
//
//   NEW APPROACH (async with snapshot):
//     1. SYNC (combat thread):  Quick validation + BH shortcut + snapshot threat maps + schedule task
//     2. ASYNC (task worker):   Single-pass gather → award math → distribute payouts
//     The combat thread is freed almost immediately. All heavy iteration happens off-thread.
//
//   The async adjustFrsExperience calls (the actual DB/state mutations) were already dispatched
//   as tasks in the old code. This refactor moves the data-gathering and math off-thread too.
//
// DATA FLOW:
//   calculateFRS()  →  [snapshot at kill time]  →  scheduleTask  →  [async task]:
//     ├── gatherKillerSide:  victim threatMap + killer group  →  killerGroupPower, highestFrsRank
//     ├── gatherVictimSide:  killer threatMap + victim group  →  victimGroupPower, victimAccounts
//     ├── calculateAward:    power ratio math, bonuses, penalties
//     └── distributeAwards:  killer payout + group payouts + victim deduction
//
// =============================================================================================

// Infinity: Helper struct: kill-time snapshot of a group member's full state.
// All eligibility flags, position, and skill checks captured at sync phase so the
// async task is a pure function of the snapshot — no live reads, no race against
// concurrent state changes (e.g. victim teleporting to Rikers, member dying, etc.).
struct FRSGroupPayoutCandidate {
	ManagedReference<CreatureObject*> creature;
	ManagedReference<Zone*> zoneAtKill;
	Vector3 posAtKill;
	uint32 accountID;
	int rank;
	float power;                  // Infinity: getPowerRating snapshot
	bool isFRSKnight;             // Infinity: has force_rank_light_novice or force_rank_dark_novice
	bool isOvert;                 // Infinity: getFactionStatus == OVERT
	bool isMandoBH;               // Infinity: mandalorian_master OR combat_bountyhunter_investigation_03
	bool hasBhTef;                // Infinity: PlayerObject::hasBhTef snapshot
	bool sameFactionAsVictim;     // Infinity: same faction as the victim
	bool sameFactionAsKiller;     // Infinity: same faction as the killer
	bool inStadium;               // Infinity: stadium:insideStadium screenplay state
	bool rikersCooldownActive;    // Infinity: rikers_timeout cooldown still active
	bool inPrivateBuilding;       // Infinity: rootParent is a private structure
	bool inDuelWithVictim;        // Infinity: CombatManager::areInDuel(member, victim)

	// Infinity: Required by Engine3 Vector template instantiation (never actually called)
	bool toBinaryStream(ObjectOutputStream* stream) { return false; }
	bool parseFromBinaryStream(ObjectInputStream* stream) { return false; }
};

// Infinity: Helper struct: kill-time snapshot of a non-group threatmap participant.
// Only the subset needed for power-rating math (threatmap participants don't get
// FRS payouts — they only contribute to the opposing side's group power).
struct FRSThreatmapEntrySnapshot {
	ManagedReference<CreatureObject*> creature;
	ManagedReference<Zone*> zoneAtKill;
	Vector3 posAtKill;
	uint32 accountID;
	int rank;
	float power;

	bool toBinaryStream(ObjectOutputStream* stream) { return false; }
	bool parseFromBinaryStream(ObjectInputStream* stream) { return false; }
};

// Infinity: ── getRank ─────────────────────────────────────────────────────────────────────
// Returns the FRS rank (0-11) for a player in the Force Ranking System.
// Returns -1 if not an FRS member (no force_rank_light/dark_novice skill).
int PlayerManagerImplementation::getRank(CreatureObject *player) {
	if (player == nullptr || !player->isPlayerCreature()) {
		return -1;
	}

	if (player->hasSkill("force_rank_light_novice") || player->hasSkill("force_rank_dark_novice")) {
		ManagedReference<PlayerObject*> ghost = player->getPlayerObject();
		if (ghost == nullptr) {
			return -1;
		}

		FrsData* frsData = ghost->getFrsData();
		if (frsData != nullptr) {
			return frsData->getRank();
		}
	}

	return -1;
}

// Infinity: ── getPowerRating ──────────────────────────────────────────────────────────────
// Maps FRS rank to a power rating float used for group power calculations.
// Non-FRS players get 1.0. Ranks 0-11 scale from 1.1 to 2.6.
float PlayerManagerImplementation::getPowerRating(CreatureObject* player) {
	if (player == nullptr || !player->isPlayerCreature()) {
		return 0.0f;
	}

	int rank = getRank(player);

	// Infinity: Ranks 0-7 scale linearly: 1.1 + rank * 0.1
	// Ranks 8-11 scale faster:  2.0, 2.2, 2.4, 2.6
	if (rank < 0) {
		return 1.0f;
	}
	if (rank <= 7) {
		return 1.1f + rank * 0.1f;
	}

	switch (rank) {
		case 8:  return 2.0f;
		case 9:  return 2.2f;
		case 10: return 2.4f;
		case 11: return 2.6f;
		default: return 0.0f;
	}
}

// Infinity: ── isSameFaction ───────────────────────────────────────────────────────────────
// Returns true if both players are on the same faction side (both rebel, both imperial,
// or both neutral/unaligned).
bool PlayerManagerImplementation::isSameFaction(CreatureObject* player1, CreatureObject* player2) {
	if (player1->isRebel() && player2->isRebel()) {
		return true;
	}
	if (player1->isImperial() && player2->isImperial()) {
		return true;
	}
	if ((!player1->isRebel() && !player1->isImperial()) && (!player2->isRebel() && !player2->isImperial())) {
		return true;
	}

	return false;
}

// Infinity: ── calculateFRS ────────────────────────────────────────────────────────────────
// Entry point called from the combat thread when a PvP kill occurs.
// Performs quick validation and BH shortcut synchronously, then snapshots kill-time
// state and schedules the heavy FRS calculation as an async task.
void PlayerManagerImplementation::calculateFRS(CreatureObject* killer, CreatureObject* victim) {

	bool verboseLogging = ConfigManager::instance()->getBool("Core3.PlayerManager.VerboseFRSLogging", false);

	// Infinity: ─── SYNC PHASE: Quick validation (stays on combat thread) ───────────────

	if (killer == nullptr || victim == nullptr) {
		return;
	}

	bool killerIsFRSKnightAtEntry = killer->hasSkill("force_rank_light_novice")
	                              || killer->hasSkill("force_rank_dark_novice");
	if (!killerIsFRSKnightAtEntry) {
		// Infinity: A non-FRS killer earns no FRS. If that killer is OVERT, the FRS
		// victim still pays the death cost (victim-loss-only path — killer/group
		// payout stays disabled by the !killerIsFRSKnight gate downstream).
		// A covert/civilian non-FRS killer triggers no FRS movement at all.
		// A Bounty Hunter killer is EXCLUDED — BH kills of FRS Jedi are handled
		// by the separate bounty path (BountyMissionObjectiveImplementation.cpp:
		// 814-833); a BH must not also trigger a calculateFRS victim deduction.
		// (MrO 2026-05-20: victim loses FRS "if the killer is overt (FRS or
		// non-FRS excluding BH)".)
		if (killer->getFactionStatus() != FactionStatus::OVERT) {
			if (verboseLogging) {
				error("FRS: Killer non-FRS and not overt — no FRS granted or lost");
			}
			return;
		}
		if (killer->hasSkill("combat_bountyhunter_investigation_03")) {
			if (verboseLogging) {
				error("FRS: Killer is a Bounty Hunter — victim FRS handled by bounty path, no calculateFRS deduction");
			}
			return;
		}
		if (verboseLogging) {
			error("FRS: Killer non-FRS but overt — continuing to victim-loss-only path");
		}
	}

	if (killer == victim) {
		if (verboseLogging) {
			error("FRS: killer is victim, no FRS granted");
		}
		return;
	}

	auto zoneServer = victim->getZoneServer();
	if (zoneServer == nullptr) {
		return;
	}

	ManagedReference<FrsManager*> frsManager = zoneServer->getFrsManager();
	if (frsManager == nullptr) {
		return;
	}

	ManagedReference<PlayerObject*> killerGhost = killer->getPlayerObject();
	ManagedReference<PlayerObject*> victimGhost = victim->getPlayerObject();
	if (killerGhost == nullptr || victimGhost == nullptr) {
		return;
	}

	uint32 killerAccountID = killerGhost->getAccountID();
	uint32 victimAccountID = victimGhost->getAccountID();

	// Infinity: ─── BH victim shortcut (fast path, no group analysis needed) ────────────

	if (victim->hasSkill("combat_bountyhunter_investigation_03")) {
		if (!killerIsFRSKnightAtEntry) {
			// Infinity: a non-FRS killer must never reach adjustFrsExperience.
			// BH victims cannot be FRS-ranked (skill-point budget — BH and FRS
			// are mutually exclusive), so there is no victim FRS to deduct here
			// either. No FRS movement on a non-FRS-killer + BH-victim kill.
			if (verboseLogging) {
				error("FRS: BH victim, non-FRS killer — no FRS movement");
			}
			return;
		}

		if (!allowSameAccountPvpRatingCredit && victimAccountID == killerAccountID) {
			if (verboseLogging) {
				error("FRS: No award allowed, same account");
			}
			killer->sendSystemMessage("You cannot receive FRS XP for this kill because the victim is a character from your account.");
			return;
		}

		int bhAward = 3500;
		ManagedReference<CreatureObject*> killerRef = killer;
		Core::getTaskManager()->executeTask([killerRef, frsManager, bhAward] () {
			Locker locker(killerRef);
			Locker clocker(frsManager, killerRef);
			frsManager->adjustFrsExperience(killerRef, bhAward);
		}, "FrsExperienceAdjustLambdaTask");

		if (verboseLogging) {
			error("FRS: victim is BH, granting " + String::valueOf(bhAward) + " FRS to killer and skipping group checks and payouts");
		}
		return;
	}

	// Infinity: ─── Victim faction check ────────────────────────────────────────────────

	if (!victim->isRebel() && !victim->isImperial()) {
		if (verboseLogging) {
			error("FRS: PVP Victim is not factionally aligned, no FRS Granted");
		}
		return;
	}

	// Infinity: ─── SNAPSHOT: Capture kill-time state for async processing ──────────────
	// We copy threat maps and capture object references + stable data so the
	// combat thread can be released immediately. The async task will work from
	// this snapshot. Slight staleness (someone logs out 50ms after the kill)
	// is acceptable for XP calculations — the kill already happened.
	//
	// This follows the same pattern as doPvpDeathRatingUpdate which also
	// copies the threat map and processes it in a scheduled task.

	// Infinity: Deep copy both threat maps (snapshot of who was fighting at kill time)
	Reference<ThreatMap*> victimThreatCopy = new ThreatMap(*victim->getThreatMap());
	Reference<ThreatMap*> killerThreatCopy = new ThreatMap(*killer->getThreatMap());

	// Infinity: Capture ManagedReferences (prevent objects from being collected during task)
	ManagedReference<CreatureObject*> killerRef = killer;
	ManagedReference<CreatureObject*> victimRef = victim;

	// Infinity: Snapshot kill-time scalars (these could change after combat resolves)
	bool inStadium = victim->getScreenPlayState("stadium:insideStadium") & 1;

	// Infinity: Stadium FRS-disabled fast path — if victim is inside the stadium
	// AND the operator has set stadiumFrsMultiplier to 0 (arena mode), bail before
	// doing any of the rank/power/group/threat-map work. Notifies both players and
	// returns; no FRS XP awarded, no XP deducted, no async tasks scheduled.
	if (inStadium && stadiumFrsMultiplier <= 0.0f) {
		if (verboseLogging) {
			error("FRS: Stadium with zero multiplier — skipping FRS calculation entirely");
		}
		killerRef->sendSystemMessage("Stadium kill: FRS XP is currently disabled inside the stadium.");
		victimRef->sendSystemMessage("Stadium loss: FRS XP is currently disabled inside the stadium.");
		return;
	}

	int killerRank = getRank(killer);
	int victimRank = getRank(victim);
	int victimLevel = calculatePlayerLevel(victim);
	bool victimIsKnight = victim->hasSkill("force_title_jedi_rank_03");
	bool victimIsMando = victim->hasSkill("mandalorian_master");
	bool victimHasBhTef = victimGhost->hasBhTef();
	float killerPower = getPowerRating(killer);
	float victimPower = getPowerRating(victim);
	// Infinity: Snapshot zone+position of both killer and victim. CRITICAL — these are
	// used inside the async lambda (and the sync-phase pre-walks) for zone-match and
	// range-check eligibility filters. Reading them live in the async task races
	// against `instant_pvp:travel_to_rikers` (Lua observer chain on the combat thread)
	// teleporting the victim to Rikers between the async task being scheduled and the
	// gather passes iterating — which previously caused 41–42% of battlefield-side
	// group members to be filtered out as "different zone".
	ManagedReference<Zone*> victimZoneAtKill = victim->getZone();
	Vector3 victimPosAtKill = victim->getWorldPosition();
	ManagedReference<Zone*> killerZoneAtKill = killer->getZone();
	Vector3 killerPosAtKill = killer->getWorldPosition();
	int killerGroupID = killer->isGrouped() ? killer->getGroupID() : 0;
	int victimGroupID = victim->isGrouped() ? victim->getGroupID() : 0;
	bool sameAccount = !allowSameAccountPvpRatingCredit && (victimAccountID == killerAccountID);
	bool killerIsOvert = killer->getFactionStatus() == FactionStatus::OVERT;
	bool killerIsFRSKnight = killer->hasSkill("force_rank_light_novice") || killer->hasSkill("force_rank_dark_novice");
	bool killerSameFactionAsVictim = isSameFaction(victim, killer);
	bool killerInDuel = CombatManager::instance()->areInDuel(victim, killer);
	bool allowSameAccount = allowSameAccountPvpRatingCredit;

	// Infinity: ─── SCHEDULE: Dispatch the heavy calculation to a TaskWorker thread ─────
	// This frees the combat thread immediately. The task will acquire brief,
	// targeted locks only when needed for individual operations.

	// Infinity: ─── Helper lambdas for sync-phase eligibility filters ──────────────────
	// These are used by both the sync-phase gather passes (below) and the verbose log
	// formatters. Stateless lambdas (`[]`) so they're trivially copyable.

	// Infinity: Format "zone=X pos=(Y,Z)" for a creature using its LIVE position.
	// Use loc() for participants whose state isn't snapshotted (group/threatmap members).
	auto loc = [](CreatureObject* c) -> String {
		if (c == nullptr) return String(" zone=? pos=(?,?)");
		auto z = c->getZone();
		String zn = (z != nullptr) ? z->getZoneName() : String("?");
		return " zone=" + zn + " pos=(" + String::valueOf((int)c->getWorldPositionX())
		     + "," + String::valueOf((int)c->getWorldPositionY()) + ")";
	};

	// Infinity: Format "zone=X pos=(Y,Z)" using a captured zone+position snapshot.
	// Use locSnap for victim/killer/group-candidate logging — immune to teleport mid-task.
	auto locSnap = [](Zone* z, const Vector3& p) -> String {
		String zn = (z != nullptr) ? z->getZoneName() : String("?");
		return " zone=" + zn + " pos=(" + String::valueOf((int)p.getX()) + "," + String::valueOf((int)p.getY()) + ")";
	};

	// Infinity: Rikers exception predicate — member is on endor with active rikers_timeout cooldown.
	// Used in group-member zone/range checks so battlefield combatants who just died
	// (cooldown set in instant_pvp:playerClone, lua:930, BEFORE switchzone at lua:934)
	// still count as participants while they're respawning at Riker's Island.
	auto isRikersAt = [](CreatureObject* who) -> bool {
		if (who == nullptr) return false;
		auto z = who->getZone();
		return !who->checkCooldownRecovery("rikers_timeout") && z != nullptr && z->getZoneName().toLowerCase() == "endor";
	};

	// Infinity: Snapshot-aware distance check — squared distance from a creature's LIVE
	// world position to a captured Vector3. Mirrors SceneObject::isInRange's flat-z math
	// (SceneObjectImplementation.cpp:1490) but against the captured pos instead of a live
	// SceneObject. Used to compare against victim/killer kill-time positions.
	auto inRangeOfPos = [](CreatureObject* who, const Vector3& pos, float range) -> bool {
		if (who == nullptr) return false;
		Vector3 wp = who->getWorldPosition();
		wp.setZ(0);
		Vector3 vp = pos;
		vp.setZ(0);
		return wp.squaredDistanceTo(vp) <= range * range;
	};

	// Infinity: ─── SYNC-PHASE GATHER: build per-side power totals + payout candidates ──
	// All eligibility filtering happens here on the combat thread, where every read is
	// consistent (the victim has not yet been teleported to Rikers; group members are
	// stable). The async lambda below becomes a pure function of these snapshots — no
	// live reads of zone, position, faction, cooldown, skills, or duel state.
	//
	// This eliminates the order-sensitive race that previously caused 41–42% of
	// battlefield-side group members to be filtered as "different zone" because
	// `victimRef->getZone()` flipped from dathomir to endor mid-iteration as the Lua
	// observer chain (`instant_pvp:playerKilled` → `playerClone` → `travel_to_rikers`
	// → `switchZone`) executed concurrently on the combat thread.

	float killerGroupPower = killerPower;  // Infinity: Start with killer's own power
	int killerGroupSize = 1;               // Infinity: Killer counts as 1
	float victimGroupPower = victimPower;  // Infinity: Start with victim's own power
	int victimGroupSize = 1;

	// Infinity: Per-account dedup table — only highest-ranked CreatureObject per account
	// gets FRS payout. Built across Pass 1A (victim's threatmap) and Pass 1B (killer's group).
	VectorMap<uint32, ManagedReference<CreatureObject*>> highestFrsRank;
	highestFrsRank.put(killerAccountID, killerRef);

	// Infinity: Accounts on the victim's side — used to block opposition-account FRS payout.
	// Built across Pass 2A (killer's threatmap) and Pass 2B (victim's group).
	HashSet<uint32> victimAccounts;
	victimAccounts.add(victimAccountID);

	// Infinity: Pre-built payout candidates for the killer's group (full snapshot per member).
	// The async distribute phase iterates this list using the snapshotted flags, never
	// re-reading live member state.
	Vector<FRSGroupPayoutCandidate> killerGroupCandidates;

	// Infinity: ── Pass 1A (sync): victim's threatmap → killer-side power ─────────────
	// Non-group attackers from the victim's threatmap contribute power to the killer's side.
	// Pets add a flat 0.5; player participants add their power if they pass eligibility
	// (in same zone as victim AT KILL TIME, within 128m of victim's KILL-TIME position).

	if (verboseLogging) {
		error("FRS: Victim threatMap size = " + String::valueOf(victimThreatCopy->size()));
	}

	for (int i = 0; i < victimThreatCopy->size(); ++i) {
		ThreatMapEntry* entry = &victimThreatCopy->elementAt(i).getValue();
		ManagedReference<TangibleObject*> attackerTano = victimThreatCopy->elementAt(i).getKey();

		ManagedReference<CreatureObject*> attacker = nullptr;
		if (attackerTano != nullptr) {
			attacker = attackerTano->asCreatureObject();
		}

		if (entry == nullptr || attacker == nullptr || attacker == killerRef || attacker == victimRef) {
			continue;
		}

		// Infinity: Pets add a flat 0.5 power but don't count as participants
		if (attacker->isPet()) {
			killerGroupPower += 0.5f;
			continue;
		}

		if (!attacker->isPlayerCreature()) {
			continue;
		}

		ManagedReference<PlayerObject*> attackerGhost = attacker->getPlayerObject();
		if (attackerGhost == nullptr) {
			continue;
		}

		if (verboseLogging) {
			error("FRS: VictimThreatmap: checking participant [" + attacker->getFirstName() + "]" + loc(attacker));
		}

		// Infinity: Skip group members — they'll be counted in Pass 1B with group-specific rules
		if (attacker->isGrouped()) {
			int attackerGroupID = attacker->getGroupID();
			if (attackerGroupID == victimGroupID || attackerGroupID == killerGroupID) {
				if (verboseLogging) {
					error("FRS: VictimThreatmap: " + attacker->getFirstName() + " in killer/victim group, skipping (handled in group pass)");
				}
				continue;
			}
		}

		// Infinity: Use SNAPSHOTTED victim zone+position — NOT live `victimRef->getZone()`.
		// This is the critical fix: live reads here race with `switchZone` to Rikers.
		if (attacker->getZone() != victimZoneAtKill) {
			if (verboseLogging) {
				error("FRS: VictimThreatmap: " + attacker->getFirstName() + " different zone, skipping" + loc(attacker));
			}
			continue;
		}
		if (!inRangeOfPos(attacker, victimPosAtKill, 128.f)) {
			if (verboseLogging) {
				error("FRS: VictimThreatmap: " + attacker->getFirstName() + " out of range, skipping" + loc(attacker));
			}
			continue;
		}

		// Infinity: Per-account dedup: keep the highest-ranked character from each account
		uint32 attackerAccountID = attackerGhost->getAccountID();
		int attackerRank = getRank(attacker);

		if (highestFrsRank.contains(attackerAccountID)) {
			ManagedReference<CreatureObject*> storedMember = highestFrsRank.get(attackerAccountID);
			if (storedMember != nullptr && attackerRank > getRank(storedMember)) {
				highestFrsRank.drop(attackerAccountID);
				highestFrsRank.put(attackerAccountID, attacker);
				if (verboseLogging) {
					error("FRS: VictimThreatmap: replaced " + storedMember->getFirstName() + " with " + attacker->getFirstName() + " (higher rank) for account " + String::valueOf(attackerAccountID));
				}
			}
		} else {
			highestFrsRank.put(attackerAccountID, attacker);
			if (verboseLogging) {
				error("FRS: VictimThreatmap: added " + attacker->getFirstName() + " to highestFrsRank, account " + String::valueOf(attackerAccountID));
			}
		}

		killerGroupPower += getPowerRating(attacker);
	}

	// Infinity: ── Pass 1B (sync): killer's group → power + payout candidates ─────────
	// Each eligible group member contributes power AND becomes a payout candidate
	// with full snapshot state. The distribute phase iterates this list using only
	// the snapshotted fields, never re-reading live member state.
	ManagedReference<GroupObject*> killerGroup = killer->getGroup();
	if (killerGroup != nullptr) {
		for (int i = 0; i < killerGroup->getGroupSize(); i++) {
			ManagedReference<CreatureObject*> member = killerGroup->getGroupMember(i);
			if (member == nullptr) {
				continue;
			}

			ManagedReference<PlayerObject*> memberGhost = member->getPlayerObject();
			if (memberGhost == nullptr) {
				continue;
			}

			if (member == killerRef || !member->isPlayerCreature()) {
				continue;
			}

			// Infinity: Teffed bounty hunters count toward power even if not overt or same faction
			bool memberIsTeffedBH = (member->hasSkill("combat_bountyhunter_investigation_03") || member->hasSkill("mandalorian_master")) && memberGhost->hasBhTef();

			if (member->getFactionStatus() != FactionStatus::OVERT && !memberIsTeffedBH) {
				if (verboseLogging) {
					error("FRS: KillerGroup power: " + member->getFirstName() + " not overt and not teffed BH, skipping" + loc(member));
				}
				continue;
			}

			if (isSameFaction(victimRef, member) && !memberIsTeffedBH) {
				if (verboseLogging) {
					error("FRS: KillerGroup power: " + member->getFirstName() + " same faction as victim, skipping" + loc(member));
				}
				continue;
			}

			// Infinity: Zone/range/building checks (with rikers and stadium exceptions).
			// Snapshot member's zone+pos at the same kill-time moment for the candidate.
			ManagedReference<Zone*> memberZone = member->getZone();
			Vector3 memberPos = member->getWorldPosition();
			bool rikers = isRikersAt(member);
			bool memberInStadium = member->getScreenPlayState("stadium:insideStadium") & 1;

			if (!rikers && memberZone != victimZoneAtKill) {
				if (verboseLogging) {
					error("FRS: KillerGroup power: " + member->getFirstName() + " different zone, skipping" + loc(member));
				}
				continue;
			}

			if (!rikers && !memberInStadium && !inRangeOfPos(member, victimPosAtKill, 128.f)) {
				if (verboseLogging) {
					error("FRS: KillerGroup power: " + member->getFirstName() + " out of range, skipping" + loc(member));
				}
				continue;
			}

			ManagedReference<BuildingObject*> building = cast<BuildingObject*>(member->getRootParent());
			bool inPrivate = (building != nullptr && building->isPrivateStructure());
			if (inPrivate) {
				if (verboseLogging) {
					error("FRS: KillerGroup power: " + member->getFirstName() + " in private building, skipping" + loc(member));
				}
				continue;
			}

			// Infinity: Per-account dedup
			uint32 memberAccountID = memberGhost->getAccountID();
			int memberRank = getRank(member);

			if (highestFrsRank.contains(memberAccountID)) {
				ManagedReference<CreatureObject*> storedMember = highestFrsRank.get(memberAccountID);
				if (storedMember != nullptr && memberRank > getRank(storedMember)) {
					highestFrsRank.drop(memberAccountID);
					highestFrsRank.put(memberAccountID, member);
					if (verboseLogging) {
						error("FRS: KillerGroup: replaced " + storedMember->getFirstName() + " with " + member->getFirstName() + " (higher rank) for account " + String::valueOf(memberAccountID));
					}
				}
			} else {
				highestFrsRank.put(memberAccountID, member);
				if (verboseLogging) {
					error("FRS: KillerGroup: added " + member->getFirstName() + " to highestFrsRank, account " + String::valueOf(memberAccountID));
				}
			}

			killerGroupSize++;
			float memberPower = getPowerRating(member);
			killerGroupPower += memberPower;

			// Infinity: ── Build full snapshot for the distribute phase ───────────────
			// Every flag the distribute phase will check is captured here. The async
			// lambda iterates this list using only these fields — no live reads.
			FRSGroupPayoutCandidate candidate;
			candidate.creature = member;
			candidate.zoneAtKill = memberZone;
			candidate.posAtKill = memberPos;
			candidate.accountID = memberAccountID;
			candidate.rank = memberRank;
			candidate.power = memberPower;
			candidate.isFRSKnight = member->hasSkill("force_rank_light_novice") || member->hasSkill("force_rank_dark_novice");
			candidate.isOvert = (member->getFactionStatus() == FactionStatus::OVERT);
			candidate.isMandoBH = member->hasSkill("combat_bountyhunter_investigation_03") || member->hasSkill("mandalorian_master");
			candidate.hasBhTef = memberGhost->hasBhTef();
			candidate.sameFactionAsVictim = isSameFaction(victimRef, member);
			candidate.sameFactionAsKiller = isSameFaction(killerRef, member);
			candidate.inStadium = memberInStadium;
			candidate.rikersCooldownActive = !member->checkCooldownRecovery("rikers_timeout");
			candidate.inPrivateBuilding = inPrivate;
			candidate.inDuelWithVictim = CombatManager::instance()->areInDuel(member, victimRef);
			killerGroupCandidates.add(candidate);
		}
	}

	// Infinity: ── Pass 2A (sync): killer's threatmap → victim-side power ─────────────
	// Non-group defenders (people who damaged the killer) contribute to the victim's
	// side power. Same eligibility model as Pass 1A but referenced against the killer.

	if (verboseLogging) {
		error("FRS: Killer threatMap size = " + String::valueOf(killerThreatCopy->size()));
	}

	for (int i = 0; i < killerThreatCopy->size(); ++i) {
		ThreatMapEntry* entry = &killerThreatCopy->elementAt(i).getValue();
		ManagedReference<TangibleObject*> attackerTano = killerThreatCopy->elementAt(i).getKey();

		ManagedReference<CreatureObject*> attacker = nullptr;
		if (attackerTano != nullptr) {
			attacker = attackerTano->asCreatureObject();
		}

		if (entry == nullptr || attacker == nullptr || attacker == killerRef || attacker == victimRef) {
			continue;
		}

		if (attacker->isPet()) {
			victimGroupPower += 0.5f;
			continue;
		}

		if (!attacker->isPlayerCreature()) {
			continue;
		}

		ManagedReference<PlayerObject*> attackerGhost = attacker->getPlayerObject();
		if (attackerGhost == nullptr) {
			continue;
		}

		bool attackerIsTeffedBH = (attacker->hasSkill("combat_bountyhunter_investigation_03") || attacker->hasSkill("mandalorian_master")) && attackerGhost->hasBhTef();

		if (verboseLogging) {
			error("FRS: KillerThreatmap: checking participant [" + attacker->getFirstName() + "]" + loc(attacker));
		}

		// Infinity: Skip group members — handled in Pass 2B
		if (attacker->isGrouped()) {
			int attackerGroupID = attacker->getGroupID();
			if (attackerGroupID == victimGroupID || attackerGroupID == killerGroupID) {
				if (verboseLogging) {
					error("FRS: KillerThreatmap: " + attacker->getFirstName() + " in killer/victim group, skipping");
				}
				continue;
			}
		}

		// Infinity: Must be overt or teffed BH to count
		if (attacker->getFactionStatus() != FactionStatus::OVERT && !attackerIsTeffedBH) {
			if (verboseLogging) {
				error("FRS: KillerThreatmap: " + attacker->getFirstName() + " not overt and not teffed BH, skipping" + loc(attacker));
			}
			continue;
		}

		// Infinity: Must be opposing faction (or teffed BH exception)
		if (isSameFaction(killerRef, attacker) && !attackerIsTeffedBH) {
			if (verboseLogging) {
				error("FRS: KillerThreatmap: " + attacker->getFirstName() + " same faction as killer, skipping" + loc(attacker));
			}
			continue;
		}

		// Infinity: Use SNAPSHOTTED killer zone+position
		if (attacker->getZone() != killerZoneAtKill) {
			if (verboseLogging) {
				error("FRS: KillerThreatmap: " + attacker->getFirstName() + " different zone, skipping" + loc(attacker));
			}
			continue;
		}

		if (!inRangeOfPos(attacker, killerPosAtKill, 128.f)) {
			if (verboseLogging) {
				error("FRS: KillerThreatmap: " + attacker->getFirstName() + " out of range, skipping" + loc(attacker));
			}
			continue;
		}

		uint32 attackerAccountID = attackerGhost->getAccountID();

		if (!victimAccounts.contains(attackerAccountID)) {
			victimAccounts.add(attackerAccountID);
			if (verboseLogging) {
				error("FRS: KillerThreatmap: added account " + String::valueOf(attackerAccountID) + " (" + attacker->getFirstName() + ") to victimAccounts");
			}
		}

		victimGroupPower += getPowerRating(attacker);
	}

	// Infinity: ── Pass 2B (sync): victim's group → power ────────────────────────────
	ManagedReference<GroupObject*> victimGroup = victim->getGroup();
	if (victimGroup != nullptr) {
		for (int i = 0; i < victimGroup->getGroupSize(); i++) {
			ManagedReference<CreatureObject*> member = victimGroup->getGroupMember(i);
			if (member == nullptr) {
				continue;
			}

			ManagedReference<PlayerObject*> memberGhost = member->getPlayerObject();
			if (memberGhost == nullptr) {
				continue;
			}

			if (member == victimRef || !member->isPlayerCreature()) {
				continue;
			}

			bool memberIsTeffedBH = (member->hasSkill("combat_bountyhunter_investigation_03") || member->hasSkill("mandalorian_master")) && memberGhost->hasBhTef();

			if (member->getFactionStatus() != FactionStatus::OVERT && !memberIsTeffedBH) {
				if (verboseLogging) {
					error("FRS: VictimGroup: " + member->getFirstName() + " not overt and not teffed BH, skipping" + loc(member));
				}
				continue;
			}

			if (isSameFaction(killerRef, member) && !memberIsTeffedBH) {
				if (verboseLogging) {
					error("FRS: VictimGroup: " + member->getFirstName() + " same faction as killer, skipping" + loc(member));
				}
				continue;
			}

			ManagedReference<Zone*> memberZone = member->getZone();
			bool rikers = isRikersAt(member);
			bool memberInStadium = member->getScreenPlayState("stadium:insideStadium") & 1;

			if (!rikers && memberZone != victimZoneAtKill) {
				if (verboseLogging) {
					error("FRS: VictimGroup: " + member->getFirstName() + " different zone, skipping" + loc(member));
				}
				continue;
			}

			if (!rikers && !memberInStadium && !inRangeOfPos(member, killerPosAtKill, 128.f)) {
				if (verboseLogging) {
					error("FRS: VictimGroup: " + member->getFirstName() + " out of range, skipping" + loc(member));
				}
				continue;
			}

			ManagedReference<BuildingObject*> building = cast<BuildingObject*>(member->getRootParent());
			if (building != nullptr && building->isPrivateStructure()) {
				if (verboseLogging) {
					error("FRS: VictimGroup: " + member->getFirstName() + " in private building, skipping" + loc(member));
				}
				continue;
			}

			uint32 memberAccountID = memberGhost->getAccountID();

			if (!victimAccounts.contains(memberAccountID)) {
				victimAccounts.add(memberAccountID);
				if (verboseLogging) {
					error("FRS: VictimGroup: added account " + String::valueOf(memberAccountID) + " (" + member->getFirstName() + ") to victimAccounts");
				}
			}

			victimGroupSize++;
			victimGroupPower += getPowerRating(member);
		}
	}

	// Infinity: ─── SCHEDULE: dispatch the formula+distribute work to a TaskWorker ────
	// All eligibility filtering complete. The async lambda is now a pure function of:
	//   - scalar snapshots (victimZoneAtKill, killerZoneAtKill, victimPosAtKill, etc.)
	//   - killerGroupCandidates (Vector of full-state snapshots)
	//   - killerGroupPower / killerGroupSize
	//   - victimGroupPower / victimGroupSize
	//   - highestFrsRank (per-account dedup table)
	//   - victimAccounts (opposition-side accounts)
	// No live reads of zone, position, faction, cooldown, skill, or duel state inside.

	Core::getTaskManager()->executeTask([=] () {

		// Infinity: snapshot-aware location helper (uses captured zone+pos, immune to teleport)
		auto locSnap = [](Zone* z, const Vector3& p) -> String {
			String zn = (z != nullptr) ? z->getZoneName() : String("?");
			return " zone=" + zn + " pos=(" + String::valueOf((int)p.getX()) + "," + String::valueOf((int)p.getY()) + ")";
		};

		if (verboseLogging) {
			error("FRS: Async task started for kill of " + victimRef->getFirstName() + locSnap(victimZoneAtKill, victimPosAtKill)
			      + " by " + killerRef->getFirstName() + locSnap(killerZoneAtKill, killerPosAtKill));
		}

		// =====================================================================
		// Infinity: PHASE 3: CALCULATE AWARD (pure math from captured snapshots)
		// =====================================================================

		if (verboseLogging) {
			error("FRS: Kill summary: victim=" + victimRef->getFirstName() + locSnap(victimZoneAtKill, victimPosAtKill) + " rank=" + String::valueOf(victimRank)
				+ " killer=" + killerRef->getFirstName() + locSnap(killerZoneAtKill, killerPosAtKill)
				+ " attackPower=" + String::valueOf(killerGroupPower) + " attackSize=" + String::valueOf(killerGroupSize)
				+ " defendPower=" + String::valueOf(victimGroupPower) + " defendSize=" + String::valueOf(victimGroupSize));
		}

		// Infinity: ── Base award from victim's combat level, skills, and rank ──────────
		int award = 1000;  // Infinity: Base payout for any PvP kill

		if (victimLevel >= 15) {
			award += 2000;  // Infinity: Combat toon bonus (CL15+)
			if (verboseLogging) {
				error("FRS: Award: +2000 combat toon bonus = " + String::valueOf(award));
			}
		}

		if (victimIsMando) {
			award += 1000;  // Infinity: Mandalorian bonus
			if (verboseLogging) {
				error("FRS: Award: +1000 Mandalorian bonus = " + String::valueOf(award));
			}
		}

		if (victimIsKnight) {
			award += 2000;  // Infinity: Jedi Knight bonus
			if (verboseLogging) {
				error("FRS: Award: +2000 Knight bonus = " + String::valueOf(award));
			}
		}

		if (victimRank > 0) {
			int rankBonus = victimRank * 500;  // Infinity: 500 per FRS rank
			award += rankBonus;
			if (verboseLogging) {
				error("FRS: Award: +" + String::valueOf(rankBonus) + " rank bonus = " + String::valueOf(award));
			}
		}

		// Infinity: ── Power ratio adjustment ──────────────────────────────────────────
		// Compares total attacking power vs defending power. Higher ratio = fewer
		// attackers relative to defenders = more reward. Capped at 0.75 min ratio.
		float adjustedVictimPower = Math::max(victimGroupPower, 0.25f);
		float powerRatio = killerGroupPower / adjustedVictimPower;

		if (verboseLogging) {
			error("FRS: Power ratio = " + String::valueOf(powerRatio) + " (attacker=" + String::valueOf(killerGroupPower) + " / defender=" + String::valueOf(adjustedVictimPower) + ")");
		}

		int awardBase = award * 0.10;  // Infinity: 10% guaranteed minimum
		int awardVariable = award * 0.90;   // Infinity: 90% scaled by power ratio
		int modifiedAward = awardBase + (int)(awardVariable / Math::max(powerRatio, 0.75f));

		if (verboseLogging) {
			error("FRS: modifiedAward after power ratio = " + String::valueOf(modifiedAward));
		}

		// Infinity: ── Stadium penalty (configurable via stadiumFrsMultiplier) ─────────
		// Default 0.75 (25% reduction). Set in player_manager.lua, runtime override:
		//   /server playermanager setstadiumfrsmultiplier <float>
		if (inStadium) {
			modifiedAward *= stadiumFrsMultiplier;
			if (verboseLogging) {
				error("FRS: modifiedAward after stadium penalty (mult=" + String::valueOf(stadiumFrsMultiplier) + ") = " + String::valueOf(modifiedAward));
			}
		}

		// Infinity: ── Group size penalty (scales with attacker count) ─────────────────
		int finalAward = modifiedAward;

		if (killerGroupSize > 1) {
			int gSize = Math::min(20, killerGroupSize);
			float groupPenalty = 0;

			if (gSize >= 2 && gSize <= 10) {
				groupPenalty = gSize * 0.05f;
			} else if (gSize >= 11) {
				groupPenalty = 0.50f + (gSize - 10) * 0.025f;
			}

			if (verboseLogging) {
				error("FRS: Group size penalty = " + String::valueOf(groupPenalty * 100) + "% (size=" + String::valueOf(gSize) + ")");
			}

			finalAward *= (1 - groupPenalty);

			// Infinity: Floor: group penalty never reduces below 25% of pre-penalty value
			if (finalAward < (int)(modifiedAward * 0.25)) {
				finalAward = modifiedAward * 0.25;
				if (verboseLogging) {
					error("FRS: Group penalty floored at 25%, finalAward = " + String::valueOf(finalAward));
				}
			}
		}

		// Infinity: ── Overwhelming odds bonus (attacker outnumbered 3:1+) ─────────────
		if (powerRatio <= 0.33f) {
			int bonusAward = modifiedAward * 0.25f * (1.0f / powerRatio - 1.0f);
			bonusAward = Math::min(bonusAward, modifiedAward);  // Infinity: Cap at 1x base
			finalAward += bonusAward;
			if (verboseLogging) {
				error("FRS: Overwhelming odds bonus = " + String::valueOf(bonusAward) + ", finalAward = " + String::valueOf(finalAward));
			}
		}

		// =====================================================================
		// Infinity: PHASE 4: DISTRIBUTE AWARDS
		// =====================================================================
		// Pay the killer, eligible group members, and deduct from victim. All
		// eligibility values come from the kill-time snapshot — no live reads.

		bool awardAllowed = true;

		if (sameAccount) {
			if (verboseLogging) {
				error("FRS: Killer same account as victim, no award");
			}
			killerRef->sendSystemMessage("You cannot receive FRS XP for this kill because the victim is a character from your account.");
			awardAllowed = false;
		}

		if (awardAllowed && !killerIsFRSKnight) {
			if (verboseLogging) {
				error("FRS: Killer not FRS knight, no award");
			}
			awardAllowed = false;
		}

		if (awardAllowed && !killerIsOvert) {
			if (verboseLogging) {
				error("FRS: Killer not overt, no award");
			}
			awardAllowed = false;
		}

		if (awardAllowed && killerSameFactionAsVictim) {
			if (verboseLogging) {
				error("FRS: Killer same faction as victim, no award");
			}
			awardAllowed = false;
		}

		if (awardAllowed && killerInDuel) {
			if (verboseLogging) {
				error("FRS: Killer in duel with victim, no award");
			}
			awardAllowed = false;
		}

		if (awardAllowed && victimHasBhTef) {
			if (verboseLogging) {
				error("FRS: Victim has BH tef, no FRS earned");
			}
			killerRef->sendSystemMessage(victimRef->getFirstName() + " has recently been hunted by a bounty hunter and is not currently worth FRS Experience.");
			awardAllowed = false;
		}

		// Infinity: Overwhelming power (4:1+): no FRS for attacker, no FRS lost for defender
		if (powerRatio >= 4.0f) {
			if (verboseLogging) {
				error("FRS: Overwhelming power ratio (" + String::valueOf(powerRatio) + "), no FRS granted or lost");
			}
			killerRef->sendSystemMessage("Your side won with an overwhelming power ratio, so no FRS XP granted for killing " + victimRef->getFirstName() + ".");
			victimRef->sendSystemMessage("You lost against overwhelming odds, so no FRS XP was deducted.");
			awardAllowed = false;
		}
		// Note: stadium-with-zero-multiplier already early-returns from
		// calculateFRS() before this lambda was scheduled — no in-lambda gate needed here.

		// Infinity: ─── victim-loss gate (separate from awardAllowed) ──────────────────────
		// awardAllowed gates the killer/group PAYOUT. Victim FRS loss is gated separately
		// so a non-FRS overt killer (no payout) can still cost the FRS victim FRS XP.
		bool victimLossAllowed;
		if (killerIsFRSKnight) {
			// FRS-ranked killer: victim loss stays IDENTICAL to current behavior —
			// gated by the same awardAllowed result. No change for FRS killers.
			victimLossAllowed = awardAllowed;
		} else {
			// Non-FRS killer: reached here only if OVERT (covert returned at Site 1).
			// Victim still pays the death cost. victimHasBhTef remains a hard
			// protection (per MrO: current behavior correct). powerRatio>=4 zero-out
			// is applied inside the deduction block itself, unchanged.
			victimLossAllowed = !victimHasBhTef;
		}

		if (awardAllowed) {

			// Infinity: ── Pay the killer ───────────────────────────────────────────────
			if (verboseLogging) {
				error("FRS: Paying killer " + killerRef->getFirstName() + locSnap(killerZoneAtKill, killerPosAtKill) + " rank=" + String::valueOf(killerRank) + " award=" + String::valueOf(finalAward));
			}

			if (finalAward > 0) {
				ManagedReference<CreatureObject*> killerPayRef = killerRef;
				Core::getTaskManager()->executeTask([killerPayRef, frsManager, finalAward] () {
					Locker locker(killerPayRef);
					Locker clocker(frsManager, killerPayRef);
					frsManager->adjustFrsExperience(killerPayRef, finalAward);
				}, "FrsExperienceAdjustLambdaTask");
			}

			// Infinity: ── Pay eligible group members (using pre-built snapshot) ──────
			// Iterate the candidate Vector built in Pass 1B. All zone/range/building/
			// faction checks were done at sync. Here we apply only the payout-only
			// filters using the snapshotted flags — never re-read live member state.

			if (verboseLogging && killerGroupCandidates.size() > 0) {
				error("FRS: Checking " + String::valueOf(killerGroupCandidates.size()) + " group candidates for payout");
			}

			for (int i = 0; i < killerGroupCandidates.size(); i++) {
				const FRSGroupPayoutCandidate& candidate = killerGroupCandidates.get(i);
				ManagedReference<CreatureObject*> member = candidate.creature;

				if (member == nullptr) {
					continue;
				}

				// Infinity: payout-only filters using snapshotted candidate flags
				if (!candidate.isFRSKnight) {
					if (verboseLogging) {
						error("FRS: GroupPayout: " + member->getFirstName() + " not FRS knight, skipping" + locSnap(candidate.zoneAtKill, candidate.posAtKill));
					}
					continue;
				}

				if (!candidate.isOvert) {
					if (verboseLogging) {
						error("FRS: GroupPayout: " + member->getFirstName() + " not overt, skipping" + locSnap(candidate.zoneAtKill, candidate.posAtKill));
					}
					continue;
				}

				if (candidate.sameFactionAsVictim) {
					if (verboseLogging) {
						error("FRS: GroupPayout: " + member->getFirstName() + " same faction as victim, skipping" + locSnap(candidate.zoneAtKill, candidate.posAtKill));
					}
					continue;
				}

				if (candidate.inDuelWithVictim) {
					if (verboseLogging) {
						error("FRS: GroupPayout: " + member->getFirstName() + " in duel with victim, skipping" + locSnap(candidate.zoneAtKill, candidate.posAtKill));
					}
					continue;
				}

				if (victimHasBhTef) {
					if (verboseLogging) {
						error("FRS: GroupPayout: " + member->getFirstName() + " skipped, victim has BH tef");
					}
					member->sendSystemMessage(victimRef->getFirstName() + " has recently been hunted by a bounty hunter and is not currently worth FRS Experience.");
					continue;
				}

				// Infinity: Account-level checks (using snapshotted accountID)
				if (!allowSameAccount && candidate.accountID == victimAccountID) {
					member->sendSystemMessage("You cannot receive FRS XP for this kill because the victim is a character from your account.");
					if (verboseLogging) {
						error("FRS: GroupPayout: " + member->getFirstName() + " same account as victim, skipping");
					}
					continue;
				}

				if (!allowSameAccount && (candidate.accountID == killerAccountID || (highestFrsRank.contains(candidate.accountID) && highestFrsRank.get(candidate.accountID) != member))) {
					member->sendSystemMessage("You can only receive FRS XP on one character per account per victim.");
					if (verboseLogging) {
						StringBuffer msg;
						msg << "FRS: GroupPayout: " << member->getFirstName() << " already paid to this account, skipping" << endl;
						msg << "memberAccountID=" << candidate.accountID << " killerAccountID=" << killerAccountID;
						if (highestFrsRank.contains(candidate.accountID)) {
							msg << " highestFrsRank OID=" << highestFrsRank.get(candidate.accountID)->getObjectID() << " member OID=" << member->getObjectID();
						}
						error(msg.toString());
					}
					continue;
				}

				if (!allowSameAccount && victimAccounts.contains(candidate.accountID)) {
					member->sendSystemMessage("You have a character fighting for the opposition.  No FRS awarded.");
					if (verboseLogging) {
						error("FRS: GroupPayout: " + member->getFirstName() + " has character on opposition side, skipping");
					}
					continue;
				}

				// Infinity: ── All checks passed — dispatch FRS XP award ────────────────
				if (verboseLogging) {
					error("FRS: GroupPayout: awarding " + String::valueOf(finalAward) + " to " + member->getFirstName() + locSnap(candidate.zoneAtKill, candidate.posAtKill) + " rank=" + String::valueOf(candidate.rank));
				}

				ManagedReference<CreatureObject*> memberRef = member;
				Core::getTaskManager()->executeTask([memberRef, frsManager, finalAward] () {
					Locker locker(memberRef);
					Locker clocker(frsManager, memberRef);
					frsManager->adjustFrsExperience(memberRef, finalAward);
				}, "FrsExperienceAdjustLambdaTask");
			}
		}

		// Infinity: ── Deduct FRS XP from victim ────────────────────────────────────
		// Gated by victimLossAllowed (NOT awardAllowed) so a non-FRS overt killer
		// still costs the FRS victim FRS XP even though no killer/group payout ran.
		if (victimLossAllowed) {
			int deductionXp = -1 * modifiedAward * 0.65;  // Infinity: 65% of modified award charged to defender

			if (verboseLogging) {
				error("FRS: Victim deduction = " + String::valueOf(deductionXp) + " (65% of " + String::valueOf(modifiedAward) + ")");
			}

			// Infinity: Overwhelming power ratio: no FRS lost for victim
			if (powerRatio >= 4.0f) {
				deductionXp = 0;
				if (verboseLogging) {
					error("FRS: Overwhelming odds protection, no FRS lost for " + victimRef->getFirstName());
				}
			}

			if (deductionXp < 0) {
				if (verboseLogging) {
					error("FRS: Deducting " + String::valueOf(-deductionXp) + " FRS XP from " + victimRef->getFirstName() + locSnap(victimZoneAtKill, victimPosAtKill));
				}

				ManagedReference<CreatureObject*> victimPayRef = victimRef;
				Core::getTaskManager()->executeTask([victimPayRef, frsManager, deductionXp] () {
					Locker locker(victimPayRef);
					Locker clocker(frsManager, victimPayRef);
					frsManager->adjustFrsExperience(victimPayRef, deductionXp);
				}, "FrsExperienceAdjustLambdaTask");
			}
		}

		if (verboseLogging) {
			error("FRS: Async calculation complete for kill of " + victimRef->getFirstName());
		}

	}, "CalculateFRSTask");
}

void PlayerManagerImplementation::unlockPadawanForTesting(CreatureObject* player) {
	PlayerObject* ghost = player->getPlayerObject();

	if (ghost == nullptr) {
		return;
	}

	if (player->hasSkill("force_title_jedi_rank_02")) {
		player->sendSystemMessage("You already have Jedi Padawan skill box.");
		return;
	}

	SkillManager* skillManager = SkillManager::instance();

	int glowyBadgeIds[] = { 12, 14, 15, 16, 17, 19, 20, 21, 23, 30, 38, 39, 71, 105, 106, 107 };

	for (int i = 0; i < 16; i++) {
		ghost->awardBadge(glowyBadgeIds[i]);
	}

	SkillManager::instance()->surrenderAllSkills(player, true, false);

	Lua* lua = DirectorManager::instance()->getLuaInstance();

	Reference<LuaFunction*> luaPadawanTesting = lua->createFunction("FsIntro", "completeVillageIntroFrog", 0);
	*luaPadawanTesting << player;

	luaPadawanTesting->callFunction();

	String branches[] = {
			"force_sensitive_combat_prowess_ranged_accuracy",
			"force_sensitive_combat_prowess_ranged_speed",
			"force_sensitive_combat_prowess_melee_accuracy",
			"force_sensitive_combat_prowess_melee_speed",
			"force_sensitive_enhanced_reflexes_ranged_defense",
			"force_sensitive_enhanced_reflexes_melee_defense"
		};

	for (int i = 0; i < 6; i++) {
		String branch = branches[i];
		player->setScreenPlayState("VillageUnlockScreenPlay:" + branch, 2);
		skillManager->awardSkill(branch + "_04", player, true, true, true);
	}

	luaPadawanTesting = lua->createFunction("FsOutro", "completeVillageOutroFrog", 0);
	*luaPadawanTesting << player;

	luaPadawanTesting->callFunction();

	luaPadawanTesting = lua->createFunction("JediTrials", "completePadawanForTesting", 0);
	*luaPadawanTesting << player;

	luaPadawanTesting->callFunction();

}

void PlayerManagerImplementation::validateBadges(PlayerObject* ghost, bool recalculateBadgeCount) {
	if (ghost == nullptr) {
		error("ghost is nullptr");
		return;
	}

    //
    //Infinity:  Reworked both of these functions to check for missing accumulation/exploration badges each time since the switch statements sometimes missed them
    //

    BadgeList* badgeList = BadgeList::instance();

    if (badgeList == nullptr) {
		error("badgeList is nullptr");
		return;
	}

    uint8 numBadges = 0;

    if (recalculateBadgeCount) {
        for (uint8 i = 0; i <= 250; ++i) { //Infinity:  Allow up to 251 badges
            if (ghost->hasBadge(i)) {
                numBadges++;
            }
        }
        ghost->setNumBadges(numBadges);
    }
    else {
        numBadges = ghost->getNumBadges();
    }

    Vector<int> intervals = {5, 10, 25, 50, 75, 100, 125, 150, 175, 200, 225, 250}; // Accumulation intervals

    for (int i = 0; i < intervals.size(); ++i) {
        int currentInterval = intervals.get(i);

        const Badge* accumulationBadge = badgeList->get("count_" + String::valueOf(currentInterval));

        if (accumulationBadge != nullptr) {
            unsigned int accumulationBadgeID = accumulationBadge->getIndex();

            if (currentInterval > numBadges) {
                if (ghost->hasBadge(accumulationBadgeID)) {
                    revokeBadge(ghost, accumulationBadge);
                }
            }
            else if (!ghost->hasBadge(accumulationBadgeID)) {
                awardBadge(ghost, accumulationBadge);
            }
        }
    }

    uint8 explorationBadgeCount = ghost->getBadgeTypeCount(static_cast<uint8>(Badge::EXPLORATION));
    Vector<int> awardIntervals = {10, 20, 30, 40, 45, 55, 65, 75}; // Exploration badge intervals

    for (int i = 0; i < awardIntervals.size(); ++i) {
        int currentInterval = awardIntervals.get(i);

        const Badge* explorationBadge = badgeList->get("bdg_exp_" + String::valueOf(currentInterval) + "_badges");

        if (explorationBadge != nullptr) {
            unsigned int explorationBadgeID = explorationBadge->getIndex();

            if (currentInterval > explorationBadgeCount) {
                if (ghost->hasBadge(explorationBadgeID)) {
                    revokeBadge(ghost, explorationBadge);
                }
            }
            else if (!ghost->hasBadge(explorationBadgeID)) {
                awardBadge(ghost, explorationBadge);
            }
        }
    }
}

bool PlayerManagerImplementation::checkExistentNameInDatabase(const String& firstName) {
	// Infinity: Stub - name uniqueness is already handled by the character creation system
	return false;
}