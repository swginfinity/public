krayt_dragon_queen = Creature:new {
	objectName = "@mob/creature_names:krayt_dragon_queen",
	socialGroup = "krayt",
	faction = "",
	level = 350,
	chanceHit = 65,
	damageMin = 3170,
	damageMax = 5550,
	baseXp = 28549,
	baseHAM = 710000,
	baseHAMmax = 1201000,
	armor = 3,
	-- {kinetic,energy,blast,heat,cold,electricity,acid,stun,ls}
	resists = {195,195,195,195,165,195,195,195,-1},
	meatType = "meat_carnivore",
	meatAmount = 1000,
	hideType = "hide_bristley",
	hideAmount = 950,
	boneType = "bone_mammal",
	boneAmount = 905,
	milk = 0,
	tamingChance = 0,
	ferocity = 30,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + KILLER + STALKER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,
	templates = {"object/mobile/krayt_dragon.iff"},
	scale = 2.5,
	lootGroups = {
		{
	        groups = {
				{group = "krayt_tissue_rare", chance = 2500000},
				{group = "krayt_dragon_common", chance = 3500000},
				{group = "armor_all", chance = 1500000},
				{group = "weapons_all", chance = 2500000},
			},
			lootChance = 10000000
		},
		{
			groups = {
				{group = "krayt_tissue_rare", chance = 2500000},
				{group = "krayt_dragon_common", chance = 3500000},
				{group = "weapons_all", chance = 4000000},
			},
			lootChance = 7500000,
		},
		{
			groups = {
				{group = "krayt_tissue_rare", chance = 2500000},
				{group = "krayt_dragon_common", chance = 3000000},
				{group = "weapons_all", chance = 4500000},
			},
			lootChance = 4500000,
		},
		{
			groups = {
				{group = "krayt_tissue_uncommon", chance = 1500000},
				{group = "krayt_dragon_common", chance = 1500000},
				{group = "weapons_all", chance = 3500000},
				{group = "armor_all", chance = 3500000},
			},
			lootChance = 4500000,
		},
		{
			groups = {
				{group = "krayt_tissue_uncommon", chance = 1500000},
				{group = "krayt_dragon_common", chance = 1500000},
				{group = "weapons_all", chance = 3500000},
				{group = "armor_all", chance = 3500000},
			},
			lootChance = 4500000,
		},
		{
			groups = {
				{group = "krayt_tissue_uncommon", chance = 1500000},
				{group = "krayt_dragon_common", chance = 1500000},
				{group = "weapons_all", chance = 2000000},
				{group = "armor_all", chance = 2000000},
				{group = "armor_attachments", chance = 2000000},  -- 9% chance
				{group = "vehicle_house_group", chance = 1000000},    -- 45% * 10% = 4.5%
			},
			lootChance = 4500000,
		},
		-- Pearl groups
		{
	        groups = {
				{group = "krayt_pearls", chance = 10000000},
			},
			lootChance = 2500000   -- 25%
		},
		{
	        groups = {
				{group = "krayt_pearls", chance = 10000000},
			},
			lootChance = 1500000  -- 15%
		},
		{
	        groups = {
				{group = "krayt_pearls", chance = 10000000},
			},
			lootChance = 1000000   -- 10%
		},
	},
	weapons = {},
	conversationTemplate = "",
	attacks = {
		{"creatureareacombo","stateAccuracyBonus=100"},
		{"creatureareaknockdown","stateAccuracyBonus=100"}
	}
}

CreatureTemplates:addCreatureTemplate(krayt_dragon_queen, "krayt_dragon_queen")
