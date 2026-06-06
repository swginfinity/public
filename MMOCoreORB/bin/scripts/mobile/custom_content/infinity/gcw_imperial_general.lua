gcw_imperial_general = Creature:new {
	objectName = "@mob/creature_names:imperial_general",
	customName = "Malcor Brashin (an Imperial Grand General)",
	faction = "imperial",
	socialGroup = "imperial",
	mobType = MOB_NPC,
	level = 400,
	chanceHit = 4,
	damageMin = 3200,
	damageMax = 3800,
	specialDamageMult = 5.5,
	baseXp = 26654,
	baseHAM = 864232,
	baseHAMmax = 932546,
	armor = 3,
	-- {kinetic,energy,blast,heat,cold,electricity,acid,stun,ls}
	resists = {65,55,55,75,75,70,55,70,25},
	meatType = "",
	meatAmount = 0,
	hideType = "",
	hideAmount = 0,
	boneType = "",
	boneAmount = 0,
	milk = 0,
	tamingChance = 0,
	ferocity = 0,
	pvpBitmask = ATTACKABLE,
	creatureBitmask = PACK + KILLER + HEALER,
	optionsBitmask = AIENABLED,
	diet = HERBIVORE,

	templates = {"object/mobile/dressed_imperial_inquisitor_human_male_02.iff"},
	lootGroups = {
		{
			groups = {
				{group = "power_crystals", chance = 2500000},
				{group = "lockedcontainer", chance = 2500000},
				{group = "armor_attachments", chance = 2500000},
				{group = "clothing_attachments", chance = 2500000},

			},
			lootChance = 10000000,  -- 100% chance for this group
		},
		{
			groups = {
				{group = "weapons_all", chance = 2000000},
				{group = "clothing_attachments", chance = 2000000},
				{group = "armor_attachments", chance = 2000000},
				{group = "lockedcontainer", chance = 4000000},
			},
			lootChance = 10000000,  -- 100% chance for this group
		},
		{
			groups = {
				{group = "weapons_all", chance = 3000000},
				{group = "clothing_attachments", chance = 3000000},
				{group = "armor_attachments", chance = 3000000},
				{group = "lockedcontainer", chance = 1000000},
			},
			lootChance = 10000000,  -- 100% chance for this group
		},
		{
			groups = {
				{group = "power_crystals", chance = 3000000},
				{group = "clothing_attachments", chance = 3000000},
				{group = "armor_attachments", chance = 3000000},
				{group = "lockedcontainer", chance = 1000000},
			},
			lootChance = 6000000,  -- 60% chance for this group
		},
		{
			groups = {
				{group = "power_crystals", chance = 10000000},
				},
			lootChance = 5000000, -- 50% chance for 3rd crystal.
		},
		{
			groups = {
				{group = "landspeeder_ab1", chance = 10000000},
				},
			lootChance = 10000000, -- 100% chance of mount
		},
	},
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "ranged_weapons",
	secondaryWeapon = "unarmed",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = merge(riflemanmaster,carbineermaster),
	secondaryAttacks = brawlermaster,
	reactionStf = "@npc_reaction/military",
}

CreatureTemplates:addCreatureTemplate(gcw_imperial_general, "gcw_imperial_general")
