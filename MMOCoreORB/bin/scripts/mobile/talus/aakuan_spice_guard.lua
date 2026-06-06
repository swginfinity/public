aakuan_spice_guard = Creature:new {
	objectName = "",
	randomNameType = NAME_GENERIC,
	randomNameTag = true,
	customName = "an Aa'kuan Spice Guard",
	socialGroup = "aakuans",
	faction = "aakuans",
	mobType = MOB_NPC,
	level = 59,
	chanceHit = 1.58,
	damageMin = 435,
	damageMax = 650,
	baseXp = 5150,
	baseHAM = 11000,
	baseHAMmax = 14000,
	armor = 0,
	resists = {42.5,30,85,35,30,25,15,5,-1},
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
		"object/mobile/dressed_aakuan_defender_human_female_01.iff",
		"object/mobile/dressed_aakuan_defender_human_male_01.iff"},
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
	secondaryAttacks = { },

	reactionStf = "@npc_reaction/fancy",
}

CreatureTemplates:addCreatureTemplate(aakuan_spice_guard, "aakuan_spice_guard")
