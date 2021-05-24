mad_angler = Creature:new {
	objectName = "@mob/creature_names:mad_angler",
	socialGroup = "angler",
	faction = "",
	level = 95,
	chanceHit = 1.7,
	damageMin = 1000,
	damageMax = 1490,
	baseXp = 9057,
	baseHAM = 32400,
	baseHAMmax = 36000,
	armor = 1,
	-- {kinetic,energy,blast,heat,cold,electricity,acid,stun,ls
	resists = {180,185,150,140,120,160,145,180,135},
	meatType = "meat_insect",
	meatAmount = 250,
	hideType = "hide_scaley",
	hideAmount = 250,
	boneType = "",
	boneAmount = 0,
	milk = 0,
	tamingChance = 0.05,
	ferocity = 11,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + KILLER + STALKER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/angler_hue.iff"},
	controlDeviceTemplate = "object/intangible/pet/angler_hue.iff",
	scale = 1.4,
	lootGroups = {},
	weapons = {"creature_spit_small_toxicgreen"},
	conversationTemplate = "",


	attacks = {
		{"creatureareapoison",""},
		{"strongpoison",""}
	}
}

CreatureTemplates:addCreatureTemplate(mad_angler, "mad_angler")
