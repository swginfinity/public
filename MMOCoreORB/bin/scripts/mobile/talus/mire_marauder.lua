mire_marauder = Creature:new {
	objectName = "@mob/creature_names:mire_marauder",
	socialGroup = "panther",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 200,
	chanceHit = 4.72,
	damageMin = 650,damageMax = 1125,
	baseXp = 6819,
	baseHAM = 31253,
	baseHAMmax = 41253,
	armor = 3,
	resists = {145,175,65,140,86,85,45,75,-1},
	meatType = "meat_carnivore",
	meatAmount = 150,
	hideType = "hide_leathery",
	hideAmount = 140,
	boneType = "bone_mammal",
	boneAmount = 140,
	milk = 2 * 0,
	tamingChance = 0,
	ferocity = 12,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + KILLER + STALKER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/minor_sludge_panther.iff"},
	scale = 2.4,
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "unarmed",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = {{"posturedownattack",""},{"mediumpoison",""}	},
	secondaryAttacks = { }

}

CreatureTemplates:addCreatureTemplate(mire_marauder, "mire_marauder")
