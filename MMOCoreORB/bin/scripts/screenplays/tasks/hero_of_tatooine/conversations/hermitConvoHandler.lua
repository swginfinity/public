local ObjectManager = require("managers.object.object_manager")

heroOfTatHermitConvoHandler = conv_handler:new {

    markOfTheHeroRing = "object/tangible/wearables/ring/ring_mark_hero.iff"
}

--[[
1 - Started squill quest
2 - Completed squill quest
4 - Completed Altruism
8 - Completed Intellect
16 - Completed Courage
32 - Completed Honor
64 - Completed Mark of Hero
]]

function heroOfTatHermitConvoHandler:getInitialScreen(pPlayer, pNpc, pConvTemplate)
	local convoTemplate = LuaConversationTemplate(pConvTemplate)

    if not isPlayer(pPlayer) then
        return convoTemplate:getScreen("intro")
    end

    local player = LuaCreatureObject(pPlayer)
	if HeroOfTatooineScreenPlay:isMissingMark(pPlayer) and not player:hasScreenPlayState(64, "hero_of_tatooine") then
		return convoTemplate:getScreen("you_have_returned_missing_marks")
	elseif player:hasScreenPlayState(64, "hero_of_tatooine") then
		return convoTemplate:getScreen("hero_returns")
	elseif player:hasScreenPlayState(2, "hero_of_tatooine") then
		return convoTemplate:getScreen("you_have_returned")
	elseif player:hasScreenPlayState(1, "hero_of_tatooine") then
		return convoTemplate:getScreen("return_intro")
	else
		return convoTemplate:getScreen("intro")
	end
end

function heroOfTatHermitConvoHandler:runScreenHandlers(pConvTemplate, pPlayer, pNpc, selectedOption, pConvScreen)

    if not isPlayer(pPlayer) then
        return pConvScreen
    end

    local player = LuaCreatureObject(pPlayer)

	local pGhost = player:getPlayerObject()

	if not pGhost then
		return pConvScreen
	end

	local screen = LuaConversationScreen(pConvScreen)

	local screenID = screen:getScreenID()

	local pConvScreen = screen:cloneScreen()
	local clonedConversation = LuaConversationScreen(pConvScreen)
	if screenID == "hero_returns" then
		-- add option to turn in ring and reset
		clonedConversation:addOption("I would like to turn in my ring and start again", "reset1")
	elseif screenID == "reset2" then
		-- if they have the ring, take it
		if util:hasInventory(pPlayer, self.markOfTheHeroRing) then
			if util:removeInventory(pPlayer, self.markOfTheHeroRing) then
				HeroOfTatooineScreenPlay:reset(pPlayer)
			else
				clonedConversation:setCustomDialogText("There was an error while trying to take back the ring. If you still have the ring, try again. If not, create a support ticket. This incident has been logged.")
				util:l("Hero of Tatooine reset error", pPlayer, 3)
			end
		else
			clonedConversation:setCustomDialogText("You do not have the ring, it must be in your inventory, not on your hand or in a backpack!")
		end
	elseif screenID == "go_then" then
		player:setScreenPlayState(1, "hero_of_tatooine")
	elseif screenID == "you_have_returned_missing_marks" then
		local pInventory = player:getSlottedObject("inventory")

		if not pInventory or SceneObject(pInventory):isContainerFullRecursive() then
			clonedConversation:addOption("@conversation/quest_hero_of_tatooine_hermit:s_6441a2a6", "inv_full")
		else
			if HeroOfTatooineScreenPlay:giveMissingMarks(pPlayer) then
				clonedConversation:addOption("@conversation/quest_hero_of_tatooine_hermit:s_6441a2a6", "now_have_items")
			else
				clonedConversation:addOption("@conversation/quest_hero_of_tatooine_hermit:s_6441a2a6", "dont_have_room")
			end
		end
	elseif screenID == "you_have_returned" then
		if player:hasScreenPlayState(2, "hero_of_tatooine") and player:hasScreenPlayState(4, "hero_of_tatooine") and player:hasScreenPlayState(8, "hero_of_tatooine")
			and player:hasScreenPlayState(16, "hero_of_tatooine") and player:hasScreenPlayState(32, "hero_of_tatooine") then
			clonedConversation:addOption("@conversation/quest_hero_of_tatooine_hermit:s_cd2c070a", "you_bear_marks")
		end
		clonedConversation:addOption("@conversation/quest_hero_of_tatooine_hermit:s_503d468c", "which_mark")
		clonedConversation:addOption("@conversation/quest_hero_of_tatooine_hermit:s_da196589", "as_you_wish")
	elseif screenID == "done_great_things" then

		local pInventory = player:getSlottedObject("inventory")

		if not pInventory then
			return
		end

		if SceneObject(pInventory):isContainerFullRecursive() then
			player:setScreenPlayState(16, "hero_of_tatooine_missing_marks") -- 1 - Altruism, 2 - Intellect, 4 - Courage, 8 - Honor, 16 - Ring
			player:sendSystemMessage("@quest/hero_of_tatooine/system_messages:hero_inv_full")
		else
			local pMark = giveItem(pInventory, self.markOfTheHeroRing, -1)

			if not pMark then
				player:setScreenPlayState(16, "hero_of_tatooine_missing_marks") -- 1 - Altruism, 2 - Intellect, 4 - Courage, 8 - Honor, 16 - Ring
				player:sendSystemMessage("Error creating Mark of the Hero object. Please create a support ticket.")
			else
				TangibleObject(pMark):setOptionBit(INSURED)
                player:sendSystemMessage("@quest/hero_of_tatooine/system_messages:hero_success")
			end
		end

        if not PlayerObject(pGhost):hasBadge(POI_HEROMARK) then
	        PlayerObject(pGhost):awardBadge(POI_HEROMARK)
        end
		player:setScreenPlayState(64, "hero_of_tatooine")
		CreatureObject(pNpc):doAnimation("worship")
	elseif screenID == "return_intro" then
		local pInventory =  player:getSlottedObject("inventory")

		if pInventory then
			local pSkull = getContainerObjectByTemplate(pInventory, "object/tangible/loot/quest/hero_of_tatooine/squill_skull.iff", true)

			if pSkull then
				clonedConversation:addOption("@conversation/quest_hero_of_tatooine_hermit:s_af540d0", "proven_worthy")
			end
		end
		clonedConversation:addOption("@conversation/quest_hero_of_tatooine_hermit:s_da196589", "as_you_wish")
	elseif screenID == "proven_worthy" then
		local pInventory = player:getSlottedObject("inventory")

		if pInventory then
			local pSkull = getContainerObjectByTemplate(pInventory, "object/tangible/loot/quest/hero_of_tatooine/squill_skull.iff", true)

			if not pSkull then
				return 0
			end

			SceneObject(pSkull):destroyObjectFromWorld()
			SceneObject(pSkull):destroyObjectFromDatabase()
			player:setScreenPlayState(2, "hero_of_tatooine")
			CreatureObject(pNpc):doAnimation("taken_aback")
		end
	elseif (screenID == "intro") then
		CreatureObject(pNpc):doAnimation("greet")
	end

	return pConvScreen
end
