object_building_player_player_house_tatooine_small_style_02_dark = object_building_player_shared_player_house_tatooine_small_style_02_dark:new {
	lotSize = 2,
	publicStructure = 0,
	baseMaintenanceRate = 16,
	allowedZones = {"dantooine", "lok", "tatooine", "naboo", "rori", "corellia", "talus"},
	skillMods = {
		{"private_medical_rating", 100},
		{"private_buff_mind", 100},
		{"private_med_battle_fatigue", 5},
		{"private_safe_logout", 1}
	},
	childObjects = {
		{templateFile = "object/tangible/sign/player/house_address_tatooine.iff", x = 4.2, z = 2.25, y = 7.89, ox = 0, oy = -1, oz = 0, ow = 0, cellid = -1, containmentType = -1},
		{templateFile = "object/tangible/terminal/terminal_player_structure.iff", x = 3.79, z = 0.75, y = 6.24875, ox = 0, oy = 1, oz = 0, ow = 0, cellid = 1, containmentType = -1},
	},
	shopSigns = {
		{templateFile = "object/tangible/sign/player/house_address_tatooine.iff", x = 4.2, z = 2.25, y = 7.89, ox = 0, oy = -1, oz = 0, ow = 0, cellid = -1, containmentType = -1, requiredSkill = "", suiItem = "@player_structure:house_address"},
		{templateFile = "object/tangible/sign/player/shop_sign_s01.iff", x = 3.92, z = 0.5, y = 11.65, ox = 0, oy = 0, oz = 0, ow =  1, cellid = -1, containmentType = -1, requiredSkill = "crafting_merchant_management_01", suiItem = "@player_structure:shop_sign1"},
		{templateFile = "object/tangible/sign/player/shop_sign_s02.iff", x = 3.92, z = 0.5, y = 11.65, ox = 0, oy = 0, oz = 0, ow =  1, cellid = -1, containmentType = -1, requiredSkill = "crafting_merchant_management_02", suiItem = "@player_structure:shop_sign2"},
		{templateFile = "object/tangible/sign/player/shop_sign_s03.iff", x = 3.92, z = 0.5, y = 11.65, ox = 0, oy = 0, oz = 0, ow =  1, cellid = -1, containmentType = -1, requiredSkill = "crafting_merchant_management_03", suiItem = "@player_structure:shop_sign3"},
		{templateFile = "object/tangible/sign/player/shop_sign_s04.iff", x = 3.92, z = 0.5, y = 11.65, ox = 0, oy = 0, oz = 0, ow =  1, cellid = -1, containmentType = -1, requiredSkill = "crafting_merchant_management_04", suiItem = "@player_structure:shop_sign4"},
	},
	constructionMarker = "object/building/player/construction/construction_player_house_tatooine_small_style_01.iff",
	length = 3,
	width = 4
}
ObjectTemplates:addTemplate(object_building_player_player_house_tatooine_small_style_02_dark, "object/building/player/player_house_tatooine_small_style_02_dark.iff")
