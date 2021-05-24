mature_hanadak_rock_smasher = Creature:new {
	objectName = "@mob/creature_names:mature_hanadak_rock_smasher",
	socialGroup = "hanadak",
	faction = "",
	level = 35,
	chanceHit = 0.41,
	damageMin = 330,
	damageMax = 370,
	baseXp = 3460,
	baseHAM = 8700,
	baseHAMmax = 10700,
	armor = 0,
	resists = {20,160,20,-1,-1,-1,-1,-1,-1},
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
	creatureBitmask = PACK,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/hanadak_hue.iff"},
	controlDeviceTemplate = "object/intangible/pet/hanadak_hue.iff",
	scale = 1.05,
	lootGroups = {},
	weapons = {},
	conversationTemplate = "",
	attacks = {
		{"stunattack",""},
		{"dizzyattack",""}
	}
}

CreatureTemplates:addCreatureTemplate(mature_hanadak_rock_smasher, "mature_hanadak_rock_smasher")
