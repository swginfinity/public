greater_desert_womp_rat = Creature:new {
	objectName = "@mob/creature_names:greater_desert_womprat",
	socialGroup = "rat",
	faction = "",
	level = 22,
	chanceHit = 0.35,
	damageMin = 210,
	damageMax = 220,
	baseXp = 2006,
	baseHAM = 5900,
	baseHAMmax = 7200,
	armor = 0,
	resists = {10,10,135,130,10,-1,-1,-1,-1},
	meatType = "meat_wild",
	meatAmount = 123,
	hideType = "hide_leathery",
	hideAmount = 123,
	boneType = "bone_mammal",
	boneAmount = 123,
	milk = 0,
	tamingChance = 0.25,
	ferocity = 6,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + STALKER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/greater_desert_womp_rat.iff"},
	controlDeviceTemplate = "object/intangible/pet/womp_rat_hue.iff",
	scale = 1.25,
	lootGroups = {},
	weapons = {"creature_spit_small_yellow"},
	conversationTemplate = "",
	attacks = {
		{"blindattack",""},
		{"stunattack",""}
	}
}

CreatureTemplates:addCreatureTemplate(greater_desert_womp_rat, "greater_desert_womp_rat")
