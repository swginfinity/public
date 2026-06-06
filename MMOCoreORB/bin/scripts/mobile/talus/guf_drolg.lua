guf_drolg = Creature:new {
	objectName = "@mob/creature_names:guf_drolg",
	socialGroup = "guf_drolg",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 76,
	chanceHit = 0.65,
	damageMin = 520,
	damageMax = 790,
	baseXp = 6160,
	baseHAM = 11000,
	baseHAMmax = 13000,
	armor = 1,
	resists = {75,140,30,190,190,190,15,15,-1},
	meatType = "meat_reptilian",
	meatAmount = 350,
	hideType = "hide_leathery",
	hideAmount = 275,
	boneType = "bone_mammal",
	boneAmount = 200,
	milk = 2 * 0,
	tamingChance = 0.25,
	ferocity = 0,
	pvpBitmask = ATTACKABLE,
	creatureBitmask = PACK + STALKER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/guf_drolg.iff"},
	controlDeviceTemplate = "object/intangible/pet/guf_drolg_hue.iff",
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "unarmed",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = {	{"dizzyattack",""},	{"intimidationattack",""},},
	secondaryAttacks = { }

}

CreatureTemplates:addCreatureTemplate(guf_drolg, "guf_drolg")
