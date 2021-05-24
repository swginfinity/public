stintaril_prowler = Creature:new {
	objectName = "@mob/creature_names:stintaril_prowler",
	socialGroup = "stinaril",
	faction = "",
	level = 140,
	chanceHit = 2.0,
	damageMin = 1295,
	damageMax = 2000,
	baseXp = 7945,
	baseHAM = 120000,
	baseHAMmax = 150000,
	armor = 2,
	resists = {65,65,65,65,65,65,65,65,25},
	meatType = "meat_carnivore",
	meatAmount = 5000,
	hideType = "hide_bristley",
	hideAmount = 5000,
	boneType = "bone_mammal",
	boneAmount = 5000,
	milk = 0,
	tamingChance = 0,
	ferocity = 3,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + KILLER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/stintaril_hue.iff"},
	scale = 2.5,
	lootGroups = {},
	weapons = {"creature_spit_small_red"},
	conversationTemplate = "",
	attacks = {
		{"stunattack",""}
	}
}

CreatureTemplates:addCreatureTemplate(stintaril_prowler, "stintaril_prowler")
