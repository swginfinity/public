hilltop_kima = Creature:new {
	objectName = "@mob/creature_names:hilltop_kima",
	socialGroup = "kima",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 68,
	chanceHit = 1.75,
	damageMin = 510,
	damageMax = 730,
	baseXp = 5940,
	baseHAM = 11000,
	baseHAMmax = 14000,
	armor = 0,
	resists = {65,145,10,-1,180,-1,-1,-1,-1},
	meatType = "meat_herbivore",
	meatAmount = 123,
	hideType = "hide_bristley",
	hideAmount = 123,
	boneType = "bone_mammal",
	boneAmount = 123,
	milk = 2 * 0,
	tamingChance = 0.01,
	ferocity = 0,
	pvpBitmask = ATTACKABLE,
	creatureBitmask = PACK,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/kima_hue.iff"},
	controlDeviceTemplate = "object/intangible/pet/kima_hue.iff",
	scale = 1.2,
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "unarmed",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = {	{"dizzyattack",""},	{"creatureareacombo",""},	},
	secondaryAttacks = { }

}

CreatureTemplates:addCreatureTemplate(hilltop_kima, "hilltop_kima")
