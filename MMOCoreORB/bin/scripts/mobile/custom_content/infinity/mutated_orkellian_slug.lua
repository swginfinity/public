mutated_orkellian_slug = Creature:new {
    customName = "a Mutated Orkellian Slug",
	socialGroup = "kun",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 230,
	chanceHit = 2.75,
	damageMin = 790,
	damageMax = 1150,
	baseXp = 3921,
	baseHAM = 80300,
	baseHAMmax = 123012,
	armor = 1,
	-- {kinetic,energy,blast,heat,cold,electricity,acid,stun,ls}
	resists = {65,80,55,25,70,70,20,35,15},
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
	scale = 1.3,

	templates = {
		"object/mobile/col_forage_carnivorous_worm.iff"},

	lootGroups = {
		{
			groups = {
				{group = "color_crystals", chance = 3500000},
				{group = "power_crystals", chance = 3500000},
				{group = "holocron_dark", chance = 250000},
				{group = "orkellian_slug_blood", chance = 250000},
				{group = "clothing_attachments", chance = 2500000}
			},
			lootChance = 8500000
		},
		{
			groups = {
				{group = "color_crystals", chance = 3500000},
				{group = "power_crystals", chance = 3500000},
				{group = "orkellian_slug_blood", chance = 250000},
				{group = "holocron_light", chance = 250000},
				{group = "clothing_attachments", chance = 2500000}
			},
			lootChance = 5500000
		},
		{ -- Jedi Specific Loot Group
			groups =
			{
				{group = "jedi_clothing_attachments", chance = 4000000},
				{group = "named_crystals", chance = 6000000},
			},
			lootChance = 2000000,
		},
		{ -- Veteran Rewards
			groups =
			{
				{group = "veteran_rewards_90_days", chance = 6000000},
				{group = "veteran_rewards_180_days", chance = 3000000},
				{group = "veteran_rewards_360_days", chance = 100000},
				{group = "veteran_rewards_450_days", chance = 900000},
			},
			lootChance = 3500000,
		},
	},
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "force_sword",
	secondaryWeapon = "force_sword_ranged",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = { {"intimidationattack","intimidationChance=80"},{"strongpoison",""},{"strongdisease",""},{"forcethrow2",""},	},
	secondaryAttacks = { {"knockdownfire",""},{"forceweaken2",""},{"forcechoke",""}}
}
CreatureTemplates:addCreatureTemplate(mutated_orkellian_slug, "mutated_orkellian_slug")
