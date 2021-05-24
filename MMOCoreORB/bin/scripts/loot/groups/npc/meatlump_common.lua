meatlump_common = {
	description = "",
	minimumLevel = 0,
	maximumLevel = 0,
	lootItems = {
		{itemTemplate = "cheap_copper_battery", weight = 1000000},
		{groupTemplate = "wearables_common", weight = 1800000},
		{itemTemplate = "droid_body_schematic", weight = 800000},
		{itemTemplate = "radio_schematic", weight = 1200000},
		{itemTemplate = "streetlamp_schematic", weight = 800000},
		{groupTemplate = "clothing_attachments", weight = 400000},
		{itemTemplate = "skill_buff_carbine_speed", weight = 400000},
		{itemTemplate = "force_power_crystal", weight = 400000},
		{groupTemplate = "meatlump_melee_components", weight = 400000},
		--{itemTemplate = "skill_buff_melee_accuracy", weight = 400000},
		--{itemTemplate = "skill_buff_melee_defense", weight = 400000},
		{itemTemplate = "skill_buff_pistol_accuracy", weight = 400000},
		{itemTemplate = "skill_buff_pistol_speed", weight = 400000},
		{itemTemplate = "skill_buff_ranged_accuracy", weight = 400000},
		--{itemTemplate = "skill_buff_ranged_defense", weight = 400000},
		{itemTemplate = "skill_buff_thrown_accuracy", weight = 400000},
		{groupTemplate = "meatlump_ranged_components", weight = 400000},
		{itemTemplate = "skill_buff_twohandmelee_accuracy", weight = 400000},
		{itemTemplate = "skill_buff_twohandmelee_speed", weight = 400000}
	}
}

addLootGroupTemplate("meatlump_common", meatlump_common)
