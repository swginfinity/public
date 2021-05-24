object_tangible_loot_loot_schematic_grenade_cortosis_schematic = object_tangible_loot_loot_schematic_shared_grenade_cortosis_schematic:new {
	templateType = LOOTSCHEMATIC,
	objectMenuComponent = "LootSchematicMenuComponent",
	attributeListComponent = "LootSchematicAttributeListComponent",
	requiredSkill = "crafting_weaponsmith_master",
	targetDraftSchematic = "object/draft_schematic/munition/grenade_cortosis.iff",
	targetUseCount = 10
}

ObjectTemplates:addTemplate(object_tangible_loot_loot_schematic_grenade_cortosis_schematic, "object/tangible/loot/loot_schematic/cortosis_grenade.iff")
