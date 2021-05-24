salt_mynock = Creature:new {
	objectName = "@mob/creature_names:salt_mynock",
	socialGroup = "mynock",
	faction = "",
	level = 23,
	chanceHit = 0.35,
	damageMin = 210,
	damageMax = 220,
	baseXp = 2219,
	baseHAM = 5900,
	baseHAMmax = 7200,
	armor = 0,
	resists = {10,120,10,140,140,-1,-1,-1,-1},
	meatType = "meat_herbivore",
	meatAmount = 123,
	hideType = "hide_leathery",
	hideAmount = 123,
	boneType = "bone_mammal",
	boneAmount = 123,
	milk = 2 * 0,
	tamingChance = 0.25,
	ferocity = 0,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/salt_mynock.iff"},
	controlDeviceTemplate = "object/intangible/pet/salt_mynock_hue.iff",
	scale = 0.95,
	lootGroups = {},
	weapons = {},
	conversationTemplate = "",
	attacks = {
		{"knockdownattack",""},
		{"blindattack",""}
	}
}

CreatureTemplates:addCreatureTemplate(salt_mynock, "salt_mynock")
