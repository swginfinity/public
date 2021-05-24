tusken_observer = Creature:new {
	objectName = "@mob/creature_names:tusken_observer",
	socialGroup = "tusken_raider",
	faction = "tusken_raider",
	level = 186,
	chanceHit = 4.75,
	damageMin = 895,
	damageMax = 1500,
	baseXp = 14789,
	baseHAM = 75000,
	baseHAMmax = 90000,
	armor = 2,
	-- {kinetic,energy,blast,heat,cold,electricity,acid,stun,ls}
	resists = {15,75,15,75,75,75,75,75,-1},
	meatType = "",
	meatAmount = 0,
	hideType = "",
	hideAmount = 0,
	boneType = "",
	boneAmount = 0,
	milk = 0,
	tamingChance = 0,
	ferocity = 0,
	pvpBitmask = AGGRESSIVE + ENEMY + ATTACKABLE,
	creatureBitmask = PACK + KILLER,
	optionsBitmask = AIENABLED,
	diet = HERBIVORE,

	templates = {"object/mobile/tusken_raider.iff"},
	lootGroups = {
		{
			groups = {
				{group = "power_crystals", chance = 2000000},
				{group = "color_crystals", chance = 2000000},
				{group = "tusken_common", chance = 1000000},
				{group = "armor_attachments", chance = 1500000},   -- 70% * 15% = 10.5%
				{group = "clothing_attachments", chance = 1500000},   -- 70% * 15% = 10.5%
				{group = "melee_weapons", chance = 2000000},
			},
			lootChance = 7000000,   -- 70% chance for this group
		},
		{
			groups = {

				{group = "rifles", chance = 2500000},
				{group = "pistols", chance = 2500000},
				{group = "carbines", chance = 2500000},
				{group = "wearables_common", chance = 2500000},
			},
			lootChance = 5000000,   -- 50% chance for this group
		},
	},
	weapons = {"tusken_weapons"},
	conversationTemplate = "",
	attacks = merge(marksmanmaster,brawlermaster,fencermaster,riflemanmaster)
}

CreatureTemplates:addCreatureTemplate(tusken_observer, "tusken_observer")
