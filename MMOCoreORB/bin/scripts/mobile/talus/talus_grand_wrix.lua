talus_grand_wrix = Creature:new {
	objectName = "@mob/creature_names:grand_wrix",
	socialGroup = "wrix",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 68,
	chanceHit = 2.35,
	damageMin = 210,
	damageMax = 620,
	baseXp = 5637,
	baseHAM = 15900,
	baseHAMmax = 17200,
	armor = 1,
	resists = {155,145,35,150,-1,135,15,50,-1},
	meatType = "meat_carnivore",
	meatAmount = 123,
	hideType = "hide_bristley",
	hideAmount = 123,
	boneType = "bone_mammal",
	boneAmount = 123,
	milk = 2 * 0,
	tamingChance = 0.25,
	ferocity = 8,
	pvpBitmask = ATTACKABLE,
	creatureBitmask = PACK + HERD + KILLER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/grand_wrix.iff"},
	controlDeviceTemplate = "object/intangible/pet/gurreck_hue.iff",
	scale = 1.25,
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "unarmed",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = {	{"knockdownattack",""},	{"stunattack",""}},
	secondaryAttacks = { }

}

CreatureTemplates:addCreatureTemplate(talus_grand_wrix, "talus_grand_wrix")
