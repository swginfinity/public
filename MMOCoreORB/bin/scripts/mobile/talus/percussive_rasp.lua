percussive_rasp = Creature:new {
	objectName = "@mob/creature_names:percussive_rasp",
	socialGroup = "rasp",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 48,
	chanceHit = 0.38,
	damageMin = 425,
	damageMax = 600,
	baseXp = 3412,
	baseHAM = 9800,
	baseHAMmax = 11400,
	armor = 0,
	resists = {155,145,0,5,15,110,0,-1,-1},
	meatType = "meat_avian",
	meatAmount = 123,
	hideType = "",
	hideAmount = 0,
	boneType = "bone_avian",
	boneAmount = 123,
	milk = 2 * 0,
	tamingChance = 0,
	ferocity = 0,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + STALKER + KILLER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/flite_rasp_hue.iff"},
	scale = 1.1,
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "unarmed",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = { {"intimidationattack",""} },
	secondaryAttacks = { }

}

CreatureTemplates:addCreatureTemplate(percussive_rasp, "percussive_rasp")
