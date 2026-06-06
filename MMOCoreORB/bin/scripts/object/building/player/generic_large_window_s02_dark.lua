object_building_player_generic_large_window_s02_dark = object_building_player_shared_generic_large_window_s02_dark:new {
	lotSize = 5,
	baseMaintenanceRate = 50,
	allowedZones = {"dantooine", "lok", "tatooine", "naboo", "rori", "corellia", "talus"},
	publicStructure = 0,
	skillMods = {
		{"private_medical_rating", 100},
		{"private_buff_mind", 100},
		{"private_med_battle_fatigue", 5},
		{"private_safe_logout", 1}
	},
	childObjects = {
		{templateFile = "object/tangible/terminal/terminal_player_structure.iff", x = -0.335338, z = 0.518523, y = 8.77767, ox = 0, oy = 0.707107, oz = 0, ow = -0.707107, cellid = 2, containmentType = -1},
		{templateFile = "object/tangible/sign/player/house_address.iff", x = 9.98, z = 3.29, y = -11.16, ox = 0, oy = 0, oz = 0, ow = 1, cellid = -1, containmentType = -1}
	},
	shopSigns = {
		{templateFile = "object/tangible/sign/player/shared_house_address.iff", x = 9.98, z = 3.29, y = -11.16, ox = 0, oy = 0, oz = 0, ow = 1, cellid = -1, containmentType = -1,requiredSkill = "", suiItem = "@player_structure:house_address"},
		{templateFile = "object/tangible/sign/player/shop_sign_s01.iff", x = 11.35, z = 0.52, y = -15.75, ox = 0, oy = 0.25038, oz = 0, ow = 0.9681476, cellid = -1, containmentType = -1, requiredSkill = "crafting_merchant_management_01", suiItem = "@player_structure:shop_sign1"},
		{templateFile = "object/tangible/sign/player/shop_sign_s02.iff", x = 11.35, z = 0.52, y = -15.75, ox = 0, oy = 1, oz = 0, ow = 0, cellid = -1, containmentType = -1, requiredSkill = "crafting_merchant_management_02", suiItem = "@player_structure:shop_sign2"},
		{templateFile = "object/tangible/sign/player/shop_sign_s03.iff", x = 11.35, z = 0.52, y = -15.75, ox = 0, oy = 0, oz = 0, ow = 1, cellid = -1, containmentType = -1, requiredSkill = "crafting_merchant_management_03", suiItem = "@player_structure:shop_sign3"},
		{templateFile = "object/tangible/sign/player/shop_sign_s04.iff", x = 11.35, z = 0.52, y = -15.75, ox = 0, oy = 0, oz = 0, ow = 1, cellid = -1, containmentType = -1, requiredSkill = "crafting_merchant_management_04", suiItem = "@player_structure:shop_sign4"},
	},
	constructionMarker = "object/building/player/construction/construction_player_house_corellia_large_style_01.iff",
	length = 5,
	width = 5
}
ObjectTemplates:addTemplate(object_building_player_generic_large_window_s02_dark, "object/building/player/generic_large_window_s02_dark.iff")
