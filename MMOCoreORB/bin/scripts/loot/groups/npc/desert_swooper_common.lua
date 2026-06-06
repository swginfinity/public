desert_swooper_common = {
	description = "",
	minimumLevel = 0,
	maximumLevel = 0,
	lootItems = {
		{itemTemplate = "skill_buff_carbine_accuracy",		weight = 6.5 * (100000)},
		{itemTemplate = "skill_buff_carbine_speed",			weight = 6.5 * (100000)},
		{itemTemplate = "skill_buff_heavy_weapon_accuracy", weight = 6.5 * (100000)},
		{itemTemplate = "skill_buff_heavy_weapon_speed",	weight = 6.5 * (100000)},
		{itemTemplate = "skill_buff_melee_accuracy",		weight = 6.5 * (100000)},
		{itemTemplate = "skill_buff_melee_defense",			weight = 6.5 * (100000)},
		{itemTemplate = "skill_buff_pistol_accuracy",		weight = 6.5 * (100000)},
		{itemTemplate = "skill_buff_pistol_speed",			weight = 6.5 * (100000)},
		{itemTemplate = "skill_buff_ranged_accuracy",		weight = 6.5 * (100000)},
		{itemTemplate = "skill_buff_ranged_defense",		weight = 6.5 * (100000)},
		{itemTemplate = "skill_buff_thrown_accuracy",		weight = 6.5 * (100000)},
		{itemTemplate = "skill_buff_thrown_speed",			weight = 6.5 * (100000)},
		{itemTemplate = "skill_buff_twohandmelee_accuracy", weight = 6.5 * (100000)},
		{itemTemplate = "skill_buff_twohandmelee_speed",	weight = 6.5 * (100000)},
		{groupTemplate = "eta1_ship_group",					weight =   9 * (100000)},
	}
}

addLootGroupTemplate("desert_swooper_common", desert_swooper_common)
