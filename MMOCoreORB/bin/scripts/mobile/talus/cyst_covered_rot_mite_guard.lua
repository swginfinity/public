cyst_covered_rot_mite_guard = Creature:new {
	objectName = "@mob/creature_names:cyst_covered_rot_guard",
	socialGroup = "mite",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 80,
	chanceHit = 0.4,
	damageMin = 214,
	damageMax = 282,
	baseXp = 6461,
	baseHAM = 16500,
	baseHAMmax = 18500,
	armor = 1,
	resists = {175,125,180,190,0,0,115,190,-1},
	meatType = "meat_insect",
	meatAmount = 123,
	hideType = "",
	hideAmount = 0,
	boneType = "",
	boneAmount = 0,
	milk = 2 * 0,
	tamingChance = 0.05,
	ferocity = 0,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + HERD,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/rot_mite.iff"},
	controlDeviceTemplate = "object/intangible/pet/bark_mite_hue.iff",
	scale = 1.15,
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "unarmed",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = { {"strongdisease",""}, {"blindattack",""} },
	secondaryAttacks = { }

}

CreatureTemplates:addCreatureTemplate(cyst_covered_rot_mite_guard, "cyst_covered_rot_mite_guard")
