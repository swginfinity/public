gorax_grinch = Creature:new {
    customName = "Grinch",
	socialGroup = "townsperson",
	faction = "",
	level = 350,
	chanceHit = 2.75,
	damageMin = 1145,
	damageMax = 1766,
	baseXp = 3921,
	baseHAM = 2360434,
	baseHAMmax = 4323012,
	armor = 1,
	-- {kinetic,energy,blast,heat,cold,electricity,acid,stun,ls}
	resists = {15,15,15,15,15,15,15,15,70},
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
	diet = CARNIVORE,
  scale = 0.7,

	templates = {
		"object/mobile/gorax_grinch.iff"},

	lootGroups = {
		{
			groups = {
				{group = "gorax_common", chance = 3500000},
				{group = "power_crystals", chance = 3500000},
				{group = "holocron_dark", chance = 250000},
				{group = "named_crystals", chance = 250000},
				{group = "clothing_attachments", chance = 2500000}
			},
				lootChance = 8500000
			},
    {
      groups = {
				{group = "landspeeder_ab1", chance = 3500000},
				{group = "power_crystals", chance = 3500000},
				{group = "named_crystals", chance = 250000},
				{group = "holocron_light", chance = 250000},
				{group = "clothing_attachments", chance = 2500000}
			},
        lootChance = 5500000
        },
        {
        groups = {
    				{group = "mtg_paintings", chance = 3500000},
    				{group = "axkva_min", chance = 3500000},
    				{group = "minor_gorax", chance = 250000},
    				{group = "holocron_light", chance = 250000},
    				{group = "clothing_attachments", chance = 2500000}
    			},
            lootChance = 5500000
        },
		{ -- Jedi Specific Loot Group
		groups = 
		{
			{group = "jedi_clothing_attachments", chance = 4000000},
			{group = "named_crystals", chance = 6000000},
			},
			lootChance = 2000000,
		},
    	{ -- Veteran Rewards
		groups =
		{
        	{group = "minor_gorax", chance = 6000000},
        	{group = "veteran_rewards_180_days", chance = 3000000},
        	{group = "veteran_rewards_360_days", chance = 100000},
        	{group = "veteran_rewards_450_days", chance = 900000},
      	},
      	lootChance = 3500000,
		},
		{ -- XJ-2 Airspeeder  / New Paintings
		groups =
		{
			{group = "landspeeder_organa", chance = 5000000},
			{group = "mtg_paintings", chance = 5000000},
      	},
      	lootChance = 5000000,
    	},
	},

	weapons = {},
	conversationTemplate = "",

  attacks = {
		{"creatureareacombo","stateAccuracyBonus=100"},
		{"creatureareaknockdown","stateAccuracyBonus=100"},
		{"creatureareableeding","stateAccuracyBonus=100"},
	}
}
CreatureTemplates:addCreatureTemplate(gorax_grinch, "gorax_grinch")
