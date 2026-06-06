talus_savage_humbaba = Creature:new {
	objectName = "@mob/creature_names:savage_humbaba",
	socialGroup = "self",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 56,
	chanceHit = 5.35,
	damageMin = 450,
	damageMax = 700,
	baseXp = 4700,
	baseHAM = 16800,
	baseHAMmax = 18300,
	armor = 0,
	resists = {135,155,40,-1,-1,-1,-1,65,-1},
	meatType = "meat_domesticated",
	meatAmount = 265,
	hideType = "hide_leathery",
	hideAmount = 168,
	boneType = "bone_mammal",
	boneAmount = 123,
	milkType = "milk_wild",
	milk = 2 * 250,
	tamingChance = 0.25,
	ferocity = 0,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + HERD,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/falumpaset_hue.iff"},
	controlDeviceTemplate = "object/intangible/pet/falumpaset_hue.iff",
	scale = 2.0,
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "object/weapon/ranged/creature/creature_spit_small_yellow.iff",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = { {"stunattack",""} },
	secondaryAttacks = { },

}

CreatureTemplates:addCreatureTemplate(talus_savage_humbaba, "talus_savage_humbaba")
