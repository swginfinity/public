fbase_at_st = Creature:new {
	objectName = "@mob/creature_names:fbase_at_st",
	socialGroup = "imperial",
	faction = "imperial",
	mobType = MOB_VEHICLE,
	level = 205,
	chanceHit = 11.5,
	damageMin = 1170,
	damageMax = 2050,
	baseXp = 20500,
	baseHAM = 139000,
	baseHAMmax = 200000,
	armor = 3,
	resists = {135,135,-1,200,200,15,15,200,-1},
	meatType = "",
	meatAmount = 0,
	hideType = "",
	hideAmount = 0,
	boneType = "",
	boneAmount = 0,
	milk = 0,
	tamingChance = 0,
	ferocity = 0,
	pvpBitmask = ATTACKABLE + OVERT,
	creatureBitmask = PACK + KILLER,
	optionsBitmask = AIENABLED,
	diet = NONE,

	templates = {"object/mobile/atst.iff"},
	lootGroups = {},
	conversationTemplate = "",
	primaryWeapon = "object/weapon/ranged/droid/droid_droideka_ranged.iff",
	secondaryWeapon = "none",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = { {"creaturerangedattack",""} },
	secondaryAttacks = { }
}

CreatureTemplates:addCreatureTemplate(fbase_at_st, "fbase_at_st")
