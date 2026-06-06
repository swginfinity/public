buffBotConvoTemplate = ConvoTemplate:new {
	 initialScreen = "convoStart",
	 templateType = "Lua",
	 luaClassHandler = "BuffBotConvoHandler",
	 screens = {}
}

convoStart = ConvoScreen:new {
	 id = "convoStart",
	leftDialog = "",
	customDialogText = "I am the Special Event Medical Droid.  How may I be of assistance?",
	stopConversation = "false",
	options = {
		 {"I need to have event buffs applied.","enhanceStats"},
		 {"Can you remove all my existing buffs and empty my stomach?","clearEnhancements"},
		 {"I need my HAM damage healed.","healDamage"},
		 {"I need my wounds and battle fatigue healed.","healWounds"},
		 {"I need to have DOT's removed.","clearDots"},
	 }
 }
buffBotConvoTemplate:addScreen(convoStart);

insufficientFunds = ConvoScreen:new {
	 id = "insufficientFunds",
	leftDialog = "",
	customDialogText = "Payment is required before that service can be provided.",
	stopConversation = "true",
	options = {
	 }
 }
buffBotConvoTemplate:addScreen(insufficientFunds);

healWounds = ConvoScreen:new {
	 id = "healWounds",
	leftDialog = "",
	customDialogText = "All your wounds and battle fatigue have been removed.",
	stopConversation = "true",
	options = {
	 }
 }
buffBotConvoTemplate:addScreen(healWounds);

healDamage = ConvoScreen:new {
	id = "healDamage",
   leftDialog = "",
   customDialogText = "All your HAM damage has been healed.",
   stopConversation = "true",
   options = {
	}
}
buffBotConvoTemplate:addScreen(healDamage);

enhanceStats = ConvoScreen:new {
	 id = "enhanceStats",
	leftDialog = "",
	customDialogText = "Your event buffs have been applied.  Good luck at the event!",
	stopConversation = "true",
	options = {
	 }
 }
buffBotConvoTemplate:addScreen(enhanceStats);

clearEnhancements = ConvoScreen:new {
	 id = "clearEnhancements",
	leftDialog = "",
	customDialogText = "Sure, I'll get rid of those.  All buffs have been removed and your stomach has been emptied.",
	stopConversation = "true",
	options = {
	 }
 }
buffBotConvoTemplate:addScreen(clearEnhancements);

clearDots = ConvoScreen:new {
	 id = "clearDots",
	leftDialog = "",
	customDialogText = "Sure, I'll get rid of those.  All DOT's have been removed.",
	stopConversation = "true",
	options = {
	 }
 }
buffBotConvoTemplate:addScreen(clearDots);

addConversationTemplate("buffBotConvoTemplate", buffBotConvoTemplate);