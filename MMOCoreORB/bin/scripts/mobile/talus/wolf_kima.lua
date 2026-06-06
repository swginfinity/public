wolf_kima = Creature:new {
	objectName = "@mob/creature_names:wolf_kima",
	socialGroup = "kima",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 90,
	chanceHit = 2.5,
	damageMin = 940,
	damageMax = 1250,
	baseXp = 7940,
	baseHAM = 28495,
	baseHAMmax = 32283,
	armor = 1,
	resists = {165,165,155,155,145,135,150,145,-1},
	meatType = "meat_herbivore",
	meatAmount = 123,
	hideType = "hide_bristley",
	hideAmount = 123,
	boneType = "bone_mammal",
	boneAmount = 123,
	milk = 0,
	tamingChance = 0.05,
	ferocity = 0,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + KILLER + STALKER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/kima.iff"},
	controlDeviceTemplate = "object/intangible/pet/kima_hue.iff",
	scale = 1.1,
	lootGroups = {
		{ -- Death Watch
			groups =
			{
				{group = "kima_common", chance = 10000000},
			},
			lootChance = 17 * (100000),
		}
	},
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "unarmed",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = { {"blindattack",""},{"stunattack",""} },
	secondaryAttacks = { }

}

CreatureTemplates:addCreatureTemplate(wolf_kima, "wolf_kima")
