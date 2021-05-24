death_watch_mine_rat = Creature:new {
	objectName = "@mob/creature_names:death_watch_quenker",
	socialGroup = "death_watch",
	faction = "",
	level = 202,
	chanceHit = 1,
	damageMin =645,
	damageMax = 1000,
	baseXp = 9708,
	baseHAM = 54000,
	baseHAMmax = 60000,
	armor = 2,
	resists = {55,55,80,35,35,35,75,135,-1},
	meatType = "meat_wild",
	meatAmount = 250,
	hideType = "hide_scaley",
	hideAmount = 250,
	boneType = "",
	boneAmount = 0,
	milk = 2 * 0,
	tamingChance = 0,
	ferocity = 0,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + KILLER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/womp_rat.iff"},
	lootGroups = {},
	weapons = {},
	conversationTemplate = "",
	attacks = {
		{"intimidationattack",""},
		{"knockdownattack",""}
	}
}

CreatureTemplates:addCreatureTemplate(death_watch_mine_rat, "death_watch_mine_rat")
