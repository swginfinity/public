imperial_officer_common = {
	description = "",
	minimumLevel = 0,
	maximumLevel = 0,
	lootItems = {
		{itemTemplate = "technical_console_schematic_1",	weight = 25 * (100000)},
		{itemTemplate = "technical_console_schematic_2",	weight = 25 * (100000)},
		{itemTemplate = "couch_blue_schematic",				weight = 25 * (100000)},
		{itemTemplate = "painting_vader_victory",			weight = 25 * (100000)},
		--{itemTemplate = "house_move_schem",				weight =  1 * (100000)},
	}
}

addLootGroupTemplate("imperial_officer_common", imperial_officer_common)
