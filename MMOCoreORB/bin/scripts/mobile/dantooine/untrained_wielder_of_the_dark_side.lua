untrained_wielder_of_the_dark_side = Creature:new {
	objectName = "@mob/creature_names:untrained_wielder_of_the_dark_side",
	randomNameType = NAME_GENERIC,
	randomNameTag = true,
	socialGroup = "kun",
	faction = "",
	level = 85,
	chanceHit = 0.9,
	damageMin = 545,
	damageMax = 800,
	baseXp = 7288,
	baseHAM = 16000,
	baseHAMmax = 19500,
	armor = 1,
	-- {kinetic, energy, electric, stun, blast, heat, cold, acid, ls}
	resists = {45,45,35,5,45,25,15,15,10},
	meatType = "",
	meatAmount = 0,
	hideType = "",
	hideAmount = 0,
	boneType = "",
	boneAmount = 0,
	milk = 0,
	tamingChance = 0,
	ferocity = 0,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK,
	optionsBitmask = AIENABLED,
	diet = HERBIVORE,

	templates = {"object/mobile/dressed_untrained_wielder_of_the_darkside.iff"},
	lootGroups = {
		{
			groups = {
				{group = "junk", chance = 4000000},
				{group = "power_crystals", chance = 2500000},
				{group = "color_crystals", chance = 500000},
				{group = "holocron_dark", chance = 500000},
				{group = "holocron_light", chance = 500000},
				{group = "armor_attachments", chance = 1500000},
				{group = "clothing_attachments", chance = 500000}
			}
		},
		{
			groups = {
				{group = "art_alderaan2", chance = 10000000}
			},
			lootChance = 500000
		}
	},
	weapons = {"mixed_force_weapons"},
	conversationTemplate = "",
	attacks = merge(pikemanmaster,brawlermaster,fencermaster)
}

CreatureTemplates:addCreatureTemplate(untrained_wielder_of_the_dark_side, "untrained_wielder_of_the_dark_side")
