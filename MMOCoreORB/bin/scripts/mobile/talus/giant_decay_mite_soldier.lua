giant_decay_mite_soldier = Creature:new {
	objectName = "@mob/creature_names:giant_decay_mite_soldier",
	socialGroup = "mite",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 39,
	chanceHit = 0.41,
	damageMin = 325,
	damageMax = 400,
	baseXp = 3414,
	baseHAM = 9100,
	baseHAMmax = 11200,
	armor = 1,
	resists = {120,120,150,170,0,-1,185,190,-1},
	meatType = "meat_insect",
	meatAmount = 123,
	hideType = "hide_scaley",
	hideAmount = 123,
	boneType = "",
	boneAmount = 0,
	milk = 2 * 0,
	tamingChance = 0,
	ferocity = 0,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/decay_mite.iff"},
	scale = 1.5,
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "unarmed",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = { {"stunattack",""} },
	secondaryAttacks = { }

}

CreatureTemplates:addCreatureTemplate(giant_decay_mite_soldier, "giant_decay_mite_soldier")
