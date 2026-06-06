mauler_common = {
	description = "",
	minimumLevel = 0,
	maximumLevel = 0,
	lootItems = {
		{itemTemplate = "portable_stove_schematic",			weight = 25 * (100000)},
		{itemTemplate = "skill_buff_onehandmelee_accuracy", weight = 25 * (100000)},
		{itemTemplate = "skill_buff_onehandmelee_speed",	weight = 25 * (100000)},
		{groupTemplate = "wearables_common",				weight = 23 * (100000)},
		{groupTemplate = "eta1_ship_group",					weight =  2 * (100000)},
	}
}

addLootGroupTemplate("mauler_common", mauler_common)
