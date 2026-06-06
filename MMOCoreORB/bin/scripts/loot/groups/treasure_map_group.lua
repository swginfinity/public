treasure_map_group = {
	description = "",
	minimumLevel = 0,
	maximumLevel = 0,
	lootItems = {
		{groupTemplate = "veteran_rewards_360_days",	weight =  .5 * (100000) },
		{itemTemplate = "armor_attachments",			weight =   5 * (100000) },
		{itemTemplate = "clothing_attachments",			weight =   4 * (100000) },
		{itemTemplate = "color_crystals",				weight =   5 * (100000) },
		{itemTemplate = "power_crystals",				weight =   3 * (100000) },
		{groupTemplate = "weapons_all",					weight =  28 * (100000) },
		{groupTemplate = "armor_all",					weight =  28 * (100000) },
		{groupTemplate = "lockedcontainer",				weight =   5 * (100000) },
		{groupTemplate = "holocron_dark",				weight = 2.5 * (100000) },
		{groupTemplate = "holocron_light",				weight = 2.5 * (100000) },
		{groupTemplate = "krayt_tissue_common",			weight = 4.5 * (100000) },
		{groupTemplate = "krayt_tissue_rare",			weight =  .6 * (100000) },
		{groupTemplate = "veteran_rewards_90_days",		weight = 5.8 * (100000) },
		{groupTemplate = "veteran_rewards_180_days",	weight =   1 * (100000) },
		{groupTemplate = "veteran_rewards_270_days",	weight =  .5 * (100000) },
		{groupTemplate = "landspeeder_ab1",				weight =  .1 * (100000) },
		{groupTemplate = "vintage_loot",				weight =   4 * (100000) },
	}
}

addLootGroupTemplate("treasure_map_group", treasure_map_group)
