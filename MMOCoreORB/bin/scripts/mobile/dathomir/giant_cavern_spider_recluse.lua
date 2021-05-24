giant_cavern_spider_recluse = Creature:new {
	objectName = "@mob/creature_names:cave_gaping_spider_recluse_giant",
	socialGroup = "spider_nightsister",
	faction = "spider_nightsister",
	level = 170,
	chanceHit = 4.75,
	damageMin = 895,
	damageMax = 1500,
	baseXp = 14789,
	baseHAM = 75000,
	baseHAMmax = 90000,
	armor = 2,
	-- {kinetic,energy,blast,heat,cold,electricity,acid,stun,ls}
	resists = {180,175,165,195,165,165,195,165,140},
	meatType = "meat_insect",
	meatAmount = 400,
	hideType = "",
	hideAmount = 0,
	boneType = "",
	boneAmount = 0,
	milk = 2 * 0,
	tamingChance = 0,
	ferocity = 25,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + KILLER + STALKER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/gaping_spider_recluse.iff"},
	scale = 2.0,
	lootGroups = {},
	weapons = {},
	conversationTemplate = "",
	attacks = {
		{"creatureareapoison",""},
		{"strongpoison",""}
	}
}

CreatureTemplates:addCreatureTemplate(giant_cavern_spider_recluse, "giant_cavern_spider_recluse")
