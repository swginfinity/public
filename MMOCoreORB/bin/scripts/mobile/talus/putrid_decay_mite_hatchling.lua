putrid_decay_mite_hatchling = Creature:new {
	objectName = "@mob/creature_names:putrid_decay_mite_hatchling",
	socialGroup = "mite",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 64,
	chanceHit = 0.34,
	damageMin = 470,
	damageMax = 640,
	baseXp = 5480,
	baseHAM = 14000,
	baseHAMmax = 18000,
	armor = 0,
	resists = {155,25,140,65,-1,0,175,190,-1},	
	meatType = "meat_insect",
	meatAmount = 123,
	hideType = "",
	hideAmount = 0,
	boneType = "",
	boneAmount = 0,
	milk = 2 * 0,
	tamingChance = 0,
	ferocity = 0,
	pvpBitmask = ATTACKABLE,
	creatureBitmask = HERD,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/putrid_decay_mite_hatchling.iff"},
	scale = 0.75,
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "object/weapon/ranged/creature/creature_spit_small_yellow.iff",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = {{"strongdisease",""},{"strongpoison",""},},
	secondaryAttacks = { },

}

CreatureTemplates:addCreatureTemplate(putrid_decay_mite_hatchling, "putrid_decay_mite_hatchling")
