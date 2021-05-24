--Copyright (C) 2010 <SWGEmu>


--This File is part of Core3.



object_tangible_beta_beta_terminal_wound = object_tangible_beta_shared_beta_terminal_wound:new {
	gameObjectType = 16400,

	maxCondition = 0,
	customName = "New Player Services",

	templateType = CHARACTERBUILDERTERMINAL,

     suiBoxTitle = "New Player Services", 
     suiBoxText = "Welcome Infinitian! Please enjoy these helpful services for new players.",

	itemList = {
			"Starter Buffs [Fee: 4000cr]", "enhance_character",
			"Remove Buffs", "reset_buffs",
			"Apply DOTs", "apply_all_dots",
			"Clear DOTs", "clear_dots",
			"Skills",
			{
				"Learn Languages", "language",
				"Master Politician", "social_politician_master",
				"WARNING UNLEARN ALL SKILLS WARNING",
					{
					"Last chance to cancel, no undoing this!!", "unlearn_all_skills",
					}
			}
	}
}
ObjectTemplates:addTemplate(object_tangible_beta_beta_terminal_wound, "object/tangible/beta/beta_terminal_wound.iff")
