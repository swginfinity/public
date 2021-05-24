failed_ns_clone = Creature:new {
	customName = "A Failed Nightsister Clone",
	socialGroup = "geonosian_creature",
	pvpFaction = "",
	faction = "",
	level = 275,
	chanceHit = 15.5,
	damageMin = 1620,
	damageMax = 2750,
	baseXp = 28654,
	baseHAM = 191000,
	baseHAMmax = 232000,
	armor = 2,
	-- {kinetic,energy,blast,heat,cold,electricity,acid,stun,ls}
	resists = {55,55,55,55,55,55,55,55,15},
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
	creatureBitmask = PACK + KILLER + HEALER,
	optionsBitmask = AIENABLED,
	diet = HERBIVORE,

	templates = {"object/mobile/dressed_dathomir_nightsister_elder.iff"},
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
			lootChance = 2500000,
		},
		{
	        groups =
			{
				{group = "armor_attachments", chance = 10000000},
			},
			lootChance = 2500000,
		},
		{
	        groups =
			{
				{group = "clothing_attachments", chance = 10000000},
			},
			lootChance = 2000000,
		},
		{
	        groups =
			{
				{group = "clothing_attachments", chance = 10000000},
			},
			lootChance = 2000000,
		},
		{
			groups =
			{
				{group = "nightsister_common", chance = 10000000},
			},
			lootChance = 6500000,
		},
		{
			groups = {
				{group = "power_crystals", chance = 10000000},
			},
			lootChance = 2500000,
		},
		{
			groups = {
				{group = "power_crystals", chance = 10000000},
			},
			lootChance = 2500000,
		},
		{
			groups = {
				{group = "power_crystals", chance = 10000000},
			},
			lootChance = 2500000,
		},
		{
			groups =
			{
				{group = "pistols", chance = 2500000},
				{group = "rifles", chance = 2500000},
				{group = "carbines", chance = 2500000},
				{group = "melee_weapons", chance = 2500000},
			},
			lootChance = 7500000,
		},
		{
			groups =
			{
				{group = "geonosian_cubes", chance = 10000000},
			},
			lootChance = 3500000,
		},

		-- Jedi Specific Loot Groups
		{
			groups =
			{
				{group = "named_crystals", chance = 10000000},
			},
			lootChance = 1000000,
		},
		{
			groups =
			{
				{group = "jedi_clothing_attachments", chance = 10000000},
			},
			lootChance = 850000,
		},
		{
			groups =
			{
				{group = "jedi_clothing_attachments", chance = 10000000},
			},
			lootChance = 1000000,
		},
	},
	weapons = {"dark_trooper_weapons"},
	conversationTemplate = "",
	attacks = merge(tkamaster,brawlermaster,forcewielder)
}

CreatureTemplates:addCreatureTemplate(failed_ns_clone, "failed_ns_clone")
