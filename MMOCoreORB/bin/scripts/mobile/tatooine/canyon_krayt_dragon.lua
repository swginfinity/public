canyon_krayt_dragon = Creature:new {
	objectName = "@mob/creature_names:canyon_krayt_dragon",
	socialGroup = "krayt",
	faction = "",
	level = 200,
	chanceHit = 27.25,
	damageMin = 1520,
	damageMax = 2750,
	baseXp = 26356,
	baseHAM = 321000,
	baseHAMmax = 392000,
	armor = 3,
	-- {kinetic,energy,blast,heat,cold,electricity,acid,stun,ls}
	resists = {160,160,160,160,120,160,160,160,-1},
	meatType = "meat_carnivore",
	meatAmount = 1000,
	hideType = "hide_bristley",
	hideAmount = 750,
	boneType = "bone_mammal",
	boneAmount = 675,
	milk = 0,
	tamingChance = 0,
	ferocity = 20,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + KILLER + STALKER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,
	templates = {"object/mobile/canyon_krayt_dragon.iff"},
	scale = 1.25,
	lootGroups = {
		{
	        groups = {
				{group = "krayt_dragon_common", chance = 3000000},
				{group = "krayt_tissue_uncommon", chance = 3500000},
				{group = "weapons_all", chance = 3500000},
			},
			lootChance = 10000000
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
		{"stunattack","stateAccuracyBonus=50"}
	}
}

CreatureTemplates:addCreatureTemplate(canyon_krayt_dragon, "canyon_krayt_dragon")
