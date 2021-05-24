minor_gorax = Creature:new {
	objectName = "@mob/creature_names:minor_gorax",
	socialGroup = "gorax",
	faction = "",
	level = 200,
	chanceHit = 15,
	damageMin = 1000,
	damageMax = 1800,
	baseXp = 12424,
	baseHAM = 256000,
	baseHAMmax = 313000,
	armor = 3,
	resists = {165,165,150,150,150,0,150,150,-1},
	meatType = "",
	meatAmount = 0,
	hideType = "",
	hideAmount = 0,
	boneType = "",
	boneAmount = 0,
	milk = 2 * 0,
	tamingChance = 0,
	ferocity = 0,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + KILLER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/gorax.iff"},
	scale = 0.75,
	lootGroups = {
	    {
	        groups = {
				{group = "minor_gorax", chance = 6000000},
				{group = "armor_attachments", chance = 2000000},
				{group = "clothing_attachments", chance = 2000000}
			},
			lootChance = 8000000
		}
	},
	weapons = {},
	conversationTemplate = "",
	attacks = {
		{"creatureareaattack",""}
	}
}

CreatureTemplates:addCreatureTemplate(minor_gorax, "minor_gorax")
