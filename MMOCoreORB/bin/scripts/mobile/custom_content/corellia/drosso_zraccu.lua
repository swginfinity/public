drosso_zraccu = Creature:new {
	objectName = "@mob/creature_names:patron_chiss_male",
	customName = "Drosso Zraccu (a Hired SpyNet Operative)",
	faction = "thug",
	socialGroup = "thug",
	mobType = MOB_NPC,
	level = 330,
	chanceHit = 12.5,
	damageMin = 2000,
	damageMax = 3050,
	attackSpeed = 1.0,
	specialPercentage = 65,
	baseXp = 20000,
	baseHAM = 750000,
	baseHAMmax = 1250000,
	armor = 2,
	-- {kinetic, energy, electric, stun, blast, heat, cold, acid, ls}
	resists = {60,60,35,50,35,50,25,50,15},
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
	creatureBitmask = PACK + KILLER + NOKNOCKDOWN + NODIZZY,
	optionsBitmask = AIENABLED,
	diet = HERBIVORE,
	--Miniboss
	templates = {"object/mobile/dressed_hutt_informant_quest.iff"},

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
				{group = "power_crystals", chance = 2500000},
				{group = "clothing_attachments", chance = 2500000},
				{group = "armor_attachments", chance = 2500000},
				{group = "named_crystals", chance = 2500000},
			},
			lootChance = 9000000,  -- 90% chance for this group
		},
		{
				groups = {
				{group = "power_crystals", chance =10000000},
				},
			lootChance = 5000000, -- 50% chance for 3rd crystal.
		},
				{
				groups = {
				{group = "nge_house_group", chance =10000000},
				},
			lootChance = 1000000, -- 10% chance for 3rd nge houses.
		},
		{
			groups =
			{
				{group = "slicing_station_group", chance = 100 * (100000)}
			},
			lootChance = 50 * (100000)
		},
	},
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "one_hand",
	secondaryWeapon = "pirate_rifle",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = merge(fencermaster,brawlermaster),
	secondaryAttacks = merge(riflemanmaster,marksmanmaster),
	reactionStf = "@npc_reaction/fancy",

	}

	CreatureTemplates:addCreatureTemplate(drosso_zraccu, "drosso_zraccu")
