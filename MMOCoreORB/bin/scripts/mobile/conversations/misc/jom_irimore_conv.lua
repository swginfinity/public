-- Stub conversation for Jom Irimore (Corellia/Tyrena)
-- Placeholder until full quest line is implemented

jom_irimore_conv = ConvoTemplate:new {
	initialScreen = "init",
	templateType = "Lua",
	luaClassHandler = "",
	screens = {}
}

jom_irimore_init = ConvoScreen:new {
	id = "init",
	leftDialog = "",
	customDialogText = "Greetings, I am so happy you arrived. But my quests are not ready at this time.",
	stopConversation = "true",
	options = {}
}
jom_irimore_conv:addScreen(jom_irimore_init);

addConversationTemplate("jom_irimore_conv", jom_irimore_conv);
