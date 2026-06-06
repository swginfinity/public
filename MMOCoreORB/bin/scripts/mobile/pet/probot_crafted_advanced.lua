probot_crafted_advanced = Creature:new {
	objectName = "@droid_name:probe_droid_crafted_advanced",
	socialGroup = "",
	faction = "",
	level = 4,
	mobType = MOB_DROID,
	chanceHit = 0.24,
	damageMin = 182,
	damageMax = 191,
	baseXp = 0,
	baseHAM = 3000,
	baseHAMmax = 3200,
	armor = 0,
	resists = {0,0,0,0,0,0,0,-1,-1},
	meatType = "",
	meatAmount = 0,
	hideType = "",
	hideAmount = 0,
	boneType = "",
	boneAmount = 0,
	milk = 0,
	tamingChance = 0,
	ferocity = 0,
	pvpBitmask = ATTACKABLE,
	creatureBitmask = HERD,
	optionsBitmask = AIENABLED,
	diet = HERBIVORE,

	templates = {
		"object/creature/npc/droid/crafted/probe_droid_advanced.iff"
	},
	lootGroups = {},
	primaryWeapon = "object/weapon/ranged/droid/droid_droideka_ranged.iff",
	secondaryWeapon = "none",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = { {"creaturerangedattack",""} },
	secondaryAttacks = { },
	defaultAttack = "attack",
	conversationTemplate = "",
}

CreatureTemplates:addCreatureTemplate(probot_crafted_advanced, "probot_crafted_advanced")
