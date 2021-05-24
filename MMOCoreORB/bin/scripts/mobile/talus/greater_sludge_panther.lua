greater_sludge_panther = Creature:new {
	objectName = "@mob/creature_names:greater_sludge_panther",
	socialGroup = "panther",
	faction = "",
	level = 65,
	chanceHit = 0.85,
	damageMin = 525,
	damageMax = 750,
	baseXp = 6500,
	baseHAM = 12000,
	baseHAMmax = 15000,
	armor = 1,
	-- {kinetic,energy,blast,heat,cold,electricity,acid,stun,ls}
	resists = {165,165,160,60,50,40,65,50,-1},
	meatType = "meat_carnivore",
	meatAmount = 123,
	hideType = "hide_leathery",
	hideAmount = 123,
	boneType = "bone_mammal",
	boneAmount = 123,
	milk = 2 * 0,
	tamingChance = 0.02,
	ferocity = 11,
	pvpBitmask = ATTACKABLE,
	creatureBitmask = PACK + KILLER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/greater_sludge_panther.iff"},
	controlDeviceTemplate = "object/intangible/pet/corellian_sand_panther_hue.iff",
	scale = 2.0,
	lootGroups = {},
	weapons = {},
	conversationTemplate = "",
	attacks = {
		{"posturedownattack",""},
		{"creatureareableeding",""}
	}
}

CreatureTemplates:addCreatureTemplate(greater_sludge_panther, "greater_sludge_panther")
