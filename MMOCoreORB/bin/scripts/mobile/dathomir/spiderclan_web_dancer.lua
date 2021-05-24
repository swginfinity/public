spiderclan_web_dancer = Creature:new {
	objectName = "@mob/creature_names:spider_nightsister_web_dancer",
	randomNameType = NAME_GENERIC,
	randomNameTag = true,
	socialGroup = "spider_nightsister",
	faction = "spider_nightsister",
	level = 87,
	chanceHit = 1.15,
	damageMin = 520,
	damageMax = 750,
	baseXp = 8315,
	baseHAM = 12000,
	baseHAMmax = 15000,
	armor = 1,
	-- {kinetic,energy,blast,heat,cold,electricity,acid,stun,ls}
	resists = {75,65,75,65,75,65,65,75,-1},
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
	creatureBitmask = PACK + KILLER,
	optionsBitmask = AIENABLED,
	diet = HERBIVORE,

	templates = {"object/mobile/dressed_dathomir_spider_nightsister_web_dancer.iff"},
		lootGroups = {
		{
			groups = {
				{group = "power_crystals", chance = 2500000},
				{group = "color_crystals", chance = 2000000},
				{group = "clothing_attachments", chance = 1500000},   -- 45% * 15% = 6.75%
				{group = "armor_attachments", chance = 1500000},   -- 45% * 15% = 6.75%
				{group = "melee_weapons", chance = 2500000},
			},
			lootChance = 4500000,  -- 45% chance for this loot group
		},
		{
			groups = {
				
				{group = "rifles", chance = 2500000},
				{group = "pistols", chance = 2500000},
				{group = "carbines", chance = 2500000},
				{group = "wearables_common", chance = 2500000},
			},
			lootChance = 3500000,
		},
	},
	weapons = {"mixed_force_weapons"},
	conversationTemplate = "",
	attacks = merge(brawlermaster,pikemanmaster,swordsmanmaster) --NS Powers Removed
}

CreatureTemplates:addCreatureTemplate(spiderclan_web_dancer, "spiderclan_web_dancer")
