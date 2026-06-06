talus_liberation_party_activist = Creature:new {
	objectName = "@mob/creature_names:liberation_activist",
	randomNameType = NAME_GENERIC,
	randomNameTag = true,
	socialGroup = "liberation_party",
	faction = "liberation_party",
	mobType = MOB_NPC,
	level = 90,
	chanceHit = 2.25,
	damageMin = 550,
	damageMax = 840,
	baseXp = 7771,
	baseHAM = 12000,
	baseHAMmax = 15000,
	armor = 1,
	resists = {25,35,165,175,175,15,5,45,-1},
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
				{group = "rifles", chance = 2000000},
				{group = "tailor_components", chance = 1500000},
				{group = "loot_kit_parts", chance = 1500000}
			}
		},
	},
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "nyaxs_weapons",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = merge(brawlermaster,marksmanmaster,swordsmanmaster,fencermaster),
	secondaryAttacks = { },
	reactionStf = "@npc_reaction/townperson",
}

CreatureTemplates:addCreatureTemplate(talus_liberation_party_activist, "talus_liberation_party_activist")
