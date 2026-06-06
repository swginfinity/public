giant_fynock = Creature:new {
	objectName = "@mob/creature_names:giant_fynock",
	socialGroup = "fynock",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 55,
	chanceHit = 0.58,
	damageMin = 525,
	damageMax = 750,
	baseXp = 4781,
	baseHAM = 12632,
	baseHAMmax = 15385,
	armor = 0,
	resists = {175,30,175,25,15,-1,15,-1,-1},
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
	primaryAttacks = {{"intimidationattack",""},	{"creatureareaknockdown",""}},
	secondaryAttacks = { }
}

CreatureTemplates:addCreatureTemplate(giant_fynock, "giant_fynock")
