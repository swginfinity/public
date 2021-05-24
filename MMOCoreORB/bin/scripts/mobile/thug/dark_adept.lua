dark_adept = Creature:new {
	objectName = "@mob/creature_names:dark_adept",
	randomNameType = NAME_GENERIC,
	randomNameTag = true,
	socialGroup = "dark_jedi",
	faction = "",
	level = 140,
	chanceHit = 4.75,
	damageMin = 1145,
	damageMax = 1700,
	baseXp = 13178,
	baseHAM = 89000,
	baseHAMmax = 122432,
	armor = 2,
	-- {kinetic,energy,blast,heat,cold,electricity,acid,stun,ls}
	resists = {80,80,80,80,80,80,80,80,-1},
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
	creatureBitmask = KILLER + STALKER,
	optionsBitmask = AIENABLED,
	diet = HERBIVORE,

	templates = {"object/mobile/dressed_dark_jedi_human_male_01.iff"},
	lootGroups = {
		{
			groups = { -- DarkJedi LootGroup
				{group = "holocron_dark", chance = 4000000},
				{group = "dark_jedi_common", chance = 6000000},
			},
			lootChance = 5500000,
		},
		{
			groups = {
				{group = "power_crystals", chance = 1500000},
				{group = "color_crystals", chance = 2500000},
				{group = "clothing_attachments", chance = 1500000},
				{group = "melee_weapons", chance = 2500000},
				{group = "dark_jedi_common", chance = 2000000},
			},
			lootChance = 3500000,
		},
		{
			groups = {
				{group = "clothing_attachments", chance = 1500000},
				{group = "rifles", chance = 2500000},
				{group = "pistols", chance = 2500000},
				{group = "carbines", chance = 2500000},
				{group = "dark_jedi_common", chance = 1000000},
			},
			lootChance = 2500000,
		},
		{
			groups = {
				{group = "clothing_attachments", chance = 1500000},
				{group = "rifles", chance = 2500000},
				{group = "pistols", chance = 2500000},
				{group = "carbines", chance = 2500000},
				{group = "dark_jedi_common", chance = 1000000},
			},
			lootChance = 1500000,
		},
		{ -- Jedi Specific Loot Group
			groups = 
			{	
				{group = "jedi_clothing_attachments", chance = 5000000},
				{group = "named_crystals", chance = 5000000},
			},
			lootChance = 500000,
		},
	},
	weapons = {"dark_jedi_weapons_gen2"},
	conversationTemplate = "",
	attacks = merge(lightsabermaster,forcewielder)
}

CreatureTemplates:addCreatureTemplate(dark_adept, "dark_adept")
