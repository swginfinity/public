ForceShrineMenuComponent = {}

function ForceShrineMenuComponent:fillObjectMenuResponse(pSceneObject, pMenuResponse, pPlayer)
	local menuResponse = LuaObjectMenuResponse(pMenuResponse)

	-- Death Watch members and Friends of Greef get the Greef treatment
	if (CreatureObject(pPlayer):hasSkill("combat_death_watch_novice") or
		CreatureObject(pPlayer):hasSkill("combat_death_watch_friend")) then
		self:handleDeathWatchShrineViolation(pPlayer)
		return  -- No menu options for you
	end

	if (CreatureObject(pPlayer):hasSkill("mandalorian_novice")) then
		util:cout(pPlayer, "Your Mandalorian status prevents you from using this shrine. You must decide your path, both is not an option.", "orange")
	else
		if (CreatureObject(pPlayer):hasSkill("force_title_jedi_novice")) then
			menuResponse:addRadialMenuItem(120, 3, "@jedi_trials:meditate") -- Meditate
		end
		if util:hasQuestState(pPlayer, "mand:dropJedi", 1) then
			menuResponse:addRadialMenuItem(122, 3, "I would like Jedi back, I dropped it.")
		end
		if (CreatureObject(pPlayer):hasSkill("force_title_jedi_rank_02")) then
			menuResponse:addRadialMenuItem(121, 3, "@force_rank:recover_jedi_items") -- Recover Jedi Items
		end
	end
end

function ForceShrineMenuComponent:handleObjectMenuSelect(pObject, pPlayer, selectedID)
	if (pPlayer == nil or pObject == nil) then
		return 0
	end
	
	if (selectedID == 120 and CreatureObject(pPlayer):hasSkill("force_title_jedi_novice")) then
		if util:isAdmin(pPlayer) then
			util:cw("[mgmt] do meditate", 1)
			self:doMeditate(pObject, pPlayer)
		else
			if (CreatureObject(pPlayer):getPosture() ~= CROUCHED) then
				CreatureObject(pPlayer):sendSystemMessage("@jedi_trials:show_respect") -- Must respect
			else
				util:cw("do meditate", 1)
				self:doMeditate(pObject, pPlayer)
			end
		end
		
	elseif (selectedID == 122 and util:hasQuestState(pPlayer, "mand:dropJedi", 1)) then
		local pGhost = CreatureObject(pPlayer):getPlayerObject()
		if (pGhost == nil) then
			return 0
		end
		PlayerObject(pGhost):setJediState(2)
		util:giveProfession(pPlayer, "force_title_jedi_rank_01")
		util:giveProfession(pPlayer, "force_title_jedi_rank_02")
		util:removeQuestState(pPlayer, "mand:dropJedi", 1)
	elseif (selectedID == 121 and CreatureObject(pPlayer):hasSkill("force_title_jedi_rank_02")) then
		self:recoverRobe(pPlayer)
	end

	return 0
end

function ForceShrineMenuComponent:doMeditate(pObject, pPlayer)

    if not isPlayer(pPlayer) then
        return
    end

    local playerCreature = LuaCreatureObject(pPlayer)

	if not playerCreature:hasSkill("force_title_jedi_rank_02") and playerCreature:hasScreenPlayState(32, "VillageJediProgression") then
		local currentTrial = JediTrials:getCurrentTrial(pPlayer)
		util:cw("do meditate 1", 1)
		
		if not JediTrials:isOnPadawanTrials(pPlayer) then
			-- auto fix
			util:cw("do meditate 1a", 1)
			local showSui = true
			if playerCreature:hasScreenPlayState(32, "VillageJediProgression") then
				local completedTrials = tonumber(readScreenPlayData(pPlayer, "PadawanTrials", "completedTrials"))
				if completedTrials ~= nil and completedTrials == 1 then
					local pGhost = playerCreature:getPlayerObject()
					local state = PlayerObject(pGhost):getJediState()
					if state >= 2 then
						local learnedBranches = VillageJediManagerCommon.getLearnedForceSensitiveBranches(pPlayer)
						util:cw("learnedBranches=" .. learnedBranches, 1)
						if learnedBranches ~= nil and learnedBranches >= NUMBEROFTREESTOMASTER then
							util:cw("at this point fix them", 1)
							if not playerCreature:hasSkill("force_title_jedi_rank_01") then
								util:giveProfession(pPlayer, "force_title_jedi_rank_01")
							end
							if not playerCreature:hasSkill("force_title_jedi_rank_02") then
								util:giveProfession(pPlayer, "force_title_jedi_rank_02")
							end
							util:cout(pPlayer, "It appears that you dropped Jedi and are back to pick it up again. This is now fixed and you can once again start gaining Jedi experience. Remember to use /findMyTrainer if you forgot where you Jedi Skill Trainer is located.", "pink")
							util:cout(pPlayer, "If you do not know why you are seeing the above message, please contact support.", "orange")
							showSui = false
							Logger:log("Player fixed Jedi Status at shrine: " .. playerCreature:getFirstName(), LT_ERROR)
						else
							util:cw("does not have required qty of village trees", 1)
						end
					else
						-- not qualified
						util:cw("failed jedi state", 1)
					end
				else
					-- not qualified
					util:cw("failed to complete trials", 1)
				end
			else
				-- not qualified
				util:cw("failed VillageJediProgression", 1)
			end

			if showSui then
				util:cw("open sui", 1)
				PadawanTrials:startPadawanTrials(pObject, pPlayer)
			end
		elseif (currentTrial == 0) then
			PadawanTrials:startNextPadawanTrial(pObject, pPlayer)
			util:cw("do meditate 1b", 1)
		else
			PadawanTrials:showCurrentTrial(pObject, pPlayer)
			util:cw("do meditate 1c", 1)
		end
	elseif (JediTrials:isOnKnightTrials(pPlayer)) then
		util:cw("do meditate 2", 1)
		local pPlayerShrine = KnightTrials:getTrialShrine(pPlayer)

		if (pPlayerShrine ~= nil and pObject ~= pPlayerShrine) then
			local correctShrineZone = SceneObject(pPlayerShrine):getZoneName()
			if (correctShrineZone ~= SceneObject(pObject):getZoneName()) then
				local messageString = LuaStringIdChatParameter("@jedi_trials:knight_shrine_reminder")
				messageString:setTO(getStringId("@jedi_trials:" .. correctShrineZone))
				playerCreature:sendSystemMessage(messageString:_getObject())
			else
				playerCreature:sendSystemMessage("@jedi_trials:knight_shrine_wrong")
			end
			return
		end

		local currentTrial = JediTrials:getCurrentTrial(pPlayer)
		local trialsCompleted = JediTrials:getTrialsCompleted(pPlayer)

		if (currentTrial == 0 and trialsCompleted == 0) then
			KnightTrials:startNextKnightTrial(pPlayer)
		else
			KnightTrials:showCurrentTrial(pPlayer)
		end
	else
		util:cw("do meditate 3", 1)

		local pGhost = playerCreature:getPlayerObject()

        if not pGhost then
            return
        end

		util:cw("jedi state=" .. PlayerObject(pGhost):getJediState(), 1)
		local trialsCompleted = tonumber(readScreenPlayData(pPlayer, "KnightTrials", "trialsCompleted"))
		if trialsCompleted ~= nil and trialsCompleted >= 15 then
			if playerCreature:hasSkill("force_rank_light_novice") or playerCreature:hasSkill("force_rank_dark_novice") then
				util:cw("do nothing, they are knights", 1)
			else
				util:cw("they look like they want to be knight again", 1)
				if util:getCheck(pPlayer, 6) then
					util:cout(pPlayer, "You can only join an FRS faction once every 7 days.", "red")
				else
					if not playerCreature:hasSkill("force_rank_light_novice") and playerCreature:getFaction() == FACTIONREBEL then
                        local sui = SuiInputBox.new("ForceShrineMenuComponent", "rejoinFrsCallback")
                        sui.setTitle("Confirm Rejoining the Force Ranking System (FRS)")
                        local prompt = "Your faction is currently REBEL and you will be rejoining the FRS as a Light Jedi Knight.\n\n"
                        prompt = prompt .. "You can only rejoin the FRS once every 7 days, so be sure this is the faction that you want.\n\n"
                        prompt = prompt .. "Type YES to continue."
                        sui.setPrompt(prompt)
                        sui.sendTo(pPlayer)
					elseif not playerCreature:hasSkill("force_rank_dark_novice") and playerCreature:getFaction() == FACTIONIMPERIAL then
                        local sui = SuiInputBox.new("ForceShrineMenuComponent", "rejoinFrsCallback")
                        sui.setTitle("Confirm Rejoining the Force Ranking System (FRS)")
                        local prompt = "Your faction is currently IMPERIAL and you will be rejoining the FRS as a Dark Jedi Knight.\n\n"
                        prompt = prompt .. "You can only rejoin the FRS once every 7 days, so be sure this is the faction that you want.\n\n"
                        prompt = prompt .. "Type YES to continue."
                        sui.setPrompt(prompt)
                        sui.sendTo(pPlayer)
                    elseif playerCreature:getFaction() == FACTIONNEUTRAL then
                        util:cout(pPlayer, "You cannot re-join the FRS as a Civilian (neutral faction)", "red")
					end
				end
			end
		end
		playerCreature:sendSystemMessage("@jedi_trials:force_shrine_wisdom_" .. getRandomNumber(1, 15))
	end
	util:cw("do meditate 4", 1)
end

function ForceShrineMenuComponent:resetKnight(pPlayer)
	if isPlayer(pPlayer) then

        local playerCreature = LuaCreatureObject(pPlayer)

		local cooldown = 7 * 24 * 60 * 60
		if not util:isLive() then
			cooldown = 1 * 60
		end
		local pGhost = playerCreature:getPlayerObject()

        if not pGhost then
            return
        end
  
		util:cw("current jedi state=" .. PlayerObject(pGhost):getJediState(), 1)
		local changeVis = false
		if playerCreature:getFaction() == FACTIONREBEL then
			if playerCreature:villageKnightPrereqsMet("") then
				writeScreenPlayData(pPlayer, "JediTrials", "JediCouncil", 1)
				JediTrials:unlockJediKnight(pPlayer)
				util:cw("set cooldown", 1)
				util:setCheck(pPlayer, 6, cooldown)
				util:cw("new jedi state=" .. PlayerObject(pGhost):getJediState(), 1)
				changeVis = true
                util:cout(pPlayer, "You have rejoined the Force Ranking System as a Light Jedi Knight.  You can only re-join the FRS once every 7 days.", "green")
                Logger:log("Player " .. playerCreature:getFirstName() .. " has rejoined the FRS as a Light Jedi at a Force shrine.", LT_ERROR)
			else
				util:cout(pPlayer, "You do not meet the minimum requirements to return to Knight status.", "red")
			end
		elseif playerCreature:getFaction() == FACTIONIMPERIAL then
			if playerCreature:villageKnightPrereqsMet("") then
				writeScreenPlayData(pPlayer, "JediTrials", "JediCouncil", 2)
				JediTrials:unlockJediKnight(pPlayer)
				util:cw("set cooldown", 1)
				util:setCheck(pPlayer, 6, cooldown)
				util:cw("new jedi state=" .. PlayerObject(pGhost):getJediState(), 1)
				changeVis = true
                util:cout(pPlayer, "You have rejoined the Force Ranking System as a Dark Jedi Knight.  You can only re-join the FRS once every 7 days.", "green")
                Logger:log("Player " .. playerCreature:getFirstName() .. " has rejoined the FRS as a Dark Jedi at a Force shrine.", LT_ERROR)
			else
				util:cout(pPlayer, "You do not meet the minimum requirements to return to Knight status.", "red")
			end
		end
		if changeVis then 
			-- Remove Jedi Guardian and Jedi Master Boxes if they have them
			if playerCreature:hasSkill("force_title_jedi_master") then
				playerCreature:surrenderSkill("force_title_jedi_master")
			end
			if playerCreature:hasSkill("force_title_jedi_rank_04") then
				playerCreature:surrenderSkill("force_title_jedi_rank_04")
			end
			-- Reset FRS XP to Zero
			local frsXP = PlayerObject(pGhost):getExperience("force_rank_xp")
			if frsXP > 0 then
				playerCreature:awardExperience("force_rank_xp", frsXP * -1, false) -- Don't use modifiers
			end
			local currentVis = PlayerObject(pGhost):getVisibility()
            if currentVis == nil then
                currentVis = 0
            end
            local totalVis = 2000 + currentVis
			if totalVis > 8000 then
				totalVis = 8000
			end
        	PlayerObject(pGhost):setVisibility(totalVis)
			playerCreature:setFactionStatus(2)
		end
	end
end

function ForceShrineMenuComponent:rejoinFrsCallback(pPlayer, pSui, eventIndex, args)

	if (pPlayer == nil or not SceneObject(pPlayer):isPlayerCreature()) then
		return
	end

	local cancelPressed = (eventIndex == 1)

	if (cancelPressed) then
		util:cout(pPlayer, "Confirmation not received.", "orange")
		return
	end

    if isString(args) and string.upper(args) == "YES" then
	    ForceShrineMenuComponent:resetKnight(pPlayer)
    else
        util:cout(pPlayer, "Confirmation not received.", "orange")
    end
end
function ForceShrineMenuComponent:recoverRobe(pPlayer)
	local pInventory = SceneObject(pPlayer):getSlottedObject("inventory")

	if (pInventory == nil) then
		return
	end

	if (SceneObject(pInventory):isContainerFullRecursive()) then
		CreatureObject(pPlayer):sendSystemMessage("@jedi_spam:inventory_full_jedi_robe")
		return
	end

	local robeTemplate
	if (CreatureObject(pPlayer):hasSkill("force_title_jedi_rank_03")) then
		local councilType = JediTrials:getJediCouncil(pPlayer)

		if (councilType == JediTrials.COUNCIL_LIGHT) then
			robeTemplate = "object/tangible/wearables/robe/robe_jedi_light_s01.iff"
		else
			robeTemplate = "object/tangible/wearables/robe/robe_jedi_dark_s01.iff"
		end
	else
		robeTemplate = "object/tangible/wearables/robe/robe_jedi_padawan.iff"
	end

	giveItem(pInventory, robeTemplate, -1)
	CreatureObject(pPlayer):sendSystemMessage("@force_rank:items_recovered")
end

--[[
	Death Watch Shrine Violation Handler

	When a Death Watch member tries to use a Jedi shrine, Greef finds out immediately.
	This is triggered even if they're just messing around - Greef has eyes everywhere.

	Consequences:
	1. SUI message from Greef (threatening)
	2. Strip any Jedi progression states they have
	3. Broadcast bounty to all Death Watch players online
]]
function ForceShrineMenuComponent:handleDeathWatchShrineViolation(pPlayer)
	if pPlayer == nil then
		return
	end

	local playerName = CreatureObject(pPlayer):getFirstName() or "Unknown"
	local pGhost = CreatureObject(pPlayer):getPlayerObject()
	if pGhost == nil then
		return
	end

	-- Check if they have any actual Jedi progress to strip
	local hadJediProgress = false
	local jediState = PlayerObject(pGhost):getJediState()
	if jediState ~= nil and jediState > 1 then
		hadJediProgress = true
	end

	-- Also check for Jedi skills
	if CreatureObject(pPlayer):hasSkill("force_title_jedi_rank_01") or
	   CreatureObject(pPlayer):hasSkill("force_title_jedi_rank_02") or
	   CreatureObject(pPlayer):hasSkill("force_title_jedi_novice") then
		hadJediProgress = true
	end

	-- Build the SUI message
	local suiMessage = "A Jedi? Are you SERIOUS?!?!?!\n\n"
	suiMessage = suiMessage .. "You are SO dead.\n\n"
	suiMessage = suiMessage .. "Run. Seriously, you better run and hide.\n\n"

	if hadJediProgress then
		suiMessage = suiMessage .. "Oh, and your Jedi stuff? I took that away from you. "
		suiMessage = suiMessage .. "Keep your Force Sensitive skills, but Jedi is OFF THE TABLE.\n\n"
		suiMessage = suiMessage .. "You thought you could play both sides? Think again."
	else
		suiMessage = suiMessage .. "You don't even HAVE Jedi training and you're sniffing around shrines? "
		suiMessage = suiMessage .. "Now I KNOW you're up to something.\n\n"
		suiMessage = suiMessage .. "Consider this your only warning. There won't be another."
	end

	-- Show the SUI message from Greef
	local sui = SuiMessageBox.new("ForceShrineMenuComponent", "shrineViolationCallback")
	sui.setTitle("INCOMING MESSAGE FROM GREEF")
	sui.setPrompt(suiMessage)
	sui.setOkButtonText("I'm dead...")
	sui.hideCancelButton()
	sui.sendTo(pPlayer)

	-- Strip Jedi states if they had any
	if hadJediProgress then
		-- Reset Jedi state to 1 (like the mando recruiter does)
		PlayerObject(pGhost):setJediState(1)

		-- Surrender Jedi skills if they have them
		if CreatureObject(pPlayer):hasSkill("force_title_jedi_master") then
			CreatureObject(pPlayer):surrenderSkill("force_title_jedi_master")
		end
		if CreatureObject(pPlayer):hasSkill("force_title_jedi_rank_04") then
			CreatureObject(pPlayer):surrenderSkill("force_title_jedi_rank_04")
		end
		if CreatureObject(pPlayer):hasSkill("force_title_jedi_rank_03") then
			CreatureObject(pPlayer):surrenderSkill("force_title_jedi_rank_03")
		end
		if CreatureObject(pPlayer):hasSkill("force_title_jedi_rank_02") then
			CreatureObject(pPlayer):surrenderSkill("force_title_jedi_rank_02")
		end
		if CreatureObject(pPlayer):hasSkill("force_title_jedi_rank_01") then
			CreatureObject(pPlayer):surrenderSkill("force_title_jedi_rank_01")
		end
		if CreatureObject(pPlayer):hasSkill("force_title_jedi_novice") then
			CreatureObject(pPlayer):surrenderSkill("force_title_jedi_novice")
		end

		-- Clear Knight/Padawan trial progress
		deleteScreenPlayData(pPlayer, "KnightTrials", "trialsCompleted")
		deleteScreenPlayData(pPlayer, "PadawanTrials", "completedTrials")

		util:cout(pPlayer, "Your Jedi training has been stripped by Greef's contacts.", "red")
	end

	-- Broadcast to all Death Watch players - 1 million credit bounty
	if dw_messaging ~= nil then
		local bountyMessage = "BOUNTY ALERT: " .. playerName .. " was caught at a Jedi shrine. "
		bountyMessage = bountyMessage .. "1,000,000 credits for their head. They're running scared - hunt them down!"

		dw_messaging:broadcastToAllDW(bountyMessage, "Greef Karga")
	end

	-- Log this incident
	Logger:log("Death Watch member " .. playerName .. " attempted to use Jedi shrine. Jedi progress stripped: " .. tostring(hadJediProgress), LT_ERROR)
end

function ForceShrineMenuComponent:shrineViolationCallback(pPlayer, pSui, eventIndex, args)
	-- Nothing to do here - just acknowledging the message
	-- The damage is already done
end
