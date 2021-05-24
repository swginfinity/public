death_watch_overlord = Creature:new {
	objectName = "",
	customName = "Drartih Oure (a Death Watch Overlord)",
	socialGroup = "death_watch",
	faction = "",
	level = 315,
	chanceHit = 1.3,
	damageMin = 1645,
	damageMax = 2500,
	baseXp = 20948,
	baseHAM = 350000,
	baseHAMmax = 550000,
	armor = 3,
	resists = {80,80,90,80,45,45,100,70,25},
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
	optionsBitmask = AIENABLED + BOSS,
	diet = HERBIVORE,
	scale = 1.15,

	templates = {"object/mobile/dressed_death_watch_gold.iff"},
	lootGroups = {
		{
			groups = {
				{group = "death_watch_bunker_overlord_shared", chance =  10000000}
			},
			lootChance = 10000000
		},
		{
			groups = {
				{group = "death_watch_bunker_overlord_shared", chance =  500000},
				{group = "death_watch_bunker_overlord_quest", chance  = 9500000}
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
			lootChance = 2000000
		},
		{
			groups = {
				{group = "death_watch_bunker_lootbox", chance = 10000000},
			},
			lootChance = 3500000
		},
		{
			groups =
			{
				{group = "power_crystals", chance = 10000000},
			},
			lootChance = 3000000,
		},
		{--- Musty Bunker Group
			groups = {
				{group = "musty_house_group", chance =  10000000},    -- 15% * 100% = 15%
			},
			lootChance = 1500000
		},
		{--- Mandalorian Armor Segment Group
			groups = {
				{group = "armor_segment_mandalorian", chance =  10000000},    -- 10 * 100% = 10
			},
			lootChance = 1000000
		}
	},
	weapons = {"dark_trooper_weapons"},
	conversationTemplate = "",
	attacks = merge(riflemanmaster,pikemanmaster,marksmanmaster),
	attacks = {{"intimidationattack","stateAccuracyBonus=100"},
		},
}

CreatureTemplates:addCreatureTemplate(death_watch_overlord, "death_watch_overlord")
