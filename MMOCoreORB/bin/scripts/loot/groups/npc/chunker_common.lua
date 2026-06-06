chunker_common = {
	description = "",
	minimumLevel = 0,
	maximumLevel = 0,
	lootItems = {
		{itemTemplate = "red_wiring",				weight = 28 * (100000)},
		{itemTemplate = "pulverizer",				weight = 37 * (100000)},
		{itemTemplate = "processor_attachment",		weight =  5 * (100000)},
		{groupTemplate = "wearables_common",		weight = 30 * (100000)},
	}
}
addLootGroupTemplate("chunker_common", chunker_common)
