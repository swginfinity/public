talus_liberation_party_partisan = Creature:new {
	objectName = "@mob/creature_names:liberation_partisan",
	randomNameType = NAME_GENERIC,
	randomNameTag = true,
	socialGroup = "liberation_party",
	faction = "liberation_party",
	mobType = MOB_NPC,
	level = 78,
	chanceHit = 1.45,
	damageMin = 460,
	damageMax = 620,
	baseXp = 6990,
	baseHAM = 11000,
	baseHAMmax = 14000,
	armor = 1,
	resists = {20,20,160,165,165,5,5,35,-1},
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
		}
	},
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "st_bombardier_weapons",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = merge(brawlermaster,marksmanmaster,commandomaster),
	secondaryAttacks = { },
	reactionStf = "@npc_reaction/townperson",
}

CreatureTemplates:addCreatureTemplate(talus_liberation_party_partisan, "talus_liberation_party_partisan")
