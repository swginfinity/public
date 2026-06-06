local ObjectManager = require("managers.object.object_manager")

BiogenicSecurityTechConvoHandler = conv_handler:new {}

function BiogenicSecurityTechConvoHandler:runScreenHandlers(pConvTemplate, pPlayer, pNpc, selectedOption, pConvScreen)
	local screen = LuaConversationScreen(pConvScreen)
	local screenID = screen:getScreenID()
	local convoTemplate = LuaConversationTemplate(pConvTemplate)
	if screenID == "sith_spit" or screenID == "seismic_event" or screenID == "forget_you_saw" then
		writeData(CreatureObject(pPlayer):getObjectID() .. ":geoSecurityTechState", 1)
	elseif screenID == "first_need_code" or screenID == "could_use_help" or screenid == "your_funeral" then
		writeData(CreatureObject(pPlayer):getObjectID() .. ":geoSecurityTechState", 2)
	-- Infinity Custom code generator
	elseif screenID == "first_need_code" then
		local code = GeonosianLabScreenPlay:getCode(2)
		clonedConversation:setCustomDialogText("Well, firstly you'll need a code to get through that security door. Punch in the code " .. tostring(code) .. " and you can get through. Once you get in, there are some things you should watch out for though.")
	elseif screenID == "could_use_help" then
		local code = GeonosianLabScreenPlay:getCode(2)
		clonedConversation:setCustomDialogText("Actually, I could use some help, but if you're going further into the compound, you'll need a code to get through that security door. Punch in the code " .. tostring(code) .. " and you can get through. There are some things you should watch out for though.")
	elseif screenID == "your_funeral" then
		local code = GeonosianLabScreenPlay:getCode(2)
		clonedConversation:setCustomDialogText("It's your funeral. You'll need to punch in the code " .. tostring(code) .. " to get through that security door. Just make sure none of those creatures gets in here before I get security back online.")
	elseif screenID == "just_punch_code" then
		local code = GeonosianLabScreenPlay:getCode(2)
		clonedConversation:setCustomDialogText("Just punch in the code " .. tostring(code) .. " to get through the security door.")
	elseif screenID == "init_dont_recommend" then
		local code = GeonosianLabScreenPlay:getCode(2)
		clonedConversation:setCustomDialogText("Remember, I don't recommend it. But if you want to get through the security door, punch in the code " .. tostring(code))
	end
	return pConvScreen
end

function BiogenicSecurityTechConvoHandler:getInitialScreen(pPlayer, pNpc, pConvTemplate)
	local convoTemplate = LuaConversationTemplate(pConvTemplate)
	local techState = readData(CreatureObject(pPlayer):getObjectID() .. ":geoSecurityTechState")
	if (techState == nil or techState == 0) then
		return convoTemplate:getScreen("init_firsttalk")
	elseif (techState == 1) then
		return convoTemplate:getScreen("im_very_busy")
	elseif (techState == 2) then
		return convoTemplate:getScreen("init_anything_else")
	end

	return convoTemplate:getScreen("init_firsttalk")
end
