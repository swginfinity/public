monstrous_brute = Creature:new {
	objectName = "@mob/creature_names:rancor_monstrous_brute",
	socialGroup = "rancor",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 225,
	chanceHit = 2.82,
	damageMin = 1400,
	damageMax = 1750,
	baseXp = 8315,
	baseHAM = 170000,
	baseHAMmax = 200000,
	armor = 1,
	resists = {185,185,185,190,170,165,170,185,135},
	meatType = "meat_carnivore",
	meatAmount = 950,
	hideType = "hide_leathery",
	hideAmount = 878,
	boneType = "bone_mammal",
	boneAmount = 778,
	milk = 0,
	tamingChance = 0,
	ferocity = 12,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + KILLER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/rancor_hue.iff"},
	scale = 1.60,
	lootGroups = {
		{
			groups = {
				{group = "rancor_common", chance = 6500000},
				{group = "weapons_all", chance = 3500000},
			},
			lootChance = 7500000
		},
		{
			groups = {
				{group = "captain_ring_group", chance = 10000000},
			},
			lootChance = 1 * (100000)
		}
	},

	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "unarmed",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = { {"knockdownattack",""}, {"creatureareaattack",""} },
	secondaryAttacks = { }
}

CreatureTemplates:addCreatureTemplate(monstrous_brute, "monstrous_brute")
