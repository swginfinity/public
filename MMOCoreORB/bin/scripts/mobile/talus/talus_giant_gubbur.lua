talus_giant_gubbur = Creature:new {
	objectName = "@mob/creature_names:giant_gubbur",
	socialGroup = "gubbur",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 38,
	chanceHit = 0.42,
	damageMin = 244,
	damageMax = 319,
	baseXp = 2719,
	baseHAM = 5685,
	baseHAMmax = 7052,
	armor = 0,
	resists = {115,110,5,-1,5,5,-1,-1,-1},
	meatType = "meat_carnivore",
	meatAmount = 123,
	hideType = "hide_leathery",
	hideAmount = 123,
	boneType = "bone_mammal",
	boneAmount = 123,
	milk = 2 * 0,
	tamingChance = 0,
	ferocity = 0,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = HERD,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/giant_gubbur.iff"},
	scale = 1.5,
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "unarmed",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = { {"",""} },
	secondaryAttacks = { }
}

CreatureTemplates:addCreatureTemplate(talus_giant_gubbur, "talus_giant_gubbur")
