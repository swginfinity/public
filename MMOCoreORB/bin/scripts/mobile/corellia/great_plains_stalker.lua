great_plains_stalker = Creature:new {
	objectName = "@mob/creature_names:great_plains_stalker",
	socialGroup = "panther",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 95,
	chanceHit = 1.5,
	damageMin = 900,
	damageMax = 1800,
	baseXp = 4916,
	baseHAM = 32400,
	baseHAMmax = 36000,
	armor = 1,
	resists = {185,180,55,65,70,180,75,80,135},
	meatType = "meat_carnivore",
	meatAmount = 65,
	hideType = "hide_bristley",
	hideAmount = 35,
	boneType = "bone_mammal",
	boneAmount = 30,
	milk = 0,
	tamingChance = 0.05,
	ferocity = 15,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = KILLER + STALKER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/corellian_sand_panther_hue.iff"},
	hues = { 24, 25, 26, 27, 28, 29, 30, 31 },
	controlDeviceTemplate = "object/intangible/pet/great_plains_stalker_hue.iff",
	scale = 1.25,
	lootGroups = {},

	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "unarmed",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = { {"creatureareaknockdown","stateAccuracyBonus=25"}, {"creatureareacombo","stateAccuracyBonus=25"} },
	secondaryAttacks = {}

}

CreatureTemplates:addCreatureTemplate(great_plains_stalker, "great_plains_stalker")
