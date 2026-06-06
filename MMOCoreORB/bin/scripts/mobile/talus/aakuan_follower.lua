aakuan_follower = Creature:new {
	objectName = "@mob/creature_names:aakuan_follower",
	randomNameType = NAME_GENERIC,
	randomNameTag = true,
	socialGroup = "aakuans",
	faction = "aakuans",
	mobType = MOB_NPC,
	level = 25,
	chanceHit = 1.28,
	damageMin = 322,
	damageMax = 472,
	baseXp = 2356,
	baseHAM = 5460,
	baseHAMmax = 7250,
	armor = 0,
	resists = {15,5,15,-1,-1,15,-1,-1,-1},
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
	creatureBitmask = PACK,
	optionsBitmask = AIENABLED,
	diet = HERBIVORE,
	templates = {
		"object/mobile/dressed_aakuan_follower_trandoshan_female_01.iff",
		"object/mobile/dressed_aakuan_follower_trandoshan_male_01.iff"},
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
	primaryAttacks = merge(brawlermid),
	secondaryAttacks = { },

	reactionStf = "@npc_reaction/fancy",
}

CreatureTemplates:addCreatureTemplate(aakuan_follower, "aakuan_follower")
