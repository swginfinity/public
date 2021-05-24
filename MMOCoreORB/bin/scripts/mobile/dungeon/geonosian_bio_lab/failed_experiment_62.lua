failed_experiment_62 = Creature:new {
	customName = "Failed Experiment #62",
	socialGroup = "geonosian_creature",
	pvpFaction = "",
	faction = "",
	level = 300,
	chanceHit = 35.5,
	damageMin = 2015,
	damageMax = 2580,
	baseXp = 49884,
	baseHAM = 551600,
	baseHAMmax = 648000,
	armor = 2,
	-- {kinetic,energy,blast,heat,cold,electricity,acid,stun,ls}
	resists = {0,0,0,0,0,0,0,0,0},
	meatType = "",
	meatAmount = 0,
	hideType = "",
	hideAmount = 0,
	boneType = "",
	boneAmount = 0,
	milk = 0,
	tamingChance = 0,
	ferocity = 25,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + KILLER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/acklay_hue.iff"},
	scale = 0.5,
	lootGroups = {
		{
	        groups =
			{
				{group = "geonosian_epic", chance = 10000000},
			},
			lootChance = 3500000,
		},
		{
	        groups =
			{
				{group = "armor_attachments", chance = 10000000},
			},
			lootChance = 3500000,
		},
		{
	        groups =
			{
				{group = "clothing_attachments", chance = 10000000},
			},
			lootChance = 2500000,
		},
		{
			groups =
			{
				{group = "geonosian_cubes", chance = 10000000},
			},
			lootChance = 4500000,
		},
		{
			groups =
			{
				{group = "acklay", chance = 10000000},
			},
			lootChance = 4500000,
		},
		{
			groups =
			{
				{group = "acklay", chance = 10000000},
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
			lootChance = 5000000,
		},
		{
			groups =
			{
				{group = "pistols", chance = 2500000},
				{group = "rifles", chance = 2500000},
				{group = "carbines", chance = 2500000},
				{group = "melee_weapons", chance = 2500000},
			},
			lootChance = 5000000,
		},
	},
	weapons = {},
	conversationTemplate = "",
	attacks = {
		{"creatureareacombo",""},
		{"posturedownattack","postureDownChance=75"}
	}
}

CreatureTemplates:addCreatureTemplate(failed_experiment_62, "failed_experiment_62")
