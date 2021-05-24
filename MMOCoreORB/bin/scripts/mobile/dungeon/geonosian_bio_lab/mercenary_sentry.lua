mercenary_sentry = Creature:new {
	objectName = "@mob/creature_names:geonosian_human_security_force",
	socialGroup = "geonosian",
	pvpFaction = "",
	faction = "",
	level = 252,
	chanceHit = 25.65,
	damageMin = 470,
	damageMax = 750,
	baseXp = 36380,
	baseHAM = 72000,
	baseHAMmax = 84000,
	armor = 1,
	resists = {40,40,35,35,35,35,35,15,-1},
	meatType = "",
	meatAmount = 0,
	hideType = "",
	hideAmount = 0,
	boneType = "",
	boneAmount = 0,
	milk = 0,
	tamingChance = 0,
	ferocity = 0,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + KILLER,
	optionsBitmask = AIENABLED,
	diet = HERBIVORE,

	templates = {
		"object/mobile/dressed_noble_human_male_03.iff"},
	lootGroups = {
		{
			groups = {
				{group = "geonosian_hard", chance = 1000000},
				{group = "geonosian_common", chance = 4500000},
				{group = "geonosian_relic", chance = 4500000}
			},
			lootChance = 250000,
		}
	},
	weapons = {"geonosian_mercenary_weapons"},
	conversationTemplate = "",
	attacks = merge(brawlermaster,marksmanmaster,pistoleermaster,riflemanmaster)
}

CreatureTemplates:addCreatureTemplate(mercenary_sentry, "mercenary_sentry")
