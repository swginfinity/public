/*
		Copyright <SWGEmu>
	See file COPYING for copying conditions.*/

/*
* CommandQueue.cpp
*
* Created on: Jan. 1, 2022
* Author: Hakry
*/

#include "CommandQueue.h"
#include "server/zone/managers/objectcontroller/ObjectController.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/objects/creature/commands/QueueCommand.h"
#include "server/zone/ZoneServer.h"
#include "conf/ConfigManager.h"
#include "server/zone/objects/creature/variables/CommandQueueActionVector.h"
#include "server/zone/packets/object/CommandQueueRemove.h"
#include "server/zone/packets/object/CommandQueueAdd.h"
#include "templates/params/ObserverEventType.h"
// Infinity: Custom includes
#include "server/zone/objects/creature/ai/AiAgent.h"
#include "server/zone/managers/creature/CreatureTemplateManager.h"
#include "server/zone/objects/player/PlayerObject.h"
#include "server/zone/managers/stringid/StringIdManager.h"

CommandQueue::CommandQueue(CreatureObject* creature) {
#ifdef DEBUG_QUEUE
	info(true) << __FUNCTION__ << "() called on " << toString();
#endif // DEBUG_QUEUE
	if (creature == nullptr) {
		return;
	}

	weakCreature = creature;
	queueTask = new CommandQueueTask(this);
	state = WAITING;

	clearQueueActions(false);

	StringBuffer logName;
	logName << "CommandQueue[" << creature->getDisplayedName() << "]-[" << creature->getObjectID() << "]";

	setLoggingName(logName.toString());

#ifdef DEBUG_QUEUE
	runNumber = 1;
	info(true) << __FUNCTION__ << "(" << creature << ") called on " << toString();
#endif // DEBUG_QUEUE
}

CommandQueue::~CommandQueue() {
#ifdef DEBUG_QUEUE
	info(true) << __FUNCTION__ << "() called on " << toString();
#endif // DEBUG_QUEUE
	if (queueTask != nullptr) {
		queueTask->cancel();
		queueTask = nullptr;
	}
}

void CommandQueue::removeAction(CommandQueueAction* actionToDelete) {
	if (actionToDelete == nullptr)
		return;

	Locker guard(&queueMutex);

	for (int i = 0; i < queueVector.size(); i++) {
		CommandQueueAction* action = queueVector.get(i);

		if (action == actionToDelete) {
			queueVector.remove(i);

#ifdef DEBUG_QUEUE
			info(true) << " Queue action - " << action->getCommand() << " - REMOVED from " << toString();
#endif
			break;
		}
	}
}

CommandReference<CommandQueueAction*> CommandQueue::getNextAction() {
	auto creature = weakCreature.get();

	if (creature == nullptr || queueVector.size() == 0) {
		return nullptr;
	}

	auto zoneServer = creature->getZoneServer();

	if (zoneServer == nullptr) {
		return nullptr;
	}

	auto objectController = zoneServer->getObjectController();

	if (objectController == nullptr) {
		return nullptr;
	}

	bool hasFront = false;
	int frontAction = -1;

	// Check queue for immediate actions, they get executed first
	for (int i = 0; i < queueVector.size(); i++) {
		auto immediateAction = queueVector.get(i);

		if (immediateAction == nullptr)
			continue;

		auto queueCommand = objectController->getQueueCommand(immediateAction->getCommand());

		if (queueCommand != nullptr) {
			int priority = queueCommand->getDefaultPriority();

			if (priority == QueueCommand::IMMEDIATE && (!creature->hasAttackDelay() || !queueCommand->addToCombatQueue())) {
				return immediateAction;
			}

			if (!hasFront && priority == QueueCommand::FRONT) {
				hasFront = true;
				frontAction = i;
			}
		}
	}

	// No immediate commands, however a front command is present
	if (hasFront && frontAction > -1) {
		return queueVector.get(frontAction);
	}

	// None of the queue actions were immediate, select first command in the queue
	return queueVector.get(0);
}

int CommandQueue::handleRunningState() {
	auto creature = weakCreature.get();

	if (creature == nullptr) {
		return 0;
	}

	Locker guard(&queueMutex);

	// Infinity: Disable for now - targeted debug logging for specific mobs
	bool logMe = false;
	if (creature->isAiAgent()) {
		ManagedReference<AiAgent*> agent = creature->asAiAgent();
		if (agent != nullptr) {
			String mobileName = agent->getCreatureTemplate()->getTemplateName();
			logMe = mobileName == "laen_pieweto" || mobileName == "nabek_naga";
		}
	}
	logMe = false;

	bool isPet = creature->isPet();

#ifdef DEBUG_QUEUE
	info(true) << "CommandQueue - handleRunningState called on " << toString();
#endif

	if (queueVector.size() <= 0)
		return 0;

	ZoneServer* zoneServer = creature->getZoneServer();

	if (zoneServer == nullptr) {
		return 0;
	}

	auto objectController = zoneServer->getObjectController();

	if (objectController == nullptr) {
		return 0;
	}

	auto action = getNextAction();

	if (action == nullptr) {
		return 0;
	}

	guard.release();

	float time = 0.f;

	auto queueCommand = objectController->getQueueCommand(action->getCommand());

	if (queueCommand == nullptr)
		return time;

	int priority = queueCommand->getDefaultPriority();

#ifdef DEBUG_QUEUE
	info(true) << "Command Name: " << queueCommand->getName() << " with a priority of " << priority;
#endif

	Locker lock(creature);

	Time* nextActionTime = creature->getNextActionTime();

	if (nextActionTime == nullptr)
		return time;

	Time currTime;
	int remainingActionTime = nextActionTime->getMiliTime() - currTime.getMiliTime();

	// Auto attack timer only applies to players
	if (creature->isPlayerCreature()) {

#ifdef DEBUG_QUEUE
		info(true) << "Remaining action time = " << remainingActionTime;
#endif

		CooldownTimerMap* cooldownTimerMap = creature->getCooldownTimerMap();

		if (cooldownTimerMap != nullptr && !cooldownTimerMap->isPast("autoAttackDelay")) {
			const Time* autoAttackTime = cooldownTimerMap->getTime("autoAttackDelay");

			if (autoAttackTime != nullptr) {
				auto autoTime = fabs(autoAttackTime->miliDifference());

				if (autoTime > remainingActionTime) {
#ifdef DEBUG_QUEUE
					info(true) << "Auto attack delay is > than remainingActionTime adding to delay = " << autoTime;
#endif
					nextActionTime->addMiliTime(autoTime);

					if (priority == QueueCommand::NORMAL)
						return autoTime;
				}
			}
		}
	}

	if (priority == QueueCommand::NORMAL && nextActionTime->isFuture()) {
#ifdef DEBUG_QUEUE
		info(true) << "Next normal action is future returning time of " << remainingActionTime;
#endif
		return remainingActionTime;
	}

	if (queueCommand->addToCombatQueue() && ((creature->hasPostureChangeDelay() && priority != QueueCommand::IMMEDIATE) || creature->hasAttackDelay())) {
		return DEFAULTTIME;
	}

	// Infinity: AutoStand detection (log-only, Phase 1) — a combat-queue command
	// (addToCombatQueue()) reaching activateCommand while postureChangeDelay is
	// active AND the player is knocked down or incapacitated. The postureChangeDelay
	// check alone is too broad — it also fires on postureUP/DOWN transitions (2.5s
	// delay on normal crouched/prone states), generating false positives. Only
	// KNOCKEDDOWN/INCAPACITATED postures are the actual attack surface. Equilibrium
	// TKAs grant posture but don't fire combat-queue cmds during the delay, so this
	// does NOT trip on equilibrium. Per-player throttle.
	// Infinity 2026-05-30: gate the log-only detector behind a config flag (default false) so
	// live core3.log stays silent (Phase-1 disable per R2). Set Core3.AutoStandDetectLog=true
	// only during a TC collection window. Placed last in the && chain so the getBool lookup
	// only runs in the already-rare knocked-down-firing-combat-cmd case.
	if (creature->isPlayerCreature() && queueCommand->addToCombatQueue() && creature->hasPostureChangeDelay()
	    && (creature->isKnockedDown() || creature->isIncapacitated())
	    && ConfigManager::instance()->getBool("Core3.AutoStandDetectLog", false)) {
		if (creature->checkCooldownRecovery("autoStandLog")) {
			auto ghost = creature->getPlayerObject();
			uint32 accountID = (ghost != nullptr) ? ghost->getAccountID() : 0;
			creature->warning() << "[AutoStand] " << creature->getDisplayedName()
				<< " accountID=" << accountID
				<< " cmd=" << queueCommand->getQueueCommandName()
				<< " priority=" << priority
				<< " posture=" << (int)creature->getPosture()
				<< " zone=" << (creature->getZone() != nullptr ? creature->getZone()->getZoneName() : "?");
			creature->updateCooldownTimer("autoStandLog", 3600000);
		}
	}

	try {
		time = objectController->activateCommand(creature, action->getCommand(), action->getActionCounter(), action->getTarget(), action->getArguments());
	} catch (const Exception &e) {
		Logger::console.error() << "CommandQueue ERROR -- Creature: " << creature->getDisplayedName() << " ID: " << creature->getObjectID() << " Command Name: " << queueCommand->getName() << " Issue: " << e.getMessage();
		e.printStackTrace();
	}

#ifdef DEBUG_QUEUE
	info(true) << "Command Queue activateCommand called -- time = " << time << " ---- Removing Action";
#endif

	removeAction(action);

	if (priority == QueueCommand::NORMAL)
		nextActionTime->updateToCurrentTime();

	if (creature->isAiAgent()) {
		if (priority == QueueCommand::NORMAL) {

			// Infinity: Going to get weaponSpeed from agent, and not assume global 2.0 speed
			ManagedReference<AiAgent*> agent = creature->asAiAgent();
			if (agent != nullptr) {
				time = agent->getWeaponSpeed();
			} else {
				time = CreatureTemplateManager::instance()->getGlobalAttackSpeedOverride();
			}

			uint64 miliTime = time * 1000;
#ifdef DEBUG_QUEUE
			info(true) << "Command Delay Time > 0 -- adding to next action time " << miliTime;
#endif
			nextActionTime->addMiliTime(miliTime);

		} else {
			return DEFAULTTIME;
		}
	} else if (time > 0) {
		uint64 miliTime = time * 1000;
#ifdef DEBUG_QUEUE
		info(true) << "Command Delay Time > 0 -- adding to next action time " << miliTime;
#endif

		nextActionTime->addMiliTime(miliTime);
	}

	return time * 1000;
}

void CommandQueue::run() {
	auto creature = weakCreature.get();

	if (creature == nullptr)
		return;

#ifdef DEBUG_QUEUE
	auto debugMsg = [&]() -> LoggerHelper {
		auto msg = info(true);
		msg << "CommandQueue::run() " << toString() << ": ";
		return msg;
	};

	runNumber++;
	debugMsg() << "START";
	lastRun.updateToCurrentTime();
	Timer profile;
	profile.start();
#endif // DEBUG_QUEUE

	switch (state) {
		case WAITING: {
			if (queueVector.size() == 0) {
				break;
			}

			state = RUNNING;
		}
		case RUNNING: {
			if (queueVector.size() == 0) {
				state = WAITING;
				break;
			}

			int delay = handleRunningState();

			Time* nextActionTime = creature->getNextActionTime();

			if (creature->hasAttackDelay() || creature->hasPostureChangeDelay() || (nextActionTime != nullptr && creature->isPlayerCreature() && nextActionTime->isFuture())) {
				state = DELAY;
				delay = DEFAULTTIME;
			}

			if (delay <= 0)
				delay = DEFAULTTIME;

			if (creature->isAiAgent())
				queueTask->reschedule(delay);
			else
				queueTask->reschedule(DEFAULTTIME);

			break;
		}
		case DELAY: {
			bool attackDelay = creature->hasAttackDelay();
			bool postureDelay = creature->hasPostureChangeDelay();

			Time* nextActionTime = creature->getNextActionTime();
			int actions = checkActions(creature);

			if ((!attackDelay && !postureDelay && nextActionTime != nullptr && !nextActionTime->isFuture()) || actions == QueueCommand::NOCOMBATQUEUE || actions == QueueCommand::FRONT || (!attackDelay && actions == QueueCommand::IMMEDIATE)) {
				state = RUNNING;
			}

			queueTask->reschedule(DEFAULTTIME);
		}
		default:
			break;
	}

#ifdef DEBUG_QUEUE
	debugMsg() << "FINISHED after " << Logger::nsToString(profile.stop()) << "; isScheduled=" << queueTask->isScheduled();
#endif // DEBUG_QUEUE
}

int CommandQueue::checkActions(CreatureObject* creature) {
	if (creature == nullptr)
		return 0;

	ZoneServer* zoneServer = creature->getZoneServer();

	if (zoneServer == nullptr) {
		return 0;
	}

	auto objectController = zoneServer->getObjectController();

	if (objectController == nullptr) {
		return 0;
	}

	bool hasImmediate = false;
	bool hasFront = false;

	Locker guard(&queueMutex);

	for (int i = 0; i < queueVector.size(); i++) {
		CommandQueueAction* action = queueVector.get(i);

		if (action == nullptr)
			continue;

		const QueueCommand* queueCommand = objectController->getQueueCommand(action->getCommand());

		if (queueCommand != nullptr) {
			if (!queueCommand->addToCombatQueue())
				return QueueCommand::NOCOMBATQUEUE;

			int priority = queueCommand->getDefaultPriority();

			if (priority == QueueCommand::IMMEDIATE) {
				hasImmediate = true;
			}

			if (priority == QueueCommand::FRONT) {
				hasFront = true;
			}
		}

		// Queue contains both immediate and front commands, stop iteration
		if (hasImmediate && hasFront)
			break;
	}

	if (hasFront)
		return QueueCommand::FRONT;
	else if (hasImmediate)
		return QueueCommand::IMMEDIATE;

	return 0;
}

void CommandQueue::clearQueueAction(unsigned int actioncntr, float timer, unsigned int tab1, unsigned int tab2) {
	auto creature = weakCreature.get();

	if (creature == nullptr || !creature->isPlayerCreature() || actioncntr == 0)
		return;

	creature->sendMessage(new CommandQueueRemove(creature, actioncntr, timer, tab1, tab2));
}

void CommandQueue::enqueueCommand(unsigned int actionCRC, unsigned int actionCount, uint64 targetID, const UnicodeString& arguments, int priority, int compareCounter) {
	auto creature = weakCreature.get();

	if (creature == nullptr) {
#ifdef DEBUG_QUEUE
		info(true) << "CommandQueue - enqueueCommand Called but creature == nullptr: " << toString();
#endif // DEBUG_QUEUE
		return;
	}

#ifdef DEBUG_QUEUE
	info(true) << "CommandQueue - enqueueCommand Called -- " << toString();
#endif // DEBUG_QUEUE

	// Infinity: Disable for now - targeted debug logging for specific mobs
	bool logMe = false;
	if (creature->isAiAgent()) {
		ManagedReference<AiAgent*> agent = creature->asAiAgent();
		if (agent != nullptr) {
			String mobileName = agent->getCreatureTemplate()->getTemplateName();
			logMe = mobileName == "laen_pieweto" || mobileName == "nabek_naga";
		}
	}
	logMe = false;

	auto zoneServer = creature->getZoneServer();

	if (zoneServer == nullptr) {
		return;
	}

	auto objectController = zoneServer->getObjectController();

	if (objectController == nullptr) {
		return;
	}

	Locker guard(&queueMutex);

	const QueueCommand* queueCommand = objectController->getQueueCommand(actionCRC);

	if (queueCommand == nullptr) {
		// Infinity: For Debugging invalid command enqueue
		StringBuffer msg;
		msg << "CommandQueue Invalid enqueueCommand call -- Creature: " << creature->getDisplayedName() << " ID: " << creature->getObjectID() << "  Player: " << (creature->isPlayerCreature() ? "Yes" : "No") << " actionCRC: " << actionCRC << "  actionCount: " << actionCount << "  targetID: " << targetID << endl;

		if (creature->isAiAgent()) {
			auto objectAgent = creature->asAiAgent();

			if (objectAgent != nullptr) {
				auto attackMap = objectAgent->getPrimaryAttackMap();
				String map = "Primary";

				if (attackMap != nullptr && attackMap->size() > 0) {
					msg << map << " Weapon Attack Map" << endl;
					for (int i = 0; i < attackMap->size(); i++) {
						String cmd = attackMap->getCommand(i);
						String formattedName = "";
						if (cmd.isEmpty()) {
							formattedName = "none";
						} else {
							String lookup = "@combat_effects:" + cmd;
							formattedName = StringIdManager::instance()->getStringId(lookup.hashCode()).toString();
						}

						msg << "  Attack #" << i + 1 << ":  " << (formattedName != "" ? formattedName : cmd) << endl;
					}
				}
			}
		}

		if (creature->checkCooldownRecovery("CommandQueueCRC")) {
			Logger::console.error(msg.toString());
			creature->updateCooldownTimer("CommandQueueCRC", 1000 * 60 * 60 * 1); // Only log once per hour
		}

		return;
	}

	if (creature->isPlayerCreature() && queueCommand->addToCombatQueue()) {
		creature->notifyObservers(ObserverEventType::COMBATCOMMANDENQUEUED);
	}

	int size = queueVector.size();

	/* Infinity: Save for testing - command queue enqueue debug logging
	if (creature->isPlayerCreature() || creature->isPet()) {
		creature->info(true) << "Enqueue Command " << queueCommand->getQueueCommandName() << ": with priority = " << priority << ", actionCount = " << actionCount <<
			 ", Queue Size = " << size << ", isCombat = " << queueCommand->isCombatCommand() << ", addToCombatQueue = " << queueCommand->addToCombatQueue();
	}
	*/

	if (logMe) {
		creature->info(true) << "Enqueue Command " << queueCommand->getQueueCommandName() << ": with priority = " << priority << ", actionCount = " << actionCount <<
			 ", Queue Size = " << size << ", isCombat = " << queueCommand->isCombatCommand() << ", addToCombatQueue = " << queueCommand->addToCombatQueue();
	}

	if (priority == QueueCommand::NORMAL) {
		if ((creature->isAiAgent() && size >= 5) || size > 15) {
			clearQueueAction(actionCount, 0, 0, 0);
			return;
		}
	}

	CommandReference<CommandQueueAction*> action = new CommandQueueAction(creature, targetID, actionCRC, actionCount, arguments);

	if (compareCounter >= 0)
		action->setCompareToCounter((int)compareCounter);

#ifdef DEBUG_QUEUE
	info(true) << "Enqueuing Command " << queueCommand->getQueueCommandName() << ": with priority - " << priority << " compareCount of " << compareCounter << " Queue Size = " << queueVector.size();
#endif

	if (priority == QueueCommand::FRONT) {
		Time* nextAction = creature->getNextActionTime();

		if (nextAction != nullptr && queueVector.size() > 0 && !nextAction->isPast()) {
			action->setCompareToCounter(queueVector.get(0)->getCompareToCounter() - 1);
		}
	}

	queueVector.put(action.get());

// Infinity: Keep for further command queue testing
#ifdef DEBUG_QUEUE
	int oldSize = queueVector.size();

	creature->info() << "Queue Vector size changed from " << oldSize << " to " << queueVector.size();

	if (creature->isPet()) {
		info(true) << "Enqueue Pet Command " << queueCommand->getQueueCommandName() << ": with priority = " << priority << ", actionCount = " << actionCount <<
			 ", Queue Size = " << size << ", isCombat = " << queueCommand->isCombatCommand() << ", addToCombatQueue = " << queueCommand->addToCombatQueue();

		for (unsigned int i = 0; i < queueVector.size(); i++) {
			CommandQueueAction* command = queueVector.get(i);

			const QueueCommand* queueCommand = objectController->getQueueCommand(command->getCommand());

			info(true) << creature->getDisplayedName() << " -- Command #" << i << " : " << queueCommand->getQueueCommandName();
		}
	}

	if (creature->isPlayerCreature()) {
		info(true) << "Enqueue Player Command " << queueCommand->getQueueCommandName() << ": with priority = " << priority << ", actionCount = " << actionCount <<
				", Queue Size = " << size << ", isCombat = " << queueCommand->isCombatCommand() << ", addToCombatQueue = " << queueCommand->addToCombatQueue();
	}
#endif

	if (state == WAITING && !queueTask->isScheduled()) {
#ifdef DEBUG_QUEUE
		info(true) << "Scheduling for DEFAULTTIME";
#endif // DEBUG_QUEUE
		queueTask->schedule(DEFAULTTIME);
	}

#ifdef DEBUG_QUEUE
	info(true) << "enqueueCommand finished: " << toString();
#endif // DEBUG_QUEUE
}

void CommandQueue::sendCommand(uint32 crc, const UnicodeString& args, uint64 targetID, int priority) {
	auto creature = weakCreature.get();

	if (creature == nullptr)
		return;

	uint32 nextCounter = creature->incrementLastActionCounter();

	CommandQueueAdd* msg = new CommandQueueAdd(creature, crc, nextCounter);
	creature->sendMessage(msg);

	int compareCnt = -1;

	if (queueVector.size() == 0 || priority == QueueCommand::FRONT)
		compareCnt = 0;
	else if (priority == QueueCommand::NORMAL) {
		Locker guard(&queueMutex);
		compareCnt = queueVector.get(queueVector.size() - 1)->getCompareToCounter() + 1;
	}

	enqueueCommand(crc, nextCounter, targetID, args, priority, compareCnt);
}

void CommandQueue::clearQueueActions(bool combatOnly) {
	auto creature = weakCreature.get();

	if (creature == nullptr)
		return;

	Locker guard(&queueMutex);

	for (int i = queueVector.size() - 1; i >= 0; i--) {
		CommandQueueAction* command = queueVector.get(i);

		if (command == nullptr)
			continue;

		if (combatOnly) {
			ZoneServer* zoneServer = creature->getZoneServer();
			if (zoneServer == nullptr)
				continue;

			ObjectController* objectController = zoneServer->getObjectController();
			if (objectController == nullptr)
				continue;

			const QueueCommand* queueCommand = objectController->getQueueCommand(command->getCommand());
			if (queueCommand == nullptr)
				continue;

			uint32 nameCrc = queueCommand->getNameCRC();
			if (!queueCommand->isCombatCommand() && !(nameCrc == STRING_HASHCODE("petattack") || nameCrc == STRING_HASHCODE("petspecialattack"))) {  // Infinity: Be sure to clear these on peace
				continue;
			}
		}

		if (command->getActionCounter() != 0)
			clearQueueAction(command->getActionCounter(), 0, 0, 0);

		queueVector.remove(i);
	}
}

void CommandQueue::deleteQueueAction(uint32 actionCount) {
	Locker guard(&queueMutex);

	for (int i = 0; i < queueVector.size(); ++i) {
		CommandQueueAction* action = queueVector.get(i);

		if (action->getActionCounter() == actionCount) {
			queueVector.remove(i);
			break;
		}
	}
}

String CommandQueue::toString() const {
	auto creo = weakCreature.get();

	StringBuffer buf;

	buf <<"CommandQueue(state=";

	// clang-format off
	switch (state) {
	case NONE:    buf << "NONE";    break;
	case WAITING: buf << "WAITING"; break;
	case RUNNING: buf << "RUNNING"; break;
	case DELAY:   buf << "DELAY";   break;
	}
	// clang-format on

	buf << ", creature=" << (creo != nullptr ? creo->getObjectID() : 0)
#ifdef DEBUG_QUEUE
		<< ", runNumber=" << runNumber << ", lastRunMs=" << lastRun.miliDifference()
#endif // DEBUG_QUEUE
	<< ", queueVector.size=" << queueVector.size() << ", queueTask=" << hex << queueTask;

	if (queueTask != nullptr) {
		buf << "[" << (queueTask->isScheduled() ? "isScheduled" : "!isScheduled") << "]";
	}

	buf << ")";

	return buf.toString();
}
