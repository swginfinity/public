object_draft_schematic_clothing_clothing_backpack_krayt_skull = object_draft_schematic_clothing_shared_clothing_backpack_krayt_skull:new {
   templateType = DRAFTSCHEMATIC,

   customObjectName = "Krayt Skull Backpack",

   craftingToolTab = 8, -- (See DraftSchematicObjectTemplate.h)
   complexity = 1,
   size = 1,
   factoryCrateSize = 0,

   xpType = "crafting_clothing_general",
   xp = 110,

   assemblySkill = "clothing_assembly",
   experimentingSkill = "clothing_experimentation",
   customizationSkill = "clothing_customization",

   customizationOptions = {},
   customizationStringNames = {},
   customizationDefaults = {},

   ingredientTemplateNames = {"craft_clothing_ingredients_n", "craft_clothing_ingredients_n", "craft_clothing_ingredients_n", "craft_clothing_ingredients_n", "craft_clothing_ingredients_n", "craft_clothing_ingredients_n"},
   ingredientTitleNames = {"krayt_bones", "krayt_heart_d", "krayt_pearl_gland", "strap_pad", "strap_adjustment", "frame_reinforcement"},
   ingredientSlotType = {1, 1, 1, 0, 0, 0},
   resourceTypes = {"object/tangible/custom/shared_loot_324_000000000000000000000.iff", "object/tangible/custom/shared_loot_305_000000000000000000000.iff", "object/tangible/custom/shared_loot_256_000000000000000000000.iff", "hide", "metal", "aluminum_titanium"},
   resourceQuantities = {1, 1, 1, 10, 5, 10},
   contribution = {100, 100, 100, 100, 100, 100},


   targetTemplate = "object/tangible/wearables/backpack/backpack_krayt_skull.iff",

   additionalTemplates = {
             },

   skillMods = { 
	    {"absorption_disease",	10},
	    {"absorption_poison",	10},
		{"resistance_poison",	5},
	    {"resistance_disease",	5},

	}
}

ObjectTemplates:addTemplate(object_draft_schematic_clothing_clothing_backpack_krayt_skull, "object/draft_schematic/clothing/clothing_backpack_krayt_skull.iff")
