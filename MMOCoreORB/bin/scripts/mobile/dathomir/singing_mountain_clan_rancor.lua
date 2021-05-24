singing_mountain_clan_rancor = Creature:new {
	objectName = "@mob/creature_names:singing_mountain_clan_rancor",
	socialGroup = "mtn_clan",
	faction = "mtn_clan",
	level = 135,
	chanceHit = 1.35,
	damageMin = 370,
	damageMax = 450,
	baseXp = 5281,
	baseHAM = 33000,
	baseHAMmax = 42000,
	armor = 1,
	resists = {130,160,5,200,200,200,5,5,-1},
	meatType = "meat_carnivore",
	meatAmount = 920,
	hideType = "hide_leathery",
	hideAmount = 710,
	boneType = "bone_mammal",
	boneAmount = 780,
	milk = 0,
	tamingChance = 0,
	ferocity = 10,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + STALKER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/rancor.iff"},
	scale = 1.15,
	lootGroups = {
		{
			groups = {
				{group = "rancor_common", chance = 4000000},
				{group = "armor_all", chance = 2000000},
				{group = "weapons_all", chance = 2500000},
				{group = "wearables_all", chance = 1500000}
			},
			lootChance = 6600000
		}
	},
	weapons = {},
	conversationTemplate = "",
	attacks = {
		{"",""},
		{"intimidationattack",""}
	}
}

CreatureTemplates:addCreatureTemplate(singing_mountain_clan_rancor, "singing_mountain_clan_rancor")
