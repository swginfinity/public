/*
    Copyright <SWGEmu>
    See file COPYING for copying conditions.

    Infinity: /botquery — structured data dump for headless client / bot CSR
    Outputs [BOTQUERY] prefixed key=value lines for machine parsing.
    Usage: /botquery player <name>
*/

#ifndef BOTQUERYCOMMAND_H_
#define BOTQUERYCOMMAND_H_

#include "server/zone/objects/creature/commands/QueueCommand.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/objects/player/PlayerObject.h"
#include "server/zone/ZoneServer.h"
#include "server/zone/managers/player/PlayerManager.h"
#include "server/zone/managers/skill/SkillManager.h"
#include "server/zone/objects/creature/buffs/Buff.h"
#include "server/zone/objects/creature/buffs/BuffList.h"
#include "server/zone/objects/creature/variables/SkillModList.h"
#include "server/zone/objects/tangible/wearables/WearableObject.h"
#include "server/zone/objects/tangible/wearables/WearableContainerObject.h"
#include "server/zone/objects/tangible/weapon/WeaponObject.h"
#include "server/zone/managers/skill/SkillModManager.h"
#include "server/zone/Zone.h"
#include "templates/params/creature/CreatureState.h"
#include "templates/params/creature/CreaturePosture.h"
#include "templates/params/creature/CreatureLocomotion.h"

#include <fstream>
#include <sys/stat.h>

class BotQueryCommand : public QueueCommand {
public:
	BotQueryCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {
	}

	// Collects lines during a query, sends to player and writes to file.
	// When creature is null (console invocation), lines are printed to System::out instead.
	class QueryOutput {
		CreatureObject* creature;
		StringBuffer fileBuffer;
		String subcommand;
		String targetName;

	public:
		QueryOutput(CreatureObject* c, const String& sub, const String& target)
			: creature(c), subcommand(sub), targetName(target) {
		}

		void addLine(const String& line) {
			if (creature != nullptr) {
				creature->sendSystemMessage("[BOTQUERY] " + line);
			} else {
				System::out << "[BOTQUERY] " << line << endl;
			}
			fileBuffer << line << "\n";
		}

		void writeToFile() {
			mkdir("log/botquery", 0755);

			StringBuffer filename;
			filename << "log/botquery/" << subcommand << "_" << targetName << "_" << System::getMiliTime() << ".txt";

			String filenameStr = filename.toString();

			std::ofstream file(filenameStr.toCharArray());
			if (file.is_open()) {
				file << fileBuffer.toString().toCharArray();
				file.close();

				if (creature != nullptr) {
					creature->sendSystemMessage("[BOTQUERY] wrote=" + filenameStr);
				} else {
					System::out << "[BOTQUERY] wrote=" << filenameStr << endl;
				}
			}
		}
	};

	void sendLine(CreatureObject* creature, const String& line) const {
		creature->sendSystemMessage("[BOTQUERY] " + line);
	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {
		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		StringTokenizer args(arguments.toString());

		if (!args.hasMoreTokens()) {
			creature->sendSystemMessage("Usage: /botquery player <name>");
			return INVALIDPARAMETERS;
		}

		String subcommand;
		args.getStringToken(subcommand);
		subcommand = subcommand.toLowerCase();

		if (subcommand == "player") {
			return queryPlayer(creature, args);
		} else {
			creature->sendSystemMessage("Unknown subcommand: " + subcommand);
			creature->sendSystemMessage("Available: player");
			return INVALIDPARAMETERS;
		}

		return SUCCESS;
	}

	int queryPlayer(CreatureObject* admin, StringTokenizer& args) const {
		if (!args.hasMoreTokens()) {
			admin->sendSystemMessage("Usage: /botquery player <firstname>");
			return INVALIDPARAMETERS;
		}

		String playerName;
		args.getStringToken(playerName);

		auto zoneServer = server->getZoneServer();
		if (zoneServer == nullptr)
			return GENERALERROR;

		auto playerManager = zoneServer->getPlayerManager();
		if (playerManager == nullptr)
			return GENERALERROR;

		ManagedReference<CreatureObject*> player = playerManager->getPlayer(playerName);
		if (player == nullptr) {
			admin->sendSystemMessage("Player not found: " + playerName);
			return INVALIDTARGET;
		}

		Locker clocker(player, admin);

		QueryOutput out(admin, "player", playerName);
		dumpPlayerData(out, player);
		out.writeToFile();
		return SUCCESS;
	}

	// Runs the player dump from the core3 console (no admin creature).
	// Registered in ServerCore.cpp as `botquery player <firstname>`.
	static int queryPlayerFromConsole(ZoneServer* zoneServer, const String& playerName) {
		if (zoneServer == nullptr) {
			System::out << "[BOTQUERY] zoneServer is null" << endl;
			return GENERALERROR;
		}

		auto playerManager = zoneServer->getPlayerManager();
		if (playerManager == nullptr) {
			System::out << "[BOTQUERY] playerManager is null" << endl;
			return GENERALERROR;
		}

		ManagedReference<CreatureObject*> player = playerManager->getPlayer(playerName);
		if (player == nullptr) {
			System::out << "[BOTQUERY] Player not found: " << playerName << endl;
			return INVALIDTARGET;
		}

		Locker clocker(player);

		QueryOutput out(nullptr, "player", playerName);
		dumpPlayerData(out, player);
		out.writeToFile();
		return SUCCESS;
	}

	static void dumpPlayerData(QueryOutput& out, CreatureObject* player) {
		out.addLine("BEGIN player");
		out.addLine("name=" + player->getFirstName());
		out.addLine("objectID=" + String::valueOf(player->getObjectID()));

		// Online status
		auto ghost = player->getPlayerObject();
		if (ghost != nullptr) {
			out.addLine("online=" + String::valueOf(ghost->isOnline()));
			out.addLine("godMode=" + String::valueOf(ghost->hasGodMode()));
		}

		// Location
		out.addLine("zone=" + (player->getZone() != nullptr ? player->getZone()->getZoneName() : "none"));
		out.addLine("x=" + String::valueOf(player->getPositionX()));
		out.addLine("y=" + String::valueOf(player->getPositionY()));
		out.addLine("z=" + String::valueOf(player->getPositionZ()));
		out.addLine("parentID=" + String::valueOf(player->getParentID()));

		// Posture & Locomotion
		uint8 posture = player->getPosture();
		out.addLine("posture=" + String::valueOf(posture));
		out.addLine("postureName=" + CreaturePosture::postureToString(posture));

		uint8 locomotion = player->getLocomotion();
		out.addLine("locomotion=" + String::valueOf(locomotion));

		static const char* locoNames[] = {
			"standing", "sneaking", "walking", "running", "kneeling",
			"crouchSneaking", "crouchWalking", "prone", "crawling",
			"climbingStationary", "climbing", "hovering", "flying",
			"lyingDown", "sitting", "skillAnimating", "drivingVehicle",
			"ridingCreature", "knockedDown", "incapacitated", "dead", "blocking"
		};
		if (locomotion < 22) {
			out.addLine("locomotionName=" + String(locoNames[locomotion]));
		}
		out.addLine("dead=" + String::valueOf(player->isDead()));
		out.addLine("incapacitated=" + String::valueOf(player->isIncapacitated()));
		out.addLine("inCombat=" + String::valueOf(player->isInCombat()));

		// Full state bitmask (#1463)
		uint64 stateBitmask = player->getStateBitmask();
		out.addLine("stateBitmask=0x" + String::hexvalueOf(stateBitmask));

		struct StateInfo { uint64 bit; const char* name; };
		static const StateInfo allStates[] = {
			{ CreatureState::COVER,                    "cover" },
			{ CreatureState::COMBAT,                   "combat" },
			{ CreatureState::PEACE,                    "peace" },
			{ CreatureState::AIMING,                   "aiming" },
			{ CreatureState::ALERT,                    "alert" },
			{ CreatureState::BERSERK,                  "berserk" },
			{ CreatureState::FEIGNDEATH,               "feignDeath" },
			{ CreatureState::COMBATATTITUDEEVASIVE,    "combatAttitudeEvasive" },
			{ CreatureState::COMBATATTITUDENORMAL,     "combatAttitudeNormal" },
			{ CreatureState::COMBATATTITUDEAGGRESSIVE, "combatAttitudeAggressive" },
			{ CreatureState::TUMBLING,                 "tumbling" },
			{ CreatureState::RALLIED,                  "rallied" },
			{ CreatureState::STUNNED,                  "stunned" },
			{ CreatureState::BLINDED,                  "blinded" },
			{ CreatureState::DIZZY,                    "dizzy" },
			{ CreatureState::INTIMIDATED,              "intimidated" },
			{ CreatureState::IMMOBILIZED,              "immobilized" },
			{ CreatureState::FROZEN,                   "frozen" },
			{ CreatureState::SWIMMING,                 "swimming" },
			{ CreatureState::SITTINGONCHAIR,           "sittingOnChair" },
			{ CreatureState::CRAFTING,                 "crafting" },
			{ CreatureState::GLOWINGJEDI,              "glowingJedi" },
			{ CreatureState::MASKSCENT,                "maskScent" },
			{ CreatureState::POISONED,                 "poisoned" },
			{ CreatureState::BLEEDING,                 "bleeding" },
			{ CreatureState::DISEASED,                 "diseased" },
			{ CreatureState::ONFIRE,                   "onFire" },
			{ CreatureState::RIDINGMOUNT,              "ridingMount" },
			{ CreatureState::MOUNTEDCREATURE,          "mountedCreature" },
			{ CreatureState::PILOTINGSHIP,             "pilotingShip" },
			{ CreatureState::SHIPOPERATIONS,           "shipOperations" },
			{ CreatureState::SHIPGUNNER,               "shipGunner" },
			{ CreatureState::SHIPINTERIOR,             "shipInterior" },
			{ CreatureState::PILOTINGPOBSHIP,          "pilotingPobShip" },
		};

		for (const auto& s : allStates) {
			bool active = (stateBitmask & s.bit) != 0;
			out.addLine(String("state.") + s.name + "=" + (active ? "true" : "false"));
		}

		// HAM pools
		static const char* hamNames[] = {"health", "strength", "constitution", "action", "quickness", "stamina", "mind", "focus", "willpower"};
		for (int i = 0; i < 9; i++) {
			String prefix = String(hamNames[i]);
			out.addLine("ham." + prefix + ".current=" + String::valueOf(player->getHAM(i)));
			out.addLine("ham." + prefix + ".max=" + String::valueOf(player->getMaxHAM(i)));
			out.addLine("ham." + prefix + ".base=" + String::valueOf(player->getBaseHAM(i)));
			out.addLine("ham." + prefix + ".wounds=" + String::valueOf(player->getWounds(i)));
		}

		// Encumbrances (only 3: health=0, action=1, mind=2)
		static const char* encNames[] = {"health", "action", "mind"};
		for (int i = 0; i < 3; i++) {
			out.addLine("encumbrance." + String(encNames[i]) + "=" + String::valueOf(player->getEncumbrance(i)));
		}

		out.addLine("shockWounds=" + String::valueOf(player->getShockWounds()));

		// Faction
		out.addLine("faction=" + String::valueOf(player->getFaction()));
		out.addLine("factionStatus=" + String::valueOf(player->getFactionStatus()));

		// Species/level
		out.addLine("species=" + String::valueOf(player->getSpecies()));
		out.addLine("level=" + String::valueOf(player->getLevel()));

		// Force
		if (ghost != nullptr) {
			out.addLine("forcePower=" + String::valueOf(ghost->getForcePower()));
			out.addLine("forcePowerMax=" + String::valueOf(ghost->getForcePowerMax()));
		}

		// Buffs
		const auto* buffList = player->getBuffList();
		if (buffList != nullptr) {
			int buffCount = buffList->getBuffListSize();
			out.addLine("buffCount=" + String::valueOf(buffCount));

			for (int i = 0; i < buffCount; i++) {
				ManagedReference<Buff*> buff = buffList->getBuffByIndex(i);
				if (buff != nullptr) {
					String buffPrefix = "buff." + String::valueOf(i);
					out.addLine(buffPrefix + ".crc=" + String::valueOf(buff->getBuffCRC()));
					out.addLine(buffPrefix + ".duration=" + String::valueOf(buff->getBuffDuration()));
					out.addLine(buffPrefix + ".timeRemaining=" + String::valueOf(buff->getTimeLeft()));
				}
			}
		}

		// Skills
		auto* skillList = player->getSkillList();
		if (skillList != nullptr) {
			int skillCount = skillList->size();
			out.addLine("skillCount=" + String::valueOf(skillCount));

			StringBuffer skills;
			for (int i = 0; i < skillCount; i++) {
				auto skill = skillList->get(i);
				if (skill != nullptr) {
					if (i > 0) skills << ",";
					skills << skill->getSkillName();
				}
			}
			out.addLine("skills=" + skills.toString());
		}

		// Equipped wearables, weapons, and wearable containers — dumps each item's
		// template skill mods (from SharedTangibleObjectTemplate) and SEA skill mods
		// (from wearableSkillMods populated by applyAttachment). An overlap.<key>
		// line is emitted when the same stat is present on both sources — that is
		// the condition that forces compareMods to reshuffle WEARABLE vs TEMPLATE
		// buckets and fires "Wearable mods don't match" warnings on every equip.
		{
			int slotSize = player->getSlottedObjectsSize();
			int wearableCount = 0;
			SortedVector<uint64> seenObjects;
			seenObjects.setNoDuplicateInsertPlan();

			for (int i = 0; i < slotSize; i++) {
				ManagedReference<SceneObject*> slot = player->getSlottedObject(i);
				if (slot == nullptr)
					continue;

				ManagedReference<TangibleObject*> obj = slot.castTo<TangibleObject*>();
				if (obj == nullptr)
					continue;

				bool isWearable = obj->isWearableObject();
				bool isContainer = obj->isWearableContainerObject();
				bool isWeapon = obj->isWeaponObject();
				if (!isWearable && !isContainer && !isWeapon)
					continue;

				// Multi-slot items occupy several slot indices — dedupe by object id
				if (seenObjects.contains(obj->getObjectID()))
					continue;
				seenObjects.put(obj->getObjectID());

				String prefix = "wearable." + String::valueOf(wearableCount);
				out.addLine(prefix + ".oid=" + String::valueOf(obj->getObjectID()));
				out.addLine(prefix + ".displayedName=" + obj->getDisplayedName());
				out.addLine(prefix + ".type=" + String(isWearable ? "wearable" : (isContainer ? "container" : "weapon")));

				if (obj->getArrangementDescriptorSize() > 0) {
					const auto* arr = obj->getArrangementDescriptor(0);
					if (arr != nullptr && arr->size() > 0) {
						out.addLine(prefix + ".slot=" + arr->get(0));
					}
				}

				const VectorMap<String, int>* seaMods = nullptr;
				if (isWearable) {
					WearableObject* w = cast<WearableObject*>(obj.get());
					if (w != nullptr) {
						seaMods = w->getWearableSkillMods();
						out.addLine(prefix + ".maxSockets=" + String::valueOf(w->getMaxSockets()));
						out.addLine(prefix + ".remainingSockets=" + String::valueOf(w->getRemainingSockets()));
					}
				} else if (isContainer) {
					WearableContainerObject* w = cast<WearableContainerObject*>(obj.get());
					if (w != nullptr) {
						seaMods = w->getWearableSkillMods();
						out.addLine(prefix + ".remainingSockets=" + String::valueOf(w->getRemainingSockets()));
					}
				} else if (isWeapon) {
					WeaponObject* w = cast<WeaponObject*>(obj.get());
					if (w != nullptr) {
						seaMods = w->getWearableSkillMods();
					}
				}

				if (seaMods != nullptr) {
					for (int j = 0; j < seaMods->size(); j++) {
						out.addLine(prefix + ".sea." + seaMods->elementAt(j).getKey() + "=" + String::valueOf(seaMods->elementAt(j).getValue()));
					}
				}

				const VectorMap<String, int>* tplMods = obj->getTemplateSkillMods();
				if (tplMods != nullptr) {
					for (int j = 0; j < tplMods->size(); j++) {
						out.addLine(prefix + ".template." + tplMods->elementAt(j).getKey() + "=" + String::valueOf(tplMods->elementAt(j).getValue()));
					}
				}

				if (seaMods != nullptr && tplMods != nullptr) {
					for (int j = 0; j < seaMods->size(); j++) {
						const String& key = seaMods->elementAt(j).getKey();
						if (tplMods->contains(key)) {
							out.addLine(prefix + ".overlap." + key + "=sea:" + String::valueOf(seaMods->get(key)) + ",template:" + String::valueOf(tplMods->get(key)));
						}
					}
				}

				wearableCount++;
			}

			out.addLine("wearableCount=" + String::valueOf(wearableCount));
		}

		// Skill mod groups — raw contents of each bucket on the creature.
		// Used with the wearable.*.overlap lines above to verify the compareMods
		// WEARABLE/TEMPLATE bucket-split theory for the skillmod-mismatch warnings.
		{
			auto skillModMutex = player->getSkillModMutex();
			Locker skillModLocker(skillModMutex);

			SkillModList* skillModList = player->getSkillModList();
			if (skillModList != nullptr) {
				struct GroupInfo { uint32 type; const char* name; };
				static const GroupInfo groupsToDump[] = {
					{ (uint32)SkillModManager::PERMANENTMOD, "PERMANENTMOD" },
					{ (uint32)SkillModManager::TEMPLATE,     "TEMPLATE" },
					{ (uint32)SkillModManager::SKILLBOX,     "SKILLBOX" },
					{ (uint32)SkillModManager::WEARABLE,     "WEARABLE" },
					{ (uint32)SkillModManager::STRUCTURE,    "STRUCTURE" },
					{ (uint32)SkillModManager::CITY,         "CITY" },
					{ (uint32)SkillModManager::DROID,        "DROID" },
					{ (uint32)SkillModManager::BUFF,         "BUFF" },
					{ (uint32)SkillModManager::ABILITYBONUS, "ABILITYBONUS" },
				};

				for (const auto& g : groupsToDump) {
					const SkillModGroup* group = skillModList->getSkillModGroup(g.type);
					if (group == nullptr || group->size() == 0)
						continue;

					out.addLine("skillmodGroup." + String(g.name) + ".size=" + String::valueOf(group->size()));
					for (int i = 0; i < group->size(); i++) {
						out.addLine("skillmod." + String(g.name) + "." + group->elementAt(i).getKey() + "=" + String::valueOf(group->elementAt(i).getValue()));
					}
				}
			}
		}

		// Filling / credits
		if (ghost != nullptr) {
			out.addLine("foodFilling=" + String::valueOf(ghost->getFoodFilling()));
			out.addLine("foodFillingMax=" + String::valueOf(ghost->getFoodFillingMax()));
			out.addLine("drinkFilling=" + String::valueOf(ghost->getDrinkFilling()));
			out.addLine("drinkFillingMax=" + String::valueOf(ghost->getDrinkFillingMax()));
		}

		out.addLine("cashCredits=" + String::valueOf(player->getCashCredits()));
		out.addLine("bankCredits=" + String::valueOf(player->getBankCredits()));

		// Quest states (active and completed bit arrays)
		if (ghost != nullptr) {
			StringBuffer activeQuests;
			StringBuffer completedQuests;
			int activeCount = 0;
			int completedCount = 0;

			for (int i = 0; i < 256; i++) {
				if (ghost->hasActiveQuestBitSet(i)) {
					if (activeCount > 0) activeQuests << ",";
					activeQuests << i;
					activeCount++;
				}
				if (ghost->hasCompletedQuestsBitSet(i)) {
					if (completedCount > 0) completedQuests << ",";
					completedQuests << i;
					completedCount++;
				}
			}

			out.addLine("activeQuestCount=" + String::valueOf(activeCount));
			out.addLine("activeQuests=" + activeQuests.toString());
			out.addLine("completedQuestCount=" + String::valueOf(completedCount));
			out.addLine("completedQuests=" + completedQuests.toString());

			// ScreenPlay data — full dump of the screenPlayData VectorMap (every namespace/quest)
			String spData = ghost->getAllScreenPlayData();
			StringTokenizer spTok(spData);
			spTok.setDelimeter("\n");
			while (spTok.hasMoreTokens()) {
				String spLine;
				spTok.getStringToken(spLine);
				if (!spLine.isEmpty())
					out.addLine(spLine);
			}

			// ScreenPlay states — full dump of the screenPlayStates VectorMap
			String spStates = player->getAllScreenPlayStates();
			StringTokenizer stTok(spStates);
			stTok.setDelimeter("\n");
			while (stTok.hasMoreTokens()) {
				String stLine;
				stTok.getStringToken(stLine);
				if (!stLine.isEmpty())
					out.addLine(stLine);
			}
		}

		out.addLine("END player");
	}
};

#endif // BOTQUERYCOMMAND_H_
