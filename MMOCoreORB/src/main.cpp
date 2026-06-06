/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions. */

#ifndef PLATFORM_WIN
#include "CoreProcess.h"
#endif

#include "server/ServerCore.h"
#include "server/zone/managers/collision/NavMeshManager.h"
#include "server/zone/managers/director/DirectorManager.h"
#include "server/zone/managers/object/ObjectManager.h"
#include "server/OfflineTools.h"

#ifdef COMPILE_CORE3_TESTS
#include "tests/TestCore.h"

#include "gtest/gtest.h"
#endif

int main(int argc, char* argv[]) {
	System::setStreamBuffer(stdout, nullptr);

	if (argc) {
		StackTrace::setBinaryName(argv[0]);
	} else {
		Logger::console.warning("empty argv list");
	}

	static int ret = 0;

	try {
		SortedVector<String> arguments;
		arguments.setNoDuplicateInsertPlan();

		for (int i = 1; i < argc; ++i) {
			String currentArg = argv[i];
			arguments.put(currentArg.toLowerCase());
		}

		if (arguments.contains("testscreenplays")) {
			DirectorManager::DEBUG_MODE = 1;

			ConfigManager::instance()->loadConfigData();

			DirectorManager::instance()->info("Testing screen plays...", true);

			auto elapsed = Timer().run([]() { DirectorManager::instance()->getLuaInstance(); });

			DirectorManager::instance()->info(true) << "Done in " << elapsed / 1000000 << "ms";
		} else if (arguments.contains("service")) {
#ifndef PLATFORM_WIN
			while (true) {
				CoreProcess core(arguments);
				core.start();

				core.wait();
			}
#endif
#ifdef COMPILE_CORE3_TESTS
		} else if (arguments.contains("rununittests")) {
			TestCore core;
			core.info("Running unit tests...", true);

			testing::InitGoogleTest(&argc, argv);

			ret = RUN_ALL_TESTS();

			_exit(ret);

#endif
		} else if (arguments.contains("dumpnavmeshestofile")) {
			NavMeshManager::instance()->info("Dumping nav meshes to files...", true);

			NavMeshManager::instance()->dumpMeshesToFiles();

		// Infinity: Offline database maintenance tools
		} else if (arguments.contains("removeadk")) {
			ret = OfflineTools::removeADK();
			_exit(ret);

		} else if (arguments.contains("purgeinvalidcrc")) {
			// Check for deletesingle <OID> subcommand
			bool doDeleteSingle = arguments.contains("deletesingle");

			if (doDeleteSingle) {
				uint64 targetOID = 0;

				for (int i = 1; i < argc; ++i) {
					String arg = argv[i];

					if (arg != "purgeinvalidcrc" && arg != "deletesingle") {
						try {
							targetOID = UnsignedLong::valueOf(arg);
						} catch (...) {
							printf("Invalid OID: %s\n", arg.toCharArray());
							_exit(1);
						}
					}
				}

				if (targetOID == 0) {
					printf("Usage: ./core3 purgeinvalidcrc deletesingle <OID>\n");
					_exit(1);
				}

				ret = OfflineTools::deleteSingleObject(targetOID);
			} else {
				bool doDelete = arguments.contains("delete");
				ret = OfflineTools::purgeInvalidCRC(doDelete);
			}

			_exit(ret);

		} else if (arguments.contains("purgeships")) {
			bool doDelete = arguments.contains("delete");
			ret = OfflineTools::purgeShips(doDelete);
			_exit(ret);

		} else if (arguments.contains("showobject")) {
			ret = OfflineTools::showObject(argc, argv);
			_exit(ret);

		} else if (arguments.contains("purgeability")) {
			String abilityName;
			bool doDelete = arguments.contains("delete");

			for (int i = 1; i < argc; ++i) {
				String arg = argv[i];
				String lower = arg.toLowerCase();

				if (lower == "purgeability" || lower == "delete")
					continue;

				abilityName = arg; // preserve original case
				break;
			}

			if (abilityName.isEmpty()) {
				printf("Usage: ./core3 purgeAbility <abilityName> [delete]\n");
				_exit(1);
			}

			ret = OfflineTools::purgeAbility(abilityName, doDelete);
			_exit(ret);

		} else if (arguments.contains("reconcileaccountids")) {
			bool doFix = arguments.contains("fix");
			bool doResetOrphans = arguments.contains("resetorphans");
			ret = OfflineTools::reconcileAccountIDs(doFix, doResetOrphans);
			_exit(ret);

		} else if (arguments.contains("convert")) {
			// Infinity: Database conversion and navmesh rebuild
			// Database conversion has moved to the standalone ./dbconvert tool.
			// Navmesh rebuild still requires ServerCore, so it stays here.

			String subCmd;
			String extraArg;
			for (int i = 1; i < argc; ++i) {
				String arg = argv[i];
				String lower = arg.toLowerCase();
				if (lower == "convert")
					continue;
				if (subCmd.isEmpty()) {
					subCmd = lower;
				} else if (extraArg.isEmpty()) {
					extraArg = lower;
				}
			}

			// ── NAVMESH — delegate to ServerCore navmesh mode ──
			if (subCmd == "navmesh") {
				if (!extraArg.isEmpty())
					arguments.put("navmeshzone=" + extraArg);
				arguments.put("navmeshmode");

				ServerCore core(false, arguments);
				core.start();
				_exit(0);
			}

			// Everything else has moved to ./dbconvert
			System::out << "Database conversion has moved to the standalone tool." << endl;
			System::out << endl;
			System::out << "Usage:" << endl;
			System::out << "  ./dbconvert all              - convert all databases" << endl;
			System::out << "  ./dbconvert <database>       - convert single database" << endl;
			System::out << "  ./dbconvert sample <db>      - convert 1 record per class type" << endl;
			System::out << "  ./dbconvert hashonly [db]    - hash fix only (skip round-trip)" << endl;
			System::out << "  ./dbconvert clean            - reset BDB LSNs" << endl;
			System::out << endl;
			System::out << "  ./core3 convert navmesh [zone]  - rebuild navmeshes (still in core3)" << endl;
			_exit(1);

		} else {
			bool truncateData = arguments.contains("clean");

			ServerCore core(truncateData, arguments);
			core.start();
		}

	} catch (const Exception& e) {
		e.printStackTrace();
	} catch (...) {
		System::err << "unreported exception caught main()" << endl;
	}

	pthread_exit(&ret);

	return 0;
}
