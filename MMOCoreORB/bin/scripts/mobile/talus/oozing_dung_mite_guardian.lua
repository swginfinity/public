oozing_dung_mite_guardian = Creature:new {
	objectName = "@mob/creature_names:oozing_dung_royal_guardian",
	socialGroup = "mite",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 78,
	chanceHit = 0.41,
	damageMin = 229,
	damageMax = 300,
	baseXp = 5940,
	baseHAM = 21500,
	baseHAMmax = 23500,
	armor = 1,
	resists = {145,135,170,45,5,-1,190,195,-1},
	meatType = "meat_insect",
	meatAmount = 123,
	hideType = "",
	hideAmount = 0,
	boneType = "",
	boneAmount = 0,
	milk = 2 * 0,
	tamingChance = 0.25,
	ferocity = 0,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + HERD + KILLER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/dung_mite.iff"},
	controlDeviceTemplate = "object/intangible/pet/bark_mite_hue.iff",
	scale = 1.15,
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "object/weapon/ranged/creature/creature_spit_small_red.iff",
	secondaryWeapon = "unarmed",
	conversationTemplate = "",

	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = { {"mediumdisease",""}, {"stunattack",""} },
	secondaryAttacks = { {"mediumdisease",""}, {"stunattack",""} }
}

CreatureTemplates:addCreatureTemplate(oozing_dung_mite_guardian, "oozing_dung_mite_guardian")
