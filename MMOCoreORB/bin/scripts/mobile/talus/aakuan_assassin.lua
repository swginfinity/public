aakuan_assassin = Creature:new {
	objectName = "",
	randomNameType = NAME_GENERIC,
	randomNameTag = true,
	customName = "an Aa'kuan Assassin",
	socialGroup = "aakuans",
	faction = "aakuans",
	mobType = MOB_NPC,
	level = 70,
	chanceHit = 1.63,
	damageMin = 450,
	damageMax = 960,
	baseXp = 6896,
	baseHAM = 12000,
	baseHAMmax = 15000,
	armor = 1,
	resists = {45,55,35,15,45,-1,-1,5,-1},
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
	creatureBitmask = PACK + KILLER,
	optionsBitmask = AIENABLED,
	diet = HERBIVORE,
	templates = {
		"object/mobile/dressed_aakuan_follower_trandoshan_female_01.iff",
		"object/mobile/dressed_aakuan_follower_trandoshan_male_01.iff"},
	lootGroups = {
		{
			groups = {
				{group = "junk",					chance = 23 * (100000)},
				{group = "wearables_all",			chance = 10 * (100000)},
				{group = "data_storage_unit_parts", chance = 16 * (100000)},
				{group = "aakuan_common",			chance = 40 * (100000)},
				{group = "armor_attachments",		chance =  5 * (100000)},
				{group = "clothing_attachments",	chance =  6 * (100000)}
			},
			lootChance = 50 * (100000),
		}
	},
	primaryWeapon = "melee_weapons",
	secondaryWeapon = "none",
	primaryAttacks = merge(brawlermaster,swordsmanmaster),
	secondaryAttacks = merge(brawlermaster,tkamaster),

	reactionStf = "@npc_reaction/fancy",
}

CreatureTemplates:addCreatureTemplate(aakuan_assassin, "aakuan_assassin")
