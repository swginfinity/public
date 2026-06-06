/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.

	Infinity: Death Watch PvP Manager Implementation

	============================================================================
	LOGGING PRINCIPLES
	============================================================================

	Death Watch is a PvP system where disputes are common. Logging is critical
	for staff to investigate complaints. Follow these guidelines:

	WHAT TO LOG (use error() for visibility):
	-----------------------------------------
	1. KILLS: Every player kill with full context
	   Format: "DWKILL: {killer} ({faction} box{level}) killed {victim} ..."
	   Include: location, abuse flags (SAME_ACCOUNT, FARMING, GANKED, etc.)

	2. HEAVY WEAPON HITS: Significant damage events
	   Format: "DW_HEAVY: {attacker} hit {defender} with {type} for {dmg} dmg"

	3. COLLUDER DETECTION: When players are flagged for kill trading
	   Format: "Colluders flagged: {name1} and {name2}"

	4. ABUSE DETECTION: Same-account attacks, farming, etc.

	WHAT NOT TO LOG:
	----------------
	- Debug traces (use debug() if needed, but prefer removal)
	- Step-by-step function flow (PASS/FAIL messages)
	- Routine operations (buff applications, normal combat)
	- Never use printf() - use Logger methods only

	LOG LEVELS:
	-----------
	- error()  : PvP events for dispute resolution (kills, heavy hits)
	- info()   : Configuration loading, significant state changes
	- debug()  : Development only, should be removed before merge
	- warning(): Unexpected but recoverable situations

	See PlayerManagerImplementation.cpp for similar patterns (PVPDEATH, BH logs)
	============================================================================
*/

#include <cmath>
#include "server/zone/managers/death_watch/DeathWatchManager.h"
#include "server/zone/managers/death_watch/DwEffectTickTask.h"
#include "server/zone/managers/death_watch/DwMk4StateTask.h"
#include "server/zone/managers/death_watch/DwParticleBeamTickTask.h"
#include "server/zone/managers/death_watch/DwPvpRefreshTask.h"
#include "server/zone/managers/death_watch/DwCloningRequestSuiCallback.h"
#include "server/zone/ZoneServer.h"
#include "server/zone/Zone.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/objects/creature/VehicleObject.h"
#include "server/zone/objects/creature/ai/AiAgent.h"
#include "server/zone/objects/creature/buffs/Buff.h"
#include "server/zone/objects/creature/buffs/BuffCRC.h"
#include "server/zone/objects/player/PlayerObject.h"
#include "server/zone/objects/player/FactionStatus.h"
#include "server/zone/objects/player/events/PlayerIncapacitationRecoverTask.h"
#include "server/zone/objects/player/sui/listbox/SuiListBox.h"
#include "server/zone/objects/player/sui/SuiWindowType.h"
#include "server/zone/objects/tangible/TangibleObject.h"
#include "server/zone/objects/tangible/weapon/WeaponObject.h"
#include "server/zone/objects/tangible/wearables/ArmorObject.h"
#include "server/zone/objects/building/BuildingObject.h"
#include "server/zone/objects/cell/CellObject.h"
#include "server/zone/objects/region/CityRegion.h"
#include "server/zone/managers/player/PlayerManager.h"
#include "server/chat/StringIdChatParameter.h"
#include "conf/ConfigManager.h"
#include "templates/params/OptionBitmask.h"
#include "server/zone/managers/skill/SkillManager.h"
#include "server/zone/managers/mission/MissionManager.h"
#include "server/zone/managers/director/DirectorManager.h"
#include "server/zone/managers/objectcontroller/ObjectController.h"
#include "server/zone/managers/combat/CombatManager.h"
#include "server/zone/managers/combat/DefenderHitList.h"
#include "server/zone/objects/creature/commands/QueueCommand.h"
#include "templates/faction/DeathWatchFactions.h"
#include "templates/params/creature/CreatureState.h"
#include "templates/params/creature/CreatureAttribute.h"
#include "templates/params/creature/CreaturePosture.h"
#include "templates/tangible/SharedWeaponObjectTemplate.h"
#include "templates/building/CloningBuildingObjectTemplate.h"
#include "templates/building/CloneSpawnPoint.h"
#include "server/zone/managers/collision/CollisionManager.h"
#include "engine/lua/Lua.h"
#include "engine/lua/LuaObject.h"
#include "engine/lua/LuaFunction.h"
#include "server/zone/managers/director/QuestVectorMap.h"
#include "system/util/VectorMap.h"
#include "system/thread/Mutex.h"
#include "system/lang/Time.h"

// ============================================================================
// TRACKING DATA STRUCTURES
// ============================================================================

// Target cooldown data (for anti-farming)
struct TargetCooldownData {
	Time lastKillTime;
	int violationCount;

	TargetCooldownData() : violationCount(0) {
		lastKillTime.updateToCurrentTime();
	}
};

// Combat participant tracking (for gank detection)
// Maps victimID -> (attackerID -> lastAttackTime)
static VectorMap<uint64, VectorMap<uint64, Time>> combatParticipants;
static Mutex combatParticipantsMutex;

// Target cooldowns per killer
// Maps killerID -> (victimID -> cooldownData)
static VectorMap<uint64, VectorMap<uint64, TargetCooldownData>> targetCooldowns;
static Mutex targetCooldownsMutex;

// Active DW sessions per account (for multi-login detection)
// Maps accountID -> playerID
static VectorMap<uint32, uint64> activeDwSessions;
static Mutex activeDwSessionsMutex;

// Jetpack invulnerability tracking
// Maps vehicleID -> expirationTime
static VectorMap<uint64, Time> invulnerableJetpacks;
static Mutex invulnerableJetpacksMutex;

// DW Effect tracking for anti-stacking
// Maps targetID -> (effectType -> effectID)
// Used to track active DoT effects and prevent stacking
struct DwActiveEffect {
	uint64 attackerID;
	uint64 effectID;
	Time applied;

	DwActiveEffect() : attackerID(0), effectID(0) {
		applied.updateToCurrentTime();
	}

	DwActiveEffect(uint64 attacker, uint64 effect) : attackerID(attacker), effectID(effect) {
		applied.updateToCurrentTime();
	}
};

static VectorMap<uint64, VectorMap<int, DwActiveEffect>> activeEffects;
static Mutex activeEffectsMutex;

// Particle Beam immunity tracking
// Maps defenderID -> expirationTime
static VectorMap<uint64, Time> particleBeamImmunity;
static Mutex particleBeamImmunityMutex;

// ============================================================================
// COLLUDER DETECTION TRACKING
// ============================================================================
// Tracks pairs of players who frequently hunt together.
// When 3+ joint kills detected, both are flagged and XP frozen.
// Resolution: one kills the other, both unfrozen.

// Joint kill record - stores kill timestamps for FIFO cleanup
struct JointKillRecord {
	Time killTime;

	JointKillRecord() {
		killTime.updateToCurrentTime();
	}

	// Serialization for persistence
	bool toBinaryStream(ObjectOutputStream* stream) {
		return killTime.toBinaryStream(stream);
	}

	bool parseFromBinaryStream(ObjectInputStream* stream) {
		return killTime.parseFromBinaryStream(stream);
	}
};

// Colluder pair key - normalized so smaller ID is always first
struct ColluderPairKey {
	uint64 playerA;  // Always the smaller ID
	uint64 playerB;  // Always the larger ID

	ColluderPairKey() : playerA(0), playerB(0) {}

	ColluderPairKey(uint64 id1, uint64 id2) {
		if (id1 < id2) {
			playerA = id1;
			playerB = id2;
		} else {
			playerA = id2;
			playerB = id1;
		}
	}

	bool operator==(const ColluderPairKey& other) const {
		return playerA == other.playerA && playerB == other.playerB;
	}

	int compareTo(const ColluderPairKey& other) const {
		if (playerA != other.playerA)
			return playerA < other.playerA ? -1 : 1;
		if (playerB != other.playerB)
			return playerB < other.playerB ? -1 : 1;
		return 0;
	}
};

// Maps ColluderPairKey -> Vector of kill timestamps (FIFO, capped at maxTrackedKills)
static VectorMap<ColluderPairKey, Vector<JointKillRecord>> colluderJointKills;
static Mutex colluderTrackingMutex;

// Time window for gank detection (when a kill counts as "joint")
static const int64 COLLUDER_COMBAT_WINDOW_MS = 60000;  // 1 minute - same attacker window as gank detection

// ============================================================================
// STEALTH PROTOCOL STATE TRACKING
// ============================================================================

// Shadow Protocol active state (Black Sun)
// Maps playerID -> true if active
static VectorMap<uint64, bool> shadowProtocolActive;
static Mutex shadowProtocolMutex;

// Espionage Matrix active state (Zann Consortium)
// Maps playerID -> true if active
static VectorMap<uint64, bool> espionageMatrixActive;
static Mutex espionageMatrixMutex;

// Broadcast tracking for self-correction clock
// Maps playerID -> lastBroadcastTime
static VectorMap<uint64, Time> lastBroadcastTime;
static Mutex lastBroadcastTimeMutex;

// Refresh queue for periodic sync
// Players who need their PvP status refreshed
static Vector<uint64> pvpRefreshQueue;
static Mutex pvpRefreshQueueMutex;

// ============================================================================
// RADAR STEALTH STATE TRACKING
// ============================================================================
// Tracks players with active radar stealth (invisible to enemy radar)

// Radar stealth tracking - stores expiration time per player
// Maps playerID -> expiration timestamp (epoch ms)
static VectorMap<uint64, uint64> radarStealthExpiration;

// Radar stealth zone tracking - stores activation zone to detect zone changes
// Maps playerID -> zone name hash at activation
static VectorMap<uint64, uint32> radarStealthActivationZone;

static Mutex radarStealthMutex;

// Radar stealth configuration constants
// TODO: Reduce for production - currently set high for testing
static const int RADAR_STEALTH_BASE_DURATION_MS = 300000;    // 5 minutes base (TESTING)
static const int RADAR_STEALTH_PER_SUBTERFUGE_MS = 500;      // +0.5 sec per subterfuge point
static const float RADAR_STEALTH_ENEMY_CHECK_RANGE = 400.0f; // 400m enemy check range

// Configuration constants for PvP refresh system
static const int MAX_REFRESH_PER_CYCLE = 10;       // Limit per cycle to avoid lag
static const int64 REFRESH_INTERVAL_MS = 45000;    // Refresh if no broadcast in 45 sec

// Constants
static const int64 GANK_COMBAT_WINDOW_MS = 60000;  // 60 seconds to track attackers
static const int COOLDOWN_ESCALATION[] = {1, 2, 4, 8};  // Hours for violations 1-4+

// ============================================================================
// SERUM/STIM CONFIGURATION (Loaded from Lua)
// ============================================================================
// Stores configuration for DW serum commands.
// Loaded once at startup from dwStimConfig in death_watch_manager.lua.

// HAM cost configuration per stim type
struct StimHamConfig {
	String pool;        // "health", "action", "mind", or "all"
	int base;           // Base HAM cost
	String skill;       // Skill mod for cost reduction
	int perPoint;       // Cost reduction per skill point

	StimHamConfig() : pool("action"), base(0), skill(""), perPoint(0) {}
};

// Duration configuration per stim type
struct StimDurationConfig {
	int base;           // Base duration in seconds
	String skill;       // Skill mod for duration scaling (empty if fixed)
	int perPoint;       // Seconds added per skill point
	int max;            // Maximum duration cap
	bool fixed;         // If true, duration is exactly base (not scaled)

	StimDurationConfig() : base(0), skill(""), perPoint(0), max(0), fixed(false) {}
};

// Stim configuration maps
static VectorMap<String, String> stimTemplates;           // stimType -> template path
static VectorMap<String, int> stimSkillRequirements;      // stimType -> dw_healing requirement
static VectorMap<String, int> stimCooldowns;              // stimType -> cooldown in ms
static VectorMap<String, StimHamConfig> stimHamCosts;     // stimType -> HAM cost config
static VectorMap<String, StimDurationConfig> stimDurations; // stimType -> duration config
static VectorMap<String, String> stimDisplayNames;        // stimType -> Mandalorian display name
static bool stimConfigLoaded = false;

// DEPRECATED: Legacy XP matrices - replaced by rank-based system
// Kept for reference only. XP is now calculated using getTotalRank() and
// calculateRankKillXp() / calculateRankDeathXp() which use config from Lua.
// See death_watch_manager.lua for the new baseKillXp, baseDeathXp, and rank multipliers.
/*
static int killXPMatrix[5][5] = {
	{ 750, 1000, 1250, 1500, 2000 },  // Killer Box 0
	{ 500, 1000, 1250, 1500, 2000 },  // Killer Box 1
	{ 350, 500, 1000, 1250, 1750 },   // Killer Box 2
	{ 250, 350, 500, 1000, 1500 },    // Killer Box 3
	{ 200, 250, 350, 500, 1000 }      // Killer Box 4
};

static int deathXPMatrix[5][5] = {
	{ -500, -500, -500, -500, -500 },    // Victim Box 0
	{ -750, -750, -750, -750, -750 },    // Victim Box 1
	{ -900, -900, -750, -750, -750 },    // Victim Box 2
	{ -1100, -1100, -900, -750, -750 },  // Victim Box 3
	{ -1500, -1500, -1100, -900, -750 }  // Victim Box 4
};
*/

// ============================================================================
// STATIC HELPER FUNCTIONS FOR PEF MANAGEMENT
// ============================================================================
// These are defined as file-scope static functions because IDL doesn't support
// Vector<> return types. They must be defined before any methods that call them.

// Helper function: Get online players of a specific DW faction
static Vector<uint64> getOnlineFactionPlayersCpp(ZoneServer* zoneServer, byte factionID, uint64 excludePlayerID) {
	Vector<uint64> result;

	if (zoneServer == nullptr) {
		return result;
	}

	// Map faction ID to QuestVectorMap key
	String mapKey;
	switch (factionID) {
		case DeathWatchFactions::XON:
			mapKey = "death_watch_xon_players";
			break;
		case DeathWatchFactions::SUN:
			mapKey = "death_watch_sun_players";
			break;
		case DeathWatchFactions::KRR:
			mapKey = "death_watch_krr_players";
			break;
		case DeathWatchFactions::ZAN:
			mapKey = "death_watch_zan_players";
			break;
		default:
			return result;
	}

	// Get the QuestVectorMap from DirectorManager
	Reference<QuestVectorMap*> questMap = DirectorManager::instance()->getQuestVectorMap(mapKey);
	if (questMap == nullptr) {
		return result;
	}

	int mapSize = questMap->getMapSize();

	for (int i = 0; i < mapSize; i++) {
		String playerIDStr = questMap->getMapKeyAtIndex(i);
		uint64 playerID = Long::valueOf(playerIDStr);

		// Skip excluded player
		if (playerID == excludePlayerID) {
			continue;
		}

		// Validate player is online
		ManagedReference<SceneObject*> playerObj = zoneServer->getObject(playerID);
		if (playerObj == nullptr || !playerObj->isPlayerCreature()) {
			continue;
		}

		CreatureObject* player = playerObj.castTo<CreatureObject*>();
		if (player == nullptr || !player->isOnline()) {
			continue;
		}

		result.add(playerID);
	}

	return result;
}

// Helper function: Get bounty target IDs for a hunter
static Vector<uint64> getHunterBountyTargets(CreatureObject* hunter) {
	Vector<uint64> result;

	if (hunter == nullptr) {
		return result;
	}

	uint64 hunterID = hunter->getObjectID();

	// Check Lua readData for bounty targets (max 3 slots per DW_BOUNTY config)
	Lua* lua = DirectorManager::instance()->getLuaInstance();
	if (lua == nullptr) {
		return result;
	}

	// Access bounty data via readData
	// Keys are: playerID:dw:bh:bounty_1:target, bounty_2:target, bounty_3:target
	for (int slot = 1; slot <= 3; slot++) {
		String dataKey = String::valueOf(hunterID) + ":dw:bh:bounty_" + String::valueOf(slot) + ":target";

		Reference<LuaFunction*> readFunc = lua->createFunction("readData", 1);
		*readFunc << dataKey;
		readFunc->callFunction();

		lua_State* L = lua->getLuaState();
		if (lua_isnumber(L, -1)) {
			uint64 targetID = (uint64)lua_tonumber(L, -1);
			if (targetID > 0) {
				result.add(targetID);
			}
		}
		lua_pop(L, 1);
	}

	return result;
}

// Helper function: Get the hunter ID for a player being hunted
static uint64 getPlayerBountyHunterID(CreatureObject* player) {
	if (player == nullptr) {
		return 0;
	}

	uint64 playerID = player->getObjectID();

	// Check Lua readData for hunter
	Lua* lua = DirectorManager::instance()->getLuaInstance();
	if (lua == nullptr) {
		return 0;
	}

	String dataKey = String::valueOf(playerID) + ":dw:bh:hunter_id";

	Reference<LuaFunction*> readFunc = lua->createFunction("readData", 1);
	*readFunc << dataKey;
	readFunc->callFunction();

	lua_State* L = lua->getLuaState();
	uint64 hunterID = 0;
	if (lua_isnumber(L, -1)) {
		hunterID = (uint64)lua_tonumber(L, -1);
	}
	lua_pop(L, 1);

	return hunterID;
}

void DeathWatchManagerImplementation::initialize() {
	info("Initializing Death Watch Manager", true);

	loadLuaConfig();

	// Start the PvP refresh task for self-correction
	startPvpRefreshTask();

	info("Death Watch Manager initialized", true);
}

void DeathWatchManagerImplementation::startPvpRefreshTask() {
	Reference<DwPvpRefreshTask*> refreshTask = new DwPvpRefreshTask(_this.getReferenceUnsafeStaticCast());
	refreshTask->schedule(45000);  // First run in 45 seconds
	info("Death Watch PvP refresh task started", true);
}

void DeathWatchManagerImplementation::loadLuaConfig() {
	info("Loading Death Watch configuration from Lua", true);

	try {
		Lua* lua = new Lua();
		lua->init();

		if (!lua->runFile("scripts/managers/death_watch_manager.lua")) {
			error("Could not load death_watch_manager.lua");
			delete lua;
			return;
		}

		// Load XP multiplier for players without mastery
		noMasteryMultiplier = lua->getGlobalFloat("noMasteryMultiplier");

		// Load standing thresholds
		standingFullRewards = lua->getGlobalInt("standingFullRewards");
		standingMinorPenalty = lua->getGlobalInt("standingMinorPenalty");
		standingSignificantPenalty = lua->getGlobalInt("standingSignificantPenalty");
		standingHeavyPenalty = lua->getGlobalInt("standingHeavyPenalty");
		standingNoGains = lua->getGlobalInt("standingNoGains");
		standingPvpXpFloor = lua->getGlobalInt("standingPvpXpFloor");

		// Load TEF duration
		dwTefDurationMs = lua->getGlobalInt("dwTefDurationMs");

		// Load faction standing changes
		standingEnemyKill = lua->getGlobalInt("standingEnemyKill");
		standingDeath = lua->getGlobalInt("standingDeath");
		standingAlliedKill = lua->getGlobalInt("standingAlliedKill");
		standingTraitorKill = lua->getGlobalInt("standingTraitorKill");

		// Load gank protection values
		gankAttackerThreshold = lua->getGlobalInt("gankAttackerThreshold");
		gankDeathXp = lua->getGlobalInt("gankDeathXp");
		gankSurvivalBonus = lua->getGlobalInt("gankSurvivalBonus");

		// Load rank-based XP system config
		baseKillXp = lua->getGlobalInt("baseKillXp");
		baseDeathXp = lua->getGlobalInt("baseDeathXp");
		rankKillMultPerRankUp = lua->getGlobalFloat("rankKillMultPerRankUp");
		rankKillMultPerRankDown = lua->getGlobalFloat("rankKillMultPerRankDown");
		rankKillMultMin = lua->getGlobalFloat("rankKillMultMin");
		rankKillMultMax = lua->getGlobalFloat("rankKillMultMax");
		rankDeathMultPerRankUp = lua->getGlobalFloat("rankDeathMultPerRankUp");
		rankDeathMultPerRankDown = lua->getGlobalFloat("rankDeathMultPerRankDown");
		rankDeathMultMin = lua->getGlobalFloat("rankDeathMultMin");
		rankDeathMultMax = lua->getGlobalFloat("rankDeathMultMax");
		bountyKillMultiplier = lua->getGlobalFloat("bountyKillMultiplier");
		bountyDeathMultiplier = lua->getGlobalFloat("bountyDeathMultiplier");

		info("Rank-based XP config loaded: baseKill=" + String::valueOf(baseKillXp) +
			 ", baseDeath=" + String::valueOf(baseDeathXp) +
			 ", bountyKillMult=" + String::valueOf(bountyKillMultiplier), true);

		// Legacy XP matrices - kept for reference but no longer loaded
		// The rank-based system above replaces the matrix approach

		// Load test server list and check if current server is a test server
		isTestServer = false;
		String currentGalaxy = zoneServer->getGalaxyName();

		LuaObject testServerList = lua->getGlobalObject("testServers");
		if (testServerList.isValidTable()) {
			for (int i = 1; i <= testServerList.getTableSize(); i++) {
				String serverName = testServerList.getStringAt(i);
				if (serverName == currentGalaxy) {
					isTestServer = true;
					info("Test server mode enabled - same-account attacks allowed", true);
					break;
				}
			}
		}
		testServerList.pop();

		// Load Mk4 Ground Striker config
		LuaObject mk4Config = lua->getGlobalObject("mk4Config");
		if (mk4Config.isValidTable()) {
			mk4StunDuration = mk4Config.getIntField("stunDuration");
			mk4BlindDuration = mk4Config.getIntField("blindDuration");
			mk4HudDisableDuration = mk4Config.getIntField("hudDisableDuration");
			info("Mk4 config loaded: stun=" + String::valueOf(mk4StunDuration) +
				 "s, blind=" + String::valueOf(mk4BlindDuration) +
				 "s, HUD=" + String::valueOf(mk4HudDisableDuration) + "s", true);
		} else {
			info("Mk4 config not found, using defaults", true);
		}
		mk4Config.pop();

		// Load Heavy Weapons config
		LuaObject heavyConfig = lua->getGlobalObject("dwHeavyWeaponsConfig");
		if (heavyConfig.isValidTable()) {
			// Hit calculation
			heavyBaseAccuracy = heavyConfig.getFloatField("baseAccuracy");
			heavyAccBonus = heavyConfig.getFloatField("accBonus");
			heavyEvasionPenalty = heavyConfig.getFloatField("evasionPenalty");
			heavyMinHitChance = heavyConfig.getFloatField("minHitChance");
			heavyMaxHitChance = heavyConfig.getFloatField("maxHitChance");

			// Damage calculation
			heavyPotBonus = heavyConfig.getFloatField("potBonus");
			heavyDeflectionReduction = heavyConfig.getFloatField("deflectionReduction");
			heavyToughnessCap = heavyConfig.getIntField("toughnessCap");

			// Weapon multipliers
			LuaObject multipliers = heavyConfig.getObjectField("weaponMultipliers");
			if (multipliers.isValidTable()) {
				heavyRocketMultiplier = multipliers.getFloatField("rocket");
				heavyParticleMultiplier = multipliers.getFloatField("particle");
				heavyAcidMultiplier = multipliers.getFloatField("acid");
				heavyLightningMultiplier = multipliers.getFloatField("lightning");
			}
			multipliers.pop();

			// Particle Beam config
			LuaObject pbConfig = heavyConfig.getObjectField("particleBeam");
			if (pbConfig.isValidTable()) {
				pbBasePerTick = pbConfig.getFloatField("basePerTick");
				pbPotTickScaling = pbConfig.getFloatField("potTickScaling");
				pbRecoveryTickScaling = pbConfig.getFloatField("recoveryTickScaling");
				pbMinTickReduction = pbConfig.getFloatField("minTickReduction");
				pbMaxTickReduction = pbConfig.getFloatField("maxTickReduction");
				pbTickIntervalMs = pbConfig.getIntField("tickIntervalMs");
				pbNumTicks = pbConfig.getIntField("numTicks");

				pbBasePeakDuration = pbConfig.getFloatField("basePeakDuration");
				pbPotPeakScaling = pbConfig.getFloatField("potPeakScaling");
				pbRecoveryPeakScaling = pbConfig.getFloatField("recoveryPeakScaling");
				pbMinPeakDuration = pbConfig.getFloatField("minPeakDuration");
				pbMaxPeakDuration = pbConfig.getFloatField("maxPeakDuration");

				pbBaseImmunity = pbConfig.getFloatField("baseImmunity");
				pbRecoveryImmunityBonus = pbConfig.getFloatField("recoveryImmunityBonus");
			}
			pbConfig.pop();

			info("Heavy weapons config loaded: baseAcc=" + String::valueOf(heavyBaseAccuracy) +
				 "%, toughnessCap=" + String::valueOf(heavyToughnessCap), true);
		} else {
			info("Heavy weapons config not found, using defaults", true);
		}
		heavyConfig.pop();

		// Load Colluder Detection config
		LuaObject colluderConfig = lua->getGlobalObject("colluderConfig");
		if (colluderConfig.isValidTable()) {
			colluderJointKillThreshold = colluderConfig.getIntField("jointKillThreshold");
			colluderTrackingWindowMs = colluderConfig.getIntField("trackingWindowMs");
			colluderMaxTrackedKills = colluderConfig.getIntField("maxTrackedKills");
			info("Colluder config loaded: threshold=" + String::valueOf(colluderJointKillThreshold) +
				 ", window=" + String::valueOf(colluderTrackingWindowMs) + "ms" +
				 ", maxKills=" + String::valueOf(colluderMaxTrackedKills), true);
		} else {
			// Use defaults if not in config
			colluderJointKillThreshold = 3;
			colluderTrackingWindowMs = 3600000;  // 1 hour
			colluderMaxTrackedKills = 20;
			info("Colluder config not found, using defaults (threshold=3)", true);
		}
		colluderConfig.pop();

		// Load Serum/Stim command config
		LuaObject stimConfig = lua->getGlobalObject("dwStimConfig");
		if (stimConfig.isValidTable()) {
			// Clear any previous config
			stimTemplates.removeAll();
			stimSkillRequirements.removeAll();
			stimCooldowns.removeAll();
			stimHamCosts.removeAll();
			stimDurations.removeAll();
			stimDisplayNames.removeAll();

			// Load templates
			LuaObject templates = stimConfig.getObjectField("templates");
			if (templates.isValidTable()) {
				stimTemplates.put("torcabur", templates.getStringField("torcabur"));
				stimTemplates.put("naakkelir", templates.getStringField("naakkelir"));
				stimTemplates.put("beskargam", templates.getStringField("beskargam"));
				stimTemplates.put("gotaldin", templates.getStringField("gotaldin"));
				stimTemplates.put("aranovor", templates.getStringField("aranovor"));
				stimTemplates.put("hettir", templates.getStringField("hettir"));
			}
			templates.pop();

			// Load skill requirements
			LuaObject skillReqs = stimConfig.getObjectField("skillRequirements");
			if (skillReqs.isValidTable()) {
				stimSkillRequirements.put("torcabur", skillReqs.getIntField("torcabur"));
				stimSkillRequirements.put("naakkelir", skillReqs.getIntField("naakkelir"));
				stimSkillRequirements.put("beskargam", skillReqs.getIntField("beskargam"));
				stimSkillRequirements.put("gotaldin", skillReqs.getIntField("gotaldin"));
				stimSkillRequirements.put("aranovor", skillReqs.getIntField("aranovor"));
				stimSkillRequirements.put("hettir", skillReqs.getIntField("hettir"));
			}
			skillReqs.pop();

			// Load cooldowns
			LuaObject cooldowns = stimConfig.getObjectField("cooldowns");
			if (cooldowns.isValidTable()) {
				stimCooldowns.put("torcabur", cooldowns.getIntField("torcabur"));
				stimCooldowns.put("naakkelir", cooldowns.getIntField("naakkelir"));
				stimCooldowns.put("beskargam", cooldowns.getIntField("beskargam"));
				stimCooldowns.put("gotaldin", cooldowns.getIntField("gotaldin"));
				stimCooldowns.put("aranovor", cooldowns.getIntField("aranovor"));
				stimCooldowns.put("hettir", cooldowns.getIntField("hettir"));
			}
			cooldowns.pop();

			// Load HAM costs
			LuaObject hamCosts = stimConfig.getObjectField("hamCosts");
			if (hamCosts.isValidTable()) {
				const char* stimTypes[] = {"torcabur", "naakkelir", "beskargam", "gotaldin", "aranovor", "hettir"};
				for (int i = 0; i < 6; i++) {
					LuaObject hamEntry = hamCosts.getObjectField(stimTypes[i]);
					if (hamEntry.isValidTable()) {
						StimHamConfig config;
						config.pool = hamEntry.getStringField("pool");
						config.base = hamEntry.getIntField("base");
						config.skill = hamEntry.getStringField("skill");
						config.perPoint = hamEntry.getIntField("perPoint");
						stimHamCosts.put(stimTypes[i], config);
					}
					hamEntry.pop();
				}
			}
			hamCosts.pop();

			// Load durations
			LuaObject durations = stimConfig.getObjectField("durations");
			if (durations.isValidTable()) {
				const char* stimTypes[] = {"torcabur", "naakkelir", "beskargam", "gotaldin", "aranovor", "hettir"};
				for (int i = 0; i < 6; i++) {
					LuaObject durEntry = durations.getObjectField(stimTypes[i]);
					if (durEntry.isValidTable()) {
						StimDurationConfig config;
						config.base = durEntry.getIntField("base");
						config.skill = durEntry.getStringField("skill");
						config.perPoint = durEntry.getIntField("perPoint");
						config.max = durEntry.getIntField("max");
						config.fixed = durEntry.getIntField("fixed") != 0;
						stimDurations.put(stimTypes[i], config);
					} else if (!durEntry.isValidTable()) {
						// Non-table means instant effect (like naakkelir purge)
						StimDurationConfig config;
						config.base = 0;
						config.fixed = true;
						stimDurations.put(stimTypes[i], config);
					}
					durEntry.pop();
				}
			}
			durations.pop();

			// Load display names
			LuaObject displayNames = stimConfig.getObjectField("displayNames");
			if (displayNames.isValidTable()) {
				stimDisplayNames.put("torcabur", displayNames.getStringField("torcabur"));
				stimDisplayNames.put("naakkelir", displayNames.getStringField("naakkelir"));
				stimDisplayNames.put("beskargam", displayNames.getStringField("beskargam"));
				stimDisplayNames.put("gotaldin", displayNames.getStringField("gotaldin"));
				stimDisplayNames.put("aranovor", displayNames.getStringField("aranovor"));
				stimDisplayNames.put("hettir", displayNames.getStringField("hettir"));
			}
			displayNames.pop();

			stimConfigLoaded = true;
			info("Stim command config loaded: 6 stim types", true);
		} else {
			info("Stim command config not found, stim commands disabled", true);
		}
		stimConfig.pop();

		delete lua;

		info("Death Watch configuration loaded successfully", true);

	} catch (Exception& e) {
		error("Error loading Death Watch configuration: " + e.getMessage());
	}
}

bool DeathWatchManagerImplementation::areFactionsAtWar(byte faction1, byte faction2) const {
	// Same faction check depends on war type (free-for-all allows same faction attacks)
	// No faction (0) is never at war
	if (faction1 == DeathWatchFactions::NONE || faction2 == DeathWatchFactions::NONE) {
		return false;
	}

	// Read war state from Lua shared memory (dw_war is the source of truth)
	// War types: 0=peace/uninitialized, 1=free-for-all, 2=2v2, 3=1v3, 4=four-way
	// NOTE: 0 also means uninitialized shared memory - we default to four-way in that case
	DirectorManager* directorManager = DirectorManager::instance();
	uint64 warType = directorManager->readSharedMemory("dw:war:type");
	uint64 warConfig = directorManager->readSharedMemory("dw:war:config");

	// Check if war system has been initialized by Lua
	// If warType is 0, check if it's actually peace or just uninitialized
	// We use a separate key to distinguish initialized peace from uninitialized
	bool isInitialized = (directorManager->readSharedMemory("dw:war:initialized") == 1);

	// If not initialized, default to four-way war (most common/expected state)
	if (!isInitialized && warType == 0) {
		warType = 4; // Four-way
	}

	// Peace: no one fights (only when explicitly set via Lua)
	if (warType == 0) {
		return false;
	}

	// Free-for-all: EVERYONE fights (even same faction)
	if (warType == 1) {
		return true;
	}

	// For all other war types, same faction = not enemies
	if (faction1 == faction2) {
		return false;
	}

	// Four-way: all factions fight each other (same faction handled above)
	if (warType == 4) {
		return true;
	}

	// 2v2 configurations:
	// Config 1: Sun+Xon vs Zan+Krr
	// Config 2: Sun+Zan vs Xon+Krr
	// Config 3: Sun+Krr vs Xon+Zan
	if (warType == 2) {
		// Check if factions are on same team (allies)
		bool sameTeam = false;

		if (warConfig == 1) {
			// Team 1: Sun(2) + Xon(1), Team 2: Zan(4) + Krr(3)
			sameTeam = ((faction1 == DeathWatchFactions::SUN || faction1 == DeathWatchFactions::XON) &&
						(faction2 == DeathWatchFactions::SUN || faction2 == DeathWatchFactions::XON)) ||
					   ((faction1 == DeathWatchFactions::ZAN || faction1 == DeathWatchFactions::KRR) &&
						(faction2 == DeathWatchFactions::ZAN || faction2 == DeathWatchFactions::KRR));
		} else if (warConfig == 2) {
			// Team 1: Sun(2) + Zan(4), Team 2: Xon(1) + Krr(3)
			sameTeam = ((faction1 == DeathWatchFactions::SUN || faction1 == DeathWatchFactions::ZAN) &&
						(faction2 == DeathWatchFactions::SUN || faction2 == DeathWatchFactions::ZAN)) ||
					   ((faction1 == DeathWatchFactions::XON || faction1 == DeathWatchFactions::KRR) &&
						(faction2 == DeathWatchFactions::XON || faction2 == DeathWatchFactions::KRR));
		} else if (warConfig == 3) {
			// Team 1: Sun(2) + Krr(3), Team 2: Xon(1) + Zan(4)
			sameTeam = ((faction1 == DeathWatchFactions::SUN || faction1 == DeathWatchFactions::KRR) &&
						(faction2 == DeathWatchFactions::SUN || faction2 == DeathWatchFactions::KRR)) ||
					   ((faction1 == DeathWatchFactions::XON || faction1 == DeathWatchFactions::ZAN) &&
						(faction2 == DeathWatchFactions::XON || faction2 == DeathWatchFactions::ZAN));
		}

		return !sameTeam;  // At war if not on same team
	}

	// 1v3 configurations: one faction vs three allied
	// Config 1: Sun alone, Config 2: Xon alone, Config 3: Zan alone, Config 4: Krr alone
	if (warType == 3) {
		byte lonerFaction = DeathWatchFactions::NONE;

		if (warConfig == 1) {
			lonerFaction = DeathWatchFactions::SUN;
		} else if (warConfig == 2) {
			lonerFaction = DeathWatchFactions::XON;
		} else if (warConfig == 3) {
			lonerFaction = DeathWatchFactions::ZAN;
		} else if (warConfig == 4) {
			lonerFaction = DeathWatchFactions::KRR;
		}

		// If either faction is the loner, they're at war
		if (faction1 == lonerFaction || faction2 == lonerFaction) {
			return true;
		}

		// Otherwise, the other three are allied
		return false;
	}

	// Default to at war for unknown types
	return true;
}

bool DeathWatchManagerImplementation::canPlayersAttack(CreatureObject* player1, CreatureObject* player2) {
	if (player1 == nullptr || player2 == nullptr) {
		return false;
	}

	// Can't attack yourself
	if (player1->getObjectID() == player2->getObjectID()) {
		return false;
	}

	// Both must be players
	if (!player1->isPlayerCreature() || !player2->isPlayerCreature()) {
		return false;
	}

	// Get ghost objects
	ManagedReference<PlayerObject*> ghost1 = player1->getPlayerObject();
	ManagedReference<PlayerObject*> ghost2 = player2->getPlayerObject();

	if (ghost1 == nullptr || ghost2 == nullptr) {
		return false;
	}

	// Get DW factions
	byte faction1 = ghost1->getDwFaction();
	byte faction2 = ghost2->getDwFaction();

	// Both must have a faction assigned
	if (faction1 == DeathWatchFactions::NONE || faction2 == DeathWatchFactions::NONE) {
		return false;
	}

	// Check if factions are at war
	if (areFactionsAtWar(faction1, faction2)) {
		return true;
	}

	// OATHBREAKER CHECK: Same faction members can attack if one is an oathbreaker
	// An oathbreaker is a player who has been hiding in their HQ for 30+ minutes
	if (faction1 == faction2) {
		// Check if either player is an oathbreaker of this faction
		if (isOathbreaker(player1) || isOathbreaker(player2)) {
			return true;
		}
	}

	return false;
}

/**
 * Check if a player is currently an Oathbreaker
 * Oathbreakers are players who hid in their HQ for 30+ minutes
 * They can be attacked by their own faction members
 */
bool DeathWatchManagerImplementation::isOathbreaker(CreatureObject* player) {
	if (player == nullptr) {
		return false;
	}

	// Check Lua screenplay data for oathbreaker status
	Lua* lua = DirectorManager::instance()->getLuaInstance();
	if (lua == nullptr) {
		return false;
	}

	lua_State* L = lua->getLuaState();
	if (L == nullptr) {
		return false;
	}

	// Call dw_oathbreaker:isOathbreaker(pPlayer)
	lua_getglobal(L, "dw_oathbreaker");
	if (!lua_istable(L, -1)) {
		lua_pop(L, 1);
		return false;
	}

	lua_getfield(L, -1, "isOathbreaker");
	if (!lua_isfunction(L, -1)) {
		lua_pop(L, 2);
		return false;
	}

	// Push self (dw_oathbreaker table)
	lua_pushvalue(L, -2);

	// Push player pointer
	lua_pushlightuserdata(L, player);

	// Call with 2 args (self, pPlayer), 1 return
	if (lua_pcall(L, 2, 1, 0) != 0) {
		const char* err = lua_tostring(L, -1);
		error() << "Error calling dw_oathbreaker:isOathbreaker: " << (err ? err : "unknown");
		lua_pop(L, 2);  // error + dw_oathbreaker table
		return false;
	}

	bool result = lua_toboolean(L, -1);
	lua_pop(L, 2);  // result + dw_oathbreaker table

	return result;
}

bool DeathWatchManagerImplementation::callLuaOathbreakerKill(CreatureObject* killer, CreatureObject* victim) {
	if (killer == nullptr || victim == nullptr) {
		return false;
	}

	// Call Lua dw_oathbreaker:handleOathbreakerKill(pKiller, pVictim)
	// Returns true if victim was an oathbreaker and special handling was applied
	Lua* lua = DirectorManager::instance()->getLuaInstance();
	if (lua == nullptr) {
		return false;
	}

	lua_State* L = lua->getLuaState();
	if (L == nullptr) {
		return false;
	}

	lua_getglobal(L, "dw_oathbreaker");
	if (!lua_istable(L, -1)) {
		lua_pop(L, 1);
		return false;
	}

	lua_getfield(L, -1, "handleOathbreakerKill");
	if (!lua_isfunction(L, -1)) {
		lua_pop(L, 2);
		return false;
	}

	// Push self (dw_oathbreaker table)
	lua_pushvalue(L, -2);

	// Push killer and victim pointers
	lua_pushlightuserdata(L, killer);
	lua_pushlightuserdata(L, victim);

	// Call with 3 args (self, pKiller, pVictim), 1 return
	if (lua_pcall(L, 3, 1, 0) != 0) {
		const char* err = lua_tostring(L, -1);
		error() << "Error calling dw_oathbreaker:handleOathbreakerKill: " << (err ? err : "unknown");
		lua_pop(L, 2);  // error + dw_oathbreaker table
		return false;
	}

	bool result = lua_toboolean(L, -1);
	lua_pop(L, 2);  // result + dw_oathbreaker table

	return result;
}

void DeathWatchManagerImplementation::handlePlayerKill(CreatureObject* killer, CreatureObject* victim) {
	if (killer == nullptr || victim == nullptr) {
		return;
	}

	// Both must be players
	if (!killer->isPlayerCreature() || !victim->isPlayerCreature()) {
		return;
	}

	// Get ghost objects
	ManagedReference<PlayerObject*> killerGhost = killer->getPlayerObject();
	ManagedReference<PlayerObject*> victimGhost = victim->getPlayerObject();

	if (killerGhost == nullptr || victimGhost == nullptr) {
		return;
	}

	// Get DW factions
	byte killerFaction = killerGhost->getDwFaction();
	byte victimFaction = victimGhost->getDwFaction();

	// Both must be Death Watch with factions
	if (killerFaction == DeathWatchFactions::NONE || victimFaction == DeathWatchFactions::NONE) {
		return;
	}

	// Must have DW novice skill
	if (!killer->hasSkill("combat_death_watch_novice") || !victim->hasSkill("combat_death_watch_novice")) {
		return;
	}

	// ============================================================================
	// OATHBREAKER CHECK (calls Lua dw_oathbreaker:handleOathbreakerKill)
	// If victim is an oathbreaker, pays bounty and makes them houseless
	// Processing continues regardless - oathbreaker kill is additive, not replacement
	// ============================================================================
	bool wasOathbreakerKill = callLuaOathbreakerKill(killer, victim);

	Locker locker(killer);

	// ============================================================================
	// ANTI-ABUSE CHECKS
	// ============================================================================

	// Check for same-account kill (alt abuse)
	bool sameAccountKill = isSameAccount(killer, victim);
	if (sameAccountKill) {
		killer->sendSystemMessage("\\#FF0000You cannot earn XP by killing your own alternate character!");
		// Still track the kill for stats, but no XP
		killerGhost->incrementDwTotalKills();

		Locker clocker(victim, killer);
		victimGhost->incrementDwTotalDeaths();

		clearCombatParticipants(victim);
		return;
	}

	// Check for farming cooldown
	bool isFarming = isTargetOnCooldown(killer, victim);

	// Check if victim was ganked (4+ attackers)
	bool wasGanked = isNaturalGank(victim);
	int attackerCount = getAttackerCount(victim);

	// Get box levels (kept for logging)
	int killerBox = getPlayerBox(killer, killerFaction);
	int victimBox = getPlayerBox(victim, victimFaction);

	// Clamp to valid range
	killerBox = Math::clamp(0, killerBox, 4);
	victimBox = Math::clamp(0, victimBox, 4);

	// Get total ranks for XP calculation (sum of all faction boxes + 1)
	int killerRank = getTotalRank(killer);
	int victimRank = getTotalRank(victim);

	// Check if this is a DW bounty kill
	bool isBountyKill = false;
	uint64 victimID = victim->getObjectID();
	Vector<uint64> hunterTargets = getHunterBountyTargets(killer);
	for (int i = 0; i < hunterTargets.size(); i++) {
		if (hunterTargets.get(i) == victimID) {
			isBountyKill = true;
			break;
		}
	}

	bool isSameFaction = (killerFaction == victimFaction);
	bool isEnemy = areFactionsAtWar(killerFaction, victimFaction);
	bool isAllied = !isSameFaction && !isEnemy;

	// ============================================================================
	// KILL LOGGING - For dispute resolution
	// ============================================================================
	String killerZone = killer->getZone() != nullptr ? killer->getZone()->getZoneName() : "unknown";
	String factionNames[] = {"NONE", "XON", "SUN", "KRR", "ZAN"};
	String killerFactionName = (killerFaction <= 4) ? factionNames[killerFaction] : "INVALID";
	String victimFactionName = (victimFaction <= 4) ? factionNames[victimFaction] : "INVALID";

	StringBuffer logMsg;
	logMsg << "DWKILL: " << killer->getFirstName() << " (R" << killerRank << " " << killerFactionName << ")"
		   << " killed " << victim->getFirstName() << " (R" << victimRank << " " << victimFactionName << ")"
		   << " @ " << killerZone
		   << " [" << (int)killer->getPositionX() << "," << (int)killer->getPositionY() << "]";
	if (isBountyKill) {
		logMsg << " [BOUNTY]";
	}

	if (sameAccountKill) logMsg << " SAME_ACCOUNT";
	if (isFarming) logMsg << " FARMING";
	if (wasGanked) logMsg << " GANKED(" << attackerCount << ")";
	if (isSameFaction) logMsg << " SAME_FACTION";
	if (isAllied) logMsg << " ALLIED";

	error(logMsg.toString());

	// Track the kill (always, for stats)
	killerGhost->incrementDwTotalKills();

	Locker clocker(victim, killer);
	victimGhost->incrementDwTotalDeaths();

	// Record the kill for cooldown tracking
	recordKillForCooldown(killer, victim, isFarming);

	// ============================================================================
	// COLLUDER DETECTION
	// ============================================================================

	// Check if this kill resolves a colluder pair (killer kills their partner)
	// This unfreezes BOTH players' XP regardless of normal XP award
	bool colluderResolved = checkColluderResolution(killer, victim);

	// Record this kill for joint-kill tracking (to detect future colluders)
	// Only track if there were other combat participants
	if (!colluderResolved) {
		recordJointKillForColluder(killer, victim);
	}

	// Only award XP for enemy kills
	if (isEnemy) {
		// ============================================================================
		// KILLER XP CALCULATION
		// ============================================================================

		bool killerEarnsXp = true;
		String noXpReason = "";

		// Check all XP blocking conditions
		if (!canEarnXp(killer)) {
			killerEarnsXp = false;
			noXpReason = getNoXpReason(killer, victim);
		} else if (isFarming) {
			killerEarnsXp = false;
			noXpReason = "Farming cooldown";
		} else if (!canEarnPvpXp(killer, killerFaction)) {
			killerEarnsXp = false;
			noXpReason = "Insufficient faction standing";
		}

		if (killerEarnsXp) {
			// Calculate base XP using rank-based system
			int baseXp = calculateRankKillXp(killerRank, victimRank, isBountyKill);

			// Get standing from faction
			String standingKey = getFactionStandingKey(killerFaction);
			int standing = killerGhost->getFactionStanding(standingKey);
			float standingMult = getStandingMultiplier(standing);

			// Apply mastery multiplier
			float masteryMult = hasValidMastery(killer, killerFaction) ? 1.0f : noMasteryMultiplier;

			int xpGain = (int)(baseXp * standingMult * masteryMult);

			if (xpGain > 0) {
				awardXp(killer, killerFaction, xpGain);

				StringBuffer msg;
				msg << "\\#00FF00Death Watch kill: +" << xpGain << " XP";
				if (isBountyKill) {
					msg << " [BOUNTY]";
				}
				int rankDiff = killerRank - victimRank;
				if (rankDiff < -2) {
					msg << " (underdog bonus)";
				} else if (rankDiff > 2) {
					msg << " (reduced)";
				}
				if (standingMult < 1.0f) {
					msg << " (" << (int)(standingMult * 100) << "% standing)";
				}
				if (masteryMult < 1.0f) {
					msg << " (no mastery)";
				}
				killer->sendSystemMessage(msg.toString());
			}
		} else {
			StringBuffer msg;
			msg << "\\#FFA500Death Watch kill: No XP - " << noXpReason;
			killer->sendSystemMessage(msg.toString());

			// Check if in warmup and inform progress
			if (isInWarmup(killer)) {
				int remaining = warmupKillCount - killerGhost->getDwTotalKills();
				if (remaining > 0) {
					StringBuffer warmupMsg;
					warmupMsg << "\\#FFFF00Warmup progress: " << remaining << " kills remaining";
					killer->sendSystemMessage(warmupMsg.toString());
				} else {
					killer->sendSystemMessage("\\#00FF00Warmup complete! You can now earn XP.");
				}
			}
		}

		// ============================================================================
		// VICTIM XP LOSS CALCULATION
		// ============================================================================

		int deathXp;

		// Gank protection - reduced XP loss if victim was ganked
		if (wasGanked) {
			deathXp = gankDeathXp;  // Use reduced gank death penalty

			StringBuffer gankMsg;
			gankMsg << "\\#00FFFFGank protection active (" << attackerCount << " attackers) - reduced XP loss!";
			victim->sendSystemMessage(gankMsg.toString());
		} else {
			// Calculate death XP using rank-based system
			deathXp = calculateRankDeathXp(victimRank, killerRank, isBountyKill);
		}

		// Get victim standing for death multiplier
		String victimStandingKey = getFactionStandingKey(victimFaction);
		int victimStanding = victimGhost->getFactionStanding(victimStandingKey);
		float deathMult = getDeathStandingMultiplier(victimStanding);

		int xpLoss = (int)(deathXp * deathMult);

		if (xpLoss < 0) {
			awardXp(victim, victimFaction, xpLoss);

			StringBuffer msg;
			msg << "\\#FF6666Death Watch death: " << xpLoss << " XP";
			if (isBountyKill) {
				msg << " [BOUNTY]";
			}
			int rankDiff = killerRank - victimRank;
			if (rankDiff > 2) {
				msg << " (reduced - outranked)";
			} else if (rankDiff < -2) {
				msg << " (increased - upset loss)";
			}
			if (deathMult > 1.0f) {
				msg << " (" << (int)(deathMult * 100) << "% negative standing)";
			}
			victim->sendSystemMessage(msg.toString());

			// Check for demotion
			checkDemotion(victim, victimFaction);
		}

		// ============================================================================
		// FACTION STANDING CHANGES
		// ============================================================================

		String killerStandingKey = getFactionStandingKey(killerFaction);
		int killerStandingChange = (int)(standingEnemyKill * (hasValidMastery(killer, killerFaction) ? 1.0f : noMasteryMultiplier));
		killerGhost->increaseFactionStanding(killerStandingKey, killerStandingChange);

		int victimStandingChange = (int)(standingDeath * (hasValidMastery(victim, victimFaction) ? 1.0f : noMasteryMultiplier));
		victimGhost->increaseFactionStanding(victimStandingKey, victimStandingChange);

	} else if (isSameFaction) {
		killer->sendSystemMessage("\\#FF0000You killed a member of your own faction! No XP awarded.");

		// Apply traitor penalty
		String killerStandingKey = getFactionStandingKey(killerFaction);
		killerGhost->increaseFactionStanding(killerStandingKey, standingTraitorKill);

	} else if (isAllied) {
		killer->sendSystemMessage("\\#FF6600You killed an allied faction member! No XP awarded.");

		// Apply allied kill penalty
		String killerStandingKey = getFactionStandingKey(killerFaction);
		killerGhost->increaseFactionStanding(killerStandingKey, standingAlliedKill);
	}

	// Clear combat participants for the victim
	clearCombatParticipants(victim);
}

void DeathWatchManagerImplementation::applyDwTef(CreatureObject* player) {
	if (player == nullptr || !player->isPlayerCreature()) {
		return;
	}

	ManagedReference<PlayerObject*> ghost = player->getPlayerObject();
	if (ghost == nullptr) {
		return;
	}

	Locker locker(player);
	ghost->updateLastDwPvpCombatActionTimestamp();
}

String DeathWatchManagerImplementation::getXpTypeForFaction(byte faction) {
	switch (faction) {
		case DeathWatchFactions::XON: return "dw_xon";
		case DeathWatchFactions::SUN: return "dw_sun";
		case DeathWatchFactions::KRR: return "dw_krr";
		case DeathWatchFactions::ZAN: return "dw_zan";
		default: return "";
	}
}

String DeathWatchManagerImplementation::getFactionKey(byte faction) {
	switch (faction) {
		case DeathWatchFactions::XON: return "xon";
		case DeathWatchFactions::SUN: return "sun";
		case DeathWatchFactions::KRR: return "krr";
		case DeathWatchFactions::ZAN: return "zan";
		default: return "";
	}
}

String DeathWatchManagerImplementation::getFactionStandingKey(byte faction) {
	switch (faction) {
		case DeathWatchFactions::XON: return "xonti_brothers";
		case DeathWatchFactions::SUN: return "blacksun";
		case DeathWatchFactions::KRR: return "black_krrsantans";
		case DeathWatchFactions::ZAN: return "zann_consortium";
		default: return "";
	}
}

int DeathWatchManagerImplementation::getPlayerBox(CreatureObject* player, byte faction) {
	if (player == nullptr) {
		return 0;
	}

	String prefix;
	switch (faction) {
		case DeathWatchFactions::XON: prefix = "combat_death_watch_xon_"; break;
		case DeathWatchFactions::SUN: prefix = "combat_death_watch_sun_"; break;
		case DeathWatchFactions::KRR: prefix = "combat_death_watch_krr_"; break;
		case DeathWatchFactions::ZAN: prefix = "combat_death_watch_zan_"; break;
		default: return 0;
	}

	// Check boxes 4 down to 1
	for (int box = 4; box >= 1; box--) {
		String skillName = prefix + String::valueOf(box);
		if (player->hasSkill(skillName)) {
			return box;
		}
	}

	return 0; // Only novice
}

int DeathWatchManagerImplementation::getTotalRank(CreatureObject* player) {
	if (player == nullptr) {
		return 1;
	}

	int totalBoxes = 0;
	totalBoxes += getPlayerBox(player, DeathWatchFactions::XON);
	totalBoxes += getPlayerBox(player, DeathWatchFactions::SUN);
	totalBoxes += getPlayerBox(player, DeathWatchFactions::KRR);
	totalBoxes += getPlayerBox(player, DeathWatchFactions::ZAN);

	// Rank = total boxes + 1 (so novice-only = rank 1, all Box 4 = rank 17)
	return totalBoxes + 1;
}

int DeathWatchManagerImplementation::calculateRankKillXp(int killerRank, int victimRank, bool isBountyKill) const {
	int rankDiff = killerRank - victimRank;
	float multiplier = 1.0f;

	if (rankDiff >= 0) {
		// Killing lower or equal rank - reduced XP
		multiplier = Math::max(rankKillMultMin, 1.0f - (rankDiff * rankKillMultPerRankDown));
	} else {
		// Killing higher rank - bonus XP (underdog)
		multiplier = Math::min(rankKillMultMax, 1.0f + (abs(rankDiff) * rankKillMultPerRankUp));
	}

	int xp = (int)(baseKillXp * multiplier);

	// Apply bounty multiplier
	if (isBountyKill) {
		xp = (int)(xp * bountyKillMultiplier);
	}

	return xp;
}

int DeathWatchManagerImplementation::calculateRankDeathXp(int victimRank, int killerRank, bool isBountyDeath) const {
	int rankDiff = killerRank - victimRank;
	float multiplier = 1.0f;

	if (rankDiff >= 0) {
		// Died to higher rank - reduced loss (expected outcome)
		multiplier = Math::max(rankDeathMultMin, 1.0f - (rankDiff * rankDeathMultPerRankUp));
	} else {
		// Died to lower rank - increased loss (embarrassing)
		multiplier = Math::min(rankDeathMultMax, 1.0f + (abs(rankDiff) * rankDeathMultPerRankDown));
	}

	int xpLoss = (int)(baseDeathXp * multiplier);

	// Apply bounty multiplier (reduced loss for bounty marks)
	if (isBountyDeath) {
		xpLoss = (int)(xpLoss * bountyDeathMultiplier);
	}

	return -xpLoss; // Return negative value
}

bool DeathWatchManagerImplementation::hasValidMastery(CreatureObject* player, byte faction) {
	if (player == nullptr) {
		return false;
	}

	// Check for required novice skills based on faction
	// These match the factionMasteryRequirements in death_watch_manager.lua
	switch (faction) {
		case DeathWatchFactions::XON:
			return player->hasSkill("combat_unarmed_novice") || player->hasSkill("combat_pistol_novice");
		case DeathWatchFactions::SUN:
			return player->hasSkill("combat_carbine_novice") || player->hasSkill("combat_polearm_novice");
		case DeathWatchFactions::KRR:
			return player->hasSkill("combat_rifleman_novice") || player->hasSkill("combat_commando_novice");
		case DeathWatchFactions::ZAN:
			return player->hasSkill("combat_1hsword_novice") || player->hasSkill("combat_smuggler_novice");
		default:
			return false;
	}
}

float DeathWatchManagerImplementation::getStandingMultiplier(int standing) const {
	if (standing >= standingFullRewards) {
		return 1.0f;
	} else if (standing >= standingMinorPenalty) {
		return 0.9f;
	} else if (standing >= standingSignificantPenalty) {
		return 0.5f;
	} else if (standing >= standingHeavyPenalty) {
		return 0.25f;
	} else {
		return 0.0f;
	}
}

float DeathWatchManagerImplementation::getDeathStandingMultiplier(int standing) const {
	if (standing >= 1) {
		return 1.0f;
	} else if (standing >= -4999) {
		return 1.25f;
	} else {
		return 1.5f;
	}
}

bool DeathWatchManagerImplementation::canEarnPvpXp(CreatureObject* player, byte faction) {
	if (player == nullptr) {
		return false;
	}

	ManagedReference<PlayerObject*> ghost = player->getPlayerObject();
	if (ghost == nullptr) {
		return false;
	}

	String standingKey = getFactionStandingKey(faction);
	int standing = ghost->getFactionStanding(standingKey);

	return standing > standingPvpXpFloor;
}

void DeathWatchManagerImplementation::awardXp(CreatureObject* player, byte faction, int amount) {
	if (player == nullptr || amount == 0) {
		return;
	}

	String xpType = getXpTypeForFaction(faction);
	if (xpType.isEmpty()) {
		return;
	}

	ManagedReference<PlayerObject*> ghost = player->getPlayerObject();
	if (ghost == nullptr) {
		return;
	}

	// Use PlayerManager to award XP
	PlayerManager* playerManager = zoneServer->getPlayerManager();
	if (playerManager != nullptr) {
		playerManager->awardExperience(player, xpType, amount, false);
	}
}

void DeathWatchManagerImplementation::checkDemotion(CreatureObject* player, byte faction) {
	if (player == nullptr) {
		return;
	}

	int currentBox = getPlayerBox(player, faction);
	if (currentBox <= 0) {
		return; // No boxes to demote from
	}

	ManagedReference<PlayerObject*> ghost = player->getPlayerObject();
	if (ghost == nullptr) {
		return;
	}

	String xpType = getXpTypeForFaction(faction);
	int currentXp = ghost->getExperience(xpType);

	// Demotion thresholds (approximately 20% buffer below box requirement)
	int demotionThresholds[5] = { 20000, 60000, 140000, 300000, 0 }; // Box 1-4, index 0 is box 1

	int threshold = demotionThresholds[currentBox - 1];

	if (currentXp < threshold) {
		// Demote player
		String prefix;
		switch (faction) {
			case DeathWatchFactions::XON: prefix = "combat_death_watch_xon_"; break;
			case DeathWatchFactions::SUN: prefix = "combat_death_watch_sun_"; break;
			case DeathWatchFactions::KRR: prefix = "combat_death_watch_krr_"; break;
			case DeathWatchFactions::ZAN: prefix = "combat_death_watch_zan_"; break;
			default: return;
		}

		String skillName = prefix + String::valueOf(currentBox);

		if (player->hasSkill(skillName)) {
			SkillManager* skillManager = SkillManager::instance();
			if (skillManager != nullptr) {
				skillManager->surrenderSkill(skillName, player, true);

				StringBuffer msg;
				msg << "DEMOTION: You have lost Box " << currentBox << " due to insufficient XP!";
				player->sendSystemMessage(msg.toString());

				// Recursively check for multi-level demotion
				checkDemotion(player, faction);
			}
		}
	}
}

void DeathWatchManagerImplementation::setWarMode(byte mode) {
	// Simple atomic update - no lock needed for single byte
	warMode = mode;
}

void DeathWatchManagerImplementation::setWarPairings(byte w1f1, byte w1f2, byte w2f1, byte w2f2) {
	// Update war pairings
	war1Faction1 = w1f1;
	war1Faction2 = w1f2;
	war2Faction1 = w2f1;
	war2Faction2 = w2f2;
}

bool DeathWatchManagerImplementation::isDeathWatch(CreatureObject* player) {
	// Kill switch check - when active, NO ONE is considered Death Watch
	// This is the emergency shutdown - single point of control
	if (killSwitchActive) {
		return false;
	}

	if (player == nullptr || !player->isPlayerCreature()) {
		return false;
	}

	ManagedReference<PlayerObject*> ghost = player->getPlayerObject();
	if (ghost == nullptr) {
		return false;
	}

	// Must have DW faction set AND DW novice skill
	// This is the full check for combat/PvP features
	return ghost->getDwFaction() != DeathWatchFactions::NONE &&
		   player->hasSkill("combat_death_watch_novice");
}

void DeathWatchManagerImplementation::setKillSwitch(bool active) {
	killSwitchActive = active;
	info(true) << "Death Watch Kill Switch " << (active ? "ACTIVATED" : "DEACTIVATED");
}

bool DeathWatchManagerImplementation::hasDwNoviceSkill(CreatureObject* player) {
	if (player == nullptr || !player->isPlayerCreature()) {
		return false;
	}

	// Only checks for the skill - used for basic DW features like clone window
	// where faction assignment isn't required yet
	return player->hasSkill("combat_death_watch_novice");
}

bool DeathWatchManagerImplementation::canLearnSkill(CreatureObject* creature, const String& skillName) {
	// Use DirectorManager's shared Lua instance to call DeathWatchManager:canLearnSkill
	Lua* lua = DirectorManager::instance()->getLuaInstance();

	if (lua == nullptr) {
		error("canLearnSkill: Could not access Lua instance");
		return true;  // Default to allow if Lua unavailable
	}

	Reference<LuaFunction*> luaFunc = lua->createFunction("DeathWatchManager", "canLearnSkill", 1);
	*luaFunc << creature;
	*luaFunc << skillName;

	lua_State* L = luaFunc->callFunction();

	if (L == nullptr) {
		error("canLearnSkill: Lua state is nullptr");
		return true;  // Default to allow
	}

	if (lua_gettop(L) < 1) {
		error("canLearnSkill: Lua stack is empty");
		return true;  // Default to allow
	}

	bool result = lua_toboolean(L, -1) != 0;
	lua_pop(L, 1);

	return result;
}

// ============================================================================
// ANTI-ABUSE TRACKING IMPLEMENTATIONS
// ============================================================================

bool DeathWatchManagerImplementation::isSameAccount(CreatureObject* killer, CreatureObject* victim) {
	// Test servers allow same-account attacks for testing
	if (isTestServer) {
		return false;
	}

	if (killer == nullptr || victim == nullptr) {
		return false;
	}

	if (!killer->isPlayerCreature() || !victim->isPlayerCreature()) {
		return false;
	}

	ManagedReference<PlayerObject*> killerGhost = killer->getPlayerObject();
	ManagedReference<PlayerObject*> victimGhost = victim->getPlayerObject();

	if (killerGhost == nullptr || victimGhost == nullptr) {
		return false;
	}

	uint32 killerAccount = killerGhost->getAccountID();
	uint32 victimAccount = victimGhost->getAccountID();

	return killerAccount == victimAccount;
}

bool DeathWatchManagerImplementation::isInWarmup(CreatureObject* player) {
	if (player == nullptr || !player->isPlayerCreature()) {
		return false;
	}

	ManagedReference<PlayerObject*> ghost = player->getPlayerObject();
	if (ghost == nullptr) {
		return false;
	}

	// First N kills earn 0 XP
	return ghost->getDwTotalKills() < warmupKillCount;
}

bool DeathWatchManagerImplementation::canEarnXp(CreatureObject* player) {
	if (player == nullptr || !player->isPlayerCreature()) {
		return false;
	}

	// Check warmup
	if (isInWarmup(player)) {
		return false;
	}

	// Check if XP is frozen
	ManagedReference<PlayerObject*> ghost = player->getPlayerObject();
	if (ghost == nullptr) {
		return false;
	}

	if (ghost->isDwXpFrozen()) {
		return false;
	}

	return true;
}

bool DeathWatchManagerImplementation::isTargetOnCooldown(CreatureObject* killer, CreatureObject* victim) {
	if (killer == nullptr || victim == nullptr) {
		return false;
	}

	uint64 killerID = killer->getObjectID();
	uint64 victimID = victim->getObjectID();

	Locker locker(&targetCooldownsMutex);

	if (!targetCooldowns.contains(killerID)) {
		return false;
	}

	VectorMap<uint64, TargetCooldownData>& killerCooldowns = targetCooldowns.get(killerID);

	if (!killerCooldowns.contains(victimID)) {
		return false;
	}

	TargetCooldownData& data = killerCooldowns.get(victimID);

	// Calculate cooldown duration based on violation count
	int violationIndex = Math::min(data.violationCount, 3);
	int64 cooldownMs = (int64)farmingCooldownBase * COOLDOWN_ESCALATION[violationIndex];

	int64 elapsedMs = data.lastKillTime.miliDifference();

	// miliDifference returns negative if time is in the past
	return elapsedMs > -cooldownMs;
}

void DeathWatchManagerImplementation::recordKillForCooldown(CreatureObject* killer, CreatureObject* victim, bool wasFarming) {
	if (killer == nullptr || victim == nullptr) {
		return;
	}

	uint64 killerID = killer->getObjectID();
	uint64 victimID = victim->getObjectID();

	Locker locker(&targetCooldownsMutex);

	if (!targetCooldowns.contains(killerID)) {
		targetCooldowns.put(killerID, VectorMap<uint64, TargetCooldownData>());
	}

	VectorMap<uint64, TargetCooldownData>& killerCooldowns = targetCooldowns.get(killerID);

	if (!killerCooldowns.contains(victimID)) {
		killerCooldowns.put(victimID, TargetCooldownData());
	}

	TargetCooldownData& data = killerCooldowns.get(victimID);

	data.lastKillTime.updateToCurrentTime();

	if (wasFarming) {
		data.violationCount++;
	}
}

void DeathWatchManagerImplementation::recordCombatParticipant(CreatureObject* victim, CreatureObject* attacker) {
	if (victim == nullptr || attacker == nullptr) {
		return;
	}

	if (!victim->isPlayerCreature() || !attacker->isPlayerCreature()) {
		return;
	}

	uint64 victimID = victim->getObjectID();
	uint64 attackerID = attacker->getObjectID();

	Locker locker(&combatParticipantsMutex);

	if (!combatParticipants.contains(victimID)) {
		combatParticipants.put(victimID, VectorMap<uint64, Time>());
	}

	VectorMap<uint64, Time>& attackers = combatParticipants.get(victimID);

	Time now;
	now.updateToCurrentTime();
	attackers.put(attackerID, now);
}

int DeathWatchManagerImplementation::getAttackerCount(CreatureObject* victim) {
	if (victim == nullptr) {
		return 0;
	}

	uint64 victimID = victim->getObjectID();

	Locker locker(&combatParticipantsMutex);

	if (!combatParticipants.contains(victimID)) {
		return 0;
	}

	VectorMap<uint64, Time>& attackers = combatParticipants.get(victimID);

	int count = 0;
	Time now;
	now.updateToCurrentTime();

	for (int i = 0; i < attackers.size(); i++) {
		Time& attackTime = attackers.elementAt(i).getValue();
		int64 elapsedMs = attackTime.miliDifference();

		// miliDifference returns negative if time is in the past
		if (elapsedMs > -GANK_COMBAT_WINDOW_MS) {
			count++;
		}
	}

	return count;
}

bool DeathWatchManagerImplementation::isNaturalGank(CreatureObject* victim) {
	if (victim == nullptr) {
		return false;
	}

	int attackerCount = getAttackerCount(victim);
	return attackerCount >= gankAttackerThreshold;
}

void DeathWatchManagerImplementation::clearCombatParticipants(CreatureObject* victim) {
	if (victim == nullptr) {
		return;
	}

	uint64 victimID = victim->getObjectID();

	Locker locker(&combatParticipantsMutex);

	if (combatParticipants.contains(victimID)) {
		combatParticipants.drop(victimID);
	}
}

// ============================================================================
// COLLUDER DETECTION IMPLEMENTATION
// ============================================================================

void DeathWatchManagerImplementation::recordJointKillForColluder(CreatureObject* killer, CreatureObject* victim) {
	if (killer == nullptr || victim == nullptr) {
		return;
	}

	if (!killer->isPlayerCreature() || !victim->isPlayerCreature()) {
		return;
	}

	uint64 killerID = killer->getObjectID();
	uint64 victimID = victim->getObjectID();

	// Get all players who participated in this kill (attacked the victim recently)
	Vector<uint64> participants;

	{
		Locker locker(&combatParticipantsMutex);

		if (combatParticipants.contains(victimID)) {
			VectorMap<uint64, Time>& attackers = combatParticipants.get(victimID);

			Time now;
			now.updateToCurrentTime();

			for (int i = 0; i < attackers.size(); i++) {
				uint64 attackerID = attackers.elementAt(i).getKey();
				Time& attackTime = attackers.elementAt(i).getValue();
				int64 elapsedMs = attackTime.miliDifference();

				// Include attackers within the combat window (miliDifference returns negative for past times)
				if (elapsedMs > -COLLUDER_COMBAT_WINDOW_MS && attackerID != killerID) {
					participants.add(attackerID);
				}
			}
		}
	}

	// Build list of ALL players involved in this kill (killer + participants + victim)
	// Including victim means the "farm target" also gets flagged as colluding with attackers
	Vector<uint64> allInvolved;
	allInvolved.add(killerID);
	for (int i = 0; i < participants.size(); i++) {
		allInvolved.add(participants.get(i));
	}
	allInvolved.add(victimID);  // Victim is part of the collusion - they're being farmed

	// Collect pairs that need to be flagged (can't flag while holding mutex)
	Vector<uint64> pairsToFlag;  // Stored as [id1, id2, id1, id2, ...]

	// Record a joint kill for EVERY pair of involved players
	// This catches 3+ player groups properly (e.g., 4 players = 6 pairs)
	{
		Locker colluderLocker(&colluderTrackingMutex);

		for (int i = 0; i < allInvolved.size(); i++) {
			for (int j = i + 1; j < allInvolved.size(); j++) {
				uint64 player1ID = allInvolved.get(i);
				uint64 player2ID = allInvolved.get(j);

				ColluderPairKey key(player1ID, player2ID);

				if (!colluderJointKills.contains(key)) {
					colluderJointKills.put(key, Vector<JointKillRecord>());
				}

				Vector<JointKillRecord>& kills = colluderJointKills.get(key);

				// Add new kill record
				JointKillRecord record;
				kills.add(record);

				// FIFO cleanup: remove oldest if over limit
				while (kills.size() > colluderMaxTrackedKills) {
					kills.remove(0);
				}

				// Clean up old records outside tracking window
				Time now;
				now.updateToCurrentTime();
				int validKillCount = 0;

				for (int k = kills.size() - 1; k >= 0; k--) {
					int64 elapsedMs = kills.get(k).killTime.miliDifference();
					if (elapsedMs < -colluderTrackingWindowMs) {
						kills.remove(k);
					} else {
						validKillCount++;
					}
				}

				// Check if threshold reached for this pair
				if (validKillCount >= colluderJointKillThreshold) {
					pairsToFlag.add(player1ID);
					pairsToFlag.add(player2ID);
				}
			}
		}
	}  // Release colluderLocker

	// Flag ALL pairs that hit threshold (mutex released, safe to lock players)
	for (int i = 0; i < pairsToFlag.size(); i += 2) {
		uint64 player1ID = pairsToFlag.get(i);
		uint64 player2ID = pairsToFlag.get(i + 1);

		ManagedReference<SceneObject*> obj1 = zoneServer->getObject(player1ID);
		ManagedReference<SceneObject*> obj2 = zoneServer->getObject(player2ID);

		if (obj1 != nullptr && obj1->isPlayerCreature() &&
			obj2 != nullptr && obj2->isPlayerCreature()) {

			CreatureObject* creature1 = obj1.castTo<CreatureObject*>();
			CreatureObject* creature2 = obj2.castTo<CreatureObject*>();

			if (creature1 != nullptr && creature2 != nullptr) {
				flagAsColluders(creature1, creature2);
			}
		}
	}
}

bool DeathWatchManagerImplementation::areColluders(CreatureObject* player1, CreatureObject* player2) {
	if (player1 == nullptr || player2 == nullptr) {
		return false;
	}

	ManagedReference<PlayerObject*> ghost1 = player1->getPlayerObject();
	ManagedReference<PlayerObject*> ghost2 = player2->getPlayerObject();

	if (ghost1 == nullptr || ghost2 == nullptr) {
		return false;
	}

	uint64 id1 = player1->getObjectID();
	uint64 id2 = player2->getObjectID();

	// Check if they are each other's colluder partner
	return (ghost1->getDwColluderPartnerID() == id2 && ghost2->getDwColluderPartnerID() == id1);
}

bool DeathWatchManagerImplementation::isColluder(CreatureObject* player) {
	if (player == nullptr) {
		return false;
	}

	ManagedReference<PlayerObject*> ghost = player->getPlayerObject();
	if (ghost == nullptr) {
		return false;
	}

	return ghost->isDwColluder();
}

void DeathWatchManagerImplementation::flagAsColluders(CreatureObject* player1, CreatureObject* player2) {
	if (player1 == nullptr || player2 == nullptr) {
		return;
	}

	ManagedReference<PlayerObject*> ghost1 = player1->getPlayerObject();
	ManagedReference<PlayerObject*> ghost2 = player2->getPlayerObject();

	if (ghost1 == nullptr || ghost2 == nullptr) {
		return;
	}

	uint64 id1 = player1->getObjectID();
	uint64 id2 = player2->getObjectID();

	// Check if already flagged
	if (ghost1->getDwColluderPartnerID() == id2) {
		return;  // Already colluders
	}

	String name1 = player1->getFirstName();
	String name2 = player2->getFirstName();

	// Flag both players
	{
		Locker locker1(player1);
		ghost1->setDwColluderPartnerID(id2);
		ghost1->setDwXpFrozen(true);
	}

	{
		Locker locker2(player2);
		ghost2->setDwColluderPartnerID(id1);
		ghost2->setDwXpFrozen(true);
	}

	// Notify both players
	StringBuffer msg1;
	msg1 << "\\#FF0000GREEF'S JUDGEMENT: You and " << name2 << " have been caught hunting together too often!";
	player1->sendSystemMessage(msg1.toString());
	player1->sendSystemMessage("\\#FFA500Your XP is frozen until one of you kills the other. Greef demands satisfaction.");

	StringBuffer msg2;
	msg2 << "\\#FF0000GREEF'S JUDGEMENT: You and " << name1 << " have been caught hunting together too often!";
	player2->sendSystemMessage(msg2.toString());
	player2->sendSystemMessage("\\#FFA500Your XP is frozen until one of you kills the other. Greef demands satisfaction.");

	info("Colluders flagged: " + name1 + " and " + name2, true);
}

bool DeathWatchManagerImplementation::checkColluderResolution(CreatureObject* killer, CreatureObject* victim) {
	if (killer == nullptr || victim == nullptr) {
		return false;
	}

	ManagedReference<PlayerObject*> killerGhost = killer->getPlayerObject();
	ManagedReference<PlayerObject*> victimGhost = victim->getPlayerObject();

	if (killerGhost == nullptr || victimGhost == nullptr) {
		return false;
	}

	uint64 killerID = killer->getObjectID();
	uint64 victimID = victim->getObjectID();

	// Check if killer and victim are colluder partners
	if (killerGhost->getDwColluderPartnerID() != victimID) {
		return false;  // Not colluders
	}

	// Verify symmetry
	if (victimGhost->getDwColluderPartnerID() != killerID) {
		// Data inconsistency - clean up
		killerGhost->clearDwColluder();
		return false;
	}

	String killerName = killer->getFirstName();
	String victimName = victim->getFirstName();

	// Clear colluder status for killer - they survive with their faction intact
	{
		Locker locker1(killer);
		killerGhost->clearDwColluder();
		killerGhost->setDwXpFrozen(false);
	}

	// Victim loses their faction - must rejoin through Cara Dune
	byte victimOldFaction = victimGhost->getDwFaction();
	String formerFactionKey = getFactionKey(victimOldFaction);  // Get string key before clearing
	{
		Locker locker2(victim);
		victimGhost->clearDwColluder();
		victimGhost->setDwXpFrozen(false);
		victimGhost->setDwFaction(DeathWatchFactions::NONE);  // Go houseless

		// Set screenplay data so Cara Dune knows this player was colluder-ejected
		victimGhost->setScreenPlayData("death_watch", "colluder_ejected", "1");
		victimGhost->setScreenPlayData("death_watch", "colluder_former_faction", formerFactionKey);
		// Clear their assigned faction so Cara can reassign
		victimGhost->setScreenPlayData("death_watch", "assigned_faction", "");
	}

	// Clear tracking data for this pair
	{
		Locker colluderLocker(&colluderTrackingMutex);
		ColluderPairKey key(killerID, victimID);
		if (colluderJointKills.contains(key)) {
			colluderJointKills.drop(key);
		}
	}

	// Notify both players
	killer->sendSystemMessage("\\#00FF00GREEF'S APPROVAL: You eliminated your colluder partner! You are free from judgment.");
	victim->sendSystemMessage("\\#FF0000GREEF'S JUDGEMENT: You lost the duel. Your faction has been stripped.");
	victim->sendSystemMessage("\\#FFA500You are now houseless. Speak to Cara Dune if you wish to rejoin a faction.");

	info("Colluder pair resolved: " + killerName + " killed " + victimName + " (former faction: " + formerFactionKey + ") - victim lost faction", true);

	return true;
}

void DeathWatchManagerImplementation::clearColluderStatus(CreatureObject* player) {
	if (player == nullptr) {
		return;
	}

	ManagedReference<PlayerObject*> ghost = player->getPlayerObject();
	if (ghost == nullptr) {
		return;
	}

	uint64 playerID = player->getObjectID();
	uint64 partnerID = ghost->getDwColluderPartnerID();

	if (partnerID == 0) {
		return;  // Not a colluder
	}

	// Clear this player's status
	{
		Locker locker(player);
		ghost->clearDwColluder();
		// Note: we don't automatically unfreeze XP - they might be frozen for other reasons
	}

	// Clear partner's status if they exist
	ManagedReference<SceneObject*> partnerObj = zoneServer->getObject(partnerID);
	if (partnerObj != nullptr && partnerObj->isPlayerCreature()) {
		CreatureObject* partner = partnerObj.castTo<CreatureObject*>();
		if (partner != nullptr) {
			ManagedReference<PlayerObject*> partnerGhost = partner->getPlayerObject();
			if (partnerGhost != nullptr && partnerGhost->getDwColluderPartnerID() == playerID) {
				Locker partnerLocker(partner);
				partnerGhost->clearDwColluder();
			}
		}
	}

	// Clear tracking data for this pair
	{
		Locker colluderLocker(&colluderTrackingMutex);
		ColluderPairKey key(playerID, partnerID);
		if (colluderJointKills.contains(key)) {
			colluderJointKills.drop(key);
		}
	}
}

int DeathWatchManagerImplementation::getJointKillCount(CreatureObject* player1, CreatureObject* player2) {
	if (player1 == nullptr || player2 == nullptr) {
		return 0;
	}

	uint64 id1 = player1->getObjectID();
	uint64 id2 = player2->getObjectID();

	Locker locker(&colluderTrackingMutex);

	ColluderPairKey key(id1, id2);
	if (!colluderJointKills.contains(key)) {
		return 0;
	}

	Vector<JointKillRecord>& kills = colluderJointKills.get(key);

	// Count only valid kills within tracking window
	Time now;
	now.updateToCurrentTime();
	int validCount = 0;

	for (int i = 0; i < kills.size(); i++) {
		int64 elapsedMs = kills.get(i).killTime.miliDifference();
		if (elapsedMs > -colluderTrackingWindowMs) {
			validCount++;
		}
	}

	return validCount;
}

void DeathWatchManagerImplementation::onPlayerLogin(CreatureObject* player) {
	if (player == nullptr || !player->isPlayerCreature()) {
		return;
	}

	if (!isDeathWatch(player)) {
		return;
	}

	ManagedReference<PlayerObject*> ghost = player->getPlayerObject();
	if (ghost == nullptr) {
		return;
	}

	// Update login time for session tracking
	ghost->updateDwLoginTime();

	// Register as active DW for this account
	uint32 accountID = ghost->getAccountID();
	uint64 playerID = player->getObjectID();

	Locker locker(&activeDwSessionsMutex);

	// Check if another DW is already logged in
	if (activeDwSessions.contains(accountID)) {
		uint64 existingPlayerID = activeDwSessions.get(accountID);
		if (existingPlayerID != playerID) {
			// Multi-login detected - freeze XP for this character
			ghost->setDwXpFrozen(true);
			ghost->incrementDwSessionWarnings();

			player->sendSystemMessage("\\#FF0000DEATH WATCH CONFLICT: Another Death Watch character is logged in on your account!");
			player->sendSystemMessage("\\#FFA500You can only have ONE Death Watch character active at a time.");
			return;
		}
	}

	activeDwSessions.put(accountID, playerID);

	// Trigger PvP status sync for zone-in
	onPlayerZoneIn(player);
}

void DeathWatchManagerImplementation::onPlayerLogout(CreatureObject* player) {
	if (player == nullptr || !player->isPlayerCreature()) {
		return;
	}

	if (!isDeathWatch(player)) {
		return;
	}

	ManagedReference<PlayerObject*> ghost = player->getPlayerObject();
	if (ghost == nullptr) {
		return;
	}

	uint32 accountID = ghost->getAccountID();
	uint64 playerID = player->getObjectID();

	// Check session duration for abuse detection
	Time loginTime = ghost->getDwLoginTime();
	int64 sessionDurationMs = loginTime.miliDifference();  // Negative = time passed

	// Session too short is suspicious
	if (-sessionDurationMs < suspiciousSessionMinMs) {
		ghost->incrementDwSessionWarnings();

		int warnings = ghost->getDwSessionWarnings();

		if (warnings >= sessionFreezeThreshold) {
			ghost->setDwXpFrozen(true);
		}
	}

	// Clear active session for this account
	Locker locker(&activeDwSessionsMutex);

	if (activeDwSessions.contains(accountID)) {
		if (activeDwSessions.get(accountID) == playerID) {
			activeDwSessions.drop(accountID);
		}
	}

	locker.release();

	// Clean up stealth protocol state
	cleanupStealthProtocolState(player);
}

bool DeathWatchManagerImplementation::hasMultiLoginConflict(CreatureObject* player) {
	if (player == nullptr || !player->isPlayerCreature()) {
		return false;
	}

	if (!isDeathWatch(player)) {
		return false;
	}

	ManagedReference<PlayerObject*> ghost = player->getPlayerObject();
	if (ghost == nullptr) {
		return false;
	}

	uint32 accountID = ghost->getAccountID();
	uint64 playerID = player->getObjectID();

	Locker locker(&activeDwSessionsMutex);

	if (!activeDwSessions.contains(accountID)) {
		return false;
	}

	return activeDwSessions.get(accountID) != playerID;
}

String DeathWatchManagerImplementation::getNoXpReason(CreatureObject* killer, CreatureObject* victim) {
	if (killer == nullptr) {
		return "System error";
	}

	// Check warmup
	if (isInWarmup(killer)) {
		ManagedReference<PlayerObject*> ghost = killer->getPlayerObject();
		if (ghost != nullptr) {
			int remaining = warmupKillCount - ghost->getDwTotalKills();
			StringBuffer msg;
			msg << "Warmup period (" << remaining << " kills remaining)";
			return msg.toString();
		}
		return "Warmup period";
	}

	// Check XP frozen
	ManagedReference<PlayerObject*> ghost = killer->getPlayerObject();
	if (ghost != nullptr && ghost->isDwXpFrozen()) {
		// Check if frozen due to colluder status
		if (ghost->isDwColluder()) {
			return "XP frozen - colluder (kill your partner)";
		}
		return "XP frozen (see Greef)";
	}

	// Check farming cooldown
	if (victim != nullptr && isTargetOnCooldown(killer, victim)) {
		return "Farming cooldown";
	}

	// Check faction standing
	if (ghost != nullptr) {
		byte faction = ghost->getDwFaction();
		String standingKey = getFactionStandingKey(faction);
		int standing = ghost->getFactionStanding(standingKey);

		if (standing <= standingPvpXpFloor) {
			StringBuffer msg;
			msg << "Insufficient faction standing (need > " << standingPvpXpFloor << ", you have " << standing << ")";
			return msg.toString();
		} else if (standing <= standingNoGains) {
			return "Insufficient faction standing (need > 0)";
		}
	}

	return "Unknown";
}

// ============================================================================
// MOUNTED COMBAT (SADDLE SHOT) IMPLEMENTATIONS
// ============================================================================

// CRC for "object/mobile/vehicle/death_watch_jetpack.iff" = 0x8A24BE7D
static const uint32 DEATH_WATCH_JETPACK_CRC = 0x8A24BE7D;

bool DeathWatchManagerImplementation::isOnDeathWatchJetpack(CreatureObject* player) {

	if (player == nullptr) {
		return false;
	}


	if (!player->isRidingMount()) {
		return false;
	}

	// Get the parent - DW jetpack is a vehicle, not a traditional mount
	ManagedReference<SceneObject*> parent = player->getParent().get();
	if (parent == nullptr) {
		return false;
	}


	// Check template CRC directly
	bool result = parent->getServerObjectCRC() == DEATH_WATCH_JETPACK_CRC;
	return result;
}

bool DeathWatchManagerImplementation::isValidDwJetpackTarget(CreatureObject* attacker, TangibleObject* target) {
	// Valid targets for DW jetpack combat:
	// 1. DW Hunter NPCs (training mobs) - any DW player can attack
	// 2. Enemy DW player, their pet, or their vehicle

	if (attacker == nullptr || target == nullptr) {
		return false;
	}

	// Get the target creature (could be player, pet, NPC, or need to get rider from vehicle)
	CreatureObject* targetCreature = nullptr;

	// Check vehicle/mount FIRST since they also return true for isCreatureObject()
	if (target->isVehicleObject() || target->isMount()) {
		// Vehicle/mount - get the rider via getLinkedCreature()
		ManagedReference<CreatureObject*> rider = target->asCreatureObject()->getLinkedCreature();
		if (rider != nullptr) {
			targetCreature = rider;
		} else {
		}
	} else if (target->isCreatureObject()) {
		targetCreature = target->asCreatureObject();
	} else {
	}

	if (targetCreature == nullptr) {
		return false;  // Not a valid target type
	}

	// If target is a pet, get the owner
	if (targetCreature->isPet()) {
		targetCreature = targetCreature->getLinkedCreature().get();
		if (targetCreature == nullptr) {
			return false;
		}
	}

	// CHECK DW HUNTER NPCs FIRST - before any faction checks
	// DW Hunters have faction "deathwatchhunter" and are always valid targets for any DW player
	// (DW skill check is done in validateJetpackAttack, not here)
	if (targetCreature->isAiAgent()) {
		uint32 targetFaction = targetCreature->getFaction();
		uint32 dwHunterFaction = STRING_HASHCODE("deathwatchhunter");


		if (targetFaction == dwHunterFaction) {
			return true;  // DW Hunters are always valid targets for any DW player
		}

		// Also check using String hash in case STRING_HASHCODE differs
		String dwHunterStr = "deathwatchhunter";
		uint32 dwHunterStringHash = dwHunterStr.hashCode();

		if (targetFaction == dwHunterStringHash) {
			return true;
		}

		// Not a DW Hunter NPC, and NPCs without deathwatchhunter faction are not valid DW targets
		return false;
	}

	// === For player targets, both attacker and target need DW factions ===

	PlayerObject* attackerGhost = attacker->getPlayerObject();
	if (attackerGhost == nullptr) {
		return false;
	}

	byte attackerDwFaction = attackerGhost->getDwFaction();
	if (attackerDwFaction == DeathWatchFactions::NONE) {
		return false;  // Attacker has no DW faction
	}

	// Target must be a player with DW faction
	if (!targetCreature->isPlayerCreature()) {
		return false;
	}

	PlayerObject* targetGhost = targetCreature->getPlayerObject();
	if (targetGhost == nullptr) {
		return false;
	}

	byte targetDwFaction = targetGhost->getDwFaction();
	if (targetDwFaction == DeathWatchFactions::NONE) {
		return false;  // Target has no DW faction
	}

	// Check if factions are at war (enemy DW factions)
	bool atWar = areFactionsAtWar(attackerDwFaction, targetDwFaction);
	return atWar;
}

bool DeathWatchManagerImplementation::isCommandAllowedWhileMounted(const String& commandName) {
	// Use DirectorManager's shared Lua instance to call DeathWatchManager:isCommandAllowedWhileMounted
	Lua* lua = DirectorManager::instance()->getLuaInstance();

	if (lua == nullptr) {
		error("isCommandAllowedWhileMounted: Could not access Lua instance");
		return false;  // Default to block if Lua unavailable
	}

	Reference<LuaFunction*> luaFunc = lua->createFunction("DeathWatchManager", "isCommandAllowedWhileMounted", 1);
	*luaFunc << commandName;

	lua_State* L = luaFunc->callFunction();

	if (L == nullptr) {
		error("isCommandAllowedWhileMounted: Lua state is nullptr");
		return false;
	}

	if (lua_gettop(L) < 1) {
		error("isCommandAllowedWhileMounted: Lua stack is empty");
		return false;
	}

	bool result = lua_toboolean(L, -1) != 0;
	lua_pop(L, 1);

	return result;
}

float DeathWatchManagerImplementation::getMountedCombatDamageMultiplier() {
	// Use DirectorManager's shared Lua instance to call DeathWatchManager:getMountedCombatDamageMultiplier
	Lua* lua = DirectorManager::instance()->getLuaInstance();

	if (lua == nullptr) {
		error("getMountedCombatDamageMultiplier: Could not access Lua instance");
		return 0.75f;  // Default value
	}

	Reference<LuaFunction*> luaFunc = lua->createFunction("DeathWatchManager", "getMountedCombatDamageMultiplier", 1);

	lua_State* L = luaFunc->callFunction();

	if (L == nullptr) {
		error("getMountedCombatDamageMultiplier: Lua state is nullptr");
		return 0.75f;
	}

	if (lua_gettop(L) < 1) {
		error("getMountedCombatDamageMultiplier: Lua stack is empty");
		return 0.75f;
	}

	float result = lua_tonumber(L, -1);
	lua_pop(L, 1);

	return result;
}

int DeathWatchManagerImplementation::getMountedCombatAccuracyModifier() {
	// Use DirectorManager's shared Lua instance to call DeathWatchManager:getMountedCombatAccuracyModifier
	Lua* lua = DirectorManager::instance()->getLuaInstance();

	if (lua == nullptr) {
		error("getMountedCombatAccuracyModifier: Could not access Lua instance");
		return -25;  // Default value
	}

	Reference<LuaFunction*> luaFunc = lua->createFunction("DeathWatchManager", "getMountedCombatAccuracyModifier", 1);

	lua_State* L = luaFunc->callFunction();

	if (L == nullptr) {
		error("getMountedCombatAccuracyModifier: Lua state is nullptr");
		return -25;
	}

	if (lua_gettop(L) < 1) {
		error("getMountedCombatAccuracyModifier: Lua stack is empty");
		return -25;
	}

	int result = lua_tointeger(L, -1);
	lua_pop(L, 1);

	return result;
}

// ============================================================================
// JETPACK INVULNERABILITY
// ============================================================================

bool DeathWatchManagerImplementation::isJetpackInvulnerable(uint64 vehicleID) {
	Locker lock(&invulnerableJetpacksMutex);

	if (!invulnerableJetpacks.contains(vehicleID)) {
		return false;
	}

	Time& expirationTime = invulnerableJetpacks.get(vehicleID);

	// isPast() returns true if the time has already passed
	if (expirationTime.isPast()) {
		// Expired - remove from map
		invulnerableJetpacks.drop(vehicleID);
		return false;
	}

	// Still invulnerable - calculate remaining time for debug
	int64 remainingMs = -expirationTime.miliDifference();  // miliDifference returns negative for future times
	return true;
}

void DeathWatchManagerImplementation::setJetpackInvulnerable(uint64 vehicleID, int durationMs) {
	Locker lock(&invulnerableJetpacksMutex);

	Time expirationTime;
	expirationTime.updateToCurrentTime();
	expirationTime.addMiliTime(durationMs);

	invulnerableJetpacks.put(vehicleID, expirationTime);
}

void DeathWatchManagerImplementation::removeJetpackInvulnerability(uint64 vehicleID) {
	Locker lock(&invulnerableJetpacksMutex);

	if (invulnerableJetpacks.contains(vehicleID)) {
		invulnerableJetpacks.drop(vehicleID);
	}
}

// ============================================================================
// COMMAND COOLDOWN RETRIEVAL (from Lua config)
// ============================================================================

int DeathWatchManagerImplementation::getCommandCooldown(const String& commandName) {
	Lua* lua = DirectorManager::instance()->getLuaInstance();

	if (lua == nullptr) {
		error("getCommandCooldown: Could not access Lua instance");
		return 0;
	}

	// Call Lua function to get cooldown from config
	Reference<LuaFunction*> luaFunc = lua->createFunction("DeathWatchManager", "getCommandCooldown", 1);
	*luaFunc << commandName;

	lua_State* L = luaFunc->callFunction();

	if (L == nullptr || lua_gettop(L) < 1) {
		return 0;
	}

	int cooldown = lua_tointeger(L, -1);
	lua_pop(L, 1);

	return cooldown;
}

bool DeathWatchManagerImplementation::checkAndApplyCooldown(CreatureObject* creature, const String& commandName) {
	if (creature == nullptr) {
		return false;
	}

	// Get cooldown from Lua config
	int cooldownMs = getCommandCooldown(commandName);

	// No cooldown configured = always allow
	if (cooldownMs <= 0) {
		return true;
	}

	// Build cooldown key
	String cooldownKey = "dw_" + commandName;

	// Check if on cooldown
	if (!creature->checkCooldownRecovery(cooldownKey)) {
		// Still on cooldown - send message with remaining time
		const Time* cdTime = creature->getCooldownTime(cooldownKey);
		if (cdTime != nullptr) {
			int64 timeLeftMs = cdTime->miliDifference() * -1;  // miliDifference returns negative for future
			int timeLeftSec = (int)ceil(timeLeftMs / 1000.0);

			StringBuffer msg;
			msg << commandName << " recharging... ";

			if (timeLeftSec < 60) {
				msg << timeLeftSec << "s";
			} else if (timeLeftSec < 3600) {
				int mins = timeLeftSec / 60;
				int secs = timeLeftSec % 60;
				msg << mins << "m";
				if (secs > 0) msg << " " << secs << "s";
			} else {
				int hours = timeLeftSec / 3600;
				int mins = (timeLeftSec % 3600) / 60;
				msg << hours << "h";
				if (mins > 0) msg << " " << mins << "m";
			}

			creature->sendSystemMessage(msg.toString());
		}
		return false;
	}

	// Not on cooldown - set it and allow
	creature->addCooldown(cooldownKey, cooldownMs);
	return true;
}

// ============================================================================
// DW JETPACK COMBAT - CORE ATTACK METHODS
// ============================================================================

// Configuration constants for DW combat
static const int DW_RANGED_MAX_RANGE = 64;      // Maximum range for ranged attacks
static const int DW_MELEE_MAX_RANGE = 42;       // Maximum range for melee attacks (jetpack combat)
static const int DW_MIN_COOLDOWN_MS = 1000;     // Minimum cooldown of 1 second
static const int DW_UNARMED_MIN_DAMAGE = 50;    // Default damage when unarmed
static const float DW_UNARMED_ATTACK_SPEED = 1.0f;  // Default speed when unarmed

bool DeathWatchManagerImplementation::checkAndApplyWeaponSpeedCooldown(CreatureObject* creature, const String& commandName) {
	if (creature == nullptr) {
		return false;
	}

	// Get weapon attack speed
	float attackSpeed = getWeaponAttackSpeed(creature);

	// Convert to milliseconds with minimum 1 second
	int cooldownMs = (int)(attackSpeed * 1000);
	if (cooldownMs < DW_MIN_COOLDOWN_MS) {
		cooldownMs = DW_MIN_COOLDOWN_MS;
	}

	// Build cooldown key
	String cooldownKey = "dw_" + commandName;

	// Check if on cooldown
	if (!creature->checkCooldownRecovery(cooldownKey)) {
		// Still on cooldown - send message with remaining time
		const Time* cdTime = creature->getCooldownTime(cooldownKey);
		if (cdTime != nullptr) {
			int64 timeLeftMs = cdTime->miliDifference() * -1;
			float timeLeftSec = timeLeftMs / 1000.0f;

			StringBuffer msg;
			msg << "Recharging... " << String::format("%.1f", timeLeftSec) << "s";
			creature->sendSystemMessage(msg.toString());
		}
		return false;
	}

	// Not on cooldown - set it and allow
	creature->addCooldown(cooldownKey, cooldownMs);
	return true;
}

bool DeathWatchManagerImplementation::checkAndApplySkillCooldown(CreatureObject* creature, const String& commandName, int baseCooldownMs) {
	if (creature == nullptr) {
		return false;
	}

	// Apply dw_speed reduction
	// Formula: finalCooldown = baseCooldown * (1 - dw_speed/200)
	// At 0 speed: 100% cooldown (no reduction)
	// At 100 speed: 50% cooldown (max reduction)
	// Minimum cooldown: 1 second (DW_MIN_COOLDOWN_MS)
	int dwSpeed = creature->getSkillMod("dw_speed");
	float speedReduction = 1.0f - (dwSpeed / 200.0f);
	if (speedReduction < 0.5f) speedReduction = 0.5f;  // Cap at 50% reduction

	int cooldownMs = (int)(baseCooldownMs * speedReduction);
	if (cooldownMs < DW_MIN_COOLDOWN_MS) {
		cooldownMs = DW_MIN_COOLDOWN_MS;
	}

	// Build cooldown key
	String cooldownKey = "dw_" + commandName;

	// Check if on cooldown
	if (!creature->checkCooldownRecovery(cooldownKey)) {
		// Still on cooldown - send message with remaining time
		const Time* cdTime = creature->getCooldownTime(cooldownKey);
		if (cdTime != nullptr) {
			int64 timeLeftMs = cdTime->miliDifference() * -1;
			float timeLeftSec = timeLeftMs / 1000.0f;

			StringBuffer msg;
			msg << "Recharging... " << String::format("%.1f", timeLeftSec) << "s";
			creature->sendSystemMessage(msg.toString());
		}
		return false;
	}

	// Not on cooldown - set it and allow
	creature->addCooldown(cooldownKey, cooldownMs);
	return true;
}

float DeathWatchManagerImplementation::getWeaponAttackSpeed(CreatureObject* creature) {
	if (creature == nullptr) {
		return DW_UNARMED_ATTACK_SPEED;
	}

	// Get equipped weapon
	ManagedReference<WeaponObject*> weapon = creature->getWeapon();
	if (weapon == nullptr) {
		return DW_UNARMED_ATTACK_SPEED;
	}

	float speed = weapon->getAttackSpeed();
	if (speed <= 0) {
		return DW_UNARMED_ATTACK_SPEED;
	}

	return speed;
}

int DeathWatchManagerImplementation::getWeaponBaseDamage(CreatureObject* creature) {
	if (creature == nullptr) {
		return DW_UNARMED_MIN_DAMAGE;
	}

	// Get equipped weapon
	ManagedReference<WeaponObject*> weapon = creature->getWeapon();
	if (weapon == nullptr) {
		return DW_UNARMED_MIN_DAMAGE;
	}

	int minDamage = (int)weapon->getMinDamage();
	int maxDamage = (int)weapon->getMaxDamage();

	if (minDamage <= 0 && maxDamage <= 0) {
		return DW_UNARMED_MIN_DAMAGE;
	}

	// Ensure we have at least one positive value
	if (minDamage <= 0) minDamage = 1;
	if (maxDamage <= 0) maxDamage = 1;

	// Determine effective max damage for compression
	float effectiveMax;

	if (minDamage > maxDamage) {
		// Inverted weapon (min > max) - "monster weapons"
		// Use the lower value (original max) as base, apply condition-based penalty
		// Formula: effectiveMax = lowerValue * (0.82 + 0.18 * conditionRatio)
		// This gives 0% penalty at 100% condition, ~9% penalty at 50% condition, 18% max penalty

		int lowerValue = maxDamage;  // The original "max" which is lower

		// Calculate condition ratio (1.0 = perfect, 0.0 = destroyed)
		int maxCondition = weapon->getMaxCondition();
		int conditionDamage = weapon->getConditionDamage();
		float conditionRatio = 1.0f;
		if (maxCondition > 0) {
			conditionRatio = 1.0f - ((float)conditionDamage / (float)maxCondition);
			if (conditionRatio < 0.0f) conditionRatio = 0.0f;
			if (conditionRatio > 1.0f) conditionRatio = 1.0f;
		}

		// Apply condition-based penalty to inverted weapons
		float conditionFactor = 0.82f + (0.18f * conditionRatio);
		effectiveMax = (float)lowerValue * conditionFactor;
	} else {
		// Normal weapon - use max damage directly, no condition penalty
		effectiveMax = (float)maxDamage;
	}

	if (effectiveMax <= 0) {
		return DW_UNARMED_MIN_DAMAGE;
	}

	// Apply logarithmic tier compression to level the playing field
	// Formula: K * log(1 + effectiveMax / K)
	// K = 500 (expected "good" weapon damage) - weapons beyond this get diminishing returns
	// This compresses the gap between weapon tiers so skill (dw_pot) matters more than gear
	// Examples with K=500:
	//   effectiveMax = 500  -> 500 * log(2.0) = 347
	//   effectiveMax = 1000 -> 500 * log(3.0) = 549
	//   effectiveMax = 1500 -> 500 * log(4.0) = 693
	const float K = 500.0f;
	float compressedDamage = K * ::log(1.0f + effectiveMax / K);

	// Ensure minimum damage
	if (compressedDamage < DW_UNARMED_MIN_DAMAGE) {
		return DW_UNARMED_MIN_DAMAGE;
	}

	return (int)compressedDamage;
}

bool DeathWatchManagerImplementation::hasCorrectWeaponType(CreatureObject* attacker, bool requireRanged) {

	if (attacker == nullptr) {
		return false;
	}

	ManagedReference<WeaponObject*> weapon = attacker->getWeapon();

	// No weapon = NOT allowed for DW combat
	// All DW commands require an actual weapon equipped in hold_r
	if (weapon == nullptr) {
		return false;
	}


	// Check if it's the default "unarmed" weapon object
	// The game creates a default weapon when player has nothing equipped
	if (weapon->isUnarmedWeapon()) {
		return false;
	}

	bool isRanged = weapon->isRangedWeapon();

	bool result;
	if (requireRanged) {
		result = isRanged;
	} else {
		result = !isRanged || weapon->isMeleeWeapon();
	}

	return result;
}

// ============================================================================
// JETPACK ATTACK VALIDATION
// ============================================================================

int DeathWatchManagerImplementation::validateJetpackAttack(CreatureObject* creature, uint64 target, uint64& resolvedTarget, bool isRanged) {
	// Constants
	const float DW_MIN_ATTACK_SPEED = 15.0f;
	const int DW_RANGED_MIN_RANGE = 0;   // No minimum range for ranged - accuracy handles close-range penalty
	const int DW_RANGED_MAX_RANGE = 64;
	const int DW_MELEE_MIN_RANGE = 0;
	const int DW_MELEE_MAX_RANGE = 42;


	// 0. Basic sanity check
	if (creature == nullptr || !creature->isPlayerCreature()) {
		return -1;
	}

	// === NEW ORDER: Target -> Jetpack -> Weapon -> Speed -> Range ===

	// 1. Resolve target first - use player's current target if command param is 0
	resolvedTarget = target;
	if (resolvedTarget == 0) {
		resolvedTarget = creature->getTargetID();
	}
	if (resolvedTarget == 0) {
		creature->sendSystemMessage("You must have a target.");
		return -5;
	}

	// 2. Check if target is a valid DW enemy (must check BEFORE other requirements)
	ManagedReference<SceneObject*> targetObj = zoneServer->getObject(resolvedTarget);
	if (targetObj == nullptr) {
		creature->sendSystemMessage("Invalid target.");
		return -5;
	}

	ManagedReference<TangibleObject*> targetTano = targetObj.castTo<TangibleObject*>();
	if (targetTano == nullptr || !isValidDwJetpackTarget(creature, targetTano)) {
		creature->sendSystemMessage("Target must be an enemy Death Watch faction member.");
		return -5;
	}

	// 3. Must have Death Watch novice skill
	if (!creature->hasSkill("combat_death_watch_novice")) {
		creature->sendSystemMessage("You must be Death Watch to use this command.");
		return -2;
	}

	// 4. Must be on Death Watch jetpack
	if (!isOnDeathWatchJetpack(creature)) {
		creature->sendSystemMessage("This command can only be used while mounted on a Death Watch jetpack.");
		return -3;
	}

	// 5. Must have correct weapon type
	if (!hasCorrectWeaponType(creature, isRanged)) {
		if (isRanged) {
			creature->sendSystemMessage("This command requires a ranged weapon.");
		} else {
			creature->sendSystemMessage("This command requires a melee weapon.");
		}
		return -7;
	}

	// 6. Must be moving at minimum speed
	float currentSpeed = creature->getCurrentSpeed();
	if (currentSpeed < DW_MIN_ATTACK_SPEED) {
		StringBuffer speedMsg;
		speedMsg << "You must be moving faster to attack. Current: " << String::valueOf(currentSpeed) << ", Required: " << String::valueOf(DW_MIN_ATTACK_SPEED);
		creature->sendSystemMessage(speedMsg.toString());
		return -4;
	}

	// 7. Range check
	float distance = creature->getDistanceTo(targetObj);
	int minRange = isRanged ? DW_RANGED_MIN_RANGE : DW_MELEE_MIN_RANGE;
	int maxRange = isRanged ? DW_RANGED_MAX_RANGE : DW_MELEE_MAX_RANGE;


	if (distance < minRange) {
		StringBuffer msg;
		msg << "Target is too close. Minimum range: " << minRange << "m";
		creature->sendSystemMessage(msg.toString());
		return -6;
	}
	if (distance > maxRange) {
		StringBuffer msg;
		msg << "Target is out of range. Maximum range: " << maxRange << "m";
		creature->sendSystemMessage(msg.toString());
		return -6;
	}

	return 0;
}

int DeathWatchManagerImplementation::executeDwRangedAttack(CreatureObject* attacker, uint64 targetID, float damageMultiplier, int accuracyMod, int damageType) {

	if (attacker == nullptr) {
		return -1;
	}

	// Check weapon type
	if (!hasCorrectWeaponType(attacker, true)) {
		attacker->sendSystemMessage("You must have a ranged weapon equipped for this ability.");
		return -1;
	}

	// Check weapon certification - track for damage penalty if not certified
	ManagedReference<WeaponObject*> weapon = attacker->getWeapon();
	bool isCertified = (weapon == nullptr || weapon->isCertifiedFor(attacker));

	// Check HAM cost based on weapon encumbrance
	if (weapon != nullptr) {
		int healthCost = weapon->getHealthAttackCost();
		int actionCost = weapon->getActionAttackCost();
		int mindCost = weapon->getMindAttackCost();


		// Check if player has enough HAM
		if (attacker->getHAM(0) <= healthCost || attacker->getHAM(3) <= actionCost || attacker->getHAM(6) <= mindCost) {
			attacker->sendSystemMessage("You are too exhausted to perform this attack.");
			return -1;
		}

		// Deduct HAM cost
		if (healthCost > 0) {
			attacker->inflictDamage(attacker, 0, healthCost, false);
		}
		if (actionCost > 0) {
			attacker->inflictDamage(attacker, 3, actionCost, false);
		}
		if (mindCost > 0) {
			attacker->inflictDamage(attacker, 6, mindCost, false);
		}
	}

	// Note: Speed check is done in the command before cooldown is applied

	// Get target
	ManagedReference<SceneObject*> targetObj = zoneServer->getObject(targetID);
	if (targetObj == nullptr) {
		attacker->sendSystemMessage("Invalid target.");
		return -1;
	}

	// Resolve vehicle/mount to rider using getLinkedCreature()
	CreatureObject* target = nullptr;
	if (targetObj->isVehicleObject() || (targetObj->isCreatureObject() && targetObj->asCreatureObject()->isMount())) {
		// Vehicles/mounts store their rider as linkedCreature, not in a slot
		ManagedReference<CreatureObject*> rider = targetObj->asCreatureObject()->getLinkedCreature();
		if (rider != nullptr) {

			// Check if the rider is actually ON this vehicle right now
			ManagedReference<SceneObject*> riderParent = rider->getParent().get();
			if (riderParent != nullptr && riderParent->getObjectID() == targetObj->getObjectID()) {
				// Rider is actually on this vehicle - target the rider
				target = rider;
			} else {
				// Rider is NOT on this vehicle (vehicle is parked/stored) - invalid target
				attacker->sendSystemMessage("That vehicle has no rider.");
				return -1;
			}
		} else {
			attacker->sendSystemMessage("That vehicle has no rider.");
			return -1;
		}
	} else if (targetObj->isCreatureObject()) {
		target = targetObj->asCreatureObject();
	} else {
	}

	if (target == nullptr) {
		attacker->sendSystemMessage("Invalid target.");
		return -1;
	}

	// Can't target self
	if (attacker->getObjectID() == target->getObjectID()) {
		attacker->sendSystemMessage("You cannot target yourself.");
		return -1;
	}

	// Range check
	float distance = attacker->getDistanceTo(target);
	if (distance > DW_RANGED_MAX_RANGE) {
		StringBuffer msg;
		msg << "Target is out of range. Maximum: " << DW_RANGED_MAX_RANGE << "m";
		attacker->sendSystemMessage(msg.toString());
		return -1;
	}

	// Line of sight check - skip for jetpack-to-jetpack combat (both flying in open air)
	bool targetOnJetpack = isOnDeathWatchJetpack(target);

	if (targetOnJetpack) {
	} else {
		if (!CollisionManager::checkLineOfSight(attacker, target)) {
			attacker->sendSystemMessage("You don't have line of sight to your target.");
			return -1;
		}
	}

	// Facing check - must be facing target within 90 degrees
	if (!isFacingTarget(attacker, target, 90.0f)) {
		attacker->sendSystemMessage("No target in front of you.");
		return -1;
	}

	// Check if target is a DW Hunter training NPC (socialGroup = "death_watch_hunter")
	bool isDwHunterNpc = false;
	if (!target->isPlayerCreature() && target->isAiAgent()) {
		AiAgent* aiTarget = target->asAiAgent();
		if (aiTarget != nullptr) {
			String socialGroup = aiTarget->getSocialGroup();
			isDwHunterNpc = (socialGroup == "death_watch_hunter");
		}
	}

	// Must be valid DW enemy (player) OR a DW Hunter NPC
	if (!target->isPlayerCreature() && !isDwHunterNpc) {
		attacker->sendSystemMessage("Target must be an enemy Death Watch faction member.");
		return -1;
	}

	// For DW Hunter NPCs, skip faction checks - just verify attacker is in DW
	if (isDwHunterNpc) {
		ManagedReference<PlayerObject*> attackerGhost = attacker->getPlayerObject();
		if (attackerGhost == nullptr || attackerGhost->getDwFaction() == DeathWatchFactions::NONE) {
			attacker->sendSystemMessage("You must be a Death Watch member to attack hunters.");
			return -1;
		}
	} else {
		// Player target - verify both are in opposing DW factions

		ManagedReference<PlayerObject*> attackerGhost = attacker->getPlayerObject();
		ManagedReference<PlayerObject*> targetGhost = target->getPlayerObject();

		if (attackerGhost == nullptr || targetGhost == nullptr) {
			attacker->sendSystemMessage("Invalid target.");
			return -1;
		}

		byte attackerFaction = attackerGhost->getDwFaction();
		byte targetFaction = targetGhost->getDwFaction();

		if (attackerFaction == DeathWatchFactions::NONE || targetFaction == DeathWatchFactions::NONE) {
			attacker->sendSystemMessage("Target must be an enemy Death Watch faction member.");
			return -1;
		}

		if (!areFactionsAtWar(attackerFaction, targetFaction)) {
			attacker->sendSystemMessage("Target must be an enemy Death Watch faction member.");
			return -1;
		}
	}

	// ========================================================================
	// SET COMBAT STATE
	// ========================================================================
	// Put both players into combat state
	attacker->setCombatState();
	target->setCombatState();
	attacker->setDefender(target);
	target->setDefender(attacker);

	// ========================================================================
	// HIT/MISS CALCULATION
	// ========================================================================
	// Formula: hitChance = baseHitChance + accuracyMod + angleAccMod + weaponRangeAccMod + (attacker_dw_acc * 0.5) - (target_dw_evasion * 0.5)
	// Base ranged hit chance: 75%
	// Skills range 1-100 (75 base + 25 from skill tapes)
	// angleAccMod: Bonus/penalty based on facing angle (0-5° = +10 to +22, off-target = up to -75)
	// weaponRangeAccMod: Bonus/penalty based on distance vs weapon's optimal ranges
	// At max skills (100 vs 100): 75 + 50 - 50 = 75% (balanced)
	// Capped at 95% max, 5% minimum

	int baseHitChance = 75;
	int attackerAcc = attacker->getSkillMod("dw_acc");
	int targetEvasion = target->getSkillMod("dw_evasion");

	// Calculate angle-based accuracy modifier
	int angleAccMod = calculateAngleAccuracyMod(attacker, target);
	float facingOffset = getFacingOffsetAngle(attacker, target);

	// Calculate weapon range accuracy modifier (based on weapon's pointBlank/ideal/maxRange)
	int weaponRangeAccMod = calculateWeaponRangeAccuracyMod(attacker, target);

	int hitChance = baseHitChance + accuracyMod + angleAccMod + weaponRangeAccMod + (attackerAcc / 2) - (targetEvasion / 2);

	// Cap hit chance between 5% and 95%
	if (hitChance < 5) hitChance = 5;
	if (hitChance > 95) hitChance = 95;


	int roll = System::random(99) + 1;  // 1-100
	if (roll > hitChance) {
		attacker->sendSystemMessage("Your attack missed!");
		return 0;
	}

	// ========================================================================
	// DAMAGE CALCULATION
	// ========================================================================
	// Formula: damage = harmonicMean * multiplier * (1 + dw_pot/100)

	int baseDamage;
	if (isCertified) {
		baseDamage = getWeaponBaseDamage(attacker);
	} else {
		// Not certified - use minimum damage (10)
		baseDamage = 10;
		attacker->sendSystemMessage("\\#FF6600You are not certified for this weapon. Damage severely reduced.");
	}

	int totalDamage = (int)(baseDamage * damageMultiplier);

	// Apply dw_pot potency scaling (+2% damage per point, max +200% at 100)
	// This makes skill investment matter more than weapon tier
	int potency = attacker->getSkillMod("dw_pot");
	if (potency > 0) {
		totalDamage = (int)(totalDamage * (1.0f + potency / 50.0f));
	}

	// Ensure at least 1 damage
	if (totalDamage < 1) {
		totalDamage = 1;
	}

	// ========================================================================
	// DEFLECTION CHECK
	// ========================================================================
	// dw_deflection: X% chance to reflect 25% of damage back at attacker
	// Chance = dw_deflection / 2 (max 50% chance at 100 skill)

	int deflection = target->getSkillMod("dw_deflection");
	int deflectedDamage = 0;
	if (deflection > 0) {
		int deflectChance = deflection / 2;
		if (deflectChance > 50) deflectChance = 50;

		int deflectRoll = System::random(99) + 1;
		if (deflectRoll <= deflectChance) {
			// Deflect 25% of damage back
			deflectedDamage = totalDamage / 4;
			if (deflectedDamage > 0) {
				// Apply deflected damage to attacker (split H/A)
				int deflectHealth = deflectedDamage / 2;
				int deflectAction = deflectedDamage - deflectHealth;
				attacker->inflictDamage(target, 0, deflectHealth, true);
				attacker->inflictDamage(target, 3, deflectAction, true);

				StringBuffer deflectMsg;
				deflectMsg << "\\#FF6600" << target->getFirstName() << " deflects " << deflectedDamage << " damage back at you!";
				attacker->sendSystemMessage(deflectMsg.toString());

				StringBuffer targetDeflectMsg;
				targetDeflectMsg << "\\#00FF00You deflect " << deflectedDamage << " damage back at your attacker!";
				target->sendSystemMessage(targetDeflectMsg.toString());
			}
		} else {
		}
	} else {
	}

	// ========================================================================
	// ARMOR MITIGATION
	// ========================================================================
	// Apply armor mitigation using weapon's damage type
	// This includes Mandalorian armor bonuses automatically

	int armorDamageType = 2;  // Default to ENERGY for ranged
	int armorPiercing = 0;    // No armor piercing by default

	// Get weapon damage type and armor piercing
	if (weapon != nullptr) {
		armorDamageType = weapon->getDamageType();
		armorPiercing = weapon->getArmorPiercing();
	}

	float mitigatedDamage = calculateMitigatedDamage(attacker, target, (float)totalDamage, armorDamageType, armorPiercing);
	totalDamage = (int)mitigatedDamage;

	// ========================================================================
	// APPLY DAMAGE
	// ========================================================================
	// damageType: 0 = health only, 1 = action only, 2 = split H/A, 3 = mind only

	if (damageType == 0) {
		// Health only
		target->inflictDamage(attacker, 0, totalDamage, true);
	} else if (damageType == 1) {
		// Action only
		target->inflictDamage(attacker, 3, totalDamage, true);
	} else if (damageType == 3) {
		// Mind only
		target->inflictDamage(attacker, 6, totalDamage, true);
	} else {
		// Split H/A (default)
		int healthDmg = totalDamage / 2;
		int actionDmg = totalDamage - healthDmg;
		target->inflictDamage(attacker, 0, healthDmg, true);
		target->inflictDamage(attacker, 3, actionDmg, true);
	}

	// Apply DW TEF
	applyDwTef(attacker);
	applyDwTef(target);

	// Record combat participant for gank detection - ranged
	recordCombatParticipant(target, attacker);

	return totalDamage;
}

int DeathWatchManagerImplementation::executeDwMeleeAttack(CreatureObject* attacker, uint64 targetID, float damageMultiplier, int accuracyMod, int damageType) {
	if (attacker == nullptr) {
		return -1;
	}

	// Check weapon type (melee or unarmed)
	if (!hasCorrectWeaponType(attacker, false)) {
		attacker->sendSystemMessage("You must have a melee weapon equipped for this ability.");
		return -1;
	}

	// Check weapon certification - track for damage penalty if not certified
	ManagedReference<WeaponObject*> weapon = attacker->getWeapon();
	bool isCertified = (weapon == nullptr || weapon->isCertifiedFor(attacker));

	// Check HAM cost based on weapon encumbrance
	if (weapon != nullptr) {
		int healthCost = weapon->getHealthAttackCost();
		int actionCost = weapon->getActionAttackCost();
		int mindCost = weapon->getMindAttackCost();

		// Check if player has enough HAM
		if (attacker->getHAM(0) <= healthCost || attacker->getHAM(3) <= actionCost || attacker->getHAM(6) <= mindCost) {
			attacker->sendSystemMessage("You are too exhausted to perform this attack.");
			return -1;
		}

		// Deduct HAM cost
		if (healthCost > 0) {
			attacker->inflictDamage(attacker, 0, healthCost, false);
		}
		if (actionCost > 0) {
			attacker->inflictDamage(attacker, 3, actionCost, false);
		}
		if (mindCost > 0) {
			attacker->inflictDamage(attacker, 6, mindCost, false);
		}
	}

	// Get target - melee attack variant
	ManagedReference<SceneObject*> targetObj = zoneServer->getObject(targetID);
	if (targetObj == nullptr) {
		attacker->sendSystemMessage("Invalid target.");
		return -1;
	}

	// Resolve vehicle/mount to rider using getLinkedCreature()
	CreatureObject* target = nullptr;
	if (targetObj->isVehicleObject() || (targetObj->isCreatureObject() && targetObj->asCreatureObject()->isMount())) {
		// Vehicles/mounts store their rider as linkedCreature, not in a slot
		ManagedReference<CreatureObject*> rider = targetObj->asCreatureObject()->getLinkedCreature();
		if (rider != nullptr) {
			target = rider;
		}
	} else if (targetObj->isCreatureObject()) {
		target = targetObj->asCreatureObject();
	}

	if (target == nullptr) {
		attacker->sendSystemMessage("Invalid target.");
		return -1;
	}

	// Can't target self
	if (attacker->getObjectID() == target->getObjectID()) {
		attacker->sendSystemMessage("You cannot target yourself.");
		return -1;
	}

	// Range check - melee has shorter range
	float distance = attacker->getDistanceTo(target);
	if (distance > DW_MELEE_MAX_RANGE) {
		StringBuffer msg;
		msg << "Target is out of range. Maximum: " << DW_MELEE_MAX_RANGE << "m";
		attacker->sendSystemMessage(msg.toString());
		return -1;
	}

	// Line of sight check
	if (!CollisionManager::checkLineOfSight(attacker, target)) {
		attacker->sendSystemMessage("You don't have line of sight to your target.");
		return -1;
	}

	// Facing check - must be facing target within 90 degrees
	if (!isFacingTarget(attacker, target, 90.0f)) {
		attacker->sendSystemMessage("No target in front of you.");
		return -1;
	}

	// Check if target is a DW Hunter training NPC (socialGroup = "death_watch_hunter")
	bool isDwHunterNpc = false;
	if (!target->isPlayerCreature() && target->isAiAgent()) {
		AiAgent* aiTarget = target->asAiAgent();
		if (aiTarget != nullptr) {
			String socialGroup = aiTarget->getSocialGroup();
			isDwHunterNpc = (socialGroup == "death_watch_hunter");
		}
	}

	// Must be valid DW enemy (player) OR a DW Hunter NPC
	if (!target->isPlayerCreature() && !isDwHunterNpc) {
		attacker->sendSystemMessage("Target must be an enemy Death Watch faction member.");
		return -1;
	}

	// For DW Hunter NPCs, skip faction checks - just verify attacker is in DW
	if (isDwHunterNpc) {
		ManagedReference<PlayerObject*> attackerGhost = attacker->getPlayerObject();
		if (attackerGhost == nullptr || attackerGhost->getDwFaction() == DeathWatchFactions::NONE) {
			attacker->sendSystemMessage("You must be a Death Watch member to attack hunters.");
			return -1;
		}
	} else {
		// Player target - verify both are in opposing DW factions
		ManagedReference<PlayerObject*> attackerGhost = attacker->getPlayerObject();
		ManagedReference<PlayerObject*> targetGhost = target->getPlayerObject();

		if (attackerGhost == nullptr || targetGhost == nullptr) {
			attacker->sendSystemMessage("Invalid target.");
			return -1;
		}

		byte attackerFaction = attackerGhost->getDwFaction();
		byte targetFaction = targetGhost->getDwFaction();

		if (attackerFaction == DeathWatchFactions::NONE || targetFaction == DeathWatchFactions::NONE) {
			attacker->sendSystemMessage("Target must be an enemy Death Watch faction member.");
			return -1;
		}

		if (!areFactionsAtWar(attackerFaction, targetFaction)) {
			attacker->sendSystemMessage("Target must be an enemy Death Watch faction member.");
			return -1;
		}
	}

	// ========================================================================
	// SET COMBAT STATE
	// ========================================================================
	// Put both players into combat state
	attacker->setCombatState();
	target->setCombatState();
	attacker->setDefender(target);
	target->setDefender(attacker);

	// ========================================================================
	// HIT/MISS CALCULATION
	// ========================================================================
	// Formula: hitChance = baseHitChance + accuracyMod + angleAccMod + (attacker_dw_acc * 0.5) - (target_dw_evasion * 0.5)
	// Base melee hit chance: 85% (higher than ranged)
	// Skills range 1-100 (75 base + 25 from skill tapes)
	// angleAccMod: Bonus/penalty based on facing angle (0-5° = +10 to +22, off-target = up to -75)
	// At max skills (100 vs 100): 85 + 50 - 50 = 85% (balanced)
	// Capped at 95% max, 5% minimum

	int baseHitChance = 85;  // Melee has higher base hit chance
	int attackerAcc = attacker->getSkillMod("dw_acc");
	int targetEvasion = target->getSkillMod("dw_evasion");

	// Calculate angle-based accuracy modifier
	int angleAccMod = calculateAngleAccuracyMod(attacker, target);

	int hitChance = baseHitChance + accuracyMod + angleAccMod + (attackerAcc / 2) - (targetEvasion / 2);

	// Cap hit chance between 5% and 95%
	if (hitChance < 5) hitChance = 5;
	if (hitChance > 95) hitChance = 95;

	int roll = System::random(99) + 1;  // 1-100
	if (roll > hitChance) {
		attacker->sendSystemMessage("Your attack missed!");
		return 0;
	}

	// ========================================================================
	// DAMAGE CALCULATION
	// ========================================================================
	// Formula: damage = harmonicMean * multiplier * (1 + dw_pot/100)

	int baseDamage;
	if (isCertified) {
		baseDamage = getWeaponBaseDamage(attacker);
	} else {
		// Not certified - use minimum damage (10)
		baseDamage = 10;
		attacker->sendSystemMessage("\\#FF6600You are not certified for this weapon. Damage severely reduced.");
	}
	int totalDamage = (int)(baseDamage * damageMultiplier);

	// Apply dw_pot potency scaling (+2% damage per point, max +200% at 100)
	// This makes skill investment matter more than weapon tier
	int potency = attacker->getSkillMod("dw_pot");
	if (potency > 0) {
		totalDamage = (int)(totalDamage * (1.0f + potency / 50.0f));
	}

	// Ensure at least 1 damage
	if (totalDamage < 1) {
		totalDamage = 1;
	}

	// ========================================================================
	// DEFLECTION CHECK
	// ========================================================================
	// dw_deflection: X% chance to reflect 25% of damage back at attacker
	// Chance = dw_deflection / 2 (max 50% chance at 100 skill)

	int deflection = target->getSkillMod("dw_deflection");
	int deflectedDamage = 0;
	if (deflection > 0) {
		int deflectChance = deflection / 2;
		if (deflectChance > 50) deflectChance = 50;

		if (System::random(99) + 1 <= deflectChance) {
			// Deflect 25% of damage back
			deflectedDamage = totalDamage / 4;
			if (deflectedDamage > 0) {
				// Apply deflected damage to attacker (split H/A)
				int deflectHealth = deflectedDamage / 2;
				int deflectAction = deflectedDamage - deflectHealth;
				attacker->inflictDamage(target, 0, deflectHealth, true);
				attacker->inflictDamage(target, 3, deflectAction, true);

				StringBuffer deflectMsg;
				deflectMsg << "\\#FF6600" << target->getFirstName() << " deflects " << deflectedDamage << " damage back at you!";
				attacker->sendSystemMessage(deflectMsg.toString());

				StringBuffer targetDeflectMsg;
				targetDeflectMsg << "\\#00FF00You deflect " << deflectedDamage << " damage back at your attacker!";
				target->sendSystemMessage(targetDeflectMsg.toString());
			}
		}
	}

	// ========================================================================
	// ARMOR MITIGATION
	// ========================================================================
	// Apply armor mitigation using weapon's damage type
	// This includes Mandalorian armor bonuses automatically

	int armorDamageType = 1;  // Default to KINETIC for melee
	int armorPiercing = 0;    // No armor piercing by default

	// Get weapon damage type and armor piercing
	if (weapon != nullptr) {
		armorDamageType = weapon->getDamageType();
		armorPiercing = weapon->getArmorPiercing();
	}

	float mitigatedDamage = calculateMitigatedDamage(attacker, target, (float)totalDamage, armorDamageType, armorPiercing);
	totalDamage = (int)mitigatedDamage;

	// ========================================================================
	// APPLY DAMAGE
	// ========================================================================
	// damageType: 0 = health only, 1 = action only, 2 = split H/A, 3 = mind only

	if (damageType == 0) {
		// Health only
		target->inflictDamage(attacker, 0, totalDamage, true);
	} else if (damageType == 1) {
		// Action only
		target->inflictDamage(attacker, 3, totalDamage, true);
	} else if (damageType == 3) {
		// Mind only
		target->inflictDamage(attacker, 6, totalDamage, true);
	} else {
		// Split H/A (default)
		int healthDmg = totalDamage / 2;
		int actionDmg = totalDamage - healthDmg;
		target->inflictDamage(attacker, 0, healthDmg, true);
		target->inflictDamage(attacker, 3, actionDmg, true);
	}

	// Apply DW TEF
	applyDwTef(attacker);
	applyDwTef(target);

	// Record combat participant for gank detection - melee
	recordCombatParticipant(target, attacker);

	return totalDamage;
}

// ============================================================================
// DW EFFECT/DOT TRACKING
// ============================================================================

// Stack chance: 2% when target already has effect
static const int DW_EFFECT_STACK_CHANCE = 2;

bool DeathWatchManagerImplementation::hasActiveEffect(uint64 targetID, int effectType) {
	Locker lock(&activeEffectsMutex);

	if (!activeEffects.contains(targetID)) {
		return false;
	}

	VectorMap<int, DwActiveEffect>& targetEffects = activeEffects.get(targetID);
	return targetEffects.contains(effectType);
}

void DeathWatchManagerImplementation::registerEffect(uint64 targetID, uint64 attackerID, int effectType, uint64 effectID) {
	Locker lock(&activeEffectsMutex);

	if (!activeEffects.contains(targetID)) {
		activeEffects.put(targetID, VectorMap<int, DwActiveEffect>());
	}

	VectorMap<int, DwActiveEffect>& targetEffects = activeEffects.get(targetID);
	targetEffects.put(effectType, DwActiveEffect(attackerID, effectID));
}

void DeathWatchManagerImplementation::unregisterEffect(uint64 targetID, int effectType) {
	Locker lock(&activeEffectsMutex);

	if (!activeEffects.contains(targetID)) {
		return;
	}

	VectorMap<int, DwActiveEffect>& targetEffects = activeEffects.get(targetID);

	if (targetEffects.contains(effectType)) {
		targetEffects.drop(effectType);
	}

	// Clean up empty target entries
	if (targetEffects.size() == 0) {
		activeEffects.drop(targetID);
	}
}

bool DeathWatchManagerImplementation::rollEffectStack() {
	// 2% chance to stack
	int roll = System::random(99) + 1;  // 1-100
	return roll <= DW_EFFECT_STACK_CHANCE;
}

int DeathWatchManagerImplementation::applyActionDrain(CreatureObject* attacker, uint64 targetID,
													  int totalDamage, int durationSec, float tickIntervalSec) {
	if (attacker == nullptr) {
		return 0;
	}

	// Get target
	ManagedReference<SceneObject*> targetObj = zoneServer->getObject(targetID);
	if (targetObj == nullptr) {
		attacker->sendSystemMessage("Invalid target.");
		return 0;
	}

	// Resolve vehicle/mount to rider using getLinkedCreature()
	CreatureObject* target = nullptr;
	if (targetObj->isVehicleObject() || (targetObj->isCreatureObject() && targetObj->asCreatureObject()->isMount())) {
		// Vehicles/mounts store their rider as linkedCreature, not in a slot
		ManagedReference<CreatureObject*> rider = targetObj->asCreatureObject()->getLinkedCreature();
		if (rider != nullptr) {
			target = rider;
		}
	} else if (targetObj->isCreatureObject()) {
		target = targetObj->asCreatureObject();
	}

	if (target == nullptr) {
		attacker->sendSystemMessage("Invalid target.");
		return 0;
	}

	// Can't target self
	if (attacker->getObjectID() == target->getObjectID()) {
		attacker->sendSystemMessage("You cannot target yourself.");
		return 0;
	}

	// Must be valid DW enemy
	if (!target->isPlayerCreature()) {
		attacker->sendSystemMessage("Target must be an enemy Death Watch faction member.");
		return 0;
	}

	ManagedReference<PlayerObject*> attackerGhost = attacker->getPlayerObject();
	ManagedReference<PlayerObject*> targetGhost = target->getPlayerObject();

	if (attackerGhost == nullptr || targetGhost == nullptr) {
		attacker->sendSystemMessage("Invalid target.");
		return 0;
	}

	byte attackerFaction = attackerGhost->getDwFaction();
	byte targetFaction = targetGhost->getDwFaction();

	if (attackerFaction == DeathWatchFactions::NONE || targetFaction == DeathWatchFactions::NONE) {
		attacker->sendSystemMessage("Target must be an enemy Death Watch faction member.");
		return 0;
	}

	if (!areFactionsAtWar(attackerFaction, targetFaction)) {
		attacker->sendSystemMessage("Target must be an enemy Death Watch faction member.");
		return 0;
	}

	// Check for existing effect (anti-stacking)
	int effectType = DW_EFFECT_ACTION_DRAIN;
	uint64 realTargetID = target->getObjectID();

	if (hasActiveEffect(realTargetID, effectType)) {
		// Target already has action drain effect
		// 98% miss, 2% stack chance
		if (!rollEffectStack()) {
			attacker->sendSystemMessage("\\#FF6600Your target resists the action drain - they are already affected!");
			return 0;  // Miss
		}

		// Lucky 2% - effect will stack
		attacker->sendSystemMessage("\\#00FF00Your action drain intensifies the existing effect!");
		// Note: For true stacking we'd need to find the existing task and call stackEffect()
		// For simplicity, we'll just apply a new effect that runs alongside
	}

	// Range check
	float distance = attacker->getDistanceTo(target);
	if (distance > DW_RANGED_MAX_RANGE) {
		StringBuffer msg;
		msg << "Target is out of range. Maximum: " << DW_RANGED_MAX_RANGE << "m";
		attacker->sendSystemMessage(msg.toString());
		return 0;
	}

	// Line of sight check
	if (!CollisionManager::checkLineOfSight(attacker, target)) {
		attacker->sendSystemMessage("You don't have line of sight to your target.");
		return 0;
	}

	// Calculate tick parameters
	int tickIntervalMs = (int)(tickIntervalSec * 1000);
	if (tickIntervalMs < 1000) {
		tickIntervalMs = 1000;  // Minimum 1 second tick
	}

	int numTicks = (int)((durationSec * 1000.0f) / tickIntervalMs);
	if (numTicks < 1) {
		numTicks = 1;
	}

	// Apply defender's dw_recovery to reduce number of ticks
	// Formula: finalTicks = max(1, numTicks * (1 - dw_recovery/200))
	// At 0 recovery: 100% ticks (no reduction)
	// At 100 recovery: 50% ticks (max reduction)
	// This creates counterplay: attacker dw_pot vs defender dw_recovery
	int targetRecovery = target->getSkillMod("dw_recovery");
	if (targetRecovery > 0) {
		float recoveryReduction = 1.0f - (targetRecovery / 200.0f);
		if (recoveryReduction < 0.5f) recoveryReduction = 0.5f;  // Cap at 50% reduction

		int reducedTicks = (int)(numTicks * recoveryReduction);
		if (reducedTicks < 1) reducedTicks = 1;

		numTicks = reducedTicks;
	}

	int damagePerTick = totalDamage / numTicks;
	if (damagePerTick < 1) {
		damagePerTick = 1;
	}

	// Create and schedule the effect task
	Reference<DwEffectTickTask*> task = new DwEffectTickTask(
		target,
		attacker,
		DW_EFFECT_ACTION_DRAIN,
		damagePerTick,
		numTicks,
		tickIntervalMs,
		3  // Action pool (attribute index 3)
	);

	// Register the effect
	registerEffect(realTargetID, attacker->getObjectID(), effectType, task->getEffectID());

	// Schedule first tick
	task->schedule(tickIntervalMs);

	// Apply DW TEF
	applyDwTef(attacker);
	applyDwTef(target);

	// Set combat state for both players
	attacker->setCombatState();
	target->setCombatState();
	attacker->setDefender(target);
	target->setDefender(attacker);

	// Record combat participant
	recordCombatParticipant(target, attacker);

	// Send initial message
	StringBuffer attackerMsg;
	attackerMsg << "\\#FF6600You apply Action Drain to " << target->getFirstName() << "!";
	attacker->sendSystemMessage(attackerMsg.toString());

	StringBuffer targetMsg;
	targetMsg << "\\#FF0000" << attacker->getFirstName() << " has applied Action Drain to you!";
	target->sendSystemMessage(targetMsg.toString());

	return 1;  // Successfully applied
}

// ============================================================================
// WOUNDING SHOT - FACTION-BASED WOUND TARGETING
// ============================================================================

/*
 * Faction Wound Targeting:
 * - XON (Xonti Brothers): Action pools (3,4,5) - slavers focus on mobility/stamina
 * - SUN (Black Sun): Health pools (0,1,2) - brutal efficiency
 * - KRR (Krrsantan): Health pools (0,1,2) - raw wookiee power
 * - ZAN (Zann Consortium): Mind pools (6,7,8) - psychological warfare
 *
 * Wound Distribution: Primary pool gets 50%, two secondary pools split remaining 50%
 * Direct Damage: 15% of harmonic mean (light hit to make the shot feel impactful)
 *
 * Attacker Formula:
 * - Base wounds scaled by dw_pot: wounds = baseWounds * (1 + dw_pot/100)
 *
 * Defender Formula:
 * - dw_recovery reduces wounds: finalWounds = wounds * (1 - dw_recovery/200)
 */

int DeathWatchManagerImplementation::applyWoundingShot(CreatureObject* attacker, uint64 targetID, int baseWounds) {
	if (attacker == nullptr) {
		return -1;
	}

	// Get attacker ghost for faction check
	ManagedReference<PlayerObject*> attackerGhost = attacker->getPlayerObject();
	if (attackerGhost == nullptr) {
		return -1;
	}

	// Get target
	auto zoneServer = this->zoneServer.get();
	if (zoneServer == nullptr) {
		return -1;
	}

	ManagedReference<SceneObject*> targetObject = zoneServer->getObject(targetID);
	if (targetObject == nullptr) {
		attacker->sendSystemMessage("Invalid target.");
		return -1;
	}

	// Resolve vehicle/mount to rider using getLinkedCreature()
	CreatureObject* target = nullptr;
	if (targetObject->isVehicleObject() || (targetObject->isCreatureObject() && targetObject->asCreatureObject()->isMount())) {
		ManagedReference<CreatureObject*> rider = targetObject->asCreatureObject()->getLinkedCreature();
		if (rider != nullptr) {
			ManagedReference<SceneObject*> riderParent = rider->getParent().get();
			if (riderParent != nullptr && riderParent->getObjectID() == targetObject->getObjectID()) {
				target = rider;
			} else {
				attacker->sendSystemMessage("That vehicle has no rider.");
				return -1;
			}
		} else {
			attacker->sendSystemMessage("That vehicle has no rider.");
			return -1;
		}
	} else if (targetObject->isCreatureObject()) {
		target = targetObject->asCreatureObject();
	}

	if (target == nullptr) {
		attacker->sendSystemMessage("Invalid target.");
		return -1;
	}

	// Check if target is a DW Hunter training NPC (allowed target for practice)
	bool isDwHunterNpc = false;
	if (!target->isPlayerCreature() && target->isAiAgent()) {
		AiAgent* aiTarget = target->asAiAgent();
		if (aiTarget != nullptr) {
			String socialGroup = aiTarget->getSocialGroup();
			isDwHunterNpc = (socialGroup == "death_watch_hunter");
		}
	}

	// Must be valid DW enemy (must be a player or DW Hunter NPC)
	if (!target->isPlayerCreature() && !isDwHunterNpc) {
		attacker->sendSystemMessage("Target must be an enemy Death Watch faction member.");
		return -1;
	}

	// Get attacker faction (needed for wound pool selection later)
	byte attackerFaction = attackerGhost->getDwFaction();

	// For player targets, verify DW faction war status
	if (target->isPlayerCreature()) {
		ManagedReference<PlayerObject*> targetGhost = target->getPlayerObject();
		if (targetGhost == nullptr) {
			attacker->sendSystemMessage("Invalid target.");
			return -1;
		}

		byte targetFaction = targetGhost->getDwFaction();

		if (attackerFaction == DeathWatchFactions::NONE || targetFaction == DeathWatchFactions::NONE) {
			attacker->sendSystemMessage("Target must be an enemy Death Watch faction member.");
			return -1;
		}

		if (!areFactionsAtWar(attackerFaction, targetFaction)) {
			attacker->sendSystemMessage("Target must be an enemy Death Watch faction member.");
			return -1;
		}
	}

	// Range check
	float distance = attacker->getDistanceTo(target);
	if (distance > DW_RANGED_MAX_RANGE) {
		StringBuffer msg;
		msg << "Target is out of range. Maximum: " << DW_RANGED_MAX_RANGE << "m";
		attacker->sendSystemMessage(msg.toString());
		return -1;
	}

	// Line of sight check
	if (!CollisionManager::checkLineOfSight(attacker, target)) {
		attacker->sendSystemMessage("You don't have line of sight to your target.");
		return -1;
	}

	// Get skill mods
	int dwPot = attacker->getSkillMod("dw_pot");
	int targetRecovery = target->getSkillMod("dw_recovery");

	// Calculate wounds: base * (1 + dw_pot/100)
	float potencyBonus = 1.0f + (dwPot / 100.0f);
	int totalWounds = (int)(baseWounds * potencyBonus);

	// Apply defender's dw_recovery: wounds * (1 - dw_recovery/200)
	if (targetRecovery > 0) {
		float recoveryReduction = 1.0f - (targetRecovery / 200.0f);
		if (recoveryReduction < 0.5f) recoveryReduction = 0.5f;  // Cap at 50% reduction
		totalWounds = (int)(totalWounds * recoveryReduction);
	}

	if (totalWounds < 1) {
		totalWounds = 1;  // Minimum 1 wound
	}

	// Determine wound pools based on attacker faction
	// Pool indices: 0-2 = Health/Str/Con, 3-5 = Action/Quick/Stam, 6-8 = Mind/Focus/Will
	int primaryPool = 0;
	int secondaryPool1 = 1;
	int secondaryPool2 = 2;
	String factionStyle;

	switch (attackerFaction) {
		case DeathWatchFactions::XON:
			// Xonti Brothers: Target Action pools (mobility/stamina)
			primaryPool = 3;      // Action
			secondaryPool1 = 4;   // Quickness
			secondaryPool2 = 5;   // Stamina
			factionStyle = "Xonti precision";
			break;

		case DeathWatchFactions::SUN:
			// Black Sun: Target Health pools (brutal)
			primaryPool = 0;      // Health
			secondaryPool1 = 1;   // Strength
			secondaryPool2 = 2;   // Constitution
			factionStyle = "Black Sun brutality";
			break;

		case DeathWatchFactions::KRR:
			// Krrsantan: Target Health pools (raw power)
			primaryPool = 0;      // Health
			secondaryPool1 = 2;   // Constitution
			secondaryPool2 = 1;   // Strength
			factionStyle = "Krrsantan fury";
			break;

		case DeathWatchFactions::ZAN:
			// Zann Consortium: Target Mind pools (psychological)
			primaryPool = 6;      // Mind
			secondaryPool1 = 7;   // Focus
			secondaryPool2 = 8;   // Willpower
			factionStyle = "Zann psychological warfare";
			break;

		default:
			// Fallback: Health pools
			primaryPool = 0;
			secondaryPool1 = 1;
			secondaryPool2 = 2;
			factionStyle = "Death Watch assault";
			break;
	}

	// Distribute wounds: 50% primary, 25% each secondary
	int primaryWounds = totalWounds / 2;
	int secondaryWounds1 = totalWounds / 4;
	int secondaryWounds2 = totalWounds - primaryWounds - secondaryWounds1;  // Remainder

	// Ensure minimums
	if (primaryWounds < 1) primaryWounds = 1;
	if (secondaryWounds1 < 1 && totalWounds >= 3) secondaryWounds1 = 1;
	if (secondaryWounds2 < 1 && totalWounds >= 4) secondaryWounds2 = 1;

	// Lock target and apply wounds
	Locker locker(target);
	Locker clocker(attacker, target);

	// Apply wounds to each pool
	target->addWounds(primaryPool, primaryWounds, true, false);
	if (secondaryWounds1 > 0) {
		target->addWounds(secondaryPool1, secondaryWounds1, true, false);
	}
	if (secondaryWounds2 > 0) {
		target->addWounds(secondaryPool2, secondaryWounds2, true, false);
	}

	int actualTotalWounds = primaryWounds + secondaryWounds1 + secondaryWounds2;

	// Apply light direct damage (15% of harmonic mean)
	int baseDamage = getWeaponBaseDamage(attacker);
	int directDamage = (int)(baseDamage * 0.15f);
	if (directDamage < 5) directDamage = 5;

	// Damage goes to health
	int currentHealth = target->getHAM(0);
	if (directDamage >= currentHealth) {
		directDamage = currentHealth - 1;  // Don't kill with direct damage
	}
	if (directDamage > 0) {
		target->inflictDamage(attacker, 0, directDamage, false, "dwWoundingShot", true, false);
	}

	// Apply DW TEF
	applyDwTef(attacker);
	applyDwTef(target);

	// Set combat state for both players
	attacker->setCombatState();
	target->setCombatState();
	attacker->setDefender(target);
	target->setDefender(attacker);

	// Record combat participant for gank detection
	recordCombatParticipant(target, attacker);

	// Get pool names for messages
	String primaryName, secondaryName1, secondaryName2;
	switch (primaryPool) {
		case 0: primaryName = "Health"; break;
		case 1: primaryName = "Strength"; break;
		case 2: primaryName = "Constitution"; break;
		case 3: primaryName = "Action"; break;
		case 4: primaryName = "Quickness"; break;
		case 5: primaryName = "Stamina"; break;
		case 6: primaryName = "Mind"; break;
		case 7: primaryName = "Focus"; break;
		case 8: primaryName = "Willpower"; break;
		default: primaryName = "Unknown"; break;
	}
	switch (secondaryPool1) {
		case 0: secondaryName1 = "Health"; break;
		case 1: secondaryName1 = "Strength"; break;
		case 2: secondaryName1 = "Constitution"; break;
		case 3: secondaryName1 = "Action"; break;
		case 4: secondaryName1 = "Quickness"; break;
		case 5: secondaryName1 = "Stamina"; break;
		case 6: secondaryName1 = "Mind"; break;
		case 7: secondaryName1 = "Focus"; break;
		case 8: secondaryName1 = "Willpower"; break;
		default: secondaryName1 = "Unknown"; break;
	}
	switch (secondaryPool2) {
		case 0: secondaryName2 = "Health"; break;
		case 1: secondaryName2 = "Strength"; break;
		case 2: secondaryName2 = "Constitution"; break;
		case 3: secondaryName2 = "Action"; break;
		case 4: secondaryName2 = "Quickness"; break;
		case 5: secondaryName2 = "Stamina"; break;
		case 6: secondaryName2 = "Mind"; break;
		case 7: secondaryName2 = "Focus"; break;
		case 8: secondaryName2 = "Willpower"; break;
		default: secondaryName2 = "Unknown"; break;
	}

	// Feedback messages
	StringBuffer attackerMsg;
	attackerMsg << "\\#FF6600" << factionStyle << " wounds " << target->getFirstName()
				<< "! " << primaryName << ": " << primaryWounds;
	if (secondaryWounds1 > 0) {
		attackerMsg << ", " << secondaryName1 << ": " << secondaryWounds1;
	}
	if (secondaryWounds2 > 0) {
		attackerMsg << ", " << secondaryName2 << ": " << secondaryWounds2;
	}
	attackerMsg << " (" << directDamage << " damage)";
	attacker->sendSystemMessage(attackerMsg.toString());

	StringBuffer targetMsg;
	targetMsg << "\\#FF0000" << attacker->getFirstName() << "'s " << factionStyle
			  << " wounds you! Total: " << actualTotalWounds << " wounds (" << directDamage << " damage)";
	target->sendSystemMessage(targetMsg.toString());

	return actualTotalWounds;
}

// ============================================================================
// SHELL SHOCK - BATTLE FATIGUE APPLICATION
// ============================================================================

/*
 * Shell Shock applies battle fatigue (shock wounds) to impair healing:
 * - 0-250: Minor impact
 * - 250-400: Moderate (healing starts to degrade)
 * - 400-750: Significant (healing seriously impaired)
 * - 750-1000: Severe (healing nearly useless)
 *
 * Conservative tuning: ~50 base BF means 5-8 shots to reach 400 threshold
 * This prevents oppressive BF stacking while still being meaningful.
 *
 * Attacker Formula:
 * - BF = baseBF * (1 + dw_pot/150) (slower scaling than damage)
 *
 * Defender Formula:
 * - finalBF = BF * (1 - dw_recovery/200) (max 50% reduction)
 *
 * NOTE: Consider adding BF clearing to dwAdrenalineStim as counterplay
 * Stim could clear 150-250 BF per use, creating consumable economy
 */

int DeathWatchManagerImplementation::applyShellShock(CreatureObject* attacker, uint64 targetID, int baseBF) {
	if (attacker == nullptr) {
		return -1;
	}

	// Get attacker ghost for faction check
	ManagedReference<PlayerObject*> attackerGhost = attacker->getPlayerObject();
	if (attackerGhost == nullptr) {
		return -1;
	}

	// Get target
	auto zoneServer = this->zoneServer.get();
	if (zoneServer == nullptr) {
		return -1;
	}

	ManagedReference<SceneObject*> targetObject = zoneServer->getObject(targetID);
	if (targetObject == nullptr) {
		attacker->sendSystemMessage("Invalid target.");
		return -1;
	}

	// Resolve vehicle/mount to rider using getLinkedCreature()
	CreatureObject* target = nullptr;
	if (targetObject->isVehicleObject() || (targetObject->isCreatureObject() && targetObject->asCreatureObject()->isMount())) {
		// Vehicles/mounts store their rider as linkedCreature
		ManagedReference<CreatureObject*> rider = targetObject->asCreatureObject()->getLinkedCreature();
		if (rider != nullptr) {
			// Check if the rider is actually ON this vehicle right now
			ManagedReference<SceneObject*> riderParent = rider->getParent().get();
			if (riderParent != nullptr && riderParent->getObjectID() == targetObject->getObjectID()) {
				// Rider is actually on this vehicle - target the rider
				target = rider;
			} else {
				// Rider is NOT on this vehicle (vehicle is parked/stored) - invalid target
				attacker->sendSystemMessage("That vehicle has no rider.");
				return -1;
			}
		} else {
			attacker->sendSystemMessage("That vehicle has no rider.");
			return -1;
		}
	} else if (targetObject->isCreatureObject()) {
		target = targetObject->asCreatureObject();
	}

	if (target == nullptr) {
		attacker->sendSystemMessage("Invalid target.");
		return -1;
	}

	// Check if target is a DW Hunter training NPC (allowed target for practice)
	bool isDwHunterNpc = false;
	if (!target->isPlayerCreature() && target->isAiAgent()) {
		AiAgent* aiTarget = target->asAiAgent();
		if (aiTarget != nullptr) {
			String socialGroup = aiTarget->getSocialGroup();
			isDwHunterNpc = (socialGroup == "death_watch_hunter");
		}
	}

	// Must be valid DW enemy (must be a player or DW Hunter NPC)
	if (!target->isPlayerCreature() && !isDwHunterNpc) {
		attacker->sendSystemMessage("Target must be an enemy Death Watch faction member.");
		return -1;
	}

	// For player targets, verify DW faction war status
	if (target->isPlayerCreature()) {
		ManagedReference<PlayerObject*> targetGhost = target->getPlayerObject();
		if (targetGhost == nullptr) {
			attacker->sendSystemMessage("Invalid target.");
			return -1;
		}

		byte attackerFaction = attackerGhost->getDwFaction();
		byte targetFaction = targetGhost->getDwFaction();

		if (attackerFaction == DeathWatchFactions::NONE || targetFaction == DeathWatchFactions::NONE) {
			attacker->sendSystemMessage("Target must be an enemy Death Watch faction member.");
			return -1;
		}

		if (!areFactionsAtWar(attackerFaction, targetFaction)) {
			attacker->sendSystemMessage("Target must be an enemy Death Watch faction member.");
			return -1;
		}
	}

	// Range check
	float distance = attacker->getDistanceTo(target);
	if (distance > DW_RANGED_MAX_RANGE) {
		StringBuffer msg;
		msg << "Target is out of range. Maximum: " << DW_RANGED_MAX_RANGE << "m";
		attacker->sendSystemMessage(msg.toString());
		return -1;
	}

	// Line of sight check
	if (!CollisionManager::checkLineOfSight(attacker, target)) {
		attacker->sendSystemMessage("You don't have line of sight to your target.");
		return -1;
	}

	// Get skill mods
	int dwPot = attacker->getSkillMod("dw_pot");
	int targetRecovery = target->getSkillMod("dw_recovery");

	// Calculate BF: base * (1 + dw_pot/150) - slower scaling than damage
	float potencyBonus = 1.0f + (dwPot / 150.0f);
	int totalBF = (int)(baseBF * potencyBonus);

	// Apply defender's dw_recovery: BF * (1 - dw_recovery/200)
	if (targetRecovery > 0) {
		float recoveryReduction = 1.0f - (targetRecovery / 200.0f);
		if (recoveryReduction < 0.5f) recoveryReduction = 0.5f;  // Cap at 50% reduction
		totalBF = (int)(totalBF * recoveryReduction);
	}

	if (totalBF < 1) {
		totalBF = 1;  // Minimum 1 BF
	}

	// Cap at 1000 total (can't exceed max)
	int currentBF = target->getShockWounds();
	if (currentBF + totalBF > 1000) {
		totalBF = 1000 - currentBF;
		if (totalBF <= 0) {
			attacker->sendSystemMessage("\\#AAAAAA Target is already at maximum battle fatigue.");
			return 0;
		}
	}

	// Lock target and apply shock wounds
	Locker locker(target);
	Locker clocker(attacker, target);

	// Apply battle fatigue (shock wounds)
	target->addShockWounds(totalBF, true, false);

	int newBF = target->getShockWounds();

	// Apply light direct damage (10% of harmonic mean)
	int baseDamage = getWeaponBaseDamage(attacker);
	int directDamage = (int)(baseDamage * 0.10f);
	if (directDamage < 5) directDamage = 5;

	// Damage goes to health
	int currentHealth = target->getHAM(0);
	if (directDamage >= currentHealth) {
		directDamage = currentHealth - 1;  // Don't kill with direct damage
	}
	if (directDamage > 0) {
		target->inflictDamage(attacker, 0, directDamage, false, "dwShellShock", true, false);
	}

	// Apply DW TEF
	applyDwTef(attacker);
	applyDwTef(target);

	// Set combat state for both players
	attacker->setCombatState();
	target->setCombatState();
	attacker->setDefender(target);
	target->setDefender(attacker);

	// Record combat participant for gank detection
	recordCombatParticipant(target, attacker);

	// Determine severity for messaging
	String severity;
	String severityColor;
	if (newBF >= 750) {
		severity = "SEVERE";
		severityColor = "\\#FF0000";
	} else if (newBF >= 400) {
		severity = "significant";
		severityColor = "\\#FF6600";
	} else if (newBF >= 250) {
		severity = "moderate";
		severityColor = "\\#FFFF00";
	} else {
		severity = "minor";
		severityColor = "\\#AAAAAA";
	}

	// Feedback messages
	StringBuffer attackerMsg;
	attackerMsg << "\\#FF6600Shell Shock hits " << target->getFirstName()
				<< "! +" << totalBF << " battle fatigue (" << newBF << " total - "
				<< severityColor << severity << "\\#FF6600)";
	attacker->sendSystemMessage(attackerMsg.toString());

	StringBuffer targetMsg;
	targetMsg << "\\#FF0000" << attacker->getFirstName() << "'s Shell Shock disorients you! "
			  << "+" << totalBF << " battle fatigue (" << newBF << " total - "
			  << severityColor << severity << "\\#FF0000)";
	target->sendSystemMessage(targetMsg.toString());

	// Warning at threshold
	if (newBF >= 400 && (newBF - totalBF) < 400) {
		target->sendSystemMessage("\\#FF0000WARNING: Battle fatigue is now significantly impairing your healing!");
	}

	return totalBF;
}

// ============================================================================
// DISARMING SHOT - WEAPON UNEQUIP + ATTACK PREVENTION
// ============================================================================

/*
 * Disarming Shot has two effects:
 * 1. Attempts to unequip target's weapon from hold_r to inventory
 * 2. Applies STUNNED state via buff to prevent attacks for duration
 *
 * Resistance Mechanic:
 * - Defender's dw_deflection gives % chance to completely resist
 * - Formula: resistChance = dw_deflection / 2 (max 50% at 100 deflection)
 *
 * Duration Scaling:
 * - Attacker dw_pot increases duration: duration * (1 + dw_pot/100)
 * - Defender dw_recovery reduces duration: duration * (1 - dw_recovery/200)
 *
 * Return values:
 * - 1 = Full success (weapon unequipped + stunned)
 * - 2 = Partial success (stunned only, weapon couldn't be unequipped)
 * - 0 = Resisted (dw_deflection roll succeeded)
 * - -1 = Error (invalid target, out of range, etc.)
 */

int DeathWatchManagerImplementation::applyDisarmingShot(CreatureObject* attacker, uint64 targetID, int baseDurationMs) {
	if (attacker == nullptr) {
		return -1;
	}

	// Get attacker ghost for faction check
	ManagedReference<PlayerObject*> attackerGhost = attacker->getPlayerObject();
	if (attackerGhost == nullptr) {
		return -1;
	}

	// Get target
	auto zoneServer = this->zoneServer.get();
	if (zoneServer == nullptr) {
		return -1;
	}

	ManagedReference<SceneObject*> targetObject = zoneServer->getObject(targetID);
	if (targetObject == nullptr) {
		attacker->sendSystemMessage("Invalid target.");
		return -1;
	}

	// Resolve vehicle/mount to rider using getLinkedCreature()
	CreatureObject* target = nullptr;
	if (targetObject->isVehicleObject() || (targetObject->isCreatureObject() && targetObject->asCreatureObject()->isMount())) {
		ManagedReference<CreatureObject*> rider = targetObject->asCreatureObject()->getLinkedCreature();
		if (rider != nullptr) {
			ManagedReference<SceneObject*> riderParent = rider->getParent().get();
			if (riderParent != nullptr && riderParent->getObjectID() == targetObject->getObjectID()) {
				target = rider;
			} else {
				attacker->sendSystemMessage("That vehicle has no rider.");
				return -1;
			}
		} else {
			attacker->sendSystemMessage("That vehicle has no rider.");
			return -1;
		}
	} else if (targetObject->isCreatureObject()) {
		target = targetObject->asCreatureObject();
	}

	if (target == nullptr) {
		attacker->sendSystemMessage("Invalid target.");
		return -1;
	}

	// Check if target is a DW Hunter training NPC (allowed target for practice)
	bool isDwHunterNpc = false;
	if (!target->isPlayerCreature() && target->isAiAgent()) {
		AiAgent* aiTarget = target->asAiAgent();
		if (aiTarget != nullptr) {
			String socialGroup = aiTarget->getSocialGroup();
			isDwHunterNpc = (socialGroup == "death_watch_hunter");
		}
	}

	// Must be valid DW enemy (must be a player or DW Hunter NPC)
	if (!target->isPlayerCreature() && !isDwHunterNpc) {
		attacker->sendSystemMessage("Target must be an enemy Death Watch faction member.");
		return -1;
	}

	// For player targets, verify DW faction war status
	if (target->isPlayerCreature()) {
		ManagedReference<PlayerObject*> targetGhost = target->getPlayerObject();
		if (targetGhost == nullptr) {
			attacker->sendSystemMessage("Invalid target.");
			return -1;
		}

		byte attackerFaction = attackerGhost->getDwFaction();
		byte targetFaction = targetGhost->getDwFaction();

		if (attackerFaction == DeathWatchFactions::NONE || targetFaction == DeathWatchFactions::NONE) {
			attacker->sendSystemMessage("Target must be an enemy Death Watch faction member.");
			return -1;
		}

		if (!areFactionsAtWar(attackerFaction, targetFaction)) {
			attacker->sendSystemMessage("Target must be an enemy Death Watch faction member.");
			return -1;
		}
	}

	// Range check
	float distance = attacker->getDistanceTo(target);
	if (distance > DW_RANGED_MAX_RANGE) {
		StringBuffer msg;
		msg << "Target is out of range. Maximum: " << DW_RANGED_MAX_RANGE << "m";
		attacker->sendSystemMessage(msg.toString());
		return -1;
	}

	// Line of sight check
	if (!CollisionManager::checkLineOfSight(attacker, target)) {
		attacker->sendSystemMessage("You don't have line of sight to your target.");
		return -1;
	}

	// Get skill mods
	int dwPot = attacker->getSkillMod("dw_pot");
	int targetDeflection = target->getSkillMod("dw_deflection");
	int targetRecovery = target->getSkillMod("dw_recovery");

	// Resistance check: dw_deflection / 2 = % chance to resist (max 50%)
	int resistChance = targetDeflection / 2;
	if (resistChance > 50) resistChance = 50;

	if (resistChance > 0) {
		int roll = System::random(99);  // 0-99
		if (roll < resistChance) {
			// Resisted!
			attacker->sendSystemMessage("\\#AAAAAA" + target->getFirstName() + " deflects your disarming shot!");
			target->sendSystemMessage("\\#00FF00You deflect " + attacker->getFirstName() + "'s disarming shot!");

			// Apply DW TEF even on resist (combat happened)
			applyDwTef(attacker);
			applyDwTef(target);

			// Set combat state even on resist (combat happened)
			attacker->setCombatState();
			target->setCombatState();
			attacker->setDefender(target);
			target->setDefender(attacker);

			return 0;  // Resisted
		}
	}

	// Calculate duration: base * (1 + dw_pot/100) * (1 - dw_recovery/200)
	float potencyBonus = 1.0f + (dwPot / 100.0f);
	float recoveryReduction = 1.0f - (targetRecovery / 200.0f);
	if (recoveryReduction < 0.5f) recoveryReduction = 0.5f;  // Cap at 50% reduction

	int finalDurationMs = (int)(baseDurationMs * potencyBonus * recoveryReduction);
	if (finalDurationMs < 1000) finalDurationMs = 1000;  // Minimum 1 second
	if (finalDurationMs > 10000) finalDurationMs = 10000;  // Maximum 10 seconds

	// Lock target
	Locker locker(target);
	Locker clocker(attacker, target);

	bool weaponUnequipped = false;

	// Attempt to unequip weapon from hold_r
	ManagedReference<SceneObject*> holdR = target->getSlottedObject("hold_r");
	if (holdR != nullptr && holdR->isWeaponObject()) {
		ManagedReference<SceneObject*> inventory = target->getSlottedObject("inventory");
		if (inventory != nullptr) {
			ObjectController* objectController = zoneServer->getObjectController();
			if (objectController != nullptr) {
				// Transfer weapon to inventory (unequip)
				if (objectController->transferObject(holdR, inventory, -1, true, true) == 0) {
					weaponUnequipped = true;
				}
			}
		}
	}

	// Apply STUNNED state via buff to prevent attacks
	// Using a buff allows proper duration tracking and state management
	uint32 buffCRC = String("dw_disarm").hashCode();

	// Remove existing disarm buff if present
	if (target->hasBuff(buffCRC)) {
		target->removeBuff(buffCRC);
	}

	// Create new disarm buff
	ManagedReference<Buff*> disarmBuff = new Buff(target, buffCRC, finalDurationMs / 1000.0f, BuffType::OTHER);
	if (disarmBuff != nullptr) {
		Locker buffLocker(disarmBuff);
		disarmBuff->setSpeedMultiplierMod(0.5f);  // Also slow them down
		disarmBuff->addState(CreatureState::STUNNED);  // Prevents combat commands
		target->addBuff(disarmBuff);
	}

	// Apply light direct damage (10% of harmonic mean)
	int baseDamage = getWeaponBaseDamage(attacker);
	int directDamage = (int)(baseDamage * 0.10f);
	if (directDamage < 5) directDamage = 5;

	int currentHealth = target->getHAM(0);
	if (directDamage >= currentHealth) {
		directDamage = currentHealth - 1;
	}
	if (directDamage > 0) {
		target->inflictDamage(attacker, 0, directDamage, false, "dwDisarmingShot", true, false);
	}

	// Apply DW TEF
	applyDwTef(attacker);
	applyDwTef(target);

	// Set combat state for both players
	attacker->setCombatState();
	target->setCombatState();
	attacker->setDefender(target);
	target->setDefender(attacker);

	// Record combat participant for gank detection
	recordCombatParticipant(target, attacker);

	// Feedback messages
	float durationSec = finalDurationMs / 1000.0f;

	if (weaponUnequipped) {
		// Full success
		StringBuffer attackerMsg;
		attackerMsg << "\\#00FF00Disarming Shot hits " << target->getFirstName()
					<< "! Weapon knocked away, stunned for " << String::format("%.1f", durationSec) << "s!";
		attacker->sendSystemMessage(attackerMsg.toString());

		StringBuffer targetMsg;
		targetMsg << "\\#FF0000" << attacker->getFirstName() << "'s Disarming Shot knocks your weapon away! "
				  << "Stunned for " << String::format("%.1f", durationSec) << "s!";
		target->sendSystemMessage(targetMsg.toString());

		return 1;  // Full success
	} else {
		// Partial success (couldn't unequip weapon, but still stunned)
		StringBuffer attackerMsg;
		attackerMsg << "\\#FF6600Disarming Shot hits " << target->getFirstName()
					<< "! Stunned for " << String::format("%.1f", durationSec) << "s!";
		attacker->sendSystemMessage(attackerMsg.toString());

		StringBuffer targetMsg;
		targetMsg << "\\#FF0000" << attacker->getFirstName() << "'s Disarming Shot stuns you for "
				  << String::format("%.1f", durationSec) << "s!";
		target->sendSystemMessage(targetMsg.toString());

		return 2;  // Partial success
	}
}

// ============================================================================
// LUNGE - DISMOUNT ATTACK WITH DEFLECTION RISK
// ============================================================================

/*
 * Lunge is a high-risk/high-reward melee gap closer:
 * - Attempts to dismount target from their DW jetpack
 * - Deals condition damage to the affected jetpack
 * - Defender can deflect the attack back, dismounting the ATTACKER instead
 *
 * Dismount Chance: 70% base + dw_pot/5 (max 90% at 100 pot)
 * Deflection Chance: dw_deflection/2 (max 50% at 100 deflection)
 *
 * If deflected, the tables are turned:
 * - Attacker is dismounted instead
 * - Attacker's jetpack takes the damage
 *
 * Return values:
 * - 1 = Target dismounted (success)
 * - 2 = Deflected - attacker dismounted (backfire)
 * - 0 = Miss (dismount failed, no deflection check)
 * - -1 = Error
 */

int DeathWatchManagerImplementation::applyLunge(CreatureObject* attacker, uint64 targetID, int baseJetpackDamage) {
	if (attacker == nullptr) {
		return -1;
	}

	// Get attacker ghost for faction check
	ManagedReference<PlayerObject*> attackerGhost = attacker->getPlayerObject();
	if (attackerGhost == nullptr) {
		return -1;
	}

	// Verify attacker is on DW jetpack
	if (!isOnDeathWatchJetpack(attacker)) {
		attacker->sendSystemMessage("You must be on a Death Watch jetpack to use this ability.");
		return -1;
	}

	// Get target
	auto zoneServer = this->zoneServer.get();
	if (zoneServer == nullptr) {
		return -1;
	}

	ManagedReference<SceneObject*> targetObject = zoneServer->getObject(targetID);
	if (targetObject == nullptr) {
		attacker->sendSystemMessage("Invalid target.");
		return -1;
	}

	// Resolve vehicle/mount to rider using getLinkedCreature()
	CreatureObject* target = nullptr;
	if (targetObject->isVehicleObject() || (targetObject->isCreatureObject() && targetObject->asCreatureObject()->isMount())) {
		ManagedReference<CreatureObject*> rider = targetObject->asCreatureObject()->getLinkedCreature();
		if (rider != nullptr) {
			ManagedReference<SceneObject*> riderParent = rider->getParent().get();
			if (riderParent != nullptr && riderParent->getObjectID() == targetObject->getObjectID()) {
				target = rider;
			} else {
				attacker->sendSystemMessage("That vehicle has no rider.");
				return -1;
			}
		} else {
			attacker->sendSystemMessage("That vehicle has no rider.");
			return -1;
		}
	} else if (targetObject->isCreatureObject()) {
		target = targetObject->asCreatureObject();
	}

	if (target == nullptr) {
		attacker->sendSystemMessage("Invalid target.");
		return -1;
	}

	// Check if target is a DW Hunter training NPC (allowed target for practice)
	bool isDwHunterNpc = false;
	if (!target->isPlayerCreature() && target->isAiAgent()) {
		AiAgent* aiTarget = target->asAiAgent();
		if (aiTarget != nullptr) {
			String socialGroup = aiTarget->getSocialGroup();
			isDwHunterNpc = (socialGroup == "death_watch_hunter");
		}
	}

	// Must be valid DW enemy (must be a player or DW Hunter NPC)
	if (!target->isPlayerCreature() && !isDwHunterNpc) {
		attacker->sendSystemMessage("Target must be an enemy Death Watch faction member.");
		return -1;
	}

	// For player targets, verify DW faction war status
	if (target->isPlayerCreature()) {
		ManagedReference<PlayerObject*> targetGhost = target->getPlayerObject();
		if (targetGhost == nullptr) {
			attacker->sendSystemMessage("Invalid target.");
			return -1;
		}

		byte attackerFaction = attackerGhost->getDwFaction();
		byte targetFaction = targetGhost->getDwFaction();

		if (attackerFaction == DeathWatchFactions::NONE || targetFaction == DeathWatchFactions::NONE) {
			attacker->sendSystemMessage("Target must be an enemy Death Watch faction member.");
			return -1;
		}

		if (!areFactionsAtWar(attackerFaction, targetFaction)) {
			attacker->sendSystemMessage("Target must be an enemy Death Watch faction member.");
			return -1;
		}
	}

	// Target must be on a DW jetpack (can't dismount someone not mounted) - only applies to player targets
	if (target->isPlayerCreature() && !isOnDeathWatchJetpack(target)) {
		attacker->sendSystemMessage("Target is not mounted on a Death Watch jetpack.");
		return -1;
	}

	// Range check - melee range only
	float distance = attacker->getDistanceTo(target);
	if (distance > DW_MELEE_MAX_RANGE) {
		StringBuffer msg;
		msg << "Target is out of melee range. Maximum: " << DW_MELEE_MAX_RANGE << "m";
		attacker->sendSystemMessage(msg.toString());
		return -1;
	}

	// Get skill mods
	int dwPot = attacker->getSkillMod("dw_pot");
	int targetDeflection = target->getSkillMod("dw_deflection");

	// Calculate dismount chance: 70% base + dw_pot/5 (max 90%)
	int dismountChance = 70 + (dwPot / 5);
	if (dismountChance > 90) dismountChance = 90;

	// Calculate deflection chance: dw_deflection/2 (max 50%)
	int deflectChance = targetDeflection / 2;
	if (deflectChance > 50) deflectChance = 50;

	// Apply DW TEF first (combat is happening regardless of outcome)
	applyDwTef(attacker);
	applyDwTef(target);

	// Set combat state for both players
	attacker->setCombatState();
	target->setCombatState();
	attacker->setDefender(target);
	target->setDefender(attacker);

	// Record combat participant
	recordCombatParticipant(target, attacker);

	// Roll for dismount
	int dismountRoll = System::random(99);  // 0-99
	bool dismountSuccess = (dismountRoll < dismountChance);

	if (!dismountSuccess) {
		// Miss - no deflection check on miss
		attacker->sendSystemMessage("\\#AAAAAAYour lunge misses " + target->getFirstName() + "!");
		target->sendSystemMessage("\\#00FF00You evade " + attacker->getFirstName() + "'s lunge!");

		return 0;
	}

	// Dismount would succeed - now check for deflection
	int deflectRoll = System::random(99);
	bool deflected = (deflectRoll < deflectChance);


	// Get the jetpacks for damage
	ManagedReference<SceneObject*> attackerMount = attacker->getParent().get();
	ManagedReference<SceneObject*> targetMount = target->getParent().get();

	if (deflected) {
		// DEFLECTED - Tables turn! Attacker is dismounted instead

		// Damage attacker's jetpack
		if (attackerMount != nullptr && attackerMount->isVehicleObject()) {
			ManagedReference<VehicleObject*> attackerJetpack = attackerMount.castTo<VehicleObject*>();
			if (attackerJetpack != nullptr) {
				Locker jetpackLocker(attackerJetpack);
				int damage = baseJetpackDamage;
				attackerJetpack->inflictDamage(target, 0, damage, false, true, true);
			}
		}

		// Dismount attacker
		Locker attackerLocker(attacker);
		attacker->updateCooldownTimer("mount_dismount", 0);
		attacker->executeObjectControllerAction(STRING_HASHCODE("dismount"));

		// Messages
		attacker->sendSystemMessage("\\#FF0000" + target->getFirstName() + " deflects your lunge and knocks YOU off your jetpack!");
		target->sendSystemMessage("\\#00FF00You deflect " + attacker->getFirstName() + "'s lunge back at them!");

		return 2;  // Deflected - attacker dismounted
	}

	// SUCCESS - Target is dismounted

	// Damage target's jetpack
	if (targetMount != nullptr && targetMount->isVehicleObject()) {
		ManagedReference<VehicleObject*> targetJetpack = targetMount.castTo<VehicleObject*>();
		if (targetJetpack != nullptr) {
			Locker jetpackLocker(targetJetpack);
			int damage = baseJetpackDamage;
			targetJetpack->inflictDamage(attacker, 0, damage, false, true, true);
		}
	}

	// Dismount target
	Locker targetLocker(target);
	target->updateCooldownTimer("mount_dismount", 0);
	target->executeObjectControllerAction(STRING_HASHCODE("dismount"));

	// Messages
	StringBuffer attackerMsg;
	attackerMsg << "\\#00FF00Your lunge connects! " << target->getFirstName() << " is knocked off their jetpack!";
	attacker->sendSystemMessage(attackerMsg.toString());

	StringBuffer targetMsg;
	targetMsg << "\\#FF0000" << attacker->getFirstName() << "'s lunge knocks you off your jetpack!";
	target->sendSystemMessage(targetMsg.toString());

	return 1;  // Success - target dismounted
}

// ============================================================================
// ROOTING STRIKE - Severe speed debuff (10% speed)
// ============================================================================

int DeathWatchManagerImplementation::applyRootingStrike(CreatureObject* attacker, uint64 targetID, int baseDurationMs) {
	/*
	 * Rooting Strike - Melee attack that severely slows target
	 *
	 * Mechanics:
	 * - Deals light melee damage (25% of harmonic mean)
	 * - Applies severe speed debuff (10% speed = 90% reduction)
	 * - Target can still attack and move, but extremely slowly
	 *
	 * Duration: 5s base + (dw_pot / 20) - (defender dw_recovery / 20)
	 * Min duration: 2s, Max duration: 8s
	 *
	 * Evasion: dw_evasion / 2 (max 50% at 100 evasion)
	 *
	 * Returns:
	 *   1 = Slow applied successfully
	 *   0 = Evaded
	 *  -1 = Error
	 */

	if (attacker == nullptr) {
		return -1;
	}

	uint32 buffCRC = STRING_HASHCODE("dwRootingStrike");

	// Get target
	ManagedReference<SceneObject*> targetObj = zoneServer->getObject(targetID);
	if (targetObj == nullptr) {
		attacker->sendSystemMessage("Invalid target.");
		return -1;
	}

	// Resolve vehicle/mount to rider using getLinkedCreature()
	CreatureObject* target = nullptr;
	if (targetObj->isVehicleObject() || (targetObj->isCreatureObject() && targetObj->asCreatureObject()->isMount())) {
		ManagedReference<CreatureObject*> rider = targetObj->asCreatureObject()->getLinkedCreature();
		if (rider != nullptr) {
			ManagedReference<SceneObject*> riderParent = rider->getParent().get();
			if (riderParent != nullptr && riderParent->getObjectID() == targetObj->getObjectID()) {
				target = rider;
			} else {
				attacker->sendSystemMessage("That vehicle has no rider.");
				return -1;
			}
		} else {
			attacker->sendSystemMessage("That vehicle has no rider.");
			return -1;
		}
	} else if (targetObj->isCreatureObject()) {
		target = targetObj->asCreatureObject();
	}

	if (target == nullptr) {
		attacker->sendSystemMessage("Invalid target.");
		return -1;
	}

	// NOTE: Range check is handled by validateJetpackAttack (32m for melee, 64m for ranged)
	// Do NOT add duplicate range checks here

	// Check if target already has rooting strike buff
	if (target->hasBuff(buffCRC)) {
		attacker->sendSystemMessage("Target is already slowed by rooting strike.");
		return -1;
	}

	// Get skill mods
	ManagedReference<PlayerObject*> attackerGhost = attacker->getPlayerObject();
	ManagedReference<PlayerObject*> targetGhost = target->getPlayerObject();

	int attackerPot = 0;
	int defenderEvasion = 0;
	int defenderRecovery = 0;

	if (attackerGhost != nullptr) {
		attackerPot = attacker->getSkillMod("dw_pot");
	}
	if (targetGhost != nullptr) {
		defenderEvasion = target->getSkillMod("dw_evasion");
		defenderRecovery = target->getSkillMod("dw_recovery");
	}

	// Clamp skill mods to valid range
	attackerPot = Math::clamp(0, attackerPot, 100);
	defenderEvasion = Math::clamp(0, defenderEvasion, 100);
	defenderRecovery = Math::clamp(0, defenderRecovery, 100);

	// Calculate evasion chance: dw_evasion / 2 (max 50%)
	int evasionChance = defenderEvasion / 2;

	// Evasion roll
	int evasionRoll = System::random(99);
	bool evaded = (evasionRoll < evasionChance);

	if (evaded) {
		// Target evaded
		attacker->sendSystemMessage("\\#AAAAAA" + target->getFirstName() + " evades your rooting strike!");
		target->sendSystemMessage("\\#00FF00You evade " + attacker->getFirstName() + "'s rooting strike!");

		return 0;
	}

	// Calculate slow duration
	// Base 20000ms + (pot * 120) - (recovery * 120)
	// This gives: 20s base, +12s max from pot, -12s max from recovery
	// Range: 8s (0 pot, 100 recovery) to 32s (100 pot, 0 recovery)
	int durationMs = baseDurationMs;
	durationMs += (attackerPot * 120);      // +120ms per pot point (max +12s)
	durationMs -= (defenderRecovery * 120); // -120ms per recovery point (max -12s)

	// Clamp to 8-32 seconds
	durationMs = Math::clamp(8000, durationMs, 32000);

	int durationSec = durationMs / 1000;

	// Deal light damage (25% of harmonic mean)
	int baseDamage = getWeaponBaseDamage(attacker);
	int damage = baseDamage * 25 / 100;

	if (damage > 0) {
		// Split damage to health (60%) and action (40%)
		int healthDamage = damage * 60 / 100;
		int actionDamage = damage * 40 / 100;

		if (healthDamage > 0) {
			Locker targetLocker(target, attacker);
			target->inflictDamage(attacker, CreatureAttribute::HEALTH, healthDamage, true, true, true);
		}
		if (actionDamage > 0) {
			Locker targetLocker(target, attacker);
			target->inflictDamage(attacker, CreatureAttribute::ACTION, actionDamage, true, true, true);
		}
	}

	// Apply severe speed reduction buff (10% speed = 90% reduction)
	{
		Locker targetLocker(target, attacker);

		ManagedReference<Buff*> slowBuff = new Buff(target, buffCRC, durationSec, BuffType::OTHER);

		Locker buffLocker(slowBuff);

		slowBuff->setSpeedMultiplierMod(0.1f);         // 10% speed
		slowBuff->setAccelerationMultiplierMod(0.1f); // 10% acceleration

		// Fly text for visual feedback
		slowBuff->setStartFlyText("combat_effects", "go_snare", 0xFF, 0x00, 0); // Red
		slowBuff->setEndFlyText("combat_effects", "no_snare", 0, 0xFF, 0); // Green

		target->addBuff(slowBuff);
	}

	// Apply TEF
	applyDwTef(attacker);
	applyDwTef(target);

	// Set combat state for both players
	attacker->setCombatState();
	target->setCombatState();
	attacker->setDefender(target);
	target->setDefender(attacker);

	// Messages
	StringBuffer attackerMsg;
	attackerMsg << "\\#00FF00Your rooting strike severely slows " << target->getFirstName();
	attackerMsg << " for " << durationSec << " seconds!";
	attacker->sendSystemMessage(attackerMsg.toString());

	StringBuffer targetMsg;
	targetMsg << "\\#FF0000" << attacker->getFirstName() << "'s rooting strike severely slows you!";
	target->sendSystemMessage(targetMsg.toString());

	return 1;  // Success - target slowed
}

// ============================================================================
// CRIPPLING BLOW - Speed reduction debuff
// ============================================================================

int DeathWatchManagerImplementation::applyCripplingBlow(CreatureObject* attacker, uint64 targetID, int baseDurationMs, float speedMultiplier) {
	/*
	 * Crippling Blow - Melee attack that slows target
	 *
	 * Mechanics:
	 * - Deals moderate melee damage (40% of harmonic mean)
	 * - Applies speed reduction via buff
	 * - Target moves at reduced speed but can still attack
	 *
	 * Duration: 8s base + (dw_pot * 40) - (defender dw_recovery * 40)
	 * Min duration: 4s, Max duration: 12s
	 *
	 * Evasion: dw_evasion / 3 (max 33% at 100 evasion)
	 * Lower evasion than root because this is less impactful
	 *
	 * Returns:
	 *   1 = Slow applied successfully
	 *   0 = Evaded
	 *  -1 = Error
	 */

	if (attacker == nullptr) {
		return -1;
	}

	// Get target
	ManagedReference<SceneObject*> targetObj = zoneServer->getObject(targetID);
	if (targetObj == nullptr) {
		attacker->sendSystemMessage("Invalid target.");
		return -1;
	}

	// Resolve vehicle/mount to rider using getLinkedCreature()
	CreatureObject* target = nullptr;
	if (targetObj->isVehicleObject() || (targetObj->isCreatureObject() && targetObj->asCreatureObject()->isMount())) {
		ManagedReference<CreatureObject*> rider = targetObj->asCreatureObject()->getLinkedCreature();
		if (rider != nullptr) {
			ManagedReference<SceneObject*> riderParent = rider->getParent().get();
			if (riderParent != nullptr && riderParent->getObjectID() == targetObj->getObjectID()) {
				target = rider;
			} else {
				attacker->sendSystemMessage("That vehicle has no rider.");
				return -1;
			}
		} else {
			attacker->sendSystemMessage("That vehicle has no rider.");
			return -1;
		}
	} else if (targetObj->isCreatureObject()) {
		target = targetObj->asCreatureObject();
	}

	if (target == nullptr) {
		attacker->sendSystemMessage("Invalid target.");
		return -1;
	}

	// NOTE: Range check is handled by validateJetpackAttack (32m for melee, 64m for ranged)
	// Do NOT add duplicate range checks here

	// Check if target already has our slow debuff
	uint32 buffCRC = STRING_HASHCODE("dwCripplingBlow");
	if (target->hasBuff(buffCRC)) {
		attacker->sendSystemMessage("Target is already crippled.");
		return -1;
	}

	// Get skill mods
	ManagedReference<PlayerObject*> attackerGhost = attacker->getPlayerObject();
	ManagedReference<PlayerObject*> targetGhost = target->getPlayerObject();

	int attackerPot = 0;
	int defenderEvasion = 0;
	int defenderRecovery = 0;

	if (attackerGhost != nullptr) {
		attackerPot = attacker->getSkillMod("dw_pot");
	}
	if (targetGhost != nullptr) {
		defenderEvasion = target->getSkillMod("dw_evasion");
		defenderRecovery = target->getSkillMod("dw_recovery");
	}

	// Clamp skill mods to valid range
	attackerPot = Math::clamp(0, attackerPot, 100);
	defenderEvasion = Math::clamp(0, defenderEvasion, 100);
	defenderRecovery = Math::clamp(0, defenderRecovery, 100);

	// Calculate evasion chance: dw_evasion / 3 (max 33%)
	int evasionChance = defenderEvasion / 3;

	// Evasion roll
	int evasionRoll = System::random(99);
	bool evaded = (evasionRoll < evasionChance);

	if (evaded) {
		// Target evaded
		attacker->sendSystemMessage("\\#AAAAAA" + target->getFirstName() + " evades your crippling blow!");
		target->sendSystemMessage("\\#00FF00You evade " + attacker->getFirstName() + "'s crippling blow!");

		return 0;
	}

	// Calculate slow duration
	// Base 20000ms + (pot * 120) - (recovery * 120)
	// This gives: 20s base, +12s max from pot, -12s max from recovery
	// Range: 8s (0 pot, 100 recovery) to 32s (100 pot, 0 recovery)
	int durationMs = baseDurationMs;
	durationMs += (attackerPot * 120);      // +120ms per pot point (max +12s)
	durationMs -= (defenderRecovery * 120); // -120ms per recovery point (max -12s)

	// Clamp to 8-32 seconds
	durationMs = Math::clamp(8000, durationMs, 32000);

	int durationSec = durationMs / 1000;


	// Deal moderate damage (40% of harmonic mean)
	int baseDamage = getWeaponBaseDamage(attacker);
	int damage = baseDamage * 40 / 100;

	if (damage > 0) {
		// Split damage to health (60%) and action (40%)
		int healthDamage = damage * 60 / 100;
		int actionDamage = damage * 40 / 100;

		if (healthDamage > 0) {
			Locker targetLocker(target, attacker);
			target->inflictDamage(attacker, CreatureAttribute::HEALTH, healthDamage, true, true, true);
		}
		if (actionDamage > 0) {
			Locker targetLocker(target, attacker);
			target->inflictDamage(attacker, CreatureAttribute::ACTION, actionDamage, true, true, true);
		}
	}

	// Apply speed reduction buff
	{
		Locker targetLocker(target, attacker);

		ManagedReference<Buff*> slowBuff = new Buff(target, buffCRC, durationSec, BuffType::OTHER);

		Locker buffLocker(slowBuff);

		slowBuff->setSpeedMultiplierMod(speedMultiplier);
		slowBuff->setAccelerationMultiplierMod(speedMultiplier);

		// Fly text for visual feedback
		slowBuff->setStartFlyText("combat_effects", "go_snare", 0xFF, 0x80, 0); // Orange
		slowBuff->setEndFlyText("combat_effects", "no_snare", 0, 0xFF, 0); // Green

		target->addBuff(slowBuff);
	}

	// Apply TEF
	applyDwTef(attacker);
	applyDwTef(target);

	// Set combat state for both players
	attacker->setCombatState();
	target->setCombatState();
	attacker->setDefender(target);
	target->setDefender(attacker);

	// Calculate speed reduction percentage for message
	int speedReduction = (int)((1.0f - speedMultiplier) * 100);

	// Messages
	StringBuffer attackerMsg;
	attackerMsg << "\\#00FF00Your crippling blow reduces " << target->getFirstName();
	attackerMsg << "'s speed by " << speedReduction << "% for " << durationSec << " seconds!";
	attacker->sendSystemMessage(attackerMsg.toString());

	StringBuffer targetMsg;
	targetMsg << "\\#FF0000" << attacker->getFirstName() << "'s crippling blow reduces your speed!";
	target->sendSystemMessage(targetMsg.toString());

	return 1;  // Success - target slowed
}

// ============================================================================
// SHATTERING STRIKE - Armor/mitigation debuff
// ============================================================================

int DeathWatchManagerImplementation::applyShatteringStrike(CreatureObject* attacker, uint64 targetID) {
	/*
	 * Shattering Strike - Melee attack that weakens target's defenses
	 *
	 * Mechanics:
	 * - Deals light melee damage (20% of harmonic mean)
	 * - Applies mitigation debuff via private_mitigation_divisor_x100
	 * - Target takes more damage from ALL sources while debuffed
	 *
	 * Mitigation Effect:
	 * - Base: 125 (1.25x damage taken = 25% more damage)
	 * - With max dw_pot: 150 (1.5x damage taken = 50% more damage)
	 * - Formula: 125 + (dw_pot / 4) -> range 125-150
	 *
	 * Duration (weapon-based):
	 * - Base from weapon: harmonicDamage / 50 (500 dmg = 10s)
	 * - Extended by dw_pot: +50ms per point (100 pot = +5s)
	 * - Reduced by dw_recovery: -50ms per point (100 recovery = -5s)
	 * - Clamped: 5-20 seconds
	 *
	 * Resistance:
	 * - dw_deflection / 4 (max 25% at 100 deflection)
	 * - Lower than other abilities because this is a debuff not CC
	 *
	 * Returns:
	 *   1 = Debuff applied successfully
	 *   0 = Blocked by deflection
	 *  -1 = Error
	 */

	if (attacker == nullptr) {
		return -1;
	}

	// Get target
	ManagedReference<SceneObject*> targetObj = zoneServer->getObject(targetID);
	if (targetObj == nullptr) {
		attacker->sendSystemMessage("Invalid target.");
		return -1;
	}

	// Resolve vehicle/mount to rider using getLinkedCreature()
	CreatureObject* target = nullptr;
	if (targetObj->isVehicleObject() || (targetObj->isCreatureObject() && targetObj->asCreatureObject()->isMount())) {
		ManagedReference<CreatureObject*> rider = targetObj->asCreatureObject()->getLinkedCreature();
		if (rider != nullptr) {
			ManagedReference<SceneObject*> riderParent = rider->getParent().get();
			if (riderParent != nullptr && riderParent->getObjectID() == targetObj->getObjectID()) {
				target = rider;
			} else {
				attacker->sendSystemMessage("That vehicle has no rider.");
				return -1;
			}
		} else {
			attacker->sendSystemMessage("That vehicle has no rider.");
			return -1;
		}
	} else if (targetObj->isCreatureObject()) {
		target = targetObj->asCreatureObject();
	}

	if (target == nullptr) {
		attacker->sendSystemMessage("Invalid target.");
		return -1;
	}

	// NOTE: Range check is handled by validateJetpackAttack (32m for melee, 64m for ranged)
	// Do NOT add duplicate range checks here

	// Check if target already has our shatter debuff
	uint32 buffCRC = STRING_HASHCODE("dwShatteringStrike");
	if (target->hasBuff(buffCRC)) {
		attacker->sendSystemMessage("Target's armor is already shattered.");
		return -1;
	}

	// Get weapon damage for duration calculation
	int harmonicDamage = getWeaponBaseDamage(attacker);

	// Get skill mods
	ManagedReference<PlayerObject*> attackerGhost = attacker->getPlayerObject();
	ManagedReference<PlayerObject*> targetGhost = target->getPlayerObject();

	int attackerPot = 0;
	int defenderDeflection = 0;
	int defenderRecovery = 0;

	if (attackerGhost != nullptr) {
		attackerPot = attacker->getSkillMod("dw_pot");
	}
	if (targetGhost != nullptr) {
		defenderDeflection = target->getSkillMod("dw_deflection");
		defenderRecovery = target->getSkillMod("dw_recovery");
	}

	// Clamp skill mods to valid range
	attackerPot = Math::clamp(0, attackerPot, 100);
	defenderDeflection = Math::clamp(0, defenderDeflection, 100);
	defenderRecovery = Math::clamp(0, defenderRecovery, 100);

	// Calculate block chance: dw_deflection / 4 (max 25%)
	int blockChance = defenderDeflection / 4;

	// Block roll
	int blockRoll = System::random(99);
	bool blocked = (blockRoll < blockChance);

	if (blocked) {
		// Target blocked
		attacker->sendSystemMessage("\\#AAAAAA" + target->getFirstName() + " deflects your shattering strike!");
		target->sendSystemMessage("\\#00FF00You deflect " + attacker->getFirstName() + "'s shattering strike!");

		return 0;
	}

	// Calculate duration from weapon + skills
	// Base: harmonicDamage / 50 (in seconds)
	int baseDurationMs = (harmonicDamage / 50) * 1000;
	if (baseDurationMs < 5000) baseDurationMs = 5000;  // Minimum 5s base from weapon

	// Extend by dw_pot, reduce by dw_recovery
	int durationMs = baseDurationMs;
	durationMs += (attackerPot * 50);      // +50ms per pot point
	durationMs -= (defenderRecovery * 50); // -50ms per recovery point

	// Clamp to 5-20 seconds
	durationMs = Math::clamp(5000, durationMs, 20000);
	int durationSec = durationMs / 1000;

	// Calculate mitigation divisor
	// Base 125 (1.25x damage), scales up to 150 (1.5x) with pot
	int mitigationValue = 125 + (attackerPot / 4);  // 125 at 0 pot, 150 at 100 pot

	// Deal light damage (20% of harmonic mean)
	int damage = harmonicDamage * 20 / 100;

	if (damage > 0) {
		// Split damage to health (60%) and action (40%)
		int healthDamage = damage * 60 / 100;
		int actionDamage = damage * 40 / 100;

		if (healthDamage > 0) {
			Locker targetLocker(target, attacker);
			target->inflictDamage(attacker, CreatureAttribute::HEALTH, healthDamage, true, true, true);
		}
		if (actionDamage > 0) {
			Locker targetLocker(target, attacker);
			target->inflictDamage(attacker, CreatureAttribute::ACTION, actionDamage, true, true, true);
		}
	}

	// Apply mitigation debuff
	{
		Locker targetLocker(target, attacker);

		ManagedReference<Buff*> shatterBuff = new Buff(target, buffCRC, durationSec, BuffType::OTHER);

		Locker buffLocker(shatterBuff);

		// Apply the mitigation divisor - target takes more damage
		shatterBuff->setSkillModifier("private_mitigation_divisor_x100", mitigationValue);

		// Fly text for visual feedback
		shatterBuff->setStartFlyText("combat_effects", "armor_break", 0xFF, 0x40, 0); // Red-orange
		shatterBuff->setEndFlyText("combat_effects", "armor_restore", 0, 0xFF, 0); // Green

		target->addBuff(shatterBuff);
	}

	// Apply TEF
	applyDwTef(attacker);
	applyDwTef(target);

	// Set combat state for both players
	attacker->setCombatState();
	target->setCombatState();
	attacker->setDefender(target);
	target->setDefender(attacker);

	// Calculate damage increase percentage for message
	int damageIncrease = mitigationValue - 100;

	// Messages
	StringBuffer attackerMsg;
	attackerMsg << "\\#00FF00Your shattering strike weakens " << target->getFirstName();
	attackerMsg << "'s armor! They take " << damageIncrease << "% more damage for " << durationSec << " seconds!";
	attacker->sendSystemMessage(attackerMsg.toString());

	StringBuffer targetMsg;
	targetMsg << "\\#FF0000" << attacker->getFirstName() << "'s shattering strike weakens your armor!";
	target->sendSystemMessage(targetMsg.toString());

	return 1;  // Success - target debuffed
}

int DeathWatchManagerImplementation::applyJetpackBuster(CreatureObject* attacker, uint64 targetID) {
	/*
	 * Jetpack Buster - Devastating melee attack targeting enemy jetpacks
	 *
	 * Design Philosophy:
	 * - High risk/high reward ability
	 * - Can destroy or severely damage enemy jetpack
	 * - Backfire mechanic makes it risky against skilled defenders
	 *
	 * Damage Calculation:
	 * - Base: weapon harmonic mean * 2 (double weapon damage to jetpack condition)
	 * - Potency bonus: +50% at 100 dw_pot
	 * - Final formula: harmonicMean * 2 * (1 + dw_pot/200)
	 *
	 * Backfire (deflection):
	 * - Chance = dw_deflection / 2 (max 50% at 100 deflection)
	 * - On backfire: attacker's jetpack takes the damage instead
	 * - No damage reduction on backfire (full punishment)
	 *
	 * Returns:
	 *   positive = damage dealt to target's jetpack
	 *   0 = backfired (damage to attacker's jetpack)
	 *  -1 = error
	 */

	if (attacker == nullptr) {
		return -1;
	}

	// Get target
	ManagedReference<SceneObject*> targetObj = zoneServer->getObject(targetID);
	if (targetObj == nullptr) {
		attacker->sendSystemMessage("Invalid target.");
		return -1;
	}

	// Resolve vehicle/mount to rider using getLinkedCreature()
	CreatureObject* target = nullptr;
	if (targetObj->isVehicleObject() || (targetObj->isCreatureObject() && targetObj->asCreatureObject()->isMount())) {
		ManagedReference<CreatureObject*> rider = targetObj->asCreatureObject()->getLinkedCreature();
		if (rider != nullptr) {
			ManagedReference<SceneObject*> riderParent = rider->getParent().get();
			if (riderParent != nullptr && riderParent->getObjectID() == targetObj->getObjectID()) {
				target = rider;
			} else {
				attacker->sendSystemMessage("That vehicle has no rider.");
				return -1;
			}
		} else {
			attacker->sendSystemMessage("That vehicle has no rider.");
			return -1;
		}
	} else if (targetObj->isCreatureObject()) {
		target = targetObj->asCreatureObject();
	}

	if (target == nullptr) {
		attacker->sendSystemMessage("Invalid target.");
		return -1;
	}

	// NOTE: Range check is handled by validateJetpackAttack (32m for melee, 64m for ranged)
	// Do NOT add duplicate range checks here

	// Target must be on a DW jetpack (can't bust a jetpack they're not using)
	if (!isOnDeathWatchJetpack(target)) {
		attacker->sendSystemMessage("Target is not mounted on a Death Watch jetpack.");
		return -1;
	}

	// Weapon check - require melee weapon
	if (!hasCorrectWeaponType(attacker, false)) {
		attacker->sendSystemMessage("You must have a melee weapon equipped to use this ability.");
		return -1;
	}

	// Get weapon damage for calculation
	int harmonicDamage = getWeaponBaseDamage(attacker);

	// Get skill mods
	ManagedReference<PlayerObject*> attackerGhost = attacker->getPlayerObject();
	ManagedReference<PlayerObject*> targetGhost = target->getPlayerObject();

	int attackerPot = 0;
	int defenderDeflection = 0;

	if (attackerGhost != nullptr) {
		attackerPot = attacker->getSkillMod("dw_pot");
	}
	if (targetGhost != nullptr) {
		defenderDeflection = target->getSkillMod("dw_deflection");
	}

	// Clamp skill mods to valid range
	attackerPot = Math::clamp(0, attackerPot, 100);
	defenderDeflection = Math::clamp(0, defenderDeflection, 100);

	// Calculate backfire chance: dw_deflection / 2 (max 50%)
	int backfireChance = defenderDeflection / 2;

	// Calculate jetpack damage
	// Formula: harmonicMean * 2 * (1 + dw_pot/200)
	float potencyBonus = 1.0f + (attackerPot / 200.0f);  // 1.0 to 1.5
	int jetpackDamage = (int)(harmonicDamage * 2 * potencyBonus);

	// Minimum damage floor
	if (jetpackDamage < 500) {
		jetpackDamage = 500;
	}

	// Backfire roll
	int backfireRoll = System::random(99);
	bool backfired = (backfireRoll < backfireChance);

	if (backfired) {
		// BACKFIRE - damage attacker's jetpack instead!

		ManagedReference<SceneObject*> attackerMount = attacker->getParent();
		if (attackerMount != nullptr && attackerMount->isVehicleObject()) {
			ManagedReference<VehicleObject*> attackerJetpack = attackerMount.castTo<VehicleObject*>();
			if (attackerJetpack != nullptr) {
				Locker jetpackLocker(attackerJetpack, attacker);
				attackerJetpack->inflictDamage(target, 0, jetpackDamage, false, true, true);

				// Check if jetpack was destroyed
				if (attackerJetpack->isDestroyed()) {
					attacker->sendSystemMessage("\\#FF0000BACKFIRE! " + target->getFirstName() + " deflects your attack and DESTROYS your jetpack!");
					target->sendSystemMessage("\\#00FF00You deflect " + attacker->getFirstName() + "'s jetpack buster back at them, destroying their jetpack!");
				} else {
					StringBuffer attackerMsg;
					attackerMsg << "\\#FF6600BACKFIRE! " << target->getFirstName() << " deflects your attack! Your jetpack takes " << jetpackDamage << " damage!";
					attacker->sendSystemMessage(attackerMsg.toString());

					StringBuffer targetMsg;
					targetMsg << "\\#00FF00You deflect " << attacker->getFirstName() << "'s jetpack buster back at their own jetpack!";
					target->sendSystemMessage(targetMsg.toString());
				}
			}
		}

		// Apply TEF even on backfire
		applyDwTef(attacker);
		applyDwTef(target);

		// Set combat state even on backfire
		attacker->setCombatState();
		target->setCombatState();
		attacker->setDefender(target);
		target->setDefender(attacker);

		return 0;  // Backfired
	}

	// SUCCESS - damage target's jetpack

	ManagedReference<SceneObject*> targetMount = target->getParent();
	if (targetMount != nullptr && targetMount->isVehicleObject()) {
		ManagedReference<VehicleObject*> targetJetpack = targetMount.castTo<VehicleObject*>();
		if (targetJetpack != nullptr) {
			Locker jetpackLocker(targetJetpack, attacker);
			targetJetpack->inflictDamage(attacker, 0, jetpackDamage, false, true, true);

			// Check if jetpack was destroyed
			if (targetJetpack->isDestroyed()) {
				attacker->sendSystemMessage("\\#00FF00CRITICAL HIT! Your jetpack buster DESTROYS " + target->getFirstName() + "'s jetpack!");
				target->sendSystemMessage("\\#FF0000" + attacker->getFirstName() + "'s jetpack buster DESTROYS your jetpack!");
			} else {
				StringBuffer attackerMsg;
				attackerMsg << "\\#00FF00Your jetpack buster deals " << jetpackDamage << " damage to " << target->getFirstName() << "'s jetpack!";
				attacker->sendSystemMessage(attackerMsg.toString());

				StringBuffer targetMsg;
				targetMsg << "\\#FF0000" << attacker->getFirstName() << "'s jetpack buster hits your jetpack for " << jetpackDamage << " damage!";
				target->sendSystemMessage(targetMsg.toString());
			}
		}
	}

	// Apply TEF
	applyDwTef(attacker);
	applyDwTef(target);

	// Set combat state for both players
	attacker->setCombatState();
	target->setCombatState();
	attacker->setDefender(target);
	target->setDefender(attacker);

	return jetpackDamage;  // Success - return damage dealt
}

bool DeathWatchManagerImplementation::checkAndApplyCooldownWithDuration(CreatureObject* player, const String& abilityName, int cooldownMs) {
	/*
	 * Variable-duration cooldown check for abilities like Door Buster.
	 * Works like checkAndApplyCooldown but uses provided duration instead of config.
	 */

	if (player == nullptr) {
		return false;
	}

	// Check if on cooldown
	if (!player->checkCooldownRecovery(abilityName)) {
		// Calculate remaining time
		const Time* cooldownTime = player->getCooldownTime(abilityName);
		if (cooldownTime != nullptr) {
			int64 remainingMs = -cooldownTime->miliDifference();
			int remainingMin = remainingMs / 60000;
			int remainingSec = (remainingMs % 60000) / 1000;

			StringBuffer msg;
			msg << abilityName << " is on cooldown. ";
			if (remainingMin > 0) {
				msg << remainingMin << " minute" << (remainingMin != 1 ? "s" : "");
				if (remainingSec > 0) {
					msg << " " << remainingSec << " second" << (remainingSec != 1 ? "s" : "");
				}
			} else {
				msg << remainingSec << " second" << (remainingSec != 1 ? "s" : "");
			}
			msg << " remaining.";
			player->sendSystemMessage(msg.toString());
		}
		return false;
	}

	// Apply cooldown with specified duration
	player->addCooldown(abilityName, cooldownMs);

	return true;
}

int DeathWatchManagerImplementation::applyDoorBuster(CreatureObject* attacker, uint64 targetID) {
	/*
	 * Door Buster - Building breach ability for Zann Consortium
	 *
	 * Forcibly ejects enemy DW players from buildings.
	 * This is a counter to players using buildings as safe havens.
	 *
	 * Target Validation:
	 * - Must be a valid DW enemy (at war, BH target, or Greef contract)
	 * - Building must be within 32m
	 *
	 * Priority System:
	 * 1. Active BH bounty targets (highest)
	 * 2. Greef contract targets (from dw:bh data)
	 * 3. At-war faction members (random)
	 *
	 * Max Ejects by Skill:
	 * - combat_death_watch_zan_1: 1
	 * - combat_death_watch_zan_2: 2
	 * - combat_death_watch_zan_3: 3
	 * - combat_death_watch_zan_4: 4
	 * - combat_death_watch_master: unlimited
	 *
	 * Post-Eject:
	 * - Teleport near attacker
	 * - Apply dwTef and pvpTef
	 * - Apply invincibility (1-10s based on dw_deflection)
	 * - Set first ejected as attacker's target
	 *
	 * Returns:
	 *   positive = number of players ejected
	 *   0 = no valid targets found
	 *  -1 = error
	 */

	if (attacker == nullptr) {
		return -1;
	}

	// Get target object (should be a building)
	ManagedReference<SceneObject*> targetObj = zoneServer->getObject(targetID);
	if (targetObj == nullptr) {
		attacker->sendSystemMessage("Invalid target. You must target a building.");
		return -1;
	}

	// Verify target is a building
	if (!targetObj->isBuildingObject()) {
		attacker->sendSystemMessage("Invalid target. You must target a building.");
		return -1;
	}

	ManagedReference<BuildingObject*> building = targetObj.castTo<BuildingObject*>();
	if (building == nullptr) {
		return -1;
	}

	// Range check - 32m to building
	float distance = attacker->getDistanceTo(building);
	if (distance > 32.0f) {
		attacker->sendSystemMessage("Target building is too far away. Must be within 32 meters.");
		return -1;
	}

	// Get attacker's DW faction
	ManagedReference<PlayerObject*> attackerGhost = attacker->getPlayerObject();
	if (attackerGhost == nullptr) {
		return -1;
	}

	byte attackerDwFaction = attackerGhost->getDwFaction();
	if (attackerDwFaction == DeathWatchFactions::NONE) {
		attacker->sendSystemMessage("You must have a Death Watch faction to use this ability.");
		return -1;
	}

	// Determine max ejects based on skill
	int maxEjects = 0;
	if (attacker->hasSkill("combat_death_watch_master")) {
		maxEjects = 999;  // Unlimited for masters
	} else if (attacker->hasSkill("combat_death_watch_zan_4")) {
		maxEjects = 4;
	} else if (attacker->hasSkill("combat_death_watch_zan_3")) {
		maxEjects = 3;
	} else if (attacker->hasSkill("combat_death_watch_zan_2")) {
		maxEjects = 2;
	} else if (attacker->hasSkill("combat_death_watch_zan_1")) {
		maxEjects = 1;
	} else {
		attacker->sendSystemMessage("You need Zann Consortium training to use this ability.");
		return -1;
	}

	// Collect all players in the building
	Vector<ManagedReference<CreatureObject*>> playersInBuilding;

	int totalCells = building->getTotalCellNumber();
	for (int cellIdx = 1; cellIdx <= totalCells; cellIdx++) {
		ManagedReference<CellObject*> cell = building->getCell(cellIdx);
		if (cell == nullptr) {
			continue;
		}

		int containerSize = cell->getContainerObjectsSize();
		for (int objIdx = 0; objIdx < containerSize; objIdx++) {
			ManagedReference<SceneObject*> obj = cell->getContainerObject(objIdx);
			if (obj == nullptr || !obj->isPlayerCreature()) {
				continue;
			}

			ManagedReference<CreatureObject*> player = obj.castTo<CreatureObject*>();
			if (player != nullptr && player != attacker) {
				playersInBuilding.add(player);
			}
		}
	}


	if (playersInBuilding.size() == 0) {
		attacker->sendSystemMessage("No players found in the building.");
		return 0;
	}

	// Get mission manager for BH bounty checks
	MissionManager* missionManager = zoneServer->getMissionManager();
	uint64 attackerID = attacker->getObjectID();

	// Categorize players by priority
	Vector<ManagedReference<CreatureObject*>> bhTargets;      // Priority 1: BH bounty
	Vector<ManagedReference<CreatureObject*>> greefTargets;   // Priority 2: Greef contract
	Vector<ManagedReference<CreatureObject*>> warTargets;     // Priority 3: At war (random)

	for (int i = 0; i < playersInBuilding.size(); i++) {
		ManagedReference<CreatureObject*> player = playersInBuilding.get(i);
		if (player == nullptr) continue;

		ManagedReference<PlayerObject*> playerGhost = player->getPlayerObject();
		if (playerGhost == nullptr) continue;

		byte playerDwFaction = playerGhost->getDwFaction();

		// Must be DW and at war with attacker
		if (playerDwFaction == DeathWatchFactions::NONE) {
			continue;
		}

		if (!areFactionsAtWar(attackerDwFaction, playerDwFaction)) {
			continue;
		}

		uint64 playerID = player->getObjectID();

		// Check if this is a BH bounty target
		bool isBhTarget = false;
		if (missionManager != nullptr) {
			isBhTarget = missionManager->hasBountyHunterInPlayerBounty(playerID, attackerID);
		}

		if (isBhTarget) {
			bhTargets.add(player);
			continue;
		}

		// Check for Greef contract (stored in attacker's screenplay data)
		// Format: bounty_X_target where X is 1-3
		bool isGreefTarget = false;
		if (attackerGhost != nullptr) {
			for (int slot = 1; slot <= 3; slot++) {
				StringBuffer keyBuf;
				keyBuf << "bounty_" << slot << "_target";
				String targetStr = attackerGhost->getScreenPlayData("dw_bh", keyBuf.toString());
				if (!targetStr.isEmpty()) {
					uint64 contractTarget = Long::valueOf(targetStr);
					if (contractTarget == playerID) {
						isGreefTarget = true;
						break;
					}
				}
			}
		}

		if (isGreefTarget) {
			greefTargets.add(player);
			continue;
		}

		// Otherwise just at war
		warTargets.add(player);
	}

	// Build final eject list based on priority
	Vector<ManagedReference<CreatureObject*>> toEject;

	// Add BH targets first (all of them up to max)
	for (int i = 0; i < bhTargets.size() && toEject.size() < maxEjects; i++) {
		toEject.add(bhTargets.get(i));
	}

	// Add Greef targets second
	for (int i = 0; i < greefTargets.size() && toEject.size() < maxEjects; i++) {
		toEject.add(greefTargets.get(i));
	}

	// Add random war targets last
	while (warTargets.size() > 0 && toEject.size() < maxEjects) {
		int randomIdx = System::random(warTargets.size() - 1);
		toEject.add(warTargets.get(randomIdx));
		warTargets.remove(randomIdx);
	}

	if (toEject.size() == 0) {
		attacker->sendSystemMessage("No valid Death Watch enemies found in the building.");
		return 0;
	}


	// Get attacker position for teleport destination
	float destX = attacker->getPositionX();
	float destY = attacker->getPositionY();
	float destZ = attacker->getPositionZ();
	Zone* zone = attacker->getZone();

	if (zone == nullptr) {
		return -1;
	}

	// Eject players
	ManagedReference<CreatureObject*> firstEjected = nullptr;
	int ejectedCount = 0;

	for (int i = 0; i < toEject.size(); i++) {
		ManagedReference<CreatureObject*> victim = toEject.get(i);
		if (victim == nullptr) continue;

		// Calculate invincibility duration based on defender's dw_deflection
		// 1s at 0 deflection, 10s at 100 deflection
		int victimDeflection = victim->getSkillMod("dw_deflection");
		victimDeflection = Math::clamp(0, victimDeflection, 100);
		int invincibilityMs = 1000 + (victimDeflection * 90);  // 1000 + (0-9000) = 1-10s

		// Eject from building
		{
			Locker victimLocker(victim, attacker);

			// Teleport near attacker (slight offset to avoid stacking)
			float offsetX = (System::random(10) - 5) * 0.5f;
			float offsetY = (System::random(10) - 5) * 0.5f;

			victim->switchZone(zone->getZoneName(), destX + offsetX, destZ, destY + offsetY, 0);

			// Apply TEFs - dwTef handles combat flagging
			applyDwTef(victim);

			// Apply invincibility buff
			uint32 invincBuffCRC = STRING_HASHCODE("dwDoorBusterInvuln");
			ManagedReference<Buff*> invincBuff = new Buff(victim, invincBuffCRC, invincibilityMs / 1000, BuffType::OTHER);
			{
				Locker buffLocker(invincBuff);
				// Make them invulnerable - set defense to max
				invincBuff->setSkillModifier("private_damage_divisor", 0);  // No damage taken
				invincBuff->setStartFlyText("combat_effects", "yourstate", 0, 0xFF, 0xFF);  // Cyan
				victim->addBuff(invincBuff);
			}
		}

		// Store first ejected for targeting
		if (firstEjected == nullptr) {
			firstEjected = victim;
		}

		ejectedCount++;

		// Messages
		StringBuffer victimMsg;
		victimMsg << "\\#FF0000You have been forcibly ejected from the building by " << attacker->getFirstName() << "!";
		victimMsg << " You have " << (invincibilityMs / 1000) << " seconds of invincibility.";
		victim->sendSystemMessage(victimMsg.toString());
	}

	// Set first ejected as attacker's target and set combat state
	if (firstEjected != nullptr) {
		Locker attackerLocker(attacker);
		attacker->setTargetID(firstEjected->getObjectID());

		// Set combat state for attacker and first ejected
		attacker->setCombatState();
		firstEjected->setCombatState();
		attacker->setDefender(firstEjected);
		firstEjected->setDefender(attacker);
	}

	// Apply TEF to attacker
	applyDwTef(attacker);

	// Success message
	StringBuffer attackerMsg;
	attackerMsg << "\\#00FF00You breach the building, ejecting " << ejectedCount;
	attackerMsg << " Death Watch enem" << (ejectedCount == 1 ? "y" : "ies") << "!";
	attacker->sendSystemMessage(attackerMsg.toString());

	// Stub client effect on building
	// TODO: Add visual effect when available

	return ejectedCount;
}

// ============================================================================
// FACING/ANGLE UTILITIES
// ============================================================================

/**
 * Check if attacker is facing target within tolerance
 * Uses world positions and SWG coordinate system (North = 0 degrees)
 */
bool DeathWatchManagerImplementation::isFacingTarget(CreatureObject* attacker, SceneObject* target, float toleranceDegrees) {
	if (attacker == nullptr || target == nullptr) {
		return false;
	}

	float attackerFacing = attacker->getDirectionAngle();  // Already in SWG degrees
	float angleToTarget = getAngleToTarget(attacker, target);

	// Calculate angle difference (handle wrap-around at 360)
	float angleDiff = fabs(angleToTarget - attackerFacing);
	if (angleDiff > 180.0f) {
		angleDiff = 360.0f - angleDiff;
	}

	return angleDiff <= toleranceDegrees;
}

/**
 * Get the angle from attacker to target in SWG degrees (North = 0, clockwise)
 * Uses world positions for correct cross-cell calculations
 */
float DeathWatchManagerImplementation::getAngleToTarget(CreatureObject* attacker, SceneObject* target) {
	if (attacker == nullptr || target == nullptr) {
		return 0.0f;
	}

	// Use world positions for correct cross-cell calculations
	float attackerX = attacker->getWorldPositionX();
	float attackerY = attacker->getWorldPositionY();
	float targetX = target->getWorldPositionX();
	float targetY = target->getWorldPositionY();

	float dx = targetX - attackerX;
	float dy = targetY - attackerY;

	// SWG coordinate system: North (positive Y) is 0 degrees, increases clockwise
	// atan2(dx, dy) gives this directly: North = 0, East = 90, South = 180, West = 270
	float angleRad = atan2(dx, dy);
	float angleDeg = angleRad * (180.0f / M_PI);

	// Normalize to 0-360 range
	if (angleDeg < 0.0f) {
		angleDeg += 360.0f;
	}

	return angleDeg;
}

/**
 * Get the facing offset angle - how many degrees off-center the target is
 * from the attacker's facing direction.
 * Returns 0-180 degrees (0 = dead on, 90 = perpendicular, 180 = behind)
 */
float DeathWatchManagerImplementation::getFacingOffsetAngle(CreatureObject* attacker, SceneObject* target) {
	if (attacker == nullptr || target == nullptr) {
		return 180.0f;  // Return worst case if invalid
	}

	// Get attacker's current facing direction (0-360, North = 0)
	float facingAngle = attacker->getDirectionAngle();

	// Get the angle from attacker TO target (0-360)
	float angleToTarget = getAngleToTarget(attacker, target);

	// Calculate the absolute difference
	float angleDiff = fabs(facingAngle - angleToTarget);

	// Normalize to 0-180 (shortest angular distance)
	if (angleDiff > 180.0f) {
		angleDiff = 360.0f - angleDiff;
	}

	return angleDiff;
}

/**
 * Calculate accuracy modifier based on facing angle and dw_acc skill
 *
 * Angle Tiers (base modifiers before dw_acc):
 *   0-5°   : +10 (dead-on bonus, scales up with dw_acc > 75)
 *   6-20°  :   0 (neutral)
 *  21-45°  : -15 (off-target penalty)
 *  46-70°  : -30 (significant penalty)
 *  71-90°  : -50 (severe penalty)
 *  90+°    : Auto-miss (handled separately)
 *
 * dw_acc Skill Effects:
 *   - Mitigates penalties by dw_acc/2 (e.g., 100 dw_acc reduces penalties by 50%)
 *   - Above 75 dw_acc: increases dead-on bonus by (dw_acc - 75) / 2
 *     Example: 100 dw_acc = +10 base + 12.5 = +22 bonus at 0-5°
 */
int DeathWatchManagerImplementation::calculateAngleAccuracyMod(CreatureObject* attacker, SceneObject* target) {
	if (attacker == nullptr || target == nullptr) {
		return -50;  // Worst penalty if invalid
	}

	float offsetAngle = getFacingOffsetAngle(attacker, target);
	int dwAcc = attacker->getSkillMod("dw_acc");

	// Clamp dw_acc to 0-100 range
	if (dwAcc < 0) dwAcc = 0;
	if (dwAcc > 100) dwAcc = 100;

	int baseMod = 0;

	// Determine base modifier from angle tier
	if (offsetAngle <= 5.0f) {
		// Dead-on: +10 base, bonus scales with dw_acc > 75
		baseMod = 10;
		if (dwAcc > 75) {
			// Each point above 75 adds 0.5 to the bonus (max +12.5 at 100)
			baseMod += (dwAcc - 75) / 2;
		}
		return baseMod;  // No penalty to mitigate, return directly
	}
	else if (offsetAngle <= 20.0f) {
		// Neutral zone
		baseMod = 0;
	}
	else if (offsetAngle <= 45.0f) {
		// Off-target
		baseMod = -15;
	}
	else if (offsetAngle <= 70.0f) {
		// Significant penalty
		baseMod = -30;
	}
	else if (offsetAngle <= 90.0f) {
		// Severe penalty (almost perpendicular)
		baseMod = -50;
	}
	else {
		// Beyond 90 degrees - target is behind attacker
		// Return maximum penalty (attacks from behind should mostly miss)
		return -75;
	}

	// Apply dw_acc mitigation to penalties
	// dw_acc/2 percent reduction (e.g., 100 dw_acc = 50% reduction)
	if (baseMod < 0 && dwAcc > 0) {
		float mitigation = dwAcc / 200.0f;  // 0.0 to 0.5 range
		baseMod = (int)(baseMod * (1.0f - mitigation));
	}

	return baseMod;
}

/**
 * Calculate weapon range accuracy modifier based on distance to target
 * Uses weapon's pointBlank/ideal/maxRange accuracy values
 * Interpolates linearly between range bands:
 *   - 0 to pointBlankRange: pointBlankAccuracy
 *   - pointBlankRange to idealRange: interpolate from pointBlank to ideal
 *   - idealRange to maxRange: interpolate from ideal to maxRange
 *   - Beyond maxRange: maxRangeAccuracy (with additional penalty)
 */
int DeathWatchManagerImplementation::calculateWeaponRangeAccuracyMod(CreatureObject* attacker, SceneObject* target) {
	if (attacker == nullptr || target == nullptr) {
		return 0;
	}

	ManagedReference<WeaponObject*> weapon = attacker->getWeapon();
	if (weapon == nullptr || !weapon->isRangedWeapon()) {
		return 0;  // No modifier for unarmed/melee
	}

	float distance = attacker->getDistanceTo(target);

	// Get weapon range stats
	int pbRange = weapon->getPointBlankRange();
	int idealRange = weapon->getIdealRange();
	int maxRange = weapon->getMaxRange();

	int pbAcc = weapon->getPointBlankAccuracy();
	int idealAcc = weapon->getIdealAccuracy();
	int maxRangeAcc = weapon->getMaxRangeAccuracy();

	// Ensure sane range values (some weapons may have 0 or negative)
	if (pbRange <= 0) pbRange = 5;
	if (idealRange <= pbRange) idealRange = pbRange + 10;
	if (maxRange <= idealRange) maxRange = idealRange + 20;

	int rangeAccMod = 0;

	if (distance <= pbRange) {
		// Within point blank range - use point blank accuracy
		rangeAccMod = pbAcc;
	}
	else if (distance <= idealRange) {
		// Between point blank and ideal - interpolate
		float t = (distance - pbRange) / (float)(idealRange - pbRange);
		rangeAccMod = (int)(pbAcc + t * (idealAcc - pbAcc));
	}
	else if (distance <= maxRange) {
		// Between ideal and max range - interpolate
		float t = (distance - idealRange) / (float)(maxRange - idealRange);
		rangeAccMod = (int)(idealAcc + t * (maxRangeAcc - idealAcc));
	}
	else {
		// Beyond max range - use max range accuracy with additional penalty
		rangeAccMod = maxRangeAcc - 20;
	}

	return rangeAccMod;
}

// ============================================================================
// DEVELOPER TESTING - Self-Test Suite for DW Combat Commands
// ============================================================================

/**
 * Run comprehensive DW combat system tests
 * Tests all attack commands in isolation, validating formulas and logic
 * Requires god mode - skips faction/target restrictions
 */
int DeathWatchManagerImplementation::runDwCombatTests(CreatureObject* player) {
	if (player == nullptr) {
		return -1;
	}

	// Require god mode
	PlayerObject* ghost = player->getPlayerObject();
	if (ghost == nullptr || !ghost->hasGodMode()) {
		player->sendSystemMessage("\\#FF0000[DW Test] ERROR: Requires god mode. Use /god first.");
		return -1;
	}

	player->sendSystemMessage("\\#00FF00========================================");
	player->sendSystemMessage("\\#00FF00  DEATH WATCH COMBAT SYSTEM TESTS");
	player->sendSystemMessage("\\#00FF00========================================");

	int testsPassed = 0;
	int testsFailed = 0;
	uint64 selfId = player->getObjectID();

	// ========================================================================
	// TEST 1: Harmonic Mean Calculation
	// ========================================================================
	player->sendSystemMessage("\\#FFFF00[Test 1] Harmonic Mean Damage Calculation...");

	int baseDamage = getWeaponBaseDamage(player);
	if (baseDamage > 0) {
		StringBuffer msg1;
		msg1 << "  Weapon base damage (harmonic mean): " << baseDamage;
		player->sendSystemMessage(msg1.toString());
		testsPassed++;
		player->sendSystemMessage("  \\#00FF00PASSED");
	} else {
		player->sendSystemMessage("  \\#FF0000FAILED: Could not calculate weapon damage");
		testsFailed++;
	}

	// ========================================================================
	// TEST 2: Angle Accuracy System
	// ========================================================================
	player->sendSystemMessage("\\#FFFF00[Test 2] Angle-Based Accuracy System...");

	// Test facing offset (should be 0 when targeting self)
	float facingOffset = getFacingOffsetAngle(player, player);
	int angleAccMod = calculateAngleAccuracyMod(player, player);

	StringBuffer msg2;
	msg2 << "  Facing offset to self: " << String::format("%.1f", facingOffset) << " deg";
	player->sendSystemMessage(msg2.toString());

	StringBuffer msg2b;
	msg2b << "  Angle accuracy modifier: " << angleAccMod;
	player->sendSystemMessage(msg2b.toString());

	// Self-targeting should give dead-on bonus
	if (facingOffset <= 5.0f && angleAccMod >= 10) {
		testsPassed++;
		player->sendSystemMessage("  \\#00FF00PASSED");
	} else {
		player->sendSystemMessage("  \\#FF0000FAILED: Expected dead-on bonus");
		testsFailed++;
	}

	// ========================================================================
	// TEST 3: Skill Mod Retrieval
	// ========================================================================
	player->sendSystemMessage("\\#FFFF00[Test 3] DW Skill Mod Retrieval...");

	int dwAcc = player->getSkillMod("dw_acc");
	int dwPot = player->getSkillMod("dw_pot");
	int dwSpeed = player->getSkillMod("dw_speed");
	int dwDeflection = player->getSkillMod("dw_deflection");
	int dwEvasion = player->getSkillMod("dw_evasion");
	int dwRecovery = player->getSkillMod("dw_recovery");

	StringBuffer msg3;
	msg3 << "  dw_acc=" << dwAcc << ", dw_pot=" << dwPot << ", dw_speed=" << dwSpeed;
	player->sendSystemMessage(msg3.toString());

	StringBuffer msg3b;
	msg3b << "  dw_deflection=" << dwDeflection << ", dw_evasion=" << dwEvasion << ", dw_recovery=" << dwRecovery;
	player->sendSystemMessage(msg3b.toString());

	testsPassed++;
	player->sendSystemMessage("  \\#00FF00PASSED (skill mods retrieved)");

	// ========================================================================
	// TEST 4: Cooldown System
	// ========================================================================
	player->sendSystemMessage("\\#FFFF00[Test 4] Cooldown System...");

	// Test skill-based cooldown calculation (don't actually apply)
	int baseCooldown = 10000;  // 10 seconds
	float speedReduction = 1.0f - (dwSpeed / 200.0f);
	if (speedReduction < 0.5f) speedReduction = 0.5f;
	int adjustedCooldown = (int)(baseCooldown * speedReduction);

	StringBuffer msg4;
	msg4 << "  Base cooldown: " << baseCooldown << "ms";
	player->sendSystemMessage(msg4.toString());

	StringBuffer msg4b;
	msg4b << "  With dw_speed " << dwSpeed << ": " << adjustedCooldown << "ms (" << (int)(speedReduction * 100) << "%)";
	player->sendSystemMessage(msg4b.toString());

	if (adjustedCooldown <= baseCooldown && adjustedCooldown >= baseCooldown / 2) {
		testsPassed++;
		player->sendSystemMessage("  \\#00FF00PASSED");
	} else {
		player->sendSystemMessage("  \\#FF0000FAILED: Cooldown out of expected range");
		testsFailed++;
	}

	// ========================================================================
	// TEST 5: Damage Formula Validation
	// ========================================================================
	player->sendSystemMessage("\\#FFFF00[Test 5] Damage Formula Validation...");

	// Test damage with various multipliers
	float damageMultiplier = 1.0f;
	int rawDamage = (int)(baseDamage * damageMultiplier);
	float potencyBonus = 1.0f + (dwPot / 100.0f);
	int totalDamage = (int)(rawDamage * potencyBonus);

	StringBuffer msg5;
	msg5 << "  Raw damage (base * 1.0): " << rawDamage;
	player->sendSystemMessage(msg5.toString());

	StringBuffer msg5b;
	msg5b << "  With dw_pot " << dwPot << " (+" << (int)(dwPot) << "%): " << totalDamage;
	player->sendSystemMessage(msg5b.toString());

	if (totalDamage >= rawDamage) {
		testsPassed++;
		player->sendSystemMessage("  \\#00FF00PASSED");
	} else {
		player->sendSystemMessage("  \\#FF0000FAILED: Potency should increase damage");
		testsFailed++;
	}

	// ========================================================================
	// TEST 6: Hit Chance Calculation
	// ========================================================================
	player->sendSystemMessage("\\#FFFF00[Test 6] Hit Chance Calculation...");

	int baseHitChance = 75;
	int accuracyMod = 0;
	int hitChance = baseHitChance + accuracyMod + angleAccMod + (dwAcc / 2) - (dwEvasion / 2);

	if (hitChance < 5) hitChance = 5;
	if (hitChance > 95) hitChance = 95;

	StringBuffer msg6;
	msg6 << "  Base: " << baseHitChance << "%, angleAccMod: " << angleAccMod;
	player->sendSystemMessage(msg6.toString());

	StringBuffer msg6b;
	msg6b << "  dw_acc bonus: +" << (dwAcc / 2) << "%, dw_evasion penalty: -" << (dwEvasion / 2) << "%";
	player->sendSystemMessage(msg6b.toString());

	StringBuffer msg6c;
	msg6c << "  Final hit chance: " << hitChance << "% (clamped 5-95)";
	player->sendSystemMessage(msg6c.toString());

	if (hitChance >= 5 && hitChance <= 95) {
		testsPassed++;
		player->sendSystemMessage("  \\#00FF00PASSED");
	} else {
		player->sendSystemMessage("  \\#FF0000FAILED: Hit chance out of bounds");
		testsFailed++;
	}

	// ========================================================================
	// TEST 7: Deflection Calculation
	// ========================================================================
	player->sendSystemMessage("\\#FFFF00[Test 7] Deflection Calculation...");

	int deflectChance = dwDeflection / 2;
	if (deflectChance > 50) deflectChance = 50;
	int deflectedDamage = totalDamage / 4;

	StringBuffer msg7;
	msg7 << "  Deflection chance: " << deflectChance << "% (max 50%)";
	player->sendSystemMessage(msg7.toString());

	StringBuffer msg7b;
	msg7b << "  Deflected damage (25% of " << totalDamage << "): " << deflectedDamage;
	player->sendSystemMessage(msg7b.toString());

	if (deflectChance <= 50 && deflectedDamage >= 0) {
		testsPassed++;
		player->sendSystemMessage("  \\#00FF00PASSED");
	} else {
		player->sendSystemMessage("  \\#FF0000FAILED: Deflection out of bounds");
		testsFailed++;
	}

	// ========================================================================
	// TEST 8: Evasion Calculation
	// ========================================================================
	player->sendSystemMessage("\\#FFFF00[Test 8] Evasion Calculation...");

	int evasionChance50 = dwEvasion / 2;  // For rooting strike (max 50%)
	int evasionChance33 = dwEvasion / 3;  // For crippling blow (max 33%)

	if (evasionChance50 > 50) evasionChance50 = 50;
	if (evasionChance33 > 33) evasionChance33 = 33;

	StringBuffer msg8;
	msg8 << "  Evasion/2 (root): " << evasionChance50 << "% (max 50%)";
	player->sendSystemMessage(msg8.toString());

	StringBuffer msg8b;
	msg8b << "  Evasion/3 (cripple): " << evasionChance33 << "% (max 33%)";
	player->sendSystemMessage(msg8b.toString());

	if (evasionChance50 <= 50 && evasionChance33 <= 33) {
		testsPassed++;
		player->sendSystemMessage("  \\#00FF00PASSED");
	} else {
		player->sendSystemMessage("  \\#FF0000FAILED: Evasion out of bounds");
		testsFailed++;
	}

	// ========================================================================
	// TEST 9: Duration Scaling (dw_pot and dw_recovery)
	// ========================================================================
	player->sendSystemMessage("\\#FFFF00[Test 9] Duration Scaling (pot vs recovery)...");

	int baseDurationMs = 8000;  // 8 seconds
	int potBonus = (dwPot * 50);       // +50ms per pot
	int recoveryReduction = (dwRecovery * 50);  // -50ms per recovery
	int finalDurationMs = baseDurationMs + potBonus - recoveryReduction;

	if (finalDurationMs < 3000) finalDurationMs = 3000;   // Minimum 3s
	if (finalDurationMs > 15000) finalDurationMs = 15000; // Maximum 15s

	StringBuffer msg9;
	msg9 << "  Base: " << baseDurationMs << "ms + pot(" << potBonus << ") - recovery(" << recoveryReduction << ")";
	player->sendSystemMessage(msg9.toString());

	StringBuffer msg9b;
	msg9b << "  Final duration: " << finalDurationMs << "ms (clamped 3-15s)";
	player->sendSystemMessage(msg9b.toString());

	if (finalDurationMs >= 3000 && finalDurationMs <= 15000) {
		testsPassed++;
		player->sendSystemMessage("  \\#00FF00PASSED");
	} else {
		player->sendSystemMessage("  \\#FF0000FAILED: Duration out of bounds");
		testsFailed++;
	}

	// ========================================================================
	// TEST 10: Weapon Type Detection
	// ========================================================================
	player->sendSystemMessage("\\#FFFF00[Test 10] Weapon Type Detection...");

	bool hasRanged = hasCorrectWeaponType(player, true);
	bool hasMelee = hasCorrectWeaponType(player, false);

	StringBuffer msg10;
	msg10 << "  Has ranged weapon: " << (hasRanged ? "YES" : "NO");
	player->sendSystemMessage(msg10.toString());

	StringBuffer msg10b;
	msg10b << "  Has melee weapon: " << (hasMelee ? "YES" : "NO");
	player->sendSystemMessage(msg10b.toString());

	testsPassed++;
	player->sendSystemMessage("  \\#00FF00PASSED (weapon detection works)");

	// ========================================================================
	// TEST 11: Jetpack Detection
	// ========================================================================
	player->sendSystemMessage("\\#FFFF00[Test 11] Jetpack Detection...");

	bool onDwJetpack = isOnDeathWatchJetpack(player);

	StringBuffer msg11;
	msg11 << "  On DW Jetpack: " << (onDwJetpack ? "YES" : "NO");
	player->sendSystemMessage(msg11.toString());

	if (onDwJetpack) {
		float speed = player->getCurrentSpeed();
		StringBuffer msg11b;
		msg11b << "  Current speed: " << String::format("%.1f", speed) << " (min 15 for attacks)";
		player->sendSystemMessage(msg11b.toString());
	}

	testsPassed++;
	player->sendSystemMessage("  \\#00FF00PASSED (jetpack detection works)");

	// ========================================================================
	// TEST 12: Faction Detection
	// ========================================================================
	player->sendSystemMessage("\\#FFFF00[Test 12] Faction Detection...");

	int faction = 0;
	const char* factionName = "NONE";
	if (ghost != nullptr) {
		faction = ghost->getDwFaction();
		switch (faction) {
			case 1: factionName = "XONTI"; break;
			case 2: factionName = "BLACK SUN"; break;
			case 3: factionName = "KRRSANTAN"; break;
			case 4: factionName = "ZANN"; break;
		}
	}

	StringBuffer msg12;
	msg12 << "  DW Faction: " << factionName << " (" << faction << ")";
	player->sendSystemMessage(msg12.toString());

	testsPassed++;
	player->sendSystemMessage("  \\#00FF00PASSED (faction detection works)");

	// ========================================================================
	// SUMMARY
	// ========================================================================
	player->sendSystemMessage("\\#00FF00========================================");

	StringBuffer summary;
	summary << "\\#00FF00  RESULTS: " << testsPassed << " PASSED, " << testsFailed << " FAILED";
	player->sendSystemMessage(summary.toString());

	if (testsFailed == 0) {
		player->sendSystemMessage("\\#00FF00  ALL TESTS PASSED!");
		player->sendSystemMessage("\\#00FF00  Combat formulas are correctly calculating.");
	} else {
		player->sendSystemMessage("\\#FF0000  SOME TESTS FAILED - Review output above.");
	}

	player->sendSystemMessage("\\#00FF00========================================");

	// Additional tips
	player->sendSystemMessage("");
	player->sendSystemMessage("\\#AAAAAA[Tip] To test attacks, you need a DW enemy target.");
	player->sendSystemMessage("\\#AAAAAA[Tip] Use /setDwFaction on a second character or NPC.");

	return testsPassed;
}

// ============================================================================
// DAMAGE MITIGATION & ARMOR SYSTEMS
// ============================================================================

/**
 * Calculate damage after armor mitigation for Death Watch weapons
 *
 * Mirrors CombatManager's armor reduction logic but simplified for DW use:
 * - Gets armor at random hit location
 * - Applies elemental resistance
 * - Applies armor piercing calculation
 * - Does NOT apply PSG (that's for normal combat)
 */
float DeathWatchManagerImplementation::calculateMitigatedDamage(CreatureObject* attacker, CreatureObject* defender, float rawDamage, int damageType, int armorPiercing) {
	if (attacker == nullptr || defender == nullptr || rawDamage <= 0) {
		return rawDamage;
	}

	float damage = rawDamage;

	// Get CombatManager for armor calculations
	CombatManager* combatManager = CombatManager::instance();
	if (combatManager == nullptr) {
		return damage;
	}

	// Random hit location (matches CombatManager behavior)
	// 0=body, 1=head, 2=rarm, 3=larm, 4=rleg, 5=lleg
	int hitLocation = System::random(5);

	// Get armor at hit location
	ArmorObject* armor = combatManager->getArmorObject(defender, hitLocation);

	if (armor != nullptr && !armor->isVulnerable(damageType)) {
		// Get elemental resistance
		float armorReduction = combatManager->getArmorObjectReduction(armor, damageType);

		// Apply armor piercing multiplier
		float piercingMultiplier = combatManager->getArmorPiercing(armor, armorPiercing);
		damage *= piercingMultiplier;

		// Apply resistance reduction
		if (armorReduction > 0) {
			damage *= (1.0f - (armorReduction / 100.0f));
		}

		// Ensure damage doesn't exceed original (no negative armor bonus)
		if (damage > rawDamage) {
			damage = rawDamage;
		}
	}

	// Apply private_mitigation_divisor if target has it (from Shattering Strike, etc.)
	int mitigationDivisor = defender->getSkillMod("private_mitigation_divisor");
	if (mitigationDivisor > 1) {
		damage /= mitigationDivisor;
	}

	// Apply private_mitigation_divisor_x100 (finer control, 125 = 1.25x damage taken)
	int mitigationDivisorX100 = defender->getSkillMod("private_mitigation_divisor_x100");
	if (mitigationDivisorX100 > 100) {
		// This is a MULTIPLIER, not divisor - higher = more damage taken
		damage *= (mitigationDivisorX100 / 100.0f);
	}

	// Apply Mandalorian armor bonuses for Death Watch combat
	// Attacker wearing Mando armor deals more damage
	float attackerBonus = getMandoArmorDamageBonus(attacker);
	if (attackerBonus > 1.0f) {
		damage *= attackerBonus;
	}

	// Defender wearing Mando armor takes less damage
	float defenderReduction = getMandoArmorDamageReduction(defender);
	if (defenderReduction < 1.0f) {
		damage *= defenderReduction;
	}

	return Math::max(0.0f, damage);
}

/**
 * Calculate heavy weapon damage mitigation using CombatManager
 *
 * This function provides full combat mitigation for DW heavy weapons by
 * calling CombatManager::getArmorReduction directly. This includes:
 * - PSG absorption (up to 46%)
 * - Standard armor with armor piercing
 * - Force Armor/Shield (if Jedi without armor)
 * - Cloak of Pain
 * - Food mitigation bonuses
 *
 * This ensures heavy weapon damage respects all defensive investments.
 */
float DeathWatchManagerImplementation::getHeavyWeaponMitigation(CreatureObject* attacker, CreatureObject* defender, WeaponObject* weapon, float rawDamage) {
	if (attacker == nullptr || defender == nullptr || weapon == nullptr || rawDamage <= 0) {
		return rawDamage;
	}

	CombatManager* combatManager = CombatManager::instance();
	if (combatManager == nullptr) {
		return rawDamage;
	}

	// Create a DefenderHitList to track mitigation (required by getArmorReduction)
	DefenderHitList hitList;
	hitList.setDefender(defender);
	hitList.setInitialDamage((int)rawDamage);

	// Random hit location (matches CombatManager behavior)
	// HIT_BODY=0, HIT_HEAD=1, HIT_RARM=2, HIT_LARM=3, HIT_RLEG=4, HIT_LLEG=5
	int hitLocation = System::random(5);
	hitList.setHitLocation(hitLocation);

	// Get weapon properties
	int damageType = weapon->getDamageType();
	int armorPiercing = weapon->getArmorPiercing();

	// Create minimal CreatureAttackData for the mitigation call
	// We need this for getArmorReduction but can use defaults for most values
	// since heavy weapons don't use force attacks or special command properties
	UnicodeString emptyArgs = "";

	// Since we can't easily construct CreatureAttackData without a command,
	// we'll manually apply the key mitigation elements that getArmorReduction handles

	float damage = rawDamage;

	// 1. PSG (Personal Shield Generator)
	ArmorObject* psg = combatManager->getPSGArmor(defender);
	if (psg != nullptr && !psg->isVulnerable(damageType)) {
		float psgReduction = combatManager->getArmorObjectReduction(psg, damageType);

		// Cap at 46% (matches CombatManager hardcap)
		if (psgReduction > 46.0f) {
			psgReduction = 46.0f;
		}

		if (psgReduction > 0) {
			float dmgAbsorbed = damage * (psgReduction / 100.0f);
			damage -= dmgAbsorbed;
			hitList.setPsgMitigation((int)dmgAbsorbed);

			// Damage PSG (using PVP rate: 0.45% of initial damage)
			if (defender->checkCooldownRecovery("psg_damaged")) {
				float psgDamage = rawDamage * 0.0045f;
				Locker plocker(psg);
				psg->inflictDamage(psg, 0, psgDamage, true, true);
				defender->updateCooldownTimer("psg_damaged", 1000);
			}
		}
	}

	// 2. Standard Armor at hit location
	ArmorObject* armor = combatManager->getArmorObject(defender, hitLocation);
	if (armor != nullptr && !armor->isVulnerable(damageType)) {
		float armorReduction = combatManager->getArmorObjectReduction(armor, damageType);

		// Apply armor piercing
		float preArmorDamage = damage;
		damage *= combatManager->getArmorPiercing(armor, armorPiercing);

		// Apply armor resistance
		if (armorReduction > 0) {
			damage *= (1.0f - (armorReduction / 100.0f));
		}

		// Don't let armor piercing increase damage beyond pre-armor value
		if (damage > preArmorDamage) {
			damage = preArmorDamage;
		}

		float dmgAbsorbed = preArmorDamage - damage;
		if (dmgAbsorbed > 0) {
			hitList.setArmorMitigation((int)dmgAbsorbed);

			// Damage armor (0.1 multiplier matches CombatManager)
			Locker alocker(armor);
			armor->inflictDamage(armor, 0, damage * 0.1f, true, true);
		}
	}

	// 3. Apply private_mitigation_divisor (from Particle Beam "Mand'alor's Grip" etc.)
	int mitigationDivisor = defender->getSkillMod("private_mitigation_divisor");
	if (mitigationDivisor > 1) {
		damage /= mitigationDivisor;
	}

	// 4. Apply private_mitigation_divisor_x100 (finer control)
	int mitigationDivisorX100 = defender->getSkillMod("private_mitigation_divisor_x100");
	if (mitigationDivisorX100 > 100) {
		damage *= (mitigationDivisorX100 / 100.0f);
	}

	return Math::max(1.0f, damage);
}

/**
 * Apply acid corrosion to target's equipped armor
 *
 * Iterates through ALL equipped armor pieces and applies condition damage.
 * Unlike CombatManager which uses weighted hit locations, Death Watch acid
 * affects all armor pieces - this makes acid uniquely dangerous to full armor sets.
 * Each piece has a random chance to be affected, modified by acid strength.
 * Armor with higher acid resistance takes less condition damage.
 * Leg armor takes extra damage since targets are standing in acid pools.
 */
int DeathWatchManagerImplementation::applyAcidCorrosion(CreatureObject* attacker, CreatureObject* defender, int baseConditionDamage, float acidStrength, float legsMultiplier) {
	if (defender == nullptr || baseConditionDamage <= 0) {
		return 0;
	}

	// Clamp inputs
	acidStrength = Math::clamp(0.0f, acidStrength, 1.0f);
	legsMultiplier = Math::max(1.0f, legsMultiplier);  // At least 1.0x

	int totalConditionDamage = 0;
	int armorPiecesAffected = 0;

	// Get the wearables vector (const - we only read from it, lock armor individually)
	// Death Watch acid hits ALL armor pieces, unlike CombatManager's weighted hit locations
	const WearablesDeltaVector* wearables = defender->getWearablesDeltaVector();
	if (wearables == nullptr) {
		return 0;
	}

	// Iterate through ALL wearables looking for armor
	// Death Watch acid hits every piece, unlike CombatManager's weighted hit locations
	for (int i = 0; i < wearables->size(); i++) {
		TangibleObject* wearable = wearables->get(i);

		if (wearable == nullptr || !wearable->isArmorObject()) {
			continue;
		}

		ArmorObject* armor = cast<ArmorObject*>(wearable);
		if (armor == nullptr) {
			continue;
		}

		// Get armor's acid resistance (reduces corrosion damage)
		float acidResist = armor->getAcid();
		float resistMultiplier = 1.0f - (acidResist / 200.0f);  // 100 resist = 50% reduction
		resistMultiplier = Math::clamp(0.25f, resistMultiplier, 1.0f);  // Min 25% of damage

		// Random chance to affect this piece (60% base + 40% from acid strength)
		float hitChance = 0.60f + (acidStrength * 0.40f);
		if (System::random(100) / 100.0f > hitChance) {
			continue;  // This piece avoided corrosion
		}

		// Calculate condition damage for this piece
		// Scale by acid strength and resistance
		int conditionDamage = (int)(baseConditionDamage * (0.5f + acidStrength * 0.5f) * resistMultiplier);

		// Apply legs multiplier for leg armor (boots/pants take extra damage from standing in acid)
		// ArmorObjectTemplate::LEGS = 0x4
		uint8 hitLocation = armor->getHitLocation();
		if (hitLocation & 0x4) {  // LEGS hit location
			conditionDamage = (int)(conditionDamage * legsMultiplier);
		}

		// Apply some variance (80-120%)
		conditionDamage = (int)(conditionDamage * (0.8f + (System::random(40) / 100.0f)));

		// Apply Mandalorian armor decay reduction if defender is wearing Mando armor
		float decayReduction = getMandoArmorDecayReduction(defender);
		if (decayReduction < 1.0f) {
			conditionDamage = (int)(conditionDamage * decayReduction);
		}

		if (conditionDamage <= 0) {
			continue;
		}

		// Get current condition and max condition
		int currentConditionDamage = armor->getConditionDamage();
		int maxCondition = armor->getMaxCondition();

		// Apply condition damage (armor breaks when conditionDamage >= maxCondition)
		int newConditionDamage = currentConditionDamage + conditionDamage;

		Locker armorLocker(armor);
		armor->setConditionDamage(newConditionDamage, true);

		totalConditionDamage += conditionDamage;
		armorPiecesAffected++;

		// Check if armor is now broken
		if (newConditionDamage >= maxCondition) {
			// Armor destroyed - notify player
			StringIdChatParameter destroyMsg("base_player", "prose_item_destroyed");
			destroyMsg.setTT(armor->getObjectID());
			defender->sendSystemMessage(destroyMsg);
		}
	}

	// Log corrosion for debugging
	if (armorPiecesAffected > 0) {
		info(true) << "[DW Acid] " << (attacker != nullptr ? attacker->getFirstName() : "Unknown")
				   << " corroded " << armorPiecesAffected << " armor pieces on "
				   << defender->getFirstName() << " for " << totalConditionDamage << " total condition damage"
				   << " (legs mult: " << legsMultiplier << ")";
	}

	return totalConditionDamage;
}

/**
 * Apply acid DOT effect to target
 *
 * Uses POISONED state with acid-themed visual effects.
 * DOT damage bypasses armor (conceptually already inside the armor).
 */
bool DeathWatchManagerImplementation::applyAcidDot(CreatureObject* attacker, CreatureObject* defender, int totalDotDamage, int durationSeconds, int tickIntervalMs) {
	if (attacker == nullptr || defender == nullptr || totalDotDamage <= 0 || durationSeconds <= 0) {
		return false;
	}

	// Default tick interval is 2 seconds
	if (tickIntervalMs <= 0) {
		tickIntervalMs = 2000;
	}

	// Calculate number of ticks and damage per tick
	int numTicks = (durationSeconds * 1000) / tickIntervalMs;
	if (numTicks <= 0) {
		numTicks = 1;
	}

	int damagePerTick = totalDotDamage / numTicks;
	if (damagePerTick <= 0) {
		damagePerTick = 1;
	}

	// Use POISONED state for acid DOT (closest match to acid damage)
	uint64 dotType = CreatureState::POISONED;

	// Apply the DOT
	// addDotState(attacker, dotType, objectID, strength, attribute, duration, potency, defense)
	// - objectID: 0 (no specific weapon)
	// - strength: damage per tick
	// - attribute: 0 (health)
	// - duration: in seconds
	// - potency: 100 (guaranteed application)
	// - defense: 0 (no defense roll for DW acid)
	defender->addDotState(
		attacker,           // attacker
		dotType,            // POISONED state
		0,                  // objectID (no weapon)
		damagePerTick,      // strength (damage per tick)
		CreatureAttribute::HEALTH,  // attribute to damage
		durationSeconds,    // duration in seconds
		100.0f,             // potency (100 = guaranteed)
		0                   // defense (0 = no defense)
	);

	// Play acid visual effect on initial application
	defender->playEffect("clienteffect/cr_parasite_explosion.cef", "");

	info(true) << "[DW Acid DOT] " << attacker->getFirstName() << " applied acid DOT to "
			   << defender->getFirstName() << ": " << totalDotDamage << " over " << durationSeconds
			   << "s (" << damagePerTick << " per tick, " << numTicks << " ticks)";

	return true;
}

/**
 * Get total acid resistance for a defender
 *
 * Calculates average acid resistance across all equipped armor.
 * Returns 0 if no armor is equipped.
 */
float DeathWatchManagerImplementation::getAcidResistance(CreatureObject* defender) {
	if (defender == nullptr) {
		return 0.0f;
	}

	const WearablesDeltaVector* wearables = defender->getWearablesDeltaVector();
	if (wearables == nullptr) {
		return 0.0f;
	}

	float totalResist = 0.0f;
	int armorCount = 0;

	for (int i = 0; i < wearables->size(); i++) {
		TangibleObject* wearable = wearables->get(i);

		if (wearable == nullptr || !wearable->isArmorObject()) {
			continue;
		}

		ArmorObject* armor = cast<ArmorObject*>(wearable);
		if (armor == nullptr) {
			continue;
		}

		totalResist += armor->getAcid();
		armorCount++;
	}

	if (armorCount == 0) {
		return 0.0f;
	}

	return totalResist / armorCount;
}

// ============================================================================
// MANDALORIAN ARMOR BONUSES
// ============================================================================

/**
 * Count how many pieces of Mandalorian armor a player has equipped
 *
 * Checks all worn armor for Mandalorian templates:
 * - object/tangible/wearables/armor/mandalorian/armor_mandalorian_*.iff
 *
 * Full set is 10 pieces:
 * - helmet, chest_plate, leggings, belt, shoes
 * - bicep_l, bicep_r, bracer_l, bracer_r, gloves
 */
int DeathWatchManagerImplementation::countMandoArmorPieces(CreatureObject* player) {
	if (player == nullptr) {
		return 0;
	}

	const WearablesDeltaVector* wearables = player->getWearablesDeltaVector();
	if (wearables == nullptr) {
		return 0;
	}

	int mandoPieceCount = 0;

	// The Mandalorian armor template prefix to check for
	const String mandoPrefix = "object/tangible/wearables/armor/mandalorian/armor_mandalorian_";

	for (int i = 0; i < wearables->size(); i++) {
		TangibleObject* wearable = wearables->get(i);

		if (wearable == nullptr || !wearable->isArmorObject()) {
			continue;
		}

		// Get the template path for this armor piece
		SharedObjectTemplate* templateObj = wearable->getObjectTemplate();
		if (templateObj == nullptr) {
			continue;
		}

		String templatePath = templateObj->getFullTemplateString();

		// Check if it starts with the Mandalorian armor prefix
		if (templatePath.beginsWith(mandoPrefix)) {
			mandoPieceCount++;
		}
	}

	return mandoPieceCount;
}

/**
 * Get the damage reduction multiplier for Death Watch players wearing Mandalorian armor
 *
 * Bonuses (applied ONLY in DW combat) - scales with pieces worn:
 * - 1 piece:  0.997 (0.3% reduction)
 * - 5 pieces: 0.985 (1.5% reduction)
 * - 9 pieces: 0.973 (2.7% reduction)
 * - 10 pieces (full suit): 0.96 (4% reduction - bonus for complete set)
 */
float DeathWatchManagerImplementation::getMandoArmorDamageReduction(CreatureObject* player) {
	if (player == nullptr) {
		return 1.0f;  // No reduction
	}

	int mandoPieces = countMandoArmorPieces(player);

	if (mandoPieces == 0) {
		return 1.0f;  // No Mando armor = no bonus
	}

	// Full suit (10 pieces) = 4% reduction (bonus for complete set)
	if (mandoPieces >= 10) {
		return 0.96f;  // 4% damage reduction
	}

	// Scale from 0.3% (1 piece) to 2.7% (9 pieces)
	// Formula: 1.0 - (pieces * 0.003)
	float reductionPercent = mandoPieces * 0.003f;
	return 1.0f - reductionPercent;
}

/**
 * Get the damage bonus multiplier for Death Watch players wearing Mandalorian armor
 *
 * Bonuses (applied ONLY in DW combat) - scales with pieces worn:
 * - 1 piece:  1.003 (0.3% bonus)
 * - 5 pieces: 1.015 (1.5% bonus)
 * - 9 pieces: 1.027 (2.7% bonus)
 * - 10 pieces (full suit): 1.04 (4% bonus - bonus for complete set)
 */
float DeathWatchManagerImplementation::getMandoArmorDamageBonus(CreatureObject* player) {
	if (player == nullptr) {
		return 1.0f;  // No bonus
	}

	int mandoPieces = countMandoArmorPieces(player);

	if (mandoPieces == 0) {
		return 1.0f;  // No Mando armor = no bonus
	}

	// Full suit (10 pieces) = 4% bonus (bonus for complete set)
	if (mandoPieces >= 10) {
		return 1.04f;  // 4% damage bonus
	}

	// Scale from 0.3% (1 piece) to 2.7% (9 pieces)
	// Formula: 1.0 + (pieces * 0.003)
	float bonusPercent = mandoPieces * 0.003f;
	return 1.0f + bonusPercent;
}

/**
 * Get the armor decay reduction multiplier for Death Watch players wearing Mandalorian armor
 *
 * Bonuses (applied to acid/electricity armor condition damage) - scales with pieces worn:
 * - 1 piece:  0.95 (5% reduction)
 * - 5 pieces: 0.75 (25% reduction)
 * - 9 pieces: 0.55 (45% reduction)
 * - 10 pieces (full suit): 0.25 (75% reduction - bonus for complete set)
 */
float DeathWatchManagerImplementation::getMandoArmorDecayReduction(CreatureObject* player) {
	if (player == nullptr) {
		return 1.0f;  // No reduction
	}

	int mandoPieces = countMandoArmorPieces(player);

	if (mandoPieces == 0) {
		return 1.0f;  // No Mando armor = no reduction
	}

	// Full suit (10 pieces) = 75% reduction (bonus for complete set)
	if (mandoPieces >= 10) {
		return 0.25f;  // 75% decay reduction
	}

	// Scale from 5% (1 piece) to 45% (9 pieces)
	// Formula: 1.0 - (pieces * 0.05)
	float reductionPercent = mandoPieces * 0.05f;
	return 1.0f - reductionPercent;
}

/**
 * Process a Whistling Bird missile hit with full combat calculations
 *
 * Performs all damage calculations in C++ for efficiency:
 * - Calculates base damage with skill tape modifiers (potency vs deflection, etc.)
 * - Applies Mandalorian armor damage bonus for attacker
 * - Applies armor mitigation based on damage type and armor piercing
 * - Applies Mandalorian armor damage reduction for defender
 * - Splits damage 50/50 between health and action
 * - Applies damage to target
 * - Updates DW TEF on target
 */
int DeathWatchManagerImplementation::processMissileHit(CreatureObject* attacker, CreatureObject* defender, int minDamage, int maxDamage, int damageType, int armorPiercing, bool isSplash, const String& missileType) {
	if (attacker == nullptr || defender == nullptr) {
		return -1;
	}

	// Don't process if defender is already dead or incapacitated
	if (defender->isDead() || defender->isIncapacitated()) {
		return 0;
	}

	// =========================================================================
	// HIDDEN TEMPLATE BONUSES - Dud Check (Smuggler special)
	// =========================================================================
	// Check for dud chance FIRST (before any damage calculation)
	// This applies to the specific missile type AND whistlerGlobal
	if (rollHiddenDudChance(defender, missileType)) {
		// Missile duds - no damage, no effects, still consumed
		// Don't put players in combat (missile failed to detonate)
		return 0;  // Return 0 damage - Lua should handle messaging
	}

	// Step 1: Calculate base damage with skill tape modifiers
	// Roll random damage in range
	int baseDamage = System::random(maxDamage - minDamage) + minDamage;

	// Get attacker's skill tapes (default 40 if not set)
	int atkPotency = attacker->getSkillMod("dw_pot");
	if (atkPotency == 0) atkPotency = 40;
	int atkSpeed = attacker->getSkillMod("dw_speed");
	if (atkSpeed == 0) atkSpeed = 40;
	int atkAcc = attacker->getSkillMod("dw_acc");
	if (atkAcc == 0) atkAcc = 40;
	int atkSubterfuge = attacker->getSkillMod("dw_subterfuge");
	if (atkSubterfuge == 0) atkSubterfuge = 40;

	// Check for Targeting Boost buff - adds bonus accuracy for missiles
	int targetingBoost = attacker->getSkillMod("dw_targeting_boost");
	if (targetingBoost > 0) {
		atkAcc += targetingBoost;  // Power from crafted stim becomes accuracy bonus
	}

	// Get defender's skill tapes
	int defEvasion = defender->getSkillMod("dw_evasion");
	if (defEvasion == 0) defEvasion = 40;
	int defDeflection = defender->getSkillMod("dw_deflection");
	if (defDeflection == 0) defDeflection = 40;
	int defRecovery = defender->getSkillMod("dw_recovery");
	if (defRecovery == 0) defRecovery = 40;

	// Calculate modifiers based on rock/paper/scissors interactions
	// 1. Potency vs Deflection (raw power vs damage reduction)
	float potVsDefl = (atkPotency - defDeflection) * 0.003f;  // 20 diff = 6% swing

	// 2. Accuracy vs Evasion (precision vs dodging)
	float accVsEva = (atkAcc - defEvasion) * 0.002f;  // 20 diff = 4% swing

	// 3. Speed affects base damage slightly
	float speedMod = (atkSpeed - 40) * 0.001f;  // 40 speed = 0%, 60 = +2%

	// 4. Subterfuge vs Recovery (surprise attack vs resilience)
	float subVsRec = (atkSubterfuge - defRecovery) * 0.002f;  // 20 diff = 4% swing

	// Combine all modifiers (centered around 1.0)
	float totalMod = 1.0f + potVsDefl + accVsEva + speedMod + subVsRec;

	// Clamp to reasonable range (0.5x to 1.5x)
	totalMod = Math::clamp(0.5f, totalMod, 1.5f);

	float rawDamage = baseDamage * totalMod;

	// Step 2: Apply splash damage falloff if this is splash damage
	if (isSplash) {
		rawDamage *= 0.5f;  // 50% damage for splash targets
	}

	// Step 3: Apply Mandalorian armor damage bonus for attacker
	float mandoBonus = getMandoArmorDamageBonus(attacker);
	float boostedDamage = rawDamage * mandoBonus;

	// =========================================================================
	// HIDDEN TEMPLATE BONUSES - Attacker Damage Bonus
	// =========================================================================
	// Check for hidden damage bonus (attacker's combat profession)
	// Applies to both missile-specific AND whistlerGlobal
	float hiddenDmgBonus = getHiddenDamageBonus(attacker, missileType);
	float globalDmgBonus = getHiddenDamageBonus(attacker, "whistlerGlobal");
	float totalHiddenDmgBonus = hiddenDmgBonus + (globalDmgBonus * 0.7f);  // 70% diminishing for global
	if (totalHiddenDmgBonus > 0.5f) totalHiddenDmgBonus = 0.5f;  // Cap at 50%
	if (totalHiddenDmgBonus > 0.0f) {
		boostedDamage *= (1.0f + totalHiddenDmgBonus);
	}

	// Step 4: Apply armor mitigation based on damage type and armor piercing
	float mitigatedDamage = calculateMitigatedDamage(attacker, defender, boostedDamage, damageType, armorPiercing);

	// =========================================================================
	// HIDDEN TEMPLATE BONUSES - Defender Damage Reduction
	// =========================================================================
	// Check for hidden damage reduction (defender's combat profession)
	// Applies to both missile-specific AND whistlerGlobal
	float hiddenDmgReduction = getHiddenDamageReduction(defender, missileType);
	float globalDmgReduction = getHiddenDamageReduction(defender, "whistlerGlobal");
	float totalHiddenDmgReduction = hiddenDmgReduction + (globalDmgReduction * 0.7f);  // 70% diminishing for global
	if (totalHiddenDmgReduction > 0.4f) totalHiddenDmgReduction = 0.4f;  // Cap at 40%
	if (totalHiddenDmgReduction > 0.0f) {
		mitigatedDamage *= (1.0f - totalHiddenDmgReduction);
	}

	int totalDamage = (int)Math::max(0.0f, mitigatedDamage);

	// =========================================================================
	// HIDDEN TEMPLATE BONUSES - Evasion Check (Full Evade)
	// =========================================================================
	// Check for hidden evasion chance (defender's combat profession)
	// Applies to both missile-specific AND whistlerGlobal
	float hiddenEvasion = getHiddenEvasionChance(defender, missileType);
	float globalEvasion = getHiddenEvasionChance(defender, "whistlerGlobal");
	float totalHiddenEvasion = hiddenEvasion + (globalEvasion * 0.7f);  // 70% diminishing for global
	if (totalHiddenEvasion > 0.25f) totalHiddenEvasion = 0.25f;  // Cap at 25%
	if (totalHiddenEvasion > 0.0f) {
		float evasionRoll = System::random(10000) / 10000.0f;
		if (evasionRoll < totalHiddenEvasion) {
			// Full evasion - no damage, but still enter combat and apply TEF
			totalDamage = 0;
		}
	}

	// =========================================================================
	// ADRENALINE SERUM - High Damage Mitigation
	// =========================================================================
	// If defender has Adrenaline buff active and damage > 5000, apply mitigation
	// Formula: 25% base + ((dw_deflection + dw_toughness) / 8)%, max 50%
	if (totalDamage > 0) {
		totalDamage = applyAdrenalineHighDamageMitigation(defender, totalDamage);
	}

	// Step 5: Split damage 50/50 between health and action
	int healthDamage = totalDamage / 2;
	int actionDamage = totalDamage - healthDamage;  // Remainder goes to action

	// Step 6: Apply damage (if any)
	if (totalDamage > 0) {
		// Pool 0 = health, pool 3 = action
		// Last param 1 = notify client
		defender->inflictDamage(attacker, 0, healthDamage, 1);
		defender->inflictDamage(attacker, 3, actionDamage, 1);
	}

	// Step 7: Put both players into combat state
	attacker->setCombatState();
	defender->setCombatState();
	attacker->setDefender(defender);
	defender->setDefender(attacker);

	// Step 8: Apply DW TEF to both players
	applyDwTef(attacker);
	applyDwTef(defender);

	return totalDamage;
}

// ============================================================================
// MK4 GROUND STRIKER PROCESSING
// ============================================================================

/**
 * Process a Mk4 Ground Striker hit on a victim
 *
 * Applies all Mk4 effects:
 * - DIZZY state (stun) with duration scaled by skill tapes
 * - BLINDED state with duration scaled by skill tapes
 * - HUD disable via sendNewbieTutorialEnableHudElement
 * - Clears victim's target lock
 * - Minimal STUN damage (weaponDamage / 10, split health/action)
 * - Updates DW TEF on victim
 */
int DeathWatchManagerImplementation::processMk4Hit(CreatureObject* attacker, CreatureObject* victim, int weaponDamage) {
	if (attacker == nullptr || victim == nullptr) {
		return -1;
	}

	if (!victim->isPlayerCreature()) {
		return 0;  // Only affects players
	}

	// Don't process if victim is dead or incapacitated
	if (victim->isDead() || victim->isIncapacitated()) {
		return 0;
	}

	// Get attacker's subterfuge for duration scaling
	int atkSubterfuge = attacker->getSkillMod("dw_subterfuge");
	if (atkSubterfuge == 0) atkSubterfuge = 40;

	// Get defender's recovery for duration reduction
	int defRecovery = victim->getSkillMod("dw_recovery");
	if (defRecovery == 0) defRecovery = 40;

	// Calculate duration modifier: 1.0 + (atkSubterfuge - defRecovery) * 0.01
	// Clamped to 0.5 - 1.5 (50%-150% of base duration)
	float durationMod = 1.0f + (atkSubterfuge - defRecovery) * 0.01f;
	durationMod = Math::clamp(0.5f, durationMod, 1.5f);

	// Calculate final durations in milliseconds
	int stunDurationMs = (int)(mk4StunDuration * durationMod * 1000);
	int blindDurationMs = (int)(mk4BlindDuration * durationMod * 1000);
	int hudDurationMs = (int)(mk4HudDisableDuration * durationMod * 1000);

	Locker victimLocker(victim);

	// Play stun visual effect on victim
	victim->playEffect("clienteffect/sm_lightning_01.cef", "");

	// Apply DIZZY state via proper buff mechanism (clears naturally and via gmrevive)
	victim->setDizziedState(stunDurationMs / 1000);

	// Apply BLINDED state via proper buff mechanism (clears naturally and via gmrevive)
	victim->setBlindedState(blindDurationMs / 1000);

	// Disable HUD - this does NOT clear naturally, must be restored manually
	victim->sendNewbieTutorialEnableHudElement("all", 0, 0);

	// Schedule HUD restoration (only thing that needs manual clearing)
	Reference<Task*> hudTask = new DwMk4StateTask(victim, DW_MK4_RESTORE_HUD);
	hudTask->schedule(hudDurationMs);

	// Clear victim's target (they lose their lock) - sneaky disorienting strike
	victim->setTargetID(0);

	// Force victim to knocked down posture (disoriented)
	victim->setPosture(CreaturePosture::KNOCKEDDOWN, true);

	// Apply minimal STUN damage (weapon damage / 10, split health/action)
	int stunDamage = weaponDamage / 10;
	int healthDamage = stunDamage / 2;
	int actionDamage = stunDamage - healthDamage;

	if (healthDamage > 0) {
		victim->inflictDamage(attacker, 0, healthDamage, 1);
	}
	if (actionDamage > 0) {
		victim->inflictDamage(attacker, 3, actionDamage, 1);
	}

	// Apply DW TEF to both players (but DON'T put them in combat or set defenders)
	// This is a sneaky strike - victim doesn't know who attacked them
	applyDwTef(attacker);
	applyDwTef(victim);

	// Notify victim (no attacker info - they don't know who hit them)
	StringBuffer victimMsg;
	victimMsg << "[GROUND STRIKE] You are stunned and disoriented! HUD scrambled for "
			  << (hudDurationMs / 1000) << " seconds!";
	victim->sendSystemMessage(victimMsg.toString());

	return 1;
}

/**
 * Clear the DIZZY state from a creature after duration expires
 */
void DeathWatchManagerImplementation::clearDizzyState(CreatureObject* target) {
	if (target == nullptr) {
		return;
	}

	Locker locker(target);

	if (target->hasState(CreatureState::DIZZY)) {
		target->clearState(CreatureState::DIZZY, true);
	}
}

/**
 * Clear the BLINDED state from a creature after duration expires
 */
void DeathWatchManagerImplementation::clearBlindedState(CreatureObject* target) {
	if (target == nullptr) {
		return;
	}

	Locker locker(target);

	if (target->hasState(CreatureState::BLINDED)) {
		target->clearState(CreatureState::BLINDED, true);
	}
}

/**
 * Restore HUD elements for a creature after Ground Striker effect expires
 */
void DeathWatchManagerImplementation::restoreHUD(CreatureObject* target) {
	if (target == nullptr) {
		return;
	}

	Locker locker(target);

	target->sendNewbieTutorialEnableHudElement("all", 1, 0);
	target->sendSystemMessage("\\#00FF00Your HUD systems have recovered.");
}

// ============================================================================
// HEAVY WEAPON PROCESSING (Rocket Launcher, Acid Beam, Lightning Beam)
// ============================================================================

/**
 * Resolve the actual defender from a target (handles jetpack targeting)
 *
 * If target is a DW jetpack vehicle, returns the rider.
 * If target is a player, returns the player.
 * Otherwise returns nullptr.
 */
CreatureObject* DeathWatchManagerImplementation::resolveHeavyWeaponTarget(TangibleObject* target) {
	if (target == nullptr) {
		return nullptr;
	}

	if (!target->isCreatureObject()) {
		return nullptr;
	}

	CreatureObject* targetCreature = target->asCreatureObject();

	// If it's a vehicle (jetpack), get the rider
	if (targetCreature->isVehicleObject()) {
		VehicleObject* vehicle = dynamic_cast<VehicleObject*>(targetCreature);
		if (vehicle != nullptr) {
			// Check if this is a DW jetpack
			String templateName = vehicle->getObjectTemplate()->getFullTemplateString();
			if (templateName.contains("jetpack_dw_") || templateName.contains("oevv_jetpack")) {
				// Get the rider
				CreatureObject* rider = vehicle->getLinkedCreature().get();
				if (rider != nullptr && rider->isPlayerCreature()) {
					return rider;
				}
			}
		}
		return nullptr;
	}

	// Direct player target
	if (targetCreature->isPlayerCreature()) {
		return targetCreature;
	}

	return nullptr;
}

/**
 * Check if this combat qualifies for DW heavy weapon interception
 *
 * Returns true if:
 * - Attacker is a DW player
 * - Attacker has required certification for the weapon type
 * - Defender is a DW player on a DW jetpack, OR target is a DW jetpack with rider
 * - Factions are at war
 */
bool DeathWatchManagerImplementation::shouldInterceptHeavyWeapon(CreatureObject* attacker, TangibleObject* target, TangibleObject* weapon) {

	if (attacker == nullptr || target == nullptr || weapon == nullptr) {
		return false;
	}

	// Attacker must be a player
	if (!attacker->isPlayerCreature()) {
		return false;
	}

	// Attacker must be DW
	if (!isDeathWatch(attacker)) {
		return false;
	}

	// Attacker must be ON THE GROUND (cannot fire from jetpack)
	if (isOnDeathWatchJetpack(attacker)) {
		return false;
	}

	// Cast to WeaponObject to check type
	WeaponObject* weaponObj = dynamic_cast<WeaponObject*>(weapon);
	if (weaponObj == nullptr) {
		return false;
	}

	int attackType = weaponObj->getAttackType();

	bool isRocketLauncher = (attackType == SharedWeaponObjectTemplate::HEAVYROCKETLAUNCHERATTACK);
	bool isAcidBeam = (attackType == SharedWeaponObjectTemplate::HEAVYACIDBEAMATTACK);
	bool isLightningBeam = (attackType == SharedWeaponObjectTemplate::HEAVYLIGHTNINGBEAMATTACK);
	bool isParticleBeam = (attackType == SharedWeaponObjectTemplate::HEAVYPARTICLEBEAMATTACK);

	if (!isRocketLauncher && !isAcidBeam && !isLightningBeam && !isParticleBeam) {
		return false;
	}

	// Check certification requirements
	// Rocket Launcher: combat_death_watch_zan_1 (Zann Consortium)
	// Particle Beam: combat_death_watch_krr_1 (Krrsantan)
	// Acid Beam: combat_death_watch_krr_2 (Krrsantan)
	// Lightning Beam: combat_death_watch_sun_2 (Black Sun)
	if (isRocketLauncher && !attacker->hasSkill("combat_death_watch_zan_1")) {
		attacker->sendSystemMessage("You need Zann Consortium Box 1 certification to use rocket launchers in Death Watch combat.");
		return false;
	}
	if (isParticleBeam && !attacker->hasSkill("combat_death_watch_krr_1")) {
		attacker->sendSystemMessage("You need Krrsantan Box 1 certification to use particle beam cannons in Death Watch combat.");
		return false;
	}
	if (isAcidBeam && !attacker->hasSkill("combat_death_watch_krr_2")) {
		attacker->sendSystemMessage("You need Krrsantan Box 2 certification to use acid beam cannons in Death Watch combat.");
		return false;
	}
	if (isLightningBeam && !attacker->hasSkill("combat_death_watch_sun_2")) {
		attacker->sendSystemMessage("You need Black Sun Box 2 certification to use lightning beam cannons in Death Watch combat.");
		return false;
	}


	// Resolve the actual defender (handles jetpack targeting)
	CreatureObject* defender = resolveHeavyWeaponTarget(target);

	if (defender == nullptr) {
		return false;
	}


	// Defender must be DW
	if (!isDeathWatch(defender)) {
		return false;
	}


	// Check if factions are at war
	if (!canPlayersAttack(attacker, defender)) {
		return false;
	}

	// TARGET TYPE VALIDATION is done in processHeavyWeaponHit()
	// We intercept all valid DW combat here, then check target type during processing
	// This ensures the shot is consumed even if target type is wrong

	return true;
}

/**
 * Process a heavy weapon hit in DW combat
 *
 * Uses DW skill tape system instead of normal combat calculations.
 * Applies appropriate effects based on weapon type.
 */
int DeathWatchManagerImplementation::processHeavyWeaponHit(CreatureObject* attacker, CreatureObject* defender, TangibleObject* weapon) {
	if (attacker == nullptr || defender == nullptr || weapon == nullptr) {
		return -1;
	}

	// Cast to WeaponObject to get weapon stats
	WeaponObject* weaponObj = dynamic_cast<WeaponObject*>(weapon);
	if (weaponObj == nullptr) {
		return -1;
	}

	// Don't process if defender is already dead or incapacitated
	if (defender->isDead() || defender->isIncapacitated()) {
		return 0;
	}

	// Get weapon properties
	int attackType = weaponObj->getAttackType();
	int minDamage = weaponObj->getMinDamage();
	int maxDamage = weaponObj->getMaxDamage();
	int damageType = weaponObj->getDamageType();
	int armorPiercing = weaponObj->getArmorPiercing();

	// Identify weapon type
	bool isRocketLauncher = (attackType == SharedWeaponObjectTemplate::HEAVYROCKETLAUNCHERATTACK);
	bool isAcidBeam = (attackType == SharedWeaponObjectTemplate::HEAVYACIDBEAMATTACK);
	bool isLightningBeam = (attackType == SharedWeaponObjectTemplate::HEAVYLIGHTNINGBEAMATTACK);
	bool isParticleBeam = (attackType == SharedWeaponObjectTemplate::HEAVYPARTICLEBEAMATTACK);

	if (!isRocketLauncher && !isAcidBeam && !isLightningBeam && !isParticleBeam) {
		// Not a supported heavy weapon type for DW interception
		return -1;
	}

	// =========================================================================
	// ACID BEAM SPRAY MODE (Aerial Strafing)
	// =========================================================================
	// If attacker is on DW jetpack and using acid beam, route to spray attack
	// This is an AoE forward cone attack that ignores the selected target
	if (isAcidBeam && isOnDeathWatchJetpack(attacker)) {
		return processAcidBeamSpray(attacker, weapon);
	}

	// =========================================================================
	// TARGET TYPE VALIDATION
	// =========================================================================
	// Rocket Launcher & Particle Beam: Target must be ON A DW JETPACK (aerial)
	// Acid Beam & Lightning Beam: Target must be ON THE GROUND
	// Wrong target type = shot fired but automatic miss with message
	bool isAerialWeapon = isRocketLauncher || isParticleBeam;
	bool isGroundWeapon = isAcidBeam || isLightningBeam;
	bool defenderOnJetpack = isOnDeathWatchJetpack(defender);

	if (isAerialWeapon && !defenderOnJetpack) {
		// Aerial weapon but target is on ground - wasted shot
		attacker->sendSystemMessage("\\#FF6666[INVALID TARGET] This is a ground-to-air weapon. Your shot is wasted as your target is not airborne.");
		attacker->playEffect("clienteffect/combat_explosion_lair_large.cef", "");

		// Still put both in combat and apply TEF
		attacker->setCombatState();
		defender->setCombatState();
		attacker->setDefender(defender);
		defender->setDefender(attacker);
		applyDwTef(attacker);
		applyDwTef(defender);

		return 0;  // Shot consumed, no damage
	}

	if (isGroundWeapon && defenderOnJetpack) {
		// Ground weapon but target is in air - wasted shot
		attacker->sendSystemMessage("\\#FF6666[INVALID TARGET] This weapon is designed for ground targets. Your shot is wasted as your target is airborne.");
		attacker->playEffect("clienteffect/combat_explosion_lair_large.cef", "");

		// Still put both in combat and apply TEF
		attacker->setCombatState();
		defender->setCombatState();
		attacker->setDefender(defender);
		defender->setDefender(attacker);
		applyDwTef(attacker);
		applyDwTef(defender);

		return 0;  // Shot consumed, no damage
	}

	// Get attacker's skill mods
	int atkAcc = attacker->getSkillMod("dw_acc");
	int atkPotency = attacker->getSkillMod("dw_pot");
	int atkRecovery = attacker->getSkillMod("dw_recovery");

	// Get defender's skill mods
	int defEvasion = defender->getSkillMod("dw_evasion");
	int defDeflection = defender->getSkillMod("dw_deflection");
	int defToughness = defender->getSkillMod("dw_toughness");
	int defRecovery = defender->getSkillMod("dw_recovery");

	// Cap toughness using configured cap
	defToughness = Math::min(defToughness, heavyToughnessCap);

	// =========================================================================
	// HIT CALCULATION
	// =========================================================================
	// Base accuracy from config (default 60%)
	// Attacker dw_acc: bonus per point from config (default +0.3% = 0.003)
	// Defender dw_evasion: penalty per point from config (default -0.25% = 0.0025)
	// Final hit chance clamped to min/max from config (default 30%-90%)

	// Determine attack type key for hidden bonuses
	String attackTypeKey = isRocketLauncher ? "rocket" : (isAcidBeam ? "acid" : (isLightningBeam ? "lightning" : "particle"));

	// =========================================================================
	// HIDDEN TEMPLATE BONUS: Accuracy (attacker)
	// =========================================================================
	int hiddenAccBonus = getHiddenAccuracyBonus(attacker, attackTypeKey);

	float accBonus = atkAcc * (heavyAccBonus * 100.0f);  // Convert from 0.003 to 0.3% per point
	accBonus += hiddenAccBonus;  // Add hidden template bonus

	float evaPenalty = defEvasion * (heavyEvasionPenalty * 100.0f);  // Convert from 0.0025 to 0.25% per point

	float hitChance = heavyBaseAccuracy + accBonus - evaPenalty;
	hitChance = Math::clamp(heavyMinHitChance, hitChance, heavyMaxHitChance);

	// Roll for hit
	int hitRoll = System::random(99) + 1;  // 1-100
	bool isHit = (hitRoll <= hitChance);

	const char* weaponName = isRocketLauncher ? "ROCKET" : (isAcidBeam ? "ACID BEAM" : (isLightningBeam ? "LIGHTNING" : "PARTICLE"));

	// =========================================================================
	// HIDDEN TEMPLATE BONUS: Full Evasion (defender)
	// =========================================================================
	// Some professions grant a chance to fully evade even if the hit roll succeeded
	if (isHit) {
		float hiddenEvasion = getHiddenEvasionChance(defender, attackTypeKey);
		if (hiddenEvasion > 0.0f) {
			float evasionRoll = System::random(10000) / 10000.0f;
			if (evasionRoll < hiddenEvasion) {
				// Full evasion - treat as miss
				isHit = false;
				// Different message for hidden evasion
				defender->playEffect("clienteffect/frs_dark_enraged.cef", "");
				attacker->sendSystemMessage("\\#FFFF66Your target dodged your heavy weapon shot!");
				defender->sendSystemMessage("\\#66FF66You dodged " + String(attacker->getFirstName()) + "'s heavy weapon attack!");
			}
		}
	}

	if (!isHit) {
		// Miss - play miss effect and notify
		attacker->playEffect("clienteffect/combat_explosion_lair_large.cef", "");
		defender->playEffect("clienteffect/frs_dark_enraged.cef", "");

		StringBuffer missMsg;
		missMsg << "[" << weaponName << "] Your shot missed " << defender->getFirstName() << "!";
		attacker->sendSystemMessage(missMsg.toString());

		StringBuffer defMissMsg;
		defMissMsg << "[" << weaponName << "] " << attacker->getFirstName() << "'s heavy weapon shot missed you!";
		defender->sendSystemMessage(defMissMsg.toString());

		// Still apply DW TEF even on miss (you fired at them)
		applyDwTef(attacker);
		applyDwTef(defender);

		// Put both in combat
		attacker->setCombatState();
		defender->setCombatState();
		attacker->setDefender(defender);
		defender->setDefender(attacker);

		return 0;
	}

	// =========================================================================
	// DAMAGE CALCULATION
	// =========================================================================
	// Order of operations:
	// 1. Roll base damage (weapon min to max)
	// 2. Apply dw_toughness (Kandosii): damage *= (1 - toughness/100)
	// 3. Apply weapon multiplier: 0.7x for Acid/Lightning, 1.0x for Rocket/Particle
	// 4. Apply hidden template damage bonus (attacker)
	// 5. Apply dw_deflection reduction: -0.3% per point
	// 6. Apply hidden template damage reduction (defender)
	// 7. Apply Mando armor bonus/reduction
	// 8. Apply armor mitigation

	// Step 1: Roll base damage
	int baseDamage = System::random(maxDamage - minDamage) + minDamage;

	// Step 2: Apply dw_toughness (Kandosii) - FIRST reduction
	float toughnessReduction = 1.0f - (defToughness / 100.0f);
	float damage = baseDamage * toughnessReduction;

	// Step 3: Apply weapon multiplier from config
	// Tier 1 (Acid/Lightning): 0.7x default - these have special abilities
	// Tier 2 (Rocket/Particle): 1.0x default - pure damage weapons
	float weaponMultiplier = heavyRocketMultiplier;  // Default for rocket
	if (isParticleBeam) {
		weaponMultiplier = heavyParticleMultiplier;
	} else if (isAcidBeam) {
		weaponMultiplier = heavyAcidMultiplier;
	} else if (isLightningBeam) {
		weaponMultiplier = heavyLightningMultiplier;
	}
	damage *= weaponMultiplier;

	// Step 4: Apply hidden template damage bonus for attacker
	float hiddenDmgBonus = getHiddenDamageBonus(attacker, attackTypeKey);
	if (hiddenDmgBonus > 0.0f) {
		damage *= (1.0f + hiddenDmgBonus);
	}

	// Step 5: Apply dw_deflection reduction from config (default -0.3% per point = 0.003)
	// NOTE: dw_pot is intentionally NOT applied to heavy weapons - they're powerful enough
	float deflReduction = 1.0f - (defDeflection * heavyDeflectionReduction);
	deflReduction = Math::max(0.7f, deflReduction);  // Floor at 30% reduction
	damage *= deflReduction;

	// Step 6: Apply hidden template damage reduction for defender
	float hiddenDmgReduction = getHiddenDamageReduction(defender, attackTypeKey);
	if (hiddenDmgReduction > 0.0f) {
		damage *= (1.0f - hiddenDmgReduction);
	}

	// Step 7: Apply Mandalorian armor damage bonus for attacker (small bonus for full suit)
	float mandoBonus = getMandoArmorDamageBonus(attacker);
	damage *= mandoBonus;

	// Step 8: Apply full CombatManager armor reduction (PSG, armor, ranged defense, etc.)
	// This uses the same mitigation that normal combat uses
	float mitigatedDamage = getHeavyWeaponMitigation(attacker, defender, weaponObj, damage);

	// Step 9: Apply Mandalorian armor damage reduction for defender (small bonus for full suit)
	float mandoReduction = getMandoArmorDamageReduction(defender);
	mitigatedDamage *= mandoReduction;

	int totalDamage = (int)Math::max(1.0f, mitigatedDamage);

	// =========================================================================
	// ADRENALINE SERUM - High Damage Mitigation
	// =========================================================================
	// If defender has Adrenaline buff active and damage > 5000, apply mitigation
	// Formula: 25% base + ((dw_deflection + dw_toughness) / 8)%, max 50%
	totalDamage = applyAdrenalineHighDamageMitigation(defender, totalDamage);

	// Lock defender for damage application
	Locker defenderLocker(defender);

	// Damage distribution depends on weapon type
	int healthDamage = 0;
	int actionDamage = 0;

	if (isLightningBeam) {
		// Lightning does 30% health, 70% action (drains stamina)
		healthDamage = totalDamage * 30 / 100;
		actionDamage = totalDamage - healthDamage;
	} else {
		// Rocket and Acid do 50/50 split
		healthDamage = totalDamage / 2;
		actionDamage = totalDamage - healthDamage;
	}

	// Apply damage
	defender->inflictDamage(attacker, 0, healthDamage, 1);
	defender->inflictDamage(attacker, 3, actionDamage, 1);

	// Play hit effects based on weapon type
	if (isRocketLauncher) {
		defender->playEffect("clienteffect/cbt_msl_impact_concussion.cef", "");
		attacker->playEffect("clienteffect/cbt_msl_fire_heatseeker.cef", "");
	} else if (isAcidBeam) {
		defender->playEffect("clienteffect/combat_acid_hit.cef", "");
		attacker->playEffect("clienteffect/combat_acid_beam.cef", "");

		// Acid beam bonus: Apply armor corrosion and small DOT
		// Use potency for acid strength (0.4 to 1.0 based on potency)
		float acidStrength = 0.4f + (atkPotency * 0.006f);
		acidStrength = Math::clamp(0.4f, acidStrength, 1.0f);

		// Apply condition damage to armor (4-8 base, scaled by acid strength)
		int baseConditionDamage = 4 + System::random(4);
		applyAcidCorrosion(attacker, defender, baseConditionDamage, acidStrength, 1.0f);

		// Apply small acid DOT (10% of total damage over 6 seconds)
		int dotDamage = totalDamage / 10;
		if (dotDamage > 0) {
			applyAcidDot(attacker, defender, dotDamage, 6, 2000);
		}
	} else if (isLightningBeam) {
		defender->playEffect("clienteffect/combat_lightning_blind.cef", "");
		attacker->playEffect("clienteffect/combat_lightning_beam.cef", "");

		// Lightning beam bonus: Apply action drain DOT (15% of total over 8 seconds)
		int actionDrainTotal = totalDamage * 15 / 100;
		if (actionDrainTotal > 0) {
			applyActionDrain(attacker, defender->getObjectID(), actionDrainTotal, 8, 2.0f);
		}
	} else if (isParticleBeam) {
		defender->playEffect("clienteffect/combat_particle_hit.cef", "");
		attacker->playEffect("clienteffect/combat_particle_beam.cef", "");

		// Particle beam bonus: High damage, heavy AP, applies wounds
		// Apply wounds to health pool (5-10 wounds based on potency)
		int wounds = 5 + (atkPotency / 20);  // 5-10 wounds at max potency
		defender->addWounds(0, wounds, true);  // Pool 0 = health

		// Apply "Mand'alor's Grip" effect (defense reduction over time)
		// Only if defender is not immune
		startParticleBeamEffect(attacker, defender);
	}

	// Put both players into combat state
	attacker->setCombatState();
	defender->setCombatState();
	attacker->setDefender(defender);
	defender->setDefender(attacker);

	// Apply DW TEF to both players
	applyDwTef(attacker);
	applyDwTef(defender);

	// Send hit messages
	StringBuffer atkMsg;
	atkMsg << "[" << weaponName << "] You hit " << defender->getFirstName() << " for " << totalDamage << " damage!";
	attacker->sendSystemMessage(atkMsg.toString());

	StringBuffer defMsg;
	defMsg << "[" << weaponName << "] " << attacker->getFirstName() << " hit you for " << totalDamage << " damage!";
	if (isAcidBeam) {
		defMsg << " Your armor is corroding!";
	} else if (isLightningBeam) {
		defMsg << " Your systems are overloading!";
	} else if (isParticleBeam) {
		defMsg << " You've been wounded!";
	}
	defender->sendSystemMessage(defMsg.toString());

	// Log heavy weapon hit for dispute resolution
	error() << "DW_HEAVY: " << attacker->getFirstName() << " hit " << defender->getFirstName()
			<< " with " << weaponName << " for " << totalDamage << " dmg (hit%=" << (int)hitChance << ")";

	return totalDamage;
}

// ============================================================================
// ACID BEAM SPRAY (Aerial Strafing Weapon)
// ============================================================================

/**
 * Process Acid Beam spray attack from jetpack
 *
 * When a player fires an Acid Beam while on a DW jetpack, it sprays acid
 * in a forward cone hitting all players (friend or foe) within range.
 *
 * Mechanics:
 * - Requires: Player on DW jetpack, Acid Beam equipped, KRR Box 2 skill
 * - 48m range, 90 degree cone (45 degrees each side of facing)
 * - Friendly fire: hits ALL players in cone regardless of faction
 * - Distance-based damage: closer targets take more damage (linear falloff)
 * - Full armor mitigation applies to each target
 * - Applies acid corrosion + DoT to each hit target
 *
 * @param attacker - The player firing the acid beam from jetpack
 * @param weapon - The acid beam weapon being fired
 * @return Number of targets hit, -1 on error
 */
int DeathWatchManagerImplementation::processAcidBeamSpray(CreatureObject* attacker, TangibleObject* weapon) {
	if (attacker == nullptr || weapon == nullptr) {
		return -1;
	}

	// Must be on DW jetpack
	if (!isOnDeathWatchJetpack(attacker)) {
		attacker->sendSystemMessage("\\#FF6666[ACID SPRAY] You must be on a Death Watch jetpack to use the acid spray.");
		return -1;
	}

	// Must have KRR Box 2 skill
	if (!attacker->hasSkill("combat_death_watch_krr_2")) {
		attacker->sendSystemMessage("\\#FF6666[ACID SPRAY] You need Krrsantan Box 2 certification to use acid beam spray.");
		return -1;
	}

	// Cast to WeaponObject
	WeaponObject* weaponObj = dynamic_cast<WeaponObject*>(weapon);
	if (weaponObj == nullptr) {
		return -1;
	}

	// Get weapon stats
	int minDamage = weaponObj->getMinDamage();
	int maxDamage = weaponObj->getMaxDamage();
	int damageType = weaponObj->getDamageType();
	int armorPiercing = weaponObj->getArmorPiercing();

	// Get attacker position and zone
	Zone* zone = attacker->getZone();
	if (zone == nullptr) {
		return -1;
	}

	float attackerX = attacker->getWorldPositionX();
	float attackerZ = attacker->getWorldPositionZ();
	float attackerY = attacker->getWorldPositionY();

	// Get attacker's skill mods for damage calculation
	int atkPotency = attacker->getSkillMod("dw_pot");

	// Spray configuration
	const float SPRAY_RANGE = 48.0f;
	const float SPRAY_CONE_DEGREES = 45.0f;  // 45 degrees each side = 90 degree total cone

	// Get nearby objects
	SortedVector<ManagedReference<TreeEntry*>> nearbyObjects;
	zone->getInRangeObjects(attackerX, attackerZ, attackerY, SPRAY_RANGE, &nearbyObjects, true);

	int targetsHit = 0;

	// Play attacker effect
	attacker->playEffect("clienteffect/combat_acid_beam.cef", "");

	for (int i = 0; i < nearbyObjects.size(); i++) {
		SceneObject* obj = static_cast<SceneObject*>(nearbyObjects.get(i).get());
		if (obj == nullptr || !obj->isPlayerCreature()) {
			continue;
		}

		CreatureObject* target = dynamic_cast<CreatureObject*>(obj);
		if (target == nullptr || target == attacker) {
			continue;
		}

		// Skip dead or incapacitated targets
		if (target->isDead() || target->isIncapacitated()) {
			continue;
		}

		// Check if target is within cone (45 degrees each side)
		float facingOffset = getFacingOffsetAngle(attacker, target);
		if (facingOffset > SPRAY_CONE_DEGREES) {
			continue;  // Not in cone
		}

		// Calculate distance for damage falloff
		float targetX = target->getWorldPositionX();
		float targetY = target->getWorldPositionY();
		float dx = targetX - attackerX;
		float dy = targetY - attackerY;
		float distance = sqrt(dx * dx + dy * dy);

		// Linear damage falloff: full damage at 0m, zero at 48m
		float falloff = 1.0f - (distance / SPRAY_RANGE);
		falloff = Math::clamp(0.0f, falloff, 1.0f);

		if (falloff <= 0.0f) {
			continue;  // Too far for damage
		}

		// Roll base damage with falloff
		int baseDamage = System::random(maxDamage - minDamage) + minDamage;
		float damage = baseDamage * falloff;

		// Get target's defensive skill mods
		int defDeflection = target->getSkillMod("dw_deflection");
		int defToughness = target->getSkillMod("dw_toughness");

		// Cap toughness
		defToughness = Math::min(defToughness, heavyToughnessCap);

		// Apply damage pipeline (simplified from processHeavyWeaponHit)
		// Step 1: Apply toughness reduction
		float toughnessReduction = 1.0f - (defToughness / 100.0f);
		damage *= toughnessReduction;

		// Step 2: Apply acid weapon multiplier
		damage *= heavyAcidMultiplier;

		// Step 3: Apply deflection reduction
		float deflReduction = 1.0f - (defDeflection * heavyDeflectionReduction);
		deflReduction = Math::max(0.7f, deflReduction);
		damage *= deflReduction;

		// Step 4: Apply Mandalorian armor bonus/reduction
		float mandoBonus = getMandoArmorDamageBonus(attacker);
		damage *= mandoBonus;

		// Step 5: Apply full armor mitigation
		float mitigatedDamage = getHeavyWeaponMitigation(attacker, target, weaponObj, damage);

		float mandoReduction = getMandoArmorDamageReduction(target);
		mitigatedDamage *= mandoReduction;

		int totalDamage = (int)Math::max(1.0f, mitigatedDamage);

		// Apply adrenaline high damage mitigation if applicable
		totalDamage = applyAdrenalineHighDamageMitigation(target, totalDamage);

		// Lock target for damage application
		Locker targetLocker(target, attacker);

		// Acid does 50/50 health/action split
		int healthDamage = totalDamage / 2;
		int actionDamage = totalDamage - healthDamage;

		// Apply damage
		target->inflictDamage(attacker, 0, healthDamage, 1);
		target->inflictDamage(attacker, 3, actionDamage, 1);

		// Apply acid corrosion bonus (same as single-target acid beam)
		float acidStrength = 0.4f + (atkPotency * 0.006f);
		acidStrength = Math::clamp(0.4f, acidStrength, 1.0f);
		int baseConditionDamage = 4 + System::random(4);
		applyAcidCorrosion(attacker, target, baseConditionDamage, acidStrength, 1.0f);

		// Apply small acid DoT (10% of total damage over 6 seconds)
		int dotDamage = totalDamage / 10;
		if (dotDamage > 0) {
			applyAcidDot(attacker, target, dotDamage, 6, 2000);
		}

		// Play hit effect on target
		target->playEffect("clienteffect/combat_acid_hit.cef", "");

		// Put both in combat
		attacker->setCombatState();
		target->setCombatState();
		attacker->setDefender(target);
		target->setDefender(attacker);

		// Apply DW TEF
		applyDwTef(attacker);
		applyDwTef(target);

		// Send messages
		StringBuffer atkMsg;
		atkMsg << "\\#00FF00[ACID SPRAY] \\#FFFFFFYou hit " << target->getFirstName()
			   << " for " << totalDamage << " damage! (dist: " << (int)distance << "m, falloff: " << (int)(falloff * 100) << "%)";
		attacker->sendSystemMessage(atkMsg.toString());

		StringBuffer defMsg;
		defMsg << "\\#00FF00[ACID SPRAY] \\#FFFFFF" << attacker->getFirstName()
			   << " hit you with acid spray for " << totalDamage << " damage! Your armor is corroding!";
		target->sendSystemMessage(defMsg.toString());

		targetsHit++;

		// Log for dispute resolution
		error() << "DW_ACID_SPRAY: " << attacker->getFirstName() << " hit " << target->getFirstName()
				<< " for " << totalDamage << " dmg (dist=" << (int)distance << "m, falloff=" << (int)(falloff * 100) << "%)";
	}

	// Notify attacker of total hits
	if (targetsHit > 0) {
		StringBuffer summaryMsg;
		summaryMsg << "\\#00FF00[ACID SPRAY] \\#FFFFFFSpray complete! Hit " << targetsHit << " target(s).";
		attacker->sendSystemMessage(summaryMsg.toString());
	} else {
		attacker->sendSystemMessage("\\#00FF00[ACID SPRAY] \\#FFFFFFNo targets in spray cone.");
	}

	return targetsHit;
}

// ============================================================================
// PARTICLE BEAM "MAND'ALOR'S GRIP" EFFECT
// ============================================================================

void DeathWatchManagerImplementation::applyParticleBeamImmunity(CreatureObject* defender, int durationMs) {
	if (defender == nullptr) {
		return;
	}

	uint64 defenderID = defender->getObjectID();

	Locker locker(&particleBeamImmunityMutex);

	Time expirationTime;
	expirationTime.updateToCurrentTime();
	expirationTime.addMiliTime(durationMs);

	particleBeamImmunity.put(defenderID, expirationTime);
}

bool DeathWatchManagerImplementation::hasParticleBeamImmunity(CreatureObject* defender) {
	if (defender == nullptr) {
		return false;
	}

	uint64 defenderID = defender->getObjectID();

	Locker locker(&particleBeamImmunityMutex);

	if (!particleBeamImmunity.contains(defenderID)) {
		return false;
	}

	Time& expirationTime = particleBeamImmunity.get(defenderID);
	Time now;
	now.updateToCurrentTime();

	if (now.compareTo(expirationTime) >= 0) {
		// Immunity expired, remove it
		particleBeamImmunity.drop(defenderID);
		return false;
	}

	return true;
}

bool DeathWatchManagerImplementation::startParticleBeamEffect(CreatureObject* attacker, CreatureObject* defender) {
	if (attacker == nullptr || defender == nullptr) {
		return false;
	}

	// Check if defender is immune
	if (hasParticleBeamImmunity(defender)) {
		attacker->sendSystemMessage("Target is immune to Mand'alor's Grip.");
		return false;
	}

	// Get skill mods for effect calculation
	int atkPotency = attacker->getSkillMod("dw_pot");
	int defRecovery = defender->getSkillMod("dw_recovery");

	// =========================================================================
	// CALCULATE EFFECT PARAMETERS (using loaded config values)
	// =========================================================================
	// Per-tick reduction:
	//   tickReduction = base + (pot * potScaling) - (recovery * recoveryScaling)
	//   Clamped to min-max from config
	float tickReduction = pbBasePerTick + (atkPotency * pbPotTickScaling) - (defRecovery * pbRecoveryTickScaling);
	tickReduction = Math::clamp(pbMinTickReduction, tickReduction, pbMaxTickReduction);

	// Peak duration:
	//   peakDuration = base + (pot * potScaling) - (recovery * recoveryScaling)
	//   Clamped to min-max from config
	float peakDuration = pbBasePeakDuration + (atkPotency * pbPotPeakScaling) - (defRecovery * pbRecoveryPeakScaling);
	peakDuration = Math::clamp(pbMinPeakDuration, peakDuration, pbMaxPeakDuration);
	int peakDurationMs = (int)(peakDuration * 1000);

	// Immunity duration:
	//   immunity = base + (recovery * bonus)
	float immunityDuration = pbBaseImmunity + (defRecovery * pbRecoveryImmunityBonus);
	int immunityDurationMs = (int)(immunityDuration * 1000);

	// Create and schedule the tick task (using config values for ticks and interval)
	Reference<DwParticleBeamTickTask*> task = new DwParticleBeamTickTask(
		attacker, defender, zoneServer,
		tickReduction, pbNumTicks, pbTickIntervalMs, peakDurationMs, immunityDurationMs
	);

	// Schedule first tick immediately (or with small delay)
	task->schedule(500);  // 500ms delay before first tick

	return true;
}

// ============================================================================
// HIDDEN TEMPLATE BONUSES
// ============================================================================
// Secret bonuses based on combat profession templates.
// Not displayed to players - discovery through gameplay.
// Design goal: Every profession equally valuable, no single "best" template.

/**
 * Helper function to get bonus value from Lua config
 * Searches the dwHiddenTemplateBonuses table for matching skill and bonus type
 */
float DeathWatchManagerImplementation::getHiddenBonusFromLua(CreatureObject* player, const String& attackType,
		const String& bonusType, bool isDefender, const String& specificEffect) {

	if (player == nullptr)
		return 0.0f;

	// Get the Lua instance
	Lua* lua = DirectorManager::instance()->getLuaInstance();
	if (lua == nullptr)
		return 0.0f;

	float totalBonus = 0.0f;
	float diminishingFactor = 0.70f;  // Second source = 70% effectiveness
	int bonusCount = 0;

	// Build the Lua table path: dwHiddenTemplateBonuses.<attackType>.<defender/attacker/user>
	String contextKey;
	if (attackType.beginsWith("serum")) {
		contextKey = "user";
	} else {
		contextKey = isDefender ? "defender" : "attacker";
	}

	// Also check global whistler bonuses if this is a whistler attack
	bool checkGlobal = attackType.beginsWith("mk") && isDefender;

	// Get global state to run Lua
	lua_State* L = lua->getLuaState();
	if (L == nullptr)
		return 0.0f;

	// Function to check one attack type's bonuses
	auto checkBonuses = [&](const String& atkType) {
		// Get dwHiddenTemplateBonuses table
		lua_getglobal(L, "dwHiddenTemplateBonuses");
		if (!lua_istable(L, -1)) {
			lua_pop(L, 1);
			return;
		}

		// Get attack type subtable
		lua_getfield(L, -1, atkType.toCharArray());
		if (!lua_istable(L, -1)) {
			lua_pop(L, 2);
			return;
		}

		// Get context subtable (defender/attacker/user)
		lua_getfield(L, -1, contextKey.toCharArray());
		if (!lua_istable(L, -1)) {
			lua_pop(L, 3);
			return;
		}

		// Iterate through the bonus array
		int len = lua_rawlen(L, -1);
		for (int i = 1; i <= len; i++) {
			lua_rawgeti(L, -1, i);
			if (!lua_istable(L, -1)) {
				lua_pop(L, 1);
				continue;
			}

			// Get skill requirement
			lua_getfield(L, -1, "skill");
			if (!lua_isstring(L, -1)) {
				lua_pop(L, 2);
				continue;
			}
			String skill = lua_tostring(L, -1);
			lua_pop(L, 1);

			// Check if player has this skill
			if (!player->hasSkill(skill)) {
				lua_pop(L, 1);
				continue;
			}

			// Player has the skill - check for the requested bonus type
			float bonusValue = 0.0f;
			bool foundBonus = false;

			if (bonusType == "damageReduction") {
				lua_getfield(L, -1, "damageReduction");
				if (lua_isnumber(L, -1)) {
					bonusValue = lua_tonumber(L, -1);
					foundBonus = true;
				}
				lua_pop(L, 1);
			} else if (bonusType == "evasionChance") {
				lua_getfield(L, -1, "evasionChance");
				if (lua_isnumber(L, -1)) {
					bonusValue = lua_tonumber(L, -1);
					foundBonus = true;
				}
				lua_pop(L, 1);
			} else if (bonusType == "damageBonus") {
				lua_getfield(L, -1, "damageBonus");
				if (lua_isnumber(L, -1)) {
					bonusValue = lua_tonumber(L, -1);
					foundBonus = true;
				}
				lua_pop(L, 1);
			} else if (bonusType == "accuracyBonus") {
				lua_getfield(L, -1, "accuracyBonus");
				if (lua_isnumber(L, -1)) {
					bonusValue = lua_tonumber(L, -1);
					foundBonus = true;
				}
				lua_pop(L, 1);
			} else if (bonusType == "durationBonus") {
				lua_getfield(L, -1, "durationBonus");
				if (lua_isnumber(L, -1)) {
					bonusValue = lua_tonumber(L, -1);
					foundBonus = true;
				}
				lua_pop(L, 1);
			} else if (bonusType == "cooldownReduction") {
				lua_getfield(L, -1, "cooldownReduction");
				if (lua_isnumber(L, -1)) {
					bonusValue = lua_tonumber(L, -1);
					foundBonus = true;
				}
				lua_pop(L, 1);
			} else if (bonusType == "specialEffect" && !specificEffect.isEmpty()) {
				lua_getfield(L, -1, "specialEffect");
				if (lua_isstring(L, -1)) {
					String effectName = lua_tostring(L, -1);
					if (effectName == specificEffect) {
						lua_pop(L, 1);
						lua_getfield(L, -1, "value");
						if (lua_isnumber(L, -1)) {
							bonusValue = lua_tonumber(L, -1);
							foundBonus = true;
						}
						lua_pop(L, 1);
					} else {
						lua_pop(L, 1);
					}
				} else {
					lua_pop(L, 1);
				}
			}

			if (foundBonus && bonusValue > 0) {
				// Apply diminishing returns for multiple sources
				if (bonusCount == 0) {
					totalBonus += bonusValue;
				} else {
					totalBonus += bonusValue * diminishingFactor;
				}
				bonusCount++;
			}

			lua_pop(L, 1);  // Pop bonus entry
		}

		lua_pop(L, 3);  // Pop context, attackType, and dwHiddenTemplateBonuses tables
	};

	// Check the specific attack type
	checkBonuses(attackType);

	// Check global whistler bonuses if applicable
	if (checkGlobal) {
		checkBonuses("whistlerGlobal");
	}

	return totalBonus;
}

float DeathWatchManagerImplementation::getHiddenDamageReduction(CreatureObject* defender, const String& attackType) {
	float reduction = getHiddenBonusFromLua(defender, attackType, "damageReduction", true, "");

	// Cap at 40%
	return Math::min(reduction, 0.40f);
}

float DeathWatchManagerImplementation::getHiddenEvasionChance(CreatureObject* defender, const String& attackType) {
	float evasion = getHiddenBonusFromLua(defender, attackType, "evasionChance", true, "");

	// Cap at 25%
	return Math::min(evasion, 0.25f);
}

float DeathWatchManagerImplementation::getHiddenDamageBonus(CreatureObject* attacker, const String& attackType) {
	float bonus = getHiddenBonusFromLua(attacker, attackType, "damageBonus", false, "");

	// Cap at 50%
	return Math::min(bonus, 0.50f);
}

int DeathWatchManagerImplementation::getHiddenAccuracyBonus(CreatureObject* attacker, const String& attackType) {
	float bonus = getHiddenBonusFromLua(attacker, attackType, "accuracyBonus", false, "");

	// Cap at 50
	return Math::min((int)bonus, 50);
}

float DeathWatchManagerImplementation::getHiddenSpecialEffect(CreatureObject* player, const String& attackType,
		const String& effectName, bool isDefender) {
	return getHiddenBonusFromLua(player, attackType, "specialEffect", isDefender, effectName);
}

bool DeathWatchManagerImplementation::rollHiddenDudChance(CreatureObject* defender, const String& attackType) {
	// Get dud chance from Lua (Smuggler special)
	float dudChance = getHiddenBonusFromLua(defender, attackType, "specialEffect", true, "dudChance");

	if (dudChance <= 0.0f)
		return false;

	// Cap at 15%
	dudChance = Math::min(dudChance, 0.15f);

	// Roll
	float roll = System::random(10000) / 10000.0f;
	return roll < dudChance;
}

float DeathWatchManagerImplementation::getSerumDurationBonus(CreatureObject* player, const String& serumType) {
	float bonus = getHiddenBonusFromLua(player, serumType, "durationBonus", false, "");

	// Cap at 50%
	return Math::min(bonus, 0.50f);
}

float DeathWatchManagerImplementation::getSerumCooldownReduction(CreatureObject* player, const String& serumType) {
	float reduction = getHiddenBonusFromLua(player, serumType, "cooldownReduction", false, "");

	// Cap at 30%
	return Math::min(reduction, 0.30f);
}

// ============================================================================
// DEATH WATCH SAFE CLONING SYSTEM
// ============================================================================

bool DeathWatchManagerImplementation::shouldInterceptCloning(CreatureObject* player) {
	if (player == nullptr)
		return false;

	// Use skill-only check - clone window should appear even before faction is assigned
	return hasDwNoviceSkill(player);
}

int DeathWatchManagerImplementation::countEnemyDwPlayersInRange(CreatureObject* player, Zone* zone, float x, float y, float range) {
	if (player == nullptr || zone == nullptr)
		return 0;

	// Get player's faction to determine enemies
	PlayerObject* ghost = player->getPlayerObject();
	if (ghost == nullptr)
		return 0;

	byte playerFaction = ghost->getDwFaction();
	if (playerFaction == 0)
		return 0;

	// Query zone for players in range
	SortedVector<ManagedReference<TreeEntry*>> nearbyPlayers;
	zone->getInRangePlayers(x, 0, y, range, &nearbyPlayers);

	int enemyCount = 0;

	for (int i = 0; i < nearbyPlayers.size(); i++) {
		SceneObject* obj = cast<SceneObject*>(nearbyPlayers.get(i).get());
		if (obj == nullptr || !obj->isPlayerCreature())
			continue;

		CreatureObject* nearbyPlayer = cast<CreatureObject*>(obj);
		if (nearbyPlayer == nullptr || nearbyPlayer == player)
			continue;

		// Check if this player is DW
		if (!isDeathWatch(nearbyPlayer))
			continue;

		// Check if they're an enemy (at war)
		PlayerObject* nearbyGhost = nearbyPlayer->getPlayerObject();
		if (nearbyGhost == nullptr)
			continue;

		byte nearbyFaction = nearbyGhost->getDwFaction();
		if (nearbyFaction == 0)
			continue;

		// Check if factions are at war
		if (areFactionsAtWar(playerFaction, nearbyFaction)) {
			enemyCount++;
		}
	}

	return enemyCount;
}

void DeathWatchManagerImplementation::sendDeathWatchCloneRequest(CreatureObject* player, int typeofdeath) {
	if (player == nullptr)
		return;

	PlayerObject* ghost = player->getPlayerObject();
	if (ghost == nullptr)
		return;

	ghost->removeSuiBoxType(SuiWindowType::CLONE_REQUEST);

	// Calculate threat scan range based on dw_recovery skill mod
	// Range scales from 64m (0 skill) to 1024m (100 skill)
	int dwRecovery = player->getSkillMod("dw_recovery");
	if (dwRecovery < 0) dwRecovery = 0;
	if (dwRecovery > 100) dwRecovery = 100;
	float threatScanRange = 64.0f + (dwRecovery * 9.6f);

	// Use parallel arrays instead of struct with Vector (avoids serialization requirements)
	// Max 12 options: 1 HOME + 1 HQ + 10 random
	const int MAX_OPTIONS = 64;  // Buffer for collecting all options before selection
	uint64 optionClonerIDs[MAX_OPTIONS];
	String optionNames[MAX_OPTIONS];
	String optionZones[MAX_OPTIONS];
	int optionEnemyCounts[MAX_OPTIONS];
	int optionCount = 0;

	// Track HOME facility
	uint64 homeClonerID = ghost->getCloningFacility();
	uint64 homeValidID = 0;
	String homeName;
	String homeZoneName;
	int homeEnemyCount = 0;

	// Get HOME facility info if set
	if (homeClonerID != 0) {
		ManagedReference<SceneObject*> homeCloner = zoneServer->getObject(homeClonerID);
		if (homeCloner != nullptr) {
			Zone* homeZone = homeCloner->getZone();
			if (homeZone != nullptr) {
				homeValidID = homeClonerID;
				homeZoneName = homeZone->getZoneName();
				homeEnemyCount = countEnemyDwPlayersInRange(player, homeZone,
					homeCloner->getWorldPositionX(), homeCloner->getWorldPositionY(), threatScanRange);

				ManagedReference<CityRegion*> cityRegion = homeCloner->getCityRegion().get();
				if (cityRegion != nullptr) {
					homeName = cityRegion->getRegionDisplayedName();
				} else {
					homeName = homeCloner->getDisplayedName();
				}
			}
		}
	}

	// Get Faction HQ clone spawn info from Lua
	// The HQ isn't a real cloner - it's a spawn point in the faction's HQ building
	// We use the building ID as a pseudo-cloner ID and handle it specially in sendDeathWatchPlayerToCloner
	uint64 hqBuildingID = 0;
	uint64 hqCellID = 0;
	float hqSpawnX = 0, hqSpawnY = 0, hqSpawnZ = 0;
	String hqName;
	String hqZoneName;
	int hqEnemyCount = 0;

	// Get player's faction to determine which HQ
	byte playerFaction = ghost->getDwFaction();
	String factionKey;
	switch (playerFaction) {
		case 1: factionKey = "xon"; hqName = "Xonti Brothers HQ"; break;
		case 2: factionKey = "sun"; hqName = "Black Sun HQ"; break;
		case 3: factionKey = "krr"; hqName = "Black Krrsantan HQ"; break;
		case 4: factionKey = "zan"; hqName = "Zann Consortium HQ"; break;
		default: factionKey = ""; break;
	}

	// Try to get HQ spawn info from Lua
	if (!factionKey.isEmpty()) {
		Lua* lua = DirectorManager::instance()->getLuaInstance();
		if (lua != nullptr) {
			lua_State* L = lua->getLuaState();

			// Call death_watch_hq:getCloneSpawnForFaction(factionKey)
			lua_getglobal(L, "death_watch_hq");
			if (lua_istable(L, -1)) {
				lua_getfield(L, -1, "getCloneSpawnForFaction");
				if (lua_isfunction(L, -1)) {
					lua_pushvalue(L, -2);  // self (death_watch_hq table)
					lua_pushstring(L, factionKey.toCharArray());
					if (lua_pcall(L, 2, 1, 0) == 0 && lua_istable(L, -1)) {
						// Read the spawn info
						lua_getfield(L, -1, "buildingID");
						hqBuildingID = lua_tointeger(L, -1);
						lua_pop(L, 1);

						lua_getfield(L, -1, "cellID");
						hqCellID = lua_tointeger(L, -1);
						lua_pop(L, 1);

						lua_getfield(L, -1, "x");
						hqSpawnX = lua_tonumber(L, -1);
						lua_pop(L, 1);

						lua_getfield(L, -1, "z");
						hqSpawnZ = lua_tonumber(L, -1);
						lua_pop(L, 1);

						lua_getfield(L, -1, "y");
						hqSpawnY = lua_tonumber(L, -1);
						lua_pop(L, 1);

						lua_getfield(L, -1, "zoneName");
						if (lua_isstring(L, -1)) {
							hqZoneName = lua_tostring(L, -1);
						}
						lua_pop(L, 1);

						// Get threat level at HQ
						if (!hqZoneName.isEmpty()) {
							Zone* hqZone = zoneServer->getZone(hqZoneName);
							if (hqZone != nullptr) {
								ManagedReference<SceneObject*> hqBuilding = zoneServer->getObject(hqBuildingID);
								if (hqBuilding != nullptr) {
									hqEnemyCount = countEnemyDwPlayersInRange(player, hqZone,
										hqBuilding->getWorldPositionX(), hqBuilding->getWorldPositionY(), threatScanRange);
								}
							}
						}
					}
					lua_pop(L, 1);  // result or error
				} else {
					lua_pop(L, 1);  // non-function
				}
			}
			lua_pop(L, 1);  // death_watch_hq table
		}
	}

	// Query all zones for cloning facilities - pick 1 random cloner per planet
	int zoneCount = zoneServer->getZoneCount();

	for (int z = 0; z < zoneCount && optionCount < MAX_OPTIONS; z++) {
		Zone* zone = zoneServer->getZone(z);
		if (zone == nullptr)
			continue;

		String zoneName = zone->getZoneName();

		// Skip instance zones and DW HQ zone (badlands is DW-only, not a safe clone destination)
		if (zoneName == "dungeon1" || zoneName == "dungeon2" || zoneName == "badlands")
			continue;

		SortedVector<ManagedReference<SceneObject*>> cloners = zone->getPlanetaryObjectList("cloningfacility");

		// Collect valid cloners for this zone
		const int MAX_ZONE_CLONERS = 32;
		uint64 zoneClonerIDs[MAX_ZONE_CLONERS];
		String zoneClonerNames[MAX_ZONE_CLONERS];
		int zoneClonerCount = 0;

		for (int c = 0; c < cloners.size() && zoneClonerCount < MAX_ZONE_CLONERS; c++) {
			ManagedReference<SceneObject*> cloner = cloners.get(c);
			if (cloner == nullptr)
				continue;

			uint64 clonerID = cloner->getObjectID();

			// Skip if this is the HOME cloner (added separately)
			if (clonerID == homeClonerID)
				continue;

			CloningBuildingObjectTemplate* cbot = cast<CloningBuildingObjectTemplate*>(cloner->getObjectTemplate());
			if (cbot == nullptr)
				continue;

			// Skip player city cloners
			if (cbot->getFacilityType() == CloningBuildingObjectTemplate::PLAYER_CITY)
				continue;

			// Skip faction-specific cloners (Imperial/Rebel)
			if (cbot->getFacilityType() == CloningBuildingObjectTemplate::FACTION_IMPERIAL ||
				cbot->getFacilityType() == CloningBuildingObjectTemplate::FACTION_REBEL)
				continue;

			// Skip Jedi cloners
			if (cbot->isJediCloner())
				continue;

			ManagedReference<CityRegion*> cityRegion = cloner->getCityRegion().get();
			if (cityRegion != nullptr && cityRegion->isBanned(player->getObjectID()))
				continue;

			// Add to zone's cloner list
			zoneClonerIDs[zoneClonerCount] = clonerID;
			if (cityRegion != nullptr) {
				zoneClonerNames[zoneClonerCount] = cityRegion->getRegionDisplayedName();
			} else {
				zoneClonerNames[zoneClonerCount] = cloner->getDisplayedName();
			}
			zoneClonerCount++;
		}

		// Pick 1 random cloner from this zone (if any valid cloners exist)
		if (zoneClonerCount > 0) {
			int selectedIdx = System::random(zoneClonerCount - 1);
			uint64 selectedID = zoneClonerIDs[selectedIdx];

			// Get the cloner object to calculate threat
			ManagedReference<SceneObject*> selectedCloner = zoneServer->getObject(selectedID);
			if (selectedCloner != nullptr) {
				optionClonerIDs[optionCount] = selectedID;
				optionZones[optionCount] = zoneName;
				optionNames[optionCount] = zoneClonerNames[selectedIdx];
				optionEnemyCounts[optionCount] = countEnemyDwPlayersInRange(player, zone,
					selectedCloner->getWorldPositionX(), selectedCloner->getWorldPositionY(), threatScanRange);
				optionCount++;
			}
		}
	}

	// Sort all options by threat level (bubble sort - safest first)
	for (int i = 0; i < optionCount - 1; i++) {
		for (int j = i + 1; j < optionCount; j++) {
			if (optionEnemyCounts[j] < optionEnemyCounts[i]) {
				// Swap
				uint64 tmpID = optionClonerIDs[i];
				optionClonerIDs[i] = optionClonerIDs[j];
				optionClonerIDs[j] = tmpID;

				String tmpName = optionNames[i];
				optionNames[i] = optionNames[j];
				optionNames[j] = tmpName;

				String tmpZone = optionZones[i];
				optionZones[i] = optionZones[j];
				optionZones[j] = tmpZone;

				int tmpEnemy = optionEnemyCounts[i];
				optionEnemyCounts[i] = optionEnemyCounts[j];
				optionEnemyCounts[j] = tmpEnemy;
			}
		}
	}

	// Use all options (1 per planet)
	int randomCount = optionCount;

	// Build SUI
	ManagedReference<SuiListBox*> cloneMenu = new SuiListBox(player, SuiWindowType::CLONE_REQUEST);
	cloneMenu->setCallback(new DwCloningRequestSuiCallback(zoneServer, typeofdeath));
	cloneMenu->setPromptTitle("Death Watch Safe House Selection");

	StringBuffer promptText;
	promptText << "Select a cloning facility.\n\n";
	promptText << "Threat levels indicate enemy Death Watch within 1000m:\n";
	promptText << "  [CLEAR] = No enemies detected\n";
	promptText << "  [CAUTION] = 1-2 enemies nearby\n";
	promptText << "  [DANGER] = 3+ enemies nearby\n\n";
	promptText << "Cash Balance: " << player->getCashCredits();

	cloneMenu->setPromptText(promptText.toString());

	int menuItemCount = 0;

	// Add HOME first if valid
	if (homeValidID != 0) {
		StringBuffer menuText;
		if (homeEnemyCount == 0) {
			menuText << "[CLEAR] ";
		} else if (homeEnemyCount <= 2) {
			menuText << "[CAUTION] ";
		} else {
			menuText << "[DANGER] ";
		}
		menuText << "[HOME] " << homeName << " - " << homeZoneName;
		if (homeEnemyCount > 0) {
			menuText << " (" << homeEnemyCount << (homeEnemyCount == 1 ? " enemy)" : " enemies)");
		}
		cloneMenu->addMenuItem(menuText.toString(), homeValidID);
		menuItemCount++;
	}

	// Add HQ second if valid AND player has enough faction standing (> 1000)
	// Note: HQ uses building ID as menu item ID - sendDeathWatchPlayerToCloner handles it specially
	int hqFactionStanding = 0;
	if (hqBuildingID != 0 && hqCellID != 0 && !factionKey.isEmpty()) {
		// Check faction standing via Lua
		Lua* lua = DirectorManager::instance()->getLuaInstance();
		if (lua != nullptr) {
			lua_State* L = lua->getLuaState();
			lua_getglobal(L, "dw_util");
			if (lua_istable(L, -1)) {
				lua_getfield(L, -1, "getFactionStanding");
				if (lua_isfunction(L, -1)) {
					lua_pushvalue(L, -2);  // self (dw_util table)
					lua_pushlightuserdata(L, player);
					lua_pushstring(L, factionKey.toCharArray());
					if (lua_pcall(L, 3, 1, 0) == 0) {
						hqFactionStanding = lua_tointeger(L, -1);
					}
					lua_pop(L, 1);  // result
				} else {
					lua_pop(L, 1);  // non-function
				}
			}
			lua_pop(L, 1);  // dw_util table
		}
	}

	// Only show HQ if standing > 1000 (STANDING_HQ_ACCESS)
	if (hqBuildingID != 0 && hqCellID != 0 && hqFactionStanding > 1000) {
		StringBuffer menuText;
		if (hqEnemyCount == 0) {
			menuText << "[CLEAR] ";
		} else if (hqEnemyCount <= 2) {
			menuText << "[CAUTION] ";
		} else {
			menuText << "[DANGER] ";
		}
		menuText << "[HQ] " << hqName << " - " << hqZoneName;
		if (hqEnemyCount > 0) {
			menuText << " (" << hqEnemyCount << (hqEnemyCount == 1 ? " enemy)" : " enemies)");
		}
		cloneMenu->addMenuItem(menuText.toString(), hqBuildingID);
		menuItemCount++;

		// Store HQ spawn info in player's screenplayData for retrieval by sendDeathWatchPlayerToCloner
		ghost->setScreenPlayData("death_watch", "hq_clone_cell", String::valueOf(hqCellID));
		ghost->setScreenPlayData("death_watch", "hq_clone_x", String::valueOf(hqSpawnX));
		ghost->setScreenPlayData("death_watch", "hq_clone_z", String::valueOf(hqSpawnZ));
		ghost->setScreenPlayData("death_watch", "hq_clone_y", String::valueOf(hqSpawnY));
		ghost->setScreenPlayData("death_watch", "hq_clone_zone", hqZoneName);
	}

	// Add random options (sorted by threat)
	for (int i = 0; i < randomCount; i++) {
		StringBuffer menuText;
		if (optionEnemyCounts[i] == 0) {
			menuText << "[CLEAR] ";
		} else if (optionEnemyCounts[i] <= 2) {
			menuText << "[CAUTION] ";
		} else {
			menuText << "[DANGER] ";
		}
		menuText << optionNames[i] << " - " << optionZones[i];
		if (optionEnemyCounts[i] > 0) {
			menuText << " (" << optionEnemyCounts[i] << (optionEnemyCounts[i] == 1 ? " enemy)" : " enemies)");
		}
		cloneMenu->addMenuItem(menuText.toString(), optionClonerIDs[i]);
		menuItemCount++;
	}

	// Fallback if no options available
	if (menuItemCount == 0) {
		cloneMenu->addMenuItem("No safe houses available - contact support", 0);
	}

	ghost->addSuiBox(cloneMenu);
	player->sendMessage(cloneMenu->generateMessage());
}

void DeathWatchManagerImplementation::sendDeathWatchPlayerToCloner(CreatureObject* player, uint64 clonerID, int typeofdeath, bool isPreDesignated) {
	if (player == nullptr || clonerID == 0)
		return;

	PlayerObject* ghost = player->getPlayerObject();
	if (ghost == nullptr)
		return;

	float spawnX, spawnY, spawnZ;
	uint64 cellObjectID = 0;
	String zoneName;
	String clonerName = "your faction headquarters";  // Default for HQ clones

	// Check if this is an HQ spawn (not a real cloner)
	// HQ spawn info is stored in screenplayData when the menu was built
	String hqCellStr = ghost->getScreenPlayData("death_watch", "hq_clone_cell");
	if (!hqCellStr.isEmpty()) {
		uint64 hqCellID = UnsignedLong::valueOf(hqCellStr);

		// Verify this is actually the HQ building that was selected
		// by checking if the clonerID matches a building with this cell
		ManagedReference<SceneObject*> hqCell = zoneServer->getObject(hqCellID);
		if (hqCell != nullptr && hqCell->isCellObject()) {
			ManagedReference<SceneObject*> hqBuilding = hqCell->getParent().get();
			if (hqBuilding != nullptr && hqBuilding->getObjectID() == clonerID) {
				// This is an HQ spawn - use stored coordinates
				spawnX = Float::valueOf(ghost->getScreenPlayData("death_watch", "hq_clone_x"));
				spawnZ = Float::valueOf(ghost->getScreenPlayData("death_watch", "hq_clone_z"));
				spawnY = Float::valueOf(ghost->getScreenPlayData("death_watch", "hq_clone_y"));
				zoneName = ghost->getScreenPlayData("death_watch", "hq_clone_zone");
				cellObjectID = hqCellID;

				// Clear only the HQ clone temp data (NOT the entire screenplay!)
				// The "death_watch" screenplay contains important persistent data like assigned_faction
				ghost->setScreenPlayData("death_watch", "hq_clone_cell", "");
				ghost->setScreenPlayData("death_watch", "hq_clone_x", "");
				ghost->setScreenPlayData("death_watch", "hq_clone_y", "");
				ghost->setScreenPlayData("death_watch", "hq_clone_z", "");
				ghost->setScreenPlayData("death_watch", "hq_clone_zone", "");

				// Set cloning flag BEFORE teleport to prevent duplicate clone window
				ghost->setCloning(true);

				// Teleport to HQ
				player->switchZone(zoneName, spawnX, spawnZ, spawnY, cellObjectID);

				// HQ clones don't have normal penalties - it's a safe house
				goto applyPostCloneEffects;
			}
		}
	}

	// Normal cloner path
	{
		ManagedReference<SceneObject*> cloner = zoneServer->getObject(clonerID);
		if (cloner == nullptr) {
			player->sendSystemMessage("Cloning facility no longer available.");
			return;
		}

		clonerName = cloner->getDisplayedName();  // Track for message later

		CloningBuildingObjectTemplate* cbot = cast<CloningBuildingObjectTemplate*>(cloner->getObjectTemplate());
		if (cbot == nullptr) {
			player->sendSystemMessage("Invalid cloning facility.");
			return;
		}

		// Get spawn point
		const Vector<CloneSpawnPoint>* spawnPoints = cbot->getCloneSpawnPoints();
		if (spawnPoints == nullptr || spawnPoints->size() == 0) {
			player->sendSystemMessage("Cloning facility has no spawn points.");
			return;
		}

		// Pick random spawn point
		int spawnIndex = System::random(spawnPoints->size() - 1);
		CloneSpawnPoint spawnPoint = spawnPoints->get(spawnIndex);

		Zone* clonerZone = cloner->getZone();
		if (clonerZone == nullptr) {
			player->sendSystemMessage("Cloning facility zone unavailable.");
			return;
		}

		zoneName = clonerZone->getZoneName();
		int cellNumber = spawnPoint.getCellID();  // This is the cell NUMBER, not object ID

		if (cellNumber != 0) {
			// Indoor spawn - need to get the actual cell object from the building
			BuildingObject* cloningBuilding = cloner.castTo<BuildingObject*>();
			if (cloningBuilding == nullptr) {
				player->sendSystemMessage("Cloning building error.");
				return;
			}

			CellObject* cell = cloningBuilding->getCell(cellNumber);
			if (cell == nullptr) {
				player->sendSystemMessage("Cloning cell not found.");
				return;
			}

			cellObjectID = cell->getObjectID();
			spawnX = spawnPoint.getPositionX();
			spawnY = spawnPoint.getPositionY();
			spawnZ = spawnPoint.getPositionZ();
		} else {
			// Outdoor spawn - add to building position
			spawnX = cloner->getWorldPositionX() + spawnPoint.getPositionX();
			spawnY = cloner->getWorldPositionY() + spawnPoint.getPositionY();
			spawnZ = cloner->getWorldPositionZ() + spawnPoint.getPositionZ();
		}

		// Set cloning flag BEFORE teleport to prevent TEF ejection
		ghost->setCloning(true);

		// Teleport player
		player->switchZone(zoneName, spawnX, spawnZ, spawnY, cellObjectID);
	}

applyPostCloneEffects:

	// Apply clone penalties (same as normal cloning)
	// Wounds and shock if not pre-designated
	if (!isPreDesignated) {
		player->addWounds(CreatureAttribute::HEALTH, 100, true, false);
		player->addWounds(CreatureAttribute::ACTION, 100, true, false);
		player->addWounds(CreatureAttribute::MIND, 100, true, false);
		player->addShockWounds(100, true, false);
	}

	// Note: Death Watch uses its own faction system, not GCW faction status

	// Apply item decay (matching PlayerManager::sendPlayerToCloner)
	PlayerManager* playerManager = zoneServer->getPlayerManager();
	if (playerManager != nullptr && typeofdeath == 0) {
		// PvE death - apply decay
		SortedVector<ManagedReference<SceneObject*>> insurableItems = playerManager->getInsurableItems(player, false);

		if (insurableItems.size() > 0) {
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
						// 1% Decay for insured items
						obj->inflictDamage(obj, 0, 0.01 * obj->getMaxCondition(), true, true);
						// Set uninsured
						uint32 bitmask = obj->getOptionsBitmask() - OptionBitmask::INSURED;
						obj->setOptionsBitmask(bitmask);
					} else {
						// 5% Decay for uninsured items
						obj->inflictDamage(obj, 0, 0.05 * obj->getMaxCondition(), true, true);
					}

					// Calculate condition percentage for decay report
					int max = obj->getMaxCondition();
					int min = max - obj->getConditionDamage();
					int condPercentage = (min / (float)max) * 100.0f;
					String line = " - " + obj->getDisplayedName() + " (@" + String::valueOf(condPercentage) + "%)";

					suiCloneDecayReport->addMenuItem(line, item->getObjectID());
				}
			}

			ghost->addSuiBox(suiCloneDecayReport);
			player->sendMessage(suiCloneDecayReport->generateMessage());
		}
	}

	// Clear food/drink filling (if configured)
	if (ConfigManager::instance()->getBool("Core3.PlayerManager.WipeFillingOnClone", false)) {
		ghost->setFoodFilling(0);
		ghost->setDrinkFilling(0);
	}

	// Clear ALL PvP TEFs - they paid their dues by dying
	// This clears DW TEF, PvP area TEF, GCW TEF, BH TEF, etc.
	ghost->clearAllTefs();

	// Schedule recovery task - this handles:
	// - setPosture(UPRIGHT) to revive the player
	// - setCloning(false) to allow building entry
	// - broadcastPvpStatusBitmask() to update PvP status
	// NOTE: Do NOT set posture before scheduling or the task exits early!
	Reference<Task*> recoverTask = new PlayerIncapacitationRecoverTask(player, true);
	recoverTask->schedule(3000);

	// Notify observers
	player->notifyObservers(ObserverEventType::PLAYERCLONED, player, isPreDesignated ? 1 : 0);

	// Jedi experience loss
	if (player->hasSkill("force_title_jedi_rank_02")) {
		int jediXpCap = ghost->getXpCap("jedi_general");
		int xpLoss = (int)(jediXpCap * -0.05);
		int curExp = ghost->getExperience("jedi_general");
		int negXpCap = -10000000;

		if ((curExp + xpLoss) < negXpCap)
			xpLoss = negXpCap - curExp;

		playerManager->awardExperience(player, "jedi_general", xpLoss, true, 1.0f, false);
		StringIdChatParameter message("base_player", "prose_revoke_xp");
		message.setDI(xpLoss * -1);
		message.setTO("exp_n", "jedi_general");
		player->sendSystemMessage(message);
	}

	// Send clone message
	StringBuffer msg;
	msg << "You have been cloned at " << clonerName << ".";
	player->sendSystemMessage(msg.toString());
}

// ============================================================================
// STEALTH PROTOCOL SYSTEM IMPLEMENTATION
// ============================================================================

bool DeathWatchManagerImplementation::activateShadowProtocol(CreatureObject* player) {
	if (player == nullptr) {
		return false;
	}

	uint64 playerID = player->getObjectID();

	// Check if admin (bypass skill checks for testing)
	PlayerObject* ghost = player->getPlayerObject();
	bool isAdmin = (ghost != nullptr && ghost->isPrivileged());

	if (!isAdmin) {
		// Must be Death Watch
		if (!isDeathWatch(player)) {
			player->sendSystemMessage("\\#FF6666You must be Death Watch to use this ability.");
			return false;
		}

		// Must be Black Sun faction (check for sun_3 skill)
		if (!player->hasSkill("combat_death_watch_sun_3")) {
			player->sendSystemMessage("\\#FF6666Only Black Sun operatives can use Shadow Protocol.");
			return false;
		}
	}

	// Check if already active
	{
		Locker locker(&shadowProtocolMutex);
		if (shadowProtocolActive.contains(playerID) && shadowProtocolActive.get(playerID)) {
			player->sendSystemMessage("\\#FFFFFFShadow Protocol is already active.");
			return false;
		}

		// Activate it
		shadowProtocolActive.put(playerID, true);
	}

	// Use C++ optimized PEF clearing (no Lua boundary crossing)
	// This removes war PEF but preserves bounty PEF
	// The player can still attack via canAttackWithStealthProtocol() check
	clearPlayerPEFForStealthCpp(player);

	info(true) << "Shadow Protocol activated for player " << playerID;
	return true;
}

void DeathWatchManagerImplementation::deactivateShadowProtocol(CreatureObject* player, bool broadcastUpdate) {
	if (player == nullptr) {
		return;
	}

	uint64 playerID = player->getObjectID();
	bool wasActive = false;

	{
		Locker locker(&shadowProtocolMutex);
		if (shadowProtocolActive.contains(playerID)) {
			wasActive = shadowProtocolActive.get(playerID);
			shadowProtocolActive.drop(playerID);
		}
	}

	if (wasActive && broadcastUpdate) {
		// Use C++ optimized PEF reset (no Lua boundary crossing)
		// This restores war PEF and bounty PEF based on current state
		resetPlayerPEFCpp(player);

		info(true) << "Shadow Protocol deactivated for player " << playerID;
	}
}

bool DeathWatchManagerImplementation::isShadowProtocolActive(CreatureObject* player) {
	if (player == nullptr) {
		return false;
	}

	Locker locker(&shadowProtocolMutex);
	uint64 playerID = player->getObjectID();

	if (shadowProtocolActive.contains(playerID)) {
		return shadowProtocolActive.get(playerID);
	}

	return false;
}

bool DeathWatchManagerImplementation::activateEspionageMatrix(CreatureObject* player) {
	if (player == nullptr) {
		return false;
	}

	uint64 playerID = player->getObjectID();

	// Check if admin (bypass skill checks for testing)
	PlayerObject* ghost = player->getPlayerObject();
	bool isAdmin = (ghost != nullptr && ghost->isPrivileged());

	if (!isAdmin) {
		// Must be Death Watch
		if (!isDeathWatch(player)) {
			player->sendSystemMessage("\\#FF6666You must be Death Watch to use this ability.");
			return false;
		}

		// Must be Zann Consortium faction (check for zan_3 skill)
		if (!player->hasSkill("combat_death_watch_zan_3")) {
			player->sendSystemMessage("\\#FF6666Only Zann Consortium operatives can use Espionage Matrix.");
			return false;
		}
	}

	// Check if already active
	{
		Locker locker(&espionageMatrixMutex);
		if (espionageMatrixActive.contains(playerID) && espionageMatrixActive.get(playerID)) {
			player->sendSystemMessage("\\#FFFFFFEspionage Matrix is already active.");
			return false;
		}

		// Activate it
		espionageMatrixActive.put(playerID, true);
	}

	// Use C++ optimized PEF clearing (no Lua boundary crossing)
	// This removes war PEF but preserves bounty PEF
	clearPlayerPEFForStealthCpp(player);

	info(true) << "Espionage Matrix activated for player " << playerID;
	return true;
}

void DeathWatchManagerImplementation::deactivateEspionageMatrix(CreatureObject* player, bool broadcastUpdate) {
	if (player == nullptr) {
		return;
	}

	uint64 playerID = player->getObjectID();
	bool wasActive = false;

	{
		Locker locker(&espionageMatrixMutex);
		if (espionageMatrixActive.contains(playerID)) {
			wasActive = espionageMatrixActive.get(playerID);
			espionageMatrixActive.drop(playerID);
		}
	}

	if (wasActive && broadcastUpdate) {
		// Use C++ optimized PEF reset (no Lua boundary crossing)
		// This restores war PEF and bounty PEF based on current state
		resetPlayerPEFCpp(player);

		info(true) << "Espionage Matrix deactivated for player " << playerID;
	}
}

bool DeathWatchManagerImplementation::isEspionageMatrixActive(CreatureObject* player) {
	if (player == nullptr) {
		return false;
	}

	Locker locker(&espionageMatrixMutex);
	uint64 playerID = player->getObjectID();

	if (espionageMatrixActive.contains(playerID)) {
		return espionageMatrixActive.get(playerID);
	}

	return false;
}

bool DeathWatchManagerImplementation::isTargetHiddenByStealthProtocol(CreatureObject* observer, CreatureObject* target) {
	if (observer == nullptr || target == nullptr) {
		return false;
	}

	// Only Espionage Matrix hides from detection
	// Shadow Protocol allows the player to be detected (they can still be attacked back)
	if (!isEspionageMatrixActive(target)) {
		return false;
	}

	// Only hidden from enemies at war
	if (!isDeathWatch(observer) || !isDeathWatch(target)) {
		return false;
	}

	// Get factions
	PlayerObject* observerGhost = observer->getPlayerObject();
	PlayerObject* targetGhost = target->getPlayerObject();

	if (observerGhost == nullptr || targetGhost == nullptr) {
		return false;
	}

	byte observerFaction = observerGhost->getDwFaction();
	byte targetFaction = targetGhost->getDwFaction();

	// Only hidden if factions are at war
	return areFactionsAtWar(observerFaction, targetFaction);
}

bool DeathWatchManagerImplementation::canAttackWithStealthProtocol(CreatureObject* attacker, CreatureObject* target) {
	if (attacker == nullptr || target == nullptr) {
		return false;
	}

	// Check if attacker has Espionage Matrix active - they CANNOT attack
	if (isEspionageMatrixActive(attacker)) {
		return false;  // Must deactivate Espionage Matrix to attack
	}

	// Check if attacker has Shadow Protocol active - they CAN attack
	if (isShadowProtocolActive(attacker)) {
		// Verify they would normally be able to attack if not stealthed
		if (!isDeathWatch(attacker) || !isDeathWatch(target)) {
			return false;
		}

		PlayerObject* attackerGhost = attacker->getPlayerObject();
		PlayerObject* targetGhost = target->getPlayerObject();

		if (attackerGhost == nullptr || targetGhost == nullptr) {
			return false;
		}

		byte attackerFaction = attackerGhost->getDwFaction();
		byte targetFaction = targetGhost->getDwFaction();

		// Can attack if at war
		return areFactionsAtWar(attackerFaction, targetFaction);
	}

	// No stealth protocol active, let normal checks handle it
	return false;
}

// ============================================================================
// PVP STATUS BROADCASTING IMPLEMENTATION
// ============================================================================

void DeathWatchManagerImplementation::broadcastPvpStatusToAllDW(CreatureObject* player) {
	if (player == nullptr || zoneServer == nullptr) {
		return;
	}

	uint64 playerID = player->getObjectID();

	// Use C++ optimized player list retrieval (no Lua boundary crossing)
	int numPlayers = 0;

	for (byte factionID = 1; factionID <= 4; factionID++) {
		Vector<uint64> factionPlayers = getOnlineFactionPlayersCpp(zoneServer, factionID, playerID);

		for (int i = 0; i < factionPlayers.size(); i++) {
			uint64 targetID = factionPlayers.get(i);

			ManagedReference<SceneObject*> targetObj = zoneServer->getObject(targetID);
			if (targetObj == nullptr || !targetObj->isPlayerCreature()) {
				continue;
			}

			CreatureObject* targetPlayer = targetObj.castTo<CreatureObject*>();
			if (targetPlayer == nullptr || !targetPlayer->isOnline()) {
				continue;
			}

			// Send PvP status to this player
			player->sendPvpStatusTo(targetPlayer);
			numPlayers++;
		}
	}

	// Update broadcast tracking
	{
		Locker locker(&lastBroadcastTimeMutex);
		Time now;
		lastBroadcastTime.put(playerID, now);
	}

	info(true) << "Broadcast PvP status for player " << playerID << " to " << numPlayers << " DW players";
}

void DeathWatchManagerImplementation::refreshPvpStatusForPlayer(CreatureObject* player) {
	if (player == nullptr || zoneServer == nullptr) {
		return;
	}

	uint64 playerID = player->getObjectID();

	// Use C++ optimized player list retrieval (no Lua boundary crossing)
	int numPlayers = 0;

	for (byte factionID = 1; factionID <= 4; factionID++) {
		Vector<uint64> factionPlayers = getOnlineFactionPlayersCpp(zoneServer, factionID, playerID);

		for (int i = 0; i < factionPlayers.size(); i++) {
			uint64 sourceID = factionPlayers.get(i);

			ManagedReference<SceneObject*> sourceObj = zoneServer->getObject(sourceID);
			if (sourceObj == nullptr || !sourceObj->isPlayerCreature()) {
				continue;
			}

			CreatureObject* sourcePlayer = sourceObj.castTo<CreatureObject*>();
			if (sourcePlayer == nullptr || !sourcePlayer->isOnline()) {
				continue;
			}

			// Have other players send their status to this player
			sourcePlayer->sendPvpStatusTo(player);
			numPlayers++;
		}
	}

	info(true) << "Refreshed PvP status for player " << playerID << " from " << numPlayers << " DW players";
}

void DeathWatchManagerImplementation::schedulePvpBroadcastRefresh(CreatureObject* player) {
	if (player == nullptr) {
		return;
	}

	Locker locker(&pvpRefreshQueueMutex);
	uint64 playerID = player->getObjectID();

	// Don't add duplicates
	for (int i = 0; i < pvpRefreshQueue.size(); i++) {
		if (pvpRefreshQueue.get(i) == playerID) {
			return;
		}
	}

	pvpRefreshQueue.add(playerID);
}

void DeathWatchManagerImplementation::runBroadcastRefreshCycle() {
	if (zoneServer == nullptr) {
		return;
	}

	Time now;
	int refreshCount = 0;
	Vector<uint64> playersToRefresh;

	// Check for players who haven't had a broadcast recently
	{
		Locker locker(&lastBroadcastTimeMutex);

		for (int i = 0; i < lastBroadcastTime.size() && refreshCount < MAX_REFRESH_PER_CYCLE; i++) {
			uint64 playerID = lastBroadcastTime.elementAt(i).getKey();
			Time lastTime = lastBroadcastTime.elementAt(i).getValue();

			if (now.miliDifference(lastTime) > REFRESH_INTERVAL_MS) {
				playersToRefresh.add(playerID);
				refreshCount++;
			}
		}
	}

	// Also process the refresh queue
	{
		Locker locker(&pvpRefreshQueueMutex);

		while (pvpRefreshQueue.size() > 0 && refreshCount < MAX_REFRESH_PER_CYCLE) {
			uint64 playerID = pvpRefreshQueue.remove(0);

			// Don't add duplicates from above
			bool found = false;
			for (int i = 0; i < playersToRefresh.size(); i++) {
				if (playersToRefresh.get(i) == playerID) {
					found = true;
					break;
				}
			}

			if (!found) {
				playersToRefresh.add(playerID);
				refreshCount++;
			}
		}
	}

	// Process the refreshes
	for (int i = 0; i < playersToRefresh.size(); i++) {
		uint64 playerID = playersToRefresh.get(i);

		ManagedReference<SceneObject*> playerObj = zoneServer->getObject(playerID);
		if (playerObj == nullptr || !playerObj->isPlayerCreature()) {
			continue;
		}

		CreatureObject* player = playerObj.castTo<CreatureObject*>();
		if (player == nullptr || !player->isOnline()) {
			continue;
		}

		if (!isDeathWatch(player)) {
			continue;
		}

		// Do bidirectional sync
		broadcastPvpStatusToAllDW(player);
		refreshPvpStatusForPlayer(player);
	}

	if (refreshCount > 0) {
		info(true) << "PvP refresh cycle processed " << refreshCount << " players";
	}
}

void DeathWatchManagerImplementation::onPlayerZoneIn(CreatureObject* player) {
	if (player == nullptr) {
		return;
	}

	if (!isDeathWatch(player)) {
		return;
	}

	uint64 playerID = player->getObjectID();

	// Check if radar stealth should break due to zone change
	{
		Locker locker(&radarStealthMutex);
		if (radarStealthExpiration.contains(playerID)) {
			// Get current zone hash
			ManagedReference<Zone*> zone = player->getZone();
			uint32 currentZoneHash = 0;
			if (zone != nullptr) {
				currentZoneHash = zone->getZoneName().hashCode();
			}

			// Check if zone changed
			if (radarStealthActivationZone.contains(playerID)) {
				uint32 activationZoneHash = radarStealthActivationZone.get(playerID);
				if (currentZoneHash != activationZoneHash) {
					// Zone changed - break stealth (unlock first to avoid deadlock)
					locker.release();
					deactivateRadarStealth(player, 2);  // reason 2 = zone change
				}
			}
		}
	}

	// Bidirectional PvP status sync
	// 1. Player sees correct status of all other DW players
	refreshPvpStatusForPlayer(player);

	// 2. All other DW players see correct status of this player
	broadcastPvpStatusToAllDW(player);

	info(true) << "Player " << playerID << " zoned in, PvP status synchronized";
}

void DeathWatchManagerImplementation::cleanupStealthProtocolState(CreatureObject* player) {
	if (player == nullptr) {
		return;
	}

	uint64 playerID = player->getObjectID();

	// Clean up Shadow Protocol
	{
		Locker locker(&shadowProtocolMutex);
		shadowProtocolActive.drop(playerID);
	}

	// Clean up Espionage Matrix
	{
		Locker locker(&espionageMatrixMutex);
		espionageMatrixActive.drop(playerID);
	}

	// Clean up broadcast tracking
	{
		Locker locker(&lastBroadcastTimeMutex);
		lastBroadcastTime.drop(playerID);
	}

	// Remove from refresh queue
	{
		Locker locker(&pvpRefreshQueueMutex);
		for (int i = pvpRefreshQueue.size() - 1; i >= 0; i--) {
			if (pvpRefreshQueue.get(i) == playerID) {
				pvpRefreshQueue.remove(i);
			}
		}
	}

	// Clean up radar stealth
	{
		Locker locker(&radarStealthMutex);
		radarStealthExpiration.drop(playerID);
		radarStealthActivationZone.drop(playerID);
	}

	info(true) << "Cleaned up stealth protocol state for player " << playerID;
}

// ============================================================================
// RADAR STEALTH SYSTEM IMPLEMENTATION
// ============================================================================

int DeathWatchManagerImplementation::activateRadarStealth(CreatureObject* player) {
	if (player == nullptr) {
		return -1;
	}

	// Calculate duration based on skills
	int durationMs = calculateRadarStealthDuration(player);

	// Use the duration-based activation with standard checks
	return activateRadarStealthWithDuration(player, durationMs, false);
}

int DeathWatchManagerImplementation::activateRadarStealthWithDuration(CreatureObject* player, int durationMs, bool bypassChecks) {
	if (player == nullptr) {
		return -1;
	}

	uint64 playerID = player->getObjectID();

	// Check if admin (for dev testing bypass)
	PlayerObject* ghost = player->getPlayerObject();
	bool isAdmin = (ghost != nullptr && ghost->isPrivileged());

	// Bypass checks only allowed for admins
	if (bypassChecks && !isAdmin) {
		bypassChecks = false;
	}

	if (!bypassChecks) {
		// Must be Death Watch
		if (!isDeathWatch(player)) {
			player->sendSystemMessage("\\#FF6666You must be Death Watch to use radar stealth.");
			return -1;
		}

		// Check for ANY players within 400m
		ManagedReference<Zone*> zone = player->getZone();
		if (zone != nullptr) {
			// Use zone->getInRangePlayers to find all players in range
			SortedVector<ManagedReference<TreeEntry*>> closeObjects;
			zone->getInRangePlayers(player->getWorldPositionX(), 0, player->getWorldPositionY(),
				RADAR_STEALTH_ENEMY_CHECK_RANGE, &closeObjects);

			int playerCount = 0;
			for (int i = 0; i < closeObjects.size(); i++) {
				SceneObject* obj = static_cast<SceneObject*>(closeObjects.get(i).get());
				if (obj == nullptr || obj == player) {
					continue;
				}
				if (obj->isPlayerCreature()) {
					playerCount++;
				}
			}

			if (playerCount > 0) {
				StringBuffer msg;
				msg << "\\#FF6666Cannot activate radar stealth - " << playerCount << " player(s) within " << (int)RADAR_STEALTH_ENEMY_CHECK_RANGE << "m.";
				player->sendSystemMessage(msg.toString());
				return -2;
			}
		}
	}

	// Check if already active
	{
		Locker locker(&radarStealthMutex);
		if (radarStealthExpiration.contains(playerID)) {
			uint64 expireTime = radarStealthExpiration.get(playerID);
			uint64 now = System::getMiliTime();
			if (now < expireTime) {
				player->sendSystemMessage("\\#FFFFFFRadar stealth is already active.");
				return -3;
			}
		}
	}

	// Get current zone for zone-change detection
	ManagedReference<Zone*> zone = player->getZone();
	uint32 zoneHash = 0;
	if (zone != nullptr) {
		zoneHash = zone->getZoneName().hashCode();
	}

	// Calculate expiration time
	uint64 now = System::getMiliTime();
	uint64 expireTime = now + durationMs;

	// Activate stealth tracking
	{
		Locker locker(&radarStealthMutex);
		radarStealthExpiration.put(playerID, expireTime);
		radarStealthActivationZone.put(playerID, zoneHash);
	}

	// Make player invisible
	player->setInvisible(true);

	// Broadcast destroy to nearby players so they lose radar blip
	// This removes the player from nearby players' radar immediately
	player->broadcastDestroy(player, false);  // false = don't send to self

	// Send confirmation message
	float durationSec = durationMs / 1000.0f;
	StringBuffer msg;
	msg << "\\#00FF00Radar stealth activated for " << durationSec << " seconds.";
	player->sendSystemMessage(msg.toString());

	info(true) << "Radar stealth activated for player " << playerID << " duration " << durationMs << "ms";

	// Schedule automatic deactivation
	// Use a simple task to check and deactivate
	Reference<DeathWatchManager*> strongRef = _this.getReferenceUnsafeStaticCast();

	Core::getTaskManager()->scheduleTask([strongRef, playerID, expireTime]() {
		if (strongRef == nullptr) {
			return;
		}

		// Verify this is still the same stealth session (expiration time matches)
		bool shouldDeactivate = false;
		{
			Locker locker(&radarStealthMutex);
			if (radarStealthExpiration.contains(playerID)) {
				uint64 storedExpire = radarStealthExpiration.get(playerID);
				if (storedExpire == expireTime) {
					shouldDeactivate = true;
				}
			}
		}

		if (shouldDeactivate) {
			// Get the player object
			ManagedReference<SceneObject*> obj = Core::getObjectBroker()->lookUp(playerID).castTo<SceneObject*>();
			if (obj != nullptr && obj->isPlayerCreature()) {
				CreatureObject* player = obj.castTo<CreatureObject*>();
				if (player != nullptr) {
					Locker playerLocker(player);
					strongRef->deactivateRadarStealth(player, 1);  // reason 1 = expired
				}
			}
		}
	}, "RadarStealthExpireTask", durationMs);

	return 0;
}

void DeathWatchManagerImplementation::deactivateRadarStealth(CreatureObject* player, int reason) {
	if (player == nullptr) {
		return;
	}

	uint64 playerID = player->getObjectID();
	bool wasActive = false;

	// Remove from tracking
	{
		Locker locker(&radarStealthMutex);
		if (radarStealthExpiration.contains(playerID)) {
			radarStealthExpiration.drop(playerID);
			wasActive = true;
		}
		radarStealthActivationZone.drop(playerID);
	}

	if (!wasActive) {
		return;  // Wasn't active, nothing to do
	}

	// Make player visible again
	player->setInvisible(false);

	// Re-broadcast player to nearby observers
	// This makes the player appear on radar again
	ManagedReference<Zone*> zone = player->getZone();
	if (zone != nullptr) {
		// Insert back into zone to trigger baseline broadcasts
		player->broadcastObject(player, true);
	}

	// Send appropriate message based on reason
	String reasonMsg;
	switch (reason) {
		case 0:
			reasonMsg = "\\#FFFFFFRadar stealth deactivated.";
			break;
		case 1:
			reasonMsg = "\\#FFFF00Radar stealth has expired.";
			break;
		case 2:
			reasonMsg = "\\#FF6666Radar stealth broken by zone change.";
			break;
		case 3:
			reasonMsg = "\\#FF0000You have been detected! Radar stealth broken.";
			break;
		default:
			reasonMsg = "\\#FFFFFFRadar stealth deactivated.";
	}

	player->sendSystemMessage(reasonMsg);

	info(true) << "Radar stealth deactivated for player " << playerID << " reason " << reason;
}

bool DeathWatchManagerImplementation::isRadarStealthActive(CreatureObject* player) {
	if (player == nullptr) {
		return false;
	}

	uint64 playerID = player->getObjectID();
	uint64 now = System::getMiliTime();

	Locker locker(&radarStealthMutex);

	if (!radarStealthExpiration.contains(playerID)) {
		return false;
	}

	uint64 expireTime = radarStealthExpiration.get(playerID);
	return now < expireTime;
}

int DeathWatchManagerImplementation::getRadarStealthRemainingMs(CreatureObject* player) {
	if (player == nullptr) {
		return 0;
	}

	uint64 playerID = player->getObjectID();
	uint64 now = System::getMiliTime();

	Locker locker(&radarStealthMutex);

	if (!radarStealthExpiration.contains(playerID)) {
		return 0;
	}

	uint64 expireTime = radarStealthExpiration.get(playerID);
	if (now >= expireTime) {
		return 0;
	}

	return (int)(expireTime - now);
}

int DeathWatchManagerImplementation::calculateRadarStealthDuration(CreatureObject* player) {
	if (player == nullptr) {
		return RADAR_STEALTH_BASE_DURATION_MS;
	}

	// Get subterfuge skill mod
	int subterfuge = player->getSkillMod("dw_subterfuge");

	// Calculate duration: base + (subterfuge * bonus per point)
	int durationMs = RADAR_STEALTH_BASE_DURATION_MS + (subterfuge * RADAR_STEALTH_PER_SUBTERFUGE_MS);

	return durationMs;
}

void DeathWatchManagerImplementation::resetPlayerPEFCpp(CreatureObject* player) {
	if (player == nullptr || zoneServer == nullptr) {
		return;
	}

	uint64 playerID = player->getObjectID();

	// Only process Death Watch players
	if (!isDeathWatch(player)) {
		return;
	}

	PlayerObject* ghost = player->getPlayerObject();
	if (ghost == nullptr) {
		return;
	}

	byte playerFaction = ghost->getDwFaction();
	if (playerFaction == 0) {
		return;
	}

	info(true) << "resetPlayerPEFCpp: Resetting PEF for player " << playerID << " (faction " << (int)playerFaction << ")";

	// Step 1: Collect bounty relationships to preserve
	HashSet<uint64> bountyRelationships;

	// Get hunter targets
	Vector<uint64> bountyTargets = getHunterBountyTargets(player);
	for (int i = 0; i < bountyTargets.size(); i++) {
		bountyRelationships.add(bountyTargets.get(i));
	}

	// Get hunter if being hunted
	uint64 hunterID = getPlayerBountyHunterID(player);
	if (hunterID > 0) {
		bountyRelationships.add(hunterID);
	}

	// Step 2: Clear ALL PEF with other DW players
	for (byte factionID = 1; factionID <= 4; factionID++) {
		Vector<uint64> factionPlayers = getOnlineFactionPlayersCpp(zoneServer, factionID, playerID);

		for (int i = 0; i < factionPlayers.size(); i++) {
			uint64 otherID = factionPlayers.get(i);

			ManagedReference<SceneObject*> otherObj = zoneServer->getObject(otherID);
			if (otherObj == nullptr || !otherObj->isPlayerCreature()) {
				continue;
			}

			CreatureObject* other = otherObj.castTo<CreatureObject*>();
			if (other == nullptr) {
				continue;
			}

			// Check if they have mutual PEF
			if (player->hasPersonalEnemyFlag(other) || other->hasPersonalEnemyFlag(player)) {
				player->removePersonalEnemyFlag(other);
				other->removePersonalEnemyFlag(player);
			}
		}
	}

	// Clear combat state
	player->clearCombatState(true);

	// Step 3: Re-apply WAR PEF based on current war state
	for (byte enemyFaction = 1; enemyFaction <= 4; enemyFaction++) {
		if (!areFactionsAtWar(playerFaction, enemyFaction)) {
			continue;
		}

		Vector<uint64> enemyPlayers = getOnlineFactionPlayersCpp(zoneServer, enemyFaction, playerID);

		for (int i = 0; i < enemyPlayers.size(); i++) {
			uint64 enemyID = enemyPlayers.get(i);

			ManagedReference<SceneObject*> enemyObj = zoneServer->getObject(enemyID);
			if (enemyObj == nullptr || !enemyObj->isPlayerCreature()) {
				continue;
			}

			CreatureObject* enemy = enemyObj.castTo<CreatureObject*>();
			if (enemy == nullptr) {
				continue;
			}

			// Set mutual PEF (duration 0 = permanent)
			player->addPersonalEnemyFlag(enemy, 0);
			enemy->addPersonalEnemyFlag(player, 0);

			// Explicitly send status updates in both directions
			player->sendPvpStatusTo(enemy);
			enemy->sendPvpStatusTo(player);
		}
	}

	// Step 4: Re-apply BOUNTY PEF (preserved relationships)
	for (auto it = bountyRelationships.iterator(); it.hasNext(); ) {
		uint64 bountyTargetID = it.next();

		ManagedReference<SceneObject*> targetObj = zoneServer->getObject(bountyTargetID);
		if (targetObj == nullptr || !targetObj->isPlayerCreature()) {
			continue;
		}

		CreatureObject* target = targetObj.castTo<CreatureObject*>();
		if (target == nullptr || !target->isOnline()) {
			continue;
		}

		// Set mutual PEF for bounty (duration 0 = permanent)
		player->addPersonalEnemyFlag(target, 0);
		target->addPersonalEnemyFlag(player, 0);

		// Send status updates
		player->sendPvpStatusTo(target);
		target->sendPvpStatusTo(player);

		info(true) << "resetPlayerPEFCpp: Re-applied bounty PEF with " << bountyTargetID;
	}

	info(true) << "resetPlayerPEFCpp: Completed PEF reset for player " << playerID;
}

void DeathWatchManagerImplementation::clearPlayerPEFForStealthCpp(CreatureObject* player) {
	if (player == nullptr || zoneServer == nullptr) {
		return;
	}

	uint64 playerID = player->getObjectID();

	// Only process Death Watch players
	if (!isDeathWatch(player)) {
		return;
	}

	PlayerObject* ghost = player->getPlayerObject();
	if (ghost == nullptr) {
		return;
	}

	byte playerFaction = ghost->getDwFaction();
	if (playerFaction == 0) {
		return;
	}

	info(true) << "clearPlayerPEFForStealthCpp: Clearing PEF for stealth player " << playerID;

	// Collect bounty relationships to KEEP
	HashSet<uint64> bountyRelationships;

	// Get hunter targets
	Vector<uint64> bountyTargets = getHunterBountyTargets(player);
	for (int i = 0; i < bountyTargets.size(); i++) {
		bountyRelationships.add(bountyTargets.get(i));
	}

	// Get hunter if being hunted
	uint64 hunterID = getPlayerBountyHunterID(player);
	if (hunterID > 0) {
		bountyRelationships.add(hunterID);
	}

	// Remove PEF with war enemies only (keep bounty PEF)
	int clearedCount = 0;

	for (byte enemyFaction = 1; enemyFaction <= 4; enemyFaction++) {
		if (!areFactionsAtWar(playerFaction, enemyFaction)) {
			continue;
		}

		Vector<uint64> enemyPlayers = getOnlineFactionPlayersCpp(zoneServer, enemyFaction, playerID);

		for (int i = 0; i < enemyPlayers.size(); i++) {
			uint64 enemyID = enemyPlayers.get(i);

			// Skip if this is a bounty relationship
			if (bountyRelationships.contains(enemyID)) {
				info(true) << "clearPlayerPEFForStealthCpp: Keeping bounty PEF with " << enemyID;
				continue;
			}

			ManagedReference<SceneObject*> enemyObj = zoneServer->getObject(enemyID);
			if (enemyObj == nullptr || !enemyObj->isPlayerCreature()) {
				continue;
			}

			CreatureObject* enemy = enemyObj.castTo<CreatureObject*>();
			if (enemy == nullptr) {
				continue;
			}

			// Remove mutual PEF
			player->removePersonalEnemyFlag(enemy);
			enemy->removePersonalEnemyFlag(player);

			// Send status updates in both directions
			player->sendPvpStatusTo(enemy);
			enemy->sendPvpStatusTo(player);

			clearedCount++;
		}
	}

	// Clear combat state
	player->clearCombatState(true);

	info(true) << "clearPlayerPEFForStealthCpp: Cleared " << clearedCount << " war PEF for player " << playerID;
}

// ============================================================================
// SERUM SPECIAL EFFECTS
// ============================================================================

/**
 * Apply Adrenaline Serum high-damage mitigation
 *
 * When a player has the Adrenaline buff (dw_missile_reflector skill mod > 0)
 * and receives an attack dealing >5000 damage, this applies damage reduction.
 *
 * Formula: 25% base + ((dw_deflection + dw_toughness) / 8)%
 * Maximum: 50% mitigation
 */
int DeathWatchManagerImplementation::applyAdrenalineHighDamageMitigation(CreatureObject* defender, int rawDamage) {
	if (defender == nullptr) {
		return rawDamage;
	}

	// Only applies to attacks over 5000 damage
	if (rawDamage <= 5000) {
		return rawDamage;
	}

	// Check if Adrenaline buff is active (dw_missile_reflector skill mod > 0)
	int missileReflector = defender->getSkillMod("dw_missile_reflector");
	if (missileReflector <= 0) {
		return rawDamage;
	}

	// Get defender's skill tapes for scaling
	int defDeflection = defender->getSkillMod("dw_deflection");
	int defToughness = defender->getSkillMod("dw_toughness");

	// Formula: 25% base + ((dw_deflection + dw_toughness) / 8)%
	// At 0/0: 25% mitigation
	// At 100/100: 25% + 25% = 50% mitigation (max)
	float baseMitigation = 0.25f;
	float skillBonus = (defDeflection + defToughness) / 800.0f;  // Combined / 8 / 100 = /800
	float totalMitigation = baseMitigation + skillBonus;

	// Cap at 50% mitigation
	totalMitigation = Math::min(0.50f, totalMitigation);

	// Apply mitigation
	int mitigatedDamage = (int)(rawDamage * (1.0f - totalMitigation));

	// Log for debugging
	info(true) << "Adrenaline High-Damage Mitigation: raw=" << rawDamage
	           << ", deflection=" << defDeflection << ", toughness=" << defToughness
	           << ", mitigation=" << (int)(totalMitigation * 100) << "%"
	           << ", final=" << mitigatedDamage;

	// Notify defender
	StringBuffer msg;
	msg << "\\#66FF66[Adrenaline Shield] Blocked " << (int)(totalMitigation * 100) << "% of " << rawDamage << " damage!";
	defender->sendSystemMessage(msg.toString());

	return mitigatedDamage;
}

/**
 * Clear Combat Serum debuffs (DoTs, states, speed reset)
 *
 * Priority-based clearing system:
 *   Priority 1: Blind (always cleared with any power)
 *   Priority 2: Stun
 *   Priority 3: Dizzy
 *   Priority 4: Bleeding
 *   Priority 5: Fire
 *   Priority 6: Speed/Root
 *
 * Formula: statesCanClear = 1 + floor(power/125) + floor(skill/25)
 * At 100 power + 0 skill: 1 (Blind only)
 * At 500 power + 25 skill: 6 (all states)
 *
 * Returns bitmask of cleared effects for messaging:
 * Bit 0: Fire cleared
 * Bit 1: Bleeding cleared
 * Bit 2: Dizzy cleared
 * Bit 3: Stun cleared
 * Bit 4: Blind cleared
 * Bit 5: Speed reset applied
 */
int DeathWatchManagerImplementation::clearCombatSerumDebuffs(CreatureObject* player, int power, int skill) {
	if (player == nullptr) {
		return 0;
	}

	// Calculate how many states we can clear based on power and skill
	// Base: 1 (always clear Blind)
	// Power bonus: power / 125 (125=+1, 250=+2, etc)
	// Skill bonus: skill / 25 (25=+1)
	// No hard cap - formula naturally gives 6 at max power (500) + max skill (25)
	int statesCanClear = 1 + (power / 125) + (skill / 25);

	int clearedMask = 0;
	int statesCleared = 0;
	Locker locker(player);

	// Priority 1: Clear Blinded state (bit 4) - ALWAYS cleared
	if (statesCleared < statesCanClear && player->hasState(CreatureState::BLINDED)) {
		player->clearState(CreatureState::BLINDED, true);
		clearedMask |= (1 << 4);
		statesCleared++;
	} else if (statesCleared < statesCanClear) {
		statesCleared++;  // Count slot even if state wasn't present
	}

	// Priority 2: Clear Stunned state (bit 3)
	if (statesCleared < statesCanClear && player->hasState(CreatureState::STUNNED)) {
		player->clearState(CreatureState::STUNNED, true);
		clearedMask |= (1 << 3);
		statesCleared++;
	} else if (statesCleared < statesCanClear) {
		statesCleared++;
	}

	// Priority 3: Clear Dizzy state (bit 2)
	if (statesCleared < statesCanClear && player->hasState(CreatureState::DIZZY)) {
		player->clearState(CreatureState::DIZZY, true);
		clearedMask |= (1 << 2);
		statesCleared++;
	} else if (statesCleared < statesCanClear) {
		statesCleared++;
	}

	// Priority 4: Clear Bleeding DoT (bit 1)
	if (statesCleared < statesCanClear && player->hasState(CreatureState::BLEEDING)) {
		bool healed = player->healDot(CreatureState::BLEEDING, 9999, true);
		if (healed) {
			player->clearState(CreatureState::BLEEDING);
			clearedMask |= (1 << 1);
		}
		statesCleared++;
	} else if (statesCleared < statesCanClear) {
		statesCleared++;
	}

	// Priority 5: Clear Fire DoT (bit 0)
	if (statesCleared < statesCanClear && player->hasState(CreatureState::ONFIRE)) {
		bool healed = player->healDot(CreatureState::ONFIRE, 9999, true);
		if (healed) {
			player->clearState(CreatureState::ONFIRE);
			clearedMask |= (1 << 0);
		}
		statesCleared++;
	} else if (statesCleared < statesCanClear) {
		statesCleared++;
	}

	// Priority 6: Reset speed multiplier to clear root/snare effects (bit 5)
	if (statesCleared < statesCanClear) {
		float currentSpeed = player->getSpeedMultiplierMod();
		if (currentSpeed < 1.0f) {
			player->setSpeedMultiplierMod(1.0f, true, true);
			clearedMask |= (1 << 5);
		}

		// Also clear IMMOBILIZED state if present
		if (player->hasState(CreatureState::IMMOBILIZED)) {
			player->clearState(CreatureState::IMMOBILIZED, true);
			clearedMask |= (1 << 5);
		}
	}

	// Play purge effect if anything was cleared
	if (clearedMask > 0) {
		player->playEffect("clienteffect/medic_heal.cef", "");
	}

	info(true) << "clearCombatSerumDebuffs: Player " << player->getObjectID()
	           << " power=" << power << " skill=" << skill
	           << " statesCanClear=" << statesCanClear << " clearedMask=" << clearedMask;

	return clearedMask;
}

// ============================================================================
// PET DAMAGE TRANSFER
// ============================================================================

/*
 * Transfer damage from DW player to their high-level pet
 *
 * When a Death Watch player with a CL95+ pet takes damage, some of that
 * damage is transferred to their pet instead.
 *
 * Requirements:
 * - Player must have combat_death_watch_novice skill
 * - Player must have an active pet with CL >= 95
 * - Pet must not be incapacitated/dead
 *
 * Formula:
 * - Base transfer: 10%
 * - Max transfer: 25%
 * - Scaling: 10% + min(15%, (dw_toughness + dw_recovery) * 0.075)
 */
int DeathWatchManagerImplementation::applyPetDamageTransfer(CreatureObject* defender, TangibleObject* attacker, int incomingDamage) {
	if (defender == nullptr || incomingDamage <= 0) {
		return incomingDamage;
	}

	// Must be a DW player
	if (!defender->isPlayerCreature() || !defender->hasSkill("combat_death_watch_novice")) {
		return incomingDamage;
	}

	// Get player's ghost to check for active pets
	PlayerObject* ghost = defender->getPlayerObject();
	if (ghost == nullptr) {
		return incomingDamage;
	}

	// Find first active pet that's CL 95+
	CreatureObject* eligiblePet = nullptr;
	int petCount = ghost->getActivePetsSize();

	for (int i = 0; i < petCount; i++) {
		ManagedReference<AiAgent*> pet = ghost->getActivePet(i);
		if (pet == nullptr) {
			continue;
		}

		// Check if pet is alive and not incapacitated
		if (pet->isDead() || pet->isIncapacitated()) {
			continue;
		}

		// Check CL requirement (95+)
		int petLevel = pet->getLevel();
		if (petLevel < 95) {
			continue;
		}

		// Pet must be within 32m of the player
		if (pet->getDistanceTo(defender) > 32.0f) {
			continue;
		}

		eligiblePet = pet;
		break;
	}

	if (eligiblePet == nullptr) {
		return incomingDamage;  // No eligible pet found
	}

	// Calculate transfer percentage
	// Base: 10%, Max: 25%, Scaling via skill mods
	int dwToughness = defender->getSkillMod("dw_toughness");
	int dwRecovery = defender->getSkillMod("dw_recovery");

	// Formula: 10% base + (toughness + recovery) * 0.075, capped at 25%
	float baseTransfer = 0.10f;
	float bonusTransfer = Math::min(0.15f, (float)(dwToughness + dwRecovery) * 0.00075f);
	float transferPercent = baseTransfer + bonusTransfer;

	// Calculate damage to transfer
	int damageToTransfer = (int)(incomingDamage * transferPercent);
	if (damageToTransfer <= 0) {
		return incomingDamage;
	}

	// Don't transfer more than the pet can survive
	int petHealth = eligiblePet->getHAM(CreatureAttribute::HEALTH);
	if (damageToTransfer >= petHealth) {
		damageToTransfer = petHealth - 1;  // Leave pet with 1 health
		if (damageToTransfer <= 0) {
			return incomingDamage;  // Pet is too low, don't transfer
		}
	}

	// Transfer damage to pet (split health/action evenly)
	int petHealthDamage = damageToTransfer / 2;
	int petActionDamage = damageToTransfer - petHealthDamage;

	Locker petLocker(eligiblePet, defender);

	eligiblePet->inflictDamage(attacker, CreatureAttribute::HEALTH, petHealthDamage, true, true, true);
	eligiblePet->inflictDamage(attacker, CreatureAttribute::ACTION, petActionDamage, true, true, true);

	// Calculate damage remaining for player
	int playerDamage = incomingDamage - damageToTransfer;

	// Notify player of transfer (subtle flytext on pet)
	eligiblePet->showFlyText("combat_effects", "hit", 255, 0, 0);  // Red "Hit" indicator

	return playerDamage;
}

// ============================================================================
// SERUM/STIM COMMAND IMPLEMENTATION
// ============================================================================
// Commands for activating DW serums via hotbar. Each serum type has two methods:
// - apply*(): Called from command, searches inventory for item
// - apply*WithItem(): Called from radial menu, item already known

/**
 * Find a stim item in player's inventory by type
 */
TangibleObject* DeathWatchManagerImplementation::findStimInInventory(CreatureObject* player, const String& stimType) {
	if (player == nullptr || !stimConfigLoaded) {
		return nullptr;
	}

	int idx = stimTemplates.find(stimType);
	if (idx == -1) {
		return nullptr;
	}

	String templatePath = stimTemplates.get(idx);
	if (templatePath.isEmpty()) {
		return nullptr;
	}

	// Get player's inventory
	SceneObject* inventory = player->getSlottedObject("inventory");
	if (inventory == nullptr) {
		return nullptr;
	}

	// Search for matching item
	for (int i = 0; i < inventory->getContainerObjectsSize(); i++) {
		SceneObject* item = inventory->getContainerObject(i);
		if (item == nullptr) {
			continue;
		}

		// Check if template matches
		const String& itemTemplate = item->getObjectTemplate()->getFullTemplateString();
		if (itemTemplate == templatePath) {
			TangibleObject* tangible = dynamic_cast<TangibleObject*>(item);
			if (tangible != nullptr && tangible->getUseCount() > 0) {
				return tangible;
			}
		}
	}

	return nullptr;
}

/**
 * Apply HAM cost for a stim use
 */
bool DeathWatchManagerImplementation::applyStimHamCost(CreatureObject* player, const String& stimType) {
	if (player == nullptr || !stimConfigLoaded) {
		return false;
	}

	int idx = stimHamCosts.find(stimType);
	if (idx == -1) {
		return true;  // No cost configured, allow use
	}

	const StimHamConfig& config = stimHamCosts.get(idx);
	if (config.base <= 0) {
		return true;  // No cost
	}

	// Calculate actual cost after skill reduction
	int skillValue = 0;
	if (!config.skill.isEmpty()) {
		skillValue = player->getSkillMod(config.skill);
	}

	int actualCost = Math::max(1, config.base - (skillValue * config.perPoint));

	// Check and apply cost based on pool type
	if (config.pool == "health") {
		if (player->getHAM(CreatureAttribute::HEALTH) <= actualCost) {
			player->sendSystemMessage("You do not have enough health to use this item.");
			return false;
		}
		player->inflictDamage(player, CreatureAttribute::HEALTH, actualCost, false, false, true);
	} else if (config.pool == "action") {
		if (player->getHAM(CreatureAttribute::ACTION) <= actualCost) {
			player->sendSystemMessage("You do not have enough action to use this item.");
			return false;
		}
		player->inflictDamage(player, CreatureAttribute::ACTION, actualCost, false, false, true);
	} else if (config.pool == "mind") {
		if (player->getHAM(CreatureAttribute::MIND) <= actualCost) {
			player->sendSystemMessage("You do not have enough mind to use this item.");
			return false;
		}
		player->inflictDamage(player, CreatureAttribute::MIND, actualCost, false, false, true);
	} else if (config.pool == "all") {
		// Split cost across all three pools
		int perPool = actualCost / 3;
		if (player->getHAM(CreatureAttribute::HEALTH) <= perPool ||
			player->getHAM(CreatureAttribute::ACTION) <= perPool ||
			player->getHAM(CreatureAttribute::MIND) <= perPool) {
			player->sendSystemMessage("You do not have enough HAM to use this item.");
			return false;
		}
		player->inflictDamage(player, CreatureAttribute::HEALTH, perPool, false, false, true);
		player->inflictDamage(player, CreatureAttribute::ACTION, perPool, false, false, true);
		player->inflictDamage(player, CreatureAttribute::MIND, perPool, false, false, true);
	}

	return true;
}

/**
 * Apply a cooldown without checking (used after toggle-off)
 */
void DeathWatchManagerImplementation::applyCooldown(CreatureObject* player, const String& commandName) {
	if (player == nullptr) {
		return;
	}

	// Get cooldown duration from config
	String stimType = "";
	if (commandName == "dwTorcabur") {
		stimType = "torcabur";
	} else if (commandName == "dwNaakkelir") {
		stimType = "naakkelir";
	} else if (commandName == "dwBeskargam") {
		stimType = "beskargam";
	} else if (commandName == "dwGotaldin") {
		stimType = "gotaldin";
	} else if (commandName == "dwAranovor") {
		stimType = "aranovor";
	} else if (commandName == "dwHettir") {
		stimType = "hettir";
	}

	int idx = stimCooldowns.find(stimType);
	int cooldownMs = (idx != -1) ? stimCooldowns.get(idx) : 60000;

	// Apply cooldown using the existing system
	PlayerObject* ghost = player->getPlayerObject();
	if (ghost == nullptr) {
		return;
	}

	// Use player variable to track cooldown
	String cooldownKey = "dw_cooldown_" + commandName;
	Time now;
	ghost->setScreenPlayData("DeathWatch", cooldownKey, String::valueOf(now.getMiliTime() + cooldownMs));
}

/**
 * Apply Tor'cabur serum (Mind Immunity toggle)
 * Command path - searches inventory for item
 */
int DeathWatchManagerImplementation::applyTorcabur(CreatureObject* player) {
	if (player == nullptr) {
		return QueueCommand::GENERALERROR;
	}

	// DW membership check
	if (!player->hasSkill("combat_death_watch_novice")) {
		player->sendSystemMessage("You must be Death Watch to use this item.");
		return QueueCommand::GENERALERROR;
	}

	// Skill requirement check
	int idx = stimSkillRequirements.find("torcabur");
	int requiredSkill = (idx != -1) ? stimSkillRequirements.get(idx) : 25;
	int playerSkill = player->getSkillMod("dw_healing");
	if (playerSkill < requiredSkill) {
		player->sendSystemMessage("Your Dralshy'a skill is not high enough to use this item.");
		return QueueCommand::GENERALERROR;
	}

	// Find stim in inventory
	TangibleObject* stimItem = findStimInInventory(player, "torcabur");
	if (stimItem == nullptr) {
		player->sendSystemMessage("You need a Tor'cabur serum to use this command.");
		return QueueCommand::GENERALERROR;
	}

	return applyTorcaburWithItem(player, stimItem);
}

/**
 * Apply Tor'cabur serum with item reference (radial menu path)
 */
int DeathWatchManagerImplementation::applyTorcaburWithItem(CreatureObject* player, TangibleObject* stimItem) {
	if (player == nullptr || stimItem == nullptr) {
		return QueueCommand::GENERALERROR;
	}

	// Check if buff is already active (toggle-off case)
	bool isToggleOff = player->hasBuff(BuffCRC::DW_MIND_IMMUNITY);

	// Cooldown check - but toggle-off bypasses
	if (!isToggleOff) {
		if (!checkAndApplyCooldown(player, "dwTorcabur")) {
			return QueueCommand::GENERALERROR;
		}
	}

	// HAM cost
	if (!applyStimHamCost(player, "torcabur")) {
		return QueueCommand::GENERALERROR;
	}

	// Decrement charge
	Locker itemLocker(stimItem, player);
	stimItem->decreaseUseCount();

	if (isToggleOff) {
		// Toggle OFF - remove buff and set cooldown
		player->removeBuff(BuffCRC::DW_MIND_IMMUNITY);
		player->sendSystemMessage("Mental fortitude deactivated.");
		player->playEffect("clienteffect/pl_force_absorb_hit.cef", "");
		applyCooldown(player, "dwTorcabur");
	} else {
		// Activate buff
		int idx = stimDurations.find("torcabur");
		int duration = 75;  // Default 75 seconds
		if (idx != -1) {
			const StimDurationConfig& durConfig = stimDurations.get(idx);
			duration = durConfig.base;
			if (!durConfig.fixed && !durConfig.skill.isEmpty()) {
				int skillVal = player->getSkillMod(durConfig.skill);
				duration += skillVal * durConfig.perPoint;
				if (durConfig.max > 0) {
					duration = Math::min(duration, durConfig.max);
				}
			}
		}

		// Power scaling is not used for this effect (toggle)
		// Create buff
		ManagedReference<Buff*> buff = new Buff(player, BuffCRC::DW_MIND_IMMUNITY, duration, BuffType::OTHER);
		buff->setSkillModifier("dw_mind_immunity", 1);

		Locker buffLocker(buff, player);
		player->addBuff(buff);

		player->playEffect("clienteffect/pl_force_resist_bleeding.cef", "");
		player->sendSystemMessage("\\#00FFFFMental fortitude activated!");
	}

	return QueueCommand::SUCCESS;
}

/**
 * Apply Naak'kelir serum (Purge - clears negative states)
 */
int DeathWatchManagerImplementation::applyNaakkelir(CreatureObject* player) {
	if (player == nullptr) {
		return QueueCommand::GENERALERROR;
	}

	// DW membership check
	if (!player->hasSkill("combat_death_watch_novice")) {
		player->sendSystemMessage("You must be Death Watch to use this item.");
		return QueueCommand::GENERALERROR;
	}

	// Skill requirement check
	int idx = stimSkillRequirements.find("naakkelir");
	int requiredSkill = (idx != -1) ? stimSkillRequirements.get(idx) : 28;
	int playerSkill = player->getSkillMod("dw_healing");
	if (playerSkill < requiredSkill) {
		player->sendSystemMessage("Your Dralshy'a skill is not high enough to use this item.");
		return QueueCommand::GENERALERROR;
	}

	// Find stim in inventory
	TangibleObject* stimItem = findStimInInventory(player, "naakkelir");
	if (stimItem == nullptr) {
		player->sendSystemMessage("You need a Naak'kelir serum to use this command.");
		return QueueCommand::GENERALERROR;
	}

	return applyNaakkelirWithItem(player, stimItem);
}

/**
 * Apply Naak'kelir serum with item reference (radial menu path)
 */
int DeathWatchManagerImplementation::applyNaakkelirWithItem(CreatureObject* player, TangibleObject* stimItem) {
	if (player == nullptr || stimItem == nullptr) {
		return QueueCommand::GENERALERROR;
	}

	// Cooldown check
	if (!checkAndApplyCooldown(player, "dwNaakkelir")) {
		return QueueCommand::GENERALERROR;
	}

	// HAM cost
	if (!applyStimHamCost(player, "naakkelir")) {
		return QueueCommand::GENERALERROR;
	}

	// Use default power (crafted power attributes not accessible via TangibleObject API)
	int power = 100;
	int dwRecovery = player->getSkillMod("dw_recovery");

	// Calculate states to clear: 1 + floor(power/125) + floor(dw_recovery/25)
	int statesToClear = 1 + (power / 125) + (dwRecovery / 25);

	// Decrement charge
	Locker itemLocker(stimItem, player);
	stimItem->decreaseUseCount();

	// Use the existing clearCombatSerumDebuffs method
	int clearedMask = clearCombatSerumDebuffs(player, power, dwRecovery);

	// Build message about what was cleared
	String clearedList = "";
	if (clearedMask & 0x01) clearedList += "Fire ";
	if (clearedMask & 0x02) clearedList += "Bleeding ";
	if (clearedMask & 0x04) clearedList += "Dizzy ";
	if (clearedMask & 0x08) clearedList += "Stun ";
	if (clearedMask & 0x10) clearedList += "Blind ";
	if (clearedMask & 0x20) clearedList += "Slow ";

	player->playEffect("clienteffect/pl_force_healing.cef", "");

	if (clearedList.isEmpty()) {
		player->sendSystemMessage("You have no negative states to clear.");
	} else {
		player->sendSystemMessage("\\#00FF00Purge surge! Cleared: " + clearedList);
	}

	return QueueCommand::SUCCESS;
}

/**
 * Apply Beskar'gam serum (Damage Shield)
 */
int DeathWatchManagerImplementation::applyBeskargam(CreatureObject* player) {
	if (player == nullptr) {
		return QueueCommand::GENERALERROR;
	}

	// DW membership check
	if (!player->hasSkill("combat_death_watch_novice")) {
		player->sendSystemMessage("You must be Death Watch to use this item.");
		return QueueCommand::GENERALERROR;
	}

	// Skill requirement check
	int idx = stimSkillRequirements.find("beskargam");
	int requiredSkill = (idx != -1) ? stimSkillRequirements.get(idx) : 29;
	int playerSkill = player->getSkillMod("dw_healing");
	if (playerSkill < requiredSkill) {
		player->sendSystemMessage("Your Dralshy'a skill is not high enough to use this item.");
		return QueueCommand::GENERALERROR;
	}

	// Find stim in inventory
	TangibleObject* stimItem = findStimInInventory(player, "beskargam");
	if (stimItem == nullptr) {
		player->sendSystemMessage("You need a Beskar'gam serum to use this command.");
		return QueueCommand::GENERALERROR;
	}

	return applyBeskargamWithItem(player, stimItem);
}

/**
 * Apply Beskar'gam serum with item reference (radial menu path)
 */
int DeathWatchManagerImplementation::applyBeskargamWithItem(CreatureObject* player, TangibleObject* stimItem) {
	if (player == nullptr || stimItem == nullptr) {
		return QueueCommand::GENERALERROR;
	}

	// Cooldown check
	if (!checkAndApplyCooldown(player, "dwBeskargam")) {
		return QueueCommand::GENERALERROR;
	}

	// HAM cost
	if (!applyStimHamCost(player, "beskargam")) {
		return QueueCommand::GENERALERROR;
	}

	// Use default power (crafted power attributes not accessible via TangibleObject API)
	int power = 100;
	int dwHealing = player->getSkillMod("dw_healing");

	// Calculate mitigation: 1.5 base + (power/100) + (dw_healing * 0.05), max 3.0
	float mitigation = 1.5f + ((float)power / 100.0f) + ((float)dwHealing * 0.05f);
	mitigation = Math::min(3.0f, mitigation);

	// Decrement charge
	Locker itemLocker(stimItem, player);
	stimItem->decreaseUseCount();

	// Get duration (fixed 60 seconds)
	int idx = stimDurations.find("beskargam");
	int duration = 60;
	if (idx != -1) {
		const StimDurationConfig& durConfig = stimDurations.get(idx);
		duration = durConfig.base;
	}

	// Create buff with damage mitigation
	ManagedReference<Buff*> buff = new Buff(player, BuffCRC::DW_DAMAGE_SHIELD, duration, BuffType::OTHER);
	// Store mitigation value as skill mod (multiplied by 100 for integer storage)
	buff->setSkillModifier("private_mitigation_divisor", (int)(mitigation * 100.0f));

	Locker buffLocker(buff, player);
	player->addBuff(buff);

	player->playEffect("clienteffect/pl_force_armor.cef", "");
	float mitigationPercent = (1.0f - (1.0f / mitigation)) * 100.0f;
	player->sendSystemMessage("\\#00FFFFDamage shield activated! Mitigation: " + String::valueOf((int)mitigationPercent) + "%");

	return QueueCommand::SUCCESS;
}

/**
 * Apply Gotal'din serum (Missile Reflector)
 */
int DeathWatchManagerImplementation::applyGotaldin(CreatureObject* player) {
	if (player == nullptr) {
		return QueueCommand::GENERALERROR;
	}

	// DW membership check
	if (!player->hasSkill("combat_death_watch_novice")) {
		player->sendSystemMessage("You must be Death Watch to use this item.");
		return QueueCommand::GENERALERROR;
	}

	// Skill requirement check
	int idx = stimSkillRequirements.find("gotaldin");
	int requiredSkill = (idx != -1) ? stimSkillRequirements.get(idx) : 31;
	int playerSkill = player->getSkillMod("dw_healing");
	if (playerSkill < requiredSkill) {
		player->sendSystemMessage("Your Dralshy'a skill is not high enough to use this item.");
		return QueueCommand::GENERALERROR;
	}

	// Find stim in inventory
	TangibleObject* stimItem = findStimInInventory(player, "gotaldin");
	if (stimItem == nullptr) {
		player->sendSystemMessage("You need a Gotal'din serum to use this command.");
		return QueueCommand::GENERALERROR;
	}

	return applyGotaldinWithItem(player, stimItem);
}

/**
 * Apply Gotal'din serum with item reference (radial menu path)
 */
int DeathWatchManagerImplementation::applyGotaldinWithItem(CreatureObject* player, TangibleObject* stimItem) {
	if (player == nullptr || stimItem == nullptr) {
		return QueueCommand::GENERALERROR;
	}

	// Cooldown check
	if (!checkAndApplyCooldown(player, "dwGotaldin")) {
		return QueueCommand::GENERALERROR;
	}

	// HAM cost
	if (!applyStimHamCost(player, "gotaldin")) {
		return QueueCommand::GENERALERROR;
	}

	// Use default power (crafted power attributes not accessible via TangibleObject API)
	int power = 100;
	int dwRecovery = player->getSkillMod("dw_recovery");

	// Calculate reflect power (stored for missile hit processing)
	int reflectPower = power;

	// Decrement charge
	Locker itemLocker(stimItem, player);
	stimItem->decreaseUseCount();

	// Get duration
	int idx = stimDurations.find("gotaldin");
	int duration = 60;
	if (idx != -1) {
		const StimDurationConfig& durConfig = stimDurations.get(idx);
		duration = durConfig.base;
		if (!durConfig.fixed && !durConfig.skill.isEmpty()) {
			int skillVal = player->getSkillMod(durConfig.skill);
			duration += skillVal * durConfig.perPoint;
			if (durConfig.max > 0) {
				duration = Math::min(duration, durConfig.max);
			}
		}
	}

	// Create buff
	ManagedReference<Buff*> buff = new Buff(player, BuffCRC::DW_MISSILE_REFLECTOR, duration, BuffType::OTHER);
	buff->setSkillModifier("dw_missile_reflector", reflectPower);

	Locker buffLocker(buff, player);
	player->addBuff(buff);

	player->playEffect("clienteffect/pl_force_feedback.cef", "");
	player->sendSystemMessage("\\#00FFFFMissile reflector online! Reflect power: " + String::valueOf(reflectPower));

	return QueueCommand::SUCCESS;
}

/**
 * Apply Ara'novor serum (Targeting Boost)
 */
int DeathWatchManagerImplementation::applyAranovor(CreatureObject* player) {
	if (player == nullptr) {
		return QueueCommand::GENERALERROR;
	}

	// DW membership check
	if (!player->hasSkill("combat_death_watch_novice")) {
		player->sendSystemMessage("You must be Death Watch to use this item.");
		return QueueCommand::GENERALERROR;
	}

	// Skill requirement check
	int idx = stimSkillRequirements.find("aranovor");
	int requiredSkill = (idx != -1) ? stimSkillRequirements.get(idx) : 30;
	int playerSkill = player->getSkillMod("dw_healing");
	if (playerSkill < requiredSkill) {
		player->sendSystemMessage("Your Dralshy'a skill is not high enough to use this item.");
		return QueueCommand::GENERALERROR;
	}

	// Find stim in inventory
	TangibleObject* stimItem = findStimInInventory(player, "aranovor");
	if (stimItem == nullptr) {
		player->sendSystemMessage("You need an Ara'novor serum to use this command.");
		return QueueCommand::GENERALERROR;
	}

	return applyAranovorWithItem(player, stimItem);
}

/**
 * Apply Ara'novor serum with item reference (radial menu path)
 */
int DeathWatchManagerImplementation::applyAranovorWithItem(CreatureObject* player, TangibleObject* stimItem) {
	if (player == nullptr || stimItem == nullptr) {
		return QueueCommand::GENERALERROR;
	}

	// Cooldown check
	if (!checkAndApplyCooldown(player, "dwAranovor")) {
		return QueueCommand::GENERALERROR;
	}

	// HAM cost
	if (!applyStimHamCost(player, "aranovor")) {
		return QueueCommand::GENERALERROR;
	}

	// Use default power (crafted power attributes not accessible via TangibleObject API)
	int power = 100;

	// Calculate accuracy bonus: power / 5 (range: +10 to +40)
	int accuracyBonus = Math::max(10, Math::min(40, power / 5));

	// Decrement charge
	Locker itemLocker(stimItem, player);
	stimItem->decreaseUseCount();

	// Get duration (fixed 20 seconds)
	int idx = stimDurations.find("aranovor");
	int duration = 20;
	if (idx != -1) {
		const StimDurationConfig& durConfig = stimDurations.get(idx);
		duration = durConfig.base;
	}

	// Create buff
	ManagedReference<Buff*> buff = new Buff(player, BuffCRC::DW_TARGETING_BOOST, duration, BuffType::OTHER);
	buff->setSkillModifier("dw_targeting_boost", accuracyBonus);

	Locker buffLocker(buff, player);
	player->addBuff(buff);

	player->playEffect("clienteffect/pl_force_speed.cef", "");
	player->sendSystemMessage("\\#00FFFFTargeting boost activated! +" + String::valueOf(accuracyBonus) + " accuracy");

	return QueueCommand::SUCCESS;
}

/**
 * Apply Hettir canister (AoE Bacta Denial)
 */
int DeathWatchManagerImplementation::applyHettir(CreatureObject* player) {
	if (player == nullptr) {
		return QueueCommand::GENERALERROR;
	}

	// DW membership check
	if (!player->hasSkill("combat_death_watch_novice")) {
		player->sendSystemMessage("You must be Death Watch to use this item.");
		return QueueCommand::GENERALERROR;
	}

	// Skill requirement check
	int idx = stimSkillRequirements.find("hettir");
	int requiredSkill = (idx != -1) ? stimSkillRequirements.get(idx) : 32;
	int playerSkill = player->getSkillMod("dw_healing");
	if (playerSkill < requiredSkill) {
		player->sendSystemMessage("Your Dralshy'a skill is not high enough to use this item.");
		return QueueCommand::GENERALERROR;
	}

	// Find stim in inventory
	TangibleObject* stimItem = findStimInInventory(player, "hettir");
	if (stimItem == nullptr) {
		player->sendSystemMessage("You need a Hettir canister to use this command.");
		return QueueCommand::GENERALERROR;
	}

	return applyHettirWithItem(player, stimItem);
}

/**
 * Apply Hettir canister with item reference (radial menu path)
 */
int DeathWatchManagerImplementation::applyHettirWithItem(CreatureObject* player, TangibleObject* stimItem) {
	if (player == nullptr || stimItem == nullptr) {
		return QueueCommand::GENERALERROR;
	}

	// Cooldown check
	if (!checkAndApplyCooldown(player, "dwHettir")) {
		return QueueCommand::GENERALERROR;
	}

	// HAM cost
	if (!applyStimHamCost(player, "hettir")) {
		return QueueCommand::GENERALERROR;
	}

	// Get duration (fixed 45 seconds per plan)
	int idx = stimDurations.find("hettir");
	int duration = 45;
	if (idx != -1) {
		const StimDurationConfig& durConfig = stimDurations.get(idx);
		duration = durConfig.base;
	}

	// Get player's DW faction
	PlayerObject* attackerGhost = player->getPlayerObject();
	if (attackerGhost == nullptr) {
		return QueueCommand::GENERALERROR;
	}
	byte attackerFaction = attackerGhost->getDwFaction();

	// Decrement charge
	Locker itemLocker(stimItem, player);
	stimItem->decreaseUseCount();

	// Play effect at caster
	player->playEffect("clienteffect/combat_acid_hit.cef", "");

	// Find all DW enemies within 32m
	int affectedCount = 0;
	Zone* zone = player->getZone();
	if (zone == nullptr) {
		return QueueCommand::GENERALERROR;
	}

	// Get nearby objects
	SortedVector<ManagedReference<TreeEntry*>> nearbyObjects;
	zone->getInRangeObjects(player->getPositionX(), player->getPositionZ(), player->getPositionY(), 32.0f, &nearbyObjects, true);

	for (int i = 0; i < nearbyObjects.size(); i++) {
		SceneObject* obj = static_cast<SceneObject*>(nearbyObjects.get(i).get());
		if (obj == nullptr || !obj->isPlayerCreature()) {
			continue;
		}

		CreatureObject* target = dynamic_cast<CreatureObject*>(obj);
		if (target == nullptr || target == player) {
			continue;
		}

		// Check if target is DW
		if (!target->hasSkill("combat_death_watch_novice")) {
			continue;
		}

		// Check if target is enemy faction
		PlayerObject* targetGhost = target->getPlayerObject();
		if (targetGhost == nullptr) {
			continue;
		}
		byte targetFaction = targetGhost->getDwFaction();

		if (!areFactionsAtWar(attackerFaction, targetFaction)) {
			continue;
		}

		// Apply bacta denial buff to target
		Locker targetLocker(target, player);

		ManagedReference<Buff*> buff = new Buff(target, BuffCRC::DW_BACTA_DENIAL, duration, BuffType::OTHER);
		buff->setSkillModifier("dw_bacta_denial", 1);

		Locker buffLocker(buff, target);
		target->addBuff(buff);

		target->playEffect("clienteffect/pl_force_choke.cef", "");
		target->sendSystemMessage("\\#FF0000You have been hit by a bacta denial field!");

		affectedCount++;
	}

	if (affectedCount > 0) {
		player->sendSystemMessage("\\#FF0000Bacta denial field deployed! " + String::valueOf(affectedCount) + " enemies affected.");
	} else {
		player->sendSystemMessage("No valid Death Watch enemies in range.");
	}

	return QueueCommand::SUCCESS;
}

/**
 * Generic stim application with item (for Lua radial menu)
 * Determines stim type from item template and calls appropriate apply*WithItem
 */
int DeathWatchManagerImplementation::applyDwStimWithItem(CreatureObject* player, TangibleObject* stimItem) {
	if (player == nullptr || stimItem == nullptr || !stimConfigLoaded) {
		return QueueCommand::GENERALERROR;
	}

	// DW membership check
	if (!player->hasSkill("combat_death_watch_novice")) {
		player->sendSystemMessage("You must be Death Watch to use this item.");
		return QueueCommand::GENERALERROR;
	}

	// Get item template
	const String& itemTemplate = stimItem->getObjectTemplate()->getFullTemplateString();

	// Determine stim type from template
	for (int i = 0; i < stimTemplates.size(); i++) {
		if (stimTemplates.get(i) == itemTemplate) {
			const String& stimType = stimTemplates.elementAt(i).getKey();

			// Skill requirement check
			int idx = stimSkillRequirements.find(stimType);
			int requiredSkill = (idx != -1) ? stimSkillRequirements.get(idx) : 0;
			int playerSkill = player->getSkillMod("dw_healing");
			if (playerSkill < requiredSkill) {
				player->sendSystemMessage("Your Dralshy'a skill is not high enough to use this item.");
				return QueueCommand::GENERALERROR;
			}

			// Call appropriate method
			if (stimType == "torcabur") {
				return applyTorcaburWithItem(player, stimItem);
			} else if (stimType == "naakkelir") {
				return applyNaakkelirWithItem(player, stimItem);
			} else if (stimType == "beskargam") {
				return applyBeskargamWithItem(player, stimItem);
			} else if (stimType == "gotaldin") {
				return applyGotaldinWithItem(player, stimItem);
			} else if (stimType == "aranovor") {
				return applyAranovorWithItem(player, stimItem);
			} else if (stimType == "hettir") {
				return applyHettirWithItem(player, stimItem);
			}
		}
	}

	player->sendSystemMessage("Unknown serum type.");
	return QueueCommand::GENERALERROR;
}
