binayre_hooligan = Creature:new {
	objectName = "@mob/creature_names:binayre_hooligan",
	randomNameType = NAME_GENERIC,
	randomNameTag = true,
	socialGroup = "binayre",
	faction = "binayre",
	mobType = MOB_NPC,
	level = 9,
	level = 57,
	chanceHit = 0.6,
	damageMin = 370,
	damageMax = 450,
	baseXp = 5281,
	baseHAM = 9600,
	baseHAMmax = 10400,
	armor = 0,
	resists = {35,20,5,0,0,25,0,12.5,-1},	
	meatType = "",
	meatAmount = 0,
	hideType = "",
	hideAmount = 0,
	boneType = "",
	boneAmount = 0,
	milk = 2 * 0,
	tamingChance = 0,
	ferocity = 0,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK,
	optionsBitmask = AIENABLED,
	diet = HERBIVORE,

	templates = {
		"object/mobile/dressed_binayre_hooligan_zabrak_female_01.iff",
		"object/mobile/dressed_binayre_hooligan_zabrak_male_01.iff"},
	lootGroups = {
		{
			groups = {
				{group = "junk", chance = 2450000},
				{group = "tailor_components", chance = 1500000},
				{group = "loot_kit_parts", chance = 1500000},
				{group = "binayre_common", chance = 4550000}
			}
		},
		{
			groups = {
				{group = "pelgo_red", chance = 100 * (100000)},
			},
			lootChance = 3 * (100000)
		},
	},
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "pirate_weapons_light",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = merge(marksmanmid,brawlermid),
	secondaryAttacks = { },
	reactionStf = "@npc_reaction/slang",

}

CreatureTemplates:addCreatureTemplate(binayre_hooligan, "binayre_hooligan")
