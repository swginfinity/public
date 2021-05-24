gcw_elite_sand_trooper = Creature:new {
	objectName = "@mob/creature_names:elite_sand_trooper",
	randomNameType = NAME_STORMTROOPER,
	randomNameTag = true,
	socialGroup = "imperial",
	faction = "imperial",
	level = 120,
	chanceHit = 0.47,
	damageMin = 370,
	damageMax = 450,
	baseXp = 4370,
	baseHAM = 49700,
	baseHAMmax = 61900,
	armor = 0,
	resists = {5,5,40,100,25,20,20,20,5},
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
	creatureBitmask = PACK + KILLER,
	optionsBitmask = AIENABLED,
	diet = HERBIVORE,
	scale = 1.05,

	templates = {"object/mobile/dressed_stormtrooper_sand_trooper_m.iff"},

	lootGroups = {
		{
		groups = {
			{group = "color_crystals", chance = 100000},
			{group = "lockedcontainer", chance = 6550000},
			{group = "rifles", chance = 550000},
			{group = "pistols", chance = 550000},
			{group = "melee_weapons", chance = 550000},
			{group = "carbines", chance = 550000},
			{group = "clothing_attachments", chance = 25000},
			{group = "armor_attachments", chance = 25000},
			{group = "stormtrooper_common", chance = 100000},
			{group = "wearables_common", chance = 500000},
			{group = "wearables_uncommon", chance = 500000}
		},
			lootChance = 10000000,  -- 100% chance for this group
		},
		{
			groups = {
				{group = "weapons_all", chance = 3000000},
				{group = "melee_weapons", chance = 3000000},
				{group = "armor_attachments", chance = 3000000},
				{group = "lockedcontainer", chance = 1000000},
			},
			lootChance = 8000000,  -- 80% chance for this group
		},
		{
			groups = {
				{group = "power_crystals", chance = 3000000},
				{group = "clothing_attachments", chance = 3000000},
				{group = "armor_attachments", chance = 3000000},
				{group = "lockedcontainer", chance = 1000000},
			},
			lootChance = 1000000,  -- 10% chance for this group
		},
		{
			groups = {
				{group = "landspeeder_ab1", chance =10000000},
				},
			lootChance = 100000, -- 1% chance for this
		},
	},
	weapons = {"sandtrooper_weapons"},
	conversationTemplate = "",
	reactionStf = "@npc_reaction/stormtrooper",
	personalityStf = "@hireling/hireling_stormtrooper",
	attacks = merge(riflemanmaster,marksmanmaster,brawlermaster)
}

CreatureTemplates:addCreatureTemplate(gcw_elite_sand_trooper, "gcw_elite_sand_trooper")
