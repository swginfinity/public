-- JTL-DISABLED-2026-03-22
-- Generic conversation template for all JTL NPCs while space is disabled.
-- Uses jtlDisabledConvoHandler for randomized lore-friendly responses.

jtl_disabled_convo_template = ConvoTemplate:new {
	initialScreen = "",
	templateType = "Lua",
	luaClassHandler = "jtlDisabledConvoHandler",
	screens = {}
}

jtl_disabled_greeting = ConvoScreen:new {
	id = "jtl_disabled_greeting",
	leftDialog = "",
	stopConversation = "true",
	options = {}
}

jtl_disabled_convo_template:addScreen(jtl_disabled_greeting)
addConversationTemplate("jtl_disabled_convo_template", jtl_disabled_convo_template)
