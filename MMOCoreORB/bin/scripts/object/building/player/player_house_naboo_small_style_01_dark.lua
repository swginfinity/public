object_building_player_player_house_naboo_small_style_01_dark = object_building_player_shared_player_house_naboo_small_style_01_dark:new {
	lotSize = 1,
	baseMaintenanceRate = 16,
	allowedZones = {"dantooine", "lok", "tatooine", "naboo", "rori", "corellia", "talus"},
	publicStructure = 0,
	skillMods = {
		{"private_medical_rating", 100},
		{"private_buff_mind", 100},
		{"private_med_battle_fatigue", 5},
		{"private_safe_logout", 1}
	},
	childObjects = {
		{templateFile = "object/tangible/sign/player/house_address_naboo.iff", x = 1.67, z = 2.35, y = -8.38, ox = 0, oy = -0.1079993, oz = 0, ow = 0.9941509, cellid = -1, containmentType = -1},
		{templateFile = "object/tangible/terminal/terminal_player_structure.iff", x = 3.2253, z = 0.7, y = 5.51081, ox = 0, oy = -0.707107, oz = 0, ow = 0.707107, cellid = 1, containmentType = -1},
	},
	shopSigns = {
		{templateFile = "object/tangible/sign/player/house_address_naboo.iff", x = 1.67, z = 2.35, y = -8.38, ox = 0, oy = -0.1079993, oz = 0, ow = 0.9941509, cellid = -1, containmentType = -1, requiredSkill = "", suiItem = "@player_structure:house_address"},
		{templateFile = "object/tangible/sign/player/shop_sign_s01.iff", x = 5.09, z = 0.5, y = -7.22, ox = 0, oy = 1, oz = 0, ow =  0, cellid = -1, containmentType = -1, requiredSkill = "crafting_merchant_management_01", suiItem = "@player_structure:shop_sign1"},
		{templateFile = "object/tangible/sign/player/shop_sign_s02.iff", x = 5.09, z = 0.5, y = -7.22, ox = 0, oy = 1, oz = 0, ow =  0, cellid = -1, containmentType = -1, requiredSkill = "crafting_merchant_management_02", suiItem = "@player_structure:shop_sign2"},
		{templateFile = "object/tangible/sign/player/shop_sign_s03.iff", x = 5.09, z = 0.5, y = -7.22, ox = 0, oy = 1, oz = 0, ow =  0, cellid = -1, containmentType = -1, requiredSkill = "crafting_merchant_management_03", suiItem = "@player_structure:shop_sign3"},
		{templateFile = "object/tangible/sign/player/shop_sign_s04.iff", x = 5.09, z = 0.5, y = -7.22, ox = 0, oy = 1, oz = 0, ow =  0, cellid = -1, containmentType = -1, requiredSkill = "crafting_merchant_management_04", suiItem = "@player_structure:shop_sign4"},
	},
	constructionMarker = "object/building/player/construction/construction_player_house_naboo_small_style_01.iff",
	length = 4,
	width = 3
}
ObjectTemplates:addTemplate(object_building_player_player_house_naboo_small_style_01_dark, "object/building/player/player_house_naboo_small_style_01_dark.iff")
