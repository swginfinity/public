binayre_chief = Creature:new {
	objectName = "@mob/creature_names:binayre_chief",
	randomNameType = NAME_GENERIC,
	randomNameTag = true,
	socialGroup = "binayre",
	faction = "binayre",
	mobType = MOB_NPC,
	level = 89,
	chanceHit = 1.56,
	damageMin = 560,
	damageMax = 860,
	baseXp = 7981,
	baseHAM = 46000,
	baseHAMmax = 52000,
	armor = 1,
	resists = {55,65,45,15,15,60,45,35,-1},
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
	creatureBitmask = PACK,
	optionsBitmask = AIENABLED,
	diet = HERBIVORE,

	templates = {
		"object/mobile/dressed_binayre_chief_human_female_01.iff",
		"object/mobile/dressed_binayre_chief_human_male_01.iff"},
	lootGroups = {
		{
			groups = {
				{group = "junk", chance = 2000000},
				{group = "tailor_components", chance = 1500000},
				{group = "loot_kit_parts", chance = 1500000},
				{group = "binayre_common", chance = 4550000},
				{group = "color_crystals", chance = 450000}
			}
		}
	},
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "melee_weapons",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = merge(brawlermaster,fencermaster,pikemanmaster,swordsmanmaster),
	secondaryAttacks = { },
	reactionStf = "@npc_reaction/slang",

}

CreatureTemplates:addCreatureTemplate(binayre_chief, "binayre_chief")
