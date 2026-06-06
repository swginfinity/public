hidden_dagger_common = {
	description = "",
	minimumLevel = 0,
	maximumLevel = 0,
	lootItems = {
		{itemTemplate = "spinner_blade",				weight = 25 * (100000)},
		{itemTemplate = "skill_buff_carbine_accuracy",	weight = 25 * (100000)},
		{itemTemplate = "skill_buff_carbine_speed",		weight = 20 * (100000)},
		{groupTemplate = "wearables_common",			weight = 20 * (100000)},
		{groupTemplate = "eta1_ship_group",				weight = 10 * (100000)},
	}
}

addLootGroupTemplate("hidden_dagger_common", hidden_dagger_common)
