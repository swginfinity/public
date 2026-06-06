talus_vynock = Creature:new {
	objectName = "@mob/creature_names:vynock",
	socialGroup = "self",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 70,
	chanceHit = 1.53,
	damageMin = 772,
	damageMax = 889,
	baseXp = 6590,
	baseHAM = 15864,
	baseHAMmax = 17590,
	armor = 1,
	resists = {130,130,20,65,20,20,55,45,-1},
	meatType = "meat_herbivore",
	meatAmount = 123,
	hideType = "hide_leathery",
	hideAmount = 123,
	boneType = "bone_mammal",
	boneAmount = 123,
	milk = 2 * 0,
	tamingChance = 0.25,
	ferocity = 0,
	pvpBitmask = ATTACKABLE,
	creatureBitmask = NONE,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/vynock.iff"},
	controlDeviceTemplate = "object/intangible/pet/vynock_hue.iff",
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "unarmed",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = {{"stunattack",""},{"mildpoison",""},},
	secondaryAttacks = { }

}

CreatureTemplates:addCreatureTemplate(talus_vynock, "talus_vynock")
