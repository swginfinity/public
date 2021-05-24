nightsister_elder = Creature:new {
	objectName = "@mob/creature_names:nightsister_elder",
	randomNameType = NAME_GENERIC,
	randomNameTag = true,
	socialGroup = "nightsister",
	faction = "nightsister",
	level = 331, -- Should drop Flawless Crystals
	chanceHit = 15.25,
	damageMin = 1120,
	damageMax = 1950,
	baseXp = 26654,
	baseHAM = 321000,
	baseHAMmax = 392000,
	armor = 3,
	-- {kinetic,energy,blast,heat,cold,electricity,acid,stun,ls}
	resists = {65,25,25,75,75,70,55,70,-1},
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
			groups = {
				{group = "power_crystals", chance = 2000000},
				{group = "color_crystals", chance = 1500000},
				{group = "nightsister_common", chance = 3000000},
				{group = "clothing_attachments", chance = 1500000},  -- 90% * 15% = 13.5%
				{group = "melee_weapons", chance = 2000000},

			},
			lootChance = 9000000,  -- 90% chance for this group
		},
		{
			groups = {

				{group = "rifles", chance = 2500000},
				{group = "pistols", chance = 2500000},
				{group = "carbines", chance = 2500000},
				{group = "wearables_rare", chance = 1500000},
				{group = "wearables_scarce", chance = 1000000},
			},
			lootChance = 7500000, -- 75% chance for this group
		},
		{
			groups = {
				{group = "power_crystals", chance = 2000000},
				{group = "nightsister_common", chance = 2000000},
				{group = "clothing_attachments", chance = 2000000},   -- 50% * 20% = 10%
				{group = "wearables_rare", chance = 2000000},
				{group = "wearables_scarce", chance = 2000000},
			},
			lootChance = 5000000,  -- 50% chance for this group
		},
		{
			groups = {
				{group = "nightsister_common", chance = 8000000},
				{group = "nightsister_bicep_r_s01", chance = 2000000},   -- 1% * 20% = 0.2%
			},
			lootChance = 100000,    -- 1% chance for this group
		},
		{ -- Nightsister Clothing Loot Group
			groups =
			{
				{group = "nightsister_clothing", chance = 10000000},    -- 12.5% * 100% = 12.5%
			},
			lootChance = 1250000,
		},
		{ -- Jedi Specific Loot Group
			groups =
			{
				{group = "named_crystals", chance = 10000000},  -- 15% * 100% = 15%
			},
			lootChance = 1500000, -- 15% chance for this group
		},
		{ -- Jedi Specific Loot Group
			groups =
			{
				{group = "jedi_clothing_attachments", chance = 10000000}, -- 10% * 100% = 10%
			},
			lootChance = 1000000,  -- 10% chance for this group
		},
	},
	weapons = {"mixed_force_weapons"},
	conversationTemplate = "",
	attacks = merge(tkamaster,swordsmanmaster,fencermaster,pikemanmaster,brawlermaster,forcepowermaster)
}

CreatureTemplates:addCreatureTemplate(nightsister_elder, "nightsister_elder")
