chiss_poacher_bodyguard = Creature:new {
	objectName = "@mob/creature_names:mand_bunker_dthwatch_grey",
	customName = "Laen's Mandalorian Bodyguard",
	faction = "thug",
	socialGroup = "thug",
	mobType = MOB_NPC,
	level = 300,
	chanceHit = 10,
	damageMin = 1850,
	damageMax = 2550,
	attackSpeed = 1.5,
	specialPercentage = 60,
	baseXp = 20000,
	baseHAM = 295000,
	baseHAMmax = 350000,
	armor = 2,
	resists = {60,60,40,45,45,60,60,40,15},
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
	creatureBitmask = PACK + KILLER + NOPOSTUREDOWN,
	optionsBitmask = AIENABLED,
	diet = HERBIVORE,

	templates = {"object/mobile/dressed_death_watch_grey.iff"},

		lootGroups = {
		{
			groups = {

				{group = "loot_kit_parts", chance = 500000},
				{group = "armor_attachments", chance = 1000000},
				{group = "clothing_attachments", chance = 1000000},
				{group = "color_crystals", chance = 7500000},
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
	primaryAttacks = merge(bountyhuntermaster,marksmanmaster,brawlermaster,pikemanmaster,fencermaster,swordsmanmaster),
	secondaryAttacks = { },
	}

	CreatureTemplates:addCreatureTemplate(chiss_poacher_bodyguard, "chiss_poacher_bodyguard")
