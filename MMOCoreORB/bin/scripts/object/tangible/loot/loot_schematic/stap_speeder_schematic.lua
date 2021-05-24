object_tangible_loot_loot_schematic_stap_speeder_schematic = object_tangible_loot_loot_schematic_shared_stap_speeder_schematic:new {
	templateType = LOOTSCHEMATIC,
	objectMenuComponent = "LootSchematicMenuComponent",
	attributeListComponent = "LootSchematicAttributeListComponent",
	requiredSkill = "crafting_artisan_master",
	targetDraftSchematic = "object/draft_schematic/vehicle/civilian/speeder_stap.iff",
	targetUseCount = 3
}

ObjectTemplates:addTemplate(object_tangible_loot_loot_schematic_stap_speeder_schematic, "object/tangible/loot/loot_schematic/stap_speeder_schematic.iff")
