pilot_stub_convo = ConvoTemplate:new {
	initialScreen = "stub_greeting",
	templateType = "Lua",
	luaClassHandler = "defaultShipConvoHandler",
	screens = {}
}

pilot_stub_greeting = ConvoScreen:new {
	id = "stub_greeting",
	leftDialog = "",
	customDialogText = "I'm not ready to take on students just yet. Check back with me later — my training program is still being developed.",
	stopConversation = "true",
	options = {}
}

pilot_stub_convo:addScreen(pilot_stub_greeting)

-- defaultShipConvoHandler:getInitialScreen() calls getScreen("") — add an alias with empty id
-- so the conversation actually opens (without this, all stub pilot trainers are silent)
pilot_stub_greeting_default = ConvoScreen:new {
	id = "",
	leftDialog = "",
	customDialogText = "I'm not ready to take on students just yet. Check back with me later — my training program is still being developed.",
	stopConversation = "true",
	options = {}
}

pilot_stub_convo:addScreen(pilot_stub_greeting_default)
addConversationTemplate("pilot_stub_convo", pilot_stub_convo)
