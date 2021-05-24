bark_mite_burrower_king = Creature:new {
	objectName = "@mob/creature_names:bark_mite_burrower_king",
	socialGroup = "mite",
	faction = "",
	level = 95,
	chanceHit = 1.7,
	damageMin = 1000,
	damageMax = 1500,
	baseXp = 9057,
	baseHAM = 32400,
	baseHAMmax = 36000,
	armor = 1,
	-- {kinetic,energy,blast,heat,cold,electricity,acid,stun,ls}
	resists = {185,185,175,180,150,160,175,180,135},
	meatType = "meat_insect",
	meatAmount = 400,
	hideType = "",
	hideAmount = 0,
	boneType = "",
	boneAmount = 0,
	milk = 2 * 0,
	tamingChance = 0.20,
	ferocity = 11,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + KILLER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/bark_mite_hue.iff"},
	controlDeviceTemplate = "object/intangible/pet/bark_mite_hue.iff",
	scale = 1.3,
	lootGroups = {},
	weapons = {"creature_spit_small_yellow"},
	conversationTemplate = "",
	attacks = {
		{"strongdisease",""},
		{"creatureareadisease",""}
	}
}

CreatureTemplates:addCreatureTemplate(bark_mite_burrower_king, "bark_mite_burrower_king")
