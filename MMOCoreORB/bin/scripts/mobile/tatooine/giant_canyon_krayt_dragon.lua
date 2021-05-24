giant_canyon_krayt_dragon = Creature:new {
	objectName = "@mob/creature_names:giant_canyon_krayt_dragon",
	socialGroup = "krayt",
	faction = "",
	level = 220,
	chanceHit = 17.5,
	damageMin = 1645,
	damageMax = 3000,
	baseXp = 28549,
	baseHAM = 385000,
	baseHAMmax = 471000,
	armor = 3,
	-- {kinetic,energy,blast,heat,cold,electricity,acid,stun,ls}
	resists = {70,70,70,70,65,70,70,70,-1},
	meatType = "meat_carnivore",
	meatAmount = 1000,
	hideType = "hide_bristley",
	hideAmount = 870,
	boneType = "bone_mammal",
	boneAmount = 805,
	milk = 0,
	tamingChance = 0,
	ferocity = 20,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + KILLER + STALKER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,
	templates = {"object/mobile/canyon_krayt_dragon.iff"},
	scale = 1.5,
	lootGroups = {
		{
	        groups = {
				{group = "krayt_dragon_common", chance = 3000000},
				{group = "krayt_tissue_uncommon", chance = 3000000},
				{group = "weapons_all", chance = 4000000},
			},
			lootChance = 7500000
		},
		{
	        groups = {
				{group = "krayt_dragon_common", chance = 3500000},
				{group = "krayt_tissue_uncommon", chance = 2500000},
				{group = "weapons_all", chance = 2400000},
				{group = "armor_attachments", chance = 1600000},   -- 45% * 16% = 7.2%
			},
			lootChance = 4500000   -- 45% chance for  this group
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
			lootChance = 1000000   -- 10%
		}
	},

	weapons = {},
	conversationTemplate = "",
	attacks = {
		{"creatureareaattack","stateAccuracyBonus=50"},
		{"creatureareaknockdown","stateAccuracyBonus=50"}
	}
}

CreatureTemplates:addCreatureTemplate(giant_canyon_krayt_dragon, "giant_canyon_krayt_dragon")
