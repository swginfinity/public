festering_dung_mite_queen = Creature:new {
	objectName = "@mob/creature_names:festering_dung_queen",
	socialGroup = "mite",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 68,
	chanceHit = 0.42,
	damageMin = 244,
	damageMax = 319,
	baseXp = 5450,
	baseHAM = 16500,
	baseHAMmax = 18900,
	armor = 1,
	resists = {157.5,27.5,35,60,-1,-1,165,180,-1},
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
	creatureBitmask = PACK + HERD + KILLER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/dung_mite.iff"},
	scale = 1.25,
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "unarmed",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = { {"dizzyattack",""}, {"milddisease",""} },
	secondaryAttacks = { }
}

CreatureTemplates:addCreatureTemplate(festering_dung_mite_queen, "festering_dung_mite_queen")
