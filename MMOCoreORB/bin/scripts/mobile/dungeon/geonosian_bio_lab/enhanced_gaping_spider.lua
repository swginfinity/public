enhanced_gaping_spider = Creature:new {
	objectName = "@mob/creature_names:geonosian_gaping_spider_fire",
	customName = "Fire Breathing Spider",
	socialGroup = "geonosian_creature",
	mobType = MOB_CARNIVORE,
	faction = "",
	level = 300,
	chanceHit = 25.5,
	damageMin = 2115,
	damageMax = 2540,
	baseXp = 10267,
	baseHAM = 250000,
	baseHAMmax = 329000,
	armor = 2,
	resists = {175,175,135,195,145,155,145,165,140},
	meatType = "meat_insect",
	meatAmount = 50,
	hideType = "",
	hideAmount = 0,
	boneType = "",
	boneAmount = 0,
	milk = 0,
	tamingChance = 0,
	ferocity = 0,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + KILLER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/gaping_spider.iff"},
	scale = 3.0,
	lootGroups = {
		{
			groups =
			{
				{group = "fire_breathing_spider", chance = 10000000},
			},
			lootChance = 7500000,
		},
		{
			groups =
			{
				{group = "fire_breathing_spider", chance = 10000000},
			},
			lootChance = 5000000,
		},
		{
			groups =
			{
				{group = "fire_breathing_spider", chance = 5500000},
				{group = "geonosian_epic", chance = 4500000},
			},
			lootChance = 5500000,
		},
		{
			groups =
			{
				{group = "armor_attachments", chance = 10000000}, -- 25%
			},
			lootChance = 2500000,
		},

		{
			groups =
			{
				{group = "fire_breathing_spider", chance = 5500000},
				{group = "geonosian_epic", chance = 4500000},
			},
			lootChance = 4500000,
		},
		{
	        groups =
			{
				{group = "armor_attachments", chance = 500000},
				{group = "geonosian_relic", chance = 9000000},
				{group = "mastery_geocave_jewelry", chance = 500000},
			},
			lootChance = 7500000,
		},
		{
			groups =
			{
				{group = "pistols", chance = 2500000},
				{group = "rifles", chance = 2500000},
				{group = "carbines", chance = 2500000},
				{group = "melee_weapons", chance = 2500000},
			},
			lootChance = 4000000,
		},
		{
			groups =
			{
				{group = "pistols", chance = 2500000},
				{group = "rifles", chance = 2500000},
				{group = "carbines", chance = 2500000},
				{group = "melee_weapons", chance = 2500000},
			},
			lootChance = 4000000,
		},
	},

	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "object/weapon/ranged/creature/creature_spit_heavy_flame.iff",
	secondaryWeapon = "unarmed",
	conversationTemplate = "",

	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = { {"strongpoison",""}, {"stunattack",""} },
	secondaryAttacks = { {"strongpoison",""}, {"stunattack",""} }
}

CreatureTemplates:addCreatureTemplate(enhanced_gaping_spider, "enhanced_gaping_spider")
