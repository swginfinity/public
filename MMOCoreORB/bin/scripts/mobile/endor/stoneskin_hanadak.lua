stoneskin_hanadak = Creature:new {
	objectName = "@mob/creature_names:stoneskin_hanadak",
	socialGroup = "hanadak",
	faction = "",
	level = 139,
	chanceHit = 4.75,
	damageMin = 720,
	damageMax = 1150,
	baseXp = 13178,
	baseHAM = 88000,
	baseHAMmax = 107000,
	armor = 2,
	resists = {190,190,165,200,200,200,175,180,140},
	meatType = "meat_carnivore",
	meatAmount = 123,
	hideType = "hide_bristley",
	hideAmount = 123,
	boneType = "bone_mammal",
	boneAmount = 123,
	milk = 2 * 0,
	tamingChance = 0,
	ferocity = 25,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + KILLER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/hanadak_hue.iff"},
	scale = 1.25,
	lootGroups = {},
	weapons = {},
	conversationTemplate = "",
	attacks = {
		{"creatureareacombo",""},
		{"creatureareaknockdown",""}
	}
}

CreatureTemplates:addCreatureTemplate(stoneskin_hanadak, "stoneskin_hanadak")
