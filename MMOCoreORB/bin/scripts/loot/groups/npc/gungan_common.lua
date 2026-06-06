gungan_common = {
	description = "",
	minimumLevel = 0,
	maximumLevel = -1,
	lootItems = {
		{itemTemplate = "gungan_signet",		weight = 30 * (100000)},
		{itemTemplate = "gungan_lance",			weight = 30 * (100000)},
		{groupTemplate = "wearables_common",	weight = 20 * (100000)},
		{groupTemplate = "eta1_ship_group",		weight = 20 * (100000)},
	}
}

addLootGroupTemplate("gungan_common", gungan_common)
