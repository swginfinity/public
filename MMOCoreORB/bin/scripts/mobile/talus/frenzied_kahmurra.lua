frenzied_kahmurra = Creature:new {
	objectName = "@mob/creature_names:frenzied_kahmurra",
	socialGroup = "kahmurra",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 48,
	chanceHit = 0.53,
	damageMin = 340,
	damageMax = 600,
	baseXp = 4190,
	baseHAM = 10864,
	baseHAMmax = 13590,
	armor = 0,
	resists = {130,130,20,20,20,155,160,-1,-1},
	meatType = "meat_herbivore",
	meatAmount = 123,
	hideType = "hide_bristley",
	hideAmount = 123,
	boneType = "bone_mammal",
	boneAmount = 123,
	milk = 2 * 0,
	tamingChance = 0.05,
	ferocity = 6,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + KILLER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/kahmurra.iff"},
	controlDeviceTemplate = "object/intangible/pet/murra_hue.iff",
	scale = 1.25,
	lootGroups = {},
	
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "unarmed",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = { {"stunattack",""} },
	secondaryAttacks = { }

}
 
CreatureTemplates:addCreatureTemplate(frenzied_kahmurra, "frenzied_kahmurra")
