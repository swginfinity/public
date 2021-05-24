spiderclan_stalker = Creature:new {
	objectName = "@mob/creature_names:spider_nightsister_stalker",
	randomNameType = NAME_GENERIC,
	randomNameTag = true,
	socialGroup = "spider_nightsister",
	faction = "spider_nightsister",
	level = 136,
	chanceHit = 0.85,
	damageMin = 595,
	damageMax = 900,
	baseXp = 9150,
	baseHAM = 40000,
	baseHAMmax = 55000,
	armor = 1,
	-- {kinetic,energy,blast,heat,cold,electricity,acid,stun,ls}
	resists = {40,60,40,75,75,75,75,75,-1},
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
	creatureBitmask = PACK + KILLER + STALKER,
	optionsBitmask = AIENABLED,
	diet = HERBIVORE,

	templates = {"object/mobile/dressed_dathomir_spider_nightsister_stalker.iff"},
	lootGroups = {
		{
			groups = {
				{group = "power_crystals", chance = 2500000},
				{group = "color_crystals", chance = 2000000},
				{group = "clothing_attachments", chance = 1500000},   -- 55% * 15% = 8.25%
				{group = "armor_attachments", chance = 1500000},   -- 55% * 15% = 8.25%
				{group = "melee_weapons", chance = 2500000},
			},
			lootChance = 5500000,  -- 55% chance for this loot group
		},
		{
			groups = {
				
				{group = "rifles", chance = 2500000},
				{group = "pistols", chance = 2500000},
				{group = "carbines", chance = 2500000},
				{group = "wearables_common", chance = 2500000},
			},
			lootChance = 4500000,
		},
		{ -- Jedi Specific Loot Group
			groups =
			{
				{group = "jedi_clothing_attachments", chance = 5000000},    -- 7.5% * 50% = 3.75%
				{group = "named_crystals", chance = 5000000},
			},
			lootChance = 750000,   -- 7.5% chance for this group
		},
	},
	weapons = {"mixed_force_weapons"},
	conversationTemplate = "",
	attacks = merge(brawlermaster,pikemanmaster,forcewielder)
}

CreatureTemplates:addCreatureTemplate(spiderclan_stalker, "spiderclan_stalker")
