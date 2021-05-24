treasure_map_group = {
	description = "",
	minimumLevel = 0,
	maximumLevel = 0,
	lootItems = {
		{groupTemplate = "veteran_rewards_360_days", weight = 50000 },
		{itemTemplate = "armor_attachments", weight = 500000 },
		{itemTemplate = "clothing_attachments", weight = 400000 },
		{itemTemplate = "color_crystals", weight = 500000 },
		{itemTemplate = "power_crystals", weight = 300000 },
		{groupTemplate = "weapons_all", weight = 3000000},
		{groupTemplate = "armor_all", weight = 3000000},
		{groupTemplate = "lockedcontainer", weight = 500000},
		{groupTemplate = "holocron_dark", weight = 250000 },
		{groupTemplate = "holocron_light", weight = 250000 },
		{groupTemplate = "krayt_tissue_common", weight = 450000 },
		{groupTemplate = "krayt_tissue_rare", weight = 60000 },
		{groupTemplate = "veteran_rewards_90_days", weight = 580000 },
		{groupTemplate = "veteran_rewards_180_days", weight = 100000 },
		{groupTemplate = "veteran_rewards_270_days", weight = 50000},
		{groupTemplate = "landspeeder_ab1", weight = 10000},
	}
}

addLootGroupTemplate("treasure_map_group", treasure_map_group)
