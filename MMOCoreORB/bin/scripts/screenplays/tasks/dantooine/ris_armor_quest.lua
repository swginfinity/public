local ObjectManager = require("managers.object.object_manager")

risArmorQuest = ScreenPlay:new {

		questSchematics = {
			quest1 = { template = "object/draft_schematic/armor/armor_segment_ris.iff", displayName = "@craft_clothing_ingredients_n:armor_segment_ris" },
			quest3 = { template = "object/draft_schematic/armor/component/armor_layer_ris.iff", displayName = "@craft_clothing_ingredients_n:armor_layer_ris" },
			quest5 = { template = "object/draft_schematic/clothing/clothing_armor_ris_boots.iff", displayName = "@wearables_name:armor_ris_boots" }
		},


		questItems = {
			quest2 = { template = "object/tangible/component/armor/armor_segment_ris.iff", displayName = "@craft_clothing_ingredients_n:armor_segment_ris" },
			quest4 = { template = "object/tangible/component/armor/armor_layer_ris.iff", displayName = "@craft_clothing_ingredients_n:armor_layer_ris" },
			quest6 = { template = "object/tangible/wearables/armor/ris/armor_ris_boots.iff", displayName = "@wearables_name:armor_ris_boots" }
		},


		finalRewardSchematics = {
			{ template = "object/draft_schematic/clothing/clothing_armor_ris_bicep_l.iff", displayName = "@wearables_name:armor_ris_bicep_l" },
			{ template = "object/draft_schematic/clothing/clothing_armor_ris_bicep_r.iff", displayName = "@wearables_name:armor_ris_bicep_r" },
			{ template = "object/draft_schematic/clothing/clothing_armor_ris_bracer_l.iff", displayName = "@wearables_name:armor_ris_bracer_l" },
			{ template = "object/draft_schematic/clothing/clothing_armor_ris_bracer_r.iff", displayName = "@wearables_name:armor_ris_bracer_r" },
			{ template = "object/draft_schematic/clothing/clothing_armor_ris_chest_plate.iff", displayName = "@wearables_name:armor_ris_chest_plate" },
			{ template = "object/draft_schematic/clothing/clothing_armor_ris_gloves.iff", displayName = "@wearables_name:armor_ris_gloves" },
			{ template = "object/draft_schematic/clothing/clothing_armor_ris_helmet.iff", displayName = "@wearables_name:armor_ris_helmet" },
			{ template = "object/draft_schematic/clothing/clothing_armor_ris_leggings.iff", displayName = "@wearables_name:armor_ris_leggings" }
		},
		BHarmorRewardSchematics = {
			{ template = "object/draft_schematic/armor/armor_bounty_hunter_crafted_belt_schematic.iff", displayName = "@wearables_name:bounty_hunter_crafted_belt" },
			{ template = "object/draft_schematic/armor/armor_bounty_hunter_crafted_bicep_l_schematic.iff", displayName = "@wearables_name:bounty_hunter_crafted_bicep_l" },
			{ template = "object/draft_schematic/armor/armor_bounty_hunter_crafted_bicep_r_schematic.iff", displayName = "@wearables_name:bounty_hunter_crafted_bicep_r" },
			{ template = "object/draft_schematic/armor/armor_bounty_hunter_crafted_boots_schematic.iff", displayName = "@wearables_name:bounty_hunter_crafted_boots" },
			{ template = "object/draft_schematic/armor/armor_bounty_hunter_crafted_bracer_l_schematic.iff", displayName = "@wearables_name:bounty_hunter_crafted_bracer_l" },
			{ template = "object/draft_schematic/armor/armor_bounty_hunter_crafted_bracer_r_schematic.iff", displayName = "@wearables_name:bounty_hunter_crafted_bracer_r" },
			{ template = "object/draft_schematic/armor/armor_bounty_hunter_crafted_chest_plate_schematic.iff", displayName = "@wearables_name:bounty_hunter_crafted_chest_plate" },
			{ template = "object/draft_schematic/armor/armor_bounty_hunter_crafted_gloves_schematic.iff", displayName = "@wearables_name:bounty_hunter_crafted_gloves" },
			{ template = "object/draft_schematic/armor/armor_bounty_hunter_crafted_helmet_schematic.iff", displayName = "@wearables_name:bounty_hunter_crafted_helmet" },
			{ template = "object/draft_schematic/armor/armor_bounty_hunter_crafted_leggings_schematic.iff", displayName = "@wearables_name:bounty_hunter_crafted_leggings" },
		}
}

registerScreenPlay("risArmorQuest", true)

--------------------------------------
--   Initialize screenplay           -
--------------------------------------
function risArmorQuest:start()
	if (isZoneEnabled("dantooine")) then
		self:spawnMobiles()
	end
end

function risArmorQuest:spawnMobiles()
	spawnMobile("dantooine", "mol_nimai", 1, -6805, 126, 6022, 179, 0)
end


risArmorQuestConvoHandler = conv_handler:new {}

function risArmorQuestConvoHandler:getInitialScreen(pPlayer, pNpc, pConvTemplate)
	-- 1 = quest 1, 2 = quest 2, 4 = quest 3, 8 = quest 4, 16 = quest 5, 32 = quest 6, 64 = completed
	local convoTemplate = LuaConversationTemplate(pConvTemplate)

	if (not CreatureObject(pPlayer):hasSkill("crafting_armorsmith_master")) then
		return convoTemplate:getScreen("not_master_armorsmith")
	elseif (CreatureObject(pPlayer):hasScreenPlayState(256, "ris_armor_quest")) then -- All quests completed
		return convoTemplate:getScreen("quest_7_complete")
	elseif (CreatureObject(pPlayer):hasScreenPlayState(128, "ris_armor_quest")) then -- All quests completed
		return convoTemplate:getScreen("quest_7_query")
	elseif (CreatureObject(pPlayer):hasScreenPlayState(64, "ris_armor_quest")) then -- All quests completed
		return convoTemplate:getScreen("completed_all_quests")
	elseif (CreatureObject(pPlayer):hasScreenPlayState(32, "ris_armor_quest")) then -- On quest 6
		return convoTemplate:getScreen("quest_6_query")
	elseif (CreatureObject(pPlayer):hasScreenPlayState(16, "ris_armor_quest")) then
		return convoTemplate:getScreen("quest_5_start")
	elseif (CreatureObject(pPlayer):hasScreenPlayState(8, "ris_armor_quest")) then
		return convoTemplate:getScreen("quest_4_query")
	elseif (CreatureObject(pPlayer):hasScreenPlayState(4, "ris_armor_quest")) then
		return convoTemplate:getScreen("quest_3_start")
	elseif (CreatureObject(pPlayer):hasScreenPlayState(2, "ris_armor_quest")) then
		return convoTemplate:getScreen("quest_2_query")
	elseif (CreatureObject(pPlayer):hasScreenPlayState(1, "ris_armor_quest")) then
		return convoTemplate:getScreen("quest_1_description")
	else
		return convoTemplate:getScreen("quest_1_start")
	end
end

function risArmorQuestConvoHandler:runScreenHandlers(pConvTemplate, pPlayer, pNpc, selectedOption, pConvScreen)
	local screen = LuaConversationScreen(pConvScreen)
	local screenID = screen:getScreenID()
	local pConvScreen = screen:cloneScreen()
	local clonedConversation = LuaConversationScreen(pConvScreen)

	if (screenID == "quest_1_start") then
		clonedConversation:setDialogTextDI("50000")
		if (CreatureObject(pPlayer):getCashCredits() < 50000) then
			clonedConversation:addOption("@quest_armorsmith:yes_to_quest_1_start", "not_enough_money")
		else
			clonedConversation:addOption("@quest_armorsmith:yes_to_quest_1_start", "quest_1_start_yes")
		end
		clonedConversation:addOption("@quest_armorsmith:no_to_quest_1_start", "quest_1_start_no")
	elseif (screenID == "quest_1_start_yes") then
		CreatureObject(pPlayer):subtractCashCredits(50000)
		CreatureObject(pPlayer):setScreenPlayState(1, "ris_armor_quest")
	elseif (screenID == "quest_1_description") then
		self:teachSchematic(pPlayer, risArmorQuest.questSchematics.quest1)
		CreatureObject(pPlayer):setScreenPlayState(2, "ris_armor_quest")
	elseif (screenID == "quest_2_query") then
		clonedConversation:setDialogTextTO(risArmorQuest.questItems.quest2.displayName)
		if (self:hasQuestItem(pPlayer, risArmorQuest.questItems.quest2.template)) then
			clonedConversation:addOption("@quest_armorsmith:yes_to_quest_2_query", "quest_2_complete")
		else
			clonedConversation:addOption("@quest_armorsmith:yes_to_quest_2_query", "illegal_response")
		end
		clonedConversation:addOption("@quest_armorsmith:no_to_quest_2_query", "quest_2_incomplete")
	elseif (screenID == "quest_2_complete") then
		CreatureObject(pPlayer):setScreenPlayState(4, "ris_armor_quest")
		self:removeQuestItem(pPlayer, risArmorQuest.questItems.quest2.template)
	elseif (screenID == "quest_3_start") then
		self:teachSchematic(pPlayer, risArmorQuest.questSchematics.quest3)
		CreatureObject(pPlayer):setScreenPlayState(8, "ris_armor_quest")
	elseif (screenID == "quest_4_query") then
		clonedConversation:setDialogTextTO(risArmorQuest.questItems.quest4.displayName)
		if (self:hasQuestItem(pPlayer, risArmorQuest.questItems.quest4.template)) then
			clonedConversation:addOption("@quest_armorsmith:yes_to_quest_4_query", "quest_4_complete")
		else
			clonedConversation:addOption("@quest_armorsmith:yes_to_quest_4_query", "illegal_response")
		end
		clonedConversation:addOption("@quest_armorsmith:no_to_quest_4_query", "quest_4_incomplete")
	elseif (screenID == "quest_4_complete") then
		CreatureObject(pPlayer):setScreenPlayState(16, "ris_armor_quest")
		self:removeQuestItem(pPlayer, risArmorQuest.questItems.quest4.template)
	elseif (screenID == "quest_5_start") then
		self:teachSchematic(pPlayer, risArmorQuest.questSchematics.quest5)
		CreatureObject(pPlayer):setScreenPlayState(32, "ris_armor_quest")
	elseif (screenID == "quest_6_query") then
		clonedConversation:setDialogTextTO(risArmorQuest.questItems.quest6.displayName)
		if (self:hasQuestItem(pPlayer, risArmorQuest.questItems.quest6.template)) then
			clonedConversation:addOption("@quest_armorsmith:yes_to_quest_6_query", "quest_6_complete")
		else
			clonedConversation:addOption("@quest_armorsmith:yes_to_quest_6_query", "illegal_response")
		end
		clonedConversation:addOption("@quest_armorsmith:no_to_quest_6_query", "quest_6_incomplete")
	elseif (screenID == "quest_6_complete") then
		CreatureObject(pPlayer):setScreenPlayState(64, "ris_armor_quest")
		self:removeQuestItem(pPlayer, risArmorQuest.questItems.quest6.template)
		self:rewardSchematics(pPlayer)
	elseif (screenID == "completed_all_quests") then
		CreatureObject(pPlayer):setScreenPlayState(128, "ris_armor_quest")
	elseif (screenID == "quest_7_query") then
		-- check if they have the feather
		if util:hasInventory(pPlayer, "object/tangible/custom/loot_255_000000000000000000000.iff") then
			-- custom dialogue should indicate the player has a feather
			clonedConversation:setCustomDialogText("Oh it's you, welcome back.  Took you shorter than expected!  Let's have a goosey gander at this feather then.  Damn, you were supposed to be careful with it!  It has to be processed with care, not jammed up where the light from the Tatooine suns don't shine...kinda smells funny too...that you or the varactyl?")
			local pInventory = SceneObject(pPlayer):getSlottedObject("inventory")
			local pItem = getContainerObjectByTemplate(pInventory, "object/tangible/custom/loot_255_000000000000000000000.iff", false)
			CreatureObject(pPlayer):setScreenPlayState(256, "ris_armor_quest")
			self:rewardBHSchematics(pPlayer)
			if pItem ~= nil then
				print("take feather")
				SceneObject(pItem):destroyObjectFromWorld()
				SceneObject(pItem):destroyObjectFromDatabase()
			end
			util:giveQuestReward(pPlayer, "object/tangible/painting/ris_1_f00_00000000000000000000.iff")
		else
			-- custom text should 
			clonedConversation:setCustomDialogText("Oh it's you, welcome back.  What, no varactyl feather?  I really thought you had more to you that that.  As the Gungans say, mesa mor mor dissapoint! - and Gungans know nothing!  You sure you ain't a Gungan?  Go out there, and prove me wrong, Dur Dur Stinks! ")
		end		
	elseif (screenID == "buy_painting2") then
		if (CreatureObject(pPlayer):getCashCredits() >= 1000000) then
			clonedConversation:setCustomDialogText("Here you go, Enjoy it!")
			CreatureObject(pPlayer):subtractCashCredits(1000000)
			util:giveQuestReward(pPlayer, "object/tangible/painting/ris_1_f04_00000000000000000000.iff")
			clonedConversation:setCustomDialogText("Here you go, Ejoy it!")
		else
			clonedConversation:setCustomDialogText("I thought you had enough cash on hand but must be mistaken. Go hit the bank and come back.")
		end
	elseif (screenID == "quest_7_complete") then
		if (CreatureObject(pPlayer):getCashCredits() >= 1000000) then
			clonedConversation:addOption("Can I purchase the deluxe painting for one million credits?", "buy_painting2")
		end
	end
	util:cw("old RIS convo", 1)
	return pConvScreen
end

function risArmorQuestConvoHandler:teachSchematic(pPlayer, templateTable)
	local pGhost = CreatureObject(pPlayer):getPlayerObject()

	if (pGhost == nil) then
		return
	end

	PlayerObject(pGhost):addRewardedSchematic(templateTable.template, 2, -1, true)
	local messageString = LuaStringIdChatParameter("@loot_schematic:skill_granted")
	messageString:setTO(getStringId(templateTable.displayName))
	CreatureObject(pPlayer):sendSystemMessage(messageString:_getObject())
end

function risArmorQuestConvoHandler:rewardSchematics(pPlayer)
	for i = 1, #risArmorQuest.finalRewardSchematics, 1 do
		self:teachSchematic(pPlayer, risArmorQuest.finalRewardSchematics[i])
	end
end

function risArmorQuestConvoHandler:teachBHSchematic(pPlayer, templateTable)
	local pGhost = CreatureObject(pPlayer):getPlayerObject()

	if (pGhost == nil) then
		return
	end

	PlayerObject(pGhost):addRewardedSchematic(templateTable.template, 2, -1, true)
	local messageString = LuaStringIdChatParameter("@loot_schematic:skill_granted")
	messageString:setTO(getStringId(templateTable.displayName))
	CreatureObject(pPlayer):sendSystemMessage(messageString:_getObject())
end

function risArmorQuestConvoHandler:rewardBHSchematics(pPlayer)
	for i = 1, #risArmorQuest.BHarmorRewardSchematics, 1 do
		self:teachBHSchematic(pPlayer, risArmorQuest.BHarmorRewardSchematics[i])
	end
end

function risArmorQuestConvoHandler:hasQuestItem(pPlayer, template)
	local pInventory = SceneObject(pPlayer):getSlottedObject("inventory")
	return pInventory ~= nil and getContainerObjectByTemplate(pInventory, template, true) ~= nil
end

function risArmorQuestConvoHandler:removeQuestItem(pPlayer, template)
	local pInventory = SceneObject(pPlayer):getSlottedObject("inventory")

	if (pInventory == nil) then
		return
	end

	local pItem = getContainerObjectByTemplate(pInventory, template, true)

	if (pItem ~= nil) then
		SceneObject(pItem):destroyObjectFromWorld()
		SceneObject(pItem):destroyObjectFromDatabase()
	end
end
