region_area = SharedObjectTemplate:new {
	gameObjectType = 33554433,

	clientTemplateFileName = "",
	planetMapCategory = "",
	planetMapSubCategory = "",
	autoRegisterWithPlanetMap = 1,

	groundZoneComponent = "ZoneComponent",
	objectMenuComponent = "ObjectMenuComponent",
	containerComponent = "ContainerComponent"
}

ObjectTemplates:addTemplate(region_area, "object/region_area.iff")
