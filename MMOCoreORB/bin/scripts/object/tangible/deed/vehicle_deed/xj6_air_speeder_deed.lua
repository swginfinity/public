
object_tangible_deed_vehicle_deed_xj6_air_speeder_deed = object_tangible_deed_vehicle_deed_shared_xj6_air_speeder_deed:new {

	templateType = VEHICLEDEED,

	controlDeviceObjectTemplate = "object/intangible/vehicle/xj6_air_speeder_pcd.iff",
	generatedObjectTemplate = "object/mobile/vehicle/xj6_air_speeder.iff",

	numberExperimentalProperties = {1, 1, 1},
	experimentalProperties = {"XX", "XX", "SR"},
	experimentalWeights = {1, 1, 1},
	experimentalGroupTitles = {"null", "null", "exp_durability"},
	experimentalSubGroupTitles = {"null", "null", "hit_points"},
	experimentalMin = {0, 0, 1250},
	experimentalMax = {0, 0, 6200},
	experimentalPrecision = {0, 0, 0},
	experimentalCombineType = {0, 0, 1},
}
ObjectTemplates:addTemplate(object_tangible_deed_vehicle_deed_xj6_air_speeder_deed, "object/tangible/deed/vehicle_deed/xj6_air_speeder_deed.iff")
