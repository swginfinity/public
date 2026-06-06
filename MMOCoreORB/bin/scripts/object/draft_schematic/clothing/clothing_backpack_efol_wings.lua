object_draft_schematic_clothing_clothing_backpack_efol_wings = object_draft_schematic_clothing_shared_clothing_backpack_efol_wings:new {


   templateType = DRAFTSCHEMATIC,

   customObjectName = "Efol Wings Backpack",

   craftingToolTab = 8, -- (See DraftSchematicObjectTemplate.h)
   complexity = 1,
   size = 2,

   xpType = "crafting_clothing_general",
   xp = 60,

   assemblySkill = "clothing_assembly",
   experimentingSkill = "clothing_experimentation",
   customizationSkill = "clothing_customization",

   customizationOptions = {},
   customizationStringNames = {},
   customizationDefaults = {},

   ingredientTemplateNames = {"craft_clothing_ingredients_n", "craft_clothing_ingredients_n", "craft_clothing_ingredients_n", "craft_clothing_ingredients_n", "craft_clothing_ingredients_n", "craft_clothing_ingredients_n"},
   ingredientTitleNames = {"primary_pocket", "secondary_pocket", "tertiary_pocket", "strap_pad", "strap_adjustment", "frame_reinforcement"},
   ingredientSlotType = {1, 1, 1, 0, 0, 0},
   resourceTypes = {"object/tangible/component/clothing/shared_cargo_pocket.iff", "object/tangible/component/clothing/shared_cargo_pocket.iff", "object/tangible/component/clothing/shared_cargo_pocket.iff", "hide", "metal", "aluminum_titanium"},
   resourceQuantities = {1, 1, 1, 10, 5, 10},
   contribution = {100, 100, 100, 100, 100, 100},


   targetTemplate = "object/tangible/wearables/backpack/back_love_day_wings.iff",

   additionalTemplates = {
             }

}

ObjectTemplates:addTemplate(object_draft_schematic_clothing_clothing_backpack_efol_wings, "object/draft_schematic/clothing/clothing_backpack_efol_wings.iff")
