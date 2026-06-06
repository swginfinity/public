object_building_player_player_guildhall_generic_style_01_dark = object_building_player_shared_player_guildhall_generic_style_01_dark:new {
	lotSize = 5,
	baseMaintenanceRate = 50,
	allowedZones = {"dantooine", "lok", "tatooine", "naboo", "rori", "corellia", "talus"},
	publicStructure = 0,
	constructionMarker = "object/building/player/construction/construction_player_guildhall_corellia_style_01.iff",
	length = 7,
	width = 8,
	skillMods = {
		{"private_medical_rating", 100},
		{"private_buff_mind", 100},
		{"private_med_battle_fatigue", 5},
		{"private_safe_logout", 1}
	},
	childObjects = {
		{templateFile = "object/tangible/sign/player/house_address_corellia.iff", x = 4.5, z = 1, y = 22.1, ox = 0, oy = 0.707107, oz = 0, ow = 0.707107, cellid = -1, containmentType = -1},
		{templateFile = "object/tangible/terminal/terminal_player_structure.iff", x = 17.88, z= 2.75, y = 10.45, ox = 0, oy = 0.707107, oz = 0, ow = 0.707107, cellid = 7, containmentType = -1},
		{templateFile = "object/tangible/terminal/terminal_elevator_up.iff", x = 0, z = -8.5, y = 13.5, ow = 0, ox = 0, oy = 1, oz = 0, cellid = 9, containmentType = -1},
		{templateFile = "object/tangible/terminal/terminal_elevator_down.iff", x = 0, z = 2.75, y = 13.5, ow = 0, ox = 0, oy = 1, oz = 0, cellid = 9, containmentType = -1},
		{templateFile = "object/tangible/terminal/terminal_guild.iff", x = -17.0, z = 2.75, y = 20, ow = 0, ox = 0, oy = 1, oz = 0, cellid = 6, containmentType = -1}
	},
	shopSigns = {
		{templateFile = "object/tangible/sign/player/house_address_corellia.iff", x = 4.5, z = 1, y = 22.1, ox = 0, oy = 0.707107, oz = 0, ow = 0.707107, cellid = -1, containmentType = -1, requiredSkill = "", suiItem = "@player_structure:house_address"},
		{templateFile = "object/tangible/sign/player/shop_sign_s01.iff", x = 9.1, z = 0.49, y = 22.25, ox = 0, oy = 0, oz = 0, ow = 1, cellid = -1, containmentType = -1, requiredSkill = "crafting_merchant_management_01", suiItem = "@player_structure:shop_sign1"},
		{templateFile = "object/tangible/sign/player/shop_sign_s02.iff", x = 9.1, z = 0.49, y = 22.25, ox = 0, oy = 0, oz = 0, ow = 1, cellid = -1, containmentType = -1, requiredSkill = "crafting_merchant_management_02", suiItem = "@player_structure:shop_sign2"},
		{templateFile = "object/tangible/sign/player/shop_sign_s03.iff", x = 9.1, z = 0.49, y = 22.25, ox = 0, oy = 0, oz = 0, ow = 1, cellid = -1, containmentType = -1, requiredSkill = "crafting_merchant_management_03", suiItem = "@player_structure:shop_sign3"},
		{templateFile = "object/tangible/sign/player/shop_sign_s04.iff", x = 9.1, z = 0.49, y = 22.25, ox = 0, oy = 0, oz = 0, ow = 1, cellid = -1, containmentType = -1, requiredSkill = "crafting_merchant_management_04", suiItem = "@player_structure:shop_sign4"},
	},
}

ObjectTemplates:addTemplate(object_building_player_player_guildhall_generic_style_01_dark, "object/building/player/player_guildhall_generic_style_01_dark.iff")
