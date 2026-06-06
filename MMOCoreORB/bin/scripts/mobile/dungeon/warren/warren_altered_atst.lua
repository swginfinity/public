warren_altered_atst = Creature:new {
	objectName = "@mob/creature_names:warren_agro_droid_atst",
	socialGroup = "warren_imperial",
	faction = "",
	mobType = MOB_ANDROID,
	level = 46,
	chanceHit = 0.36,
	damageMin = 1400,
	damageMax = 1600,
	baseXp = 4000,
	baseHAM = 90000,
	baseHAMmax = 115000,
	armor = 0,
	resists = {30,30,40,40,-1,40,-1,-1,-1},
	meatType = "",
	meatAmount = 0,
	hideType = "",
	hideAmount = 0,
	boneType = "",
	boneAmount = 0,
	milk = 0,
	tamingChance = 0,
	ferocity = 0,
	pvpBitmask = ATTACKABLE + AGGRESSIVE + ENEMY,
	creatureBitmask = PACK + KILLER + NOKNOCKDOWN + NODOT,
	optionsBitmask = AIENABLED,
	diet = HERBIVORE,

	templates = {"object/mobile/atst.iff"},
	lootGroups = {},
	conversationTemplate = "",
	primaryWeapon = "object/weapon/ranged/droid/droid_droideka_ranged.iff",
	secondaryWeapon = "none",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = { {"creaturerangedattack",""} },
	secondaryAttacks = { },
	defaultAttack = "attack",
	defaultWeapon = "object/weapon/ranged/droid/droid_droideka_ranged.iff"
}

CreatureTemplates:addCreatureTemplate(warren_altered_atst, "warren_altered_atst")
