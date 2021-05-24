krayt_dragon_adolescent = Creature:new {
	objectName = "@mob/creature_names:krayt_dragon_adolescent",
	socialGroup = "krayt",
	faction = "",
	level = 160,
	chanceHit = 10,
	damageMin = 1270,
	damageMax = 2250,
	baseXp = 28549,
	baseHAM = 110000,
	baseHAMmax = 201000,
	armor = 3,
	-- {kinetic,energy,blast,heat,cold,electricity,acid,stun,ls}
	resists = {55,55,15,15,55,15,15,15,-1},
	meatType = "meat_carnivore",
	meatAmount = 1000,
	hideType = "hide_bristley",
	hideAmount = 750,
	boneType = "bone_mammal",
	boneAmount = 675,
	milk = 0,
	tamingChance = 0,
	ferocity = 30,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + KILLER + STALKER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,
	templates = {"object/mobile/krayt_dragon_hue.iff"},
	scale = 0.7,
	lootGroups = {
		{
	        groups = {
				{group = "krayt_tissue_rare", chance = 2000000},
				{group = "krayt_dragon_common", chance = 3000000},
				{group = "armor_all", chance = 2500000},
				{group = "weapons_all", chance = 1750000},
				{group = "armor_attachments", chance = 750000},   -- 100% * 7.5% = 7.5%
			},
			lootChance = 10000000,  -- 100%
		},
		{
	        groups = {
				{group = "krayt_dragon_common", chance = 3000000},
				{group = "armor_all", chance = 3500000},
				{group = "weapons_all", chance = 3500000},
			},
			lootChance = 3500000,
		},
		-- Pearl groups
		{
	        groups = {
				{group = "krayt_pearls", chance = 10000000},
			},
			lootChance = 1500000 -- 15% chance for this group
		},
	},
	weapons = {},
	conversationTemplate = "",
	attacks = {
		{"creatureareacombo",""},
		{"creatureareaknockdown",""}
	}
}

CreatureTemplates:addCreatureTemplate(krayt_dragon_adolescent, "krayt_dragon_adolescent")
