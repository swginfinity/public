bark_mite_burrower_king = Creature:new {
	objectName = "@mob/creature_names:bark_mite_burrower_king",
	socialGroup = "mite",
	faction = "",
	mobType = MOB_CARNIVORE,
	level = 95,
	chanceHit = 1.7,
	damageMin = 900,
	damageMax = 1800,
	baseXp = 9057,
	baseHAM = 32400,
	baseHAMmax = 36000,
	armor = 1,
	-- {kinetic,energy,blast,heat,cold,electricity,acid,stun,ls}
	resists = {185,185,175,180,175,165,175,180,135},
	meatType = "meat_insect",
	meatAmount = 400,
	hideType = "",
	hideAmount = 0,
	boneType = "",
	boneAmount = 0,
	milk = 0,
	tamingChance = 0.20,
	ferocity = 11,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + KILLER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/bark_mite_hue.iff"},
	controlDeviceTemplate = "object/intangible/pet/bark_mite_hue.iff",
	scale = 1.3,
	lootGroups = {},

	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "object/weapon/ranged/creature/creature_spit_spray_red.iff",
	secondaryWeapon = "unarmed",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = { {"strongdisease",""}, {"creatureareadisease",""} },
	secondaryAttacks = { {"strongdisease",""}, {"creatureareadisease",""} }

}
CreatureTemplates:addCreatureTemplate(bark_mite_burrower_king, "bark_mite_burrower_king")
