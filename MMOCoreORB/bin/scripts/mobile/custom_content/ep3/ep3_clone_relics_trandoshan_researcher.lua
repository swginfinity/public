ep3_clone_relics_trandoshan_researcher = Creature:new {
	customName = "Trandoshan Researcher",
	randomNameType = NAME_GENERIC_TAG,
	socialGroup = "trandoshan",
	faction = "",
	level = 30,
	chanceHit = 0.33,
	damageMin = 180,
	damageMax = 190,
	baseXp = 1609,
	baseHAM = 4500,
	baseHAMmax = 5500,
	armor = 1,
	resists = {10,10,10,10,10,10,10,-1,-1},
	meatType = "",
	meatAmount = 0,
	hideType = "",
	hideAmount = 0,
	boneType = "",
	boneAmount = 0,
	milk = 0,
	tamingChance = 0,
	ferocity = 0,
	pvpBitmask = ATTACKABLE,
	creatureBitmask = NONE,
	optionsBitmask = AIENABLED,
	diet = HERBIVORE,

	templates = {
				"object/mobile/ep3/ep3_clone_relics_trandoshan_researcher_01.iff",
				"object/mobile/ep3/ep3_clone_relics_trandoshan_researcher_02.iff",
				"object/mobile/ep3/ep3_clone_relics_trandoshan_researcher_03.iff",
				"object/mobile/ep3/ep3_clone_relics_trandoshan_researcher_04.iff",
				"object/mobile/ep3/ep3_clone_relics_trandoshan_researcher_05.iff",
				"object/mobile/ep3/ep3_clone_relics_trandoshan_researcher_06.iff",
				"object/mobile/ep3/ep3_clone_relics_trandoshan_researcher_07.iff",
				"object/mobile/ep3/ep3_clone_relics_trandoshan_researcher_08.iff"
				},
				
	lootGroups = {},
	weapons = {"blood_razer_weapons"},
	conversationTemplate = "",
	reactionStf = "@npc_reaction/slang",
	attacks = merge(brawlermaster,marksmanmaster)
}

CreatureTemplates:addCreatureTemplate(ep3_clone_relics_trandoshan_researcher, "ep3_clone_relics_trandoshan_researcher")
