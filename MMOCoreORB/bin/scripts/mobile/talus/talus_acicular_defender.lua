talus_acicular_defender = Creature:new {
	objectName = "@mob/creature_names:paralope_acicular_defender",
	socialGroup = "paralope",
	mobType = MOB_HERBIVORE,
	faction = "",
	level = 64,
	chanceHit = 3.5,
	damageMin = 550,
	damageMax = 950,
	baseXp = 6140,
	baseHAM = 14590,
	baseHAMmax = 16642,
	armor = 1,
	resists = {135,25,40,-1,-1,-1,20,45,-1},
	meatType = "meat_herbivore",
	meatAmount = 123,
	hideType = "hide_wooly",
	hideAmount = 123,
	boneType = "bone_mammal",
	boneAmount = 123,
	milk = 2 * 0,
	tamingChance = 0.01,
	ferocity = 0,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + HERD,
	optionsBitmask = AIENABLED,
	diet = HERBIVORE,

	templates = {"object/mobile/paralope.iff"},
	controlDeviceTemplate = "object/intangible/pet/swirl_prong_hue.iff",
	scale = 1.2,
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "unarmed",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = {	{"blindattack",""},	{"creatureareacombo",""},},
	secondaryAttacks = { }

}

CreatureTemplates:addCreatureTemplate(talus_acicular_defender, "talus_acicular_defender")
