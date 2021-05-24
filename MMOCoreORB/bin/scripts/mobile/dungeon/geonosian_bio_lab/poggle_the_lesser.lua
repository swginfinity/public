poggle_the_lesser = Creature:new {
	customName = "Poggle the Lesser",
	socialGroup = "geonosian",
	faction = "",
	level = 400,
	chanceHit = 2.3,
	damageMin = 2650,
	damageMax = 4250,
	baseXp = 26654,
	baseHAM = 1023000,
	baseHAMmax = 1432400,
	armor = 3,
	-- {kinetic,energy,blast,heat,cold,electricity,acid,stun,ls}
	resists = {65,55,55,75,75,70,55,50,25},
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
	optionsBitmask = AIENABLED + BOSS,
	diet = HERBIVORE,
	scale = 1.5,

	templates = {"object/mobile/dressed_geonosian_warrior_03.iff"},
	lootGroups = {
		{
			groups = {
				{group = "power_crystals", chance = 2500000},
				{group = "color_crystals", chance = 2500000},
				{group = "armor_attachments", chance = 2500000},
				{group = "geonosian_epic", chance = 2500000},

			},
			lootChance = 9000000,  -- 90% chance for this group
		},
		{
			groups = {
				{group = "power_crystals", chance = 3000000},
				{group = "clothing_attachments", chance = 3000000},
				{group = "armor_attachments", chance = 3000000},
				{group = "geonosian_hard", chance = 1000000},
			},
			lootChance = 6000000,  -- 60% chance for this group
		},
		{
			groups = {
				{group = "power_crystals", chance =10000000},
				},
			lootChance = 5000000, -- 50% chance for 3rd crystal.
		},
		{ -- Poggle Loot #1
			groups = {
				{group = "geonosian_poggle", chance = 10000000},
			},
			lootChance = 4000000,    -- 20%
		},
		{ -- Poggle Loot #2
			groups = {
				{group = "geonosian_poggle", chance = 10000000},
			},
			lootChance = 2000000,    -- 20%
		},
		{ -- Lightsaber Pike
			groups = {
				{group = "lightsaber_pike", chance = 10000000},
			},
			lootChance = 1500000,    -- 15%
		},
		{ -- Jedi Specific Loot Group
			groups =
			{
				{group = "named_crystals", chance = 10000000},
			},
			lootChance = 10000000, -- Guarunteed Drop
		},
		{ -- Jedi Specific Loot Group
			groups =
			{
				{group = "jedi_clothing_attachments", chance = 10000000}, -- 20% * 100% = 20%
			},
			lootChance = 2000000,  -- 20% chance for this group
		},
	},
	weapons = {"geonosian_weapons"},
	conversationTemplate = "",
	attacks = merge(pistoleermaster,fencermaster),
	attacks = {{"intimidationattack","stateAccuracyBonus=65"}, {"creatureareableeding",""},
		},
}

CreatureTemplates:addCreatureTemplate(poggle_the_lesser, "poggle_the_lesser")
