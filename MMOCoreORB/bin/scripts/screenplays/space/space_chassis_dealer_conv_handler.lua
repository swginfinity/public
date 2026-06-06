-- Infinity: Stub conversation handler - pending implementation from swgemu unstable
-- ConvoTemplate/ConvoScreen not available in screenplay load context, so just register the handler
space_chassis_dealer_conv_handler = conv_handler:new {}

function space_chassis_dealer_conv_handler:getInitialScreen(pPlayer, pNpc, pConvTemplate)
	return nil
end

function space_chassis_dealer_conv_handler:runScreenHandlers(pConvTemplate, pPlayer, pNpc, selectedOption, pConvScreen)
	return nil
end
