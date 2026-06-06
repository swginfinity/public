rebel_super_battle_droid = Creature:new {
	objectName = "@mob/creature_names:rebel_super_battle_droid",
	socialGroup = "rebel",
	faction = "rebel",
	mobType = MOB_ANDROID,
	level = 200,
	chanceHit = 18,
	damageMin = 1200,
	damageMax = 2300,
	baseXp = 19000,
	baseHAM = 230000,
	baseHAMmax = 230000,
	armor = 2,
	resists = {85,95,75,60,75,25,40,85,-1},--kinetic,energy,blast,heat,cold,electric,acid,stun,ls
	meatType = "",
	meatAmount = 0,
	hideType = "",
	hideAmount = 0,
	boneType = "",
	boneAmount = 0,
	milk = 0,
	tamingChance = 0,
	ferocity = 0,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = KILLER + NOINTIMIDATE,
	optionsBitmask = AIENABLED,
	diet = NONE,
	scale = 1.40,

	templates = {
		"object/mobile/super_battle_droid.iff",
	},
	lootGroups = {
		{
			groups = {
				{group = "death_watch_bunker_commoners", chance = 6000000},   -- 25% * 60% = 15%
				{group = "stap_speeder", chance = 4000000}     -- 25% * 40% = 10%
			},
			lootChance = 2500000    -- 25% chance of this group
		},
		{
			groups = {
				{group = "airstrike_group", chance = 100 * (100000)},
			},
			lootChance = 8 * (100000),
		},
		{
			groups = {
				{group = "groundstrike_group", chance = 100 * (100000)},
			},
			lootChance = 25 * (100000),
		},
	},
	conversationTemplate = "",
	primaryWeapon = "object/weapon/ranged/droid/droid_droideka_ranged.iff",
	secondaryWeapon = "none",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = { {"creaturerangedattack",""} },
	secondaryAttacks = { },
	defaultWeapon = "object/weapon/ranged/droid/droid_droideka_ranged.iff",
	defaultAttack = "creaturerangedattack",
}

CreatureTemplates:addCreatureTemplate(rebel_super_battle_droid, "rebel_super_battle_droid")
