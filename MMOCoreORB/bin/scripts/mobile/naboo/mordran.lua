mordran = Creature:new {
	objectName = "@mob/creature_names:mordran",
	socialGroup = "borvo",
	faction = "borvo",
	level = 280,
	chanceHit = 0.750000,
	damageMin = 950,
	damageMax = 1850,
	baseXp = 7484,
	baseHAM = 140232,
	baseHAMmax = 250404,
	armor = 0,
	resists = {0,75,0,45,80,0,0,0,-1},
	meatType = "",
	meatAmount = 0,
	hideType = "",
	hideAmount = 0,
	boneType = "",
	boneAmount = 0,
	milk = 2 * 0,
	tamingChance = 0.000000,
	ferocity = 0,
	pvpBitmask = ATTACKABLE + AGGRESSIVE + ENEMY,
	creatureBitmask = PACK + KILLER,
	diet = HERBIVORE,

	templates = {"object/mobile/mordran.iff"},
	lootGroups = {
		{
			groups = {
				{group = "junk", chance = 2000000},
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
	weapons = {"mordran_weapons"},
	reactionStf = "@npc_reaction/slang",
	attacks = merge(brawlermaster, swordsmanmaster)
}

CreatureTemplates:addCreatureTemplate(mordran, "mordran")
