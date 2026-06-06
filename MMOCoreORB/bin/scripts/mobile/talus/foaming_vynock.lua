foaming_vynock = Creature:new {
	objectName = "@mob/creature_names:foaming_vynock",
	socialGroup = "self",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 30,
	chanceHit = 0.4,
	damageMin = 259,
	damageMax = 338,
	baseXp = 2245,
	baseHAM = 1895,
	baseHAMmax = 2436,
	armor = 0,
	resists = {0,0,0,0,0,0,0,-1,-1},
	meatType = "meat_avian",
	meatAmount = 123,
	hideType = "hide_leathery",
	hideAmount = 123,
	boneType = "bone_mammal",
	boneAmount = 123,
	milk = 2 * 0,
	tamingChance = 0,
	ferocity = 0,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = KILLER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/vynock_hue.iff"},
	scale = 1.2,
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "unarmed",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = {{"intimidationattack",""},{"milddisease",""}},
	secondaryAttacks = { }

}

CreatureTemplates:addCreatureTemplate(foaming_vynock, "foaming_vynock")
