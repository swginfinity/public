janta_clan_leader = Creature:new {
	objectName = "@mob/creature_names:janta_clan_leader",
	randomNameType = NAME_GENERIC,
	randomNameTag = true,
	socialGroup = "janta_tribe",
	faction = "janta_tribe",
	level = 100,
	chanceHit = 1.00,
	damageMin = 555,
	damageMax = 820,
	baseXp = 7500,
	baseHAM = 22000,
	baseHAMmax = 25000,
	armor = 1,
	-- {kinetic,energy,blast,heat,cold,electricity,acid,stun,ls}
	resists = {40,60,-1,30,30,60,30,10,-1},
	meatType = "",
	meatAmount = 0,
	hideType = "",
	hideAmount = 0,
	boneType = "",
	boneAmount = 0,
	milk = 2 * 0,
	tamingChance = 0,
	ferocity = 0,
	pvpBitmask = ATTACKABLE,
	creatureBitmask = PACK + HERD + KILLER,
	optionsBitmask = AIENABLED,
	diet = HERBIVORE,

	templates = {"object/mobile/dantari_male.iff"},
	lootGroups = {
		{
			groups = {
				{group = "janta_common", chance = 5000000},
				{group = "junk", chance = 5000000}
			},
			lootChance = 10000000
		},
		},
	weapons = {"primitive_weapons"},
	conversationTemplate = "",
	attacks = merge(pikemanmaster,fencermaster,brawlermaster)
}

CreatureTemplates:addCreatureTemplate(janta_clan_leader, "janta_clan_leader")
