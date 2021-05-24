enhanced_force_jawa_lord = Creature:new {
	customName = "Zek Honks",
	socialGroup = "geonosian_creature",
	pvpFaction = "",
	faction = "",
	level = 500,
	chanceHit = 45, -- Always Hits
	damageMin = 1100,
	damageMax = 2900,
	baseXp = 9296845,
	baseHAM = 2306000,
	baseHAMmax = 2652000,
	armor = 2,
	-- {kinetic,energy,blast,heat,cold,electricity,acid,stun,ls}
	resists = {65,65,65,65,65,65,65,65,25},
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
	templates = {"object/mobile/gungan_s03_male.iff"},
	-- templates = {"object/mobile/dressed_ewok_m_09.iff"},

	scale = 1.5,
	lootGroups = {
		{
			groups = {
				{group = "geonosian_epic", chance = 100 * (100000)},
			},
			lootChance = 75 * (100000),
		},		
		{
			groups = {
				{group = "inf_gold_2", chance =    100 * (100000)}
			},
			lootChance = 8 * (100000)
		},
		{
	        groups =
			{
				{group = "mastery_geocave_jewelry", chance = 100 * (100000)},
			},
			lootChance = 75 * (100000),
		},
		-- CLOTHING ATTACHMENTS LOOT GROUP CACHE --
		{
	        groups =
			{
				{group = "clothing_attachments", chance = 100 * (100000)},
			},
			lootChance = 65 * (100000),
		},
		{
	        groups =
			{
				{group = "clothing_attachments", chance = 100 * (100000)},
			},
			lootChance = 65 * (100000),
		},
		{
	        groups =
			{
				{group = "armor_attachments", chance = 100 * (100000)},
			},
			lootChance = 25 * (100000),
		},
		{
	        groups =
			{
				{group = "clothing_attachments", chance = 100 * (100000)},
			},
			lootChance = 65 * (100000),
		},
		-- CLOTHING ATTACHMENTS LOOT GROUP CACHE --
		{
			groups =
			{
				{group = "power_crystals", chance = 100 * (100000)},
			},
			lootChance = 50 * (100000),
		},
		{
			groups =
			{
				{group = "power_crystals", chance = 100 * (100000)},
			},
			lootChance = 50 * (100000),
		},
		{
			groups =
			{
				{group = "power_crystals", chance = 100 * (100000)},
			},
			lootChance = 50 * (100000),
		},
		{
			groups =
			{
				{group = "power_crystals", chance = 100 * (100000)},
			},
			lootChance = 50 * (100000),
		},
		{
			groups =
			{
				{group = "pistols", chance = 2500000},
				{group = "rifles", chance = 2500000},
				{group = "carbines", chance = 2500000},
				{group = "melee_weapons", chance = 2500000},
			},
			lootChance = 75 * (100000),
		},
		{
			groups =
			{
				{group = "pistols", chance = 2500000},
				{group = "rifles", chance = 2500000},
				{group = "carbines", chance = 2500000},
				{group = "melee_weapons", chance = 2500000},
			},
			lootChance = 75 * (100000),
		},
		-- Jedi Specific Loot Group
		{
			groups =
			{
				{group = "named_crystals", chance = 100 * (100000)},
			},
			lootChance = 25 * (100000),
		},
		{
			groups =
			{
				{group = "named_crystals", chance = 100 * (100000)},
			},
			lootChance = 25 * (100000),
		},
		{
			groups =
			{
				{group = "jedi_clothing_attachments", chance = 100 * (100000)},
			},
			lootChance = 2000000,
		},
		{ 	-- Jedi Specific Loot Group 2 - 40%
			groups ={
				{group = "jedi_clothing_attachments", chance = 25 * (100000)},	-- 40% * 25% = 10%
				{group = "ngerobes1", chance = 3750000},	                 -- 40% * 37.5% = 15%
				{group = "ngerobes2", chance = 3750000}	                    -- 40% * 37.5% = 15%

			},
			lootChance = 4000000,
		},
	},
	weapons = {"light_jedi_weapons"},
	conversationTemplate = "",
	attacks = merge(lightsabermaster,forcepowermaster)
}

CreatureTemplates:addCreatureTemplate(enhanced_force_jawa_lord, "enhanced_force_jawa_lord")
