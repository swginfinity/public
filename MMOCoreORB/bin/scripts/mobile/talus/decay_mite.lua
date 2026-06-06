decay_mite = Creature:new {
	objectName = "@mob/creature_names:decay_mite",
	socialGroup = "mite",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 23,
	chanceHit = 0.36,
	damageMin = 245,
	damageMax = 440,
	baseXp = 1919,
	baseHAM = 6400,
	baseHAMmax = 7400,
	armor = 0,
	resists = {115,15,135,57.5,-1,0,165,175,-1},	
	meatType = "meat_insect",
	meatAmount = 123,
	hideType = "hide_scaley",
	hideAmount = 123,
	boneType = "",
	boneAmount = 0,
	milk = 2 * 0,
	tamingChance = 0.25,
	ferocity = 0,
	pvpBitmask = ATTACKABLE,
	creatureBitmask = PACK + HERD,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/decay_mite.iff"},
	controlDeviceTemplate = "object/intangible/pet/bark_mite_hue.iff",
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "unarmed",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = { {"",""}, {"posturedownattack",""} },
	secondaryAttacks = { }

}

CreatureTemplates:addCreatureTemplate(decay_mite, "decay_mite")
