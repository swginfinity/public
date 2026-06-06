--[[
	Death Watch Manager Configuration

	This file contains all tunable parameters for the Death Watch PvP system.
	Changes to this file take effect on server restart.

	Infinity Custom Content
]]

-- ============================================================================
-- TEST SERVER CONFIGURATION
-- ============================================================================
-- Servers listed here allow same-account attacks for testing purposes.
-- This bypasses the alt-abuse protection on these servers.
-- Format: { "ServerName1", "ServerName2", ... }

testServers = {
	"SWG Infinity TC",
	"WickedDev",
	"VolinorDev",
	"MrObviousDev",
}

-- ============================================================================
-- XP MULTIPLIERS
-- ============================================================================

-- Multiplier for players without required mastery for their faction
-- 1.0 = full XP, 0.6 = 60% XP
noMasteryMultiplier = 0.6

-- ============================================================================
-- FACTION STANDING THRESHOLDS
-- ============================================================================
-- These control XP gain multipliers based on faction standing

standingFullRewards = 5000           -- >= this = 100% XP
standingMinorPenalty = 4900          -- >= this = 90% XP
standingSignificantPenalty = 4000    -- >= this = 50% XP
standingHeavyPenalty = 1             -- >= this = 25% XP
standingNoGains = 0                  -- <= this = 0% XP (no gains)

-- Hard floor: must have standing ABOVE this to earn ANY PvP XP
standingPvpXpFloor = 1000

-- ============================================================================
-- DW TEF (TEMPORARY ENEMY FLAG)
-- ============================================================================

-- Duration of DW TEF in milliseconds (blocks non-DW healing during DW combat)
dwTefDurationMs = 300000  -- 5 minutes

-- ============================================================================
-- FACTION STANDING CHANGES FROM PVP
-- ============================================================================
-- PvP is about MAINTAINING standing, not gaining it.
-- Players gain standing primarily through NPC faction missions.

standingEnemyKill = 15       -- Killing an enemy faction player
standingDeath = -8           -- Dying to an enemy faction player
standingAlliedKill = -25     -- Killing an allied faction player (during 2v2 wars)
standingTraitorKill = -50    -- Killing a same-faction player (teamkill)

-- ============================================================================
-- GANK PROTECTION
-- ============================================================================

gankAttackerThreshold = 4    -- 4+ attackers = gank
gankDeathXp = -100           -- Reduced death penalty for gank victim
gankSurvivalBonus = 5000     -- Bonus XP for surviving a gank

-- ============================================================================
-- COLLUDER DETECTION
-- ============================================================================
-- Detects pairs of players who frequently hunt together.
-- When threshold is reached, both players have XP frozen until one kills the other.
-- This prevents coordinated farming of the same targets.

colluderConfig = {
	jointKillThreshold = 3,         -- Number of joint kills to flag as colluders
	trackingWindowMs = 3600000,     -- 1 hour window for counting joint kills
	maxTrackedKills = 20,           -- FIFO: keep only last 20 kill records per pair
}

-- ============================================================================
-- RANK-BASED XP SYSTEM
-- ============================================================================
-- XP is calculated based on total rank (sum of all faction boxes + 1)
-- Rank ranges from 1 (novice only) to 17 (all 4 factions at Box 4)
--
-- Killing higher ranks = bonus XP, killing lower ranks = reduced XP
-- Dying to higher ranks = reduced loss, dying to lower ranks = increased loss
-- This discourages farming low-rank players and rewards fighting up.

-- Base XP values (before rank multipliers)
baseKillXp = 4000           -- Base XP for killing an equal-rank opponent
baseDeathXp = 1500          -- Base XP loss for dying to an equal-rank opponent

-- Rank scaling parameters
-- killMultPerRankUp: bonus per rank when killing someone higher (underdog bonus)
-- killMultPerRankDown: penalty per rank when killing someone lower (stomping penalty)
-- killMultMin: minimum multiplier floor (prevents 0 XP for huge rank diff)
-- killMultMax: maximum multiplier cap (prevents excessive XP for huge underdog wins)
rankKillMultPerRankUp = 0.10    -- +10% per rank difference when killing higher
rankKillMultPerRankDown = 0.08  -- -8% per rank difference when killing lower
rankKillMultMin = 0.10          -- Floor: 10% XP minimum (e.g., rank 17 vs rank 1)
rankKillMultMax = 2.00          -- Cap: 200% XP maximum (e.g., rank 1 vs rank 17)

-- Death penalty scaling
-- deathMultPerRankUp: reduced loss when dying to someone higher (expected loss)
-- deathMultPerRankDown: increased loss when dying to someone lower (embarrassment)
rankDeathMultPerRankUp = 0.075  -- -7.5% loss per rank difference when dying to higher
rankDeathMultPerRankDown = 0.05 -- +5% loss per rank difference when dying to lower
rankDeathMultMin = 0.25         -- Floor: 25% loss minimum (dying to much higher rank)
rankDeathMultMax = 1.50         -- Cap: 150% loss maximum (dying to much lower rank)

-- Bounty multipliers (applied on top of rank multipliers)
bountyKillMultiplier = 1.5      -- Hunter gets 1.5x XP for bounty kill
bountyDeathMultiplier = 0.5     -- Mark loses only 0.5x XP when killed by hunter

-- ============================================================================
-- LEGACY XP MATRICES (DEPRECATED - kept for reference)
-- ============================================================================
-- These are no longer used. The rank-based system above replaces them.
-- Uncomment if you need to revert to the old matrix-based system.
--[[
killXpMatrix = {
	{ 750, 1000, 1250, 1500, 2000 },   -- Killer Box 0
	{ 500, 1000, 1250, 1500, 2000 },   -- Killer Box 1
	{ 350, 500, 1000, 1250, 1750 },    -- Killer Box 2
	{ 250, 350, 500, 1000, 1500 },     -- Killer Box 3
	{ 200, 250, 350, 500, 1000 },      -- Killer Box 4
}

deathXpMatrix = {
	{ -500, -500, -500, -500, -500 },     -- Victim Box 0
	{ -750, -750, -750, -750, -750 },     -- Victim Box 1
	{ -900, -900, -750, -750, -750 },     -- Victim Box 2
	{ -1100, -1100, -900, -750, -750 },   -- Victim Box 3
	{ -1500, -1500, -1100, -900, -750 },  -- Victim Box 4
}
]]

-- ============================================================================
-- DEMOTION THRESHOLDS
-- ============================================================================
-- XP thresholds below which a player loses their current box
-- These are approximately 20% below the box requirement
-- Note: These are hardcoded in C++ for now but documented here for reference
--
-- Box 1: 20,000 XP (requirement: 25,000)
-- Box 2: 60,000 XP (requirement: 75,000)
-- Box 3: 140,000 XP (requirement: 175,000)
-- Box 4: 300,000 XP (requirement: 375,000)

-- ============================================================================
-- FACTION MASTERY REQUIREMENTS
-- ============================================================================
-- Each faction requires specific combat masteries for full XP rewards.
-- Players without a valid mastery receive reduced XP (noMasteryMultiplier).
-- Format: factionMasteryRequirements[factionId] = { "skill1", "skill2", ... }
-- Faction IDs: 1 = XON, 2 = SUN, 3 = KRR, 4 = ZAN

factionMasteryRequirements = {
	-- Xonti Brothers (XON) - Faction 1
	[1] = {
		"combat_unarmed_novice",
		"combat_pistol_novice",
	},
	-- Black Sun (SUN) - Faction 2
	[2] = {
		"combat_carbine_novice",
		"combat_polearm_novice",
	},
	-- Black Krrsantan (KRR) - Faction 3
	[3] = {
		"combat_rifleman_novice",
		"combat_commando_novice",
	},
	-- Zann Consortium (ZAN) - Faction 4
	[4] = {
		"combat_1hsword_novice",
		"combat_smuggler_novice",
	},
}

-- ============================================================================
-- MASTERY DISPLAY NAMES
-- ============================================================================
-- Human-readable names for skills (used in NPC conversations)
-- Add any skill that appears in factionMasteryRequirements above

masteryDisplayNames = {
	-- Novice skills
	combat_unarmed_novice = "Novice Teras Kasi",
	combat_pistol_novice = "Novice Pistoleer",
	combat_carbine_novice = "Novice Carbineer",
	combat_polearm_novice = "Novice Pikeman",
	combat_rifleman_novice = "Novice Rifleman",
	combat_commando_novice = "Novice Commando",
	combat_1hsword_novice = "Novice Fencer",
	combat_smuggler_novice = "Novice Smuggler",
	-- Master skills (in case you switch back)
	combat_unarmed_master = "Master Teras Kasi",
	combat_pistol_master = "Master Pistoleer",
	combat_carbine_master = "Master Carbineer",
	combat_polearm_master = "Master Pikeman",
	combat_rifleman_master = "Master Rifleman",
	combat_commando_master = "Master Commando",
	combat_1hsword_master = "Master Fencer",
	combat_smuggler_master = "Master Smuggler",
}

-- ============================================================================
-- DW IDENTIFY COMMAND CONFIGURATION
-- ============================================================================
-- Skill thresholds for intel gathering via /dwIdentify command
-- Higher subterfuge skill reveals more information about targets

dwIdentifyConfig = {
	-- Range configuration
	baseRange = 32,                     -- Base range at 0 subterfuge (meters)
	rangePerSkill = 0.64,               -- Additional range per subterfuge point

	-- Cooldown configuration (milliseconds)
	globalCooldownMs = 300000,          -- 5 minutes between any scan
	targetCooldownMs = 14400000,        -- 4 hours per specific target
	recoveryCooldownReduction = 1000,   -- 1 second reduction per dw_recovery point
	minGlobalCooldownMs = 60000,        -- Minimum 1 minute global cooldown
	minTargetCooldownMs = 3600000,      -- Minimum 1 hour target cooldown

	-- Detection configuration
	baseDetectionChance = 30,           -- 30% base chance target detects scan
	detectionPerScannerSkill = -0.2,    -- Scanner's subterfuge reduces detection
	detectionPerTargetSkill = 0.2,      -- Target's subterfuge increases detection
	minDetectionChance = 5,             -- Minimum 5% detection chance
	maxDetectionChance = 75,            -- Maximum 75% detection chance

	-- SUI configuration
	suiTimeoutMs = 30000,               -- 30 seconds to respond to detection alert

	-- Skill thresholds for intel tiers
	-- At each threshold, scanner gains access to additional intel
	intelThresholds = {
		faction = 15,                   -- 15+ subterfuge: Learn target's faction name
		relationship = 30,              -- 30+ subterfuge: Learn if enemy/ally/neutral
		boxLevel = 45,                  -- 45+ subterfuge: Learn target's box level (1-4)
		standing = 60,                  -- 60+ subterfuge: Learn target's standing range
	},

	-- Maximum skill level that provides benefits (cap for intel gathering)
	maxSkillBenefit = 75,
}

-- ============================================================================
-- FACTION DISPLAY CONFIGURATION
-- ============================================================================
-- Faction mappings used by status commands and intel reports

dwFactionConfig = {
	-- Faction ID to key mapping (matches C++ enum)
	idToKey = {
		[1] = "xon",
		[2] = "sun",
		[3] = "krr",
		[4] = "zan",
	},

	-- Faction display names
	names = {
		xon = "Xonti Brothers",
		sun = "Black Sun",
		krr = "Black Krrsantan",
		zan = "Zann Consortium",
	},

	-- XP type for each faction
	xpTypes = {
		xon = "dw_xon",
		sun = "dw_sun",
		krr = "dw_krr",
		zan = "dw_zan",
	},

	-- Standing key for each faction (Mandalorian Big 4)
	standingKeys = {
		xon = "xonti_brothers",
		sun = "blacksun",
		krr = "black_krrsantans",
		zan = "zann_consortium",
	},

	-- Skill prefixes for checking box levels
	skillPrefixes = {
		xon = "combat_death_watch_xon_",
		sun = "combat_death_watch_sun_",
		krr = "combat_death_watch_krr_",
		zan = "combat_death_watch_zan_",
	},
}

-- ============================================================================
-- JETPACK CONFIGURATION
-- ============================================================================
-- Template paths for jetpack detection

dwJetpackConfig = {
	-- Only Death Watch jetpack allows special attacks
	deathWatch = "object/mobile/vehicle/death_watch_jetpack.iff",
	standard = "object/mobile/vehicle/jetpack.iff",
}

-- ============================================================================
-- WHISTLING BIRD CONFIGURATION
-- ============================================================================
-- /dwWhistlingBird <1-6> command configuration
-- Each mark has its own settings for damage, cooldown, range, and skill requirements

dwWhistlingBirdConfig = {
	-- Common settings
	minSpeed = 18,                      -- Minimum speed to fire any missile
	minRange = 10,                      -- Minimum range to target (meters)
	maxRange = 64,                      -- Maximum range to target (meters)

	-- Missile template paths
	templates = {
		[1] = "object/weapon/ranged/grenade/whistling_bird_mk1.iff",
		[2] = "object/weapon/ranged/grenade/whistling_bird_mk2.iff",
		[3] = "object/weapon/ranged/grenade/whistling_bird_mk3.iff",
		[4] = "object/weapon/ranged/grenade/whistling_bird_mk4.iff",
		[5] = "object/weapon/ranged/grenade/whistling_bird_mk5.iff",
		[6] = "object/weapon/ranged/grenade/whistling_bird_mk6.iff",
	},

	-- Skill requirements per mark
	-- nil = no faction requirement (any DW), otherwise specific skill needed
	skillRequirements = {
		[1] = "combat_death_watch_novice",      -- Mk1: Any DW Novice
		[2] = nil,                               -- Mk2: Any Box 2 (checked separately)
		[3] = nil,                               -- Mk3: Any Box 2 (checked separately)
		[4] = "combat_death_watch_krr_3",        -- Mk4: Krrsantan Box 3
		[5] = "combat_death_watch_zan_3",        -- Mk5: Zann Box 3
		[6] = "combat_death_watch_sun_3",        -- Mk6: Black Sun Box 3
	},

	-- Marks 2-3 require any faction's Box 2
	box2Required = { [2] = true, [3] = true },

	-- Per-mark configuration
	marks = {
		-- Mk1: Basic missile (Xonti special - scales with box level)
		[1] = {
			cooldownMs = 3500,              -- 3.5 seconds
			minDamage = 15000,
			maxDamage = 25000,
			effectFire = "clienteffect/cbt_msl_fire_heatseeker.cef",
			effectHit = "clienteffect/combat_pt_orbitalstrike.cef",
			-- Xonti scaling bonuses per box level
			xontiScaling = {
				[1] = { damageBonus = 0, cooldownReduction = 0 },
				[2] = { damageBonus = 0.10, cooldownReduction = 0 },      -- +10% damage
				[3] = { damageBonus = 0.20, cooldownReduction = 500 },    -- +20% damage, -0.5s CD
				[4] = { damageBonus = 0.30, cooldownReduction = 1000 },   -- +30% damage, -1s CD
			},
		},
		-- Mk2: Enhanced missile (Any Box 2)
		[2] = {
			cooldownMs = 3000,              -- 3.0 seconds
			minDamage = 35000,
			maxDamage = 55000,
			effectFire = "clienteffect/cbt_msl_fire_heatseeker.cef",
			effectHit = "clienteffect/combat_pt_orbitalstrike.cef",
		},
		-- Mk3: Deathblow missile - kills incapacitated targets (Any Box 2)
		[3] = {
			cooldownMs = 5000,              -- 5.0 seconds
			minDamage = 5000,               -- Low damage (execution focused)
			maxDamage = 10000,
			effectFire = "clienteffect/cbt_msl_fire_heatseeker.cef",
			effectHit = "clienteffect/combat_pt_orbitalstrike.cef",
			deathblow = true,               -- Special: kills incapped targets
		},
		-- Mk4: Disorientation missile (Krrsantan Box 3)
		[4] = {
			cooldownMs = 8000,              -- 8.0 seconds
			minDamage = 5000,
			maxDamage = 10000,
			effectFire = "clienteffect/cbt_msl_fire_heatseeker.cef",
			effectHit = "clienteffect/cbt_msl_impact_heatseeker.cef",
			stunDurationSec = 5,            -- 5 second stun
			blindDurationSec = 5,           -- 5 second blind
			hudDisableMs = 20000,           -- 20 second HUD disable
		},
		-- Mk5: Anti-vehicle missile (Zann Box 3)
		[5] = {
			cooldownMs = 10000,             -- 10.0 seconds
			minDamage = 8000,
			maxDamage = 15000,
			effectFire = "clienteffect/cbt_msl_fire_heatseeker.cef",
			effectHit = "clienteffect/cbt_msl_impact_heatseeker.cef",
			destroysJetpack = true,         -- Special: destroys target's jetpack
		},
		-- Mk6: Concussion missile (Black Sun Box 3)
		[6] = {
			cooldownMs = 6000,              -- 6.0 seconds
			minDamage = 8000,
			maxDamage = 15000,
			effectFire = "clienteffect/cbt_msl_fire_heatseeker.cef",
			effectHit = "clienteffect/cbt_msl_impact_heatseeker.cef",
			rootDurationSec = 5,            -- 5 second root
			knockdown = true,               -- Knockdown on hit
		},
	},
}

-- ============================================================================
-- MK4 GROUND STRIKER C++ PROCESSING CONFIG
-- ============================================================================
-- Configuration for Mk4 missile effects processed in C++.
-- These values are loaded by DeathWatchManagerImplementation::loadLuaConfig()

mk4Config = {
	stunDuration = 5,           -- Stun (dizzy) duration in seconds
	blindDuration = 5,          -- Blind duration in seconds
	hudDisableDuration = 20,    -- HUD disable duration in seconds
}

-- ============================================================================
-- /dwEscape CONFIGURATION (Zann Consortium Box 3)
-- ============================================================================
-- Emergency escape ability - destroys jetpack for high-speed getaway

dwEscapeConfig = {
	skillRequired = "combat_death_watch_zan_3",
	cooldownMs = 3600000,               -- 1 hour cooldown

	-- Speed settings
	speedBuff = 8,                      -- Speed multiplier during escape
	speedBuffDuration = 30,             -- Duration in seconds
	minSpeedThreshold = 20,             -- Speed below this triggers explosion

	-- Damage to player when jetpack explodes
	healthDamage = 500,
	actionDamage = 500,

	-- Effects
	fogEffect = "object/static/particle/pt_waterfall_mist.iff",
	explosionEffect = "clienteffect/avatar_explosion_01.cef",
	fireParticle = "object/static/particle/pt_burning_smokeandembers_large.iff",
	fireCount = 8,                      -- Number of fire particles to spawn
	fireSpreadRadius = 8,               -- Radius for fire particle spread
	fireDurationMin = 30,               -- Min seconds before fire despawns
	fireDurationMax = 60,               -- Max seconds before fire despawns

	-- Speed check interval
	speedCheckIntervalMs = 500,         -- Check speed every 500ms
}

-- ============================================================================
-- /dwCharge CONFIGURATION (Krrsantan Box 3)
-- ============================================================================
-- Berserk mode - increased damage, reduced defense

dwChargeConfig = {
	skillRequired = "combat_death_watch_krr_3",
	cooldownMs = 3600000,               -- 1 hour cooldown
	durationSec = 30,                   -- 30 second duration

	-- Base modifiers (percentages)
	baseDamageBonus = 25,               -- +25% damage
	baseDefensePenalty = 40,            -- -40% defense

	-- Caps
	maxDamageBonus = 35,                -- Cap at +35% damage
	maxDefensePenalty = 50,             -- Cap at -50% defense

	-- Skill scaling (dw_potency increases damage, dw_evasion reduces defense penalty)
	-- Formula: finalDamage = baseDamageBonus + (dw_potency * potencyDamageScale)
	-- Formula: finalDefense = baseDefensePenalty - (dw_evasion * evasionDefenseScale)
	potencyDamageScale = 0.1,           -- +0.1% damage per dw_potency point
	evasionDefenseScale = 0.1,          -- -0.1% defense penalty per dw_evasion point
	minDefensePenalty = 20,             -- Minimum defense penalty (can't go below this)

	-- Effects
	activateEffect = "clienteffect/player_clone_death.cef",
	deactivateEffect = "clienteffect/pl_force_absorb_hit.cef",
}

-- ============================================================================
-- /dwFortify CONFIGURATION (Xonti Brothers Box 3)
-- ============================================================================
-- Defensive stance - reduced speed, increased defense

dwFortifyConfig = {
	skillRequired = "combat_death_watch_xon_3",
	cooldownMs = 3600000,               -- 1 hour cooldown
	durationSec = 30,                   -- 30 second duration

	-- Base modifiers
	speedBuff = 0.1,                    -- 0.1 speed multiplier (very slow)
	baseDefenseBonus = 25,              -- +25% defense

	-- Caps
	maxDefenseBonus = 50,               -- Cap at +50% defense

	-- Skill scaling (dw_deflection increases defense bonus)
	-- Formula: finalDefense = baseDefenseBonus + (dw_deflection * deflectionDefenseScale)
	deflectionDefenseScale = 0.25,      -- +0.25% defense per dw_deflection point

	-- Effects
	activateEffect = "clienteffect/pl_force_absorb_hit.cef",
	deactivateEffect = "clienteffect/pl_force_absorb_hit.cef",
}

-- ============================================================================
-- /dwCloak CONFIGURATION (Black Sun Box 3)
-- ============================================================================
-- Stealth/disruption ability - speed boost, fog, disables enemy HUDs

dwCloakConfig = {
	skillRequired = "combat_death_watch_sun_3",
	cooldownMs = 3600000,               -- 1 hour cooldown (per faction)
	durationSec = 30,                   -- 30 second duration

	-- Speed boost
	speedBuff = 3,                      -- Speed multiplier while cloaked

	-- HUD disable settings
	hudDisableDurationSec = 20,         -- 20 seconds of HUD disable for enemies

	-- Area of effect
	baseRadius = 32,                    -- Base radius for HUD disable
	maxRadius = 64,                     -- Maximum radius with skill bonus

	-- Skill scaling (dw_subterfuge increases radius)
	-- Formula: radius = baseRadius + (dw_subterfuge * subterfugeRadiusScale)
	-- Capped at maxRadius
	subterfugeRadiusScale = 0.32,       -- +0.32m per dw_subterfuge point (100 skill = +32m = 64m total)

	-- Fog settings
	fogEffect = "object/static/particle/pt_waterfall_mist.iff",
	fogCount = 12,                      -- Number of fog particles
	fogSpreadRadius = 16,               -- Radius for fog spread around caster

	-- Effects
	activateEffect = "clienteffect/pl_force_cloak.cef",
}

-- ============================================================================
-- /dwWarStatus CONFIGURATION
-- ============================================================================
-- Quick war status display

dwWarStatusConfig = {
	skillRequired = "combat_death_watch_novice",
	-- No cooldown for status check
}

-- ============================================================================
-- SKILL RESTRICTIONS
-- ============================================================================
-- Skills that Death Watch players are forbidden from learning.
-- This is enforced via a C++ hook in SkillManager::fulfillsSkillPrerequisites
-- which calls DeathWatchManager::canLearnSkill() in Lua.
--
-- Use exact skill names (e.g., "science_medic_master") or prefixes with wildcard
-- handling in the canLearnSkill function below.

dwRestrictedSkills = {
	-- Block master medic (healing conflicts with DW combat role)
	"science_medic_master",

	-- Block BH Investigation tree (player/Jedi tracking conflicts with DW bounty system)
	-- Using prefix to block all 4 boxes: investigation_01, _02, _03, _04
	"combat_bountyhunter_investigation_",
}

-- ============================================================================
-- JETPACK COMBAT ABILITIES
-- ============================================================================
-- Configuration for DW jetpack-specific combat commands.
-- These are dedicated attacks for DW vs DW aerial combat.
-- Players must be on DW jetpack and target enemy DW faction members.

dwJetpackCombatConfig = {
	-- Damage modifiers while on jetpack
	rangedDamageMultiplier = 0.75,      -- 25% damage reduction for ranged
	meleeDamageMultiplier = 0.85,       -- 15% damage reduction for melee
	accuracyPenalty = -25,              -- Accuracy penalty for both weapon types

	-- Range limits
	rangedRange = 64,                   -- Maximum range for ranged abilities
	meleeRange = 32,                    -- Maximum range for melee abilities

	-- Skill mods used for duration/effect scaling
	offensiveMod = "dw_debuff_potency",     -- Attacker's mod increases effect duration/strength
	defensiveMod = "dw_debuff_resistance",  -- Target's mod reduces effect duration/strength

	-- Skill requirement
	skillRequired = "combat_death_watch_novice",

	-- ========== RANGED ABILITIES (64m) ==========
	dwRangedShot = {
		cooldownMs = 0,
		damageType = "split",           -- 50/50 Health/Action
		weaponType = "ranged",
	},

	dwHeadshot = {
		cooldownMs = 15000,             -- 15 seconds
		damageType = "health",          -- Health only
		accuracyMod = -20,              -- Additional accuracy penalty
		damageMod = 1.5,                -- 50% bonus damage
		weaponType = "ranged",
	},

	dwActionDrain = {
		cooldownMs = 10000,             -- 10 seconds
		damageType = "action",          -- Action only
		weaponType = "ranged",
	},

	dwWoundingShot = {
		cooldownMs = 30000,             -- 30 seconds
		healthWound = 200,              -- Base health wound (scales with skill)
		actionWound = 200,              -- Base action wound (scales with skill)
		noDamage = true,                -- No direct damage
		weaponType = "ranged",
	},

	dwShellShock = {
		cooldownMs = 45000,             -- 45 seconds
		battleFatigue = 500,            -- Base BF applied (scales with skill)
		damageMod = 0.3,                -- 30% normal damage
		weaponType = "ranged",
	},

	dwDisarmingShot = {
		cooldownMs = 60000,             -- 60 seconds
		baseDurationSec = 5,            -- Base 5 second disarm (scales with skill)
		attacksOnly = true,             -- Only prevents attacks, not buffs/heals
		weaponType = "ranged",
	},

	-- ========== MELEE ABILITIES (32m) ==========
	dwMeleeStrike = {
		cooldownMs = 0,
		damageType = "split",           -- 50/50 Health/Action
		weaponType = "melee",
	},

	dwLunge = {
		cooldownMs = 20000,             -- 20 seconds
		dismounts = true,               -- Dismounts target from jetpack
		damageType = "split",
		weaponType = "melee",
	},

	dwRootingStrike = {
		cooldownMs = 30000,             -- 30 seconds
		baseDurationSec = 5,            -- Base 5 second root (scales with skill)
		damageType = "split",
		weaponType = "melee",
	},

	dwCripplingBlow = {
		cooldownMs = 20000,             -- 20 seconds
		speedDebuff = 0.5,              -- 50% speed reduction
		baseDurationSec = 8,            -- Base 8 second duration (scales)
		damageType = "split",
		weaponType = "melee",
	},

	dwShatteringStrike = {
		cooldownMs = 45000,             -- 45 seconds
		armorReduction = 500,           -- Armor reduction amount
		baseDurationSec = 15,           -- Base 15 second duration (scales)
		damageType = "split",
		weaponType = "melee",
	},

	dwJetpackBuster = {
		cooldownMs = 90000,             -- 90 seconds
		backfireChance = 0.20,          -- 20% chance to destroy YOUR jetpack
		destroysJetpack = true,         -- Destroys target's jetpack
		knockdown = true,               -- Knocks target down
		speedDebuff = 0.25,             -- 75% speed reduction (0.25 = 25% of normal)
		baseDurationSec = 10,           -- Speed debuff duration
		damageType = "split",
		weaponType = "melee",
	},

	-- Visual effects
	effects = {
		rangedFire = "clienteffect/cbt_msl_fire_heatseeker.cef",
		rangedHit = "clienteffect/combat_pt_orbitalstrike.cef",
		meleeHit = "clienteffect/combat_onehand_miss_fly.cef",
		dismount = "clienteffect/combat_vehicle_disable.cef",
		root = "clienteffect/sm_rooted.cef",
		disarm = "clienteffect/combat_special_attacker_dizzy.cef",
		jetpackDestroy = "clienteffect/avatar_explosion_01.cef",
	},
}

-- ============================================================================
-- MOUNTED COMBAT (SADDLE SHOT)
-- ============================================================================
-- Configuration for combat abilities allowed while mounted on a Death Watch jetpack.
-- This is a unique DW ability - normally combat is blocked while mounted.
--
-- Commands in this whitelist can be used while on a DW jetpack, but with penalties.

dwMountedCombatConfig = {
	-- Damage multiplier while mounted (0.75 = 25% less damage)
	damageMultiplier = 0.75,

	-- Accuracy modifier while mounted (flat penalty, e.g., -25)
	accuracyModifier = -25,

	-- Whitelist of commands allowed while mounted on DW jetpack
	-- Use lowercase command names as they appear in the command system
	allowedCommands = {
		"overchargeshot1",
		"overchargeshot2",
		-- Add more commands here as needed:
		-- "pointblankshot1",
		-- "pointblankshot2",
		-- "berserk1",
	},
}

-- Build a lookup table for fast command checking
dwMountedCombatCommandLookup = {}
for _, cmd in ipairs(dwMountedCombatConfig.allowedCommands) do
	dwMountedCombatCommandLookup[cmd] = true
end

-- ============================================================================
-- HEAVY WEAPONS CONFIGURATION
-- ============================================================================
-- Configuration for DW heavy weapon combat (Rocket, Particle, Acid, Lightning)
-- These weapons are intercepted by DeathWatchManager when fired at DW targets
--
-- Key Rules:
-- - Attacker must be ON THE GROUND (cannot fire from jetpack)
-- - Rocket Launcher & Particle Beam: Target must be ON A DW JETPACK
-- - Acid Beam & Lightning Beam: Target must be ON THE GROUND
-- - Wrong target type = automatic miss

dwHeavyWeaponsConfig = {
	-- ========== HIT CALCULATION ==========
	baseAccuracy = 60,              -- Base 60% hit chance
	accBonus = 0.003,               -- +0.3% per dw_acc point
	evasionPenalty = 0.0025,        -- -0.25% per dw_evasion point
	minHitChance = 30,              -- Floor: 30% minimum
	maxHitChance = 90,              -- Cap: 90% maximum

	-- ========== DAMAGE CALCULATION ==========
	potBonus = 0.005,               -- +0.5% damage per dw_pot point
	deflectionReduction = 0.003,    -- -0.3% damage per dw_deflection point
	toughnessCap = 50,              -- dw_toughness caps at 50%

	-- ========== WEAPON MULTIPLIERS ==========
	-- Tier 1 (Ground targets, have special abilities): 0.7x
	-- Tier 2 (Aerial targets, pure damage): 1.0x
	weaponMultipliers = {
		rocket = 1.0,               -- Rocket Launcher (aerial)
		particle = 1.0,             -- Particle Beam (aerial)
		acid = 0.7,                 -- Acid Beam (ground, has Acid Strike)
		lightning = 0.7,            -- Lightning Beam (ground, has Orbital Strike)
	},

	-- ========== PARTICLE BEAM: MAND'ALOR'S GRIP ==========
	-- Debuff that ramps UP, holds at PEAK, then ramps DOWN
	-- Flow: HIT → Tick1-4 (ramp up) → PEAK hold → Tick4-1 (ramp down) → CLEAR → immunity
	particleBeam = {
		-- Per-tick defense reduction
		basePerTick = 3,            -- 3% base reduction per tick
		potTickScaling = 0.03,      -- +0.03% per attacker dw_pot
		recoveryTickScaling = 0.02, -- -0.02% per defender dw_recovery
		minTickReduction = 1,       -- Floor: 1% minimum per tick
		maxTickReduction = 6,       -- Cap: 6% maximum per tick
		tickIntervalMs = 5000,      -- 5 seconds between ticks
		numTicks = 4,               -- 4 ticks up, 4 ticks down

		-- Peak duration (time held at max reduction)
		basePeakDuration = 10,      -- 10 seconds base
		potPeakScaling = 0.15,      -- +0.15s per attacker dw_pot
		recoveryPeakScaling = 0.12, -- -0.12s per defender dw_recovery
		minPeakDuration = 5,        -- Floor: 5 seconds
		maxPeakDuration = 25,       -- Cap: 25 seconds

		-- Immunity after effect clears
		baseImmunity = 120,         -- 2 minutes base immunity
		recoveryImmunityBonus = 0.5,-- +0.5s per defender dw_recovery

		-- Effects
		tickEffect = "clienteffect/pl_force_choke.cef",
		peakEffect = "clienteffect/pl_force_choke_grip.cef",
	},

	-- ========== COMBAT MESSAGES ==========
	messages = {
		-- Hit messages
		rocketHit = "Your rocket strikes the target!",
		particleHit = "Your particle beam connects!",
		acidHit = "Your acid beam sears the target!",
		lightningHit = "Your lightning beam electrifies the target!",

		-- Miss messages
		miss = "Your shot misses the target!",
		wrongTargetAerial = "Your weapon is designed for aerial targets.",
		wrongTargetGround = "Your weapon is designed for ground targets.",

		-- Particle beam special
		particleGripApplied = "Mand'alor's Grip takes hold!",
		particleGripImmune = "Target is immune to Mand'alor's Grip.",
		particleGripFading = "Mand'alor's Grip is fading...",
		particleGripCleared = "Mand'alor's Grip has released.",

		-- Certification messages
		notCertifiedRocket = "You need Zann Consortium training to use the Rocket Launcher against Death Watch.",
		notCertifiedParticle = "You need Krrsantan training to use the Particle Beam against Death Watch.",
		notCertifiedAcid = "You need advanced Krrsantan training to use the Acid Beam against Death Watch.",
		notCertifiedLightning = "You need Black Sun training to use the Lightning Beam against Death Watch.",
	},

	-- ========== VISUAL EFFECTS ==========
	effects = {
		rocketFire = "clienteffect/cbt_msl_fire_heatseeker.cef",
		rocketHit = "clienteffect/combat_pt_orbitalstrike.cef",
		particleFire = "clienteffect/cbt_particle_beam_fire.cef",
		particleHit = "clienteffect/cbt_particle_beam_hit.cef",
		acidFire = "clienteffect/cbt_acid_beam_fire.cef",
		acidHit = "clienteffect/cbt_acid_beam_hit.cef",
		lightningFire = "clienteffect/cbt_lightning_beam_fire.cef",
		lightningHit = "clienteffect/cbt_lightning_beam_hit.cef",
	},
}

-- ============================================================================
-- DEATH WATCH MANAGER FUNCTIONS
-- ============================================================================
-- These functions are called from C++ via DirectorManager

DeathWatchManager = {}

--[[
	Called by SkillManager::fulfillsSkillPrerequisites to check if a Death Watch
	player is allowed to learn a specific skill.

	@param pCreature - The player attempting to learn the skill
	@param skillName - The skill being learned
	@return true if allowed, false if blocked
]]
function DeathWatchManager:canLearnSkill(pCreature, skillName)
	if pCreature == nil or skillName == nil then
		return true
	end

	-- Check if player is Death Watch
	local pGhost = CreatureObject(pCreature):getPlayerObject()
	if pGhost == nil then
		return true
	end

	local dwFaction = PlayerObject(pGhost):getDwFaction()
	if dwFaction == nil or dwFaction == 0 then
		-- Not Death Watch, no restrictions
		return true
	end

	-- Check against restricted skills list
	for _, restricted in ipairs(dwRestrictedSkills) do
		-- Check for exact match
		if skillName == restricted then
			CreatureObject(pCreature):sendSystemMessage("\\#FF6666Death Watch operatives are forbidden from learning this skill.")
			return false
		end

		-- Check for prefix match (if restricted skill ends with underscore, treat as prefix)
		if string.sub(restricted, -1) == "_" then
			if string.find(skillName, "^" .. restricted) then
				CreatureObject(pCreature):sendSystemMessage("\\#FF6666Death Watch operatives are forbidden from learning this skill.")
				return false
			end
		end
	end

	return true
end

--[[
	Check if a command is allowed while mounted on a Death Watch jetpack.
	Called from C++ CombatManager to validate mounted combat.

	@param commandName - The command being executed (lowercase)
	@return true if allowed, false if blocked
]]
function DeathWatchManager:isCommandAllowedWhileMounted(commandName)
	if commandName == nil then
		return false
	end

	-- Check against the whitelist lookup table
	return dwMountedCombatCommandLookup[commandName] == true
end

--[[
	Get the damage multiplier for mounted combat.
	Called from C++ CombatManager when calculating damage.

	@return float - Damage multiplier (e.g., 0.75 for 25% less damage)
]]
function DeathWatchManager:getMountedCombatDamageMultiplier()
	return dwMountedCombatConfig.damageMultiplier or 0.75
end

--[[
	Get the accuracy modifier for mounted combat.
	Called from C++ CombatManager when calculating accuracy.

	@return int - Accuracy modifier (e.g., -25 for 25 less accuracy)
]]
function DeathWatchManager:getMountedCombatAccuracyModifier()
	return dwMountedCombatConfig.accuracyModifier or -25
end

--[[
	Get the cooldown duration for a DW command.
	Called from C++ DeathWatchManager::getCommandCooldown

	Looks up cooldown in the following config tables:
	- dwJetpackCombatConfig.<commandName>.cooldownMs
	- dwChargeConfig.cooldownMs (for dwCharge)
	- dwCloakConfig.cooldownMs (for dwCloak)
	- dwFortifyConfig.cooldownMs (for dwFortify)
	- dwEscapeConfig.cooldownMs (for dwEscape)
	- dwWhistlingBirdConfig.marks[mark].cooldownMs (for dwWhistlingBird)
	- dwIdentifyConfig.globalCooldownMs (for dwIdentify)

	@param commandName - The command name (e.g., "dwHeadshot", "dwCloak")
	@return int - Cooldown in milliseconds, or 0 if not found
]]
function DeathWatchManager:getCommandCooldown(commandName)
	if commandName == nil then
		return 0
	end

	-- Jetpack combat abilities
	if dwJetpackCombatConfig[commandName] ~= nil then
		return dwJetpackCombatConfig[commandName].cooldownMs or 0
	end

	-- Box 3 special abilities
	if commandName == "dwCharge" then
		return dwChargeConfig.cooldownMs or 0
	elseif commandName == "dwCloak" then
		return dwCloakConfig.cooldownMs or 0
	elseif commandName == "dwFortify" then
		return dwFortifyConfig.cooldownMs or 0
	elseif commandName == "dwEscape" then
		return dwEscapeConfig.cooldownMs or 0
	end

	-- Identify command
	if commandName == "dwIdentify" then
		return dwIdentifyConfig.globalCooldownMs or 0
	end

	-- Not found
	return 0
end

-- ============================================================================
-- HIDDEN TEMPLATE BONUSES
-- ============================================================================
-- Secret bonuses granted to players based on their combat profession templates.
-- These are NOT displayed to players - they must discover them through gameplay.
-- Design goal: Every profession is equally valuable, no single "best" template.
--
-- Bonus Types:
--   damageReduction - % damage reduction when DEFENDING
--   evasionChance   - % chance to fully evade when DEFENDING
--   damageBonus     - % damage bonus when ATTACKING
--   accuracyBonus   - flat accuracy bonus when ATTACKING
--   durationBonus   - % bonus to buff/effect durations
--   cooldownReduction - % reduction to cooldowns
--   specialEffect   - unique effect (handled in C++)
--
-- Skills checked: Master box grants full bonus, lower tiers grant partial.
-- Format: skill = "combat_X_master" or "combat_X_speed_04" etc.

dwHiddenTemplateBonuses = {
	-- ========== GLOBAL CAPS ==========
	caps = {
		maxDamageReduction = 0.40,      -- 40% max damage reduction
		maxEvasionChance = 0.25,        -- 25% max evasion chance
		maxDamageBonus = 0.50,          -- 50% max damage bonus
		maxDurationBonus = 0.50,        -- 50% max duration bonus
		maxCooldownReduction = 0.30,    -- 30% max cooldown reduction
		diminishingReturnsFactor = 0.70, -- Second source of same type = 70%
	},

	-- ========== WHISTLING BIRD MK1 (Energy, Air-to-Air) ==========
	mk1 = {
		defender = {
			{ skill = "combat_pistol_master", evasionChance = 0.15 },           -- Pistoleer: 15% evade
		},
		attacker = {
			-- No specific bonuses (baseline weapon)
		},
	},

	-- ========== WHISTLING BIRD MK2 (Blast, Seeker, AoE) ==========
	mk2 = {
		defender = {
			{ skill = "combat_bountyhunter_master", specialEffect = "reducedLockTime", value = 0.20 }, -- BH: 20% harder to lock
		},
		attacker = {
			{ skill = "combat_carbine_master", specialEffect = "aoeRadiusBonus", value = 0.15 },       -- Carbineer: +15% AoE radius
		},
	},

	-- ========== WHISTLING BIRD MK3 (Bunker Buster, Fire DoT) ==========
	mk3 = {
		defender = {
			{ skill = "combat_polearm_master", damageReduction = 0.40 },        -- Pikeman: 40% fire DoT reduction
		},
		attacker = {
			-- No specific bonuses (siege weapon)
		},
	},

	-- ========== WHISTLING BIRD MK4 (Ground AoE, Stun/Blind/HUD) ==========
	mk4 = {
		defender = {
			{ skill = "combat_2hsword_master", specialEffect = "stunReduction", value = 0.35 },       -- Swordsman: 35% stun reduction
			{ skill = "combat_unarmed_master", specialEffect = "hudDisableReduction", value = 0.40 }, -- TKM: 40% HUD disable reduction
		},
		attacker = {
			-- No bonuses (friendly fire risk discourages optimization)
		},
	},

	-- ========== WHISTLING BIRD MK5 (Jetpack Killer) ==========
	mk5 = {
		defender = {
			{ skill = "combat_bountyhunter_master", damageReduction = 0.25 },   -- BH: 25% condition damage reduction
		},
		attacker = {
			{ skill = "combat_rifleman_master", damageBonus = 0.20 },           -- Rifleman: +20% condition damage
		},
	},

	-- ========== WHISTLING BIRD MK6 (Concussive, Dismount+Root+Slow) ==========
	mk6 = {
		defender = {
			{ skill = "combat_1hsword_master", specialEffect = "rootReduction", value = 0.30 },       -- Fencer: 30% root reduction
		},
		attacker = {
			-- No specific bonuses
		},
	},

	-- ========== HEAVY ROCKET LAUNCHER (Blast, Aerial Target) ==========
	rocket = {
		defender = {
			{ skill = "combat_commando_master", damageReduction = 0.20 },       -- Commando: 20% blast reduction
		},
		attacker = {
			{ skill = "combat_2hsword_master", damageBonus = 0.15 },            -- Swordsman: +15% damage
			{ skill = "combat_commando_master", damageBonus = 0.15, accuracyBonus = 10 }, -- Commando: +15% dmg, +10 acc
		},
	},

	-- ========== HEAVY PARTICLE BEAM (Energy, Aerial, Mand'alor's Grip) ==========
	particle = {
		defender = {
			{ skill = "combat_rifleman_master", specialEffect = "gripReduction", value = 0.25 },      -- Rifleman: 25% grip effect reduction
			{ skill = "science_smuggler_master", evasionChance = 0.15 },        -- Smuggler: 15% full evade
		},
		attacker = {
			-- No specific bonuses (balanced debuff weapon)
		},
	},

	-- ========== HEAVY ACID BEAM (Acid, Ground, DoT+Corrosion) ==========
	acid = {
		defender = {
			{ skill = "combat_carbine_master", specialEffect = "dotReduction", value = 0.30 },        -- Carbineer: 30% DoT reduction
			{ skill = "science_doctor_master", specialEffect = "corrosionReduction", value = 0.40 },  -- Doctor: 40% corrosion duration
		},
		attacker = {
			-- No specific bonuses
		},
	},

	-- ========== HEAVY LIGHTNING BEAM (Electricity, Ground, Armor Decay) ==========
	lightning = {
		defender = {
			{ skill = "combat_1hsword_master", damageReduction = 0.18 },        -- Fencer: 18% damage reduction
		},
		attacker = {
			-- No specific bonuses
		},
	},

	-- ========== ORBITAL STRIKE (Lightning AoE + Root) ==========
	orbital = {
		defender = {
			{ skill = "combat_rifleman_master", specialEffect = "earlyWarning", value = 1.0 },        -- Rifleman: 1s early warning
			{ skill = "combat_polearm_master", damageReduction = 0.20 },        -- Pikeman: 20% damage reduction
			{ skill = "combat_commando_master", damageReduction = 0.15 },       -- Commando: 15% damage reduction
			{ skill = "science_smuggler_master", evasionChance = 0.20 },        -- Smuggler: 20% half damage chance
		},
		attacker = {
			{ skill = "combat_commando_master", specialEffect = "selfDamageReduction", value = 0.50 }, -- Commando: 50% less self-damage
		},
	},

	-- ========== ACID STRIKE (Acid AoE + DoT) ==========
	acidStrike = {
		defender = {
			{ skill = "combat_unarmed_master", specialEffect = "dotReduction", value = 0.35 },        -- TKM: 35% DoT reduction
			{ skill = "combat_1hsword_master", specialEffect = "repositionChance", value = 0.25 },    -- Fencer: 25% auto-reposition
		},
		attacker = {
			{ skill = "combat_polearm_master", specialEffect = "radiusBonus", value = 0.20 },         -- Pikeman: +20% radius
		},
	},

	-- ========== SERUMS (Tactical Buffs) ==========
	-- Balanced Serum (Mind Immunity Toggle)
	serumBalanced = {
		user = {
			{ skill = "science_doctor_master", durationBonus = 0.30 },          -- Doctor: +30% duration
		},
	},

	-- Combat Serum (Purge)
	serumCombat = {
		user = {
			{ skill = "combat_2hsword_master", specialEffect = "guaranteedClear", value = 1.0 },      -- Swordsman: always clears 1 state
			{ skill = "science_doctor_master", specialEffect = "alwaysSucceeds", value = 1.0 },       -- Doctor: purge always succeeds
		},
	},

	-- Emergency Serum (Damage Shield)
	serumEmergency = {
		user = {
			{ skill = "combat_pistol_master", cooldownReduction = 0.20 },       -- Pistoleer: 20% cooldown reduction
			{ skill = "combat_medic_master", specialEffect = "mitigationBonus", value = 0.25 },       -- Combat Medic: +25% mitigation
		},
	},

	-- Adrenaline Serum (Missile Reflector)
	serumAdrenaline = {
		user = {
			{ skill = "combat_1hsword_master", specialEffect = "reflectBonus", value = 0.15 },        -- Fencer: +15% reflect chance
			{ skill = "combat_medic_master", durationBonus = 0.20 },            -- Combat Medic: +20% duration
		},
	},

	-- Targeting Boost (+Accuracy for Whistlers)
	serumTargeting = {
		user = {
			{ skill = "combat_pistol_master", durationBonus = 0.25 },           -- Pistoleer: +25% duration
			{ skill = "combat_medic_master", specialEffect = "accuracyCapBonus", value = 10 },        -- Combat Medic: +10 acc cap
		},
	},

	-- Bacta Denial Field (AoE Stimpack Block)
	-- TKM as DEFENDER (victim) gets 25% shorter duration when hit by this
	serumBactaDenial = {
		defender = {
			{ skill = "combat_unarmed_master", specialEffect = "durationReduction", value = 0.25 }, -- TKM: 25% shorter duration
		},
	},

	-- ========== ALL WHISTLERS (Global Bonuses) ==========
	whistlerGlobal = {
		defender = {
			{ skill = "combat_polearm_master", damageReduction = 0.10 },        -- Pikeman: 10% all whistler reduction
			{ skill = "science_smuggler_master", specialEffect = "dudChance", value = 0.12 },         -- Smuggler: 12% dud chance
		},
		attacker = {
			-- No global attacker bonuses
		},
	},
}

-- =============================================================================
-- WRAPPER FUNCTIONS FOR C++ DEATHWATCHMANAGER
-- These methods call the registered DirectorManager functions to access C++
-- =============================================================================

--[[
	Calculate mitigated damage after armor resistance
	@param pAttacker - The attacker
	@param pDefender - The defender
	@param rawDamage - Raw damage before mitigation
	@param damageType - Damage type constant (ACID=128, ELECTRICITY=256, etc.)
	@param armorPiercing - Armor piercing level (0-3)
	@return Mitigated damage
]]
function DeathWatchManager:calculateMitigatedDamage(pAttacker, pDefender, rawDamage, damageType, armorPiercing)
	if pAttacker == nil or pDefender == nil then
		return rawDamage
	end
	return getDwMitigatedDamage(pAttacker, pDefender, rawDamage, damageType, armorPiercing)
end

--[[
	Apply acid DoT to a target
	@param pAttacker - The attacker applying the DoT
	@param pTarget - The target receiving the DoT
	@param totalDamage - Total damage over the duration
	@param duration - Duration in seconds
	@param tickInterval - Tick interval in milliseconds
]]
function DeathWatchManager:applyAcidDot(pAttacker, pTarget, totalDamage, duration, tickInterval)
	if pAttacker == nil or pTarget == nil then
		return
	end
	applyDwAcidDot(pAttacker, pTarget, totalDamage, duration, tickInterval)
end

--[[
	Apply acid corrosion to target's armor
	@param pAttacker - The attacker
	@param pTarget - The target
	@param baseDamage - Base condition damage
	@param chestMultiplier - Multiplier for chest pieces
	@param legsMultiplier - Multiplier for leg pieces
	@return Total condition damage applied
]]
function DeathWatchManager:applyAcidCorrosion(pAttacker, pTarget, baseDamage, chestMultiplier, legsMultiplier)
	if pAttacker == nil or pTarget == nil then
		return 0
	end
	return applyDwAcidCorrosion(pAttacker, pTarget, baseDamage, chestMultiplier, legsMultiplier)
end

--[[
	Get hidden damage reduction for a defender based on combat profession
	@param pDefender - The defending player
	@param attackType - Attack type key (mk1, mk2, rocket, orbital, acidStrike, etc.)
	@return Damage reduction multiplier (0.0 to 0.40)
]]
function DeathWatchManager:getHiddenDamageReduction(pDefender, attackType)
	if pDefender == nil or attackType == nil then
		return 0
	end
	return getDwHiddenDamageReduction(pDefender, attackType)
end

--[[
	Get hidden evasion chance for a defender based on combat profession
	@param pDefender - The defending player
	@param attackType - Attack type key
	@return Evasion chance (0.0 to 0.25)
]]
function DeathWatchManager:getHiddenEvasionChance(pDefender, attackType)
	if pDefender == nil or attackType == nil then
		return 0
	end
	return getDwHiddenEvasionChance(pDefender, attackType)
end

--[[
	Get hidden special effect value for a player
	@param pPlayer - The player
	@param attackType - Attack type key (mk1, mk2, orbital, serumEmergency, etc.)
	@param effectName - Effect name (dudChance, radiusBonus, mitigationBonus, etc.)
	@param isDefender - true if player is defending, false if attacking/using
	@return Effect value or 0 if not found
]]
function DeathWatchManager:getHiddenSpecialEffect(pPlayer, attackType, effectName, isDefender)
	if pPlayer == nil or attackType == nil or effectName == nil then
		return 0
	end
	return getDwHiddenSpecialEffect(pPlayer, attackType, effectName, isDefender)
end

-- ============================================================================
-- DEATH WATCH SAFE CLONING - FACTION HQ CONFIGURATION
-- ============================================================================
-- TODO: Configure faction HQ cloning locations for the DW safe clone system.
--
-- When a Death Watch player dies, they see a custom clone selection showing:
--   1. HOME - Their pre-designated cloning facility (always first if set)
--   2. HQ - Their faction's HQ cloning point (always second)
--   3. Random 10 - Other NPC cloners sorted by threat level
--
-- Threat levels indicate enemy DW players within 1000m of each location:
--   [CLEAR] = 0 enemies, [CAUTION] = 1-2 enemies, [DANGER] = 3+ enemies
--
-- To enable faction HQ in the clone list, fill in the coordinates below.
-- Each entry needs:
--   zone = planet name (e.g., "endor", "tatooine")
--   x = X world coordinate
--   y = Y world coordinate
--   z = Z world coordinate (height)
--   cellId = Cell ID if indoors (0 for outdoor spawn)
--
-- Example:
--   dwFactionHQ = {
--       [1] = { zone = "endor", x = 1234, y = 5678, z = 0, cellId = 0 },  -- Xon (Xonti Brothers)
--       [2] = { zone = "tatooine", x = 1234, y = 5678, z = 0, cellId = 0 },  -- Sun (Black Sun)
--       [3] = { zone = "kashyyyk", x = 1234, y = 5678, z = 0, cellId = 0 },  -- Krr (Black Krrsantan)
--       [4] = { zone = "corellia", x = 1234, y = 5678, z = 0, cellId = 0 },  -- Zan (Zann Consortium)
--   }
--
-- Faction IDs: 1 = Xon, 2 = Sun, 3 = Krr, 4 = Zan (see DeathWatchFactions.h)

dwFactionHQ = {
	-- [1] = { zone = "TODO", x = 0, y = 0, z = 0, cellId = 0 },  -- Xon (Xonti Brothers)
	-- [2] = { zone = "TODO", x = 0, y = 0, z = 0, cellId = 0 },  -- Sun (Black Sun)
	-- [3] = { zone = "TODO", x = 0, y = 0, z = 0, cellId = 0 },  -- Krr (Black Krrsantan)
	-- [4] = { zone = "TODO", x = 0, y = 0, z = 0, cellId = 0 },  -- Zan (Zann Consortium)
}

-- ============================================================================
-- /deathWatch COMMAND CONFIGURATION (Box 3 Faction Abilities)
-- ============================================================================
-- Hidden faction-specific utility command available to DW members with Box 3 skills.
--
-- Effects by faction:
-- - Zann Consortium: Enemy Scan (64m intel report, scales with dw_subterfuge)
-- - Black Krrsantan: Savage Sprint (2.5x speed buff for 30-60 seconds)
-- - Black Sun: Shadow Protocol (PEF immunity for 30+ seconds)
-- - Xonti Brothers: Genetic Harvest (special forage, no cooldown)

dwRank3CommandConfig = {
	-- Cooldowns in seconds
	cooldowns = {
		espionage_matrix = 86400, -- Zann: 24 hours (strategic ability)
		savage_sprint = 600,     -- Krrsantan: 10 minutes
		shadow_protocol = 600,   -- Black Sun: 10 minutes
		-- genetic_harvest has no cooldown (balanced via loot percentages)
		-- enemy_scan deprecated, replaced by espionage_matrix
	},

	-- Dev mode cooldown (seconds)
	devCooldown = 10,

	-- Savage Sprint configuration
	savageSprint = {
		speedMultiplier = 2.5,   -- 2.5x normal speed
		baseDuration = 30,       -- 30 seconds base
		maxDuration = 60,        -- 60 seconds maximum
		skillMod = "dw_potency", -- Skill mod for duration bonus
	},

	-- Shadow Protocol configuration
	shadowProtocol = {
		baseDuration = 30,           -- 30 seconds base
		skillMod = "dw_subterfuge",  -- Skill mod for duration bonus
		warningTime = 5,             -- Warning 5 seconds before end
	},

	-- Espionage Matrix configuration (Zann Consortium)
	espionageMatrix = {
		baseDuration = 1200,         -- 20 minutes base (1200 seconds)
		skillMod = "dw_subterfuge",  -- Skill mod for duration bonus
		durationPerSubterfuge = 30,  -- +30 seconds per point of subterfuge
		maxDuration = 1800,          -- 30 minutes max (1800 seconds)
	},

	-- Enemy Scan configuration (DEPRECATED - replaced by Espionage Matrix)
	enemyScan = {
		range = 64,                  -- 64 meter scan radius
		skillMod = "dw_subterfuge",  -- Skill mod for info scaling
		thresholds = {
			faction = 6,             -- 6+ subterfuge: Show faction
			rank = 8,                -- 8+ subterfuge: Show rank
			professions = 12,        -- 12+ subterfuge: Show master professions
		},
	},

	-- Genetic Harvest configuration (no cooldown)
	geneticHarvest = {
		duration = 8500,             -- 8.5 seconds forage time
		actionCost = 25,             -- Base action cost
		maxMoveDistance = 2,         -- Max meters player can move during forage
		lootGroup = "forage_genetic_harvest",
		lootLevel = 150,
	},
}

-- ============================================================================
-- DEATH WATCH SERUM/STIM COMMAND CONFIGURATION
-- ============================================================================
-- Configuration for DW serum commands that activate crafted healing items.
-- Commands allow hotbar binding while still requiring crafted stim items.
--
-- Naming convention: Mandalorian names for commands
-- - Tor'cabur (Justice Guardian) - Mind Immunity toggle
-- - Naak'kelir (Peace Bringer) - Purge (clears states)
-- - Beskar'gam (Beskar Armor) - Damage Shield
-- - Gotal'din (Death's Vengeance) - Missile Reflector
-- - Ara'novor (Defender's Defense) - Targeting Boost
-- - Hettir (Burn) - AoE Bacta Denial

dwStimConfig = {
	-- Item templates for each stim type
	templates = {
		torcabur = "object/tangible/medicine/dw_balanced_stim.iff",
		naakkelir = "object/tangible/medicine/dw_combat_stim.iff",
		beskargam = "object/tangible/medicine/dw_emergency_stim.iff",
		gotaldin = "object/tangible/medicine/dw_adrenaline_stim.iff",
		aranovor = "object/tangible/medicine/dw_poison_pack.iff",
		hettir = "object/tangible/medicine/dw_area_poison_pack.iff",
	},

	-- Skill requirements (dw_healing skill mod, displayed as "Dralshy'a")
	skillRequirements = {
		torcabur = 25,
		naakkelir = 28,
		beskargam = 29,
		aranovor = 30,
		gotaldin = 31,
		hettir = 32,
	},

	-- Cooldowns in milliseconds
	cooldowns = {
		torcabur = 90000,      -- 90 seconds
		naakkelir = 300000,    -- 5 minutes
		beskargam = 300000,    -- 5 minutes
		gotaldin = 90000,      -- 90 seconds
		aranovor = 600000,     -- 10 minutes
		hettir = 600000,       -- 10 minutes
	},

	-- HAM costs (base values before skill reduction)
	-- pool: "health", "action", "mind", or "all"
	-- skill: skill mod that reduces cost
	-- perPoint: cost reduction per skill point
	hamCosts = {
		torcabur = { pool = "action", base = 600, skill = "dw_recovery", perPoint = 5 },
		naakkelir = { pool = "action", base = 400, skill = "dw_recovery", perPoint = 4 },
		beskargam = { pool = "all", base = 900, skill = "dw_pot", perPoint = 8 },
		gotaldin = { pool = "action", base = 500, skill = "dw_deflection", perPoint = 4 },
		aranovor = { pool = "action", base = 400, skill = "dw_acc", perPoint = 3 },
		hettir = { pool = "action", base = 500, skill = "dw_subterfuge", perPoint = 4 },
	},

	-- Effect durations (in seconds)
	-- base: Starting duration
	-- skill: Skill mod that extends duration (nil for fixed)
	-- perPoint: Seconds added per skill point
	-- max: Maximum duration cap
	-- fixed: If true, duration is exactly base (not scaled)
	durations = {
		torcabur = { base = 75, skill = "dw_healing", perPoint = 1, max = 90 },
		naakkelir = nil,  -- Instant effect (purge)
		beskargam = { base = 60, fixed = true },
		gotaldin = { base = 60, skill = "dw_recovery", perPoint = 1, max = 90 },
		aranovor = { base = 20, fixed = true },
		hettir = { base = 45, fixed = true },
	},

	-- Display names (Mandalorian names for player messages)
	displayNames = {
		torcabur = "Tor'cabur",
		naakkelir = "Naak'kelir",
		beskargam = "Beskar'gam",
		gotaldin = "Gotal'din",
		aranovor = "Ara'novor",
		hettir = "Hettir",
	},

	-- Buff CRCs (reference only - actual values in BuffCRC.h)
	-- DW_MIND_IMMUNITY = 0xDEA7D1C1     (torcabur)
	-- DW_DAMAGE_SHIELD = 0xDEA7D2C2     (beskargam)
	-- DW_MISSILE_REFLECTOR = 0xDEA7D3C3 (gotaldin)
	-- DW_TARGETING_BOOST = 0xDEA7D4C4   (aranovor)
	-- DW_BACTA_DENIAL = 0xDEA7BACF      (hettir)

	-- Visual effects
	effects = {
		torcabur = {
			activate = "clienteffect/pl_force_resist_bleeding.cef",
			deactivate = "clienteffect/pl_force_absorb_hit.cef",
		},
		naakkelir = {
			use = "clienteffect/pl_force_healing.cef",
		},
		beskargam = {
			activate = "clienteffect/pl_force_armor.cef",
		},
		gotaldin = {
			activate = "clienteffect/pl_force_feedback.cef",
			reflect = "clienteffect/combat_special_defender_evade.cef",
		},
		aranovor = {
			activate = "clienteffect/pl_force_speed.cef",
		},
		hettir = {
			use = "clienteffect/combat_acid_hit.cef",
			victimHit = "clienteffect/pl_force_choke.cef",
		},
	},

	-- Messages
	messages = {
		-- Validation errors (shared)
		notDeathWatch = "You must be Death Watch to use this item.",
		insufficientSkill = "Your Dralshy'a skill is not high enough to use this item.",
		insufficientHam = "You do not have enough %s to use this item.",
		onCooldown = "You cannot use this yet. Cooldown: %d seconds remaining.",

		-- Per-stim messages
		torcabur = {
			needItem = "You need a Tor'cabur serum to use this command.",
			activate = "\\#00FFFFMental fortitude activated!",
			deactivate = "Mental fortitude deactivated.",
			expire = "\\#FFFF00Mental fortitude has worn off.",
		},
		naakkelir = {
			needItem = "You need a Naak'kelir serum to use this command.",
			success = "\\#00FF00Purge surge! Cleared: %s",
			noStates = "You have no negative states to clear.",
			partial = "\\#FFFF00Purge partially successful. Cleared: %s",
		},
		beskargam = {
			needItem = "You need a Beskar'gam serum to use this command.",
			activate = "\\#00FFFFDamage shield activated! Mitigation: %.0f%%",
			expire = "\\#FFFF00Damage shield has worn off.",
		},
		gotaldin = {
			needItem = "You need a Gotal'din serum to use this command.",
			activate = "\\#00FFFFMissile reflector online! Reflect power: %d",
			expire = "\\#FFFF00Missile reflector offline.",
			reflect = "\\#FF0000Missile reflected back at attacker!",
		},
		aranovor = {
			needItem = "You need an Ara'novor serum to use this command.",
			activate = "\\#00FFFFTargeting boost activated! +%d accuracy",
			expire = "\\#FFFF00Targeting boost has worn off.",
		},
		hettir = {
			needItem = "You need a Hettir canister to use this command.",
			success = "\\#FF0000Bacta denial field deployed! %d enemies affected.",
			noEnemies = "No valid Death Watch enemies in range.",
			victimApplied = "\\#FF0000You have been hit by a bacta denial field!",
			victimExpire = "\\#00FF00Bacta denial effect has cleared.",
		},
	},
}
