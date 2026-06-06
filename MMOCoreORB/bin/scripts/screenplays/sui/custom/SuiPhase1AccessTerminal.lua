local ObjectManager = require("managers.object.object_manager")
local QuestManager = require("managers.quest.quest_manager")

SuiPhase1AccessTerminal = {
	terminalComponents = { "@quest_item_n:fs_craft_puzzle_config_processor", "@quest_item_n:fs_craft_puzzle_gyro_receiver",
		"@quest_item_n:fs_craft_puzzle_signal_amp", "@quest_item_n:fs_craft_puzzle_solid_state_array" },

	componentTemplates =  { "object/tangible/item/quest/force_sensitive/fs_craft_puzzle_config_processor.iff", "object/tangible/item/quest/force_sensitive/fs_craft_puzzle_gyro_receiver.iff",
		"object/tangible/item/quest/force_sensitive/fs_craft_puzzle_signal_amp.iff", "object/tangible/item/quest/force_sensitive/fs_craft_puzzle_solid_state_array.iff"}
}

function SuiPhase1AccessTerminal:openAccessTerminal(pPlayer, pTerminal)
	local playerID = SceneObject(pPlayer):getObjectID()
	local existingPid = readData(playerID .. ":fsCrafting1:pid")

	if (existingPid ~= 0) then
		CreatureObject(pPlayer):sendSystemMessage("@quest/force_sensitive/fs_crafting:phase1_msg_cant_open_sui")
		return
	end

	local status = { -1, -1, -1, -1 }

	for i = 1, 4, 1 do
		local tempVal = readScreenPlayData(pPlayer, "FsCrafting1", "accessTerminalStatus" .. i)

		if (tempVal == "") then
			writeScreenPlayData(pPlayer, "FsCrafting1", "accessTerminalStatus" .. i, "-1")
		else
			status[i] = tonumber(tempVal)
		end
	end

	local completed = true
	local arrayStatus = "@quest/force_sensitive/fs_crafting:sui_access_terminal_online"

	for i = 1, 4, 1 do
		if (status[i] ~= 1) then
			completed = false
			arrayStatus = "@quest/force_sensitive/fs_crafting:sui_access_terminal_offline"
		end
	end

	local suiPrompt = "@quest/force_sensitive/fs_crafting:sui_access_terminal_prompt0 \n\n"
	suiPrompt = suiPrompt .. "@quest/force_sensitive/fs_crafting:sui_access_terminal_prompt1 " .. arrayStatus .. " \n\n"
	suiPrompt = suiPrompt .. "@quest/force_sensitive/fs_crafting:sui_access_terminal_prompt2 "

	local componentStatus = { }

	for i = 1, 4, 1 do
		componentStatus[i] = self.terminalComponents[i] .. " - "
		if (status[i] == -1) then
			componentStatus[i] = componentStatus[i] .. "@quest/force_sensitive/fs_crafting:sui_access_terminal_damaged"
		elseif (status[i] == 0) then
			componentStatus[i] = componentStatus[i] .. "@quest/force_sensitive/fs_crafting:sui_access_terminal_empty";
		elseif (status[i] == 1) then
			componentStatus[i] = componentStatus[i] .. "@quest/force_sensitive/fs_crafting:sui_access_terminal_online"
		end
	end

	local sui = SuiListBox.new("SuiPhase1AccessTerminal", "accessTerminalCallback")
	sui.setTitle("@quest/force_sensitive/fs_crafting:sui_access_terminal_title")
	sui.setPrompt(suiPrompt)
	sui.showOtherButton()
	sui.setForceCloseDistance(10)
	sui.setTargetNetworkId(SceneObject(pTerminal):getObjectID())

	for i = 1, #componentStatus, 1 do
		sui.add(componentStatus[i], "")
	end

	if (completed) then
		CreatureObject(pPlayer):sendSystemMessage("@quest/force_sensitive/fs_crafting:phase1_msg_quest_finished")

		VillageJediManagerCommon.unlockBranch(pPlayer, "force_sensitive_crafting_mastery_assembly")
		QuestManager.completeQuest(pPlayer, QuestManager.quests.FS_CRAFT_PUZZLE_QUEST_00)
		QuestManager.completeQuest(pPlayer, QuestManager.quests.FS_CRAFT_PUZZLE_QUEST_03)
		VillageJediManagerCommon.setCompletedQuestThisPhase(pPlayer)

		local pInventory = SceneObject(pPlayer):getSlottedObject("inventory")

		if (pInventory ~= nil) then
			local pSculpture = giveItem(pInventory, "object/tangible/item/quest/force_sensitive/fs_sculpture_4.iff", -1, true)

			if (pSculpture == nil) then
				CreatureObject(pPlayer):sendSystemMessage("Error: Unable to generate item.")
			end
		end

		sui.setProperty("btnOk", "Visible", "false")
		sui.setProperty("btnOther", "Visible", "false")
	else
		sui.setProperty("btnOk", "Text", "@quest/force_sensitive/fs_crafting:sui_btn_replace")
		sui.setProperty("btnOther", "Text", "@quest/force_sensitive/fs_crafting:sui_btn_retrieve")
	end

	sui.sendTo(pPlayer)
end

function SuiPhase1AccessTerminal:accessTerminalCallback(pPlayer, pSui, eventIndex, rowIndex, otherPressed)
	if (eventIndex == 1) then
		return
	end

	local pPageData = LuaSuiBoxPage(pSui):getSuiPageData()

	if (pPageData == nil) then
		return
	end

	local suiPageData = LuaSuiPageData(pPageData)

	local terminalID = suiPageData:getTargetNetworkId()
	local pTerminal = getSceneObject(terminalID)
	
	rowIndex = math.floor(rowIndex)

	if (rowIndex == -1) then
		self:openAccessTerminal(pPlayer, pTerminal)
		return
	end

	if (otherPressed == "true") then -- otherPressed is sent by the client as a string
		self:retrieveComponent(pPlayer, rowIndex)
	else
		self:replaceComponent(pPlayer, rowIndex)
	end

	self:openAccessTerminal(pPlayer, pTerminal)
end

function SuiPhase1AccessTerminal:retrieveComponent(pPlayer, rowIndex)
	rowIndex = rowIndex + 1

	if (rowIndex > 4 or rowIndex < 1) then
		return
	end

	local rowStatus = tonumber(readScreenPlayData(pPlayer, "FsCrafting1", "accessTerminalStatus" .. rowIndex))

	if (rowStatus == 0) then
		CreatureObject(pPlayer):sendSystemMessage("@quest/force_sensitive/fs_crafting:phase1_msg_comp_not_installed")
		return
	end

	local pInventory = CreatureObject(pPlayer):getSlottedObject("inventory")

	if (pInventory == nil or SceneObject(pInventory):isContainerFullRecursive()) then
		CreatureObject(pPlayer):sendSystemMessage("@quest/force_sensitive/fs_crafting:phase1_msg_inventory_full")
		return
	end

	local pItem = giveItem(pInventory, self.componentTemplates[rowIndex], -1)

	if (pItem == nil) then
		CreatureObject(pPlayer):sendSystemMessage("Failed to generate component, please try again.")
		return
	end

	if (rowStatus == 1) then
		LuaFsCraftingComponentObject(pItem):setStatus(1)
	else
		LuaFsCraftingComponentObject(pItem):setStatus(-1)
	end

	writeScreenPlayData(pPlayer, "FsCrafting1", "accessTerminalStatus" .. rowIndex, "0")

	CreatureObject(pPlayer):sendSystemMessage("@quest/force_sensitive/fs_crafting:phase1_msg_component_retrieved")
end

function SuiPhase1AccessTerminal:replaceComponent(pPlayer, rowIndex)
	rowIndex = rowIndex + 1

	if (rowIndex > 4 or rowIndex < 1) then
		return
	end

	local rowStatus = tonumber(readScreenPlayData(pPlayer, "FsCrafting1", "accessTerminalStatus" .. rowIndex))

	if (rowStatus ~= 0) then
		CreatureObject(pPlayer):sendSystemMessage("@quest/force_sensitive/fs_crafting:phase1_msg_comp_installed")
		return
	end

	local pInventory = CreatureObject(pPlayer):getSlottedObject("inventory")

	if (pInventory == nil) then
		CreatureObject(pPlayer):sendSystemMessage("Error locating inventory, please try again.")
		return
	end

	local pComponent = getContainerObjectByTemplate(pInventory, self.componentTemplates[rowIndex], true)

	if (pComponent == nil) then
		CreatureObject(pPlayer):sendSystemMessage("@quest/force_sensitive/fs_crafting:phase1_msg_dont_have_component")
		return
	end

	local componentStatus = LuaFsCraftingComponentObject(pComponent):getStatus()

	writeScreenPlayData(pPlayer, "FsCrafting1", "accessTerminalStatus" .. rowIndex, tostring(componentStatus))

	SceneObject(pComponent):destroyObjectFromWorld()
	SceneObject(pComponent):destroyObjectFromDatabase()

	CreatureObject(pPlayer):sendSystemMessage("@quest/force_sensitive/fs_crafting:phase1_msg_component_replaced")
end

-- Teardown for the GM tools: reset the four terminal slots to their initial "damaged" state,
-- clear the open-SUI pid guard, and strip any leftover puzzle component items from inventory.
-- The terminals themselves are persistent quest-gated world objects, so there is nothing to
-- re-spawn — clearing this state plus an active FS_CRAFT_PUZZLE_QUEST_00 fully re-arms them.
function SuiPhase1AccessTerminal:clearCraftingState(pPlayer)
	local playerID = SceneObject(pPlayer):getObjectID()

	for i = 1, 4, 1 do
		writeScreenPlayData(pPlayer, "FsCrafting1", "accessTerminalStatus" .. i, "-1")
	end

	deleteData(playerID .. ":fsCrafting1:pid")

	local pInventory = CreatureObject(pPlayer):getSlottedObject("inventory")

	if (pInventory ~= nil) then
		for i = 1, #self.componentTemplates, 1 do
			for attempt = 1, 8, 1 do
				local pComponent = getContainerObjectByTemplate(pInventory, self.componentTemplates[i], true)

				if (pComponent == nil) then
					break
				end

				SceneObject(pComponent):destroyObjectFromWorld()
				SceneObject(pComponent):destroyObjectFromDatabase()
			end
		end
	end
end

-- GM tool: drive the player to the exact legit Quharek Crafting finish-state (the `completed`
-- branch of openAccessTerminal). Each step is guarded for a safe re-run.
function SuiPhase1AccessTerminal:forceComplete(pPlayer)
	if (pPlayer == nil) then
		return
	end

	if (not VillageJediManagerCommon.hasUnlockedBranch(pPlayer, "force_sensitive_crafting_mastery_assembly")) then
		VillageJediManagerCommon.unlockBranch(pPlayer, "force_sensitive_crafting_mastery_assembly")
	end

	-- Legit finish completes _00 + _03 at the terminal; _01/_02 complete earlier via goto1/goto2.
	-- Complete all four (guarded) so the journal matches a real finisher from any starting point.
	for i = 0, 3, 1 do
		local questID = QuestManager.quests["FS_CRAFT_PUZZLE_QUEST_0" .. i]

		if (not QuestManager.hasCompletedQuest(pPlayer, questID)) then
			QuestManager.completeQuest(pPlayer, questID)
		end
	end

	self:clearCraftingState(pPlayer)

	local pInventory = SceneObject(pPlayer):getSlottedObject("inventory")

	if (pInventory ~= nil and getContainerObjectByTemplate(pInventory, "object/tangible/item/quest/force_sensitive/fs_sculpture_4.iff", true) == nil) then
		local pSculpture = giveItem(pInventory, "object/tangible/item/quest/force_sensitive/fs_sculpture_4.iff", -1, true)

		if (pSculpture == nil) then
			CreatureObject(pPlayer):sendSystemMessage("Error: Unable to generate item.")
		end
	end

	VillageJediManagerCommon.setCompletedQuestThisPhase(pPlayer)
end

-- GM/player tool: DROP the quest entirely. Reset the crafting sub-quests, clear terminal/component
-- state, finish any in-progress goto guidance, and clear the active-quest-this-phase flag so the
-- player has no quest chosen for the phase. NOT restart-in-place: does NOT re-activate or re-arm,
-- and FS branches are NOT re-locked.
function SuiPhase1AccessTerminal:forceDrop(pPlayer)
	if (pPlayer == nil) then
		return
	end

	for i = 0, 3, 1 do
		QuestManager.resetQuest(pPlayer, QuestManager.quests["FS_CRAFT_PUZZLE_QUEST_0" .. i])
	end

	self:clearCraftingState(pPlayer)

	-- Tear down any in-progress goto guidance so the drop doesn't leave orphaned goto active-areas
	-- or a stale waypoint behind.
	if (FsCrafting1Goto1:hasTaskStarted(pPlayer)) then
		FsCrafting1Goto1:finish(pPlayer)
	end

	if (FsCrafting1Goto2:hasTaskStarted(pPlayer)) then
		FsCrafting1Goto2:finish(pPlayer)
	end

	QuestManager.setCurrentVillageQuestID(pPlayer, 0)

	VillageJediManagerCommon.clearActiveQuestThisPhase(pPlayer)
end
