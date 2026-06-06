bloodrazor_common = {
	description = "",
	minimumLevel = 0,
	maximumLevel = 0,
	lootItems = {
		{groupTemplate = "wordle_group",			weight = 20 * (100000)},
		{groupTemplate = "wearables_common",		weight = 70 * (100000)},
		{itemTemplate = "flora_recycler_schematic", weight = 10 * (100000)},
	}
}

addLootGroupTemplate("bloodrazor_common", bloodrazor_common)
