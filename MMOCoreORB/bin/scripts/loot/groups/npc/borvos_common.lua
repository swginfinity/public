borvos_common = {
	description = "",
	minimumLevel = 0,
	maximumLevel = 0,
	lootItems = {
		{itemTemplate = "carved_bowl_schematic",	weight = 12.5 * (100000)},
		{itemTemplate = "closed_basket_schematic",	weight = 12.5 * (100000)},
		{itemTemplate = "fat_bottle_schematic",		weight = 12.5 * (100000)},
		{itemTemplate = "pear_bottle_schematic",	weight = 12.5 * (100000)},
		{itemTemplate = "plain_bowl_schematic",		weight = 12.5 * (100000)},
		{itemTemplate = "tall_bottle_schematic",	weight = 12.5 * (100000)},
		{itemTemplate = "throw_pillow_schematic",	weight = 12.5 * (100000)},
		{groupTemplate = "wearables_common",		weight =  7.5 * (100000)},
		{groupTemplate = "eta1_ship_group",			weight =    5 * (100000)},
	}
}

addLootGroupTemplate("borvos_common", borvos_common)
