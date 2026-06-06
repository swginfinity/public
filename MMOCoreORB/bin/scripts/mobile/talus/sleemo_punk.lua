sleemo_punk = Creature:new {
	objectName = "@mob/creature_names:sleemo_punk",
	randomNameType = NAME_GENERIC,
	randomNameTag = true,
	socialGroup = "sleemo_gang",
	faction = "",
	mobType = MOB_NPC,
	level = 22,
	chanceHit = 0.68,
	damageMin = 200,
	damageMax = 340,
	baseXp = 2168,
	baseHAM = 4900,
	baseHAMmax = 6100,
	armor = 0,
	resists = {5,-1,20,-1,-1,5,-1,5,-1},
	meatType = "",
	meatAmount = 0,
	hideType = "",
	hideAmount = 0,
	boneType = "",
	boneAmount = 0,
	milk = 2 * 0,
	tamingChance = 0.000000,
	ferocity = 0,
	pvpBitmask = ATTACKABLE,
	creatureBitmask = STALKER + HERD,
	optionsBitmask = AIENABLED,
	diet = HERBIVORE,

	templates = {"object/mobile/dressed_hooligan_rodian_male_01.iff"},
	lootGroups = {
		{
			groups = {
				{group = "junk", chance = 3200000},
				{group = "wearables_common", chance = 2000000},
				{group = "rifles", chance = 700000},
				{group = "melee_knife", chance = 700000},
				{group = "pistols", chance = 700000},
				{group = "carbines", chance = 700000},
				{group = "printer_parts", chance = 1000000},
				{group = "loot_kit_parts", chance = 1000000},
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
	primaryAttacks = merge(fencermaster,brawlermaster),
	secondaryAttacks = { },
	reactionStf = "@npc_reaction/slang",
}

CreatureTemplates:addCreatureTemplate(sleemo_punk, "sleemo_punk")
