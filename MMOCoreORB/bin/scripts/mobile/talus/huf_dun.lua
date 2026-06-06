huf_dun = Creature:new {
	objectName = "@mob/creature_names:huf_dun",
	socialGroup = "huf_dun",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 39,
	chanceHit = 0.44,
	damageMin = 335,
	damageMax = 385,
	baseXp = 3915,
	baseHAM = 21000,
	baseHAMmax = 25000,
	armor = 0,
	resists = {155,135,175,-1,-1,-1,15,175,-1},
	meatType = "meat_herbivore",
	meatAmount = 1000,
	hideType = "hide_leathery",
	hideAmount = 1000,
	boneType = "bone_mammal",
	boneAmount = 1000,
	milk = 2 * 0,
	tamingChance = 0,
	ferocity = 4,
	pvpBitmask = ATTACKABLE,
	creatureBitmask = HERD,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/huf_dun.iff"},
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "unarmed",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = {	{"blindattack",""},	{"stunattack",""}	},
	secondaryAttacks = { }

}

CreatureTemplates:addCreatureTemplate(huf_dun, "huf_dun")
