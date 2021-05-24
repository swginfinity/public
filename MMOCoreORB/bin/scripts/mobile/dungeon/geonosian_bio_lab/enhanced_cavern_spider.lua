enhanced_cavern_spider = Creature:new {
	customName = "an Enhanced Cavern Spider",
	socialGroup = "geonosian_creature",
	pvpFaction = "",
	faction = "",
	level = 230,
	chanceHit = 33.36,
	damageMin = 650,
	damageMax = 960,
	baseXp = 19822,
	baseHAM = 46200,
	baseHAMmax = 49400,
	armor = 0,
	resists = {45,45,35,25,45,30,35,25,5},
	meatType = "",
	meatAmount = 0,
	hideType = "",
	hideAmount = 0,
	boneType = "",
	boneAmount = 0,
	milk = 0,
	tamingChance = 0,
	ferocity = 0,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + KILLER + STALKER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/gaping_spider.iff"},
	scale = 1.1,
	lootGroups = {
		{
	        groups =
			{
				{group = "armor_attachments", chance = 10000000},
			},
			lootChance = 1500000,
		},
		{
	        groups =
			{
				{group = "clothing_attachments", chance = 10000000},
			},
			lootChance = 700000,
		},
		{
			groups =
			{
				{group = "fire_breathing_spider", chance = 10000000},
			},
			lootChance = 1500000,
		},
	 	{
	        groups =
			{

				{group = "geonosian_relic", chance = 2500000},
				{group = "geonosian_common", chance = 6000000},
				{group = "mastery_geocave_jewelry", chance = 1500000},
			},
			lootChance = 7000000,
		},
		{
			groups = 
			{
				{group = "pistols", chance = 2500000},
				{group = "rifles", chance = 2500000},
				{group = "carbines", chance = 2500000},
				{group = "melee_weapons", chance = 2500000},
			},
			lootChance = 3500000,
		},
		{
			groups = 
			{
				{group = "pistols", chance = 2500000},
				{group = "rifles", chance = 2500000},
				{group = "carbines", chance = 2500000},
				{group = "melee_weapons", chance = 2500000},
			},
			lootChance = 3500000,
		},
	},
	weapons = {},
	conversationTemplate = "",
	attacks = {
		{"intimidationattack","intimidationChance=50"},
		{"mildpoison",""}
	}
}

CreatureTemplates:addCreatureTemplate(enhanced_cavern_spider, "enhanced_cavern_spider")
