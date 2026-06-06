-- JTL-DISABLED-2026-03-22
-- Generic conversation handler for JTL NPCs while space travel is disabled.
-- Displays random lore-friendly messages about the Imperial space travel shutdown.

jtlDisabledConvoHandler = conv_handler:new {}

local jtlDisabledResponses = {
	"Due to Imperial guidance, all civilian space travel has been suspended until further notice. Check back soon - we expect clearance any day now.",
	"Sorry, friend. The Empire shut down all non-military space operations in this sector. We're grounded just like you.",
	"I'd love to help you get up there, but Imperial Command has locked down all civilian flight corridors. We're waiting on word from Coruscant.",
	"Space travel? Ha. Not unless you've got Imperial clearance codes. They've got the whole sector on lockdown. Should be temporary, though.",
	"Between you and me, I think the Empire is running some kind of classified operation up there. All flights grounded until they say otherwise.",
	"No ships in or out right now. Imperial order. But keep your license current - when this lifts, we're going to be busy.",
	"The starport authority revoked all civilian launch permits last week. Some Imperial security directive. We're all just waiting it out.",
	"Trust me, nobody wants to get back in the cockpit more than I do. But the Empire says no fly, so no fly. For now.",
}

function jtlDisabledConvoHandler:getInitialScreen(pPlayer, pNpc, pConvTemplate)
	local convoTemplate = LuaConversationTemplate(pConvTemplate)
	return convoTemplate:getScreen("jtl_disabled_greeting")
end

function jtlDisabledConvoHandler:runScreenHandlers(pConvTemplate, pPlayer, pNpc, selectedOption, pConvScreen)
	local screen = LuaConversationScreen(pConvScreen)
	local pClone = screen:cloneScreen()
	local cloned = LuaConversationScreen(pClone)

	local idx = math.random(1, #jtlDisabledResponses)
	cloned:setCustomDialogText(jtlDisabledResponses[idx])

	return pClone
end
