/*
 				Copyright <SWGEmu>
		See file COPYING for copying conditions. */

#include "ServerCore.h"

#include <type_traits>
#include <dirent.h>

#include "db/MySqlDatabase.h"
#include "db/ServerDatabase.h"
#include "db/MantisDatabase.h"

#include "server/chat/ChatManager.h"
#include "server/login/LoginServer.h"
#include "server/login/account/AccountManager.h"
#include "system/lang/SignalException.h"
#ifdef WITH_SWGREALMS_API
#include "server/login/SWGRealmsAPI.h"
#endif // WITH_SWGREALMS_API
#include "server/login/VPNScreener.h"
#include "ping/PingServer.h"
#include "status/StatusServer.h"
#include "web/RESTServer.h"
#include "server/zone/ZoneServer.h"

#include "server/zone/managers/object/ObjectManager.h"
#include "engine/db/ObjectDatabase.h"
#include "engine/db/ObjectDatabaseManager.h"
#include "templates/manager/TemplateManager.h"
#include "server/zone/managers/player/PlayerManager.h"
#include "server/zone/managers/director/DirectorManager.h"
#include "server/zone/managers/collision/NavMeshManager.h"
#include "server/zone/managers/name/NameManager.h"
#include "server/zone/managers/frs/FrsManager.h"
#include "server/zone/objects/creature/commands/BotQueryCommand.h"

#include "server/zone/QuadTree.h"
#include "server/zone/Octree.h"

#include "engine/core/MetricsManager.h"
#include "engine/service/ServiceThread.h"
#include "engine/lua/LuaPanicException.h"

#include "server/zone/managers/statistics/StatisticsManager.h"
#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/objects/creature/CreatureObject.h"

ManagedReference<ZoneServer*> ServerCore::zoneServerRef = nullptr;
SortedVector<String> ServerCore::arguments;
bool ServerCore::truncateAllData = false;
ServerCore* ServerCore::instance = nullptr;

namespace coredetail {
	class ConsoleReaderService final : public ServiceThread {
		ServerCore* core;

	public:
		ConsoleReaderService(ServerCore* serverCoreInstance);

		bool inputAvailable() const;

		void run() override;
	};

#ifdef PLATFORM_WIN
	static const char* EngineConfigName = "core3engine_windows";
#else
	static const char* EngineConfigName = "core3engine";
#endif
}

ServerCore::ServerCore(bool truncateDatabases, const SortedVector<String>& args) :
		Core("log/core3.log", coredetail::EngineConfigName, LogLevel::LOG), Logger("Core") {
	orb = nullptr;

	loginServer = nullptr;
	zoneServerRef = nullptr;
	statusServer = nullptr;
	pingServer = nullptr;
#ifndef WITH_SWGREALMS_API
	database = nullptr;
#endif // !WITH_SWGREALMS_API
	mantisDatabase = nullptr;
#ifdef WITH_REST_API
	restServer = nullptr;
#endif // WITH_REST_API
#if WITH_SWGREALMS_API
	swgRealmsAPI = nullptr;
#endif // WITH_SWGREALMS_API

	truncateAllData = truncateDatabases;
	arguments = args;

	instance = this;

	setLogLevel(Logger::INFO);

	configManager = ConfigManager::instance();
	metricsManager = MetricsManager::instance();

	features = nullptr;

	handleCmds = true;

	initializeCoreContext();
}

void ServerCore::registerConsoleCommmands() {
	debug() << "registering console commands...";

	consoleCommands.setNoDuplicateInsertPlan();

	const auto addCommand = [this](auto name, auto lambda) {
		consoleCommands.put(name, lambda);
	};

	addCommand("exit", [this](const String& arguments) -> CommandResult {
		ZoneServer* zoneServer = zoneServerRef.getForUpdate();

		if (zoneServer != nullptr) {
			ChatManager* chatManager = zoneServer->getChatManager();
			chatManager->broadcastGalaxy(nullptr,
					"Server is shutting down NOW!");
		}

		return SHUTDOWN;
	});

	addCommand("logQuadtree", [this](const String& arguments) -> CommandResult {
		QuadTree::setLogging(!QuadTree::doLog());

		return SUCCESS;
	});

	addCommand("logOctree", [this](const String& arguments) -> CommandResult {
		Octree::setLogging(!Octree::doLog());

		return SUCCESS;
	});

	addCommand("info", [this](const String& arguments) -> CommandResult {
		//TaskManager::instance()->printInfo();

		ZoneServer* zoneServer = zoneServerRef.getForUpdate();

		if (loginServer != nullptr)
			loginServer->printInfo();

		if (zoneServer != nullptr)
			zoneServer->printInfo();

		if (pingServer != nullptr)
			pingServer->printInfo();

		return SUCCESS;
	});

	addCommand("lock", [this](const String& arguments) -> CommandResult {
		ZoneServer* zoneServer = zoneServerRef.getForUpdate();

		if (zoneServer != nullptr)
			zoneServer->setServerStateLocked();

		return SUCCESS;
	});

	addCommand("unlock", [this](const String& arguments) -> CommandResult {
		ZoneServer* zoneServer = zoneServerRef.getForUpdate();

		if (zoneServer != nullptr)
			zoneServer->setServerStateOnline();

		return SUCCESS;
	});

	addCommand("icap", [this](const String& arguments) -> CommandResult {
		ZoneServer* zoneServer = zoneServerRef.getForUpdate();

		if (zoneServer != nullptr)
			zoneServer->changeUserCap(50);

		return SUCCESS;
	});

	addCommand("dcap", [this](const String& arguments) -> CommandResult {
		ZoneServer* zoneServer = zoneServerRef.getForUpdate();

		if (zoneServer != nullptr)
			zoneServer->changeUserCap(-50);

		return SUCCESS;
	});

	addCommand("fixQueue", [this](const String& arguments) -> CommandResult {
		ZoneServer* zoneServer = zoneServerRef.getForUpdate();

		if (zoneServer != nullptr)
			zoneServer->fixScheduler();

		return SUCCESS;
	});

	addCommand("save", [this](const String& arguments) -> CommandResult {
		int flags = ObjectManager::SAVE_DELTA;

		if (!arguments.contains("delta") || arguments.contains("full")) {
			flags = ObjectManager::SAVE_FULL;
		}

		if (arguments.contains("debug")) {
			flags |= ObjectManager::SAVE_DEBUG;
		}

		if (arguments.contains("report")) {
			flags |= ObjectManager::SAVE_REPORT;
		}

		if (arguments.contains("dump")) {
			flags |= ObjectManager::SAVE_DUMP | ObjectManager::SAVE_FULL;
		}

		if (arguments.contains("json")) {
			flags |= ObjectManager::SAVE_JSON | ObjectManager::SAVE_FULL;
		}

		ObjectManager::instance()->createBackup(flags);

		return SUCCESS;
	});

	addCommand("reserialize", [this](const String& arguments) -> CommandResult {
		ObjectDatabaseManager* dbManager = ObjectDatabaseManager::instance();
		ObjectManager* objManager = ObjectManager::instance();

		if (arguments.isEmpty()) {
			System::out << "Usage: reserialize <database> | all [reset]" << endl;
			System::out << "Logs: log/<db>.converted, log/<db>.error, log/<db>.progress" << endl;
			return ERROR;
		}

		StringTokenizer tokenizer(arguments);
		String dbName;
		bool resetProgress = false;

		if (tokenizer.hasMoreTokens())
			tokenizer.getStringToken(dbName);

		if (tokenizer.hasMoreTokens()) {
			String option;
			tokenizer.getStringToken(option);
			if (option == "reset")
				resetProgress = true;
		}

		Vector<String> databasesToProcess;

		if (dbName == "all") {
			// Scan databases directory for all .db files
			DIR* dir = opendir("databases");
			if (dir != nullptr) {
				struct dirent* entry;
				while ((entry = readdir(dir)) != nullptr) {
					String filename = entry->d_name;
					if (filename.endsWith(".db") && filename.length() > 3) {
						String dbNameFromFile = filename.subString(0, filename.length() - 3);
						// Skip environment files
						if (!dbNameFromFile.beginsWith("__") && !dbNameFromFile.beginsWith("log."))
							databasesToProcess.add(dbNameFromFile);
					}
				}
				closedir(dir);
			}
			System::out << "Found " << databasesToProcess.size() << " databases" << endl;
		} else {
			databasesToProcess.add(dbName);
		}

		const uint64 LOG_ROTATION_SIZE = 1000000;

		for (int d = 0; d < databasesToProcess.size(); d++) {
			String currentDb = databasesToProcess.get(d);
			ObjectDatabase* database = dbManager->loadObjectDatabase(currentDb, false);

			if (database == nullptr)
				continue;

			// Count total objects first
			uint64 totalObjects = 0;
			{
				ObjectDatabaseIterator countIter(database);
				uint64 countKey;
				while (countIter.getNextKey(countKey))
					totalObjects++;
			}

			// Log file paths
			String logDir = "log/";
			String progressFile = logDir + currentDb + ".progress";
			String convertedBase = logDir + currentDb + ".converted";
			String errorBase = logDir + currentDb + ".error";

			// Check for resume point
			uint64 resumeFromKey = 0;
			uint64 resumeProcessed = 0;
			uint64 resumeErrors = 0;
			int convertedFileNum = 1;
			int errorFileNum = 1;

			if (!resetProgress) {
				FILE* progressFp = fopen(progressFile.toCharArray(), "r");
				if (progressFp != nullptr) {
					char line[256];
					if (fgets(line, sizeof(line), progressFp))
						sscanf(line, "%llu %llu %llu %d %d", &resumeFromKey, &resumeProcessed, &resumeErrors, &convertedFileNum, &errorFileNum);
					fclose(progressFp);
				}
			}

			// Open log files
			char convertedPath[512], errorPath[512];
			snprintf(convertedPath, sizeof(convertedPath), "%s_%03d.log", convertedBase.toCharArray(), convertedFileNum);
			snprintf(errorPath, sizeof(errorPath), "%s_%03d.log", errorBase.toCharArray(), errorFileNum);

			FILE* convertedFp = fopen(convertedPath, "a");
			FILE* errorFp = fopen(errorPath, "a");

			if (convertedFp == nullptr || errorFp == nullptr) {
				if (convertedFp) fclose(convertedFp);
				if (errorFp) fclose(errorFp);
				continue;
			}

			System::out << "starting: " << currentDb << endl;

			ObjectDatabaseIterator iterator(database);
			uint64 key;
			uint64 processed = resumeProcessed;
			uint64 errors = resumeErrors;
			uint64 current = 0;
			uint64 convertedInFile = 0;
			uint64 errorsInFile = 0;
			bool pastResumePoint = (resumeFromKey == 0);
			Time startTime;

			while (iterator.getNextKey(key)) {
				current++;

				if (!pastResumePoint) {
					if (key == resumeFromKey)
						pastResumePoint = true;
					continue;
				}

				String objectType = "Unknown";
				String objectName = "";
				String ownerInfo = "";
				String positionInfo = "";

				try {
					Reference<DistributedObjectStub*> object = objManager->loadPersistentObject(key);

					if (object != nullptr) {
						ManagedObject* managed = dynamic_cast<ManagedObject*>(object.get());
						if (managed != nullptr) {
							Locker locker(managed);
							objectType = managed->_getClassName();

							SceneObject* scene = dynamic_cast<SceneObject*>(managed);
							if (scene != nullptr) {
								objectName = scene->getDisplayedName();
								positionInfo = String::valueOf(scene->getPositionX()) + "," +
									String::valueOf(scene->getPositionZ()) + "," +
									String::valueOf(scene->getPositionY());

								CreatureObject* owner = scene->getParentRecursively(SceneObjectType::PLAYERCREATURE).castTo<CreatureObject*>();
								if (owner != nullptr)
									ownerInfo = owner->getFirstName();
							}

							managed->_setUpdated(true);

							fprintf(convertedFp, "%llu|%s|%s|%s|%s\n", key,
								objectType.toCharArray(), objectName.toCharArray(),
								ownerInfo.toCharArray(), positionInfo.toCharArray());

							processed++;
							convertedInFile++;

							if (convertedInFile >= LOG_ROTATION_SIZE) {
								fclose(convertedFp);
								convertedFileNum++;
								snprintf(convertedPath, sizeof(convertedPath), "%s_%03d.log", convertedBase.toCharArray(), convertedFileNum);
								convertedFp = fopen(convertedPath, "a");
								convertedInFile = 0;
							}
						}
					} else {
						fprintf(errorFp, "%llu|%s|LOAD_FAILED\n", key, objectType.toCharArray());
						errors++;
						errorsInFile++;
					}
				} catch (const Exception& e) {
					fprintf(errorFp, "%llu|%s|%s|%s|EXCEPTION|%s\n", key,
						objectType.toCharArray(), objectName.toCharArray(),
						ownerInfo.toCharArray(), e.getMessage().toCharArray());
					errors++;
					errorsInFile++;
				} catch (...) {
					fprintf(errorFp, "%llu|%s|UNKNOWN_EXCEPTION\n", key, objectType.toCharArray());
					errors++;
					errorsInFile++;
				}

				if (errorsInFile >= LOG_ROTATION_SIZE) {
					fclose(errorFp);
					errorFileNum++;
					snprintf(errorPath, sizeof(errorPath), "%s_%03d.log", errorBase.toCharArray(), errorFileNum);
					errorFp = fopen(errorPath, "a");
					errorsInFile = 0;
				}

				// Update display every 10 items
				if ((processed + errors) % 10 == 0) {
					FILE* progressFp = fopen(progressFile.toCharArray(), "w");
					if (progressFp) {
						fprintf(progressFp, "%llu %llu %llu %d %d\n", key, processed, errors, convertedFileNum, errorFileNum);
						fclose(progressFp);
					}
					fflush(convertedFp);
					fflush(errorFp);

					// Calculate ETA
					Time now;
					uint64 elapsedMs = now.miliDifference(startTime);
					uint64 done = processed + errors - resumeProcessed - resumeErrors;
					uint64 remaining = totalObjects - current;

					String eta = "calculating...";
					if (done > 0 && elapsedMs > 0) {
						uint64 etaSeconds = (elapsedMs * remaining) / (done * 1000);
						uint64 hours = etaSeconds / 3600;
						uint64 minutes = (etaSeconds % 3600) / 60;
						uint64 seconds = etaSeconds % 60;

						char etaBuf[64];
						if (hours > 0)
							snprintf(etaBuf, sizeof(etaBuf), "%lluh %llum %llus", hours, minutes, seconds);
						else if (minutes > 0)
							snprintf(etaBuf, sizeof(etaBuf), "%llum %llus", minutes, seconds);
						else
							snprintf(etaBuf, sizeof(etaBuf), "%llus", seconds);
						eta = etaBuf;
					}

					printf("\r%llu of %llu :: ETA = %s :: ERRORS = %llu :: SUCCESS = %llu          ",
						current, totalObjects, eta.toCharArray(), errors, processed);
					fflush(stdout);
				}
			}

			fclose(convertedFp);
			fclose(errorFp);

			FILE* progressFp = fopen(progressFile.toCharArray(), "w");
			if (progressFp) {
				fprintf(progressFp, "COMPLETE %llu %llu %d %d\n", processed, errors, convertedFileNum, errorFileNum);
				fclose(progressFp);
			}

			printf("\r%llu of %llu :: COMPLETE :: ERRORS = %llu :: SUCCESS = %llu          \n",
				totalObjects, totalObjects, errors, processed);
		}

		System::out << "done. run 'save full' to commit." << endl;
		return SUCCESS;
	});

	addCommand("re", [this](const String& arguments) -> CommandResult {
		auto handler = consoleCommands.get("reserialize");
		return handler(arguments);
	});

	addCommand("help", [this](const String& arguments) -> CommandResult {
		System::out << "available commands: ";

		for (const auto& entry : consoleCommands) {
			System::out << entry.getKey() << "  ";
		}

		System::out << endl;

		return SUCCESS;
	});

	addCommand("chars", [this](const String& arguments) -> CommandResult {
		ZoneServer* zoneServer = zoneServerRef.getForUpdate();
		uint32 num = 0;

		try {
			num = UnsignedInteger::valueOf(arguments);
		} catch (const Exception& e) {
			System::out << "invalid noumber of concurrent chars per account specified" << endl;

			return ERROR;
		}

		if (num != 0) {
			PlayerManager* pMan = zoneServer->getPlayerManager();
			pMan->setOnlineCharactersPerAccount(num);

			System::out << "changed max concurrent chars per account to: " << num << endl;
		}

		return SUCCESS;
	});

	addCommand("lookupcrc", [this](const String& arguments) -> CommandResult {
		uint32 crc = 0;
		try {
			crc = UnsignedInteger::valueOf(arguments);
		} catch (const Exception& e) {
			System::out << "invalid crc number expected dec";

			return ERROR;
		}

		if (crc != 0) {
			String file = TemplateManager::instance()->getTemplateFile(
					crc);

			System::out << "result: " << file << endl;
		}

		return SUCCESS;
	});

	addCommand("loglevel", [this](const String& arguments) -> CommandResult {
		int level = 0;
		try {
			level = Integer::valueOf(arguments);
		} catch (const Exception& e) {
			System::out << "invalid log level" << endl;

			return ERROR;
		}

		if (level >= Logger::NONE && level <= Logger::DEBUG) {
			Logger::setGlobalFileLogLevel(static_cast<Logger::LogLevel>(level));

			System::out << "global log level changed to: " << level << endl;
		}

		return SUCCESS;
	});

	addCommand("rev", [this](const String& arguments) -> CommandResult {
		System::out << ConfigManager::instance()->getRevision() << endl;

		return SUCCESS;
	});

	addCommand("broadcast", [this](const String& arguments) -> CommandResult {
		ZoneServer* zoneServer = zoneServerRef.getForUpdate();

		if (zoneServer != nullptr) {
			ChatManager* chatManager = zoneServer->getChatManager();
			chatManager->broadcastGalaxy(nullptr, arguments);
			info(true) << "Console broadcasted: " << arguments;
		}

		return SUCCESS;
	});

	addCommand("shutdown", [this](const String& arguments) -> CommandResult {
		int flags = ShutdownFlags::DEFAULT;
		ZoneServer* zoneServer = zoneServerRef.getForUpdate();
		int minutes = 1;

		if (arguments.contains("dev")) {
			flags |= ShutdownFlags::SKIP_BACKUP;
			minutes = 0;
		} else {
			try {
				minutes = UnsignedInteger::valueOf(arguments);
			} catch (const Exception& e) {
				System::out << "Usage: shutdown {minutes} {json} {fast}" << endl;
				System::out << "       shutdown dev" << endl;

				return ERROR;
			}
		}

		if (arguments.contains("fast")) {
			flags |= ShutdownFlags::FAST;
		}

		if (arguments.contains("json")) {
			flags |= ShutdownFlags::DUMP_JSON;
		}

		if (zoneServer != nullptr) {
			zoneServer->timedShutdown(minutes, flags);

			shutdownBlockMutex.lock();

			waitCondition.wait(&shutdownBlockMutex);

			shutdownBlockMutex.unlock();
		}

		return SHUTDOWN;
	});

	addCommand("playercleanup", [this](const String& arguments) -> CommandResult {
		ZoneServer* zoneServer = zoneServerRef.getForUpdate();

		if (zoneServerRef != nullptr) {
			ZoneServer* server = zoneServerRef.get();

			if (server != nullptr)
				server->getPlayerManager()->cleanupCharacters();
		}

		return SUCCESS;
	});

	addCommand("playercleanupstats", [this](const String& arguments) -> CommandResult {
		ZoneServer* zoneServer = zoneServerRef.getForUpdate();

		if (zoneServerRef != nullptr) {
			ZoneServer* server = zoneServerRef.get();

			if (server != nullptr)
				server->getPlayerManager()->getCleanupCharacterCount();
		}

		return SUCCESS;
	});

	addCommand("test", [this](const String& arguments) -> CommandResult {
		Lua* lua = DirectorManager::instance()->getLuaInstance();

		// create the lua function
		UniqueReference<LuaFunction*> func(lua->createFunction("Tests", arguments, 0));
		func->callFunction();

		return SUCCESS;
	});

	addCommand("reloadscreenplays", [this](const String& arguments) -> CommandResult {
		DirectorManager::instance()->reloadScreenPlays();

		return SUCCESS;
	});

	addCommand("reloadmanager", [this](const String& arguments) -> CommandResult {
		if (arguments == "name") {
			ZoneServer* server = zoneServerRef.get();

			if (server != nullptr)
				server->getNameManager()->loadConfigData(true);
		} else {
			System::out << "Invalid manager. Reloadable managers: name" << endl;
		}

		return SUCCESS;
	});

	addCommand("clearstats", [this](const String& arguments) -> CommandResult {
		Core::getTaskManager()->clearWorkersTaskStats();

		return SUCCESS;
	});

#ifdef COLLECT_TASKSTATISTICS
	addCommand("statsd", [this](const String& arguments) -> CommandResult {
		StringTokenizer argTokenizer(arguments);

		argTokenizer.setDelimiter(" ");

		String address;
		int port = 0;

		if (argTokenizer.hasMoreTokens())
			argTokenizer.getStringToken(address);

		if (argTokenizer.hasMoreTokens())
			port = argTokenizer.getIntToken();

		if (port) {
			MetricsManager::instance()->initializeStatsDConnection(address.toCharArray(), port);

			System::out << "metrics manager connection set to" << address << ":" << port << endl;
		} else {
			System::out << "invalid port or address" << endl;
		}

		return SUCCESS;
	});

	addCommand("samplerate", [this](const String& arguments) -> CommandResult {
		try {
			int rate = UnsignedInteger::valueOf(arguments);

			Core::getTaskManager()->setStatsDTaskSampling(rate);

			System::out << "statsd sampling rate changed to " << rate << endl;
		} catch (const Exception& e) {
			System::out << "invalid statsd sampling rate" << endl;

			return ERROR;
		}

		return SUCCESS;
	});

	addCommand("sampleratedb", [this](const String& arguments) -> CommandResult {
		try {
			int rate = UnsignedInteger::valueOf(arguments);

			Core::getTaskManager()->setStatsDBdbSamplingRate(rate);

			System::out << "statsd berkeley db sampling rate changed to " << rate << endl;
		} catch (const Exception& e) {
			System::out << "invalid statsd sampling rate" << endl;

			return ERROR;
		}

		return SUCCESS;
	});
#endif
	const auto pvpModeLambda = [this](const String& arguments) -> CommandResult {
		System::out << "PvpMode = " << ConfigManager::instance()->getPvpMode() << endl;

		return SUCCESS;
	};

	addCommand("getpvpmode", pvpModeLambda);
	addCommand("getpvp", pvpModeLambda);

	// Console equivalent of the in-game /botquery command. Dumps the same
	// structured [BOTQUERY] lines to stdout and writes the file under log/botquery/.
	// Usage: botquery player <firstname>
	addCommand("botquery", [this](const String& arguments) -> CommandResult {
		StringTokenizer tokens(arguments);

		if (!tokens.hasMoreTokens()) {
			System::out << "Usage: botquery player <firstname>" << endl;
			return ERROR;
		}

		String subcommand;
		tokens.getStringToken(subcommand);
		subcommand = subcommand.toLowerCase();

		if (subcommand != "player") {
			System::out << "Unknown subcommand: " << subcommand << " (available: player)" << endl;
			return ERROR;
		}

		if (!tokens.hasMoreTokens()) {
			System::out << "Usage: botquery player <firstname>" << endl;
			return ERROR;
		}

		String playerName;
		tokens.getStringToken(playerName);

		ZoneServer* zoneServer = zoneServerRef.getForUpdate();
		int result = BotQueryCommand::queryPlayerFromConsole(zoneServer, playerName);
		return result == 0 ? SUCCESS : ERROR;
	});

	const auto setPvpModeLambda = [this](const String& arguments) -> CommandResult {
		int num;

		try {
			if (arguments == "on") {
				num = 1;
			} else if (arguments == "off") {
				num = 0;
			} else {
				num = UnsignedInteger::valueOf(arguments);
			}

			if (num == 1) {
				ConfigManager::instance()->setPvpMode(true);
			} else {
				ConfigManager::instance()->setPvpMode(false);
			}

			info(true) << "console set new PvpMode = " << ConfigManager::instance()->getPvpMode();
		} catch (const Exception& e) {
			System::out << "invalid PvpMode: (0=off; 1=on)" << endl;

			return ERROR;
		}

		return SUCCESS;
	};

	addCommand("setpvpmode", setPvpModeLambda);
	addCommand("setpvp", setPvpModeLambda);

	const auto dumpConfigLambda = [this](const String& arguments) -> CommandResult {
		ConfigManager::instance()->dumpConfig(arguments == "all");

		return SUCCESS;
	};

	addCommand("dumpcfg", dumpConfigLambda);
	addCommand("dumpconfig", dumpConfigLambda);

	addCommand("clearlockout", [this](const String& arguments) -> CommandResult {
		if (arguments.isEmpty()) {
			System::out << "Usage: clearlockout <ip>" << endl;
			return ERROR;
		}

		auto* acctMgr = AccountManager::instance();
		if (acctMgr == nullptr) {
			System::out << "AccountManager not initialized." << endl;
			return ERROR;
		}

		acctMgr->clearFailedLogins(arguments.trim());
		System::out << "Cleared login lockout for IP " << arguments.trim() << endl;

		return SUCCESS;
	});

	addCommand("lockouts", [this](const String& arguments) -> CommandResult {
		auto* acctMgr = AccountManager::instance();
		if (acctMgr == nullptr) {
			System::out << "AccountManager not initialized." << endl;
			return ERROR;
		}

		acctMgr->dumpFailedLogins();
		return SUCCESS;
	});

#ifdef WITH_SWGREALMS_API
	const auto swgRealmsApiLambda = [this](const String& arguments) -> CommandResult {
		return SWGRealmsAPI::instance()->consoleCommand(arguments) ? SUCCESS : ERROR;
	};

	addCommand("swgrealms", swgRealmsApiLambda);
#endif // WITH_SWGREALMS_API

	// Infinity: emergency kill-switch for the VPN screener. Use when
	// vpnapi.io classifications are causing widespread legit-player blocks
	// or when investigating a screener-related outage. Usage:
	//   vpncheck            print current enabled flag
	//   vpncheck on         re-enable runtime check
	//   vpncheck off        flip the screener off — all logins ALLOW through
	//   vpncheck status     same as no-arg
	addCommand("vpncheck", [this](const String& arguments) -> CommandResult {
		StringTokenizer tokenizer(arguments);
		String subcmd;

		if (tokenizer.hasMoreTokens()) {
			tokenizer.getStringToken(subcmd);
			subcmd = subcmd.toLowerCase();
		}

		if (subcmd == "on" || subcmd == "enable") {
			VPNScreener::instance()->setEnabled(true);
			System::out << "VPNScreener: ON (logins screened against allowlist + cache + vpnapi.io)" << endl;
			return SUCCESS;
		}

		if (subcmd == "off" || subcmd == "disable") {
			VPNScreener::instance()->setEnabled(false);
			System::out << "VPNScreener: OFF — all logins will SKIP the screener." << endl;
			return SUCCESS;
		}

		if (subcmd.isEmpty() || subcmd == "status") {
			System::out << "VPNScreener: "
				<< (VPNScreener::instance()->isEnabled() ? "ON" : "OFF") << endl;
			return SUCCESS;
		}

		if (subcmd == "help") {
			System::out << "Usage:" << endl
				<< "\tvpncheck            - print current enabled flag (same as `status`)" << endl
				<< "\tvpncheck on         - re-enable VPN screener" << endl
				<< "\tvpncheck off        - emergency kill-switch — all logins skip screener" << endl
				<< "\tvpncheck status     - print current enabled flag" << endl;
			return SUCCESS;
		}

		System::out << "Unknown subcommand: " << subcmd << ". Try `vpncheck help`." << endl;
		return ERROR;
	});

	addCommand("toggleModifiedObjectsDump", [this](const String& arguments) -> CommandResult {
		DOBObjectManager::setDumpLastModifiedTraces(!DOBObjectManager::getDumpLastModifiedTraces());

		System::out << "dump last modified traces set to " << DOBObjectManager::getDumpLastModifiedTraces();

		return SUCCESS;
	});

	addCommand("runLuaFunction", [this](const String& arguments) -> CommandResult {
		StringTokenizer argTokenizer(arguments);

		argTokenizer.setDelimiter(":");

		String module;
		String function;

		if (argTokenizer.hasMoreTokens())
			argTokenizer.getStringToken(module);

		if (argTokenizer.hasMoreTokens())
			argTokenizer.getStringToken(function);

		if (module.isEmpty() || function.isEmpty()) {
			System::out << "Usage: runLuaFunction {module}:{function}" << endl;

			return ERROR;
		}

		System::out << "Attemping to run " << module << ":" << function << endl;

		Lua* lua = DirectorManager::instance()->getLuaInstance();
		lua_State* L = lua->getLuaState();
		CommandResult cmdResult = NOTFOUND;

		try {
			if (!lua->checkStack(0)) {
				error() << "Warning Lua Stack is not clean!";
			}

			UniqueReference<LuaFunction*> func(lua->createFunction(module, function, 1));

			while (argTokenizer.hasMoreTokens()) {
				String arg;
				argTokenizer.getStringToken(arg);
				*func << arg;
			}

			if (func->callFunction() == nullptr) {
				String errorMessage = lua_tostring(L, -1);

				lua_pop(L, 1);

				System::out << "Failed to runLuaFunction " << module << ":" << function << ": " << errorMessage << endl;

				cmdResult = ERROR;
			} else {
				lua_pop(L, 1);

				String result;

				if (lua_type(L, 0) == LUA_TSTRING) {
					result = lua_tostring(L, 0);
				} else {
					result = "<" + (String)lua_typename(L, lua_type(L, 0)) + ">";
				}

				System::out << "runLuaFunction " << module << ":" << function << ": result=[" << result << "]" << endl;

				cmdResult = SUCCESS;
			}
		} catch (const Exception& e) {
			System::out << "Exception in runLuaFunction " << module << ":" << function << " - " << e.getMessage() << endl << Lua::dumpStack(L);
		}

		return cmdResult;
	});

	debug() << "registered " << consoleCommands.size() << " console commands.";
}

ServerCore::~ServerCore() {
	finalizeContext();
}

class ZoneStatisticsTask: public Task {
	Reference<ZoneServer*> zoneServer;

public:
	ZoneStatisticsTask(ZoneServer* server) {
		zoneServer = server;
	}

	void run() {
		zoneServer->printInfo();
	}
};

void ServerCore::finalizeContext() {
	server::db::mysql::MySqlDatabase::finalizeLibrary();
}

void ServerCore::initializeCoreContext() {
	server::db::mysql::MySqlDatabase::initializeLibrary();

	class ThreadHook : public ThreadInitializer {
	public:
		void onThreadStart(Thread* thread) final {
			server::db::mysql::MySqlDatabase::onThreadStart();
		}

		void onThreadEnd(Thread* thread) final {
			server::db::mysql::MySqlDatabase::onThreadEnd();
		}
	};

	Thread::setThreadInitializer(new ThreadHook());
}

void ServerCore::signalShutdown(ShutdownFlags flags) {
	nextShutdownFlags = flags;

	shutdownBlockMutex.lock();

	waitCondition.broadcast(&shutdownBlockMutex);

	shutdownBlockMutex.unlock();
}

void ServerCore::initialize() {
	StatisticsManager::instance()->markCoreStart(Thread::getProcessID());

	info(true) << "Server start, pid: "
		<< Thread::getProcessID() << ", time: " << Time().getFormattedTime();

	processConfig();

	Logger::setGlobalFileLogger(configManager->getLogFile(), configManager->getRotateLogSizeMB(), configManager->getRotateLogAtStart());
	Logger::setGlobalFileJson(configManager->getJsonLogOutput());
	Logger::setGlobalFileLoggerSync(configManager->getSyncLogOutput());
	Logger::setGlobalFileLogLevel(static_cast<Logger::LogLevel>(configManager->getLogFileLevel()));

	registerConsoleCommmands();

	try {
		ObjectManager* objectManager = ObjectManager::instance();

#ifndef WITH_SWGREALMS_API
		database = new ServerDatabase(configManager);
#endif // !WITH_SWGREALMS_API

		mantisDatabase = new MantisDatabase(configManager);

		const String& orbaddr = configManager->getORBNamingDirectoryAddress();
		orb = DistributedObjectBroker::initialize(orbaddr,
				configManager->getORBNamingDirectoryPort());

		orb->setCustomObjectManager(objectManager);

		info() << "MetricsServer: " << configManager->shouldUseMetrics()
			<< " " << configManager->getMetricsHost() << " " << configManager->getMetricsPort();

		if (configManager->shouldUseMetrics()) {
			metricsManager->setGlobalPrefix(configManager->getMetricsPrefix());
			metricsManager->initializeStatsDConnection(
					configManager->getMetricsHost().toCharArray(),
					configManager->getMetricsPort());
		}

		if (configManager->getMakeLogin()) {
			loginServer = new LoginServer(configManager);
			loginServer->deploy("LoginServer");

			// Clear stale sessions on startup unless session persistence is enabled
			if (!ConfigManager::instance()->getLoginEnableSessionId()) {
				database->instance()->executeStatement("TRUNCATE TABLE sessions");
			}
		}

		if (configManager->getMakeZone()) {
			ZoneServer* zoneServer = new ZoneServer(configManager);
			zoneServer->deploy("ZoneServer");

			zoneServerRef = zoneServer;
		}

		if (configManager->getMakePing()) {
			pingServer = new PingServer();
		}

		if (configManager->getMakeStatus()) {
			statusServer = new StatusServer(configManager, zoneServerRef);
		}

#ifdef WITH_REST_API
		restServer = new server::web3::RESTServer();
		restServer->start();
#endif // WITH_REST_API

#if WITH_SWGREALMS_API
		if (ConfigManager::instance()->getString("Core3.Login.API.BaseURL", "").length() > 0) {
			swgRealmsAPI = SWGRealmsAPI::instance();
		}
#endif // WITH_SWGREALMS_API

		ZoneServer* zoneServer = zoneServerRef.get();

		NavMeshManager::instance()->initialize(configManager->getMaxNavMeshJobs(), zoneServer);

		if (zoneServer != nullptr) {
			int zonePort = configManager->getZoneServerPort();

			int zoneAllowedConnections =
					configManager->getZoneAllowedConnections();

			if (arguments.contains("deleteNavMeshes") && zoneServer != nullptr) {
				zoneServer->setShouldDeleteNavAreas(true);
			}

			ObjectDatabaseManager* dbManager =
					ObjectDatabaseManager::instance();
			dbManager->loadDatabases(truncateDatabases());

			int galaxyID = configManager->getZoneGalaxyID();

			try {
#ifndef WITH_SWGREALMS_API
				if (zonePort == 0) {
					const String query = "SELECT port FROM galaxy WHERE galaxy_id = "
								   + String::valueOf(galaxyID);
					UniqueReference<ResultSet*> result(database->instance()->executeQuery(query));

					if (result != nullptr && result->next()) {
						zonePort = result->getInt(0);
					}
				}
#else // WITH_SWGREALMS_API
				if (zonePort == 0) {
					auto swgRealmsAPI = SWGRealmsAPI::instance();

					if (swgRealmsAPI != nullptr) {
						auto galaxyOpt = swgRealmsAPI->getGalaxyEntry(galaxyID);

						if (galaxyOpt.has_value()) {
							zonePort = galaxyOpt.value().getPort();
						} else {
							error("Failed to load galaxy port for galaxy_id " + String::valueOf(galaxyID));
						}
					}
				}
#endif // WITH_SWGREALMS_API

#ifndef WITH_SWGREALMS_API
				database->instance()->executeStatement(
						"DELETE FROM characters_dirty WHERE galaxy_id = "
						+ String::valueOf(galaxyID));
#else // WITH_SWGREALMS_API
			auto swgRealmsAPI = SWGRealmsAPI::instance();
			if (swgRealmsAPI != nullptr) {
				String errorMessage;
				if (!swgRealmsAPI->rollbackCharactersBlocking(galaxyID, errorMessage)) {
					error() << "Failed to rollback uncommitted characters: " << errorMessage;
				}
			}
#endif // WITH_SWGREALMS_API
			} catch (const DatabaseException &e) {
				fatal(e.getMessage());
			}

			zoneServer->start(zonePort, zoneAllowedConnections);
		}

		if (statusServer != nullptr) {
			int statusPort = configManager->getStatusPort();
			int statusAllowedConnections =
					configManager->getStatusAllowedConnections();

			statusServer->start(statusPort, statusAllowedConnections);
		}

		if (pingServer != nullptr) {
			int pingPort = configManager->getPingPort();
			int pingAllowedConnections =
					configManager->getPingAllowedConnections();

			pingServer->start(pingPort, pingAllowedConnections);
		}

		if (loginServer != nullptr) {
			int loginPort = configManager->getLoginPort();
			int loginAllowedConnections =
					configManager->getLoginAllowedConnections();

			loginServer->start(loginPort, loginAllowedConnections);
		}

		ObjectManager::instance()->scheduleUpdateToDatabase();

		StatisticsManager::instance()->markCoreInitialized();

		info("initialized", true);

		System::flushStreams();

#if WITH_SWGREALMS_API
		if (ConfigManager::instance()->getString("Core3.Login.API.BaseURL", "").length() > 0) {
			if (configManager != nullptr) {
				swgRealmsAPI->notifyGalaxyStart(configManager->getZoneGalaxyID());
				swgRealmsAPI->scheduleMetricsPublish();
			}
		}
#endif // WITH_SWGREALMS_API

		if (arguments.contains("playercleanup") && zoneServer != nullptr) {
			zoneServer->getPlayerManager()->cleanupCharacters();
		}

		if (arguments.contains("playercleanupstats") && zoneServer != nullptr) {
			zoneServer->getPlayerManager()->getCleanupCharacterCount();
		}

		if (arguments.contains("shutdown")) {
			handleCmds = false;
		}

	} catch (const ServiceException& e) {
		shutdown();
	} catch (const DatabaseException& e) {
		fatal(e.getMessage());
	}
}

void ServerCore::run() {
	handleCommands();

	shutdown();
}

void ServerCore::shutdown() {
	info(true) << "shutting down server.. flags = "
		<< (nextShutdownFlags == ShutdownFlags::DEFAULT) << " DEFAULT"
		<< (nextShutdownFlags & ShutdownFlags::FAST) << " FAST"
		<< (nextShutdownFlags & ShutdownFlags::DUMP_JSON) << " DUMP_JSON"
		<< (nextShutdownFlags & ShutdownFlags::SKIP_BACKUP) << " SKIP_BACKUP"
		;

	handleCmds = false;

#ifdef WITH_REST_API
	if (restServer) {
		restServer->stop();

		delete restServer;
		restServer = nullptr;
	}
#endif // WITH_REST_API

	bool haveSave = false;

	ObjectManager* objectManager = ObjectManager::instance();

	if (objectManager->isObjectUpdateInProgress()) {
		haveSave = true;

		info(true) << "Shutdown waiting for in-progress save to complete...";

		while (objectManager->isObjectUpdateInProgress())
			Thread::sleep(500);
	}

	objectManager->cancelDeleteCharactersTask();
	objectManager->cancelUpdateModifiedObjectsTask();

	if (loginServer != nullptr) {
		loginServer->stop();
		loginServer = nullptr;
	}

	ZoneServer* zoneServer = zoneServerRef.get();

	if (zoneServer != nullptr) {
		zoneServer->setServerStateShuttingDown();

		Thread::sleep(2000);

		if (nextShutdownFlags & ShutdownFlags::FAST) {
			info(true) << "Skip disconnecting players";
		} else {
			info(true) << "Disconnecting all players";

			PlayerManager* playerManager = zoneServer->getPlayerManager();

			playerManager->stopOnlinePlayerLogTask();
			playerManager->disconnectAllPlayers();

			int count = 0;
			while (zoneServer->getConnectionCount() > 0 && count < 20) {
				Thread::sleep(500);
				count++;
			}

			info("All players disconnected", true);
		}
	}

	if (pingServer != nullptr) {
		pingServer->stop();
		pingServer = nullptr;
	}

	if (statusServer != nullptr) {
		statusServer->stop();
		statusServer = nullptr;
	}

	NavMeshManager::instance()->stop();

	Thread::sleep(5000);

	if (nextShutdownFlags & ShutdownFlags::SKIP_BACKUP) {
		info(true) << "Skipping database backup (dev mode)";
	} else {
		auto backupFlags = ObjectManager::SAVE_FULL | ObjectManager::SAVE_REPORT;

		if (nextShutdownFlags & ShutdownFlags::DUMP_JSON) {
			info(true) << "Backing up with JSON dump of in-ram objects.";

			backupFlags |= ObjectManager::SAVE_JSON;
		}

		objectManager->createBackup(backupFlags);

		while (objectManager->isObjectUpdateInProgress())
			Thread::sleep(500);

		info(true) << "database backup done";
	}

	objectManager->cancelUpdateModifiedObjectsTask();

	if (nextShutdownFlags & ShutdownFlags::SKIP_BACKUP) {
		info(true) << "Skipping zone clear (dev mode)";
	} else if (zoneServer != nullptr) {
		zoneServer->clearZones();
	}

	orb->shutdown();

	Core::shutdownTaskManager();

	if (zoneServer != nullptr) {
		zoneServer->stop();
		zoneServer = nullptr;
	}

	DistributedObjectDirectory* dir = objectManager->getLocalObjectDirectory();

	HashTable<uint64, Reference<DistributedObject*> > tbl;
	auto objects = dir->getDistributedObjectMap();
	auto objectsIterator = objects->iterator();
	typedef std::remove_reference<decltype(*objects)>::type ObjectsMapType;

	while (objectsIterator.hasNext()) {
		ObjectsMapType::key_type* key;
		ObjectsMapType::value_type* value;

		objectsIterator.getNextKeyAndValue(key, value);

		tbl.put(*key, *value);
	}

	objectManager->finalizeInstance();

#ifdef WITH_SWGREALMS_API
	if (swgRealmsAPI) {
		if (configManager != nullptr) {
			swgRealmsAPI->notifyGalaxyShutdown();
		}

		info(true) << "Finalize SWGRealmsAPI...";
		swgRealmsAPI->finalizeInstance();
		swgRealmsAPI = nullptr;
		info(true) << "Finalized SWGRealmsAPI...";
	}
#endif // WITH_SWGREALMS_API

	configManager = nullptr;
	metricsManager = nullptr;

#ifndef WITH_SWGREALMS_API
	if (database != nullptr) {
		delete database;
		database = nullptr;
	}
#endif // !WITH_SWGREALMS_API

	if (mantisDatabase != nullptr) {
		delete mantisDatabase;
		mantisDatabase = nullptr;
	}

	if (features != nullptr) {
		delete features;
		features = nullptr;
	}

	NetworkInterface::finalize();

	Logger::closeGlobalFileLogger();

	orb->finalizeInstance();

	zoneServerRef = nullptr;

	info("server closed", true);
}

ServerCore::CommandResult ServerCore::processConsoleCommand(const String& commandString) {
	CommandResult result = CommandResult::NOTFOUND;

	try {
		StringTokenizer tokenizer(commandString);

		String command, arguments;

		if (tokenizer.hasMoreTokens())
			tokenizer.getStringToken(command);

		if (tokenizer.hasMoreTokens())
			arguments = tokenizer.getRemainingString();

		auto it = consoleCommands.find(command);

		if (it != consoleCommands.npos) {
			result = consoleCommands.get(it)(arguments);
		} else {
			result = CommandResult::NOTFOUND;
		}
	} catch (const Exception& e) {
		error() << commandString << " EXCEPTION: " <<  e.getMessage();

		return CommandResult::ERROR;
	}

	return result;
}

void ServerCore::queueConsoleCommand(const String& commandString) {
	if (!handleCmds) {
		error() << "Ignoring queued command: " << commandString;
		return;
	}

	auto line = commandString + "\n";
	consoleCommandPipe.writeLine(line.toCharArray());
}

void ServerCore::handleCommands() {
	if (!handleCmds)
		return;

	consoleCommandPipe.create(false);

	auto reader = coredetail::ConsoleReaderService(instance);
	reader.start(true);

	while (handleCmds) {
		Thread::sleep(500);

		System::out << "\nREADY\n> " << flush;

		char line[256];

		auto len = consoleCommandPipe.readLine(line, sizeof(line));

		if (!len)
			continue;

		auto cmd = String(line).trim();

		if (cmd.isEmpty())
			continue;

		if (!handleCmds) {
			error() << "console command processing disabled, ignoring: " << cmd;
			break;
		}

		auto result = processConsoleCommand(cmd);

		if (result == CommandResult::SHUTDOWN)
			break;

		if (result == CommandResult::NOTFOUND)
			warning() << "unknown command (" << cmd << ")";

		System::flushStream(stdout);
	}

	reader.setRunning(false);

	Thread::yield();

	reader.join();

	consoleCommandPipe.close();

	info(true) << "Console Closed";
}

void ServerCore::processConfig() {
	if (!configManager->loadConfigData())
		warning("missing config file.. loading default values");
}

#ifndef WITH_SWGREALMS_API
int ServerCore::getSchemaVersion() {
	if (instance != nullptr && instance->database != nullptr)
		return instance->database->getSchemaVersion();

	return -1;
}
#endif // !WITH_SWGREALMS_API

coredetail::ConsoleReaderService::ConsoleReaderService(ServerCore* serverCoreInstance) : ServiceThread("ConsoleReader"), core(serverCoreInstance) {
}

bool coredetail::ConsoleReaderService::inputAvailable() const {
#ifndef PLATFORM_WIN
	struct timeval tv = {};
	tv.tv_sec = 1;
	tv.tv_usec = 0;

	fd_set fds;

	FD_ZERO(&fds);
	FD_SET(STDIN_FILENO, &fds);

	auto ret = select(STDIN_FILENO + 1, &fds, nullptr, nullptr, &tv);

	fatal(ret != -1) << "select on stdin failed";

	return FD_ISSET(STDIN_FILENO, &fds);
#else
	static auto stdinHandle = [this] () {
		auto handle = GetStdHandle(STD_INPUT_HANDLE);

		fatal(handle) << "GetStdHandle returned null stdin handle";

		return handle;
	} ();

	switch (WaitForSingleObject(stdinHandle, 1000)) {
	case WAIT_OBJECT_0:
		return true;
	default:
		return false;
	}
#endif
}

void coredetail::ConsoleReaderService::run() {
	setReady(true);

	while (doRun.get(std::memory_order_seq_cst)) {
		char* res = nullptr;

#ifndef PLATFORM_WIN
		char line[PIPE_BUF];
#else
		char line[256];
#endif

		if (!inputAvailable())
			continue;

		res = fgets(line, sizeof(line), stdin);

		if (!res)
			continue;

		auto cmd = String(line).trim();

		if (cmd.isEmpty())
			continue;

		core->queueConsoleCommand(cmd);
	}
}

