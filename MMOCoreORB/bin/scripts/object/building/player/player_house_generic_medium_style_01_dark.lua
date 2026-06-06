object_building_player_player_house_generic_medium_style_01_dark = object_building_player_shared_player_house_generic_medium_style_01_dark:new {
	lotSize = 2,
	baseMaintenanceRate = 34,
	allowedZones = {"dantooine", "lok", "tatooine", "naboo", "rori", "corellia", "talus"},
	publicStructure = 0,
	skillMods = {
		{"private_medical_rating", 100},
		{"private_buff_mind", 100},
		{"private_med_battle_fatigue", 5},
		{"private_safe_logout", 1}
	},
	childObjects = {
		{templateFile = "object/tangible/terminal/terminal_player_structure.iff", x = -10.0961, z = 0.625, y = 3.48201, ox = 0, oy = 0.707107, oz = 0, ow = 0.707107, cellid = 2, containmentType = -1},
		{templateFile = "object/tangible/sign/player/house_address.iff", x = 5.13, z = 2.25, y = 7.91, ox = 0, oy = 1, oz = 0, ow = 0, cellid = -1, containmentType = -1}
	},
	shopSigns = {
		{templateFile = "object/tangible/sign/player/house_address.iff", x = 5.13, z = 2.25, y = 7.91, ox = 0, oy = 1, oz = 0, ow = 0, cellid = -1, containmentType = -1, requiredSkill = "", suiItem = "@player_structure:house_address"},
		{templateFile = "object/tangible/sign/player/shop_sign_s01.iff", x = 11.79, z = 0.5, y = 11.64, ox = 0, oy = 0, oz = 0, ow =  1, cellid = -1, containmentType = -1, requiredSkill = "crafting_merchant_management_01", suiItem = "@player_structure:shop_sign1"},
		{templateFile = "object/tangible/sign/player/shop_sign_s02.iff", x = 11.79, z = 0.5, y = 11.64, ox = 0, oy = 0, oz = 0, ow =  1, cellid = -1, containmentType = -1, requiredSkill = "crafting_merchant_management_02", suiItem = "@player_structure:shop_sign2"},
		{templateFile = "object/tangible/sign/player/shop_sign_s03.iff", x = 11.79, z = 0.5, y = 11.64, ox = 0, oy = 0, oz = 0, ow =  1, cellid = -1, containmentType = -1, requiredSkill = "crafting_merchant_management_03", suiItem = "@player_structure:shop_sign3"},
		{templateFile = "object/tangible/sign/player/shop_sign_s04.iff", x = 11.79, z = 0.5, y = 11.64, ox = 0, oy = 0, oz = 0, ow =  1, cellid = -1, containmentType = -1, requiredSkill = "crafting_merchant_management_04", suiItem = "@player_structure:shop_sign4"},
	},
	constructionMarker = "object/building/player/construction/construction_player_house_corellia_medium_style_01.iff",
	length = 3,
	width = 4
}
ObjectTemplates:addTemplate(object_building_player_player_house_generic_medium_style_01_dark, "object/building/player/player_house_generic_medium_style_01_dark.iff")
