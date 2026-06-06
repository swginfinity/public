object_building_player_player_house_naboo_medium_style_01_dark = object_building_player_shared_player_house_naboo_medium_style_01_dark:new {
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
		{templateFile = "object/tangible/sign/player/house_address_naboo.iff", x = -0.5, z = 2.25, y = -11.5, ox = 0, oy = -0.7071068, oz = 0, ow = 0.7071068, cellid = -1, containmentType = -1},
		{templateFile = "object/tangible/terminal/terminal_player_structure.iff", x = 9.89, z = 1.1, y = -10.75, ox = 0, oy = 0, oz = 0, ow = 1, cellid = 1, containmentType = -1},
	},
	shopSigns = {
		{templateFile = "object/tangible/sign/player/house_address_naboo.iff", x = -0.5, z = 2.25, y = -11.5, ox = 0, oy = -0.7071068, oz = 0, ow = 0.707107, cellid = -1, containmentType = -1, requiredSkill = "", suiItem = "@player_structure:house_address"},
		{templateFile = "object/tangible/sign/player/shop_sign_s01.iff", x = 3.62, z = 0.5, y = -15.62, ox = 0, oy = 1, oz = 0, ow =  0, cellid = -1, containmentType = -1, requiredSkill = "crafting_merchant_management_01", suiItem = "@player_structure:shop_sign1"},
		{templateFile = "object/tangible/sign/player/shop_sign_s02.iff", x = 3.62, z = 0.5, y = -15.62, ox = 0, oy = 1, oz = 0, ow =  0, cellid = -1, containmentType = -1, requiredSkill = "crafting_merchant_management_02", suiItem = "@player_structure:shop_sign2"},
		{templateFile = "object/tangible/sign/player/shop_sign_s03.iff", x = 3.62, z = 0.5, y = -15.62, ox = 0, oy = 1, oz = 0, ow =  0, cellid = -1, containmentType = -1, requiredSkill = "crafting_merchant_management_03", suiItem = "@player_structure:shop_sign3"},
		{templateFile = "object/tangible/sign/player/shop_sign_s04.iff", x = 3.62, z = 0.5, y = -15.62, ox = 0, oy = 1, oz = 0, ow =  0, cellid = -1, containmentType = -1, requiredSkill = "crafting_merchant_management_04", suiItem = "@player_structure:shop_sign4"},
	},
	constructionMarker = "object/building/player/construction/construction_player_house_naboo_medium_style_01.iff",
	length = 3,
	width = 4
}
ObjectTemplates:addTemplate(object_building_player_player_house_naboo_medium_style_01_dark, "object/building/player/player_house_naboo_medium_style_01_dark.iff")
