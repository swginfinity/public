local ObjectManager = require("managers.object.object_manager")

BiogenicScientistHumanConvoHandler = conv_handler:new {}

function BiogenicScientistHumanConvoHandler:runScreenHandlers(pConvTemplate, pPlayer, pNpc, selectedOption, pConvScreen)
	local screen = LuaConversationScreen(pConvScreen)
	local screenID = screen:getScreenID()
	local convoTemplate = LuaConversationTemplate(pConvTemplate)
	local clonedConversation = LuaConversationScreen(pConvScreen)

	if screenID == "here_it_is" or screenID == "no_i_want_you_to" or screenID == "no_i_insist" or screenID == "no_youre_right" then
		writeData(CreatureObject(pPlayer):getObjectID() .. ":geoHumanScientistState", 2)
		GeonosianLabScreenPlay:giveGeoItem(pPlayer, "object/tangible/loot/loot_schematic/geonosian_tenloss_dxr6_schematic.iff")
	elseif screenID == "killed_quite_a_few" or screenID == "well_yes" or screenID == "thank_you_wait" then
		writeData(CreatureObject(pPlayer):getObjectID() .. ":geoHumanScientistState", 1)
	elseif screenID == "since_youre_here" or screenID == "i_hope_so" or screenID == "no_problem_at_all" then
		if screenID == "since_youre_here" then
			local code = GeonosianLabScreenPlay:getCode(5)
			clonedConversation:setCustomDialogText("Since you're here, could you please check on my colleagues in Genetics Lab One. If they were still in there when the doors locked down they might be in trouble. You can unlock the door with the code " .. tostring(code) .. " if necessary.")
		end
		if screenID == "i_hope_so" then
			local code = GeonosianLabScreenPlay:getCode(5)
			clonedConversation:setCustomDialogText("I hope so.  If you can, please check on my collegues in Genetics Lab One. If they were still in there when the doors locked down they might be in trouble. You can unlock the door with the code " .. tostring(code) .. " if necessary.")
		end
		if screenID == "no_problem_at_all" then
			local code = GeonosianLabScreenPlay:getCode(5)
			clonedConversation:setCustomDialogText("No problem at all. If you can, please check on my colleagues in Genetics Lab One. If they were still in there when the doors locked down they might be in trouble. You can unlock the door with the code " .. tostring(code) .. " if necessary.")
		end
		writeData(CreatureObject(pPlayer):getObjectID() .. ":geoHumanScientistState", 3)
	elseif screenID == "go_on_ahead" then
		local code = GeonosianLabScreenPlay:getCode(5)
		clonedConversation:setCustomDialogText("You go on ahead. I'm just going to rest here for a moment. Don't forget, if you need to get into Genetics Lab One, you can use the code " .. tostring(code) .. ".")
	end
	util:cw(screenID, 1)
	return pConvScreen
end

function BiogenicScientistHumanConvoHandler:getInitialScreen(pPlayer, pNpc, pConvTemplate)
	local convoTemplate = LuaConversationTemplate(pConvTemplate)
	local scientistState = readData(CreatureObject(pPlayer):getObjectID() .. ":geoHumanScientistState")

	local startTemplate = "thanks_so_much"
	if (scientistState == nil or scientistState == 0) then
		startTemplate = "thanks_so_much"
	elseif (scientistState == 3) then
		startTemplate = "go_on_ahead"
	elseif (scientistState == 2) then
		startTemplate = "since_youre_here"
	elseif (scientistState == 1) then
		startTemplate = "glad_youre_back"
	end
	util:cw(startTemplate, 1)
	return convoTemplate:getScreen(startTemplate)
end
