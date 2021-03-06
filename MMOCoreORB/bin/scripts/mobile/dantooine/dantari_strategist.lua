dantari_strategist = Creature:new {
	objectName = "@mob/creature_names:dantari_raider_strategist",
	randomNameType = NAME_GENERIC,
	randomNameTag = true,
	socialGroup = "dantari_raiders",
	faction = "dantari_raiders",
	level = 29,
	chanceHit = 0.39,
	damageMin = 290,
	damageMax = 300,
	baseXp = 2914,
	baseHAM = 8400,
	baseHAMmax = 10200,
	armor = 0,
	resists = {10,20,40,-1,50,50,-1,20,-1},
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
		"object/mobile/dantari_male.iff",
		"object/mobile/dantari_female.iff"},
	lootGroups = {
		{
			groups = {
				{group = "junk",					chance = 60 * 100000},
				{group = "loot_kit_parts",			chance = 30 * 100000},
				{group = "armor_attachments",		chance =  5 * 100000},
				{group = "clothing_attachments",	chance =  5 * 100000},
			}
		},
		{
			groups = {
				{group = "dantari_common", chance = 100 * (100000)}
			},
			lootChance = 50 * (100000)
		},
	},
	weapons = {"primitive_weapons"},
	conversationTemplate = "",
	attacks = merge(pikemanmaster,fencermaster,brawlermaster)
}

CreatureTemplates:addCreatureTemplate(dantari_strategist, "dantari_strategist")
