famished_sludge_panther = Creature:new {
	objectName = "@mob/creature_names:famished_sludge_panther",
	socialGroup = "panther",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 42,
	chanceHit = 0.44,
	damageMin = 305,
	damageMax = 394,
	baseXp = 3481,
	baseHAM = 4422,
	baseHAMmax = 5513,
	armor = 0,
	resists = {5,120,5,-1,-1,-1,5,-1,-1},
	meatType = "meat_carnivore",
	meatAmount = 123,
	hideType = "hide_leathery",
	hideAmount = 123,
	boneType = "bone_mammal",
	boneAmount = 123,
	milk = 2 * 0,
	tamingChance = 0.25,
	ferocity = 8,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/minor_sludge_panther.iff"},
	controlDeviceTemplate = "object/intangible/pet/corellian_sand_panther_hue.iff",
	scale = 1.1,
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "unarmed",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = { {"stunattack",""}, {"",""} },
	secondaryAttacks = { }
}

CreatureTemplates:addCreatureTemplate(famished_sludge_panther, "famished_sludge_panther")
