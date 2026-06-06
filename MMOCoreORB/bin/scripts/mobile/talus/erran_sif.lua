erran_sif = Creature:new {
	objectName = "@mob/creature_names:erran_sif",
	socialGroup = "sif",
	faction = "sif",
	mobType = MOB_NPC,
	level = 100,
	chanceHit = 3.25,
	damageMin = 550,
	damageMax = 950,
	baseXp = 9150,
	baseHAM = 26500,
	baseHAMmax = 30000,
	armor = 1,
	resists = {65,65,65,65,65,65,65,65,-1},
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
	creatureBitmask = PACK + KILLER + STALKER,
	optionsBitmask = AIENABLED,
	diet = HERBIVORE,

	templates = {"object/mobile/dressed_erran_sif.iff"},
	lootGroups = {
		{
			groups = {
				{group = "rifles",					chance = 10 * (100000)},
				{group = "clothing_attachments",	chance = 25 * (100000)},
				{group = "melee_weapons",			chance = 25 * (100000)},
				{group = "carbines",				chance = 15 * (100000)},
				{group = "armor_attachments",		chance = 25 * (100000)}
			},
			lootChance = 100 * (100000)
		},
		{
			groups = {
				{group = "rifles",					chance = 10 * (100000)},
				{group = "clothing_attachments",	chance = 25 * (100000)},
				{group = "melee_weapons",			chance = 25 * (100000)},
				{group = "carbines",				chance = 15 * (100000)},
				{group = "armor_attachments",		chance = 25 * (100000)}
			},
			lootChance = 100 * (100000)
		},
		{
			groups = {
				{group = "rifles",					chance = 10 * (100000)},
				{group = "clothing_attachments",	chance = 25 * (100000)},
				{group = "melee_weapons",			chance = 25 * (100000)},
				{group = "carbines",				chance = 15 * (100000)},
				{group = "armor_attachments",		chance = 25 * (100000)}
			},
			lootChance = 75 * (100000)
		},
		{
			groups = {
				{group = "captain_ring_group", chance = 10000000},
			},
			lootChance = 1 * (100000)
		}
	},
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "sif_weapons",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = merge(brawlermaster,fencermaster,swordsmanmaster),
	secondaryAttacks = { },
	reactionStf = "@npc_reaction/military",
}

CreatureTemplates:addCreatureTemplate(erran_sif, "erran_sif")
