black_sun_henchman = Creature:new {
	objectName = "@mob/creature_names:mand_bunker_blksun_henchman",
	randomNameType = NAME_GENERIC,
	randomNameTag = true,
	socialGroup = "death_watch",
	faction = "",
	level = 176,
	chanceHit = 0.75,
	damageMin = 520,
	damageMax = 750,
	baseXp = 7207,
	baseHAM = 82000,
	baseHAMmax = 150000,
	armor = 1,
	resists = {35,35,50,25,45,60,25,30,-1},
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
	creatureBitmask = KILLER,
	optionsBitmask = AIENABLED,
	diet = HERBIVORE,
	scale = 1.15,

	templates = {"object/mobile/dressed_black_sun_henchman.iff"},
	lootGroups = {
		{
			groups = {
				{group = "pistols", chance = 3300000},
				{group = "rifles", chance = 3400000},
				{group = "carbines", chance = 3300000},
			},
			lootChance = 7500000,
		},
		{
			groups = {
				{group = "junk", chance = 3400000},
				{group = "wearables_common", chance = 3300000},
				{group = "wearables_uncommon", chance = 3300000},
			},
			lootChance = 7500000,
		},
		{
			groups = {
				{group = "jetpack_base", chance = 10000000},
			},
			lootChance = 1000000
		},
		{
			groups = {
				{group = "bounty_hunter_armor", chance = 10000000},
			},
			lootChance = 1500000
		},
		{
			groups = {
				{group = "bounty_hunter_armor", chance = 10000000},
			},
			lootChance = 1500000
		},
		{
			groups = {
				{group = "bounty_hunter_armor", chance = 10000000},
			},
			lootChance = 1500000
		},
	},
	weapons = {"pirate_weapons_heavy"},
	conversationTemplate = "",
	attacks = merge(bountyhuntermaster,marksmanmaster,brawlermaster)
}

CreatureTemplates:addCreatureTemplate(black_sun_henchman, "black_sun_henchman")
