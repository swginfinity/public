krayt_dragon_ancient = Creature:new {
	objectName = "@mob/creature_names:krayt_dragon_ancient",
	socialGroup = "krayt",
	faction = "",
	level = 335,
	chanceHit = 25,
	damageMin = 2270,
	damageMax = 4250,
	baseXp = 28549,
	baseHAM = 410000,
	baseHAMmax = 501000,
	armor = 3,
	-- {kinetic,energy,blast,heat,cold,electricity,acid,stun,ls}
	resists = {65,65,65,65,65,65,65,65,-1},
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
	scale = 2.0,
	lootGroups = {
		{
	        groups = {
				{group = "krayt_tissue_rare", chance = 2500000},
				{group = "krayt_dragon_common", chance = 3500000},
				{group = "weapons_all", chance = 4000000},
			},
			lootChance = 10000000,
		},
		{
	        groups = {
				{group = "krayt_tissue_rare", chance = 2500000},
				{group = "krayt_dragon_common", chance = 3500000},
				{group = "weapons_all", chance = 4000000},
			},
			lootChance = 4500000,
		},
		{
	        groups = {
				{group = "krayt_dragon_common", chance = 4500000},
				{group = "weapons_all", chance = 5500000},
			},
			lootChance = 7500000,
		},
		{
	        groups = {
				{group = "krayt_dragon_common", chance = 4500000},
				{group = "weapons_all", chance = 4500000},
				{group = "armor_attachments", chance = 1000000},   -- 75% * 10% = 7.5%
			},
			lootChance = 7500000,   -- 75% chance for this group
		},
		-- Pearl groups
		{
	        groups = {
				{group = "krayt_pearls", chance = 10000000},
			},
			lootChance = 2500000   -- 25% chance for this group
		},
		{
	        groups = {
				{group = "krayt_pearls", chance = 10000000},
			},
			lootChance = 1000000    -- 10% chance for this group
		}
	},
	weapons = {},
	conversationTemplate = "",
	attacks = {
		{"creatureareacombo","stateAccuracyBonus=100"},
		{"creatureareaknockdown","stateAccuracyBonus=100"},
	}
}

CreatureTemplates:addCreatureTemplate(krayt_dragon_ancient, "krayt_dragon_ancient")
