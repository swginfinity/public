canyon_corsair_common = {
	description = "",
	minimumLevel = 0,
	maximumLevel = 0,
	lootItems = {
		{itemTemplate = "small_motor",					weight = 15 * (100000)},
		{itemTemplate = "metal_recycler_schematic",		weight = 15 * (100000)},
		{itemTemplate = "agitator_motor_schematic",		weight = 35 * (100000)},
		{itemTemplate = "skill_buff_rifle_accuracy",	weight = 10 * (100000)},
		{itemTemplate = "skill_buff_rifle_speed",		weight = 10 * (100000)},
		{groupTemplate = "wearables_common",			weight = 15 * (100000)},
	}
}

addLootGroupTemplate("canyon_corsair_common", canyon_corsair_common)
