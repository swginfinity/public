dark_jedi_master = Creature:new {
	objectName = "@mob/creature_names:dark_jedi_master",
	randomNameType = NAME_GENERIC,
	randomNameTag = true,
	socialGroup = "dark_jedi",
	faction = "",
	level = 331, -- Should drop Flawless
	chanceHit = 19.25,
	damageMin = 1800,
	damageMax = 3310,
	baseXp = 27849,
	baseHAM = 321000,
	baseHAMmax = 392000,
	armor = 3,
	-- {kinetic,energy,blast,heat,cold,electricity,acid,stun,ls}
	resists = {75,75,75,75,75,75,75,75,-1},
	meatType = "",
	meatAmount = 0,
	hideType = "",
	hideAmount = 0,
	boneType = "",
	boneAmount = 0,
	milk = 0,
	tamingChance = 0,
	ferocity = 0,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = KILLER + STALKER,
	optionsBitmask = AIENABLED,
	diet = HERBIVORE,

	templates = {
		"object/mobile/dressed_dark_jedi_human_male_01.iff",
		"object/mobile/dressed_dark_jedi_human_female_01.iff"},
	lootGroups = {
		{
			groups = { -- DarkJedi LootGroup
				{group = "holocron_dark", chance = 4000000},
				{group = "dark_jedi_common", chance = 6000000},
			},
			lootChance = 7500000,
		},
		{
			groups = {
				{group = "power_crystals", chance = 1500000},
				{group = "color_crystals", chance = 2500000},
				{group = "clothing_attachments", chance = 1500000},
				{group = "melee_weapons", chance = 2500000},
				{group = "dark_jedi_common", chance = 2000000},
			},
			lootChance = 7500000,
		},
		{
			groups = {
				{group = "clothing_attachments", chance = 1500000},
				{group = "rifles", chance = 2500000},
				{group = "pistols", chance = 2500000},
				{group = "carbines", chance = 2500000},
				{group = "dark_jedi_common", chance = 1000000},
			},
			lootChance = 7500000,
		},
		{
			groups = {
				{group = "clothing_attachments", chance = 1500000},
				{group = "rifles", chance = 2500000},
				{group = "pistols", chance = 2500000},
				{group = "carbines", chance = 2500000},
				{group = "dark_jedi_common", chance = 1000000},
			},
			lootChance = 4500000,
		},
		{ -- Jedi Specific Loot Group
			groups =
			{
				{group = "named_crystals", chance = 10000000},
			},
			lootChance = 2500000,
		},
		{ 	-- Jedi Specific Loot Group 2 - 30%
			groups ={
				{group = "jedi_clothing_attachments", chance = 2500000},	-- 40% * 25% = 10%
				{group = "ngerobes2", chance = 7500000}	                    -- 40% * 70% = 30%

			},
			lootChance = 4000000,
		},
		{
			groups = {
				{group = "art_forcewithyou1", chance = 10000000}
			},
			lootChance = 300000
		} 
	},
	weapons = {"dark_jedi_weapons_gen4"},
	conversationTemplate = "",
	attacks = merge(lightsabermaster,forcepowermaster)
}

CreatureTemplates:addCreatureTemplate(dark_jedi_master, "dark_jedi_master")
