bull_rancor = Creature:new {
	objectName = "@mob/creature_names:bull_rancor",
	socialGroup = "rancor",
	faction = "",
	level = 65,
	chanceHit = 0.6,
	damageMin = 455,
	damageMax = 620,
	baseXp = 6500,
	baseHAM = 11000,
	baseHAMmax = 14000,
	armor = 1,
	resists = {135,160,15,180,180,180,15,15,-1},
	meatType = "meat_carnivore",
	meatAmount = 1000,
	hideType = "hide_leathery",
	hideAmount = 900,
	boneType = "bone_mammal",
	boneAmount = 850,
	milk = 2 * 0,
	tamingChance = 0.02,
	ferocity = 11,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + KILLER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/bull_rancor.iff"},
	controlDeviceTemplate = "object/intangible/pet/bull_rancor_hue.iff",
	scale = 1.15,
	lootGroups = {
		{
			groups = {
				{group = "rancor_common", chance = 6500000},
				{group = "weapons_all", chance = 3500000},
			},
			lootChance = 3500000
		}
	},
	weapons = {},
	conversationTemplate = "",
	attacks = {
		{"creatureareadisease",""},
		{"dizzyattack",""}
	}
}

CreatureTemplates:addCreatureTemplate(bull_rancor, "bull_rancor")
