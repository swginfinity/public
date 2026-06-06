object_building_player_player_guildhall_tatooine_style_02_dark = object_building_player_shared_player_guildhall_tatooine_style_02_dark:new {
	lotSize = 5,
	baseMaintenanceRate = 50,
	allowedZones = {"dantooine", "lok", "tatooine", "naboo", "rori", "corellia", "talus"},
	publicStructure = 0,
	constructionMarker = "object/building/player/construction/construction_player_guildhall_tatooine_style_02.iff",
	length = 5,
	width = 9,
	skillMods = {
		{"private_medical_rating", 100},
		{"private_buff_mind", 100},
		{"private_med_battle_fatigue", 5},
		{"private_safe_logout", 1}
	},
	childObjects = {
		{templateFile = "object/tangible/sign/player/house_address_tatooine.iff", x = 0, z = 1.45, y = 16.35, ox = 0, oy = 1, oz = 0, ow = 0, cellid = -1, containmentType = -1},
		{templateFile = "object/tangible/terminal/terminal_player_structure.iff", x = 9.13004, z = 1.49609, y = 6.3587, ox = 0, oy = 0.707107, oz = 0, ow = 0.707107, cellid = 2, containmentType = -1},
		{templateFile = "object/tangible/terminal/terminal_guild.iff", x = 10.7, z = 1.50, y = 9.3, ow = 0, ox = 0, oy = 1, oz = 0, cellid = 2, containmentType = -1}
	},
	shopSigns = {
		{templateFile = "object/tangible/sign/player/house_address_tatooine.iff", x = 0, z = 1.45, y = 16.35, ox = 0, oy = 1, oz = 0, ow = 0, cellid = -1, containmentType = -1, requiredSkill = "", suiItem = "@player_structure:house_address"},
		{templateFile = "object/tangible/sign/player/shop_sign_s01.iff", x = 0, z = 0.49, y = 19.8, ox = 0, oy = 0, oz = 0, ow =  1, cellid = -1, containmentType = -1, requiredSkill = "crafting_merchant_management_01", suiItem = "@player_structure:shop_sign1"},
		{templateFile = "object/tangible/sign/player/shop_sign_s02.iff", x = 0, z = 0.49, y = 19.8, ox = 0, oy = 0, oz = 0, ow =  1, cellid = -1, containmentType = -1, requiredSkill = "crafting_merchant_management_02", suiItem = "@player_structure:shop_sign2"},
		{templateFile = "object/tangible/sign/player/shop_sign_s03.iff", x = 0, z = 0.49, y = 19.8, ox = 0, oy = 0, oz = 0, ow =  1, cellid = -1, containmentType = -1, requiredSkill = "crafting_merchant_management_03", suiItem = "@player_structure:shop_sign3"},
		{templateFile = "object/tangible/sign/player/shop_sign_s04.iff", x = 0, z = 0.49, y = 19.8, ox = 0, oy = 0, oz = 0, ow =  1, cellid = -1, containmentType = -1, requiredSkill = "crafting_merchant_management_04", suiItem = "@player_structure:shop_sign4"},
	},
}
ObjectTemplates:addTemplate(object_building_player_player_guildhall_tatooine_style_02_dark, "object/building/player/player_guildhall_tatooine_style_02_dark.iff")
