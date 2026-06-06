rotting_decay_mite_guard = Creature:new {
	objectName = "@mob/creature_names:rotting_pustule_guard",
	socialGroup = "mite",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 85,
	chanceHit = 0.4,
	damageMin = 214,
	damageMax = 282,
	baseXp = 6487,
	baseHAM = 17500,
	baseHAMmax = 19500,
	armor = 1,
	resists = {80,30,185,-1,25,15,110,180,-1},
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
	creatureBitmask = PACK + HERD + KILLER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/decay_mite.iff"},
	controlDeviceTemplate = "object/intangible/pet/bark_mite_hue.iff",
	scale = 1.05,
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "unarmed",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = {	{"intimidationattack",""},	{"strongdisease",""}},
	secondaryAttacks = { }

}

CreatureTemplates:addCreatureTemplate(rotting_decay_mite_guard, "rotting_decay_mite_guard")
