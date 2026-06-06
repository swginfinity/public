object_building_player_sandcrawler_house_dark = object_building_player_shared_sandcrawler_house_dark:new {
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
		{templateFile = "object/tangible/sign/player/house_address.iff", x = -8.91, z = 4.39, y = -12, ox = 0, oy = 0.70711, oz = 0, ow = 0.70711, cellid = -1, containmentType = -1},
		{templateFile = "object/tangible/terminal/terminal_player_structure.iff", x = -4.71, z = -0.14, y = -0.54, ox = 0, oy = 0, oz = 0, ow = 1, cellid = 1, containmentType = -1},
	},
	shopSigns = {
		{templateFile = "object/tangible/sign/player/house_address.iff", x = -8.91, z = 4.39, y = -12, ox = 0, oy = 0.70711, oz = 0, ow = 0.70711, cellid = -1, containmentType = -1, requiredSkill = "", suiItem = "@player_structure:house_address"},
		{templateFile = "object/tangible/sign/player/shop_sign_s01.iff", x = -6.62, z = -.01, y = 2.89, ox = 0, oy = 0.70711, oz = 0, ow = .70711, cellid = -1, containmentType = -1, requiredSkill = "crafting_merchant_management_01", suiItem = "@player_structure:shop_sign1"},
		{templateFile = "object/tangible/sign/player/shop_sign_s02.iff", x = -6.62, z = -.01, y = 2.89, ox = 0, oy = 0.70711, oz = 0, ow = .70711, cellid = -1, containmentType = -1, requiredSkill = "crafting_merchant_management_02", suiItem = "@player_structure:shop_sign2"},
		{templateFile = "object/tangible/sign/player/shop_sign_s03.iff", x = -6.62, z = -.01, y = 2.89, ox = 0, oy = 0.70711, oz = 0, ow = .70711, cellid = -1, containmentType = -1, requiredSkill = "crafting_merchant_management_03", suiItem = "@player_structure:shop_sign3"},
		{templateFile = "object/tangible/sign/player/shop_sign_s04.iff", x = -6.62, z = -.01, y = 2.89, ox = 0, oy = 0.70711, oz = 0, ow = .70711, cellid = -1, containmentType = -1, requiredSkill = "crafting_merchant_management_04", suiItem = "@player_structure:shop_sign4"},
	},

	constructionMarker = "object/building/player/construction/construction_player_house_corellia_large_style_01.iff",
	length = 5,
	width = 7
}
ObjectTemplates:addTemplate(object_building_player_sandcrawler_house_dark, "object/building/player/sandcrawler_house_dark.iff")
