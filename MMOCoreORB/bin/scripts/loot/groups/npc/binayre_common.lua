binayre_common = {
	description = "",
	minimumLevel = 0,
	maximumLevel = 0,
	lootItems = {
		{itemTemplate = "heating_element",					weight = 20 * (100000) },
		{itemTemplate = "blue_wiring",						weight = 20 * (100000) },
		{groupTemplate = "wearables_common",				weight = 5 * (100000) },
		{itemTemplate = "skill_buff_polearm_accuracy",		weight = 10 * (100000) },
		{itemTemplate = "skill_buff_polearm_speed",			weight = 10 * (100000) },		
		{itemTemplate = "skill_buff_twohandmelee_accuracy", weight = 10 * (100000) },
		{itemTemplate = "skill_buff_twohandmelee_speed",	weight = 10 * (100000) },
		{itemTemplate = "art_certainpov1",					weight = 15 * (100000) },
	}
}

addLootGroupTemplate("binayre_common", binayre_common)
