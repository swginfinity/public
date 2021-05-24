death_watch_wraith = Creature:new {
	objectName = "@mob/creature_names:mand_bunker_dthwatch_silver",
	randomNameType = NAME_GENERIC,
	randomNameTag = true,
	socialGroup = "death_watch",
	faction = "",
	level = 298,
	chanceHit = 1.25,
	damageMin = 1020,
	damageMax = 1750,
	baseXp = 16794,
	baseHAM = 120000,
	baseHAMmax = 250000,
	armor = 2,
	resists = {75,75,90,80,45,45,100,70,20},
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
	creatureBitmask = KILLER,
	optionsBitmask = AIENABLED,
	diet = HERBIVORE,
	scale = 1.15,

	templates = {"object/mobile/dressed_death_watch_silver.iff"},
	lootGroups = {
		{
			groups = {
				{group = "death_watch_bunker_commoners",   chance = 6500000},
				{group = "death_watch_bunker_lieutenants", chance = 3500000},
			},
			lootChance = 5000000
		},
		{
			groups = {
				{group = "clothing_attachments", chance = 10000000},
			},
			lootChance = 500000
		},
		{
			groups = {
				{group = "armor_attachments", chance = 10000000},
			},
			lootChance = 750000
		},
		{
			groups = {
				{group = "death_watch_bunker_ingredient_binary",  chance = 10000000}
			},
			lootChance = 1750000
		},
		{
			groups = {
				{group = "death_watch_bunker_ingredient_protective",  chance = 10000000}
			},
			lootChance = 1750000
		},
		{
			groups = { -- AA
				{group = "armor_attachments", chance = 10000000},  -- 7.5%
			},
			lootChance = 750000,
		},

	},
	weapons = {"pirate_weapons_heavy"},
	conversationTemplate = "",
	attacks = merge(bountyhuntermaster,marksmanmaster,brawlermaster,swordsmanmaster,pistoleermaster,fencermaster,pikemanmaster,riflemanmaster)
}

CreatureTemplates:addCreatureTemplate(death_watch_wraith, "death_watch_wraith")
