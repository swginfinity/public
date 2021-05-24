juvenile_canyon_krayt_dragon = Creature:new {
	objectName = "@mob/creature_names:juvenile_canyon_krayt",
	socialGroup = "krayt",
	faction = "",
	level = 125,
	chanceHit = 9.0,
	damageMin = 745,
	damageMax = 1200,
	baseXp = 11577,
	baseHAM = 54000,
	baseHAMmax = 64000,
	armor = 2,
	-- {kinetic,energy,blast,heat,cold,electricity,acid,stun,ls}
	resists = {55,55,15,15,55,15,15,15,-1},
	meatType = "meat_carnivore",
	meatAmount = 750,
	hideType = "hide_bristley",
	hideAmount = 500,
	boneType = "bone_mammal",
	boneAmount = 410,
	milk = 0,
	tamingChance = 0,
	ferocity = 20,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + KILLER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,
	templates = {"object/mobile/juvenile_canyon_krayt.iff"},
	scale = 0.5,
	lootGroups = {
		{
	        groups = {
				{group = "krayt_tissue_rare", chance = 2500000},
				{group = "krayt_dragon_common", chance = 3500000},
				{group = "armor_all", chance = 2000000},
				{group = "weapons_all", chance = 1250000},
				{group = "armor_attachments", chance = 750000},   -- 100% * 7.5% = 7.5%
			},
			lootChance = 10000000,
		},
		{
	        groups = {
				{group = "krayt_dragon_common", chance = 3000000},
				{group = "armor_all", chance = 3500000},
				{group = "weapons_all", chance = 3500000},
			},
			lootChance = 2500000,
		}
	},
	weapons = {},
	conversationTemplate = "",
	attacks = {
		{"posturedownattack",""},
		{"creatureareaattack",""}
	}
}

CreatureTemplates:addCreatureTemplate(juvenile_canyon_krayt_dragon, "juvenile_canyon_krayt_dragon")
