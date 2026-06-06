master_tamer_xerthan = Creature:new {
	objectName = "@mob/creature_names:patron_chiss_male",
	customName = "Master Tamer Xerthan (Laen's Protoge)",
	faction = "thug",
	socialGroup = "thug",
	mobType = MOB_NPC,
	level = 300,
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
	creatureBitmask = PACK + KILLER + NODIZZY,
	optionsBitmask = AIENABLED,
	diet = HERBIVORE,
	--Miniboss
	templates = {
			"object/mobile/shared_space_comm_quest_hortrix_steelarm_chiss_m.iff"},
			
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
	},
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "pirate_weapons_heavy",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = merge(riflemanmaster,pistoleermaster,carbineermaster,brawlermaster),
	secondaryAttacks = { },
	}
	
	CreatureTemplates:addCreatureTemplate(master_tamer_xerthan, "master_tamer_xerthan")
