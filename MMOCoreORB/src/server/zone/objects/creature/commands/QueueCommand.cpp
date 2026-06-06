/*
 * QueueCommand.cpp
 *
 *  Created on: 22/05/2010
 *      Author: victor
 */

#include "QueueCommand.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/objects/building/BuildingObject.h"
#include "server/zone/objects/player/PlayerObject.h"
#include "server/zone/objects/player/FactionStatus.h"
#include "server/zone/objects/tangible/weapon/WeaponObject.h"
#include "server/zone/managers/combat/CombatManager.h"
#include "server/zone/managers/frs/FrsManager.h"
#include "server/zone/managers/death_watch/DeathWatchManager.h"
#include "server/zone/ZoneServer.h"
#include "templates/params/creature/CreatureLocomotion.h"

QueueCommand::QueueCommand(const String& skillname, ZoneProcessServer* serv) : Logger() {
	server = serv;

	name = skillname;
	nameCRC = skillname.hashCode();

	maxRangeToTarget = 0;

	commandGroup = 0;

	stateMask = 0;
	targetType = 0;
	disabled = false;
	addToQueue = false;
	admin = false;

	defaultTime = 0.f;
	cooldown = 0;
	defaultPriority = NORMAL;

	setLogging(false);
	setGlobalLogging(true);
	setLoggingName("QueueCommand " + skillname);
}

/*
 * Sets the invalid locomotion for this command.
 * Parses the string from LUA's. Format: "4,12,13,"
 */
void QueueCommand::setInvalidLocomotions(const String& lStr) {
	StringTokenizer tokenizer(lStr);
	tokenizer.setDelimeter(",");

	String token = "";
	while (tokenizer.hasMoreTokens()) {
		tokenizer.getStringToken(token);

		if(!token.isEmpty())
			invalidLocomotion.add(Integer::valueOf(token));
	}
}


/*
 * Checks each invalid locomotion with the player's current locomotion
 * Allows DW mounted combat for whitelisted commands
 */
bool QueueCommand::checkInvalidLocomotions(CreatureObject* creature) const {
	int currentLocomotion = creature->getLocomotion();
	bool blockedByMount = false;

	for (int i = 0; i < invalidLocomotion.size(); ++i) {
		int invalidLoco = invalidLocomotion.get(i);
		if (invalidLoco == currentLocomotion) {
			// Check if this is a mount-related locomotion
			if (invalidLoco == CreatureLocomotion::RIDINGCREATURE || invalidLoco == CreatureLocomotion::DRIVINGVEHICLE) {
				blockedByMount = true;
			} else {
				// Non-mount locomotion violation - fail immediately
				return false;
			}
		}
	}

	// If blocked only by mount locomotion, check for DW jetpack exception
	if (blockedByMount) {
		if (isDwMountedCombatAllowed(creature)) {
			return true;
		}
		return false;
	}

	return true;
}

/*
 * Check if DW mounted combat is allowed for this command
 * Returns true if player is on DW jetpack and command is in whitelist
 */
bool QueueCommand::isDwMountedCombatAllowed(CreatureObject* creature) const {
	if (creature == nullptr || server == nullptr) {
		return false;
	}

	// Must be riding a mount/vehicle
	if (!creature->isRidingMount()) {
		return false;
	}

	ZoneServer* zoneServer = server->getZoneServer();
	if (zoneServer == nullptr) {
		return false;
	}

	DeathWatchManager* dwManager = zoneServer->getDeathWatchManager();
	if (dwManager == nullptr) {
		return false;
	}

	// Check if player is on a DW jetpack and command is whitelisted
	return dwManager->isOnDeathWatchJetpack(creature) &&
	       dwManager->isCommandAllowedWhileMounted(name);
}

/*
*	Checks cell access for the player creature if the target is in a cell
*/
bool QueueCommand::playerEntryCheck(CreatureObject* creature, TangibleObject* target) const {
	if (creature == nullptr || target == nullptr) {
		return false;
	}

	uint64 creoParentID = creature->getParentID();
	uint64 tarParentID = target->getParentID();

	if (!creature->isPlayerCreature() || tarParentID == 0) {
		return true;
	}

	if (creoParentID != tarParentID) {
		Reference<CellObject*> targetCell = target->getParent().get().castTo<CellObject*>();

		if (targetCell != nullptr) {
			ManagedReference<SceneObject*> parentSceneObject = targetCell->getParent().get();

			if (parentSceneObject != nullptr) {
				BuildingObject* buildingObject = parentSceneObject->asBuildingObject();

				if (buildingObject != nullptr && !buildingObject->isAllowedEntry(creature)) {
					return false;
				}
			}

			const ContainerPermissions* perms = targetCell->getContainerPermissions();

			// This portion of the check is specific for locked dungeons doors since they do not inherit perms from parent
			if (!perms->hasInheritPermissionsFromParent() && (creature->getRootParent() == target->getRootParent())) {
				if (!targetCell->checkContainerPermission(creature, ContainerPermissions::WALKIN)) {
					return false;
				}
			}
		}
	}
	return true;
}

void QueueCommand::onStateFail(CreatureObject* creature, uint32 actioncntr) const {
	if (!addToQueue) { // Infinity:  Refactored
		return;
	}

	uint64 creatureStateBitmask = creature->getStateBitmask();
	uint64 invalidStates = stateMask & creatureStateBitmask;

	if (invalidStates > 0) { // Infinity:  Only search if the creature has at least one invalid state
		uint64 state = 1;
		for (int num = 0; num < 34; num++, state <<= 1) {
			if (invalidStates & state) {
				creature->clearQueueAction(actioncntr, 0, 5, num);
				return;
			}
		}
	} else {
		StringBuffer msg;
		msg << "Unknown invalid state in onStateFail.  Additional information below:" << endl;
		msg << "  CreatureOID: " << creature->getObjectID() << "  Creature name: " << creature->getDisplayedName() << "  Command: " << name;
		msg << "  Invalid states: 0x" << String::hexvalueOf(invalidStates) << "  Statemask: 0x" << String::hexvalueOf(stateMask) << "  CreatureStateBitmask: 0x" << String::hexvalueOf(creatureStateBitmask) << endl;
		msg << "  Current Creature States: ";

		auto creatureState = CreatureState::instance();
		if (creatureState != nullptr && creatureStateBitmask > 0) {
			uint64 state = 1;
			for (int num = 0; num < 34; num++, state <<= 1) {
				if (creatureStateBitmask & state) {
					msg << " " << creatureState->getName(state, false).toUpperCase();
				}
			}
		} else {
			msg << "NO STATES FOUND";
		}
		creature->error(msg.toString());
	}
}

void QueueCommand::onLocomotionFail(CreatureObject* creature, uint32 actioncntr) const {
	if (!checkInvalidLocomotions(creature))
		creature->clearQueueAction(actioncntr, 0, 1, creature->getLocomotion());
}

/*
 * Unsuccessful command completion alerts the player of the invalid state
 */
void QueueCommand::onFail(uint32 actioncntr, CreatureObject* creature, uint32 errorNumber) const {
	StringIdChatParameter prm;
	switch (errorNumber) {
	case INVALIDSYNTAX:
		creature->sendSystemMessage(getSyntax());
		if (addToQueue)
			creature->clearQueueAction(actioncntr);
		break;
	case INVALIDSTATE:
		onStateFail(creature, actioncntr);
		break;
	case INVALIDLOCOMOTION:
		onLocomotionFail(creature, actioncntr);
		break;
	case INVALIDTARGET:
		if (addToQueue)
			creature->clearQueueAction(actioncntr, 0, 3, 0);
		break;
	case INVALIDWEAPON: { // this only gets returned from combat commands
		ManagedReference<WeaponObject*> weapon = creature->getWeapon();
		int attackType = -1;

		if (weapon != nullptr) {
			attackType = weapon->getAttackType();
		}

		switch (attackType) {
		case SharedWeaponObjectTemplate::RANGEDATTACK:
			creature->sendSystemMessage("@cbt_spam:no_attack_ranged_single");
			break;
		case SharedWeaponObjectTemplate::MELEEATTACK:
			creature->sendSystemMessage("@cbt_spam:no_attack_melee_single");
			break;
		default:
			creature->sendSystemMessage("@cbt_spam:no_attack_wrong_weapon"); // Can't be done with this weapon
			break;
		}

		if (addToQueue)
			creature->clearQueueAction(actioncntr);
		break;
	}
	case TOOFAR:
		if (addToQueue)
			creature->clearQueueAction(actioncntr, 0, 4, 0);
		break;

	case NOJEDIARMOR:
		creature->sendSystemMessage("@jedi_spam:not_with_armor"); // You cannot use Force powers or lightsaber abilities while wearing armor.
		if (addToQueue)
			creature->clearQueueAction(actioncntr);

		break;

	case NOSTACKJEDIBUFF:
		creature->sendSystemMessage("@jedi_spam:force_buff_present"); // You are already have a similar Force enhancement active.
		if (addToQueue)
			creature->clearQueueAction(actioncntr);

		break;

	case ALREADYAFFECTEDJEDIPOWER:
			creature->sendSystemMessage("@jedi_spam:power_already_active"); // This target is already affected by that power.
			if (addToQueue)
				creature->clearQueueAction(actioncntr);

			break;

	case NOPRONE:
		if (addToQueue)
			creature->clearQueueAction(actioncntr, 0, 1, 7);

		break;

	case NOKNEELING:
		if (addToQueue)
			creature->clearQueueAction(actioncntr, 0, 1, 4);

		break;
	case INSUFFICIENTPERMISSION:
		creature->sendSystemMessage("@error_message:insufficient_permissions"); //You do not have sufficient permissions to perform the requested action.

		if (addToQueue)
			creature->clearQueueAction(actioncntr);
		break;
	case TOOCLOSE:
		prm.setStringId("combat_effects", "prone_ranged_too_close");
		creature->sendSystemMessage(prm);

		if (addToQueue)
			creature->clearQueueAction(actioncntr);
		break;
	case INSUFFICIENTHAM:
		prm.setStringId("cbt_spam", "pool_drain_fail_single");
		creature->sendSystemMessage(prm);

		if (addToQueue)
			creature->clearQueueAction(actioncntr);
		break;
	default:
		if (addToQueue)
			creature->clearQueueAction(actioncntr);
		break;
	}
}

void QueueCommand::onComplete(uint32 actioncntr, CreatureObject* player, float commandDuration) const {
	if (!player->isPlayerCreature())
		return;

	int weakness = player->getSkillMod("weaken_delay");
	if (System::random(100) < weakness && player->checkCooldownRecovery("weaken_delay") && player->checkCooldownRecovery("nextAttackDelayRecovery")) { //apply delay
		player->setNextAttackDelay(nullptr, "weaken_delay", 100, 2);
		player->updateCooldownTimer("weaken_delay", 7000); //random weaken delay can only happen once every 7 seconds.
	}

	if (addToQueue) {
		player->clearQueueAction(actioncntr, commandDuration);
	}
}

int QueueCommand::doCommonMedicalCommandChecks(CreatureObject* creature) const {
	if (!checkStateMask(creature))
		return INVALIDSTATE;

	if (!checkInvalidLocomotions(creature))
		return INVALIDLOCOMOTION;

	if (creature->isProne() || creature->isMeditating() || creature->isSwimming()) {
		creature->sendSystemMessage("@error_message:wrong_state"); //You cannot complete that action while in your current state.
		return GENERALERROR;
	}

	if (creature->isRidingMount()) {
		creature->sendSystemMessage("@error_message:survey_on_mount"); //You cannot perform that action while mounted on a creature or driving a vehicle.
		return GENERALERROR;
	}

	return SUCCESS;
}

bool QueueCommand::checkForArenaDuel(CreatureObject* target) const {
	FrsManager* frsManager = server->getZoneServer()->getFrsManager();

	if (frsManager == nullptr)
		return false;

	if (!frsManager->isFrsEnabled())
		return false;

	return frsManager->isPlayerFightingInArena(target->getObjectID());
}

void QueueCommand::checkForTef(CreatureObject* creature, CreatureObject* target) const {
	if (creature == nullptr || target == nullptr)
		return;

	if (!creature->isPlayerCreature() || creature == target) {
		return;
	}

	PlayerObject* ghost = creature->getPlayerObject().get();

	if (ghost == nullptr) {
		return;
	}

	if (target->isPet()) {
		ManagedReference<CreatureObject*> owner = target->getLinkedCreature().get();

		if (owner == nullptr)
			return;

		target = owner;
	}

	if (target->isPlayerCreature() && !CombatManager::instance()->areInDuel(creature, target)) {
		PlayerObject* targetGhost = target->getPlayerObject().get();

		if (targetGhost != nullptr) {
			if (ConfigManager::instance()->useCovertOvertSystem()) {
				int healerStatus = creature->getFactionStatus();
				int targetStatus = target->getFactionStatus();

				if (creature->getFaction() == target->getFaction() && ((healerStatus >= FactionStatus::COVERT && targetGhost->hasGcwTef()) || (healerStatus == FactionStatus::COVERT && targetStatus == FactionStatus::OVERT))) {
					ghost->updateLastGcwPvpCombatActionTimestamp();
				}
			} else {
				if (target->getFactionStatus() == FactionStatus::OVERT && targetGhost->hasPvpTef()) {
					ghost->updateLastCombatActionTimestamp(false, true, false, true);  //Infinity:  Crackdown, PVP, BH, healOnly
				}
			}

			if (targetGhost->isInPvpArea(true)) {
				ghost->updateLastPvpAreaCombatActionTimestamp();
			}
		}
	}
}

bool QueueCommand::checkCooldown(CreatureObject* creo) const {
	if (cooldown == 0) {
		return true;
	}

	if (creo == nullptr) {
		error() << "checkCooldown() creature is nullptr";
		return true;
	}

	// Provide sane default but should have been set in setCooldown()
	auto cooldownKey = cooldownName.isEmpty() ? "command_" + name : cooldownName;

	if (creo->checkCooldownRecovery(cooldownKey)) {
		creo->addCooldown(cooldownKey, cooldown);
		return true;
	}

	uint32 remain = 0;
	auto cooldownTime = creo->getCooldownTime(cooldownKey);

	if (cooldownTime != nullptr) {
		Time now;
		remain = now.miliDifference(*cooldownTime) / 1000;
	}

	String logMsg;

	if (!cooldownString.isEmpty()) {
		if (cooldownString.charAt(0) == '@') {
			StringIdChatParameter stringIdMsg(cooldownString);
			stringIdMsg.setDI(remain);
			logMsg = stringIdMsg.toString();
			creo->sendSystemMessage(stringIdMsg);
		} else {
			logMsg = cooldownString.replaceFirst("%DI", String::valueOf(remain));
			creo->sendSystemMessage(logMsg);
		}
	} else {
		StringBuffer buf;

		buf << "You can't do /" << name << " again yet, please wait";

		if (remain > 0) {
			buf << " " << remain << (remain == 1 ? " second" : " seconds");
		}

		buf << " before trying again.";
		logMsg = buf.toString();
		creo->sendSystemMessage(logMsg);
	}

	creo->info(admin) << "checkCooldown /" << name << ": remain=" << remain << "; msg=\"" << logMsg << "\"";

	return false;
}
