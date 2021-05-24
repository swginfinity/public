ravenous_roba_stalker = Creature:new {
	objectName = "@mob/creature_names:ravenous_roba_stalker",
	socialGroup = "roba",
	faction = "",
	level = 40,
	chanceHit = 0.44,
	damageMin = 345,
	damageMax = 400,
	baseXp = 3915,
	baseHAM = 9300,
	baseHAMmax = 11300,
	armor = 0,
	resists = {30,30,30,175,-1,30,30,30,-1},
	meatType = "meat_carnivore",
	meatAmount = 123,
	hideType = "hide_leathery",
	hideAmount = 123,
	boneType = "bone_mammal",
	boneAmount = 123,
	milk = 2 * 0,
	tamingChance = 0.25,
	ferocity = 0,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + STALKER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/roba_hue.iff"},
	controlDeviceTemplate = "object/intangible/pet/roba_hue.iff",
	scale = 1.2,
	lootGroups = {},
	weapons = {},
	conversationTemplate = "",
	attacks = {
		{"",""},
		{"knockdownattack",""}
	}
}

CreatureTemplates:addCreatureTemplate(ravenous_roba_stalker, "ravenous_roba_stalker")
