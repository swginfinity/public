gorax_grinch = Creature:new {
    customName = "Grinch",
	socialGroup = "townsperson",
	faction = "",
	mobType = MOB_NPC,
	level = 500,
	chanceHit = 2.75,
	damageMin = 1145,
	damageMax = 1766,
	baseXp = 3921,
	baseHAM = 3250000,
	baseHAMmax = 4250000,
	armor = 1,
	-- {kinetic,energy,blast,heat,cold,electricity,acid,stun,ls}
	resists = {25,25,25,25,25,25,25,25,50},
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
	creatureBitmask = PACK,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,
	scale = 0.7,

	templates = {"object/mobile/gorax_grinch.iff"},

	lootGroups = {
		{
			groups = {
				{group = "gorax_common", chance = 35 * 100000},
				{group = "power_crystals", chance = 35 * 100000},
				{group = "holocron_dark", chance = 2.5 * 100000},
				{group = "named_crystals", chance = 2.5 * 100000},
				{group = "clothing_attachments", chance = 25 * 100000}
			},
			lootChance = 100 * 100000
		},
    	{
      		groups = {
				{group = "landspeeder_ab1", chance = 20 * 100000},
				{group = "power_crystals", chance = 30 * 100000},
				{group = "named_crystals", chance = 10 * 100000},
				{group = "holocron_light", chance = 10 * 100000},
				{group = "clothing_attachments", chance = 30 * 100000}
			},
        	lootChance = 100 * 100000
        },
        {
        	groups = {
				{group = "mtg_paintings", chance = 35 * 100000},
				{group = "axkva_min", chance = 35 * 100000},
				{group = "minor_gorax", chance = 2.5 * 100000},
				{group = "holocron_light", chance = 2.5 * 100000},
				{group = "clothing_attachments", chance = 25 * 100000}
    		},
            lootChance = 100 * 100000
        },
		{ -- Jedi Specific Loot Group
			groups = {
				{group = "jedi_clothing_attachments", chance = 40 * 100000},
				{group = "named_crystals", chance = 60 * 100000},
			},
			lootChance = 85 * 100000,
		},
    	{ -- Veteran Rewards
      		groups = {
				{group = "veteran_rewards_90_days", chance = 60 * 100000},
				{group = "veteran_rewards_180_days", chance = 30 * 100000},
				{group = "veteran_rewards_360_days", chance = 1 * 100000},
				{group = "veteran_rewards_450_days", chance = 9 * 100000},
			},
			lootChance = 55 * 100000,
		},
		{ -- XJ-2 Airspeeder  / New Paintings
			groups = {
				{group = "landspeeder_organa", chance = 50 * 100000},
				{group = "mtg_paintings", chance = 50 * 100000},
      		},
      		lootChance = 40 * 100000,
    	},
		{ -- Xmas Crystals
			groups = {
				{group = "xmas_crystals", chance = 100 * 100000},
			},
			lootChance = 30 * 100000,
    	},
	},

	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "unarmed",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = { 	{"creatureareacombo","stateAccuracyBonus=100"},	{"creatureareaknockdown","stateAccuracyBonus=100"},	{"creatureareableeding",""}, },
	secondaryAttacks = { }
}
CreatureTemplates:addCreatureTemplate(gorax_grinch, "gorax_grinch")
