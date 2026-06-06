kima = Creature:new {
	objectName = "@mob/creature_names:kima",
	socialGroup = "kima",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 56,
	chanceHit = 1.25,
	damageMin = 475,
	damageMax = 660,
	baseXp = 4719,
	baseHAM = 9780,
	baseHAMmax = 11345,
	armor = 0,
	resists = {55,145,5,-1,180,5,-1,-1,-1},
	meatType = "meat_herbivore",
	meatAmount = 123,
	hideType = "hide_bristley",
	hideAmount = 123,
	boneType = "bone_mammal",
	boneAmount = 123,
	milk = 2 * 0,
	tamingChance = 0.25,
	ferocity = 0,
	pvpBitmask = ATTACKABLE,
	creatureBitmask = PACK,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/kima.iff"},
	controlDeviceTemplate = "object/intangible/pet/kima_hue.iff",
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "unarmed",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = {{"stunattack",""},{"intimidationattack",""}},
	secondaryAttacks = { }

}

CreatureTemplates:addCreatureTemplate(kima, "kima")
