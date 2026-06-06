hedon_istee_missions =
{
	{
		missionType = "assassinate",
		primarySpawns = 
		{ 
			{ npcTemplate = "drognuz", npcName = "Drognuz" } 
		}, 
		secondarySpawns = {},
		itemSpawns = {}, 
		rewards = 
		{ 
			{ rewardType = "credits", amount = 3000 } 
		}
	},
	{
		missionType = "assassinate",
		primarySpawns = 
		{ 
			{ npcTemplate = "rath_kanaz", npcName = "Rath Kanaz" } 
		}, 
		secondarySpawns = {},
		itemSpawns = {}, 
		rewards = 
		{ 
			{ rewardType = "credits", amount = 6000 } 
		}
	},
	{
		missionType = "deliver", 
		primarySpawns = 
		{ 
			{ npcTemplate = "serene_floater", npcName = "Serene Floater" } 
		}, 
		secondarySpawns = {}, 
		itemSpawns = 
		{
			{ itemTemplate = "object/tangible/mission/quest_item/hedon_istee_q3_needed.iff", itemName = "B'omarr Sacred Scroll" }
		}, 
		rewards = 
		{ 
			{ rewardType = "loot",  lootGroup = "task_reward_hedon_istee" },
		}
	}
}

npcMapHedonIstee = 
{ 
	{ 
		spawnData = { npcTemplate = "hedon_istee", x = 1390, z = 7, y = 3197, direction = 0, cellID = 0, position = STAND, flags = AI_STATIC },
		npcNumber = 1,
		stfFile = "@static_npc/tatooine/hedon_istee",
		missions = hedon_istee_missions
	},
}

HedonIstee = ThemeParkLogic:new {
	npcMap = npcMapHedonIstee,
	className = "HedonIstee",
	screenPlayState = "hedon_istee_task",
	requiresEliteCombat = true,
	planetName = "tatooine",
	distance = 800,
}

registerScreenPlay("HedonIstee", true)

hedon_istee_mission_giver_conv_handler = mission_giver_conv_handler:new {
	themePark = HedonIstee
}
hedon_istee_mission_target_conv_handler = mission_target_conv_handler:new {
	themePark = HedonIstee
}

-- Infinity: GM-only admin overrides on Hedon's conversation handler.
-- handleScreenInit recurses via self:runScreenHandlers; _gm_recursing sentinel
-- ensures options are injected once at top-level, not per recursion frame.
local hedonBaseRunScreenHandlers = mission_giver_conv_handler.runScreenHandlers

function hedon_istee_mission_giver_conv_handler:runScreenHandlers(pConvTemplate, pPlayer, pNpc, selectedOption, pConvScreen)
	if (pPlayer == nil or pConvScreen == nil) then
		return hedonBaseRunScreenHandlers(self, pConvTemplate, pPlayer, pNpc, selectedOption, pConvScreen)
	end

	-- Per-player sentinel; the handler table is a singleton shared across
	-- concurrent conversations, so a scalar flag would race between players.
	local playerKey = SceneObject(pPlayer):getObjectID()
	self._gm_recursing = self._gm_recursing or {}
	local topLevel = not self._gm_recursing[playerKey]
	self._gm_recursing[playerKey] = true

	local screen = LuaConversationScreen(pConvScreen)
	local screenID = screen:getScreenID()

	local result
	if screenID == "gm_give_map" then
		result = self:handleGmGiveMap(pConvTemplate, pPlayer, pNpc, selectedOption, pConvScreen)
	elseif screenID == "gm_reset_target" then
		result = self:handleGmResetTarget(pConvTemplate, pPlayer, pNpc, selectedOption, pConvScreen)
	elseif screenID == "gm_give_scythe" then
		result = self:handleGmGiveScythe(pConvTemplate, pPlayer, pNpc, selectedOption, pConvScreen)
	else
		result = hedonBaseRunScreenHandlers(self, pConvTemplate, pPlayer, pNpc, selectedOption, pConvScreen)
	end

	if topLevel then
		self._gm_recursing[playerKey] = nil
		if result ~= nil and util:isAdmin(pPlayer) then
			local resultScreen = LuaConversationScreen(result)
			resultScreen:setStopConversation(false)
			resultScreen:addOption("[MGMT] Give krayt treasure map and reset my quest", "gm_give_map")
			resultScreen:addOption("[MGMT] Reset target's quest", "gm_reset_target")
			resultScreen:addOption("[MGMT] Give scythe schematic", "gm_give_scythe")
		end
	end

	return result
end

function hedon_istee_mission_giver_conv_handler:handleGmGiveMap(pConvTemplate, pPlayer, pNpc, selectedOption, pConvScreen)
	local screen = LuaConversationScreen(pConvScreen)
	pConvScreen = screen:cloneScreen()
	local clonedScreen = LuaConversationScreen(pConvScreen)

	self.themePark:resetThemePark(pPlayer)
	self.themePark:giveLoot(pPlayer, "task_reward_hedon_istee")
	clonedScreen:setCustomDialogText("[MGMT] Hedon quest fully reset. A fresh krayt treasure map has been placed in your inventory.")
	clonedScreen:removeAllOptions()

	return pConvScreen
end

function hedon_istee_mission_giver_conv_handler:handleGmGiveScythe(pConvTemplate, pPlayer, pNpc, selectedOption, pConvScreen)
	local screen = LuaConversationScreen(pConvScreen)
	pConvScreen = screen:cloneScreen()
	local clonedScreen = LuaConversationScreen(pConvScreen)

	self.themePark:giveLoot(pPlayer, "hedon_istee_treasure")
	clonedScreen:setCustomDialogText("[MGMT] Scythe schematic placed in your inventory.")
	clonedScreen:removeAllOptions()

	return pConvScreen
end

function hedon_istee_mission_giver_conv_handler:handleGmResetTarget(pConvTemplate, pPlayer, pNpc, selectedOption, pConvScreen)
	local screen = LuaConversationScreen(pConvScreen)
	pConvScreen = screen:cloneScreen()
	local clonedScreen = LuaConversationScreen(pConvScreen)

	local dialogText
	local targetID = CreatureObject(pPlayer):getTargetID()
	local pTarget = getSceneObject(targetID)

	if pTarget ~= nil and SceneObject(pTarget):isPlayerCreature() then
		self.themePark:resetThemePark(pTarget)
		dialogText = "[MGMT] Reset Hedon quest on " .. SceneObject(pTarget):getCustomObjectName() .. "."
	else
		dialogText = "[MGMT] No valid player target. Select a player first, then click again."
	end

	clonedScreen:setCustomDialogText(dialogText)
	clonedScreen:removeAllOptions()

	return pConvScreen
end
