-- Legend of Hondo Merchant System
-- By R. Bassett Jr. (Tatwi) - www.tpot.ca
-- July 2015
-- Adapted by Qrave for Infinity March 2018

local ObjectManager = require("managers.object.object_manager")

pandora = ScreenPlay:new {
	numberOfActs = 1,
	relations = {
		{name="nym", npcStanding=-9000, priceAdjust=15}, -- Adjust price only
		{name="lok_mercenaries", npcStanding=-9000, priceAdjust=5}, -- Adjust price only
		{name="bloodrazor", npcStanding=-1000, priceAdjust=20},  -- Enemy
		{name="canyon_corsair", npcStanding=-1000, priceAdjust=20}  -- Enemy
	},
	goods = {
		{optName="rngbox1", cost=500000, itemName="Mysterious Container", items={"object/tangible/container/loot/mysterious_container.iff"}},
		{optName="rngbox2", cost=1000000, itemName="Large Mysterious Container", items={"object/tangible/container/loot/large_mysterious_container.iff"}},
		{optName="rngbox3", cost=1500000, itemName="Glowing Mysterious Container", items={"object/tangible/container/loot/glowing_mysterious_container.iff"}},
	},
}


registerScreenPlay("pandora", true)

function pandora:start()
	--spawnMobile("tatooine", "pandora", 1, -2811.26, 5, 2161.16, 316, 0)
end

pandora_convo_handler = Object:new {
	tstring = "myconversation"
 }

function pandora_convo_handler:getNextConversationScreen(conversationTemplate, conversingPlayer, selectedOption)
	nextConversationScreen = MerchantSystem:nextConvoScreenInnards(conversationTemplate, conversingPlayer, selectedOption, pandora.relations, pandora.goods)

	return nextConversationScreen
end

function pandora_convo_handler:runScreenHandlers(conversationTemplate, conversingPlayer, conversingNPC, selectedOption, conversationScreen)
	conversationScreen = MerchantSystem:runScreenHandlerInnards(conversationTemplate, conversingPlayer, conversingNPC, selectedOption, conversationScreen, pandora.relations, pandora.goods)

	return conversationScreen
end
