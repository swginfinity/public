giant_decay_mite_sentry = Creature:new {
	objectName = "@mob/creature_names:giant_decay_mite_sentry",
	socialGroup = "mite",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 41,
	chanceHit = 0.41,
	damageMin = 350,
	damageMax = 425,
	baseXp = 3840,
	baseHAM = 13000,
	baseHAMmax = 17000,
	armor = 0,
	resists = {120,120,155,170,0,-1,190,190,-1},
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
	scale = 1.8,
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "object/weapon/ranged/creature/creature_spit_small_yellow.iff",
	secondaryWeapon = "unarmed",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = { {"knockdownattack",""}, {"mediumdisease",""} },
	secondaryAttacks = { {"knockdownattack",""}, {"mediumdisease",""} }
}

CreatureTemplates:addCreatureTemplate(giant_decay_mite_sentry, "giant_decay_mite_sentry")
