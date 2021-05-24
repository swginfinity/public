great_plains_stalker = Creature:new {
	objectName = "@mob/creature_names:great_plains_stalker",
	socialGroup = "panther",
	faction = "",
	level = 95,
	chanceHit = 1.75,
	damageMin = 900,
	damageMax = 1500,
	baseXp = 9057,
	baseHAM = 32400,
	baseHAMmax = 36000,
	armor = 1,
	-- {kinetic,energy,blast,heat,cold,electricity,acid,stun,ls}
	resists = {185,180,55,65,70,180,75,80,135},
	meatType = "meat_carnivore",
	meatAmount = 400,
	hideType = "hide_bristley",
	hideAmount = 400,
	boneType = "bone_mammal",
	boneAmount = 400,
	milk = 2 * 0,
	tamingChance = 0.05,
	ferocity = 11,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = KILLER + STALKER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/corellian_sand_panther_hue.iff"},
	controlDeviceTemplate = "object/intangible/pet/great_plains_stalker_hue.iff",
	scale = 1.25,
	lootGroups = {},
	weapons = {},
	conversationTemplate = "",
	attacks = {
		{"creatureareaknockdown",""},
		{"creatureareacombo",""}
	}
}

CreatureTemplates:addCreatureTemplate(great_plains_stalker, "great_plains_stalker")
