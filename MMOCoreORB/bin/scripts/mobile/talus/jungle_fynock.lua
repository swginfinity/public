jungle_fynock = Creature:new {
	objectName = "@mob/creature_names:jungle_fynock",
	socialGroup = "fynock",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 38,
	chanceHit = 0.4,
	damageMin = 330,
	damageMax = 490,
	baseXp = 3233,
	baseHAM = 8650,
	baseHAMmax = 9790,
	armor = 0,
	resists = {155,22.5,150,15,5,-1,-1,-1,-1},
	meatType = "meat_avian",
	meatAmount = 123,
	hideType = "hide_leathery",
	hideAmount = 123,
	boneType = "bone_avian",
	boneAmount = 123,
	milk = 2 * 0,
	tamingChance = 0.25,
	ferocity = 0,
	pvpBitmask = ATTACKABLE,
	creatureBitmask = PACK + STALKER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/fynock_hue.iff"},
	controlDeviceTemplate = "object/intangible/pet/fynock_hue.iff",
	scale = 1.15,
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "unarmed",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = { {"posturedownattack",""} },
	secondaryAttacks = { }

}

CreatureTemplates:addCreatureTemplate(jungle_fynock, "jungle_fynock")
