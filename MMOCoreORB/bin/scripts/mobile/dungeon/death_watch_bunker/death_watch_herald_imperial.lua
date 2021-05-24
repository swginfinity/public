death_watch_herald_imperial = Creature:new {
	objectName = "",
	customName = "Commander D'krn",
	socialGroup = "imperial",
	faction = "imperial",
	level = 330,
	chanceHit = .6,
	damageMin = 645,
	damageMax = 1000,
	baseXp = 9429,
	baseHAM = 300000,
	baseHAMmax = 450000,
	armor = 0,
	resists = {0,0,0,0,0,0,0,0,-1},
	meatType = "",
	meatAmount = 0,
	hideType = "",
	hideAmount = 0,
	boneType = "",
	boneAmount = 0,
	milk = 2 * 0,
	tamingChance = 0,
	ferocity = 0,
	pvpBitmask = NONE,
	creatureBitmask = PACK,
	optionsBitmask = INVULNERABLE + CONVERSABLE,
	diet = HERBIVORE,

	templates = {"object/mobile/dressed_imperial_colonel_m.iff"},
	lootGroups = {
			{
			groups =
			{
				{group = "imperial_officer_common", chance = 10000000},
			},
			lootChance = 7500000,
		},
	
	},
	weapons = {},
	conversationTemplate = "commanderDkrnConvoTemplate",
	attacks = {}
}

CreatureTemplates:addCreatureTemplate(death_watch_herald_imperial, "death_watch_herald_imperial")
