hooded_crystal_snake = Creature:new {
	objectName = "@mob/creature_names:crystal_snake_hooded",
	socialGroup = "snake",
	faction = "",
	level = 95,
	chanceHit = 1.7,
	damageMin = 975,
	damageMax = 1500,
	baseXp = 9057,
	baseHAM = 32400,
	baseHAMmax = 36000,
	armor = 1,
	-- {kinetic,energy,blast,heat,cold,electricity,acid,stun,ls}
	resists = {180,180,180,140,140,160,180,160,135},
	meatType = "meat_carnivore",
	meatAmount = 123,
	hideType = "hide_scaley",
	hideAmount = 123,
	boneType = "",
	boneAmount = 0,
	milk = 0,
	tamingChance = 0.02,
	ferocity = 11,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/crystal_snake.iff"},
	controlDeviceTemplate = "object/intangible/pet/crystal_snake.iff",
	scale = 1.3,
	lootGroups = {},
	weapons = {"creature_spit_small_toxicgreen"},
	conversationTemplate = "",
	attacks = {
		{"creatureareacombo",""},
		{"strongpoison",""}
	},
	lootGroups = {
		{
			groups = {
				{group = "small_generic_windowed_house_s02_group", chance = 10000000}
			},
			lootChance = 350000 -- 3.5% chance for this loot group.   It should be approximately 10.1% chance for group of 3 boss mobs.
		}
	},
}

CreatureTemplates:addCreatureTemplate(hooded_crystal_snake, "hooded_crystal_snake")
