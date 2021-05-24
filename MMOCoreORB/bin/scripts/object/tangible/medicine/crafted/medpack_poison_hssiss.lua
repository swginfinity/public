object_tangible_medicine_crafted_medpack_poison_hssiss = object_tangible_medicine_crafted_shared_medpack_poison_hssiss:new {
	gameObjectType = 8240,
	templateType = DOTPACK,
	useCount = 15,
	medicineUse = 0,
	effectiveness = 110,
	duration = 60,
	range = 32,
	rangeMod = 1.0,
	pool = 3,
	dotType = POISONED,
	potency = 150,
	commandToExecute = "venomdart",
	area = 0,

	numberExperimentalProperties = {1, 1, 2, 2, 2, 2, 2, 2, 1},
	experimentalProperties = {"XX", "XX", "OQ", "PE", "OQ", "UT", "CD", "OQ", "OQ", "PE", "OQ", "PE", "DR", "OQ", "XX"},
	experimentalWeights = {1, 1, 2, 1, 2, 1, 1, 2, 2, 1, 2, 1, 2, 1, 1},
	experimentalGroupTitles = {"null", "null", "exp_effectiveness", "exp_charges", "exp_charges", "expEaseOfUse", "expEaseOfUse", "exp_effectiveness", "null"},
	experimentalSubGroupTitles = {"null", "null", "power", "charges", "range", "skillmodmin", "potency", "duration", "hitpoints"},
	experimentalMin = {0, 0, 30, 20, 20, 5, 25, 30, 1000},
	experimentalMax = {0, 0, 100, 35, 35, 5, 50, 120, 1000},
	experimentalPrecision = {0, 0, 0, 0, 0, 0, 0, 0, 0},
	experimentalCombineType = {0, 0, 1, 1, 1, 1, 1, 1, 4},
}

ObjectTemplates:addTemplate(object_tangible_medicine_crafted_medpack_poison_hssiss, "object/tangible/medicine/crafted/medpack_poison_hssiss.iff")
