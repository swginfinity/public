fed_dub_investigator = Creature:new {
	objectName = "@mob/creature_names:fed_dub_investigator",
	randomNameType = NAME_GENERIC,
	randomNameTag = true,
	socialGroup = "fed_dub",
	faction = "fed_dub",
	mobType = MOB_NPC,
	level = 74,
	chanceHit = 1.2,
	damageMin = 350,
	damageMax = 740,
	baseXp = 7117,
	baseHAM = 11000,
	baseHAMmax = 14000,
	armor = 1,
	resists = {45,50,25,-1,-1,55,25,37.5,-1},
	meatType = "",
	meatAmount = 0,
	hideType = "",
	hideAmount = 0,
	boneType = "",
	boneAmount = 0,
	milk = 2 * 0,
	tamingChance = 0,
	ferocity = 0,
	pvpBitmask = ATTACKABLE,
	creatureBitmask = PACK + HERD,
	optionsBitmask = AIENABLED,
	diet = HERBIVORE,

	templates = {
		"object/mobile/dressed_fed_dub_investigator_twk_male_01.iff",
		"object/mobile/dressed_fed_dub_investigator_twk_female_01.iff"},
	lootGroups = {
		{
			groups = {
				{group = "junk", chance = 3000000},
				{group = "wearables_common", chance = 2000000},
				{group = "heavy_weapons", chance = 2000000},
				{group = "tailor_components", chance = 1500000},
				{group = "loot_kit_parts", chance = 1500000}
			}
		}
	},
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "imperial_weapons_heavy",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = merge(brawlermid,marksmanmid,riflemanmaster),
	secondaryAttacks = { },
	reactionStf = "@npc_reaction/townperson",
}

CreatureTemplates:addCreatureTemplate(fed_dub_investigator, "fed_dub_investigator")
