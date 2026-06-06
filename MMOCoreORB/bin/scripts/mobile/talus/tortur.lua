tortur = Creature:new {
	objectName = "@mob/creature_names:tortur",
	socialGroup = "tortur",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 58,
	chanceHit = 1.75,
	damageMin = 475,
	damageMax = 660,
	baseXp = 5025,
	baseHAM = 11000,
	baseHAMmax = 13000,
	armor = 0,
	resists = {145,25,180,180,170,-1,-1,75,-1},
	meatType = "meat_carnivore",
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

	templates = {"object/mobile/meager_tortur.iff"},
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "unarmed",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = {	{"knockdownattack",""},	{"dizzyattack",""}},
	secondaryAttacks = { }

}

CreatureTemplates:addCreatureTemplate(tortur, "tortur")
