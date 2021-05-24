mutant_rancor = Creature:new {
	objectName = "@mob/creature_names:mutant_rancor",
	socialGroup = "rancor",
	faction = "",
	level = 95,
	chanceHit = 1.5,
	damageMin = 700,
	damageMax = 1500,
	baseXp = 9057,
	baseHAM = 32400,
	baseHAMmax = 36000,
	armor = 1,
	-- {kinetic,energy,blast,heat,cold,electricity,acid,stun,ls}
	resists = {80,90,60,190,190,185,50,50,-1},
	meatType = "meat_carnivore",
	meatAmount = 1000,
	hideType = "hide_leathery",
	hideAmount = 900,
	boneType = "bone_mammal",
	boneAmount = 850,
	milk = 2 * 0,
	tamingChance = 0.03,
	ferocity = 11,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + KILLER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/mutant_rancor.iff"},
	controlDeviceTemplate = "object/intangible/pet/rancor_hue.iff",
	scale = 1.2,
	lootGroups = {
		{
			groups = {
				{group = "rancor_common", chance = 4000000},
				{group = "armor_all", chance = 2000000},
				{group = "weapons_all", chance = 2500000},
				{group = "wearables_all", chance = 1500000}
			},
			lootChance = 2500000
		}
	},
	weapons = {},
	conversationTemplate = "",
	attacks = {
		{"strongdisease",""},
		{"creatureareableeding",""}
	}
}

CreatureTemplates:addCreatureTemplate(mutant_rancor, "mutant_rancor")
