/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions. */

#ifndef SNOOPCOMMAND_H_
#define SNOOPCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/objects/player/sui/messagebox/SuiMessageBox.h"
#include "server/zone/objects/tangible/components/vendor/VendorDataComponent.h"
#include "server/zone/managers/mission/MissionManager.h"

#include "server/zone/managers/auction/AuctionManager.h"
#include "server/zone/managers/auction/AuctionsMap.h"
#include "server/zone/managers/director/ScreenPlayTask.h"
//Infinity:  Custom includes
#include "server/zone/objects/creature/ai/AiAgent.h"
#include "templates/params/creature/CreatureState.h"
#include "templates/params/creature/CreaturePosture.h"
#include "templates/params/creature/CreatureLocomotion.h"
#include "server/zone/objects/player/badges/Badge.h"
#include "server/zone/managers/player/BadgeList.h"
#include "server/zone/managers/stringid/StringIdManager.h"

class SnoopCommand : public QueueCommand {
public:
	SnoopCommand(const String& name, ZoneProcessServer* server) : QueueCommand(name, server) {
	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {
		if (!checkStateMask(creature)) {
			return INVALIDSTATE;
		}

		if (!checkInvalidLocomotions(creature)) {
			return INVALIDLOCOMOTION;
		}

		auto ghost = creature->getPlayerObject();

		if (ghost == nullptr) {
			return GENERALERROR;
		}

		auto zoneServer = creature->getZoneServer();

		if (zoneServer == nullptr) {
			return GENERALERROR;
		}

		auto playerManager = zoneServer->getPlayerManager();

		if (playerManager == nullptr) {
			return GENERALERROR;
		}

		StringTokenizer args(arguments.toString());

		ManagedReference<CreatureObject*> targetCreature = nullptr;
		uint64 targetID = creature->getTargetID();

		if (targetID > 0) {
			auto targetSceneO = zoneServer->getObject(targetID);

			if (targetSceneO == nullptr) {
				return GENERALERROR;
			}

			if (!targetSceneO->isCreatureObject()) {
				StringBuffer targetMsg;
				targetMsg << "Your target for Snoop must be a creature object -- CurrentTarget: " << targetSceneO->getDisplayedName();

				creature->sendSystemMessage(targetMsg.toString());
				return INVALIDTARGET;
			}

			targetCreature = targetSceneO->asCreatureObject();
		} else {
			if (!args.hasMoreTokens()) {
				return GENERALERROR;
			}

			String targetName = "";

			args.getStringToken(targetName);

			targetCreature = playerManager->getPlayer(targetName);
		}

		if (targetCreature == nullptr) {
			creature->sendSystemMessage("Your target for Snoop is invalid.");
			return INVALIDTARGET;
		}

		Locker guard(targetCreature, creature);

		auto targetGhost = targetCreature->getPlayerObject();

		String container = "";

		if (args.hasMoreTokens()) {
			args.getStringToken(container);
		}

		container = container.toLowerCase();

		// Infinity:  Allow snoop buffs for creatures and pets
		if (targetGhost == nullptr && container != "buffs") {
			return GENERALERROR;
		}

		if (container == "equipment") {
			targetCreature->sendWithoutParentTo(creature);
			targetCreature->openContainerTo(creature);
		} else if (container == "datapad") {
			auto creatureDatapad = targetCreature->getSlottedObject("datapad");

			if (creatureDatapad == nullptr) {
				return GENERALERROR;
			}

			creatureDatapad->sendWithoutParentTo(creature);
			creatureDatapad->openContainerTo(creature);
		} else if (container == "bank") {
			auto creatureBank = targetCreature->getSlottedObject("bank");

			if (creatureBank == nullptr) {
				return GENERALERROR;
			}

			creatureBank->sendWithoutParentTo(creature);
			creatureBank->openContainerTo(creature);
		} else if (container == "credits") {
			int cash = targetCreature->getCashCredits();
			int bank = targetCreature->getBankCredits();
			StringBuffer body;

			body << "Player Name:\t" << targetCreature->getFirstName();
			body << "\nCash Credits:\t" << String::withCommas(cash);
			body << "\nBank Credits:\t" << String::withCommas(bank);
			body << "\nBank Location:\t" << targetGhost->getBankLocation();

			ManagedReference<SuiMessageBox*> box = new SuiMessageBox(creature, SuiWindowType::ADMIN_PLAYER_CREDITS);
			box->setPromptTitle("Player Credits");
			box->setPromptText(body.toString());
			box->setUsingObject(targetCreature);
			box->setForceCloseDisabled();

			ghost->addSuiBox(box);
			creature->sendMessage(box->generateMessage());
		} else if (container == "jeditrainer") {
			if (targetGhost->getJediState() < 2 || !targetCreature->hasSkill("force_title_jedi_rank_02")) {
				creature->sendSystemMessage(targetCreature->getFirstName() + " does not have a jedi state of 2+ or does not have the padawan skill box.");
				return GENERALERROR;
			}

			String planet = targetGhost->getTrainerZoneName();
			Vector3 coords = targetGhost->getJediTrainerCoordinates();

			creature->sendSystemMessage(targetCreature->getFirstName() + "'s jedi trainer is located at " + coords.toString() + " on " + planet);
		} else if (container == "ham") {
			return sendHam(creature, targetCreature);
		} else if (container == "lots") {
			return sendLots(creature, targetCreature);
		} else if (container == "vendors") {
			return sendVendorInfo(creature, targetCreature);
		} else if (container == "veteranrewards") {
			return sendVeteranRewardInfo(creature, targetCreature);
		} else if (container == "factions") {
			return sendFactionInfo(creature, targetCreature);
		} else if (container == "faction") {
			return sendGcwFactionInfo(creature, targetCreature);
		} else if (container == "screenplaydata") {
			if (!args.hasMoreTokens()) {
				creature->sendSystemMessage("SYNTAX: /snoop [player] screenplaydata <screenplay> [variable]");
				return INVALIDPARAMETERS;
			}

			String playName, varName;
			args.getStringToken(playName);

			if (!args.hasMoreTokens()) {
				creature->sendSystemMessage("SYNTAX: /snoop [player] screenplaydata <screenplay> [variable]");
				return INVALIDPARAMETERS;
			}

			args.getStringToken(varName);

			String result = targetGhost->getScreenPlayData(playName, varName);

			creature->sendSystemMessage(targetCreature->getFirstName() + "'s screenplay data value for screenplay " + playName + " and variable " + varName + " is: " + result);
		} else if (container == "stringdata") {
			if (!args.hasMoreTokens()) {
				creature->sendSystemMessage("SYNTAX: /snoop [player] stringdata <variable>");
				return INVALIDPARAMETERS;
			}

			String varName;
			args.getStringToken(varName);

			String result = DirectorManager::instance()->readStringSharedMemory(String::valueOf(targetCreature->getObjectID()) + varName);

			creature->sendSystemMessage(targetCreature->getFirstName() + "'s string data value for variable " + varName + " = " + result);

		} else if (container == "screenplaystate") {
			if (!args.hasMoreTokens()) {
				creature->sendSystemMessage("SYNTAX: /snoop [player] screenplaystate <stateName> [state]");
				return INVALIDPARAMETERS;
			}
			String stateName;
			args.getStringToken(stateName);

			uint64 state = targetCreature->getScreenPlayState(stateName);
			if (args.hasMoreTokens()) {
				uint64 stateToCheck = args.getIntToken();
				if (state & stateToCheck)
					creature->sendSystemMessage(targetCreature->getFirstName() + " state check of '" + String::valueOf(stateToCheck) + "' for screenplayState '" + stateName + "': TRUE.");
				else
					creature->sendSystemMessage(targetCreature->getFirstName() + " state check of '" + String::valueOf(stateToCheck) + "' for screenplayState '" + stateName + "': FALSE.");
			} else {
				creature->sendSystemMessage(targetCreature->getFirstName() + " state check for screenplayState '" + stateName + "': " + String::valueOf(state) + ".");
			}
		} else if (container == "activescreenplay") {
			String key = String::valueOf(targetCreature->getObjectID()) + ":activeScreenPlay";
			String data = DirectorManager::instance()->getStringSharedMemory(key);
			creature->sendSystemMessage(targetCreature->getFirstName() + " active screenplay: " + data);
		} else if (container == "luaevents") {
			return sendLuaEvents(creature, targetCreature);
		} else if (container == "buffs") {
			return sendBuffs(creature, targetCreature);
		} else if (container == "visibility") {
			MissionManager* missionManager = creature->getZoneServer()->getMissionManager();

			if (missionManager->sendPlayerBountyDebug(creature, targetCreature)) {
				return SUCCESS;
			} else {
				return GENERALERROR;
			}
		} else if (container == "frs") {
			FrsData* playerData = targetGhost->getFrsData();


            if (playerData == nullptr) {
                return GENERALERROR;
            }

			int playerRank = playerData->getRank();
			int playerCouncil = playerData->getCouncilType();

			String councilString;
			if (playerCouncil == 1) {
				councilString = "Light";
			} else if (playerCouncil == 2) {
				councilString = "Dark";
			} else {
				councilString = "Unknown";
			}

			StringBuffer msg;

			msg << targetCreature->getFirstName() << " is a rank " << playerRank << " " << councilString << " Jedi with " << String::withCommas(targetGhost->getExperience("force_rank_xp")) << " FRS XP.";
			creature->sendSystemMessage(msg.toString());

		} else if (container == "export") {
			StringBuffer reason = "/snoop " + targetCreature->getFirstName() + " export by " + creature->getFirstName();

			if (args.hasMoreTokens()) {
				String note;
				args.finalToken(note);
				reason << "; Admin Note: " << note;
			}

			String exportFile = targetCreature->exportJSON(reason.toString());
			creature->sendSystemMessage(targetCreature->getFirstName() + " exported to " + exportFile + " ask a server admin to review the file for you.");
		} else if (container == "skills") {
			return sendSkillsInfo(creature, targetCreature, args.hasMoreTokens());

		} else if (container == "experience" || container == "xp") {
			return sendExperienceInfo(creature, targetCreature, args.hasMoreTokens());

		} else if (container == "activepets") {
			return sendActivePets(creature, targetCreature);

		} else if (container == "skillmods") {
			return sendSkillModsInfo(creature, targetCreature, args.hasMoreTokens());

		} else if (container == "badges") {
			return sendBadgeInfo(creature, targetCreature, args.hasMoreTokens());

		} else if (container == "permissiongroups") {
			SortedVector<String>* groups = targetGhost->getPermissionGroups();

			if (groups != nullptr && groups->size() > 0) {
				StringBuffer body;

				body << endl << "Permission groups for player " << targetCreature->getDisplayedName() << endl;

				for (int i = 0; i < groups->size(); ++i) {
					const String group = groups->get(i);

					body << "  " << group << endl;
				}

				creature->sendSystemMessage(body.toString());

				if (args.hasMoreTokens()) { // Send an email if more tokens after containers
					auto chatManager = server->getZoneServer()->getChatManager();
					if (chatManager != nullptr) {
						chatManager->sendMail("System", "Permission groups for " + targetCreature->getDisplayedName(), body.toString(), creature->getFirstName());
					}
				}
			} else {
				creature->sendSystemMessage(targetCreature->getDisplayedName() + " has no permission groups.");
			}
		} else if (container == "states") {
			return sendStatesInfo(creature, targetCreature);

		} else if (container == "guild") {
			if (targetCreature->isInGuild()) {
				auto guild = targetCreature->getGuildObject().get();

				String guildName = "Unknown";

				if (guild != nullptr) {
					guildName = guild->getGuildName();
				}
				creature->sendSystemMessage(targetCreature->getDisplayedName() + " is in the guild: " + guildName);
			} else {
				creature->sendSystemMessage(targetCreature->getDisplayedName() + " is not in a guild.");
			}
		} else {
			SceneObject* creatureInventory = targetCreature->getSlottedObject("inventory");

			if (creatureInventory == nullptr)
				return GENERALERROR;

			creatureInventory->sendWithoutParentTo(creature);
			creatureInventory->openContainerTo(creature);
		}

		return SUCCESS;
	}

	int sendLuaEvents(CreatureObject* creature, CreatureObject* target) const {
		if (creature == nullptr || target == nullptr) {
			return GENERALERROR;
		}

		auto ghost = creature->getPlayerObject();

		if (ghost == nullptr) {
			return GENERALERROR;
		}

		Vector<Reference<ScreenPlayTask*>> eventList = DirectorManager::instance()->getObjectEvents(target);

		ManagedReference<SuiListBox*> box = new SuiListBox(creature, 0);
		box->setPromptTitle("LUA Events");
		Time currentTime;
		box->setPromptText("Below are the LUA Events currently scheduled for the player.\n\nCurrent server time: " + currentTime.getFormattedTime());
		box->setUsingObject(target);
		box->setForceCloseDisabled();

		for (int i = 0; i < eventList.size(); i++) {
			Reference<ScreenPlayTask*> task = eventList.get(i);

			if (task == nullptr) {
				continue;
			}

			String buffer = task->getScreenPlay() + ":" + task->getTaskKey();
			String args = task->getArgs();

			if (args != "") {
				buffer += " (Args: " + args + ")";
			}

			AtomicTime nextExecutionTime;
			Core::getTaskManager()->getNextExecutionTime(task, nextExecutionTime);
			int64 miliDiff = nextExecutionTime.miliDifference();

			buffer += ", Execution (server time): " + nextExecutionTime.getFormattedTime() + " (" + getTimeString(-miliDiff) + " from now)";

			box->addMenuItem(buffer);
		}

		ghost->addSuiBox(box);
		creature->sendMessage(box->generateMessage());

		return SUCCESS;
	}

	String getTimeString(uint64 timestamp) const {
		int seconds = timestamp / 1000;

		int hours = seconds / 3600;
		seconds -= hours * 3600;

		int minutes = seconds / 60;
		seconds -= minutes * 60;

		StringBuffer buffer;

		if (hours > 0)
			buffer << hours << "h ";

		if (minutes > 0)
			buffer << minutes << "m ";

		if (seconds > 0)
			buffer << seconds << "s";

		return buffer.toString();
	}

	int sendVeteranRewardInfo(CreatureObject* creature, CreatureObject* target) const {
		if (creature == nullptr || target == nullptr) {
			return GENERALERROR;
		}

		auto targetGhost = target->getPlayerObject();
		auto ghost = creature->getPlayerObject();
		auto zoneServer = server->getZoneServer();

		if (targetGhost == nullptr || ghost == nullptr || zoneServer == nullptr) {
			return GENERALERROR;
		}

		auto playerManager = zoneServer->getPlayerManager();

		if (playerManager == nullptr) {
			return GENERALERROR;
		}

		StringBuffer body;
		body << "Player Name:\t" << target->getFirstName() << endl;
		body << "ID: " << target->getObjectID() << endl << endl;
		body << "Claimed Rewards:" << endl;
		body << "\tMilestone\tReward" << endl;

		// Sorosuub Yacht claim
		body << "\t" << "1";
		String claimedSorosuub = targetGhost->getChosenVeteranReward(1);

		if (claimedSorosuub.isEmpty()) {
			body << "\t\t\t" << "Unclaimed" << endl;
		} else {
			body << "\t\t\t" << claimedSorosuub << endl;
		}

		// Standard reward milestones
		for (int i = 0; i < playerManager->getNumVeteranRewardMilestones(); i++) {
			int milestone = playerManager->getVeteranRewardMilestone(i);

			body << "\t" << String::valueOf(milestone);
			String claimedReward = targetGhost->getChosenVeteranReward(milestone);
			if (claimedReward.isEmpty()) {
				body << "\t\t" << "Unclaimed" << endl;
			} else {
				body << "\t\t" << claimedReward << endl;
			}
		}

		ManagedReference<SuiMessageBox*> box = new SuiMessageBox(creature, 0);
		box->setPromptTitle("Veteran Reward Info - " + target->getFirstName());
		box->setPromptText(body.toString());
		box->setUsingObject(target);
		box->setForceCloseDisabled();

		ghost->addSuiBox(box);
		creature->sendMessage(box->generateMessage());

		return SUCCESS;
	}

	int sendGcwFactionInfo(CreatureObject* creature, CreatureObject* target) const {
		if (creature == nullptr || target == nullptr)
			return GENERALERROR;

		auto targetGhost = target->getPlayerObject();
		auto ghost = creature->getPlayerObject();
		auto zoneServer = server->getZoneServer();

		if (targetGhost == nullptr || ghost == nullptr || zoneServer == nullptr) {
			return GENERALERROR;
		}

		auto playerManager = zoneServer->getPlayerManager();

		if (playerManager == nullptr) {
			return GENERALERROR;
		}

		StringBuffer body;
		body << "Player Name:\t" << target->getDisplayedName() << endl;
		body << "Affiliation:\t";

		if (target->isImperial())
			body << "Imperial";
		else if (target->isRebel())
			body << "Rebel";
		else
			body << "Neutral" << endl;

		int rank = 0;

		if (target->isImperial() || target->isRebel()) {
			if (target->getFactionStatus() == FactionStatus::ONLEAVE)
				body << " (On Leave)" << endl;
			else if (target->getFactionStatus() == FactionStatus::OVERT)
				body << " (Overt)" << endl;
			else if (target->getFactionStatus() == FactionStatus::COVERT)
				body << " (Covert)" << endl;

			rank = target->getFactionRank();
			body << "Rank:\t" << FactionManager::instance()->getRankName(rank) << " (Rank " << rank + 1 << ")" << endl;
		}
		body << "Imperial Points:\t" << targetGhost->getFactionStanding("imperial") << " (Max: " << (target->isImperial() ? FactionManager::instance()->getFactionPointsCap(rank) : 1000) << ")" << endl;
		body << "Rebel Points:\t" << targetGhost->getFactionStanding("rebel") << " (Max: " << (target->isRebel() ? FactionManager::instance()->getFactionPointsCap(rank) : 1000) << ")" << endl;

		ManagedReference<SuiMessageBox*> box = new SuiMessageBox(creature, 0);
		box->setPromptTitle("GCW Faction Information for " + target->getDisplayedName());
		box->setPromptText(body.toString());
		box->setUsingObject(target);
		box->setForceCloseDisabled();

		ghost->addSuiBox(box);
		creature->sendMessage(box->generateMessage());

		return SUCCESS;
	}

	int sendFactionInfo(CreatureObject* creature, CreatureObject* target) const {
		if (creature == nullptr || target == nullptr)
			return GENERALERROR;

		auto targetGhost = target->getPlayerObject();
		auto ghost = creature->getPlayerObject();
		auto zoneServer = server->getZoneServer();

		if (targetGhost == nullptr || ghost == nullptr || zoneServer == nullptr) {
			return GENERALERROR;
		}

		auto playerManager = zoneServer->getPlayerManager();

		if (playerManager == nullptr) {
			return GENERALERROR;
		}

		StringBuffer body;

		auto factionStandingList = targetGhost->getFactionStandingList();

		if (factionStandingList != nullptr) {
			auto factions = factionStandingList->getAllFactions();
			if (factions.size() > 0) {
				for (int i = 0; i < factions.size(); ++i) {
					auto faction = factions.getKey(i);
					auto points = factions.get(faction);

					String lookup = "@faction/faction_names:" + faction;
					String stringId = StringIdManager::instance()->getStringId(lookup.hashCode()).toString();

                    body << (!stringId.isEmpty() ? stringId : faction) << ":  " << (int)points << endl;
				}
			} else {
				body << "No Faction Points." << endl;
			}
		}

		ManagedReference<SuiMessageBox*> box = new SuiMessageBox(creature, 0);
		box->setPromptTitle("Faction Information for " + target->getDisplayedName());
		box->setPromptText(body.toString());
		box->setUsingObject(target);
		box->setForceCloseDisabled();

		ghost->addSuiBox(box);
		creature->sendMessage(box->generateMessage());

		return SUCCESS;
	}

	int sendVendorInfo(CreatureObject* creature, CreatureObject* target) const {
		if (creature == nullptr || target == nullptr) {
			return GENERALERROR;
		}

		auto targetGhost = target->getPlayerObject();
		auto ghost = creature->getPlayerObject();
		auto zoneServer = server->getZoneServer();

		if (targetGhost == nullptr || ghost == nullptr || zoneServer == nullptr) {
			return GENERALERROR;
		}

		auto auctionManager = zoneServer->getAuctionManager();

		if (auctionManager == nullptr) {
			return GENERALERROR;
		}

		ManagedReference<AuctionsMap*> auctionsMap = auctionManager->getAuctionMap();
		if (auctionsMap == nullptr) {
			return GENERALERROR;
		}

		StringBuffer body;
		body << "Player Name:\t" << target->getFirstName() << endl;
		body << "Max # of vendors:\t" << target->getSkillMod("manage_vendor") << endl;
		body << "Max # of items:\t" << target->getSkillMod("vendor_item_limit") << endl;
		body << "Total # of items:\t" << auctionsMap->getPlayerItemCount(target) << endl << endl;
		body << "Vendors:" << endl;

		const SortedVector<unsigned long long>* ownedVendors = targetGhost->getOwnedVendors();

		for (int i = 0; i < ownedVendors->size(); i++) {
			ManagedReference<SceneObject*> vendor = creature->getZoneServer()->getObject(ownedVendors->elementAt(i));

			int num = i + 1;
			body << endl << String::valueOf(num) << ". ";

			if (vendor == nullptr) {
				body << "nullptr Vendor" << endl << endl;
				continue;
			}

			body << "VendorID:\t" << vendor->getObjectID() << endl;

			DataObjectComponentReference* data = vendor->getDataObjectComponent();

			if (data == nullptr || data->get() == nullptr || !data->get()->isVendorData()) {
				body << "    nullptr Data Component" << endl << endl;
				continue;
			}

			VendorDataComponent* vendorData = cast<VendorDataComponent*>(data->get());

			if (vendorData == nullptr) {
				body << "    nullptr Vendor Data Component" << endl << endl;
				continue;
			}

			body << "    Initialized?\t" << (vendorData->isInitialized() ? "Yes" : "No");
			body << endl << "    # of items:\t" << auctionsMap->getVendorItemCount(vendor) << endl;

			body << "    ParentID:\t";

			ManagedReference<SceneObject*> parent = vendor->getParent().get();

			if (parent == nullptr)
				body << "nullptr" << endl;
			else
				body << parent->getObjectID() << endl;

			body << "    Zone:\t";

			auto zone = vendor->getZone();
			if (zone == nullptr) {
				body << "nullptr" << endl;
			} else {
				body << zone->getZoneName() << endl;
				body << "    World Position:\t" << vendor->getWorldPositionX() << ", " << vendor->getWorldPositionY() << endl;
			}
		}

		ManagedReference<SuiMessageBox*> box = new SuiMessageBox(creature, 0);

		box->setPromptTitle("Vendor Info");
		box->setPromptText(body.toString());
		box->setUsingObject(target);
		box->setForceCloseDisabled();

		ghost->addSuiBox(box);
		creature->sendMessage(box->generateMessage());

		return SUCCESS;
	}

	int sendLots(CreatureObject* creature, CreatureObject* target) const {
		if (creature == nullptr || target == nullptr) {
			return GENERALERROR;
		}

		auto targetGhost = target->getPlayerObject();
		auto ghost = creature->getPlayerObject();
		auto zoneServer = creature->getZoneServer();

		if (targetGhost == nullptr || ghost == nullptr || zoneServer == nullptr) {
			return GENERALERROR;
		}

		StringBuffer body;

		body << "Player Name:\t" << target->getFirstName() << endl;
		body << "Unused Lots:\t" << targetGhost->getLotsRemaining() << endl << endl;
		body << "Player Structures:";

		for (int i = 0; i < targetGhost->getTotalOwnedStructureCount(); i++) {
			uint64 oid = targetGhost->getOwnedStructure(i);

			auto structure = zoneServer->getObject(oid).castTo<StructureObject*>();

			int num = i + 1;
			body << endl << String::valueOf(num) << ". ";

			if (structure == nullptr) {
				body << "nullptr Structure.  OID: " << oid << "." << endl << endl;
				continue;
			}

			body << "StructureID:\t" << structure->getObjectID() << endl;
			body << "    Name:\t" << structure->getCustomObjectName().toString() << endl;
			body << "    Type:\t" << structure->getObjectTemplate()->getFullTemplateString() << endl;
			body << "    Lots:\t" << String::valueOf(structure->getLotSize()) << endl;

			body << "    Zone:\t";
			Zone* zone = structure->getZone();
			if (zone == nullptr) {
				body << "nullptr" << endl;
			} else {
				body << zone->getZoneName() << endl;
				body << "    World Position:\t" << structure->getWorldPositionX() << ", " << structure->getWorldPositionY() << endl;
			}
		}

		ManagedReference<SuiMessageBox*> box = new SuiMessageBox(creature, 0);
		box->setPromptTitle("Player Lots");
		box->setPromptText(body.toString());
		box->setUsingObject(target);
		box->setForceCloseDisabled();

		ghost->addSuiBox(box);
		creature->sendMessage(box->generateMessage());

		return SUCCESS;
	}

	int sendHam(CreatureObject* creature, CreatureObject* target) const {
		if (creature == nullptr || target == nullptr) {
			return GENERALERROR;
        }

		auto ghost = creature->getPlayerObject();
		auto targetGhost = target->getPlayerObject();

		if (ghost == nullptr) {
			return GENERALERROR;
		}

		StringBuffer body;
		body << "Player Name:\t" << target->getFirstName() << endl << endl;

		for (int i = 0; i < 9; i++) {
			switch (i) {
				case 0:
					body << "Health: ";
					break;
				case 1:
					body << "Strength: ";
					break;
				case 2:
					body << "Constitution: ";
					break;
				case 3:
					body << "Action: ";
					break;
				case 4:
					body << "Quickness: ";
					break;
				case 5:
					body << "Stamina: ";
					break;
				case 6:
					body << "Mind: ";
					break;
				case 7:
					body << "Focus: ";
					break;
				case 8:
					body << "Willpower: ";
					break;
				default:
					break;
			}

			body << target->getHAM(i) << " / " << target->getMaxHAM(i) << "\n";
			body << "Wounds: " << target->getWounds(i) << "\n";
			body << "Modifiers: " << (target->getMaxHAM(i) - target->getBaseHAM(i) + target->getEncumbrance(i / 3)) << "\n";
			body << "Encumbrance: " << target->getEncumbrance(i / 3) << "\n";

			body << "\n\n";
		}

		if (targetGhost != nullptr) { // if we're not a PlayerObject, we don't hold force values
			body << "Force Power:\t" << targetGhost->getForcePower() << " / " << targetGhost->getForcePowerMax() << endl;
			body << "Force Regen:\t" << target->getSkillMod("jedi_force_power_regen") << endl;
		}

		ManagedReference<SuiMessageBox*> box = new SuiMessageBox(creature, 0);
		box->setPromptTitle("Player HAM");
		box->setPromptText(body.toString());
		box->setUsingObject(target);
		box->setForceCloseDisabled();

		ghost->addSuiBox(box);
		creature->sendMessage(box->generateMessage());

		return SUCCESS;
	}

	int sendBuffs(CreatureObject* creature, CreatureObject* target) const {
		if (creature == nullptr || target == nullptr) {
			return GENERALERROR;
		}

		auto ghost = creature->getPlayerObject();
		if (ghost == nullptr) {
			return GENERALERROR;
		}

		const BuffList* bList = target->getBuffList();
		if (bList == nullptr || bList->getBuffListSize() == 0) {
			creature->sendSystemMessage("No Buffs to Display.");
			return SUCCESS;
		}

		StringBuffer buffText;

		for (int i = 0; i < bList->getBuffListSize(); i++) {
			Buff* buff = bList->getBuffByIndex(i);
			buffText << buff->getBuffName() << ":" << endl;
			buffText << "\tCRC: 0x" << hex << buff->getBuffCRC() << endl;

			const Vector<uint64>* secondaryCRCs = buff->getSecondaryBuffCRCs();
			if (secondaryCRCs != nullptr && secondaryCRCs->size() > 0) {
				buffText << "\tSecondary CRCs: " << endl;
				for (int j = 0; j < secondaryCRCs->size(); j++) {
					buffText << "\t\t 0x" << hex << buff->getSecondaryBuffCRCs() << endl;
				}
			}

			buffText << "\tDuration (" << buff->getBuffDuration() << ") Time Left (" << buff->getTimeLeft() << ")" << endl;

			buffText << "\tAttribute Mods: " << buff->getAttributeModifierString() << endl;
			buffText << "\tSkill Mods: " << buff->getSkillModifierString() << endl;
		}

		ManagedReference<SuiMessageBox*> box = new SuiMessageBox(creature, 0);
		box->setPromptTitle("Creature Buffs");
		box->setPromptText(buffText.toString());
		box->setUsingObject(target);
		box->setForceCloseDisabled();

		ghost->addSuiBox(box);
		creature->sendMessage(box->generateMessage());

		return SUCCESS;
	}

	int sendSkillsInfo(CreatureObject* creature, CreatureObject* targetCreature, bool eMail = false) const {
		if (creature == nullptr || targetCreature == nullptr)
			return GENERALERROR;

		auto ghost = creature->getPlayerObject();
		auto targetGhost = targetCreature->getPlayerObject();

		if (targetGhost == nullptr || ghost == nullptr)
			return GENERALERROR;

		Locker locker(targetCreature, creature);

		SkillManager* skillManager = SkillManager::instance();
		const SkillList* skillList = targetCreature->getSkillList();
		StringBuffer body;
		String targetName = targetCreature->getDisplayedName();

		if (skillList == nullptr) {
			return GENERALERROR;
		}

		body << "Player Name:\t" << targetName;
		body << "\nSkills on Character:\n";
		String skillName = "";
		Vector<String> listOfNames;
		skillList->getStringList(listOfNames);
		SkillList copyOfList;
		copyOfList.loadFromNames(listOfNames);

		for (int i = 0; i < copyOfList.size(); i++) {
			Skill* skill = copyOfList.get(i);
			String skillName = skill->getSkillName();
			body << skillName << "\n";
		}

		ManagedReference<SuiMessageBox*> box = new SuiMessageBox(creature, 0);
		box->setPromptTitle("Skill Info");
		box->setPromptText(body.toString());
		box->setUsingObject(targetCreature);
		box->setForceCloseDisabled();

		ghost->addSuiBox(box);
		creature->sendMessage(box->generateMessage());

		if (eMail) { // Send an email if more tokens after experience
			auto chatManager = server->getZoneServer()->getChatManager();
			if (chatManager != nullptr)
				chatManager->sendMail("System", "Skill information for " + targetName, body.toString(), creature->getFirstName());
		}

		return SUCCESS;
	}

	int sendExperienceInfo(CreatureObject* creature, CreatureObject* targetCreature, bool eMail = false) const {
		if (creature == nullptr || targetCreature == nullptr) {
			return GENERALERROR;
		}

		auto ghost = creature->getPlayerObject();
		auto targetGhost = targetCreature->getPlayerObject();

		if (targetGhost == nullptr || ghost == nullptr) {
			return GENERALERROR;
		}

		Locker locker(targetCreature, creature);

		VectorMap<String, int>* xpCapList = targetGhost->getXpTypeCapList();

		if (xpCapList == nullptr) {
			return GENERALERROR;
		}

		StringBuffer body;
		String targetName = targetCreature->getDisplayedName();

		body << "Player Name:\t" << targetName;
		body << "\nExperience Values:\n";

		for (int i = 0; i < xpCapList->size(); ++i) {
			String xpType = xpCapList->elementAt(i).getKey();
			int xpValue = targetGhost->getExperience(xpType);
			if (xpValue != 0)
				body << xpType << ":  " << String::withCommas(xpValue) << "\n";
		}

		ManagedReference<SuiMessageBox*> box = new SuiMessageBox(creature, 0);
		box->setPromptTitle("Experience Info");
		box->setPromptText(body.toString());
		box->setUsingObject(targetCreature);
		box->setForceCloseDisabled();

		ghost->addSuiBox(box);
		creature->sendMessage(box->generateMessage());

		if (eMail) { // Send an email if more tokens after experience
			auto chatManager = server->getZoneServer()->getChatManager();
			if (chatManager != nullptr) {
				chatManager->sendMail("System", "Experience information for " + targetName, body.toString(), creature->getFirstName());
			}
		}

		return SUCCESS;
	}

	int sendActivePets(CreatureObject* creature, CreatureObject* targetCreature) const {
		if (creature == nullptr || targetCreature == nullptr) {
			return GENERALERROR;
		}

		auto ghost = creature->getPlayerObject();
		auto targetGhost = targetCreature->getPlayerObject();

		if (targetGhost == nullptr || ghost == nullptr)
			return GENERALERROR;

		String targetName = targetCreature->getDisplayedName();

		Locker locker(targetCreature, creature);

		if (targetGhost->getActivePetsSize() > 0) {
			StringBuffer msg;
			msg << "\n" << "Active Pets for player " << targetName << "\n";
			msg << "-----------------------------------------------------------------------------------------------\n";

			for (int i = 0; i < targetGhost->getActivePetsSize(); i++) {
				ManagedReference<AiAgent*> pet = targetGhost->getActivePet(i);
				if (pet != nullptr) {
					msg << "#" << i + 1 << " - OID = " << pet->getObjectID() << ".  Pet name = " << pet->getDisplayedName() << ".\n";
				}
			}

			creature->sendSystemMessage(msg.toString());
		} else {
			creature->sendSystemMessage("No active pets found for player " + targetName);
		}

		return SUCCESS;
	}

	int sendSkillModsInfo(CreatureObject* creature, CreatureObject* targetCreature, bool eMail = false) const {
		if (creature == nullptr || targetCreature == nullptr) {
			return GENERALERROR;
        }

		auto ghost = creature->getPlayerObject();
		auto targetGhost = targetCreature->getPlayerObject();
		if (targetGhost == nullptr || ghost == nullptr)
			return GENERALERROR;

		String targetName = targetCreature->getDisplayedName();

		ManagedReference<SuiMessageBox*> box = new SuiMessageBox(creature, 0);

		box->setPromptTitle("Skill Modifer Info For " + targetName);

		Locker smodLocker(targetCreature->getSkillModMutex());

		const SkillModList* skillModList = targetCreature->getSkillModList();

		smodLocker.release();

		StringBuffer promptText;

		promptText << "SkillMods" << endl << "------------" << endl;
		promptText << skillModList->getPrintableSkillModList();

		box->setPromptText(promptText.toString());
		creature->sendMessage(box->generateMessage());

		if (eMail) { // Send an email if more tokens after experience
			auto chatManager = server->getZoneServer()->getChatManager();
			if (chatManager != nullptr)
				chatManager->sendMail("System", "Skill Modifier information for " + targetName, promptText.toString(), creature->getFirstName());
		}

		return SUCCESS;
	}

	int sendBadgeInfo(CreatureObject* creature, CreatureObject* targetCreature, bool eMail = false) const {
		if (creature == nullptr || targetCreature == nullptr) {
			return GENERALERROR;
		}

		auto ghost = creature->getPlayerObject();
		auto targetGhost = targetCreature->getPlayerObject();

		if (targetGhost == nullptr || ghost == nullptr) {
			return GENERALERROR;
		}

		Locker locker(targetCreature, creature);

		auto zoneServer = server->getZoneServer();

		if (zoneServer != nullptr) {
			auto playerManager = zoneServer->getPlayerManager();
			if (playerManager != nullptr) {
				playerManager->validateBadges(targetGhost, true); // Validate badges and count
			}
		}

		StringBuffer body;
		String targetName = targetCreature->getDisplayedName();

		body << "Player Name:\t\t" << targetName << endl;
		body << "Number of badges:\t" << targetGhost->getNumBadges() << endl;
		body << "Badge List:" << endl << endl;

		const BadgeList* badgeList = BadgeList::instance();

		if (badgeList == nullptr) {
			return GENERALERROR;
		}

		for (uint8 i = 0; i <= 250; ++i) { // Infinity:  Allow up to 251 badges
			if (targetGhost->hasBadge(i)) {
				const Badge* badge = badgeList->get(i);

				if (badge != nullptr) {
					String lookup = "@badge_n:" + badge->getKey();
					String type = badge->getTypeString();
					uint32 typeHash = type.hashCode();

					switch (typeHash) {
						case STRING_HASHCODE("events"):
							type = "Events\t\t\t\t";
							break;

						case STRING_HASHCODE("master"):
							type = "Master\t\t\t\t";
							break;

						case STRING_HASHCODE("content"):
							type = "Content\t\t\t";
							break;

						case STRING_HASHCODE("interest"):
							type = "Interest\t\t\t";
							break;

						case STRING_HASHCODE("accumulation"):
							type = "Accumulation\t\t";
							break;

						case STRING_HASHCODE("master_nojedi"):
							type = "Master (Non-Glowy)\t";
							break;

						case STRING_HASHCODE("exploration_jedi"):
							type = "Jedi POI\t\t\t";
							break;

						case STRING_HASHCODE("exploration_easy"):
							type = "Easy POI\t\t\t";
							break;

						case STRING_HASHCODE("exploration_hard"):
							type = "Hard POI\t\t\t";
							break;

						case STRING_HASHCODE("exploration_dangerous"):
							type = "Dangerous POI\t\t";
							break;

						default:
							type = "Unknown Type\t\t";
							break;
					}

					body << "\tBadge ID: " << String::format("%03d", i) << "  Type: " << type << "  Badge: " << StringIdManager::instance()->getStringId(lookup.hashCode()).toString() << endl;
				}
			}
		}

		ManagedReference<SuiMessageBox*> box = new SuiMessageBox(creature, 0);
		box->setPromptTitle("Badge Info");
		box->setPromptText(body.toString());
		box->setUsingObject(targetCreature);
		box->setForceCloseDisabled();

		ghost->addSuiBox(box);
		creature->sendMessage(box->generateMessage());

		if (eMail) {	// Send an email if more tokens after experience
			auto chatManager = server->getZoneServer()->getChatManager();
			if (chatManager != nullptr)
				chatManager->sendMail("System", "Badge information for " + targetName, body.toString(), creature->getFirstName());
		}
			
		return SUCCESS;
	}
	// Infinity: /snoop [target] states — show all creature states for debugging (#1463)
	int sendStatesInfo(CreatureObject* creature, CreatureObject* target) const {
		if (creature == nullptr || target == nullptr) {
			return GENERALERROR;
		}

		auto ghost = creature->getPlayerObject();
		if (ghost == nullptr) {
			return GENERALERROR;
		}

		uint64 bitmask = target->getStateBitmask();

		StringBuffer body;
		body << "Player: " << target->getDisplayedName() << endl;
		body << "State Bitmask: 0x" << hex << bitmask << endl << endl;

		// Check each known state bit
		struct StateInfo { uint64 bit; const char* name; };
		static const StateInfo allStates[] = {
			{ CreatureState::COVER,                    "Cover" },
			{ CreatureState::COMBAT,                   "Combat" },
			{ CreatureState::PEACE,                    "Peace" },
			{ CreatureState::AIMING,                   "Aiming" },
			{ CreatureState::ALERT,                    "Alert" },
			{ CreatureState::BERSERK,                  "Berserk" },
			{ CreatureState::FEIGNDEATH,               "FeignDeath" },
			{ CreatureState::COMBATATTITUDEEVASIVE,    "CombatAttitudeEvasive" },
			{ CreatureState::COMBATATTITUDENORMAL,     "CombatAttitudeNormal" },
			{ CreatureState::COMBATATTITUDEAGGRESSIVE, "CombatAttitudeAggressive" },
			{ CreatureState::TUMBLING,                 "Tumbling" },
			{ CreatureState::RALLIED,                  "Rallied" },
			{ CreatureState::STUNNED,                  "Stunned" },
			{ CreatureState::BLINDED,                  "Blinded" },
			{ CreatureState::DIZZY,                    "Dizzy" },
			{ CreatureState::INTIMIDATED,              "Intimidated" },
			{ CreatureState::IMMOBILIZED,              "Immobilized" },
			{ CreatureState::FROZEN,                   "Frozen" },
			{ CreatureState::SWIMMING,                 "Swimming" },
			{ CreatureState::SITTINGONCHAIR,           "SittingOnChair" },
			{ CreatureState::CRAFTING,                 "Crafting" },
			{ CreatureState::GLOWINGJEDI,              "GlowingJedi" },
			{ CreatureState::MASKSCENT,                "MaskScent" },
			{ CreatureState::POISONED,                 "Poisoned" },
			{ CreatureState::BLEEDING,                 "Bleeding" },
			{ CreatureState::DISEASED,                 "Diseased" },
			{ CreatureState::ONFIRE,                   "OnFire" },
			{ CreatureState::RIDINGMOUNT,              "RidingMount" },
			{ CreatureState::MOUNTEDCREATURE,          "MountedCreature" },
			{ CreatureState::PILOTINGSHIP,             "PilotingShip" },
			{ CreatureState::SHIPOPERATIONS,           "ShipOperations" },
			{ CreatureState::SHIPGUNNER,               "ShipGunner" },
			{ CreatureState::SHIPINTERIOR,             "ShipInterior" },
			{ CreatureState::PILOTINGPOBSHIP,          "PilotingPobShip" },
		};

		for (const auto& s : allStates) {
			bool active = (bitmask & s.bit) != 0;
			body << (active ? "[X] " : "[ ] ") << s.name << " (0x" << hex << s.bit << ")" << endl;
		}

		// Check for any unknown bits
		uint64 knownMask = 0;
		for (const auto& s : allStates) {
			knownMask |= s.bit;
		}
		uint64 unknownBits = bitmask & ~knownMask;
		if (unknownBits != 0) {
			body << endl << "UNKNOWN STATE BITS: 0x" << hex << unknownBits << endl;
		}

		// Posture
		uint8 posture = target->getPosture();
		body << endl << "Posture: " << CreaturePosture::postureToString(posture) << endl;

		struct PostureInfo { uint8 id; const char* name; };
		static const PostureInfo allPostures[] = {
			{ CreaturePosture::UPRIGHT,        "Upright" },
			{ CreaturePosture::CROUCHED,       "Crouched" },
			{ CreaturePosture::PRONE,          "Prone" },
			{ CreaturePosture::SNEAKING,       "Sneaking" },
			{ CreaturePosture::BLOCKING,       "Blocking" },
			{ CreaturePosture::CLIMBING,       "Climbing" },
			{ CreaturePosture::FLYING,         "Flying" },
			{ CreaturePosture::LYINGDOWN,      "LyingDown" },
			{ CreaturePosture::SITTING,        "Sitting" },
			{ CreaturePosture::SKILLANIMATING, "SkillAnimating" },
			{ CreaturePosture::DRIVINGVEHICLE, "DrivingVehicle" },
			{ CreaturePosture::RIDINGCREATURE, "RidingCreature" },
			{ CreaturePosture::KNOCKEDDOWN,    "KnockedDown" },
			{ CreaturePosture::INCAPACITATED,  "Incapacitated" },
			{ CreaturePosture::DEAD,           "Dead" },
		};

		for (const auto& p : allPostures) {
			bool current = (posture == p.id);
			body << (current ? "[X] " : "[ ] ") << p.name << " (" << (int)p.id << ")" << endl;
		}

		// Locomotion
		uint8 locomotion = target->getLocomotion();
		body << endl << "Locomotion:" << endl;

		struct LocoInfo { uint8 id; const char* name; };
		static const LocoInfo allLocos[] = {
			{ CreatureLocomotion::STANDING,           "Standing" },
			{ CreatureLocomotion::SNEAKING,           "Sneaking" },
			{ CreatureLocomotion::WALKING,            "Walking" },
			{ CreatureLocomotion::RUNNING,            "Running" },
			{ CreatureLocomotion::KNEELING,           "Kneeling" },
			{ CreatureLocomotion::CROUCHSNEAKING,     "CrouchSneaking" },
			{ CreatureLocomotion::CROUCHWALKING,      "CrouchWalking" },
			{ CreatureLocomotion::PRONE,              "Prone" },
			{ CreatureLocomotion::CRAWLING,           "Crawling" },
			{ CreatureLocomotion::CLIMBINGSTATIONARY,  "ClimbingStationary" },
			{ CreatureLocomotion::CLIMBING,           "Climbing" },
			{ CreatureLocomotion::HOVERING,           "Hovering" },
			{ CreatureLocomotion::FLYING,             "Flying" },
			{ CreatureLocomotion::LYINGDOWN,          "LyingDown" },
			{ CreatureLocomotion::SITTING,            "Sitting" },
			{ CreatureLocomotion::SKILLANIMATING,     "SkillAnimating" },
			{ CreatureLocomotion::DRIVINGVEHICLE,     "DrivingVehicle" },
			{ CreatureLocomotion::RIDINGCREATURE,     "RidingCreature" },
			{ CreatureLocomotion::KNOCKEDDOWN,        "KnockedDown" },
			{ CreatureLocomotion::INCAPACITATED,      "Incapacitated" },
			{ CreatureLocomotion::DEAD,               "Dead" },
			{ CreatureLocomotion::BLOCKING,           "Blocking" },
		};

		for (const auto& l : allLocos) {
			bool current = (locomotion == l.id);
			body << (current ? "[X] " : "[ ] ") << l.name << " (" << (int)l.id << ")" << endl;
		}

		ManagedReference<SuiMessageBox*> box = new SuiMessageBox(creature, SuiWindowType::ADMIN_PLAYER_CREDITS);
		box->setPromptTitle("Creature States - " + target->getDisplayedName());
		box->setPromptText(body.toString());
		box->setUsingObject(target);
		box->setForceCloseDisabled();

		ghost->addSuiBox(box);
		creature->sendMessage(box->generateMessage());

		return SUCCESS;
	}
};

#endif // SNOOPCOMMAND_H_
