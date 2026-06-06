chiss_poacher_hunter = Creature:new {
	objectName = "@mob/creature_names:chiss_poacher_hunter",
	randomNameType = NAME_GENERIC,
	randomNameTag = true,
	faction = "thug",
	socialGroup = "thug",
	mobType = MOB_NPC,
	level = 300,
	chanceHit = 7,
	damageMin = 820,
	damageMax = 1350,
	baseXp = 20000,
	baseHAM = 80000,
	baseHAMmax = 120000,
	armor = 2,
	-- {kinetic, energy, electric, stun, blast, heat, cold, acid, ls}
	resists = {50,50,50,50,50,50,50,50,15},
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
	creatureBitmask = PACK + KILLER,
	optionsBitmask = AIENABLED,
	diet = HERBIVORE,

	templates = {
			"object/mobile/ep3/shared_ep3_etyyy_chiss_poacher_hunter_01.iff",
			"object/mobile/ep3/shared_ep3_etyyy_chiss_poacher_hunter_02.iff",
			"object/mobile/ep3/shared_ep3_etyyy_chiss_poacher_hunter_03.iff",
			"object/mobile/ep3/shared_ep3_etyyy_chiss_poacher_hunter_04.iff"},

		lootGroups = {
		{
			groups = {
				{group = "pistols", chance = 750000},
				{group = "carbines", chance = 750000},
				{group = "rifles", chance = 750000},
				{group = "melee_baton", chance = 1000000},
				{group = "loot_kit_parts", chance = 1000000},
				{group = "armor_attachments", chance = 2500000},
				{group = "clothing_attachments", chance = 2500000},
				{group = "color_crystals", chance = 750000},
			}
		}
	},
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "pirate_weapons_heavy",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = merge(riflemanmaster,pistoleermaster,carbineermaster,brawlermaster),
	secondaryAttacks = { },
	}

	CreatureTemplates:addCreatureTemplate(chiss_poacher_hunter, "chiss_poacher_hunter")
