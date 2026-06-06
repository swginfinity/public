treasure_jawa = Creature:new {
	customName = "a greedy Jawa",
	socialGroup = "self",
	mobType = MOB_NPC,
	faction = "",
	level = 2300,
	chanceHit = 1.98,
	damageMin = 1310,
	damageMax = 1610,
	baseXp = 50366,
	baseHAM = 102245,
	baseHAMmax = 195625,
	armor = 2,
	resists = {34,34,34,50,20,20,20,-1,-1},
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
	creatureBitmask = PACK + STALKER,
	optionsBitmask = AIENABLED,
	diet = HERBIVORE,

	templates = {"object/mobile/jawa_male.iff"},
	lootGroups = {
		{
			groups = {
				{group = "armor_attachments", chance = 1000000},
				{group = "jawa_common", chance = 2000000},
				{group = "armor_all", chance = 2000000},
				{group = "loot_kit_parts", chance = 2500000},
				{group = "color_crystals", chance = 2500000},
			},
				groups = {
				{group = "clothing_attachments", chance = 5000000},
				{group = "janta_common", chance = 2000000},
				{group = "weapons_all", chance = 3000000},
			},
				groups = {
				{group = "armor_all", chance = 2500000},
				{group = "armor_attachments", chance = 5000000},
				{group = "weapons_all", chance = 2500000},
			}
		}
	},
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "jawa_warlord_weapons",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = merge(marksmannovice,brawlernovice),
	secondaryAttacks = { }
}

CreatureTemplates:addCreatureTemplate(treasure_jawa, "treasure_jawa")
