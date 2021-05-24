object_building_player_jabbas_house = object_building_player_shared_jabbas_house:new {
	lotSize = 3,
	baseMaintenanceRate = 34,
	allowedZones = {"corellia", "dantooine", "lok", "naboo", "rori", "talus", "tatooine"},
	constructionMarker = "object/building/player/construction/construction_player_house_corellia_medium_style_02.iff",
	length = 8,
	width = 2,
	publicStructure = 0,
	skillMods = {
		{"private_medical_rating", 100},
		{"private_buff_mind", 100},
		{"private_med_battle_fatigue", 5},
		{"private_safe_logout", 1},
	},
	childObjects = {
			{templateFile = "object/tangible/terminal/terminal_player_structure.iff", x = -2.91613, z = 14.6741, y = 4.33609, ow = -1, ox = 0, oz = 0, oy = 0, cellid = 1, containmentType = -1},
			{templateFile = "object/tangible/terminal/terminal_elevator_up.iff", x = -0.0335694, z = 0.40893, y = -0.324013, ow = 0, ox = 0, oz = 0, oy = -1, cellid = 1, containmentType = -1},
			{templateFile = "object/tangible/terminal/terminal_elevator_down.iff", x = 0.445379, z = 16.1046, y = -0.323695, ow = 0, ox = 0, oz = 0, oy = -1, cellid = 1, containmentType = -1},
			{templateFile = "object/static/structure/tatooine/wall_pristine_tatooine_small_style_01.iff", x = 0.34, z = 15.31, y = -0.09, ow = 0.70711, ox = 0, oz = -0, oy = 0.70711, cellid = 1, containmentType = -1}, -- Lower Wall
			{templateFile = "object/tangible/component/structure/lightweight_turret.iff", x = -0.618712, z = 16.6, y = 0.77593, ow = -4.37114e-08, ox = 1, oz = 0, oy = 0, cellid = 1, containmentType = -1}, -- Lower Right Turret
			{templateFile = "object/tangible/component/structure/lightweight_turret.iff", x = 1.53094, z = 16.6, y = 0.77593, ow = -4.37114e-08, ox = 1, oz = 0, oy = 0, cellid = 1, containmentType = -1}, -- Lower Right Turret
			{templateFile = "object/tangible/component/structure/lightweight_turret.iff", x = -0.618712, z = 19.46, y = 0.77593, ow = 1, ox = 0, oz = 0, oy = 0, cellid = 1, containmentType = -1}, -- Upper Right Turret
			{templateFile = "object/tangible/component/structure/lightweight_turret.iff", x = 1.53094, z = 19.46, y = 0.77593, ow = 1, ox = 0, oz = 0, oy = 0, cellid = 1, containmentType = -1}, -- Upper Right Turret
			{templateFile = "object/tangible/sign/player/house_address.iff",  x = 0.08, z = 2.81, y = -3.55, ox = 0, oy = 0, oz = 0, ow = 1, cellid = -1, containmentType = -1}
	},
	shopSigns = {
			{templateFile = "object/tangible/sign/player/house_address.iff", x = 0, z = 2.81, y = -3.65, ox = 0, oy = 0, oz = 0, ow = 1, cellid = -1, containmentType = -1, requiredSkill = "", suiItem = "@player_structure:house_address"},
			{templateFile = "object/tangible/sign/player/shop_sign_s01.iff", x = 1.59, z = 0.24, y = -5.05, ox = 0, oy = 0.258819, oz = 0, ow = 0.9659258, cellid = -1, containmentType = -1, requiredSkill = "crafting_merchant_management_01", suiItem = "@player_structure:shop_sign1"},
			{templateFile = "object/tangible/sign/player/shop_sign_s02.iff", x = 1.58, z = 0.24, y = -5.32, ox = 0, oy = 1, oz = 0, ow = 0, cellid = -1, containmentType = -1, requiredSkill = "crafting_merchant_management_02", suiItem = "@player_structure:shop_sign2"},
			{templateFile = "object/tangible/sign/player/shop_sign_s03.iff", x = 1.65, z = 0.24, y = -5, ox = 0, oy = 1, oz = 0, ow = 0, cellid = -1, containmentType = -1, requiredSkill = "crafting_merchant_management_03", suiItem = "@player_structure:shop_sign3"},
			{templateFile = "object/tangible/sign/player/shop_sign_s04.iff", x = 1.71, z = 0.24, y = -5.29, ox = 0, oy = 1, oz = 0, ow = 0, cellid = -1, containmentType = -1, requiredSkill = "crafting_merchant_management_04", suiItem = "@player_structure:shop_sign4"},
	},
}
ObjectTemplates:addTemplate(object_building_player_jabbas_house, "object/building/player/jabbas_house.iff")
