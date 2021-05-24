lord_nyax = Creature:new {
	objectName = "@mob/creature_names:lord_nyax",
	socialGroup = "followers_of_lord_nyax",
	faction = "followers_of_lord_nyax",
	level = 290,
	chanceHit = 4.9,
	damageMin = 950,
	damageMax = 1530,
	baseXp = 12235,
	baseHAM = 189400,
	baseHAMmax = 245343,
	armor = 2,
	resists = {80,45,40,20,50,100,10,15,-1},
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

	templates = {"object/mobile/dressed_lord_nyax.iff"},
	lootGroups = {
		{
		groups = {
				{group = "weapons_all", chance = 4000000},
				{group = "nyax", chance = 3000000},
				{group = "crystal_strength_of_luminaria", chance = 1000000},
				{group = "armor_attachments", chance = 1000000},
				{group = "clothing_attachments", chance = 1000000}
			},
			lootChance = 10000000,
		},
		{
			groups = {
				{group = "color_crystals", chance = 2000000},
				{group = "tailor_components", chance = 1500000},
				{group = "loot_kit_parts", chance = 1500000},
				{group = "composite_armor", chance = 1500000},
				{group = "melee_two_handed", chance = 1500000},
				{group = "clothing_attachments", chance = 500000},
				{group = "armor_attachments", chance = 500000},
				{group = "wearables_common", chance = 1000000}
			},
			lootChance = 10000000,
		},
		{
	    		groups =
			{
				{group = "veteran_rewards_1080_days", chance = 10000000},
			},
			lootChance = 1000000,
		},
		{
	    		groups =
			{
				{group = "veteran_rewards_810_days", chance = 10000000},
			},
			lootChance = 1000000,
		},
	},
	weapons = {"nyaxs_weapons"},
	conversationTemplate = "",
	reactionStf = "@npc_reaction/fancy",
	attacks = merge(marksmanmaster,riflemanmaster,carbineermaster,brawlermaster,swordsmanmaster)
}

CreatureTemplates:addCreatureTemplate(lord_nyax, "lord_nyax")
