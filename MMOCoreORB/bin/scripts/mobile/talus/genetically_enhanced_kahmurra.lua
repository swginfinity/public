genetically_enhanced_kahmurra = Creature:new {
	objectName = "@mob/creature_names:genetically_enhanced_kahmurra",
	socialGroup = "kahmurra",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 90,
	chanceHit = 2.5,
	damageMin = 960,
	damageMax = 1550,
	baseXp = 6200,
	baseHAM = 29600,
	baseHAMmax = 34400,
	armor = 1,
	resists = {80,175,165,45,15,185,177.5,25,-1},
	meatType = "meat_herbivore",
	meatAmount = 123,
	hideType = "hide_bristley",
	hideAmount = 123,
	boneType = "bone_mammal",
	boneAmount = 123,
	milkType = "milk_wild",
	milk = 2 * 250,
	tamingChance = 0.03,
	ferocity = 6,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + KILLER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/kahmurra.iff"},
	controlDeviceTemplate = "object/intangible/pet/murra_hue.iff",
	scale = 1.4,
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "unarmed",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = {{"intimidationattack",""},{"stunattack",""}},
	secondaryAttacks = { }
}

CreatureTemplates:addCreatureTemplate(genetically_enhanced_kahmurra, "genetically_enhanced_kahmurra")
