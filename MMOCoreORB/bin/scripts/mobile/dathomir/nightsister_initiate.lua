nightsister_initiate = Creature:new {
	objectName = "@mob/creature_names:nightsister_initiate",
	randomNameType = NAME_GENERIC,
	randomNameTag = true,
	socialGroup = "nightsister",
	faction = "nightsister",
	level = 80,
	chanceHit = 2.5,
	damageMin = 445,
	damageMax = 600,
	baseXp = 5830,
	baseHAM = 18000,
	baseHAMmax = 21000,
	armor = 1,
	-- {kinetic,energy,blast,heat,cold,electricity,acid,stun,ls}
	resists = {10,10,10,100,100,100,100,100,-1},
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
	creatureBitmask = PACK + KILLER + STALKER,
	optionsBitmask = AIENABLED,
	diet = HERBIVORE,

	templates = {"object/mobile/dressed_dathomir_nightsister_initiate.iff"},
	lootGroups = {
		{
			groups = {
				{group = "power_crystals", chance = 3500000},
				{group = "color_crystals", chance = 1500000},
				{group = "nightsister_common", chance = 1500000},
				{group = "clothing_attachments", chance = 1000000},
				{group = "melee_weapons", chance = 2500000},
			},
			lootChance = 2500000,
		},
		{
			groups = {
				
				{group = "rifles", chance = 2500000},
				{group = "pistols", chance = 2500000},
				{group = "carbines", chance = 2500000},
				{group = "wearables_common", chance = 2500000},
			},
			lootChance = 3000000,  -- 30% chance for this group
		},
		{
			groups = {
				
				{group = "art_illum12", chance = 10000000},
			},
			lootChance = 500000,  -- 5% chance for this group
		},
	},
	weapons = {"mixed_force_weapons"},
	conversationTemplate = "",
	attacks = merge(tkamid,fencermid,swordsmanmid,pikemanmid,brawlermaster)
}

CreatureTemplates:addCreatureTemplate(nightsister_initiate, "nightsister_initiate")
