lost_aqualish_infiltrator = Creature:new {
	objectName = "@mob/creature_names:lost_aqualish_infiltrator",
	randomNameType = NAME_GENERIC,
	randomNameTag = true,
	socialGroup = "lost_aqualish",
	faction = "lost_aqualish",
	mobType = MOB_NPC,
	level = 94,
	chanceHit = 1.5,
	damageMin = 480,	
	damageMax = 810,
	baseXp = 7432,
	baseHAM = 27500,
	baseHAMmax = 33000,
	armor = 2,
	resists = {25,60,30,40,35,55,45,50,-1},
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
	creatureBitmask = PACK + HERD + STALKER,
	optionsBitmask = AIENABLED,
	diet = HERBIVORE,

	templates = {
		"object/mobile/dressed_lost_aqualish_infiltrator_female_01.iff",
		"object/mobile/dressed_lost_aqualish_infiltrator_male_01.iff"},
	lootGroups = {
		{
			groups = {
				{group = "junk", chance = 3000000},
				{group = "wearables_common", chance = 2000000},
				{group = "melee_weapons", chance = 2000000},
				{group = "tailor_components", chance = 1500000},
				{group = "loot_kit_parts", chance = 1500000}
			}
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
	primaryWeapon = "unarmed_weapons",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = merge(brawlermaster,tkamaster),
	secondaryAttacks = { },
	reactionStf = "@npc_reaction/military",
}

CreatureTemplates:addCreatureTemplate(lost_aqualish_infiltrator, "lost_aqualish_infiltrator")
