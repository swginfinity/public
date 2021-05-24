tusken_executioner = Creature:new {
	objectName = "@mob/creature_names:tusken_executioner",
	socialGroup = "tusken_raider",
	faction = "tusken_raider",
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
	creatureBitmask = PACK + KILLER,
	optionsBitmask = AIENABLED,
	diet = HERBIVORE,

	templates = {"object/mobile/tusken_raider.iff"},
	lootGroups = {
		{
			groups = {
				{group = "power_crystals", chance = 2500000},
				{group = "color_crystals", chance = 1000000},
				{group = "tusken_common", chance = 400000},
				{group = "clothing_attachments", chance = 1500000},  -- 90% * 15% = 13.5%
				{group = "armor_attachments", chance = 1500000},  -- 90% * 15% = 13.5%
				{group = "melee_weapons", chance = 2000000},
				{group = "flare_s_swoop_crafted", chance = 1100000},   -- 11% * 90% = 9.9%
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
				{group = "rifles", chance = 2500000},
				{group = "pistols", chance = 2500000},
				{group = "carbines", chance = 2500000},
				{group = "wearables_rare", chance = 1500000},
				{group = "wearables_scarce", chance = 1000000},
			},
			lootChance = 1500000, -- 75% chance for this group
		},
		{
			groups = {
				{group = "power_crystals", chance = 2000000},
				{group = "armor_attachments", chance = 2000000},      -- 50% * 20% = 10%
				{group = "clothing_attachments", chance = 2000000},   -- 50% * 20% = 10%
				{group = "wearables_rare", chance = 2000000},
				{group = "wearables_scarce", chance = 1000000},
				{group = "vehicle_house_group", chance = 1000000},    -- 50% * 10% = 5%
			},
			lootChance = 5000000,  -- 50% chance for this group
		},
	},
	weapons = {"tusken_weapons"},
	conversationTemplate = "",
	attacks = merge(marksmanmaster,brawlermaster,fencermaster,riflemanmaster)
}

CreatureTemplates:addCreatureTemplate(tusken_executioner, "tusken_executioner")
