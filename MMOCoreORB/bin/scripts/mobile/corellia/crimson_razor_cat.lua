crimson_razor_cat = Creature:new {
	objectName = "@mob/creature_names:crimson_sand_panther",
	socialGroup = "panther",
	faction = "",
	level = 29,
	chanceHit = 0.39,
	damageMin = 305,
	damageMax = 320,
	baseXp = 3005,
	baseHAM = 7700,
	baseHAMmax = 9400,
	armor = 0,
	resists = {135,135,-1,-1,15,-1,-1,-1,-1},
	meatType = "meat_carnivore",
	meatAmount = 123,
	hideType = "hide_bristley",
	hideAmount = 123,
	boneType = "bone_mammal",
	boneAmount = 123,
	milk = 2 * 0,
	tamingChance = 0.25,
	ferocity = 9,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + KILLER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/crimson_sand_panther.iff"},
	controlDeviceTemplate = "object/intangible/pet/crimson_razor_cat_hue.iff",
	scale = 1.15,
	lootGroups = {},
	weapons = {},
	conversationTemplate = "",
	attacks = {
		{"dizzyattack",""},
		{"stunattack",""}
	}
}

CreatureTemplates:addCreatureTemplate(crimson_razor_cat, "crimson_razor_cat")
