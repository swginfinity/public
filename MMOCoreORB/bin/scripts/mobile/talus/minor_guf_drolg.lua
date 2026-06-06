minor_guf_drolg = Creature:new {
	objectName = "@mob/creature_names:minor_guf_drolg",
	socialGroup = "guf_drolg",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 64,
	chanceHit = 0.5,
	damageMin = 450,
	damageMax = 725,
	baseXp = 4946,
	baseHAM = 9800,
	baseHAMmax = 10450,
	armor = 0,
	resists = {50,120,25,170,170,170,0,-1,-1},
	meatType = "meat_reptilian",
	meatAmount = 300,
	hideType = "hide_leathery",
	hideAmount = 240,
	boneType = "bone_mammal",
	boneAmount = 170,
	milk = 2 * 0,
	tamingChance = 0.25,
	ferocity = 0,
	pvpBitmask = ATTACKABLE,
	creatureBitmask = PACK,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/minor_guf_drolg.iff"},
	controlDeviceTemplate = "object/intangible/pet/guf_drolg_hue.iff",
	scale = 0.85,
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

CreatureTemplates:addCreatureTemplate(minor_guf_drolg, "minor_guf_drolg")
