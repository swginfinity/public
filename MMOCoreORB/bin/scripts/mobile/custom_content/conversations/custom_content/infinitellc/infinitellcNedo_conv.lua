-- convo template

infiniteLLCNedoConvoTemplate = ConvoTemplate:new {
	initialScreen = "init",
	templateType = "Lua",
	luaClassHandler = "infiniteLLCNedoConvoHandler",
	screens = {}
}

init = ConvoScreen:new {
	id = "init",
	leftDialog = "", 
	customDialogText = "I hope you're not bothering me here for a permit. Go to my office in the capitol during normal business hours.",
	stopConversation = "false",
	options = {
		{"What are the hours?", "what_hours"}, -- what hours?
		{"I was looking for something else...", "something_else"}, -- I was looking for something else
		{"I'm too busy right now", "end_1"}, -- 
	}
}
infiniteLLCNedoConvoTemplate:addScreen(init);

what_hours = ConvoScreen:new {
	id = "what_hours",
	leftDialog = "", 
	customDialogText = "None really, I can't show my face around there anymore after I got caught. They will probably fire me or the CEO of Infinity Heritage will kill me before I make it there.",
	stopConversation = "false",
	options = {
		{"Infinity Heritage?", "inf_heritage"}, -- 
		{"Why does he want you dead?", "wanted_dead"}, -- 
	}
}
infiniteLLCNedoConvoTemplate:addScreen(what_hours);

inf_heritage = ConvoScreen:new {
	id = "inf_heritage",
	leftDialog = "", 
	customDialogText = "Infinity Heritage LLC is a massive corporation that builds everything, they even build the stuff used to build stuff. They built this tavern, they built this lugjack machine, they even built the coins I'm using to play. Corporations that big have lots of enemies but I never meant to be one of them.",
	stopConversation = "false",
	options = {
		{"Who is the CEO?", "inf_ceo"}, -- 
		{"And they want you dead?", "wanted_dead"}, -- 
		{"Anything I can do?", "how_q1"}, -- 
	}
}
infiniteLLCNedoConvoTemplate:addScreen(inf_heritage);

inf_ceo = ConvoScreen:new {
	id = "inf_ceo",
	leftDialog = "", 
	customDialogText = "Monty, not sure his last name, his main headquarters is in Doaba Guerfel on Corellia, big gaudy sign, can't miss it. For a corporation that big you would think they would have built their own planet. Instead he has some tiny office in a big empty building. Cheap bastard!",
	stopConversation = "false",
	options = {
		{"And he wants you dead?", "wanted_dead"}, -- 
		{"Anything I can do?", "how_q1"}, --
	}
}
infiniteLLCNedoConvoTemplate:addScreen(inf_ceo);

something_else = ConvoScreen:new {
	id = "something_else",
	leftDialog = "", 
	customDialogText = "Something else? Me too, know anyone hiring? I am in debt for about 500 milliom credits so I could use a second job that pays better than the 25,000 a year I get paid now",
	stopConversation = "false",
	options = {
		{"How can I help?", "how_q1"}, -- 
		{"Debt?", "wanted_dead"}, -- 
		{"I'm too busy right now", "end_1"}, -- 
	}
}
infiniteLLCNedoConvoTemplate:addScreen(something_else);

wanted_dead = ConvoScreen:new {
	id = "wanted_dead",
	leftDialog = "", 
	customDialogText = "There's this group of theives that steal from Infinity's work sites but they are clever about it. I issued a permit to a group of archaeologists for them to remove a sacred temple not realizing they were imposters and worse, Infinity Heritage just built it for a wealthy client. They gave me some really interesting artifacts that I sold. I gave the money to them but the little box I sold for 50,000 was worth nearly 500 million.",
	stopConversation = "false",
	options = {
		{"How can I help?", "how_q1"}, -- 
		{"I'm too busy right now", "end_1"}, -- 
	}
}
infiniteLLCNedoConvoTemplate:addScreen(wanted_dead);

end_1 = ConvoScreen:new {
	id = "end_1",
	leftDialog = "", 
	customDialogText = "I understand, if you have some time later on, let me know. I'll be here.",
	stopConversation = "true",
	options = {
	}
}
infiniteLLCNedoConvoTemplate:addScreen(end_1);


how_q1 = ConvoScreen:new {
	id = "how_q1",
	leftDialog = "", 
	customDialogText = "Maybe you can help me get my good reputation back. About 1600m east of here is a small archway. Some of their thugs are excavating it but it's a sacred burrial. Ignore that they call themselves archaeologists, they are just grave robbers that should be killed.",
	stopConversation = "false",
	options = {
		{"Ok, I will go take care of that now.", "go_q1"},
		{"I'm too busy right now", "end_1"}, -- 
	}
}
infiniteLLCNedoConvoTemplate:addScreen(how_q1);
go_q1 = ConvoScreen:new {
	id = "go_q1",
	leftDialog = "", 
	customDialogText = "Excellent, come back when you are done.",
	stopConversation = "true",
	options = {	}
}
infiniteLLCNedoConvoTemplate:addScreen(go_q1);
wait_q1 = ConvoScreen:new {
	id = "wait_q1",
	leftDialog = "", 
	customDialogText = "Go on, it's about 1600m easy, you can't miss it. Just a doorway near 5400 2100 or something, I drive by it every day but can't remember the exact coordinates. And if you can't figure it out on your own, you might have trouble working for me.",
	stopConversation = "false",
	options = {
		{"Okay, okay, I'm on it", "go_q1"},
		{"I'm too busy right now", "end_1"}, -- 
	}
}
infiniteLLCNedoConvoTemplate:addScreen(wait_q1);
end_q1 = ConvoScreen:new {
	id = "end_q1",
	leftDialog = "", 
	customDialogText = "Excellent, I know another place that could use some cleanup.",
	stopConversation = "false",
	options = {
		{"Where?", "how_q2"},
		{"I'm too busy right now", "end_1"}, 
	}
}
infiniteLLCNedoConvoTemplate:addScreen(end_q1);

how_q2 = ConvoScreen:new {
	id = "how_q2",
	leftDialog = "", 
	customDialogText = "Well the good news is that the Infinity CEO doesn't want me dead, but he would like the maim me a little. Rather than get my legs broken, could you go take care of something? A good friend of Monty is trying to recover artifacts from his family's estate but these thugs are bothering him. He's near -2000 2600.",
	stopConversation = "false",
	options = {
		{"Not a problem, I'll be right back.", "go_q2"},
		{"I'm too busy right now", "end_1"}, -- end 1 works for most quests
	}
}
infiniteLLCNedoConvoTemplate:addScreen(how_q2);
go_q2 = ConvoScreen:new {
	id = "go_q2",
	leftDialog = "", 
	customDialogText = "Excellent, come back when you are done.",
	stopConversation = "true",
	options = {	}
}
infiniteLLCNedoConvoTemplate:addScreen(go_q2);
wait_q2 = ConvoScreen:new {
	id = "wait_q2",
	leftDialog = "", 
	customDialogText = "Are you sure you are cut out for this? Kill everyone in this vicinity: -2000 2600. I'd do it myself but I'm busy thinking right now.",
	stopConversation = "false",
	options = {
		{"Sorry, I will get that done now.", "go_q2"},
		{"I'm too busy right now", "end_1"}, -- 
	}
}
infiniteLLCNedoConvoTemplate:addScreen(wait_q2);
end_q2 = ConvoScreen:new {
	id = "end_q2",
	leftDialog = "", 
	customDialogText = "I like how this is moving along. Let me know when you want to help with another problem that just came up.",
	stopConversation = "false",
	options = {
		{"I'm all ears.", "how_q3"},
		{"I'm too busy right now", "end_1"}, 
	}
}
infiniteLLCNedoConvoTemplate:addScreen(end_q2);

how_q3 = ConvoScreen:new {
	id = "how_q3",
	leftDialog = "", 
	customDialogText = "I seem to be in good faith with Infinity Heritage, I owe you for that. But I found out they are going to fire me. My boss has these thugs that started bothering him at his house. I think if you take care of them, my life will be back to normal.",
	stopConversation = "false",
	options = {
		{"Not a problem, I'll be right back.", "go_q3"},
		{"I'm too busy right now", "end_1"}, -- end 1 works for most quests
	}
}
infiniteLLCNedoConvoTemplate:addScreen(how_q3);
go_q3 = ConvoScreen:new {
	id = "go_q3",
	leftDialog = "", 
	customDialogText = "Good luck out there!",
	stopConversation = "true",
	options = {	}
}
infiniteLLCNedoConvoTemplate:addScreen(go_q3);
wait_q3 = ConvoScreen:new {
	id = "wait_q3",
	leftDialog = "", 
	customDialogText = "Are you sure you are cut out for this? Kill everyone in this vicinity: -2000 2600. I'd do it myself but I'm busy thinking right now.",
	stopConversation = "false",
	options = {
		{"Sorry, I will get that done now.", "go_q3"},
		{"I'm too busy right now", "end_1"}, -- 
	}
}
infiniteLLCNedoConvoTemplate:addScreen(wait_q3);
end_q3 = ConvoScreen:new {
	id = "end_q3",
	leftDialog = "", 
	customDialogText = "I like how this is moving along. Let me know when you want to help with another problem",
	stopConversation = "false",
	options = {
		{"Certainly, what can I do?", "how_q4"},
		{"I'm too busy right now", "end_1"}, 
	}
}
infiniteLLCNedoConvoTemplate:addScreen(end_q3);

how_q4 = ConvoScreen:new {
	id = "how_q4",
	leftDialog = "", 
	customDialogText = "Now that I got my job back, I want to take a short vacation at my favorite beach but a group of thugs have taken the area over, can you wipe them out so I can bring the wife and kids for some quiet time around the lake?",
	stopConversation = "false",
	options = {
		{"Happy to help", "go_q4"},
		{"I'm too busy right now", "end_1"}, -- end 1 works for most quests
	}
}
infiniteLLCNedoConvoTemplate:addScreen(how_q4);
go_q4 = ConvoScreen:new {
	id = "go_q4",
	leftDialog = "", 
	customDialogText = "Thank you so much!",
	stopConversation = "true",
	options = {	}
}
infiniteLLCNedoConvoTemplate:addScreen(go_q4);
wait_q4 = ConvoScreen:new {
	id = "wait_q4",
	leftDialog = "", 
	customDialogText = "Please help me with my trip and cleanup the beach near 1790 -6100",
	stopConversation = "false",
	options = {
		{"Sorry, I will get that done now.", "go_q4"},
		{"I'm too busy right now", "end_1"}, -- 
	}
}
infiniteLLCNedoConvoTemplate:addScreen(wait_q4);
end_q4 = ConvoScreen:new {
	id = "end_q4",
	leftDialog = "", 
	customDialogText = "Great job, as a token of my appreciation, here is a painting I had hanging in my office. Would you like to continue? I have a friend on Talus that could really use help from someone like you.",
	stopConversation = "false",
	options = {
		{"Yes, I am up for that", "how_q5"},
		{"I'm too busy right now", "end_1"}, 
	}
}
infiniteLLCNedoConvoTemplate:addScreen(end_q4);

how_q5 = ConvoScreen:new {
	id = "how_q5",
	leftDialog = "", 
	customDialogText = "Insofar is in the Nashal Cantina trying to get some archaeological digs started. Please go see him.",
	stopConversation = "false",
	options = {
		{"Happy to help", "go_q5"},
		{"I'm too busy right now", "end_1"}, -- end 1 works for most quests
	}
}
infiniteLLCNedoConvoTemplate:addScreen(how_q5);
go_q5 = ConvoScreen:new {
	id = "go_q5",
	leftDialog = "", 
	customDialogText = "Thanks again for all your help.",
	stopConversation = "true",
	options = {	}
}
infiniteLLCNedoConvoTemplate:addScreen(go_q5);
wait_q5 = ConvoScreen:new {
	id = "wait_q5",
	leftDialog = "", 
	customDialogText = "Please go see Insofar in the Nashal Cantina.",
	stopConversation = "true",
	options = {	}
}
infiniteLLCNedoConvoTemplate:addScreen(wait_q5);


reset = ConvoScreen:new {
	id = "reset",
	leftDialog = "", 
	customDialogText = "ok",
	stopConversation = "true",
	options = {	}
}
infiniteLLCNedoConvoTemplate:addScreen(reset);

addConversationTemplate("infiniteLLCNedoConvoTemplate", infiniteLLCNedoConvoTemplate);
