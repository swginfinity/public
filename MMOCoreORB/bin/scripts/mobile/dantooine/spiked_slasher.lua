spiked_slasher = Creature:new {
	objectName = "@mob/creature_names:voritor_spiked_slasher",
	socialGroup = "voritor",
	faction = "",
	level = 95,
	chanceHit = 1.65,
	damageMin = 900,
	damageMax = 1490,
	baseXp = 9057,
	baseHAM = 32400,
	baseHAMmax = 36000,
	armor = 1,
	-- {kinetic,energy,blast,heat,cold,electricity,acid,stun,ls
	resists = {180,180,175,150,130,165,190,160,135},
	meatType = "meat_carnivore",
	meatAmount = 400,
	hideType = "hide_leathery",
	hideAmount = 400,
	boneType = "bone_avian",
	boneAmount = 400,
	milk = 2 * 0,
	tamingChance = 0.05,
	ferocity = 11,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + KILLER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/voritor_lizard_hue.iff"},
	controlDeviceTemplate = "object/intangible/pet/spiked_slasher_hue.iff",
	scale = 1.3,
	lootGroups = {
		{
			groups = {
				{group = "voritor_lizard_common", chance = 10000000}
			},
			lootChance = 2160000
		}
	},
	weapons = {},
	conversationTemplate = "",
	attacks = {
		{"creatureareapoison",""},
		{"strongdisease",""}
	}
}

CreatureTemplates:addCreatureTemplate(spiked_slasher, "spiked_slasher")
