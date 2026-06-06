huf_dun_bull = Creature:new {
	objectName = "@mob/creature_names:huf_dun_bull",
	socialGroup = "huf_dun",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 48,
	chanceHit = 0.46,
	damageMin = 345,
	damageMax = 420,
	baseXp = 4643,
	baseHAM = 26000,
	baseHAMmax = 32000,
	armor = 1,
	resists = {160,140,180,-1,-1,-1,25,180,-1},
	meatType = "meat_herbivore",
	meatAmount = 1000,
	hideType = "hide_leathery",
	hideAmount = 1000,
	boneType = "bone_mammal",
	boneAmount = 1000,
	milk = 2 * 0,
	tamingChance = 0,
	ferocity = 7,
	pvpBitmask = ATTACKABLE,
	creatureBitmask = PACK + HERD,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/huf_dun_hue.iff"},
	scale = 1.1,
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "unarmed",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = {{"dizzyattack",""},	{"stunattack",""}},
	secondaryAttacks = { }

}

CreatureTemplates:addCreatureTemplate(huf_dun_bull, "huf_dun_bull")
