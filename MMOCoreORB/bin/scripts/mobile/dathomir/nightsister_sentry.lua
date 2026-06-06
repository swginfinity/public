nightsister_sentry = Creature:new {
	objectName = "@mob/creature_names:nightsister_sentry",
	randomNameType = NAME_GENERIC,
	randomNameTag = true,
	mobType = MOB_NPC,
	socialGroup = "nightsister",
	faction = "nightsister",
	level = 77,
	chanceHit = 1.36,
	damageMin = 240,
	damageMax = 250,
	baseXp = 2730,
	baseHAM = 17200,
	baseHAMmax = 18800,
	armor = 0,
	resists = {15,15,15,55,55,55,55,-1,-1},
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
	diet = HERBIVORE,

	templates = {"object/mobile/dressed_dathomir_nightsister_guardian.iff"},
	lootGroups = {
		{
			groups = {
				{group = "power_crystals", chance = 3500000},
				{group = "color_crystals", chance = 1500000},
				{group = "nightsister_common", chance = 1500000},
				{group = "clothing_attachments", chance = 1000000},
				{group = "melee_weapons", chance = 2500000},
			},
			lootChance = 3000000,
		},
		{
			groups = {
				
				{group = "rifles", chance = 2500000},
				{group = "pistols", chance = 2500000},
				{group = "carbines", chance = 2500000},
				{group = "wearables_common", chance = 2500000},
			},
			lootChance = 2500000,   -- 25% chance of this group
		},
	},

	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "force_sword",
	secondaryWeapon = "force_sword_ranged",
	conversationTemplate = "",

	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = merge(fencernovice,swordsmannovice,pikemanmid,brawlermaster,forcewielder),
	secondaryAttacks = forcewielder
}

CreatureTemplates:addCreatureTemplate(nightsister_sentry, "nightsister_sentry")
