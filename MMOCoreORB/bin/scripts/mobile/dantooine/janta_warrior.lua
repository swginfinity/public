janta_warrior = Creature:new {
	objectName = "@mob/creature_names:janta_warrior",
	randomNameType = NAME_GENERIC,
	randomNameTag = true,
	socialGroup = "janta_tribe",
	faction = "janta_tribe",
	level = 90,
	chanceHit = 0.9,
	damageMin = 695,
	damageMax = 800,
	baseXp = 7115,
	baseHAM = 17000,
	baseHAMmax = 23000,
	armor = 1,
	-- {kinetic,energy,blast,heat,cold,electricity,acid,stun,ls}
	resists = {40,40,20,10,20,25,20,30,-1},
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
	creatureBitmask = PACK + HERD,
	optionsBitmask = AIENABLED,
	diet = HERBIVORE,

	templates = {
		"object/mobile/dantari_male.iff",
		"object/mobile/dantari_female.iff"},
	lootGroups = {
	{
	 groups = {
		 {group = "janta_common", chance = 5000000},
		 {group = "melee_weapons", chance = 2500000},
		 {group = "junk", chance = 2500000}
	 },
	 lootChance = 10000000
		},
		},
	weapons = {"primitive_weapons"},
	conversationTemplate = "",
	attacks = merge(pikemanmaster,fencermaster,brawlermaster)
}

CreatureTemplates:addCreatureTemplate(janta_warrior, "janta_warrior")
