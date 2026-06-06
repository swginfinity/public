laen_pieweto = Creature:new {
	objectName = "@mob/creature_names:patron_chiss_male",
	customName = "Laen Pieweto (Chiss Poacher Leader)",
	faction = "thug",
	socialGroup = "thug",
	mobType = MOB_NPC,
	level = 400, 
	chanceHit = 25,
	damageMin = 3000,
	damageMax = 5000,
	attackSpeed = 1.0,
	specialPercentage = 70,
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
	creatureBitmask = PACK + KILLER + HEALER + NOKNOCKDOWN + NOPOSTUREDOWN + NODIZZY + NOBLIND,
	optionsBitmask = AIENABLED,
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
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "mixed_force_weapons",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = merge(brawlermaster, chissboss),
	secondaryAttacks = {},
}

CreatureTemplates:addCreatureTemplate(laen_pieweto, "laen_pieweto")
 