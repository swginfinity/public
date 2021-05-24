object_draft_schematic_vehicle_civilian_landspeeder_ab1 = object_draft_schematic_vehicle_civilian_shared_landspeeder_ab1:new {

	templateType = DRAFTSCHEMATIC,

	customObjectName = "AB-1 Landspeeder",

	craftingToolTab = 16, -- (See DraftSchematicObjectTemplate.h)
	complexity = 1,
	size = 1,
	factoryCrateSize = 0,

	xpType = "crafting_general",
	xp = 1600,

	assemblySkill = "general_assembly",
	experimentingSkill = "general_experimentation",
	customizationSkill = "clothing_customization",

	customizationOptions = {},
	customizationStringNames = {},
	customizationDefaults = {},

	ingredientTemplateNames = {"craft_vehicle_ingredients_n", "craft_vehicle_ingredients_n", "craft_vehicle_ingredients_n", "craft_vehicle_ingredients_n", "craft_vehicle_ingredients_n", "craft_vehicle_ingredients_n"},
	ingredientTitleNames = {"vehicle_body", "structural_frame", "stabilizor_subframe", "avionic_subassembly", "repulsor_assembly", "cockpit_fitout"},
	ingredientSlotType = {0, 0, 0, 0, 0, 0},
	resourceTypes = {"aluminum_titanium", "metal_ferrous", "aluminum_linksteel", "aluminum_linksteel", "fiberplast_lok", "steel_carbonite"},
	resourceQuantities = {2650, 8950, 3220, 1840, 3650, 1250},
	contribution = {100, 100, 100, 100, 100, 100},

	targetTemplate = "object/tangible/deed/vehicle_deed/landspeeder_ab1_deed.iff",

	additionalTemplates = {}
}
ObjectTemplates:addTemplate(object_draft_schematic_vehicle_civilian_landspeeder_ab1, "object/draft_schematic/vehicle/civilian/landspeeder_ab1.iff")
