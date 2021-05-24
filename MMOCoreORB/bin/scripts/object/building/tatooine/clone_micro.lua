
object_building_tatooine_clone_micro = object_building_tatooine_shared_clone_micro:new {
	planetMapCategory = "cloningfacility",
	facilityType = CLONER_STANDARD,

	skillMods = {
		{"private_medical_rating", 120},
		{"private_med_wound_mind", 45},
		{"private_buff_mind", 125},
		{"private_med_wound_health", 120},
		{"private_med_wound_action", 120},
		{"private_med_battle_fatigue", 45},
		{"private_safe_logout", 1}
	},

	childObjects = {
		{templateFile = "object/tangible/terminal/terminal_cloning.iff", x = 2.7, z = .3, y = 0, ox = 0, oy = .7, oz = 0, ow = -.7, cellid = 1, containmentType = -1},
		{templateFile = "object/tangible/terminal/terminal_insurance.iff", x = 0, z = .3, y = -2, ox = 0, oy = 0, oz = 0, ow = 0, cellid = 1, containmentType = -1}
	},

	spawningPoints = {
		{ x = 0, z = 0, y = 0, ow = 0.704268, ox = 0, oz = 0, oy = 0, cellid = 1 },
	},
	templateType = CLONINGBUILDING
}
--
ObjectTemplates:addTemplate(object_building_tatooine_clone_micro, "object/building/tatooine/clone_micro.iff")
