object_tangible_deed_vehicle_deed_barc_speeder_rebel_deed = object_tangible_deed_vehicle_deed_shared_barc_speeder_rebel_deed:new {

	templateType = VEHICLEDEED,

    	customName = "Deed for: Rebel Command BARC",

	controlDeviceObjectTemplate = "object/intangible/vehicle/barc_speeder_rebel_pcd.iff",
	generatedObjectTemplate = "object/mobile/vehicle/barc_speeder_rebel.iff",
	experimentalProperties = {"XX", "XX", "SR"},
	experimentalWeights = {1, 1, 1},
	experimentalGroupTitles = {"null", "null", "exp_durability"},
	experimentalSubGroupTitles = {"null", "null", "hit_points"},
	experimentalMin = {0, 0, 2500},
	experimentalMax = {0, 0, 3000},
	experimentalPrecision = {0, 0, 0},
	experimentalCombineType = {0, 0, 1},
}

ObjectTemplates:addTemplate(object_tangible_deed_vehicle_deed_barc_speeder_rebel_deed, "object/tangible/deed/vehicle_deed/barc_speeder_rebel_deed.iff")
