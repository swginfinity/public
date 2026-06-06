plasma_thief_common = {
	description = "",
	minimumLevel = 0,
	maximumLevel = 0,
	lootItems = {
		{itemTemplate = "kitchen_utensils",		weight = 23 * (100000)},
		{groupTemplate = "wearables_common",	weight = 75 * (100000)},
		{groupTemplate = "eta1_ship_group",		weight =  2 * (100000)},
	}
}

addLootGroupTemplate("plasma_thief_common", plasma_thief_common)