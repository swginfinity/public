talus_paralope = Creature:new {
	objectName = "@mob/creature_names:paralope",
	socialGroup = "paralope",
	faction = "",
	mobType = MOB_HERBIVORE,
	level = 34,
	chanceHit = 0.25,
	damageMin = 420,
	damageMax = 540,
	baseXp = 2471,
	baseHAM = 5800,
	baseHAMmax = 6400,
	armor = 0,
	resists = {25,15,35,0,0,0,0,-1,-1},
	meatType = "meat_herbivore",
	meatAmount = 123,
	hideType = "hide_wooly",
	hideAmount = 123,
	boneType = "bone_mammal",
	boneAmount = 123,
	milkType = "milk_wild",
	milk = 2 * 250,
	tamingChance = 0.25,
	ferocity = 0,
	pvpBitmask = ATTACKABLE,
	creatureBitmask = HERD,
	optionsBitmask = AIENABLED,
	diet = HERBIVORE,

	templates = {"object/mobile/paralope.iff"},
	controlDeviceTemplate = "object/intangible/pet/swirl_prong_hue.iff",
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "unarmed",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = {{"dizzyattack",""},{"stunattack",""},},
	secondaryAttacks = { }

}

CreatureTemplates:addCreatureTemplate(talus_paralope, "talus_paralope")
