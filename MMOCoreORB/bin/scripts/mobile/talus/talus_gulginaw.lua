talus_gulginaw = Creature:new {
	objectName = "@mob/creature_names:gulginaw",
	socialGroup = "gulginaw",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 70,
	chanceHit = 0.54,
	damageMin = 648,
	damageMax = 1245,
	baseXp = 6296,
	baseHAM = 8032,
	baseHAMmax = 9513,
	armor = 1,
	resists = {55,45,25,25,25,55,65,70,-1},
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

	templates = {"object/mobile/gulginaw.iff"},
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "object/weapon/ranged/creature/creature_spit_small_yellow.iff",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = {{"mildpoison",""},{"intimidationattack",""}},
	secondaryAttacks = { },
	conversationTemplate = "",

}

CreatureTemplates:addCreatureTemplate(talus_gulginaw, "talus_gulginaw")
