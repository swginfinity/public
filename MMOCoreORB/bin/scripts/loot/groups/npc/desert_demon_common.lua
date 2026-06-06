desert_demon_common = {
	description = "",
	minimumLevel = 0,
	maximumLevel = 0,
	lootItems = {
		{itemTemplate = "gambling_table_schematic",			weight =  15 * (100000)},
		{itemTemplate = "valley_view_painting",				weight =   5 * (100000)},
		{itemTemplate = "skill_buff_heavy_weapon_accuracy", weight = 7.5 * (100000)},
		{itemTemplate = "skill_buff_heavy_weapon_speed",	weight = 7.5 * (100000)},
		{itemTemplate = "skill_buff_melee_accuracy",		weight = 7.5 * (100000)},
		{itemTemplate = "skill_buff_melee_defense",			weight = 7.5 * (100000)},
		{itemTemplate = "skill_buff_pistol_accuracy",		weight = 7.5 * (100000)},
		{itemTemplate = "skill_buff_pistol_speed",			weight = 7.5 * (100000)},
		{itemTemplate = "skill_buff_ranged_accuracy",		weight = 7.5 * (100000)},
		{itemTemplate = "skill_buff_ranged_defense",		weight = 7.5 * (100000)},
		{itemTemplate = "skill_buff_thrown_accuracy",		weight = 7.5 * (100000)},
		{itemTemplate = "skill_buff_thrown_speed",			weight = 7.5 * (100000)},
		{groupTemplate = "eta1_ship_group",					weight =   5 * (100000)},
	}
}

addLootGroupTemplate("desert_demon_common", desert_demon_common)
