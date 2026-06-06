male_roba = Creature:new {
	objectName = "@mob/creature_names:roba_male",
	socialGroup = "roba",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 42,
	chanceHit = 0.44,
	damageMin = 375,
	damageMax = 425,
	baseXp = 3915,
	baseHAM = 9800,
	baseHAMmax = 12200,
	armor = 0,
	resists = {40,40,40,185,-1,145,30,40,-1},
	meatType = "meat_carnivore",
	meatAmount = 123,
	hideType = "hide_leathery",
	hideAmount = 123,
	boneType = "bone_mammal",
	boneAmount = 123,
	milk = 2 * 0,
	tamingChance = 0.25,
	ferocity = 2,
	pvpBitmask = ATTACKABLE,
	creatureBitmask = PACK + HERD,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/roba_hue.iff"},
	controlDeviceTemplate = "object/intangible/pet/roba_hue.iff",
	scale = 0.9,
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "unarmed",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = { {"dizzyattack",""} },
	secondaryAttacks = { }

}

CreatureTemplates:addCreatureTemplate(male_roba, "male_roba")
