sickly_decay_mite_queen = Creature:new {
	objectName = "@mob/creature_names:sickly_decay_mite_queen",
	socialGroup = "mite",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 52,
	chanceHit = 0.44,
	damageMin = 390,
	damageMax = 580,
	baseXp = 5007,
	baseHAM = 11000,
	baseHAMmax = 13000,
	armor = 0,
	resists = {150,125,-1,-1,-1,-1,130,185,-1},	
	meatType = "meat_insect",
	meatAmount = 123,
	hideType = "",
	hideAmount = 0,
	boneType = "",
	boneAmount = 0,
	milk = 2 * 0,
	tamingChance = 0,
	ferocity = 0,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + HERD,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/decay_mite.iff"},
	scale = 1.25,
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "object/weapon/ranged/creature/creature_spit_small_yellow.iff",
	secondaryWeapon = "unarmed",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = { {"blindattack",""}, {"mediumdisease",""} },
	secondaryAttacks = { {"blindattack",""}, {"mediumdisease",""} }
}

CreatureTemplates:addCreatureTemplate(sickly_decay_mite_queen, "sickly_decay_mite_queen")
