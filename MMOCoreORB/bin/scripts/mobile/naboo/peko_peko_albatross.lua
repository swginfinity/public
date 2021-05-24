peko_peko_albatross = Creature:new {
	objectName = "@mob/creature_names:peko_peko_albatross",
	socialGroup = "peko",
	faction = "",
	level = 140,
	chanceHit = 7.75,
	damageMin = 1945,
	damageMax = 2600,
	baseXp = 14694,
	baseHAM = 174000,
	baseHAMmax = 191000,
	armor = 2,
	resists = {25,25,25,25,25,25,25,25,50},
	meatType = "meat_avian",
	meatAmount = 4000,
	hideType = "hide_wooly",
	hideAmount = 1230,
	boneType = "bone_avian",
	boneAmount = 1230,
	milk = 2 * 0,
	tamingChance = 0,
	ferocity = 8,
	pvpBitmask = ATTACKABLE,
	creatureBitmask = PACK + HERD + KILLER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/peko_peko_hue.iff"},
	scale = 2.0,
	lootGroups = {
	 {
	        groups = {
				{group = "peko_albatross", chance = 10000000}
			},
			lootChance = 5000000
		}
	},
	weapons = {},
	conversationTemplate = "",
	attacks = {
		{"creatureareacombo",""},
		{"creatureareaknockdown",""}
	}
}

CreatureTemplates:addCreatureTemplate(peko_peko_albatross, "peko_peko_albatross")