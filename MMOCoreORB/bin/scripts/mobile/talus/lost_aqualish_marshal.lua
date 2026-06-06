lost_aqualish_marshal = Creature:new {
	objectName = "@mob/creature_names:lost_aqualish_marshal",
	randomNameType = NAME_GENERIC,
	randomNameTag = true,
	socialGroup = "lost_aqualish",
	faction = "lost_aqualish",
	mobType = MOB_NPC,
	level = 105,
	chanceHit = 1.5,
	damageMin = 660,	
	damageMax = 840,
	baseXp = 8300,
	baseHAM = 33250,
	baseHAMmax = 39900,
	armor = 2,
	resists = {50,20,30,35,50,85,40,65,-1},
	meatType = "",
	meatAmount = 0,
	hideType = "",
	hideAmount = 0,
	boneType = "",
	boneAmount = 0,
	milk = 2 * 0,
	tamingChance = 0,
	ferocity = 0,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + HERD + KILLER,
	optionsBitmask = AIENABLED,
	diet = HERBIVORE,

	templates = {
		"object/mobile/dressed_lost_aqualish_marshal_female_01.iff",
		"object/mobile/dressed_lost_aqualish_marshal_male_01.iff"},
	lootGroups = {
		{
			groups = {
				{group = "junk", chance = 2500000},
				{group = "wearables_common", chance = 2000000},
				{group = "heavy_weapons", chance = 2000000},
				{group = "tailor_components", chance = 1500000},
				{group = "loot_kit_parts", chance = 1500000},
				{group = "color_crystals", chance = 500000}
			}
		},
		{
			groups = {
				{group = "art_newhope1",       chance = 100 * (100000)},
			},
			lootChance = 8 * (100000)
		},
		{
			groups = {
				{group = "pelgo_blue", chance = 100 * (100000)},
			},
			lootChance = 3 * (100000)
		},
	},
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "dark_trooper_weapons",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = merge(brawlermaster,marksmanmaster,fencermaster,riflemanmaster),
	secondaryAttacks = { },
	reactionStf = "@npc_reaction/military",
}

CreatureTemplates:addCreatureTemplate(lost_aqualish_marshal, "lost_aqualish_marshal")
