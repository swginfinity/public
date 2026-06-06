r3_crafted_advanced = Creature:new {
	objectName = "@droid_name:r3_crafted_advanced",
	socialGroup = "",
	faction = "",
	level = 4,
	mobType = MOB_DROID,
	chanceHit = 0.24,
	damageMin = 160,
	damageMax = 170,
	baseXp = 0,
	baseHAM = 4000,
	baseHAMmax = 4500,
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
		"object/creature/npc/droid/crafted/r3_droid_advanced.iff"
	},
	lootGroups = {},
	primaryWeapon = "object/weapon/ranged/droid/droid_droideka_ranged.iff",
	secondaryWeapon = "none",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = { {"creaturerangedattack",""} },
	secondaryAttacks = { },
	defaultAttack = "attack",
	defaultWeapon = "object/weapon/ranged/droid/droid_astromech_ranged.iff",
	conversationTemplate = "",
}

CreatureTemplates:addCreatureTemplate(r3_crafted_advanced, "r3_crafted_advanced")
