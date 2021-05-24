ferocious_kusak = Creature:new {
	objectName = "@mob/creature_names:ferocious_kusak",
	socialGroup = "kusak",
	faction = "",
	level = 21,
	chanceHit = 0.33,
	damageMin = 210,
	damageMax = 220,
	baseXp = 2006,
	baseHAM = 4100,
	baseHAMmax = 5000,
	armor = 0,
	resists = {130,10,10,-1,10,10,-1,-1,-1},
	meatType = "meat_carnivore",
	meatAmount = 123,
	hideType = "hide_bristley",
	hideAmount = 123,
	boneType = "bone_mammal",
	boneAmount = 123,
	milk = 2 * 0,
	tamingChance = 0.25,
	ferocity = 0,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + KILLER + STALKER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/kusak_hue.iff"},
	controlDeviceTemplate = "object/intangible/pet/kusak_hue.iff",
	scale = 1.05,
	lootGroups = {},
	weapons = {},
	conversationTemplate = "",
	attacks = {
		{"blindattack",""},
		{"posturedownattack",""}
	}
}

CreatureTemplates:addCreatureTemplate(ferocious_kusak, "ferocious_kusak")
