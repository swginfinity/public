jawa_greenred = Creature:new {
    customName = "A Festive Jawa",
	socialGroup = "townsperson",
	faction = "",
	mobType = MOB_NPC,
	level = 300,
	chanceHit = 2.75,
	damageMin = 790,
	damageMax = 1150,
	baseXp = 3921,
	baseHAM = 80300,
	baseHAMmax = 123012,
	armor = 1,
	-- {kinetic,energy,blast,heat,cold,electricity,acid,stun,ls}
	resists = {25,25,25,25,25,25,25,25,25},
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
	scale = 1.8,

	templates = {"object/mobile/jawa_greenred.iff"},

	lootGroups = {
		{
			groups = {
				{group = "mtg_paintings", chance = 30 * 100000},
				{group = "power_crystals", chance = 25 * 100000},
				{group = "holocron_dark", chance = 2.5 * 100000},
				{group = "named_crystals", chance = 17.5 * 100000},
				{group = "clothing_attachments", chance = 25 * 100000}
			},
			lootChance = 85 * 100000
		},
		{
			groups = {
				{group = "mtg_paintings", chance = 30 * 100000},
				{group = "power_crystals", chance = 25 * 100000},
				{group = "holocron_light", chance = 2.5 * 100000},
				{group = "named_crystals", chance = 17.5 * 100000},
				{group = "clothing_attachments", chance = 25 * 100000}
			},
			lootChance = 85 * 100000
		},
		{ -- Jedi Specific Loot Group
			groups = {
				{group = "jedi_clothing_attachments", chance = 40 * 100000},
				{group = "named_crystals", chance = 60 * 100000},
			},
			lootChance = 20 * 100000,
		},
		{ -- Veteran Rewards
			groups = {
				{group = "veteran_rewards_90_days", chance = 60 * 100000},
				{group = "veteran_rewards_180_days", chance = 30 * 100000},
				{group = "veteran_rewards_360_days", chance = 1 * 100000},
				{group = "veteran_rewards_450_days", chance = 9 * 100000},
			},
			lootChance = 35 * 100000,
		},
		{ -- XJ-2 Airspeeder
			groups = {
				{group = "landspeeder_organa", chance = 100 * 100000},
			},
			lootChance = 10 * 100000,
		},
		{ -- Xmas Crystals
			groups = {
				{group = "xmas_crystals", chance = 100 * 100000},
			},
			lootChance = 15 * 100000,
		},
	},
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "mixed_force_weapons",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = merge(fencermaster,swordsmanmaster,tkamid,brawlermaster,pikemanmaster,forcewielder),
	secondaryAttacks = { },
}
CreatureTemplates:addCreatureTemplate(jawa_greenred, "jawa_greenred")
