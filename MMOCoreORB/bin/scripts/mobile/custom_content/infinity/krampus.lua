krampus = Creature:new {
    customName = "krampus",
	socialGroup = "townsperson",
	faction = "",
	mobType = MOB_NPC,
	level = 500,
	chanceHit = 2.75,
	damageMin = 1145,
	damageMax = 1766,
	baseXp = 3921,
	baseHAM = 3250000,
	baseHAMmax = 4250000,
	armor = 1,
	-- {kinetic,energy,blast,heat,cold,electricity,acid,stun,ls}
	resists = {25,25,25,25,25,25,25,25,50},
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
	creatureBitmask = PACK,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,
	scale = 1.25,

	templates = {"object/mobile/coa_aclo_slicer_devaronian.iff"},

	lootGroups = {
		{ -- Xmas Crystals
			groups = {
				{group = "krampus", chance = 100 * 100000},
			},
			lootChance = 100 * 100000,
    	},
	},

	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "unarmed",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = { 	{"creatureareacombo","stateAccuracyBonus=100"},	{"creatureareaknockdown","stateAccuracyBonus=100"},	{"creatureareableeding",""}, },
	secondaryAttacks = { }
}
CreatureTemplates:addCreatureTemplate(krampus, "krampus")
