flite_rasp = Creature:new {
	objectName = "@mob/creature_names:flite_rasp",
	socialGroup = "rasp",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 44,
	chanceHit = 0.36,
	damageMin = 390,
	damageMax = 440,
	baseXp = 3509,
	baseHAM = 8900,
	baseHAMmax = 10680,
	armor = 0,
	resists = {165,165,5,10,15,30,-1,30,-1},
	meatType = "meat_avian",
	meatAmount = 123,
	hideType = "",
	hideAmount = 0,
	boneType = "bone_avian",
	boneAmount = 123,
	milk = 2 * 0,
	tamingChance = 0,
	ferocity = 0,
	pvpBitmask = ATTACKABLE,
	creatureBitmask = PACK + HERD,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/flite_rasp.iff"},
	scale = 0.9,
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "unarmed",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = { {"",""} },
	secondaryAttacks = { }
}

CreatureTemplates:addCreatureTemplate(flite_rasp, "flite_rasp")
