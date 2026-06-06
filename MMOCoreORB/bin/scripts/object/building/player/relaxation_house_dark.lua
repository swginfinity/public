object_building_player_relaxation_house_dark = object_building_player_shared_relaxation_house_dark:new {
	lotSize = 2,
	baseMaintenanceRate = 16,
	allowedZones = {"naboo", "taanab"},
	publicStructure = 0,
	skillMods = {
		{"private_medical_rating", 100},
		{"private_buff_mind", 100},
		{"private_med_battle_fatigue", 5},
		{"private_safe_logout", 1}	
	},
	childObjects = {

	},
	constructionMarker = "object/building/player/construction/construction_player_house_corellia_large_style_01.iff",
	length = 5,
	width = 7
}
ObjectTemplates:addTemplate(object_building_player_relaxation_house_dark, "object/building/player/relaxation_house_dark.iff")
