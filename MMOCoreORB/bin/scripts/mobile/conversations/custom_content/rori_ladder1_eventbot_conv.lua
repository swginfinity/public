rori_ladder1_eventbotConvoTemplate = ConvoTemplate:new {
	initialScreen = "init_non_admin",
	templateType = "Lua",
	luaClassHandler = "rori_ladder1_eventbotConvoHandler",
	screens = {}
}

init_non_admin = ConvoScreen:new {
	id = "init_non_admin",
	leftDialog = "", 
	customDialogText = "Have an A-1 Day!",
	stopConversation = "true",
	options = {	}
}
rori_ladder1_eventbotConvoTemplate:addScreen(init_non_admin);

init_admin = ConvoScreen:new {
	id = "init_admin",
	leftDialog = "", 
	customDialogText = "Your wish is my command",
	stopConversation = "false",
	options = {	}
}
rori_ladder1_eventbotConvoTemplate:addScreen(init_admin);

c1 = ConvoScreen:new {	id = "c1",
	leftDialog = "", customDialogText = "I reset the battle as you wish, El Conquistador!!!!",stopConversation = "false",options = {	
	{"I need to do more","init_admin"}}}
rori_ladder1_eventbotConvoTemplate:addScreen(c1);

c2 = ConvoScreen:new {	id = "c2",
	leftDialog = "", customDialogText = "The battle is paused for 2 hours m'lord",stopConversation = "false",options = {{"I need to do more","init_admin"}}}
rori_ladder1_eventbotConvoTemplate:addScreen(c2);

c3 = ConvoScreen:new {	id = "c3",
	leftDialog = "", customDialogText = "The battle has resumed, sir, but you may need to spawn a wave",stopConversation = "false",options = {{"I need to do more","init_admin"}}}
rori_ladder1_eventbotConvoTemplate:addScreen(c3);

c4 = ConvoScreen:new {	id = "c4",
	leftDialog = "", customDialogText = "I destroyed them all and paused the battle for 24 hours, your highness",stopConversation = "false",options = {{"I need to do more","init_admin"}}}
rori_ladder1_eventbotConvoTemplate:addScreen(c4);

c5 = ConvoScreen:new {	id = "c5",
	leftDialog = "", customDialogText = "The buffers are ready to serve.",stopConversation = "false",options = {{"I need to do more","init_admin"}}}
rori_ladder1_eventbotConvoTemplate:addScreen(c5);

c6 = ConvoScreen:new {	id = "c6",
	leftDialog = "", customDialogText = "I sent the buffers away",stopConversation = "false",options = {{"I need to do more","init_admin"}}}
rori_ladder1_eventbotConvoTemplate:addScreen(c6);

s1 = ConvoScreen:new {	id = "s1",
	leftDialog = "", customDialogText = "The Captains have been recalled, your majesty",stopConversation = "false",options = {{"I need to do more","init_admin"}}}
rori_ladder1_eventbotConvoTemplate:addScreen(s1);

s2 = ConvoScreen:new {	id = "s2",
	leftDialog = "", customDialogText = "The guards have returned to battle, oh great one",stopConversation = "false",options = {{"I need to do more","init_admin"}}}
rori_ladder1_eventbotConvoTemplate:addScreen(s2);

s3 = ConvoScreen:new {	id = "s3",
	leftDialog = "", customDialogText = "The mechanical army has returned to fight, my lord of the dance",stopConversation = "false",options = {{"I need to do more","init_admin"}}}
rori_ladder1_eventbotConvoTemplate:addScreen(s3);

s4 = ConvoScreen:new {	id = "s4",
	leftDialog = "", customDialogText = "The pilots have landed, sire",stopConversation = "false",options = {{"I need to do more","init_admin"}}}
rori_ladder1_eventbotConvoTemplate:addScreen(s4);

s5 = ConvoScreen:new {	id = "s5",
	leftDialog = "", customDialogText = "The commandos are lightning up the battle now, sir",stopConversation = "false",options = {{"I need to do more","init_admin"}}}
rori_ladder1_eventbotConvoTemplate:addScreen(s5);

s6 = ConvoScreen:new {	id = "s6",
	leftDialog = "", customDialogText = "The battle droids have started to destroy the area, your highness",stopConversation = "false",options = {{"I need to do more","init_admin"}}}
rori_ladder1_eventbotConvoTemplate:addScreen(s6);

s7 = ConvoScreen:new {	id = "s7",
	leftDialog = "", customDialogText = "The Varactyl has been released, m'lord",stopConversation = "false",options = {{"I need to do more","init_admin"}}}
rori_ladder1_eventbotConvoTemplate:addScreen(s7);

s8 = ConvoScreen:new {	id = "s8",
	leftDialog = "", customDialogText = "The Jedi have returned, your honor",stopConversation = "false",options = {{"I need to do more","init_admin"}}}
rori_ladder1_eventbotConvoTemplate:addScreen(s8);

battleOff = ConvoScreen:new {	id = "battleOff",
	leftDialog = "", customDialogText = "Battle has been turned off completely, your excellence",stopConversation = "false",options = {{"I need to do more","init_admin"}}}
rori_ladder1_eventbotConvoTemplate:addScreen(battleOff);

battleOn = ConvoScreen:new {	id = "battleOn",
	leftDialog = "", customDialogText = "Battle is back on, your majesty",stopConversation = "false",options = {{"I need to do more","init_admin"}}}
rori_ladder1_eventbotConvoTemplate:addScreen(battleOn);

addConversationTemplate("rori_ladder1_eventbotConvoTemplate", rori_ladder1_eventbotConvoTemplate);
