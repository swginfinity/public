monstrous_brute = Creature:new {
	objectName = "@mob/creature_names:rancor_monstrous_brute",
	socialGroup = "rancor",
	faction = "",
	level = 225,
	chanceHit = 2.25,
	damageMin = 1400,
	damageMax = 1750,
	baseXp = 12801,
	baseHAM = 200000,
	baseHAMmax = 230000,
	armor = 1,
	-- {kinetic,energy,blast,heat,cold,electricity,acid,stun,ls}
	resists = {185,185,185,190,170,160,150,175,135},
	meatType = "meat_carnivore",
	meatAmount = 1900,
	hideType = "hide_leathery",
	hideAmount = 1850,
	boneType = "bone_mammal",
	boneAmount = 1850,
	milk = 2 * 0,
	tamingChance = 0,
	ferocity = 30,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + KILLER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/rancor_hue.iff"},
	scale = 1.60,
	lootGroups = {
		{
			groups = {
				{group = "rancor_common", chance = 6500000},
				{group = "weapons_all", chance = 3500000},
			},
			lootChance = 7500000
		}
	},
	weapons = {},
	conversationTemplate = "",
	attacks = {
		{"creatureareaknockdown",""},
		{"creatureareableeding",""}
	}
}

CreatureTemplates:addCreatureTemplate(monstrous_brute, "monstrous_brute")
