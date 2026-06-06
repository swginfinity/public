object_draft_schematic_vehicle_civilian_xj6_air_speeder_schematic = object_draft_schematic_vehicle_civilian_shared_xj6_air_speeder_schematic:new {

	templateType = DRAFTSCHEMATIC,

	customObjectName = "XJ-6 Airspeeder",

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
	resourceTypes = {"aluminum", "metal_ferrous", "steel", "aluminum", "fiberplast", "steel_carbonite"},
	resourceQuantities = {2500, 3000, 1000, 900, 700, 1200},
	contribution = {100, 100, 100, 100, 100, 100},

	targetTemplate = "object/tangible/deed/vehicle_deed/xj6_air_speeder_deed.iff",

	additionalTemplates = {}
}
ObjectTemplates:addTemplate(object_draft_schematic_vehicle_civilian_xj6_air_speeder_schematic, "object/draft_schematic/vehicle/civilian/xj6_air_speeder_schematic.iff")
