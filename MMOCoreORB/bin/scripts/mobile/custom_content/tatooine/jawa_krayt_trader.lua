jawa_krayt_trader = Creature:new {
	objectName = "@mob/creature_names:jawa_krayt",
	randomNameType = NAME_GENERIC,
	randomNameTag = true,
	socialGroup = "krayt_trader",
	faction = "jawa",
	mobType = MOB_NPC,
	level = 60,
	chanceHit = 0.25,
	damageMin = 350,
	damageMax = 555,
	baseXp = 1470,
	baseHAM = 3200,
	baseHAMmax = 4300,
	armor = 0,
	resists = {0,0,0,0,0,0,0,-1,-1},
	meatType = "",
	meatAmount = 0,
	hideType = "",
	hideAmount = 0,
	boneType = "",
	boneAmount = 0,
	milk = 0,
	tamingChance = 0,
	ferocity = 0,
	pvpBitmask = ATTACKABLE,
	creatureBitmask = PACK,
	optionsBitmask = AIENABLED,
	diet = HERBIVORE,

	templates = {"object/mobile/jawa_male.iff"},
	lootGroups = {
		{
			groups = {
				{group = "junk", chance = 5000000},
				{group = "jawa_common", chance = 1500000},
				{group = "armor_all", chance = 800000},
				{group = "tailor_components", chance = 1000000},
				{group = "loot_kit_parts", chance = 1500000},
				{group = "color_crystals", chance = 200000}
			}
		},
		{
			groups = {
				{group = "krayt_body_jawa", chance = 100 * (100000)},
			},
			lootChance = 4 * (100000),
		},

	},
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "jawa_weaker_weapons",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = merge(marksmannovice,brawlernovice),
	secondaryAttacks = { },
}

CreatureTemplates:addCreatureTemplate(jawa_krayt_trader, "jawa_krayt_trader")
