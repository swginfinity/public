laen_pieweto = Creature:new {
	objectName = "@mob/creature_names:patron_chiss_male",
	customName = "Laen Pieweto (Chiss Poacher Leader)",
	faction = "thug",
	socialGroup = "thug",
	level = 400, 
	chanceHit = 25,
	damageMin = 4000,
	damageMax = 6000,
	specialDamageMult = 5.5,
	baseXp = 26654,
	baseHAM = 1221000,
	baseHAMmax = 1592000,
	armor = 3,
	-- {kinetic,energy,blast,heat,cold,electricity,acid,stun,ls}
	resists = {65,55,55,75,75,70,55,70,25},
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
	creatureBitmask = PACK + KILLER + HEALER,
	optionsBitmask = AIENABLED + BOSS,
	diet = HERBIVORE,

	templates = {"object/mobile/ep3/shared_ep3_etyyy_laen_pieweto.iff"},
	lootGroups = {
		{
			groups = {
				{group = "power_crystals", chance = 2500000},
				{group = "color_crystals", chance = 2500000},
				{group = "armor_attachments", chance = 2500000},
				{group = "clothing_attachments", chance = 2500000},  

			},
			lootChance = 9000000,  -- 90% chance for this group
		},
		{
			groups = {
				{group = "power_crystals", chance = 3000000},
				{group = "clothing_attachments", chance = 3000000},   
				{group = "armor_attachments", chance = 3000000},
				{group = "chiss_poacher_backpack_custom", chance = 1000000},
			},
			lootChance = 6000000,  -- 60% chance for this group	
		},
		{
			groups = {
				{group = "power_crystals", chance =10000000},
				},
			lootChance = 5000000, -- 50% chance for 3rd crystal.
		},
		{ -- Chiss backpack
			groups = {
				{group = "chiss_poacher_backpack_custom", chance = 10000000},
			},
			lootChance = 10000000,    -- Guarunteed Drop
		},
		{ -- Jedi Specific Loot Group
			groups =
			{
				{group = "named_crystals", chance = 10000000},
			},
			lootChance = 10000000, -- Guarunteed Drop
		},
		{ -- Jedi Specific Loot Group
			groups =
			{
				{group = "jedi_clothing_attachments", chance = 10000000}, -- 20% * 100% = 20%
			},
			lootChance = 2000000,  -- 10% chance for this group
		},
	},
	weapons = {"mixed_force_weapons"},
	reactionStf = "@npc_reaction/fancy",
	attacks = {
		{"creatureareacombo","stateAccuracyBonus=100"},
        {"creatureareaknockdown","stateAccuracyBonus=100"},
		{"intimidationattack","stateAccuracyBonus=75"}},
}

CreatureTemplates:addCreatureTemplate(laen_pieweto, "laen_pieweto")
