port = Creature:new {
	objectName = "",
	customName = "corellia Travel Port",
	socialGroup = "Townsperson",
	pvpFaction = "Townsperson",
	faction = "",
	level = 45,
	chanceHit = 0.47,
	damageMin = 395,
	damageMax = 500,
	baseXp = 4461,
	baseHAM = 9700,
	baseHAMmax = 11900,
	armor = 0,
	resists = {30,30,30,0,30,-1,30,-1,-1},
	meatType = "",
	meatAmount = 0,
	hideType = "",
	hideAmount = 0,
	boneType = "",
	boneAmount = 0,
	milk = 0,
	tamingChance = 0,
	ferocity = 0,
	pvpBitmask = NONE,
	creatureBitmask = PACK + HEALER,
	optionsBitmask = INVULNERABLE + INTERESTING,
	diet = HERBIVORE,

	templates = {"object/mobile/dressed_patron_chiss_m_01.iff"},
	lootGroups = {
	     {
			groups = {
				{group = "junk", chance = 6500000},
				{group = "nyms_common", chance = 1000000},
				{group = "pistols", chance = 1000000},
				{group = "carbines", chance = 1000000},
				{group = "tailor_components", chance = 500000}
			},
			lootChance = 3500000
		}
	},
	conversationTemplate = "",
	defaultWeapon = "object/weapon/ranged/droid/droid_droideka_ranged.iff",
	defaultAttack = "creaturerangedattack",
}

CreatureTemplates:addCreatureTemplate(port, "port")
