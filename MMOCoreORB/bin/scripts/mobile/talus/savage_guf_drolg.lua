savage_guf_drolg = Creature:new {
	objectName = "@mob/creature_names:savage_guf_drolg",
	socialGroup = "guf_drolg",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 80,
	chanceHit = 0.75,
	damageMin = 570,
	damageMax = 850,
	baseXp = 7040,
	baseHAM = 18000,
	baseHAMmax = 21000,
	armor = 1,
	-- {kinetic,energy,blast,heat,cold,electricity,acid,stun,ls}
	resists = {165,160,30,190,190,190,30,30,-1},
	meatType = "meat_reptilian",
	meatAmount = 550,
	hideType = "hide_leathery",
	hideAmount = 460,
	boneType = "bone_mammal",
	boneAmount = 320,
	milk = 2 * 0,
	tamingChance = 0,
	ferocity = 0,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/guf_drolg_hue.iff"},
	scale = 1.2,
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "unarmed",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = {	{"creatureareacombo",""}, {"posturedownattack",""}, {"creatureareaattack",""} },
	secondaryAttacks = { }

}

CreatureTemplates:addCreatureTemplate(savage_guf_drolg, "savage_guf_drolg")
