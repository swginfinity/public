puss_covered_decay_mite_soldier = Creature:new {
	objectName = "@mob/creature_names:puss_covered_decay_mite_soldier",
	socialGroup = "mite",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 75,
	chanceHit = 0.4,
	damageMin = 800,
	damageMax = 1345,
	baseXp = 5945,
	baseHAM = 21500,
	baseHAMmax = 23500,
	armor = 1,
	resists = {165,35,185,35,-1,15,165,190,-1},	
	meatType = "meat_insect",
	meatAmount = 123,
	hideType = "",
	hideAmount = 0,
	boneType = "",
	boneAmount = 0,
	milk = 2 * 0,
	tamingChance = 0.02,
	ferocity = 0,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + HERD + STALKER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/decay_mite.iff"},
	controlDeviceTemplate = "object/intangible/pet/bark_mite_hue.iff",
	scale = 1.1,
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "object/weapon/ranged/creature/creature_spit_large_red.iff",
	secondaryWeapon = "unarmed",
	conversationTemplate = "",

	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = { {"blindattack",""}, {"milddisease",""} },
	secondaryAttacks = { {"blindattack",""}, {"milddisease",""} }
}

CreatureTemplates:addCreatureTemplate(puss_covered_decay_mite_soldier, "puss_covered_decay_mite_soldier")
