guf_drolg_female = Creature:new {
	objectName = "@mob/creature_names:guf_drolg_female",
	socialGroup = "guf_drolg",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 74,
	chanceHit = 0.65,
	damageMin = 500,
	damageMax = 770,
	baseXp = 5878,
	baseHAM = 11000,
	baseHAMmax = 13000,
	armor = 1,
	resists = {65,140,25,190,190,190,5,5,-1},
	meatType = "meat_reptilian",
	meatAmount = 325,
	hideType = "hide_leathery",
	hideAmount = 265,
	boneType = "bone_mammal",
	boneAmount = 190,
	milk = 2 * 0,
	tamingChance = 0.03,
	ferocity = 0,
	pvpBitmask = ATTACKABLE,
	creatureBitmask = PACK + STALKER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/guf_drolg_hue.iff"},
	controlDeviceTemplate = "object/intangible/pet/guf_drolg_hue.iff",
	scale = 1.05,
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "unarmed",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = {{"stunattack",""},{"intimidationattack",""}},
	secondaryAttacks = { }

}

CreatureTemplates:addCreatureTemplate(guf_drolg_female, "guf_drolg_female")
