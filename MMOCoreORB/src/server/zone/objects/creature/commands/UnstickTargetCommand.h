/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

// Infinity: /unstickTarget — free a stuck NPC from collision geometry.
//
// Context: NPCs occasionally spawn slightly inside walls, city props, cave
// meshes, etc. Players can target them but fail the combat line-of-sight
// check (@cbt_spam:los_fail) and can't interact. Instead of filing a ticket,
// players /unstickTarget the stuck creature and it's warped into the open.
//
// Two-step interaction:
//
//   1. Player attempts to attack the stuck target. The game's authoritative
//      LOS check in CombatQueueCommand / HeavyWeaponQueueCommand /
//      ForcePowersQueueCommand fires @cbt_spam:los_fail ("You lost sight of
//      your target.") and records a 60-second per-target cooldown on the
//      player named "los_fail_<targetOID>".
//
//   2. Player targets the same NPC and runs /unstickTarget. This command
//      reads the cooldown as its stuck signal — if active, the game itself
//      has confirmed the target is blocked by geometry, so we warp the NPC
//      to the player's current position.
//
// This avoids any ad-hoc collision math in this command. The game's LOS
// raycast is the only authoritative signal and it's already been proven to
// trigger (the player just saw the failure message). If the player hasn't
// attempted an attack first, we tell them to.
//
// Undocumented staff escape hatch: /unstickTarget force ignores all safeguards
// and warps the target to the staff member's position regardless of stuck
// state, distance, combat, cell, etc. Only callable by isPrivileged() ghosts.
// The "force" keyword is deliberately NOT mentioned in any player-facing text.
//
// Tunables (cooldown, max range, cooldownString) live in
// bin/scripts/commands/unstickTarget.lua.
//
// Logging: every successful unstick writes to the Lua log (via the Lua
// engine's info channel — NOT the ERROR channel; a successful unstick isn't
// an error and we don't want it tripping log-watchers that wake on ERROR).
// Lines are prefixed with [UNSTICK_TARGET] or [UNSTICK_TARGET_FORCE] so
// screenplay authors can grep for chronically bad spawn points.
//
// Note: the LuaEngine's default file-log level is ERROR (see
// DirectorManager.cpp setLogLevel). For these INFO-level lines to appear in
// log/lua.log, set `Core3.LuaEngine.LogLevel = INFO` in config-local.lua;
// otherwise they fall through to the global core3.log via the global-log
// path. Either way they are non-ERROR and never emit to stderr.

#ifndef UNSTICKTARGETCOMMAND_H_
#define UNSTICKTARGETCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/objects/creature/ai/AiAgent.h"
#include "server/zone/objects/cell/CellObject.h"
#include "server/zone/objects/player/PlayerObject.h"
#include "server/zone/Zone.h"
#include "server/zone/ZoneServer.h"
#include "server/zone/managers/director/DirectorManager.h"

class UnstickTargetCommand : public QueueCommand {
public:

	UnstickTargetCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {
	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (creature == nullptr)
			return GENERALERROR;

		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		// --- Parse "force" flag (undocumented staff escape hatch) ---
		String argStr = arguments.toString();
		argStr = argStr.toLowerCase();
		bool force = (argStr.indexOf("force") != -1);

		// --- Resolve the target ---
		// Custom commands not in command_tables_shared.iff receive target=0
		// from the client. Fall back to the player's UI target, same pattern
		// as /dumpTargetInformation, /snoop, /gmrevive, /paymaintenance, etc.
		uint64 targetID = target;
		if (targetID == 0)
			targetID = creature->getTargetID();

		if (targetID == 0) {
			creature->sendSystemMessage("You must target the NPC you want to unstick.");
			return INVALIDTARGET;
		}

		ZoneServer* zoneServer = creature->getZoneServer();
		if (zoneServer == nullptr)
			return GENERALERROR;

		ManagedReference<SceneObject*> targetObject = zoneServer->getObject(targetID);
		if (targetObject == nullptr) {
			creature->sendSystemMessage("Your target is no longer valid.");
			return INVALIDTARGET;
		}

		if (targetObject->getObjectID() == creature->getObjectID()) {
			creature->sendSystemMessage("You can't unstick yourself with this command. Try /unstick.");
			return INVALIDTARGET;
		}

		// Vendors (player-owned tangible droids/kiosks AND NPC vendors) are
		// intentionally placed and must never be moved by players — their
		// position is meaningful to owners, cities, and quest logic. Check
		// before the isCreatureObject guard so this message is specific
		// regardless of whether the vendor is a creature, droid, or terminal.
		// Staff can still use /unstickTarget force for deliberate intervention.
		if (targetObject->isVendor()) {
			creature->sendSystemMessage("Vendors can't be unstuck — please file a support ticket instead.");
			return INVALIDTARGET;
		}

		if (!targetObject->isCreatureObject()) {
			creature->sendSystemMessage("You can only unstick creatures.");
			return INVALIDTARGET;
		}

		CreatureObject* targetCreature = cast<CreatureObject*>(targetObject.get());
		if (targetCreature == nullptr)
			return INVALIDTARGET;

		if (targetCreature->isPlayerCreature()) {
			creature->sendSystemMessage("You can't unstick other players.");
			return INVALIDTARGET;
		}

		Zone* zone = creature->getZone();
		if (zone == nullptr)
			return GENERALERROR;

		// --- Force path: undocumented staff-only escape hatch ---
		if (force) {
			PlayerObject* ghost = creature->getPlayerObject();
			if (ghost == nullptr || !ghost->isPrivileged()) {
				// Non-staff who typed /unstickTarget force get the same
				// "try attacking first" nudge as any normal user — we
				// deliberately don't hint that force exists.
				creature->sendSystemMessage("Try attacking the target first. If you see 'You lost sight of your target', you can then /unstickTarget to free it.");
				return GENERALERROR;
			}

			warpTargetToPlayer(creature, targetCreature, zone, "[UNSTICK_TARGET_FORCE] admin", true);
			creature->sendSystemMessage("You forcibly warped your target to your position.");
			return SUCCESS;
		}

		// --- Normal path safeguards ---

		if (targetCreature->isPet()) {
			creature->sendSystemMessage("You can't unstick pets — have the owner re-summon it.");
			return INVALIDTARGET;
		}

		if (targetCreature->isInCombat()) {
			creature->sendSystemMessage("You can't unstick a creature that's in combat.");
			return INVALIDTARGET;
		}

		// Distance gate — 6m belt-and-suspenders in case the Lua
		// maxRangeToTarget isn't enforced for custom commands.
		Vector3 playerWorld = creature->getWorldPosition();
		Vector3 targetWorld = targetCreature->getWorldPosition();
		float ddx = playerWorld.getX() - targetWorld.getX();
		float ddy = playerWorld.getY() - targetWorld.getY();
		if (ddx * ddx + ddy * ddy > 36.0f) { // 6m squared
			creature->sendSystemMessage("You must be within 6 meters of the stuck creature.");
			return GENERALERROR;
		}

		// Must be in the same environment — we can't drag a mob out of a
		// cave from outside.
		ManagedReference<SceneObject*> creatureRoot = creature->getRootParent();
		ManagedReference<SceneObject*> targetRoot = targetCreature->getRootParent();
		if (creatureRoot != targetRoot) {
			creature->sendSystemMessage("You must be in the same cell as the NPC.");
			return INVALIDTARGET;
		}

		// --- Stuck signal: recent LOS fail cooldown on this target ---
		// CombatQueueCommand / HeavyWeaponQueueCommand / ForcePowersQueueCommand
		// all record "los_fail_<targetID>" on the player for 60s when their
		// authoritative LOS check blocks an attack. That's our only stuck
		// signal — the game itself said so.
		StringBuffer losKeyBuf;
		losKeyBuf << "los_fail_" << targetID;
		String losKey = losKeyBuf.toString();
		bool recentLosFail = !creature->checkCooldownRecovery(losKey);

		if (!recentLosFail) {
			creature->sendSystemMessage("Try attacking the target first. If you see 'You lost sight of your target', you can then /unstickTarget to free it.");
			return GENERALERROR;
		}

		// --- Warp and clean up ---
		warpTargetToPlayer(creature, targetCreature, zone, "[UNSTICK_TARGET] player", false);

		// Clear the LOS fail hint so a second unstick on the same target
		// requires the player to trigger a fresh LOS fail first.
		creature->addCooldown(losKey, 0);

		creature->sendSystemMessage("You unstick your target.");
		return SUCCESS;
	}

private:

	// Warp `targetCreature` to the `player`'s current position + cell, lock
	// the target, and emit a log line via the Lua info channel with the
	// given prefix ("[UNSTICK_TARGET] player" or "[UNSTICK_TARGET_FORCE] admin").
	static void warpTargetToPlayer(CreatureObject* player, CreatureObject* targetCreature,
	                                Zone* zone, const String& logPrefix, bool isForce) {

		Vector3 dstWorldPos = player->getWorldPosition();
		Vector3 dstLocalPos = player->getPosition();

		uint64 destCellID = 0;
		ManagedReference<SceneObject*> playerParent = player->getParent().get();
		if (playerParent != nullptr && playerParent->isCellObject()) {
			destCellID = playerParent->getObjectID();
		}

		float oldX = targetCreature->getPositionX();
		float oldY = targetCreature->getPositionY();
		float oldZ = targetCreature->getPositionZ();

		float newX = (destCellID != 0) ? dstLocalPos.getX() : dstWorldPos.getX();
		float newY = (destCellID != 0) ? dstLocalPos.getY() : dstWorldPos.getY();
		float newZ = (destCellID != 0) ? dstLocalPos.getZ() : dstWorldPos.getZ();

		{
			Locker clocker(targetCreature, player);
			targetCreature->teleport(newX, newZ, newY, destCellID);
		}

		StringBuffer logMsg;
		logMsg << logPrefix << "=" << player->getFirstName()
		       << " target=\"";
		SharedObjectTemplate* tmpl = targetCreature->getObjectTemplate();
		logMsg << (tmpl != nullptr ? tmpl->getFullTemplateString() : String("<unknown>"));
		logMsg << "\" name=\"" << targetCreature->getDisplayedName() << "\""
		       << " from=(" << oldX << ", " << oldZ << ", " << oldY << ")"
		       << " to=(" << newX << ", " << newZ << ", " << newY << ")"
		       << " zone=" << zone->getZoneName()
		       << " cell=" << String::valueOf((uint64)destCellID);

		logToLua(logMsg.toString());
	}

	// Route a log line through the Lua engine's INFO channel (not ERROR) so
	// successful unsticks don't trip ERROR-watching log monitors. Screenplay
	// authors still grep `[UNSTICK_TARGET]` — see header comment for where
	// the line actually lands depending on LuaEngine.LogLevel config.
	// Falls back to console.info on any exception.
	static void logToLua(const String& msg) {
		try {
			Lua* lua = DirectorManager::instance()->getLuaInstance();
			if (lua != nullptr) {
				lua->info(msg, true);
				return;
			}
		} catch (...) {
		}
		Logger::console.info(msg);
	}

};

#endif //UNSTICKTARGETCOMMAND_H_
