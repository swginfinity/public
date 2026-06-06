
object_building_tatooine_shuttle_micro_wall = object_building_tatooine_shared_shuttle_micro_wall:new {
	planetMapCategory = "shuttleport",
	childObjects = {
		{templateFile = "object/tangible/terminal/terminal_travel.iff", x = 0, z = 0.0, y = 1, ox = 0, oy = 0, oz = 0, ow = 2, cellid = -1, containmentType = -1},
		{templateFile = "object/tangible/travel/ticket_collector/ticket_collector.iff", x = 0, z = 0.0, y = -1, ox = 0, oy = -0.707107, oz = 0, ow = 0.707107, cellid = -1, containmentType = -1},
		{templateFile = "object/tangible/painting/painting_schematic_transport_ship.iff", x = .5, z = 2, y = -1, ox = 0, oy = 0, oz = 0, ow = 1, cellid = -1, containmentType = -1},
	}
}
--
ObjectTemplates:addTemplate(object_building_tatooine_shuttle_micro_wall, "object/building/tatooine/shuttle_micro_wall.iff")
