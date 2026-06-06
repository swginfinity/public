object_tangible_loot_loot_schematic_landspeeder_organa_schematic = object_tangible_loot_loot_schematic_shared_landspeeder_organa_schematic:new {
	templateType = LOOTSCHEMATIC,
	objectMenuComponent = "LootSchematicMenuComponent",
	attributeListComponent = "LootSchematicAttributeListComponent",
	requiredSkill = "crafting_artisan_master",
	targetDraftSchematic = "object/draft_schematic/vehicle/civilian/landspeeder_organa.iff",
	targetUseCount = 1,
}

ObjectTemplates:addTemplate(object_tangible_loot_loot_schematic_landspeeder_organa_schematic, "object/tangible/loot/loot_schematic/landspeeder_organa_schematic.iff")
