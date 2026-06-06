woodland_kima = Creature:new {
	objectName = "@mob/creature_names:woodland_kima",
	socialGroup = "kima",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 80,
	chanceHit = 2.5,
	damageMin = 705,
	damageMax = 1250,
	baseXp = 6840,
	baseHAM = 22495,
	baseHAMmax = 25283,
	armor = 1,
	resists = {160,155,145,140,145,135,120,120,-1},
	meatType = "meat_carnivore",
	meatAmount = 123,
	hideType = "hide_bristley",
	hideAmount = 123,
	boneType = "bone_mammal",
	boneAmount = 123,
	milk = 0,
	tamingChance = 0.05,
	ferocity = 0,
	pvpBitmask = ATTACKABLE,
	creatureBitmask = PACK + KILLER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/kima.iff"},
	controlDeviceTemplate = "object/intangible/pet/kima_hue.iff",
	scale = 1.05,
	lootGroups = {
		{ -- Death Watch
			groups =
			{
				{group = "kima_common", chance = 10000000},
			},
			lootChance = 11 * (100000),
		}
	},
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "unarmed",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = { {"intimidationattack",""},{"blindattack",""}, },
	secondaryAttacks = { }

}

CreatureTemplates:addCreatureTemplate(woodland_kima, "woodland_kima")
