docile_kahmurra = Creature:new {
	objectName = "@mob/creature_names:docile_kahmurra",
	socialGroup = "self",
	faction = "",
	mobType = MOB_HERBIVORE,
	level = 30,
	chanceHit = 0.37,
	damageMin = 310,
	damageMax = 330,
	baseXp = 2632,
	baseHAM = 4350,
	baseHAMmax = 5420,
	armor = 0,
	resists = {0,15,5,0,0,120,130,-1,-1},
	meatType = "meat_herbivore",
	meatAmount = 123,
	hideType = "hide_bristley",
	hideAmount = 123,
	boneType = "bone_mammal",
	boneAmount = 123,
	milkType = "milk_domesticated",
	milk = 2 * 250,
	tamingChance = 0,
	ferocity = 1,
	pvpBitmask = ATTACKABLE,
	creatureBitmask = NONE,
	optionsBitmask = AIENABLED,
	diet = HERBIVORE,

	templates = {"object/mobile/kahmurra.iff"},
	scale = 0.9,
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

CreatureTemplates:addCreatureTemplate(docile_kahmurra, "docile_kahmurra")
