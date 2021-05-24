mokk_soothsayer = Creature:new {
	objectName = "@mob/creature_names:mokk_soothsayer",
	randomNameType = NAME_GENERIC,
	randomNameTag = true,
	socialGroup = "mokk_tribe",
	faction = "mokk_tribe",
	level = 90,
	chanceHit = 0.9,
	damageMin = 695,
	damageMax = 800,
	baseXp = 7115,
	baseHAM = 17000,
	baseHAMmax = 23000,
	armor = 1,
	-- {kinetic,energy,blast,heat,cold,electricity,acid,stun,ls}
	resists = {30,30,20,10,20,25,20,30,-1},
	meatType = "",
	meatAmount = 0,
	hideType = "",
	hideAmount = 0,
	boneType = "",
	boneAmount = 0,
	milk = 2 * 0,
	tamingChance = 0,
	ferocity = 0,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + HERD,
	optionsBitmask = AIENABLED,
	diet = HERBIVORE,

	templates = {
		"object/mobile/dantari_male.iff",
		"object/mobile/dantari_female.iff"},
	lootGroups = {
		{
			groups = {
				{group = "junk", chance = 5000000},
				{group = "color_crystals", chance = 5000000}
			}
		},
		{
			groups = {
				{group = "mokk_common", chance = 100 * (100000)}
			},
			lootChance = 50 * (100000)
		},
	},
	weapons = {"primitive_weapons"},
	conversationTemplate = "",
	attacks = merge(pikemanmaster,fencermaster,brawlermaster)
}

CreatureTemplates:addCreatureTemplate(mokk_soothsayer, "mokk_soothsayer")
