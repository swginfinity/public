spiderclan_sentry = Creature:new {
	objectName = "@mob/creature_names:spider_nightsister_sentry",
	randomNameType = NAME_GENERIC,
	randomNameTag = true,
	socialGroup = "spider_nightsister",
	faction = "spider_nightsister",
	level = 77,
	chanceHit = 1.36,
	damageMin = 240,
	damageMax = 250,
	baseXp = 2730,
	baseHAM = 17200,
	baseHAMmax = 18800,
	armor = 0,
	-- {kinetic,energy,blast,heat,cold,electricity,acid,stun,ls}
	resists = {15,15,15,55,55,55,55,-1,-1},
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

	templates = {"object/mobile/dressed_dathomir_spider_nightsister_guardian.iff"},
	lootGroups = {
		{
			groups = {
				{group = "power_crystals", chance = 2500000},
				{group = "color_crystals", chance = 2000000},
				{group = "clothing_attachments", chance = 1500000},   -- 35% * 15% = 5.25%
				{group = "armor_attachments", chance = 1500000},   -- 35% * 15% = 5.25%
				{group = "melee_weapons", chance = 2500000},
			},
			lootChance = 3500000,  -- 35% chance for this loot group
		},
		{
			groups = {
				
				{group = "rifles", chance = 2500000},
				{group = "pistols", chance = 2500000},
				{group = "carbines", chance = 2500000},
				{group = "wearables_common", chance = 2500000},
			},
			lootChance = 2500000,
		},
	},
	weapons = {"mixed_force_weapons"},
	conversationTemplate = "",
	attacks = merge(brawlermaster,pikemanmaster)
}

CreatureTemplates:addCreatureTemplate(spiderclan_sentry, "spiderclan_sentry")
