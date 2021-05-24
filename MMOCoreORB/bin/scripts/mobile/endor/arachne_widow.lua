arachne_widow = Creature:new {
	objectName = "@mob/creature_names:arachne_widow",
	socialGroup = "arachne",
	faction = "",
	level = 95,
	chanceHit = 1.7,
	damageMin = 900,
	damageMax = 1500,
	baseXp = 9057,
	baseHAM = 32400,
	baseHAMmax = 36000,
	armor = 1,
	-- {kinetic,energy,blast,heat,cold,electricity,acid,stun,ls
	resists = {180,180,175,150,130,165,190,160,135},
	meatType = "meat_insect",
	meatAmount = 123,
	hideType = "",
	hideAmount = 0,
	boneType = "",
	boneAmount = 0,
	milk = 2 * 0,
	tamingChance = 0.02,
	ferocity = 11,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + HERD + KILLER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/angler_hue.iff"},
	controlDeviceTemplate = "object/intangible/pet/angler_hue.iff",
	scale = 1.25,
	lootGroups = {},
	weapons = {},
	conversationTemplate = "",
	attacks = {
		{"creatureareapoison",""},
		{"strongpoison",""},
	}
}

CreatureTemplates:addCreatureTemplate(arachne_widow, "arachne_widow")
