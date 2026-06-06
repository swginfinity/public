aakuan_warder = Creature:new {
	objectName = "@mob/creature_names:aakuan_warder",
	randomNameType = NAME_GENERIC,
	randomNameTag = true,
	socialGroup = "aakuans",
	faction = "aakuans",
	mobType = MOB_NPC,
	level = 62,
	chanceHit = 1.63,
	damageMin = 445,
	damageMax = 745,
	baseXp = 5876,
	baseHAM = 11000,
	baseHAMmax = 14000,
	armor = 0,
	resists = {35,20,45,35,30,25,15,15,-1},
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
		"object/mobile/dressed_aakuan_warder_zabrak_female_01.iff",
		"object/mobile/dressed_aakuan_warder_zabrak_male_01.iff"},
	lootGroups = {
		{
			groups = {
				{group = "junk", chance = 4000000},
				{group = "wearables_all", chance = 1000000},
				{group = "data_storage_unit_parts", chance = 1600000},
				{group = "aakuan_common", chance = 2300000},
				{group = "armor_attachments", chance = 500000},
				{group = "clothing_attachments", chance = 600000}
			},
			lootChance = 5000000,
		}
	},
	primaryWeapon = "melee_weapons",
	secondaryWeapon = "none",
	primaryAttacks = merge(brawlermaster,swordsmanmaster),
	secondaryAttacks = merge(brawlermaster,tkamaster),

	reactionStf = "@npc_reaction/fancy",
}

CreatureTemplates:addCreatureTemplate(aakuan_warder, "aakuan_warder")
