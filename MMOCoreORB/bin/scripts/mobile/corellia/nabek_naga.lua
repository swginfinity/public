nabek_naga = Creature:new {
	objectName = "@mob/creature_names:patron_chiss_female",
	customName = "Nabek Naga (a Dark Jedi Enforcer)",
	faction = "thug",
	socialGroup = "thug",
	level = 355, 
	chanceHit = 20,
	damageMin = 2100,
	damageMax = 3000,
	baseXp = 26654,
	baseHAM = 821000,
	baseHAMmax = 1192000,
	armor = 2,
	-- {kinetic,energy,blast,heat,cold,electricity,acid,stun,ls}
	resists = {75,45,55,80,80,70,55,70,20},
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
	creatureBitmask = PACK + KILLER + HEALER,
	optionsBitmask = AIENABLED + BOSS,
	diet = HERBIVORE,

	templates = {"object/mobile/ep3/shared_ep3_etyyy_kara_corlon.iff"},
	lootGroups = {
		{
			groups = {
				{group = "power_crystals", chance = 2500000},
				{group = "color_crystals", chance = 2500000},
				{group = "armor_attachments", chance = 2500000},
				{group = "clothing_attachments", chance = 2500000},  

			},
			lootChance = 9000000,  -- 90% chance for this group
		},
		{
			groups = {
				{group = "power_crystals", chance = 3300000},
				{group = "clothing_attachments", chance = 3300000},   
				{group = "armor_attachments", chance = 3300000},
				{group = "wearables_scarce", chance = 100000},
			},
			lootChance = 6000000,  -- 60% chance for this group	
		},
		{
			groups = {
				{group = "power_crystals", chance =10000000},
				},
			lootChance = 5000000, -- 50% chance for 3rd crystal.
		},
		{ -- Chiss backpack
			groups = {
				{group = "chiss_poacher_backpack_custom", chance = 10000000},
			},
			lootChance = 1000000,    -- 10% chance for drop
		},
		{ -- Jedi Specific Loot Group
			groups =
			{
				{group = "named_crystals", chance = 10000000},
			},
			lootChance = 10000000, -- Guarunteed Drop
		},
		{ -- Jedi Specific Loot Group
			groups =
			{
				{group = "jedi_clothing_attachments", chance = 10000000}, -- 10% * 100% = 10%
			},
			lootChance = 1000000,  -- 10% chance for this group
		},
		{ -- New NGE Robes
			groups =
			{
				{group = "ngerobes1", chance = 6500000}, -- 100% * 65% = 65%
				{group = "ngerobes2", chance = 3500000}, -- 100% * 35% = 35%
			},
			lootChance = 10000000,  -- 100% chance for this group
		},
		{ -- New NGE Robes
			groups =
			{
				{group = "ngerobes1", chance = 6500000}, -- 5% * 65% = 3.25%
				{group = "ngerobes2", chance = 3500000}, -- 5% * 35% = 1.75%
			},
			lootChance = 500000,  -- 5% chance for this group
		},
	},
	weapons = {"dark_jedi_weapons_gen3"},
	reactionStf = "@npc_reaction/fancy",
	attacks = merge(lightsabermaster,forcepowermaster)
}

CreatureTemplates:addCreatureTemplate(nabek_naga, "nabek_naga")
