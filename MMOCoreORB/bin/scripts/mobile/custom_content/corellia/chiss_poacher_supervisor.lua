chiss_poacher_supervisor = Creature:new {
	objectName = "@mob/creature_names:chiss_poacher_defender",
	randomNameType = NAME_GENERIC,
	randomNameTag = true,
	faction = "thug",
	socialGroup = "thug",
	mobType = MOB_NPC,
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
	milk = 0,
	tamingChance = 0,
	ferocity = 0,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + KILLER,
	optionsBitmask = AIENABLED,
	diet = HERBIVORE,
	--Elite Trash
	templates = {
			"object/mobile/ep3/shared_ep3_etyyy_chiss_poacher_defender_01.iff",
			"object/mobile/ep3/shared_ep3_etyyy_chiss_poacher_defender_02.iff",
			"object/mobile/ep3/shared_ep3_etyyy_chiss_poacher_defender_03.iff",
			"object/mobile/ep3/shared_ep3_etyyy_chiss_poacher_defender_04.iff"},

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

	CreatureTemplates:addCreatureTemplate(chiss_poacher_supervisor, "chiss_poacher_supervisor")
