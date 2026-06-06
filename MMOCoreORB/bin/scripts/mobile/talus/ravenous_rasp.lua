ravenous_rasp = Creature:new {
	objectName = "@mob/creature_names:ravenous_rasp",
	socialGroup = "rasp",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 52,
	chanceHit = 0.54,
	damageMin = 465,
	damageMax = 640,
	baseXp = 4896,
	baseHAM = 10000,
	baseHAMmax = 12000,
	armor = 0,
	resists = {145,165,5,5,25,25,25,25,-1},
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
	creatureBitmask = PACK + HERD + KILLER + STALKER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/flite_rasp_hue.iff"},
	scale = 1.25,
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "object/weapon/ranged/creature/creature_spit_small_toxicgreen.iff",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = {	{"strongpoison",""},{"dizzyattack",""}},
	secondaryAttacks = { },
	conversationTemplate = "",

}

CreatureTemplates:addCreatureTemplate(ravenous_rasp, "ravenous_rasp")
