hssiss_lizard = Creature:new {
    customName = "Hssiss Spawnling",
	socialGroup = "kun",
	faction = "",
	level = 215,
	chanceHit = 2.75,
	damageMin = 800,
	damageMax = 1300,
	baseXp = 3521,
	baseHAM = 70800,
	baseHAMmax = 113000,
	armor = 1,

	-- {kinetic,energy,blast,heat,cold,electricity,acid,stun,ls}
	resists = {60,60,60,50,40,20,40,60,15},
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
  scale = .3,

	templates = {
		"object/mobile/vog_eel.iff"},

	lootGroups = {
		{
			groups = {
				{group = "color_crystals", chance = 4000000},
				{group = "power_crystals", chance = 2500000},
				{group = "holocron_dark", chance = 750000},
				{group = "hssiss_venom", chance = 1250000},
				{group = "clothing_attachments", chance = 1500000}
			},
				lootChance = 8500000
			},
    {
      groups = {
				{group = "weapons_all", chance = 3500000},
				{group = "armor_attachments", chance = 3500000},
				{group = "holocron_dark", chance = 250000},
				{group = "hssiss_venom", chance = 250000},
				{group = "clothing_attachments", chance = 2500000}
			},
        lootChance = 7000000
        },
		{ -- Jedi Specific Loot Group
			groups =
			{
				{group = "jedi_clothing_attachments", chance = 4000000},
				{group = "named_crystals", chance = 6000000},
			},
			lootChance = 50000,
		},
    { -- Veteran Rewards
      groups =
      {
        {group = "veteran_rewards_90_days", chance = 6000000},
        {group = "veteran_rewards_180_days", chance = 3000000},
        {group = "veteran_rewards_360_days", chance = 100000},
        {group = "veteran_rewards_450_days", chance = 900000},
      },
      lootChance = 600000,
    },
	},
	weapons = {},
	conversationTemplate = "",
  attacks = {
  {"forcelightningcone2",""},
  {"forceweaken2",""},
  {"forcechoke",""}
}
}
CreatureTemplates:addCreatureTemplate(hssiss_lizard, "hssiss_lizard")
