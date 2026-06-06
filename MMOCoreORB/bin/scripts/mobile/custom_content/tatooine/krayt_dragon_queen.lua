krayt_dragon_queen = Creature:new {
	objectName = "@mob/creature_names:krayt_dragon_queen",
	socialGroup = "krayt",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 350,
	chanceHit = 65,
	damageMin = 3170,
	damageMax = 5550,
	baseXp = 28549,
	baseHAM = 710000,
	baseHAMmax = 1201000,
	armor = 3,
	-- {kinetic,energy,blast,heat,cold,electricity,acid,stun,ls}
	resists = {195,195,195,195,165,195,195,195,-1},
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
	scale = 2.5,
	lootGroups = {
		{
	        groups = {
				{group = "krayt_tissue_rare", chance = 2500000},
				{group = "krayt_dragon_common", chance = 3500000},
				{group = "armor_all", chance = 1500000},
				{group = "weapons_all", chance = 2500000},
			},
			lootChance = 10000000
		},
		{
			groups = {
				{group = "krayt_tissue_rare", chance = 2500000},
				{group = "krayt_dragon_common", chance = 3500000},
				{group = "weapons_all", chance = 4000000},
			},
			lootChance = 7500000,
		},
		{
			groups = {
				{group = "krayt_tissue_rare", chance = 2500000},
				{group = "krayt_dragon_common", chance = 3000000},
				{group = "weapons_all", chance = 4500000},
			},
			lootChance = 4500000,
		},
		{
			groups = {
				{group = "krayt_tissue_uncommon", chance = 1500000},
				{group = "krayt_dragon_common", chance = 1500000},
				{group = "weapons_all", chance = 3500000},
				{group = "armor_all", chance = 3500000},
			},
			lootChance = 4500000,
		},
		{
			groups = {
				{group = "krayt_tissue_uncommon", chance = 1500000},
				{group = "krayt_dragon_common", chance = 1500000},
				{group = "weapons_all", chance = 3500000},
				{group = "armor_all", chance = 3500000},
			},
			lootChance = 4500000,
		},
		{
			groups = {
				{group = "krayt_tissue_uncommon", chance = 1500000},
				{group = "krayt_dragon_common", chance = 1500000},
				{group = "weapons_all", chance = 2000000},
				{group = "armor_all", chance = 2000000},
				{group = "armor_attachments", chance = 2000000},  -- 9% chance
				{group = "vehicle_house_group", chance = 1000000},    -- 45% * 10% = 4.5%
			},
			lootChance = 4500000,
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
			lootChance = 1500000  -- 15%
		},
		{
	        groups = {
				{group = "krayt_pearls", chance = 10000000},
			},
			lootChance = 1000000   -- 10%
		},
	},
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "unarmed",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = {{"creatureareacombo","stateAccuracyBonus=100"},{"creatureareaknockdown","stateAccuracyBonus=100"}	},
	secondaryAttacks = { },
}

CreatureTemplates:addCreatureTemplate(krayt_dragon_queen, "krayt_dragon_queen")
