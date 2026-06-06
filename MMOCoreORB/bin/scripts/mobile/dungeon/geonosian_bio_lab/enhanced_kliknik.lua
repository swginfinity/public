enhanced_kliknik = Creature:new {
	objectName = "@mob/creature_names:geonosian_kliknik_force_strong",
	socialGroup = "geonosian_creature",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 250,
	chanceHit = 19.2,
	damageMin = 575,
	damageMax = 860,
	baseXp = 10360,
	baseHAM = 42000,
	baseHAMmax = 67000,
	armor = 2,
	resists = {40,40,25,95,35,40,25,25,-1},
	meatType = "meat_carnivore",
	meatAmount = 45,
	hideType = "hide_scaley",
	hideAmount = 40,
	boneType = "",
	boneAmount = 0,
	milk = 0,
	tamingChance = 0,
	ferocity = 15,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + KILLER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/kliknik.iff"},
	scale = 1.4,
	lootGroups = {
	 	{
	        groups =
			{

				{group = "geonosian_relic", chance = 3500000},
				{group = "geonosian_common", chance = 5000000},
				{group = "mastery_geocave_jewelry", chance = 1500000},
			},
			lootChance = 1500000,
		},
		{
			groups = 
			{
				{group = "pistols", chance = 2500000},
				{group = "rifles", chance = 2500000},
				{group = "carbines", chance = 2500000},
				{group = "melee_weapons", chance = 2500000},
			},
			lootChance = 1500000,
		},
		{
			groups =
			{
				{group = "armor_attachments", chance = 10000000}, -- 5%
			},
			lootChance = 500000,
		},

	},

	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "object/weapon/ranged/creature/creature_spit_heavy_flame.iff",
	secondaryWeapon = "unarmed",
	conversationTemplate = "",

	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = { {"stunattack",""}, {"creatureareaattack",""} },
	secondaryAttacks = { {"stunattack",""}, {"creatureareaattack",""} }
}

CreatureTemplates:addCreatureTemplate(enhanced_kliknik, "enhanced_kliknik")
