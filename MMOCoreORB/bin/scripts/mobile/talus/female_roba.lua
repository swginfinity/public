female_roba = Creature:new {
	objectName = "@mob/creature_names:roba_female",
	socialGroup = "roba",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 41,
	chanceHit = 0.41,
	damageMin = 345,
	damageMax = 400,
	baseXp = 3515,
	baseHAM = 9300,
	baseHAMmax = 11300,
	armor = 0,
	resists = {35,35,35,175,-1,135,30,30,-1},
	meatType = "meat_carnivore",
	meatAmount = 123,
	hideType = "hide_leathery",
	hideAmount = 123,
	boneType = "bone_mammal",
	boneAmount = 123,
	milk = 2 * 0,
	tamingChance = 0.25,
	ferocity = 0,
	pvpBitmask = ATTACKABLE,
	creatureBitmask = PACK + HERD,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/roba_hue.iff"},
	controlDeviceTemplate = "object/intangible/pet/roba_hue.iff",
	scale = 0.95,
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "unarmed",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = { {"",""}, {"dizzyattack",""} },
	secondaryAttacks = { }

}

CreatureTemplates:addCreatureTemplate(female_roba, "female_roba")
