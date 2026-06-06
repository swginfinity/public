talus_humbaba = Creature:new {
	objectName = "@mob/creature_names:humbaba",
	socialGroup = "humbaba",
	faction = "",
	mobType = MOB_HERBIVORE,
	level = 46,
	chanceHit = 1.32,
	damageMin = 360,
	damageMax = 570,
	baseXp = 4202,
	baseHAM = 11200,
	baseHAMmax = 13300,
	armor = 0,
	resists = {25,35,25,5,5,5,5,45,-1},
	meatType = "meat_domesticated",
	meatAmount = 230,
	hideType = "hide_leathery",
	hideAmount = 130,
	boneType = "bone_mammal",
	boneAmount = 123,
	milkType = "milk_wild",
	milk = 2 * 250,
	tamingChance = 0.15,
	ferocity = 0,
	pvpBitmask = ATTACKABLE,
	creatureBitmask = PACK + HERD,
	optionsBitmask = AIENABLED,
	diet = HERBIVORE,

	templates = {"object/mobile/falumpaset_hue.iff"},
	controlDeviceTemplate = "object/intangible/pet/falumpaset_hue.iff",
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

CreatureTemplates:addCreatureTemplate(talus_humbaba, "talus_humbaba")
