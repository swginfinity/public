frenzied_fynock_guardian = Creature:new {
	objectName = "@mob/creature_names:frenzied_fynock_guardian",
	socialGroup = "fynock",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 42,
	chanceHit = 0.93,
	damageMin = 420,	
	damageMax = 485,
	baseXp = 3884,
	baseHAM = 6571,
	baseHAMmax = 6912,
	armor = 0,
	resists = {30,45,15,45,0,10,5,-1,-1},
	meatType = "meat_avian",
	meatAmount = 123,
	hideType = "",
	hideAmount = 0,
	boneType = "bone_avian",
	boneAmount = 123,
	milk = 2 * 0,
	tamingChance = 0.25,
	ferocity = 0,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + HERD,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/fynock_hue.iff"},
	controlDeviceTemplate = "object/intangible/pet/fynock_hue.iff",
	scale = 1.1,
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "unarmed",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = {{"mildpoison",""},{"posturedownattack",""}},
	secondaryAttacks = { }
}

CreatureTemplates:addCreatureTemplate(frenzied_fynock_guardian, "frenzied_fynock_guardian")
