ancient_mamien = Creature:new {
	objectName = "@mob/creature_names:mamien_ancient",
	socialGroup = "mamien",
	faction = "",
	level = 24,
	chanceHit = 0.34,
	damageMin = 200,
	damageMax = 210,
	baseXp = 2543,
	baseHAM = 6300,
	baseHAMmax = 7700,
	armor = 0,
	resists = {120,135,10,10,10,10,10,-1,-1},
	meatType = "meat_wild",
	meatAmount = 123,
	hideType = "hide_wooly",
	hideAmount = 123,
	boneType = "bone_mammal",
	boneAmount = 123,
	milk = 2 * 0,
	tamingChance = 0,
	ferocity = 0,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + HERD + KILLER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/mamien_ancient.iff"},
	scale = 1.15,
	lootGroups = {},
	weapons = {},
	conversationTemplate = "",
	attacks = {
		{"dizzyattack",""}
	}
}

CreatureTemplates:addCreatureTemplate(ancient_mamien, "ancient_mamien")
