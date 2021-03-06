junkDealerNadoConvoTemplate = ConvoTemplate:new {
	initialScreen = "init",
	templateType = "Lua",
	luaClassHandler = "JunkDealerNadoConvoHandler",
	screens = {}
}

init = ConvoScreen:new {
	id = "init",
	leftDialog = "@conversation/junk_nado_wattos:s_fa615166", -- Eh? Ah yes, I can tell by your look that you have something that I might be interested in buying. Well that is what Watto's is here for.
	stopConversation = "false",
	options = {
		{"@conversation/junk_nado_wattos:s_d94d5d64", "ask_for_loot"}, -- What sort of things do you buy?
		{"@conversation/junk_nado_wattos:s_50c8b3e1", "can_help"}, -- I am just taking a look around.
		{"Do you have anything rare?", "ask_for_quest"}, -- start the quest
	}
}
junkDealerNadoConvoTemplate:addScreen(init);

ask_for_quest = ConvoScreen:new {
	id = "ask_for_quest",
	leftDialog = "", 
	customDialogText = "Not here, my brother Nedo runs the Building Permit office at the capitol. He always talks about rare artifacts to sell but I'm too busy with junk to get involved.",
	stopConversation = "false",
	options = {
		{"What kinds of rare artifacts?", "rare_info"},
		{"Where is your brother?", "where_nedo"},
	}
}
junkDealerNadoConvoTemplate:addScreen(ask_for_quest);

prize = ConvoScreen:new {
   id = "prize",
   leftDialog = "", 
   customDialogText = "Yes, of course. It's a little dusty but I think you will enjoy it.",
   stopConversation = "true",
   options = {   }
}
junkDealerNadoConvoTemplate:addScreen(prize);

rare_info = ConvoScreen:new {
	id = "rare_info",
	leftDialog = "", 
	customDialogText = "Nedo handles all the building permits for Infinity Heritage LLC, the construction corporation that builds and restores every building, temple and any other construction you may run into. They even have archaeologists that excavate ruins to repurpose stuff. To push permits through they give him stuff from projects they are working on. You should see his house, it's insane.",
	stopConversation = "false",
	options = {
		{"Where is your brother?", "where_nedo"},
	}
}
junkDealerNadoConvoTemplate:addScreen(rare_info);

where_nedo = ConvoScreen:new {
	id = "where_nedo",
	leftDialog = "", 
	customDialogText = "He hangs out at a tavern in Mos Taike, we are twins so he should be easy to spot playing Lugjack.",
	stopConversation = "true",
	options = {
	}
}
junkDealerNadoConvoTemplate:addScreen(where_nedo);

can_help = ConvoScreen:new {
	id = "can_help",
	leftDialog = "@conversation/junk_nado_wattos:s_294d7f51", -- If there is anything that I can help you with just let me know.
	stopConversation = "true",
	options = {}
}
junkDealerNadoConvoTemplate:addScreen(can_help);

ask_for_loot = ConvoScreen:new {
	id = "ask_for_loot",
	leftDialog = "@conversation/junk_nado_wattos:s_17e67aee", -- We deal in a wide assortment of goods and offer the best prices on Tatooine for junk. A large number of settlers come to Watto's looking for various parts and nic-naks to suit their purpose.
	stopConversation = "false",
	options = {
		--{"@conversation/junk_nado_wattos:s_a5d02a3c", "start_sale"}, -- I think I might have a few things that will spark your interest.
		--{"@conversation/junk_nado_wattos:s_90b63763", "no_loot"}, -- I am sorry but I don't have anything you would be interested in.
	}
}
junkDealerNadoConvoTemplate:addScreen(ask_for_loot);

no_loot = ConvoScreen:new {
	id = "no_loot",
	leftDialog = "@conversation/junk_nado_wattos:s_6afe640d", -- That is alright. If you happen to come across anything you think I might like to buy stop on by and I will be more then willing to take it off your hands.
	stopConversation = "true",
	options = {}
}
junkDealerNadoConvoTemplate:addScreen(no_loot);

start_sale = ConvoScreen:new {
	id = "start_sale",
	leftDialog = "@conversation/junk_nado_wattos:s_8441c2ce", -- Excellent. Let me take a look at what you have to offer. Hmmmm...most of this isn't in very good condition but I am more then willing to make you an offer. Let me assure you no one will give you a better price.
	stopConversation = "true",
	options = {}
}
junkDealerNadoConvoTemplate:addScreen(start_sale);

addConversationTemplate("junkDealerNadoConvoTemplate", junkDealerNadoConvoTemplate);
