paralope = Creature:new {
	objectName = "@mob/creature_names:paralope",
	socialGroup = "paralope",
	faction = "",
	level = 6,
	chanceHit = 0.25,
	damageMin = 50,
	damageMax = 55,
	baseXp = 113,
	baseHAM = 180,
	baseHAMmax = 220,
	armor = 0,
	resists = {0,0,0,0,0,0,0,-1,-1},
	meatType = "meat_herbivore",
	meatAmount = 123,
	hideType = "hide_wooly",
	hideAmount = 123,
	boneType = "bone_mammal",
	boneAmount = 123,
	milkType = "milk_wild",
	milk = 2 * 250,
	tamingChance = 0.25,
	ferocity = 0,
	pvpBitmask = ATTACKABLE,
	creatureBitmask = HERD,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/paralope.iff"},
	controlDeviceTemplate = "object/intangible/pet/swirl_prong_hue.iff",
	lootGroups = {},
	weapons = {},
	conversationTemplate = "",
	attacks = {
	}
}

CreatureTemplates:addCreatureTemplate(paralope, "paralope")
