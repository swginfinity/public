aakuan_champion = Creature:new {
	objectName = "@mob/creature_names:aakuan_champion",
	randomNameType = NAME_GENERIC,
	randomNameTag = true,
	socialGroup = "aakuans",
	faction = "aakuans",
	mobType = MOB_NPC,
	level = 80,
	chanceHit = 2.71,
	damageMin = 555,
	damageMax = 820,
	baseXp = 7761,
	baseHAM = 12000,
	baseHAMmax = 15000,
	armor = 1,
	resists = {45,35,40,45,45,25,15,45,-1},
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
		"object/mobile/dressed_aakuan_champion_zabrak_female_01.iff",
		"object/mobile/dressed_aakuan_champion_zabrak_male_01.iff"},
	lootGroups = {
		{
			groups = {
				{group = "junk",					chance = 26 * (100000)},
				{group = "data_storage_unit_parts", chance = 20 * (100000)},
				{group = "aakuan_common",			chance = 40 * (100000)},
				{group = "armor_attachments",		chance =  6 * (100000)},
				{group = "clothing_attachments",	chance =  8 * (100000)}
			},
			lootChance = 50 * (100000),
		}
	},
	primaryWeapon = "melee_weapons",
	secondaryWeapon = "none",
	primaryAttacks = merge(brawlermaster,swordsmanmaster),
	secondaryAttacks = { },

	reactionStf = "@npc_reaction/fancy",
}

CreatureTemplates:addCreatureTemplate(aakuan_champion, "aakuan_champion")

