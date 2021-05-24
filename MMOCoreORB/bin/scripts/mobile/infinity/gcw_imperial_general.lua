gcw_imperial_general = Creature:new {
	objectName = "@mob/creature_names:imperial_general",
	customName = "Malcor Brashin (an Imperial Grand General)",
	faction = "imperial",
	socialGroup = "imperial",
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
	optionsBitmask = AIENABLED + BOSS,
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
	weapons = {"ranged_weapons"},
	reactionStf = "@npc_reaction/military",
	attacks = merge(riflemanmaster,carbineermaster,brawlermaster)
}

CreatureTemplates:addCreatureTemplate(gcw_imperial_general, "gcw_imperial_general")
