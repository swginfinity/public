purbole_hunter = Creature:new {
	objectName = "@mob/creature_names:purbole_hunter",
	socialGroup = "purbole",
	faction = "",
	level = 21,
	chanceHit = 0.32,
	damageMin = 170,
	damageMax = 180,
	baseXp = 2006,
	baseHAM = 5000,
	baseHAMmax = 6100,
	armor = 0,
	resists = {125,125,5,-1,-1,145,-1,-1,-1},
	meatType = "meat_carnivore",
	meatAmount = 123,
	hideType = "hide_bristley",
	hideAmount = 123,
	boneType = "bone_mammal",
	boneAmount = 123,
	milk = 2 * 0,
	tamingChance = 0.25,
	ferocity = 5,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + KILLER + STALKER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/purbole_hue.iff"},
	controlDeviceTemplate = "object/intangible/pet/purbole_hunter_hue.iff",
	scale = 1.1,
	lootGroups = {},
	weapons = {},
	conversationTemplate = "",
	attacks = {
		{"stunattack",""},
		{"intimidationattack",""}
	}
}

CreatureTemplates:addCreatureTemplate(purbole_hunter, "purbole_hunter")
