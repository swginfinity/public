local ObjectManager = require("managers.object.object_manager")

medicalBotConvoHandler = conv_handler:new {}

function medicalBotConvoHandler:runScreenHandlers(pConvTemplate, pPlayer, pNpc, selectedOption, pConvScreen)
	local screen = LuaConversationScreen(pConvScreen)
	local screenID = screen:getScreenID()
	local convoTemplate = LuaConversationTemplate(pConvTemplate)
	
	if screenID == "healWounds" then
		medicalBotScreenPlay:healWounds(pPlayer)
	elseif screenID == "clearDots" then
		medicalBotScreenPlay:clearDots(pPlayer)
	end
	return pConvScreen
end

function medicalBotConvoHandler:getInitialScreen(pPlayer, pNpc, pConvTemplate)
	local convoTemplate = LuaConversationTemplate(pConvTemplate)

	return convoTemplate:getScreen("convoStart")
end