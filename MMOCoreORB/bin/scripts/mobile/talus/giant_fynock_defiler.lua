giant_fynock_defiler = Creature:new {
	customName = "a giant fynock defiler",
	socialGroup = "fynock",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 120,
	chanceHit = 2.3,
	damageMin = 820,
	damageMax = 1725,
	baseXp = 8784,
	baseHAM = 16471,
	baseHAMmax = 19812,
	armor = 1,
	resists = {45,55,45,75,85,90,55,25,-1},
	meatType = "meat_avian",
	meatAmount = 123,
	hideType = "hide_leathery",
	hideAmount = 123,
	boneType = "bone_avian",
	boneAmount = 123,
	milk = 2 * 0,
	tamingChance = 0,
	ferocity = 9,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + KILLER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/fynock_hue.iff"},
	scale = 4,
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "object/weapon/ranged/creature/creature_spit_small_red.iff",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = {{"intimidationattack",""},{"strongpoison",""}},
	secondaryAttacks = { }

}

CreatureTemplates:addCreatureTemplate(giant_fynock_defiler, "giant_fynock_defiler")
