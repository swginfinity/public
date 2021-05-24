medical_bot = Creature: new {

	objectName = "",
	customName = "Infinity Medical Droid",
	socialGroup = "",
	faction = "",
	level = 1,
	chanceHit = 0.50,
	damageMin = 25,
	damageMax = 3000,
	baseXp = 20,
	armor = 0,
	baseHAM = 8400,
	baseHAMmax = 10200,
	resists = {100,100,100,100,100,100,100,100,100}, 
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
	creatureBitmask = NONE, 
	optionsBitmask = 264,
	diet = HERBIVORE, 
	templates = {
		"object/mobile/21b_surgical_droid.iff"
	},
	lootGroups = {},
	weapons = {},
	conversationTemplate = "medicalBotConvoTemplate",
	attacks = {}

} CreatureTemplates:addCreatureTemplate(medical_bot, "medical_bot")
