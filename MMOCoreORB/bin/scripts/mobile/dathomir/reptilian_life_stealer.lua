reptilian_life_stealer = Creature:new {
	objectName = "@mob/creature_names:reptilian_life_stealer",
	socialGroup = "reptilian_flier",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 95,
	chanceHit = 1.25,
	damageMin = 950,
	damageMax = 1800,
	baseXp = 11859,
	baseHAM = 33400,
	baseHAMmax = 36000,
	armor = 1,
	resists = {185,180,175,175,170,185,180,185,135},
	meatType = "meat_avian",
	meatAmount = 135,
	hideType = "hide_leathery",
	hideAmount = 90,
	boneType = "bone_avian",
	boneAmount = 85,
	milk = 0,
	tamingChance = 0.05,
	ferocity = 0,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + KILLER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/reptilian_flier_hue.iff"},
	hues = { 24, 25, 26, 27, 28, 29, 30, 31 },
	controlDeviceTemplate = "object/intangible/pet/reptilian_flier_hue.iff",
	scale = 1.5,
	lootGroups = {},

	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "unarmed",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = { {"creatureareacombo","stateAccuracyBonus=25"}, {"strongpoison",""} },
	secondaryAttacks = { }

}

CreatureTemplates:addCreatureTemplate(reptilian_life_stealer, "reptilian_life_stealer")
