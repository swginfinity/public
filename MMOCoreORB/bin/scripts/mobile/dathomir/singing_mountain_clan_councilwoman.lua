singing_mountain_clan_councilwoman = Creature:new {
	objectName = "@mob/creature_names:singing_mtn_clan_councilwoman",
	randomNameType = NAME_GENERIC,
	randomNameTag = true,
	socialGroup = "mtn_clan",
	faction = "mtn_clan",
	level = 331, -- Should drop Flawless Crystals
	chanceHit = 10.5,
	damageMin = 1190,
	damageMax = 2050,
	baseXp = 24180,
	baseHAM = 261000,
	baseHAMmax = 320000,
	armor = 3,
	-- {kinetic,energy,blast,heat,cold,electricity,acid,stun,ls}
	resists = {75,35,35,75,75,75,75,75,-1},
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

	templates = {"object/mobile/dressed_dathomir_sing_mt_clan_councilwoman.iff"},
	lootGroups = {
		{
			groups = {
				{group = "power_crystals", chance = 2500000},
				{group = "color_crystals", chance = 2500000},
				{group = "clothing_attachments", chance = 1500000},   -- 90% * 15% = 13.5%
				{group = "armor_attachments", chance = 1500000},      -- 90% * 15% = 13.5%
				{group = "melee_weapons", chance = 2000000},
			},
			lootChance = 9000000,  -- 90% chance for this grouop
		},
		{
			groups = {

				{group = "rifles", chance = 2500000},
				{group = "pistols", chance = 2500000},
				{group = "carbines", chance = 2500000},
				{group = "wearables_rare", chance = 1500000},
				{group = "wearables_scarce", chance = 1000000},
			},
			lootChance = 7500000,   -- 75% chance for this group
		},
		{
			groups = {
				{group = "power_crystals", chance = 2000000},
				{group = "clothing_attachments", chance = 2000000},   -- 50% * 20% = 10%
				{group = "armor_attachments", chance = 2000000},      -- 50% * 20% = 10%
				{group = "wearables_rare", chance = 2000000},
				{group = "wearables_scarce", chance = 1000000},
				{group = "vehicle_house_group", chance = 1000000},    -- 50% * 10% = 5%
			},
			lootChance = 5000000,    -- 50% chance for this loot group
		},
		{ -- special paintings
			groups =
			{
				{group = "art_palpatine2", chance = 10000000},
			},
			lootChance = 8 * (100000),
		},
		{ -- Jedi Specific Loot Group
			groups =
			{
				{group = "named_crystals", chance = 10000000},
			},
			lootChance = 1500000,
		},
		{ -- Jedi Specific Loot Group
			groups =
			{
				{group = "jedi_clothing_attachments", chance = 10000000}, -- 100% * 10% = 10%
			},
			lootChance = 1000000,
		},
	},
	weapons = {"mixed_force_weapons"},
	conversationTemplate = "",
	attacks = merge(brawlermaster,pikemanmaster,forcepowermaster)
}

CreatureTemplates:addCreatureTemplate(singing_mountain_clan_councilwoman, "singing_mountain_clan_councilwoman")
