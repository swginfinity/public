dung_mite_worker = Creature:new {
	objectName = "@mob/creature_names:sickening_dung_mite_worker",
	socialGroup = "mite",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 43,
	chanceHit = 0.41,
	damageMin = 365,
	damageMax = 460,
	baseXp = 3279,
	baseHAM = 11000,
	baseHAMmax = 13000,
	armor = 0,
	resists = {145,110,-1,5,-1,-1,180,185,-1},	
	meatType = "meat_insect",
	meatAmount = 123,
	hideType = "",
	hideAmount = 0,
	boneType = "",
	boneAmount = 0,
	milk = 2 * 0,
	tamingChance = 0.25,
	ferocity = 0,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + HERD,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/sickening_dung_mite_worker.iff"},
	controlDeviceTemplate = "object/intangible/pet/bark_mite_hue.iff",
	scale = 1.05,
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "unarmed",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = { {"stunattack",""}, {"mediumdisease",""} },
	secondaryAttacks = { }

}

CreatureTemplates:addCreatureTemplate(dung_mite_worker, "dung_mite_worker")
