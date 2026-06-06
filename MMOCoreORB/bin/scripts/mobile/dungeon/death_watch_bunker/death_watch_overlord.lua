death_watch_overlord = Creature:new {
	objectName = "",
	customName = "Drartih Oure (a Death Watch Overlord)",
	socialGroup = "death_watch",
	mobType = MOB_NPC,
	faction = "",
	level = 315,
	chanceHit = 1,
	damageMin = 1645,
	damageMax = 2500,
	specialPercentage = 60,
	baseXp = 20948,
	baseHAM = 350000,
	baseHAMmax = 550000,
	armor = 3,
	resists = {80,80,90,80,45,45,80,70,25},
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
	creatureBitmask = KILLER,
	optionsBitmask = AIENABLED,
	diet = HERBIVORE,
	scale = 1.15,

	templates = {"object/mobile/dressed_death_watch_gold.iff"},
	lootGroups = {
		{
			groups = {
				{group = "death_watch_bunker_overlord_shared", chance =  10000000}
			},
			lootChance = 10000000
		},
		{
			groups = {
				{group = "death_watch_bunker_overlord_shared", chance =  500000},
				{group = "death_watch_bunker_overlord_quest", chance  = 9500000}
			},
			lootChance = 5000000
		},
		{
			groups = {
				{group = "clothing_attachments", chance = 10000000},
			},
			lootChance = 500000
		},
		{
			groups = {
				{group = "armor_attachments", chance = 10000000},
			},
			lootChance = 2000000
		},
		{
			groups = {
				{group = "death_watch_bunker_lootbox", chance = 10000000},
			},
			lootChance = 3500000
		},
		{
			groups =
			{
				{group = "power_crystals", chance = 10000000},
			},
			lootChance = 3000000,
		},
		{--- Musty Bunker Group
			groups = {
				{group = "musty_house_group", chance =  10000000},    -- 15% * 100% = 15%
			},
			lootChance = 1500000
		},
		{--- Mandalorian Armor Segment Group
			groups = {
				{group = "armor_segment_mandalorian", chance =  10000000},    -- 10 * 100% = 10
			},
			lootChance = 1000000
		}
	},

	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "dark_trooper_weapons",
	secondaryWeapon = "none",
	conversationTemplate = "",
	thrownWeapon = "thrown_weapons",

	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = merge(riflemanmaster,fencermaster,marksmanmaster,brawlermaster),
	secondaryAttacks = { }
}

CreatureTemplates:addCreatureTemplate(death_watch_overlord, "death_watch_overlord")
