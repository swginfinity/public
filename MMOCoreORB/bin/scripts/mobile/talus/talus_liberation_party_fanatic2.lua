talus_liberation_party_fanatic2 = Creature:new {
	customName = "Liberation Army Occupation Forces",
	randomNameType = NAME_GENERIC,
	randomNameTag = true,
	socialGroup = "liberation_party",
	faction = "liberation_party",
	mobType = MOB_NPC,
	level = 150,
	chanceHit = 1.1,
	damageMin = 850,
	damageMax = 1170,
	baseXp = 8389,
	baseHAM = 36000,
	baseHAMmax = 48000,
	armor = 2,
	resists = {35,75,75,65,65,40,5,25,-1},
	meatType = "",
	meatAmount = 0,
	hideType = "",
	hideAmount = 0,
	boneType = "",
	boneAmount = 0,
	milk = 2 * 0,
	tamingChance = 0,
	ferocity = 0,
	pvpBitmask = ATTACKABLE,
	creatureBitmask = PACK + HERD,
	optionsBitmask = AIENABLED,
	diet = HERBIVORE,

	templates = {"object/mobile/dressed_talus_sif_mercenary_rodian_01.iff"},
	lootGroups = {
		{
			groups = {
				{group = "junk", chance = 3000000},
				{group = "wearables_common", chance = 2000000},
				{group = "carbines", chance = 2000000},
				{group = "tailor_components", chance = 1500000},
				{group = "loot_kit_parts", chance = 1500000}
			}
		},
		{
			groups = {
				{group = "art_myriad2",       chance = 100 * (100000)},
			},
			lootChance = 4 * (100000)
		}
	},
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "rebel_weapons_light",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = merge(brawlermaster,marksmanmaster,bountyhuntermaster,pistoleermaster),
	secondaryAttacks = { },
	reactionStf = "@npc_reaction/townperson",
}

CreatureTemplates:addCreatureTemplate(talus_liberation_party_fanatic2, "talus_liberation_party_fanatic2")
