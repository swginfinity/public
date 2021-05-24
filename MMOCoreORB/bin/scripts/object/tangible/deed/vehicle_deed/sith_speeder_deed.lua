
object_tangible_deed_vehicle_deed_sith_speeder_deed = object_tangible_deed_vehicle_deed_shared_sith_speeder_deed:new {

	templateType = VEHICLEDEED,

	controlDeviceObjectTemplate = "object/intangible/vehicle/sith_speeder_pcd.iff",
	generatedObjectTemplate = "object/mobile/vehicle/sith_speeder.iff",

	numberExperimentalProperties = {1, 1, 1},
	experimentalProperties = {"XX", "XX", "SR"},
	experimentalWeights = {1, 1, 1},
	experimentalGroupTitles = {"null", "null", "exp_durability"},
	experimentalSubGroupTitles = {"null", "null", "hit_points"},
	experimentalMin = {0, 0, 3000},
	experimentalMax = {0, 0, 5000},
	experimentalPrecision = {0, 0, 0},
	experimentalCombineType = {0, 0, 1},
}
ObjectTemplates:addTemplate(object_tangible_deed_vehicle_deed_sith_speeder_deed, "object/tangible/deed/vehicle_deed/vehicle_deed_sith_speeder.iff")
