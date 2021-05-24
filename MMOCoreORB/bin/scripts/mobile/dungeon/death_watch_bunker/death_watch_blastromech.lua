death_watch_blastromech = Creature:new {
	objectName = "@mob/creature_names:blastromech",
	socialGroup = "death_watch",
	faction = "",
	level = 300,
	chanceHit = 0.75,
	damageMin = 200,
	damageMax = 300,
	baseXp = 7668,
	baseHAM = 32000,
	baseHAMmax = 55000,
	armor = 2,
	resists = {30,30,30,30,30,30,30,30,30},
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
	creatureBitmask = PACK + KILLER,
	optionsBitmask = AIENABLED,
	diet = HERBIVORE,

	templates = {"object/mobile/blastromech.iff"},

	lootGroups = {
		{
			groups = {
				{group = "death_watch_bunker_commoners", chance = 10000000},
			},
			lootChance = 2500000
		},
		{
			groups = {
				{group = "death_watch_bunker_commoners", chance = 10000000},
			},
			lootChance = 2500000
		},
		{
			groups = {
				{group = "armor_attachments", chance = 10000000},
			},
			lootChance = 1000000
		},
		{
			groups = {
				{group = "armor_attachments", chance = 10000000},
			},
			lootChance = 1000000
		},
		{
			groups = {
				{group = "death_watch_bunker_lootbox", chance = 10000000},
			},
			lootChance = 3500000
		},
	},
	conversationTemplate = "",
	weapons = {"st_bombardier_weapons"},
	attacks = merge(commandomaster),
		attacks = {{"intimidationattack","stateAccuracyBonus=100"},
				{"flamecone2","accuracyBonus=50"},{"flamecone2","accuracyBonus=100"},{"flamesingle2","accuracyBonus=75"},
			},

}

CreatureTemplates:addCreatureTemplate(death_watch_blastromech, "death_watch_blastromech")
