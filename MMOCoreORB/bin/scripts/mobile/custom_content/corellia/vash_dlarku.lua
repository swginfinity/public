vash_dlarku = Creature:new {
	objectName = "@mob/creature_names:patron_chiss_male",
	customName = "Vash Dlarku (Chiss Poacher Clan Pilot)",
	faction = "thug",
	socialGroup = "thug",
	mobType = MOB_NPC,
	level = 330,
	chanceHit = 12.5,
	damageMin = 1700,
	damageMax = 2650,
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
	creatureBitmask = PACK + KILLER + NODIZZY + NOBLIND,
	optionsBitmask = AIENABLED,
	diet = NONE,
	--Miniboss
	templates = {"object/mobile/dressed_tatooine_jabba_henchman.iff"},

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
	primaryWeapon = "st_bombardier_weapons",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = { 	{"intimidationattack","stateAccuracyBonus=100"}, {"flamecone2","stateAccuracyBonus=65"}, {"flamesingle2","accuracyBonus=45"}, },
	secondaryAttacks = { },
	reactionStf = "@npc_reaction/fancy",

	}

	CreatureTemplates:addCreatureTemplate(vash_dlarku, "vash_dlarku")
