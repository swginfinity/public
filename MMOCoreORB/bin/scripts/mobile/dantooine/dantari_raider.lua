dantari_raider = Creature:new {
	objectName = "@mob/creature_names:dantari_raider",
	randomNameType = NAME_GENERIC,
	randomNameTag = true,
	socialGroup = "dantari_raiders",
	faction = "dantari_raiders",
	level = 25,
	chanceHit = 0.36,
	damageMin = 240,
	damageMax = 250,
	baseXp = 2637,
	baseHAM = 7200,
	baseHAMmax = 8800,
	armor = 0,
	resists = {25,15,15,15,-1,15,-1,15,-1},
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

CreatureTemplates:addCreatureTemplate(dantari_raider, "dantari_raider")
