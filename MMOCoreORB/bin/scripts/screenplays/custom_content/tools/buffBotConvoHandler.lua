local ObjectManager = require("managers.object.object_manager")

BuffBotConvoHandler = conv_handler:new {}

function BuffBotConvoHandler:runScreenHandlers(pConvTemplate, pPlayer, pNpc, selectedOption, pConvScreen)
	local screen = LuaConversationScreen(pConvScreen)
	local screenID = screen:getScreenID()
	local convoTemplate = LuaConversationTemplate(pConvTemplate)
	
	if screenID == "healWounds" then
		buffBotScreenPlay:healWounds(pPlayer)
	elseif screenID == "enhanceStats" then
		buffBotScreenPlay:applyEnhancements(pPlayer)
	elseif screenID == "clearEnhancements" then
		buffBotScreenPlay:clearEnhancements(pPlayer)
	elseif screenID == "clearDots" then
		buffBotScreenPlay:clearDots(pPlayer)
	elseif screenID == "healDamage" then
		buffBotScreenPlay:healDamage(pPlayer)
	end
	return pConvScreen
end

function BuffBotConvoHandler:getInitialScreen(pPlayer, pNpc, pConvTemplate)
	local convoTemplate = LuaConversationTemplate(pConvTemplate)

	return convoTemplate:getScreen("convoStart")
end