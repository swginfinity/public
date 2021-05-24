acklay = Creature:new {
	objectName = "@mob/creature_names:geonosian_acklay_bunker_boss",
	customName = "Acklay",
	socialGroup = "geonosian_creature",
	pvpFaction = "",
	faction = "",
	level = 500,
	chanceHit = 25,
	damageMin = 3800,
	damageMax = 4800,
	baseXp = 49884,
	baseHAM = 1216000,
	baseHAMmax = 1258000,
	armor = 2,
	-- {kinetic,energy,blast,heat,cold,electricity,acid,stun,ls}
	resists = {40,45,55,55,45,45,40,40,25},
	meatType = "",
	meatAmount = 0,
	hideType = "",
	hideAmount = 0,
	boneType = "",
	boneAmount = 0,
	milk = 0,
	tamingChance = 0,
	ferocity = 25,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = PACK + KILLER,
	optionsBitmask = AIENABLED,
	diet = CARNIVORE,

	templates = {"object/mobile/acklay_hue.iff"},
	scale = 1.5,
	lootGroups = {
		{
	        groups =
			{
				{group = "armor_attachments", chance = 3500000},
				{group = "acklay", chance = 6500000}
			},
			lootChance = 4500000,
		},
		{
	        groups =
			{
				{group = "armor_attachments", chance = 3500000},
				{group = "acklay", chance = 6500000}
			},
			lootChance = 4500000,
		},
		{
	        groups =
			{
				{group = "armor_attachments", chance = 3500000},
				{group = "acklay", chance = 6500000}
			},
			lootChance = 4500000,
		},
		{
	        groups =
			{
				{group = "geonosian_hard", chance = 7500000},
				{group = "acklay", chance = 2500000}
			},
			lootChance = 3500000,
		},
		{ 	
			groups ={
					{group = "acklay", chance = 10000000},				
				},
				lootChance = 4500000,
		},
		{ 	
			groups ={
					{group = "jedi_clothing_attachments", chance = 10000000},				
				},
				lootChance = 1500000,
		},
		{
	        groups =
			{
				{group = "clothing_attachments", chance = 500000},
				{group = "acklay", chance = 9500000}
			},
			lootChance = 3500000,
		},
		{
	        groups =
			{
				{group = "geonosian_epic", chance = 10000000},
			},
			lootChance = 2500000,
		},
		{
	        groups =
			{
				{group = "geonosian_epic", chance = 10000000},
			},
			lootChance = 2500000,
		},
		{
			groups =
			{

				{group = "geonosian_common", chance = 10000000},
			},
			lootChance = 7500000,
		},
		{
			groups =
			{

				{group = "geonosian_common", chance = 10000000},
			},
			lootChance = 5500000,
		},
	},
	weapons = {},
	conversationTemplate = "",
	attacks = {
		{"creatureareacombo",""},
		{"posturedownattack","postureDownChance=75"}
	}
}

CreatureTemplates:addCreatureTemplate(acklay, "acklay")
