kiin_dray = Creature:new {
	objectName = "@mob/creature_names:gaping_spider_recluse_giant_kiin_dray",
	socialGroup = "spider_nightsister",
	faction = "spider_nightsister",
	mobType = MOB_CARNIVORE,
	level = 331,
	chanceHit = 15.75,
	damageMin = 2270,
	damageMax = 4250,
	baseXp = 11953,
	baseHAM = 321000,
	baseHAMmax = 392000,
	armor = 2,
	resists = {175,175,135,195,145,155,145,165,140},
	meatType = "meat_insect",
	meatAmount = 11,
	hideType = "",
	hideAmount = 0,
	boneType = "",
	boneAmount = 0,
	milk = 0,
	tamingChance = 0,
	ferocity = 8,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + KILLER + STALKER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/gaping_spider_recluse_giant_kiin_dray.iff"},
	scale = 4.0,
	lootGroups = {
		{
			groups = {
				{group = "power_crystals", chance = 2500000},
				{group = "nightsister_common", chance = 2500000},
				{group = "clothing_attachments", chance = 1500000},   -- 90% * 15% = 13.5%
				{group = "armor_attachments", chance = 1500000},      -- 90% * 15% = 13.5%
				{group = "melee_weapons", chance = 2000000},
			},
			lootChance = 9000000,  -- 90% chance for this group
		},
		{
			groups = {

				{group = "rifles", chance = 2500000},
				{group = "pistols", chance = 2500000},
				{group = "carbines", chance = 2500000},
				{group = "wearables_rare", chance = 1500000},
				{group = "wearables_scarce", chance = 1000000},
			},
			lootChance = 7500000,   -- 75% chance for this group
		},
		{
			groups = {
				{group = "power_crystals", chance = 2000000},
				{group = "clothing_attachments", chance = 2000000},   -- 50% * 20% = 10%
				{group = "armor_attachments", chance = 2000000},      -- 50% * 20% = 10%
				{group = "nightsister_common", chance = 3000000},
				{group = "large_generic_windowed_house_s02_group", chance = 1000000},    -- 50% * 10% = 5%
			},
			lootChance = 5000000,    -- 50% chance for this loot group
		},
		{
			groups = {
				{group = "fire_breathing_spider", chance = 10000000},  -- 100% * 10% = 10%
			},
			lootChance = 1000000,    -- 10% chance for this loot group
		},
	},

	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "object/weapon/ranged/creature/creature_spit_spray_red.iff",
	secondaryWeapon = "unarmed",
	conversationTemplate = "",

	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = { {"creatureareapoison",""}, {"strongpoison",""} },
	secondaryAttacks = { {"creatureareapoison",""}, {"strongpoison",""} }
}

CreatureTemplates:addCreatureTemplate(kiin_dray, "kiin_dray")
