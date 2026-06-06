voracious_rasp = Creature:new {
	objectName = "@mob/creature_names:voracious_rasp",
	socialGroup = "rasp",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 57,
	chanceHit = 0.62,
	damageMin = 420,
	damageMax = 550,
	baseXp = 4866,
	baseHAM = 11000,
	baseHAMmax = 13000,
	armor = 0,
	resists = {145,145,10,10,10,125,10,10,-1},
	meatType = "meat_avian",
	meatAmount = 123,
	hideType = "",
	hideAmount = 0,
	boneType = "bone_avian",
	boneAmount = 123,
	milk = 0,
	tamingChance = 0,
	ferocity = 0,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + HERD + KILLER + STALKER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/plumed_rasp.iff"},
	scale = 1.35,
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "object/weapon/ranged/creature/creature_spit_small_toxicgreen.iff",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = {{"strongpoison",""},{"blindattack",""}},
	secondaryAttacks = { },
}

CreatureTemplates:addCreatureTemplate(voracious_rasp, "voracious_rasp")
