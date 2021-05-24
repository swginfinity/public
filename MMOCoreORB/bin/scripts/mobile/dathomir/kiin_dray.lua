kiin_dray = Creature:new {
	objectName = "@mob/creature_names:gaping_spider_recluse_giant_kiin_dray",
	socialGroup = "spider_nightsister",
	faction = "spider_nightsister",
	level = 331, -- Should drop Flawless Crystals
	chanceHit = 15.25,
	damageMin = 2270,
	damageMax = 4250,
	baseXp = 26654,
	baseHAM = 321000,
	baseHAMmax = 392000,
	armor = 2,
	level = 350, -- Should drop Flawless Crystals
	-- {kinetic,energy,blast,heat,cold,electricity,acid,stun,ls}
	resists = {175,175,135,195,145,155,145,165,140},
	meatType = "meat_insect",
	meatAmount = 500,
	hideType = "",
	hideAmount = 0,
	boneType = "",
	boneAmount = 0,
	milk = 2 * 0,
	tamingChance = 0,
	ferocity = 30,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + KILLER + STALKER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/gaping_spider_recluse_giant_kiin_dray.iff"},
	scale = 4.0,
	lootGroups = {
		{
			groups = {
				{group = "power_crystals", chance = 2500000},
				{group = "nightsister_common", chance = 2500000},
				{group = "clothing_attachments", chance = 1500000},   -- 90% * 15% = 13.5%
				{group = "armor_attachments", chance = 1500000},      -- 90% * 15% = 13.5%
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
			lootChance = 7500000,   -- 75% chance for this group
		},
		{
			groups = {
				{group = "power_crystals", chance = 2000000},
				{group = "clothing_attachments", chance = 2000000},   -- 50% * 20% = 10%
				{group = "armor_attachments", chance = 2000000},      -- 50% * 20% = 10%
				{group = "nightsister_common", chance = 3000000},
				{group = "large_generic_windowed_house_s02_group", chance = 1000000},    -- 50% * 10% = 5%
			},
			lootChance = 5000000,    -- 50% chance for this loot group
		},
		{
			groups = {
				{group = "fire_breathing_spider", chance = 10000000},  -- 100% * 10% = 10%
			},
			lootChance = 1000000,    -- 10% chance for this loot group
		},
	},
	weapons = {"creature_spit_small_toxicgreen"},
	conversationTemplate = "",
	attacks = {
		{"stunattack","stunChance=85"},
		{"mediumpoison",""}
	}
}

CreatureTemplates:addCreatureTemplate(kiin_dray, "kiin_dray")
