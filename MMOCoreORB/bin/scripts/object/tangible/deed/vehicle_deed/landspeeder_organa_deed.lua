
object_tangible_deed_vehicle_deed_landspeeder_organa_deed = object_tangible_deed_vehicle_deed_shared_landspeeder_organa_deed:new {

	templateType = VEHICLEDEED,

	controlDeviceObjectTemplate = "object/intangible/vehicle/landspeeder_organa_pcd.iff",
	generatedObjectTemplate = "object/mobile/vehicle/landspeeder_organa.iff",

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
ObjectTemplates:addTemplate(object_tangible_deed_vehicle_deed_landspeeder_organa_deed, "object/tangible/deed/vehicle_deed/landspeeder_organa_deed.iff")
