dark_jedi_knight = Creature:new {
	objectName = "@mob/creature_names:dark_jedi_knight",
	randomNameType = NAME_GENERIC,
	randomNameTag = true,
	mobType = MOB_NPC,
	socialGroup = "dark_jedi",
	faction = "",
	level = 265,
	chanceHit = 23.5,
	damageMin = 1645,
	damageMax = 3000,
	baseXp = 25266,
	baseHAM = 261000,
	baseHAMmax = 320000,
	armor = 3,
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

	templates = { "dark_jedi" },
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
				{group = "clothing_attachments", chance = 1000000},
				{group = "melee_weapons", chance = 2500000},
				{group = "dark_jedi_common", chance = 2500000},
			},
			lootChance = 5500000,
		},
		{
			groups = {
				{group = "clothing_attachments", chance = 1000000},
				{group = "rifles", chance = 2500000},
				{group = "pistols", chance = 2500000},
				{group = "carbines", chance = 2500000},
				{group = "dark_jedi_common", chance = 1500000},
			},
			lootChance = 4500000,
		},
		{
			groups = {
				{group = "clothing_attachments", chance = 1000000},
				{group = "rifles", chance = 2500000},
				{group = "pistols", chance = 2500000},
				{group = "carbines", chance = 2500000},
				{group = "dark_jedi_common", chance = 1500000},
			},
			lootChance = 2500000,
		},
		{ -- Jedi Specific Loot Group
			groups =
			{
				{group = "named_crystals", chance = 10000000},
			},
			lootChance = 1500000,
		},
		{ 	-- Jedi Specific Loot Group 2 - 15%
			groups ={
				{group = "jedi_clothing_attachments", chance = 5000000},	-- 15% * 50% = 7.5%
				{group = "ngerobes1", chance = 5000000}	                    -- 15% * 50% = 7.5%
		
			},
			lootChance = 1500000,
		},
	},

	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "dark_jedi_weapons",
	secondaryWeapon = "dark_jedi_weapons_ranged",
	conversationTemplate = "",

	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = merge(lightsabermaster,forcepowermaster),
	secondaryAttacks = forcepowermaster,
}

CreatureTemplates:addCreatureTemplate(dark_jedi_knight, "dark_jedi_knight")
