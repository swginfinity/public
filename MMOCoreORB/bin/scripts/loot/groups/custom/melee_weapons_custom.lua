melee_weapons_custom = {
	description = "",
	minimumLevel = 0,
	maximumLevel = -1,
	lootItems = {
		{itemTemplate = "two_handed_sword_sith", 	weight = 25 * (100000)},
		{itemTemplate = "blasterfist", 				weight = 25 * (100000)},
		{itemTemplate = "lance_cryo", 				weight = 25 * (100000)},
		{itemTemplate = "sword_acid", 				weight = 25 * (100000)},
		--{itemTemplate = "mandoviol_smasher", weight = 2000000},
	}
}

addLootGroupTemplate("melee_weapons_custom", melee_weapons_custom)
