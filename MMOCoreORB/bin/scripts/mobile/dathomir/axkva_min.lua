axkva_min = Creature:new {
	objectName = "@mob/creature_names:axkva_min",
	socialGroup = "nightsister",
	faction = "nightsister",
	mobType = MOB_NPC,
	level = 352,
	chanceHit = 25,
	damageMin = 1845,
	damageMax = 3100,
	attackSpeed = 1.0,
	specialPercentage = 65,
	specialDamageMult = 7.5,
	baseXp = 28549,
	baseHAM = 495000,
	baseHAMmax = 521000,
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
	creatureBitmask = PACK + KILLER + HEALER,
	optionsBitmask = AIENABLED,
	healerType = "force",
	diet = HERBIVORE,

	templates = {"object/mobile/dressed_dathomir_nightsister_axkva.iff"},
	lootGroups = {
		{
			groups = {
				{group = "power_crystals", chance = 900000},
				{group = "nightsister_common", chance = 2600000},
				{group = "pistols", chance = 1000000},
				{group = "rifles", chance = 1000000},
				{group = "carbines", chance = 1000000},
				{group = "melee_weapons", chance = 2000000},
				{group = "armor_attachments", chance = 500000},
				{group = "clothing_attachments", chance = 500000},
				{group = "wearables_rare", chance = 500000}
			},
			lootChance = 100000000
		},
		{
			groups =
			{
				{group = "power_crystals", chance = 900000},
				{group = "nightsister_common", chance = 2600000},
				{group = "pistols", chance = 1000000},
				{group = "rifles", chance = 1000000},
				{group = "carbines", chance = 1000000},
				{group = "melee_weapons", chance = 2000000},
				{group = "armor_attachments", chance = 500000},
				{group = "clothing_attachments", chance = 500000},
				{group = "wearables_rare", chance = 500000}
			},
			lootChance = 7500000
		},
		{
			groups = {
				{group = "power_crystals", chance = 900000},
				{group = "nightsister_common", chance = 2600000},
				{group = "pistols", chance = 1000000},
				{group = "rifles", chance = 1000000},
				{group = "carbines", chance = 1000000},
				{group = "melee_weapons", chance = 2000000},
				{group = "armor_attachments", chance = 500000},
				{group = "clothing_attachments", chance = 500000},
				{group = "wearables_rare", chance = 500000}
			},
			lootChance = 5000000
		},
		{
			groups = {
				{group = "power_crystals", chance = 2000000},
				{group = "nightsister_common", chance = 2000000},
				{group = "clothing_attachments", chance = 2000000},
				{group = "wearables_rare", chance = 2000000},
				{group = "wearables_scarce", chance = 2000000},
			},
			lootChance = 5000000,
		},
		{ -- Nightsister Clothing Loot Group
			groups =
			{
				{group = "nightsister_clothing", chance = 10000000},   -- 25% * 100% = 25%
			},
			lootChance = 2500000,
		},
		{
			groups = {
				{group = "axkva_min", chance = 9500000},
				{group = "jedi_clothing_attachments", chance = 500000},
			},
			lootChance = 8500000
		},
		{ -- Jedi Specific Loot Group
			groups =
			{
				{group = "named_crystals", chance = 10000000},
			},
			lootChance = 2500000,
		},
		{ 	-- Jedi Specific Loot Group 2 - 40%
			groups ={
				{group = "jedi_clothing_attachments", chance = 4000000},	-- 40% * 40% = 16%
				{group = "ngerobes2", chance = 6000000}	                    -- 40% * 60% = 24%
		
			},
			lootChance = 4000000,
		},
	},

	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "force_sword",
	secondaryWeapon = "force_sword_ranged",
	conversationTemplate = "",

	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = merge(tkamaster,swordsmanmaster,fencermaster,pikemanmaster,forcepowermaster),
	secondaryAttacks = forcepowermaster
}

CreatureTemplates:addCreatureTemplate(axkva_min, "axkva_min")
