medicalBotConvoTemplate = ConvoTemplate:new {
	 initialScreen = "convoStart",
	 templateType = "Lua",
	 luaClassHandler = "medicalBotConvoHandler",
	 screens = {}
}

convoStart = ConvoScreen:new {
	 id = "convoStart",
	leftDialog = "",
	customDialogText = "I am the Infinity Medical Droid.  How may I be of assistance?",
	stopConversation = "false",
	options = {
		 {"I need my wounds and battle fatigue healed.","healWounds"},
		 {"I need to have DOT's removed.","clearDots"},
	 }
 }
medicalBotConvoTemplate:addScreen(convoStart);

insufficientFunds = ConvoScreen:new {
	 id = "insufficientFunds",
	leftDialog = "",
	customDialogText = "Payment is required before that service can be provided.",
	stopConversation = "true",
	options = {
	 }
 }
medicalBotConvoTemplate:addScreen(insufficientFunds);

healWounds = ConvoScreen:new {
	 id = "healWounds",
	leftDialog = "",
	customDialogText = "All your wounds and battle fatigue have been removed.",
	stopConversation = "true",
	options = {
	 }
 }
medicalBotConvoTemplate:addScreen(healWounds);


clearDots = ConvoScreen:new {
	 id = "clearDots",
	leftDialog = "",
	customDialogText = "Sure, I'll get rid of those.  All DOT's have been removed.",
	stopConversation = "true",
	options = {
	 }
 }
medicalBotConvoTemplate:addScreen(clearDots);

addConversationTemplate("medicalBotConvoTemplate", medicalBotConvoTemplate);