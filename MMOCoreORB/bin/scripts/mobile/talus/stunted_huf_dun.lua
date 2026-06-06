stunted_huf_dun = Creature:new {
	objectName = "@mob/creature_names:stunted_huf_dun",
	socialGroup = "huf_dun",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 32,
	chanceHit = 0.36,
	damageMin = 320,
	damageMax = 350,
	baseXp = 2888,
	baseHAM = 14000,
	baseHAMmax = 16000,
	armor = 0,
	resists = {150,120,160,0,-1,-1,10,175,-1},
	meatType = "meat_herbivore",
	meatAmount = 775,
	hideType = "hide_leathery",
	hideAmount = 800,
	boneType = "bone_mammal",
	boneAmount = 775,
	milk = 2 * 0,
	tamingChance = 0,
	ferocity = 0,
	pvpBitmask = ATTACKABLE,
	creatureBitmask = HERD,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/stunted_huf_dun.iff"},
	scale = 0.65,
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "unarmed",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = { {"intimidationattack",""} },
	secondaryAttacks = { }

}

CreatureTemplates:addCreatureTemplate(stunted_huf_dun, "stunted_huf_dun")
