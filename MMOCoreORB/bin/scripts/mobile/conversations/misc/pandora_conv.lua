-- Legend of Hondo Merchant System
-- By Tatwi www.tpot.ca 2015
-- Adapted by Qrave for Infinity March 2018


pandora_template = ConvoTemplate:new {
	initialScreen = "start",
	templateType = "Lua",
	luaClassHandler = "pandora_convo_handler",
	screens = {}
}


pandora_start = ConvoScreen:new {
  id = "start",
  leftDialog = "",
  customDialogText = "Greetings Infinian, what can I do for you?",
  stopConversation = "false",
  options = {
  	{"Er, I hear you have some warez for sale?", "banter1"},
  }
}
pandora_template:addScreen(pandora_start);


pandora_shop = ConvoScreen:new {
  id = "shop",
  leftDialog = "",
  customDialogText = "Here's what I have in stock at the moment...",
  stopConversation = "false",
  options = {
  }
}
pandora_template:addScreen(pandora_shop);


pandora_confirm_purchase = ConvoScreen:new {
  id = "confirm_purchase",
  leftDialog = "",
  customDialogText = "Are you sure you would like to make this purchase?",
  stopConversation = "false",
  options = {
  }
}
pandora_template:addScreen(pandora_confirm_purchase);

pandora_insufficient_funds = ConvoScreen:new {
  id = "insufficient_funds",
  leftDialog = "",
  customDialogText = "Sorry, but you do not have enough credits on hand to make this purchase.",
  stopConversation = "true",
  options = {
  }
}
pandora_template:addScreen(pandora_insufficient_funds);


pandora_banter1 = ConvoScreen:new {
  id = "banter1",
  leftDialog = "",
  customDialogText = "I.. I don't know what you're talking about! What did you say your name was?",
  stopConversation = "false",
  options = {
	{"My name is...", "end1"},
	{"My name does not matter but the twin suns set on the dunes of the horizon.", "shop"},
	{"Don't worry, I am mistaken.", "end2"}
  }
}
pandora_template:addScreen(pandora_banter1);


pandora_end1 = ConvoScreen:new {
  id = "end1",
  leftDialog = "",
  customDialogText = "Sorry, I don't deal with strangers",
  stopConversation = "true",
  options = {
  }
}
pandora_template:addScreen(pandora_end1);

pandora_end2 = ConvoScreen:new {
  id = "end2",
  leftDialog = "",
  customDialogText = "Bye for now, friend",
  stopConversation = "true",
  options = {
  }
}
pandora_template:addScreen(pandora_end2);


-- Template Footer
addConversationTemplate("pandora_template", pandora_template);
