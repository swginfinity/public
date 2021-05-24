chiss_poacher_director = Creature:new {
	objectName = "@mob/creature_names:chiss_poacher_smuggler",
	randomNameType = NAME_GENERIC,
	randomNameTag = true,
	faction = "thug",
	socialGroup = "thug",
	level = 300,
	chanceHit = 9,
	damageMin = 1000,
	damageMax = 1550,
	baseXp = 20000,
	baseHAM = 450000,
	baseHAMmax = 500000,
	armor = 2,
	-- {kinetic, energy, electric, stun, blast, heat, cold, acid, ls}
	resists = {60,60,35,50,35,50,25,50,15},
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
	diet = HERBIVORE,
	--Elite Trash Mob
	templates = {
			"object/mobile/ep3/shared_ep3_etyyy_chiss_poacher_smuggler_01.iff",
			"object/mobile/ep3/shared_ep3_etyyy_chiss_poacher_smuggler_02.iff",
			"object/mobile/ep3/shared_ep3_etyyy_chiss_poacher_smuggler_03.iff",
			"object/mobile/ep3/shared_ep3_etyyy_chiss_poacher_smuggler_04.iff"},

		lootGroups = {
		{
			groups = {
				{group = "armor_attachments", chance = 1500000},
				{group = "clothing_attachments", chance = 1500000},
				{group = "color_crystals", chance = 5000000},
				{group = "power_crystals", chance = 2000000},
			}
		}
	},
	weapons = {"pirate_weapons_heavy"},
	conversationTemplate = "",
	attacks = merge(riflemanmaster,pistoleermaster,carbineermaster,brawlermaster)
	}

	CreatureTemplates:addCreatureTemplate(chiss_poacher_director, "chiss_poacher_director")
