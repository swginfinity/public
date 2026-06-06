valarian_common = {
	description = "",
	minimumLevel = 0,
	maximumLevel = 0,
	lootItems = {
		{itemTemplate = "party_poster",					weight = 25 * (100000)},
		{itemTemplate = "valley_view_painting",			weight = 25 * (100000)},
		{itemTemplate = "skill_buff_carbine_accuracy",	weight = 25 * (100000)},
		{itemTemplate = "skill_buff_carbine_speed",		weight = 25 * (100000)},
		--{itemTemplate = "house_move_schem",				weight =  3 * (100000)},
	}
}

addLootGroupTemplate("valarian_common", valarian_common)
