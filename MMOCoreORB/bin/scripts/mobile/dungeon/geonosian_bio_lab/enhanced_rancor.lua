enhanced_rancor = Creature:new {
	customName = "an Escaped Dangerous Project",
	socialGroup = "geonosian_creature",
	pvpFaction = "",
	faction = "",
	level = 350,
	chanceHit = 45.5,
	damageMin = 3000,
	damageMax = 3800,
	baseXp = 79336,
	baseHAM = 950000,
	baseHAMmax = 1000000,
	armor = 3,
	-- {kinetic,energy,blast,heat,cold,electricity,acid,stun,ls}
	resists = {55,60,45,70,70,70,45,45,35},
	meatType = "meat_carnivore",
	meatAmount = 2322,
	hideType = "hide_leathery",
	hideAmount = 1323,
	boneType = "bone_mammal",
	boneAmount = 1322,
	milk = 0,
	tamingChance = 0,
	ferocity = 15,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + KILLER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/bull_rancor.iff"},
	scale = 1.5,
	lootGroups = {
		{
	        	groups =
			{
				{group = "geonosian_common", chance = 6000000},
				{group = "mastery_geocave_jewelry", chance = 4000000},
			},
			lootChance = 7500000,
		},
		{
	        	groups =
			{
				{group = "armor_attachments", chance = 10000000},
			},
			lootChance = 4500000,
		},
		{
	        	groups =
			{
				{group = "armor_attachments", chance = 10000000},
			},
			lootChance = 4500000,
		},
		{
	        	groups =
			{
				{group = "clothing_attachments", chance = 10000000},
			},
			lootChance = 4500000,
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
				{group = "geonosian_epic", chance = 10000000},
			},
			lootChance = 3500000,
		},
		{
			groups =
			{
				{group = "geonosian_epic", chance = 10000000},
			},
			lootChance = 2500000,
		},
		{
			groups = {
				{group = "geo_rancor", chance = 10000000},
			},
			lootChance = 5500000,
		},
		{
			groups = {
				{group = "geo_rancor", chance = 10000000},
			},
			lootChance = 4500000,
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
		{"creatureareaattack",""},
		{"stunattack","stunChance=70"},
		{"creatureareableeding",""}
	}
}

CreatureTemplates:addCreatureTemplate(enhanced_rancor, "enhanced_rancor")
