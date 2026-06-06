tamers_prized_wrix2 = Creature:new {
	objectName = "@mob/creature_names:deranged_wrix",
	customName = "Velocity (Xerthan's Prized Wrix)",
	socialGroup = "thug",
	faction = "thug",
	mobType = MOB_CARNIVORE,
	level = 150,
	chanceHit = 10,
	damageMin = 1050,
	damageMax = 2050,
	attackSpeed = 2.0,
	specialPercentage = 65,
	baseXp = 1102,
	baseHAM = 175000,
	baseHAMmax = 250000,
	armor = 1,
	resists = {50,50,50,50,50,50,50,50,5},
	meatType = "meat_carnivore",
	meatAmount = 123,
	hideType = "hide_bristley",
	hideAmount = 123,
	boneType = "bone_mammal",
	boneAmount = 123,
	milk = 0,
	tamingChance = 0,
	ferocity = 0,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = NONE,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/deranged_wrix.iff"},
	scale = 1.5,
	lootGroups = {},
	
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "unarmed",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = {	{"creatureareacombo","stateAccuracyBonus=50"},	{"stunattack","stateAccuracyBonus=55"},	{"intimidationattack","stateAccuracyBonus=75"},	},
	secondaryAttacks = { },
}

CreatureTemplates:addCreatureTemplate(tamers_prized_wrix2, "tamers_prized_wrix2")
