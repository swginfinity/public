acklay = Creature:new {
	objectName = "@mob/creature_names:geonosian_acklay_bunker_boss",
	customName = "Acklay",
	socialGroup = "geonosian_creature",
	mobType = MOB_CARNIVORE,
	faction = "",
	level = 500,
	chanceHit = 25.5,
	damageMin = 3800,
	damageMax = 4800,
	attackSpeed = 1.5,
	specialPercentage = 65,
	baseXp = 14884,
	baseHAM = 1216000,
	baseHAMmax = 1258000,
	armor = 2,
	resists = {40,45,55,55,45,45,40,40,25},
	meatType = "",
	meatAmount = 0,
	hideType = "",
	hideAmount = 0,
	boneType = "",
	boneAmount = 0,
	milk = 0,
	tamingChance = 0,
	ferocity = 25,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + KILLER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,
	tauntable = false,

	templates = {"object/mobile/acklay_hue.iff"},
	lootGroups = {
		{
	        groups =
			{
				{group = "armor_attachments", chance = 3500000},
				{group = "acklay", chance = 6500000}
			},
			lootChance = 4500000,
		},
		{
	        groups =
			{
				{group = "armor_attachments", chance = 3500000},
				{group = "acklay", chance = 6500000}
			},
			lootChance = 4500000,
		},
		{
	        groups =
			{
				{group = "armor_attachments", chance = 3500000},
				{group = "acklay", chance = 6500000}
			},
			lootChance = 4500000,
		},
		{
	        groups =
			{
				{group = "geonosian_hard", chance = 7500000},
				{group = "acklay", chance = 2500000}
			},
			lootChance = 3500000,
		},
		{ 	
			groups ={
					{group = "acklay", chance = 10000000},				
				},
				lootChance = 4500000,
		},
		{ 	
			groups ={
					{group = "jedi_clothing_attachments", chance = 10000000},				
				},
				lootChance = 1500000,
		},
		{
	        groups =
			{
				{group = "clothing_attachments", chance = 500000},
				{group = "acklay", chance = 9500000}
			},
			lootChance = 3500000,
		},
		{
	        groups =
			{
				{group = "geonosian_epic", chance = 10000000},
			},
			lootChance = 2500000,
		},
		{
	        groups =
			{
				{group = "geonosian_epic", chance = 10000000},
			},
			lootChance = 2500000,
		},
		{
			groups =
			{

				{group = "geonosian_common", chance = 10000000},
			},
			lootChance = 7500000,
		},
		{
			groups =
			{

				{group = "geonosian_common", chance = 10000000},
			},
			lootChance = 5500000,
		},
	},

	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "unarmed",
	secondaryWeapon = "none",
	conversationTemplate = "",

	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = { {"posturedownattack","stateAccuracyBonus=100"}, {"creatureareacombo","stateAccuracyBonus=100"}, {"creatureareableeding",""} },
	secondaryAttacks = { }
}

CreatureTemplates:addCreatureTemplate(acklay, "acklay")
