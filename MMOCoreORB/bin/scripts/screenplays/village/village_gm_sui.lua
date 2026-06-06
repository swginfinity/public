local Logger = require("utils.logger")
local QuestManager = require("managers.quest.quest_manager")

VillageGmSui =
    ScreenPlay:new {
    productionServer = false
}

function VillageGmSui:showMainPage(pPlayer)
    if not pPlayer or not SceneObject(pPlayer):isPlayerCreature() then
        return
    end

    local pGhost = CreatureObject(pPlayer):getPlayerObject()

    if not pGhost or not PlayerObject(pGhost):isPrivileged() then
        Logger:log(SceneObject(pPlayer):getDisplayedName() .. " ID: " .. SceneObject(pPlayer):getObjectID() .. " non Privileged player attempted to acces GMFSVillage -- VillageGmSui:showMainPage.", LT_ERROR)
        return
    end

    local curPhase = VillageJediManagerTownship:getCurrentPhase()
    local phaseID = VillageJediManagerTownship:getCurrentPhaseID()
    local nextPhaseChange = VillageJediManagerTownship.getNextPhaseChangeTime()
    local phaseTimeLeft = self:getPhaseDuration()

    local suiPrompt = " \\#pcontrast1 " .. "Current Phase:" .. " \\#pcontrast2 " .. curPhase .. " (id " .. phaseID .. ")\n" .. " \\#pcontrast1 " .. "Current Server Time:" .. " \\#pcontrast2 " .. os.date("%c") .. "\n"
    local suiPrompt = suiPrompt .. " \\#pcontrast1 " .. "Next Phase Change: " .. " \\#pcontrast2 " .. os.date("%c", nextPhaseChange) .. "\n \\#pcontrast1 " .. "Phase Time Left: " .. " \\#pcontrast2 " .. phaseTimeLeft

    local pMaster = VillageJediManagerTownship:getMasterObject()

    if (pMaster ~= nil) then
        local playerTable = SceneObject(pMaster):getPlayersInRange(192)
        suiPrompt = suiPrompt .. "\n \\#pcontrast1 " .. "Players in Village: " .. " \\#pcontrast2 " .. #playerTable
    end

    local sui = SuiListBox.new("VillageGmSui", "mainCallback")
    sui.setTitle("Village GM Panel")
    sui.setPrompt(suiPrompt)

    sui.add("Lookup player by target", "playerLookupByTarget")
    sui.add("Lookup player by name", "playerLookupByName")
    sui.add("Lookup player by oid", "playerLookupByOID")
    sui.add("List players in village", "listOnlineVillagePlayers")

    if (curPhase == 3) then
        sui.add("Manage CounterStrike Bases", "manageCounterStrikeBases")
    end

    sui.add("Output LUA os.time() (Debugging)", "getOSTime")

    if (not self.productionServer) then
        sui.add("Change to next phase", "changePhase")
    end

    sui.add("Show Light Council Ranks", "showLightRanks")
    sui.add("Show Dark Council Ranks", "showDarkRanks")

    sui.sendTo(pPlayer)
end

function VillageGmSui:mainCallback(pPlayer, pSui, eventIndex, args)
    local cancelPressed = (eventIndex == 1)

    if (cancelPressed or args == nil or tonumber(args) < 0) then
        return
    end

    local pPageData = LuaSuiBoxPage(pSui):getSuiPageData()

    if (pPageData == nil) then
        return
    end

    local suiPageData = LuaSuiPageData(pPageData)
    local menuOption = suiPageData:getStoredData(tostring(args))

    local targetID

    if (string.find(menuOption, "%d")) then
        targetID = string.match(menuOption, "%d+")
        menuOption = string.gsub(menuOption, targetID, "")

        local pTarget = getSceneObject(targetID)

        if (pTarget == nil) then
            Logger:log("Unable to find player for VillageGmSui function " .. menuOption .. " using oid " .. targetID, LT_ERROR)
            return
        end
    end

    if (self[menuOption] == nil) then
        Logger:log("Tried to execute invalid function " .. menuOption .. " in VillageGmSui", LT_ERROR)
        return
    end

    self[menuOption](pPlayer, targetID)
end

function VillageGmSui.getOSTime(pPlayer)
    if (pPlayer == nil) then
        return
    end

    CreatureObject(pPlayer):sendSystemMessage("Current OS time is: " .. os.time())
end

function VillageGmSui.changePhase(pPlayer)
    if (pPlayer == nil) then
        return
    end

    local sui = SuiMessageBox.new("VillageGmSui", "changePhaseCallback")

    sui.setTitle("Village Phase Change")
    sui.setPrompt("Are you sure you want to change the village to the next phase? Doing so will reset the progress of all players in the current phase.")
    sui.setOkButtonText("Yes")
    sui.setCancelButtonText("No")

    sui.sendTo(pPlayer)
end

function VillageGmSui:changePhaseCallback(pPlayer, pSui, eventIndex, args)
    local cancelPressed = (eventIndex == 1)

    if (cancelPressed) then
        VillageGmSui:showMainPage(pPlayer)
        return
    end

    local curPhase = VillageJediManagerTownship:getCurrentPhase()
    local nextPhase = curPhase + 1

    if nextPhase == 5 then
        nextPhase = 1
    end

    CreatureObject(pPlayer):sendSystemMessage("Changing the Village from phase " .. curPhase .. " to phase " .. nextPhase .. ".")
    VillageJediManagerTownship:switchToNextPhase(true)
end

function VillageGmSui.playerLookupByTarget(pPlayer)
    if (pPlayer == nil) then
        return
    end

    local targetID = CreatureObject(pPlayer):getTargetID()

    local pTarget = getSceneObject(targetID)

    if (pTarget == nil or not SceneObject(pTarget):isPlayerCreature()) then
        CreatureObject(pPlayer):sendSystemMessage("Invalid target, must be a valid player.")
        VillageGmSui:showMainPage(pPlayer)
        return
    end

    VillageGmSui.playerInfo(pPlayer, targetID)
end

function VillageGmSui.playerLookupByName(pPlayer)
    if (pPlayer == nil) then
        return
    end

    local sui = SuiInputBox.new("VillageGmSui", "playerLookupByNameCallback")

    sui.setTitle("Village Player Lookup (Name)")
    sui.setPrompt("Enter the name of the player you are looking for below.")

    sui.sendTo(pPlayer)
end

function VillageGmSui:playerLookupByNameCallback(pPlayer, pSui, eventIndex, args)
    local cancelPressed = (eventIndex == 1)

    if (cancelPressed) then
        VillageGmSui:showMainPage(pPlayer)
        return
    end

    if (args == nil or args == "") then
        CreatureObject(pPlayer):sendSystemMessage("Invalid string entered, please try again.")
        VillageGmSui.playerLookupByName(pPlayer)
        return
    end

    local pTarget = getPlayerByName(args)

    if (pTarget == nil) then
        CreatureObject(pPlayer):sendSystemMessage("Unable to find player using the string: " .. args .. ".")
        VillageGmSui.playerLookupByName(pPlayer)
        return
    end

    local targetID = SceneObject(pTarget):getObjectID()
    VillageGmSui.playerInfo(pPlayer, targetID)
end

function VillageGmSui.playerLookupByOID(pPlayer)
    if (pPlayer == nil) then
        return
    end

    local sui = SuiInputBox.new("VillageGmSui", "playerLookupByOIDCallback")

    sui.setTitle("Village Player Lookup (Object ID)")
    sui.setPrompt("Enter the object id of the player you are looking for below.")

    sui.sendTo(pPlayer)
end

function VillageGmSui:playerLookupByOIDCallback(pPlayer, pSui, eventIndex, args)
    local cancelPressed = (eventIndex == 1)

    if (cancelPressed) then
        VillageGmSui:showMainPage(pPlayer)
        return
    end

    if (args == nil or args == "") then
        CreatureObject(pPlayer):sendSystemMessage("Invalid oid entered, please try again.")
        VillageGmSui.playerLookupByOID(pPlayer)
        return
    end

    local pTarget = getSceneObject(args)

    if (pTarget == nil) then
        CreatureObject(pPlayer):sendSystemMessage("Unable to find player using the oid: " .. args .. ".")
        VillageGmSui.playerLookupByOID(pPlayer)
        return
    end

    VillageGmSui.playerInfo(pPlayer, tonumber(args))
end

function VillageGmSui.listOnlineVillagePlayers(pPlayer)
    if (pPlayer == nil) then
        return
    end

    local pMaster = VillageJediManagerTownship:getMasterObject()

    if (pMaster == nil) then
        Logger:log("Null village master object in VillageGmSui:listOnlineVillagePlayers", LT_ERROR)
        return
    end

    local playerTable = SceneObject(pMaster):getPlayersInRange(192)

    local sui = SuiListBox.new("VillageGmSui", "mainCallback")
    sui.setTitle("Village GM Panel")
    sui.setPrompt("These are the online players within 192 meters of the center of the Village:")

    for i = 1, #playerTable, 1 do
        if (playerTable[i] ~= nil) then
            sui.add(SceneObject(playerTable[i]):getDisplayedName(), "playerInfo" .. SceneObject(playerTable[i]):getObjectID())
        end
    end

    sui.sendTo(pPlayer)
end

function VillageGmSui.playerInfo(pPlayer, targetID)
    local pTarget = getCreatureObject(targetID)

    if (pTarget == nil) then
        return
    end

    local pGhost = CreatureObject(pTarget):getPlayerObject()

    if (pGhost == nil) then
        return
    end

    local jediState = PlayerObject(pGhost):getJediState()

    local promptBuf =
        " \\#pcontrast1 " ..
        "Player:" .. " \\#pcontrast2 " .. SceneObject(pTarget):getDisplayedName() .. " (" .. targetID .. ")\n"
    promptBuf = promptBuf .. " \\#pcontrast1 " .. "Jedi State:" .. " \\#pcontrast2 " .. jediState .. "\n"
    promptBuf = promptBuf .. " \\#pcontrast1 " .. "Progression:" .. " \\#pcontrast2 "

    if (CreatureObject(pTarget):hasSkill("force_title_jedi_rank_03")) then
        promptBuf = promptBuf .. "Knight Trials Completed\n"
    elseif (tonumber(readScreenPlayData(pTarget, "PadawanTrials", "completedTrials")) == 1) then
        --elseif (VillageJediManagerCommon.hasJediProgressionScreenPlayState(pTarget, VILLAGE_JEDI_PROGRESSION_COMPLETED_PADAWAN_TRIALS)) then
        if (JediTrials:isOnKnightTrials(pTarget)) then
            promptBuf = promptBuf .. "Knight Trials (" .. JediTrials:getTrialsCompleted(pTarget) .. " completed)\n"
            local trialNumber = JediTrials:getCurrentTrial(pTarget)
            if trialNumber == nil then
                trialNumber = "nil"
            end
            local trialName = JediTrials:getTrialStateName(pTarget, trialNumber)
            if trialName == nil then
                trialName = "JediTrial:nil.  trialNumber not found."
            end
            promptBuf = promptBuf .. "    Current Trial #:  " .. trialNumber .. "  Trial Name:  " .. string.gsub(trialName, "JediTrial:", "") .. "\n"

            local trialData = knightTrialQuests[trialNumber]

            if (trialData ~= nil and (trialData.trialType == TRIAL_HUNT or trialData.trialType == TRIAL_HUNT_FACTION)) then
                local huntTarget = readScreenPlayData(pTarget, "JediTrials", "huntTarget")
                if huntTarget == nil or huntTarget == "" then
                    huntTarget = "nil or blank"
                end
                local targetCountString = readScreenPlayData(pTarget, "JediTrials", "huntTargetCount")
                local targetCount = tonumber(targetCountString)
                if targetCount == nil then
                    targetCount = "nil or blank"
                end
                local targetGoalString = readScreenPlayData(pTarget, "JediTrials", "huntTargetGoal")
                local targetGoal = tonumber(targetGoalString)
                if targetGoal == nil then
                    targetGoal = "nil or blank"
                end
                promptBuf = promptBuf .. "    Hunt Target:  " .. huntTarget .. "  Targets Killed:  " .. targetCount .. "  Target Goal: " .. targetGoal .. "\n"
            end
        else
            promptBuf = promptBuf .. "Padawan Trials Completed\n"
        end
    elseif (VillageJediManagerCommon.hasJediProgressionScreenPlayState(pTarget, VILLAGE_JEDI_PROGRESSION_DEFEATED_MELLICHAE)) then
        if (JediTrials:isOnPadawanTrials(pTarget)) then
            promptBuf = promptBuf .. "Padawan Trials (" .. JediTrials:getTrialsCompleted(pTarget) .. " completed)\n"
            local trialNumber = JediTrials:getCurrentTrial(pTarget)
            if trialNumber == nil then
                trialNumber = "nil"
            end
            local trialName = JediTrials:getTrialStateName(pTarget, trialNumber)
            if trialName == nil then
                trialName = "JediTrial:nil.  trialNumber not found."
            end
            promptBuf = promptBuf .. "    Current Trial #:  " .. trialNumber .. "  Trial Name:  " .. string.gsub(trialName, "JediTrial:", "") .. "\n"

            local trialData = padawanTrialQuests[trialNumber]

            if (trialData ~= nil and trialData.trialType ~= TRIAL_HUNT) then
                local huntTarget = readScreenPlayData(pTarget, "JediTrials", "huntTarget")
                local targetCountString = readScreenPlayData(pTarget, "JediTrials", "huntTargetCount")
                local targetCount = tonumber(targetCountString)
                if targetCount == nil then
                    targetCount = "nil or blank"
                end
                local targetGoalString = readScreenPlayData(pTarget, "JediTrials", "huntTargetGoal")
                local targetGoal = tonumber(targetGoalString)
                if targetGoal == nil then
                    targetGoal = "nil or blank"
                end
                promptBuf = promptBuf .. "    Current Trial #:  " .. trialNumber .. "  Trial Name:  " .. string.gsub(trialName, "JediTrial:", "") .. "\n"
                if (huntTarget ~= nil and huntTarget ~= "") then
                    promptBuf = promptBuf .. "    Hunt Target:  " .. huntTarget .. "  Targets Killed:  " .. targetCount .. "  Target Goal: " .. targetGoal .. "\n"
                end
            end
        else
            promptBuf = promptBuf .. "Mellichae (Defeated)\n"
        end
    elseif (VillageJediManagerCommon.hasJediProgressionScreenPlayState(pTarget, VILLAGE_JEDI_PROGRESSION_ACCEPTED_MELLICHAE)) then
        promptBuf = promptBuf .. "Mellichae\n"
    elseif (FsOutro:isOnOutro(pTarget)) then
        local curStep = FsOutro:getCurrentStep(pTarget)

        if (curStep == FsOutro.OLDMANWAIT) then
            promptBuf = promptBuf .. "Outro (Waiting for Old Man)\n"
            if (FsOutro:hasDelayPassed(pTarget)) then
                promptBuf = promptBuf .. " \\#pcontrast1 " .. "Has Delay Passed: \\#pcontrast2 YES \n"
            else
                promptBuf = promptBuf .. " \\#pcontrast1 " .. "Has Delay Passed: \\#pcontrast2 NO \n"
            end

            local timeTilVisit = readScreenPlayData(pTarget, "VillageJediProgression", "FsOutroDelay") - os.time()

            if (not PlayerObject(pGhost):isOnline()) then
                promptBuf = promptBuf .. " \\#pcontrast1 " .. "Time until visit:" .. " \\#pcontrast2 Player Offline\n"
            elseif (timeTilVisit > 0) then
                promptBuf = promptBuf .. " \\#pcontrast1 " .. "Time until visit:" .. " \\#pcontrast2 " .. VillageGmSui:getTimeString(timeTilVisit * 1000) .. "\n"
            else
                promptBuf = promptBuf .. " \\#pcontrast1 " .. "Time until visit:" .. " \\#pcontrast2 Soon\n"
            end
        elseif (curStep == FsOutro.OLDMANMEET) then
            promptBuf = promptBuf .. "Outro (Old Man Visit)\n"
        end
    elseif (VillageJediManagerCommon.hasJediProgressionScreenPlayState(pTarget, VILLAGE_JEDI_PROGRESSION_HAS_VILLAGE_ACCESS)) then
        promptBuf = promptBuf .. "Village Phase Quests\n"
    elseif (FsIntro:isOnIntro(pTarget)) then
        local curStep = FsIntro:getCurrentStep(pTarget)

        if (curStep == FsIntro.OLDMANWAIT) then
            promptBuf = promptBuf .. "Intro (Waiting for Old Man)\n"
            if (FsIntro:hasDelayPassed(pTarget)) then
                promptBuf = promptBuf .. " \\#pcontrast1 " .. "Has Delay Passed: \\#pcontrast2 YES \n"
            else
                promptBuf = promptBuf .. " \\#pcontrast1 " .. "Has Delay Passed: \\#pcontrast2 NO \n"
            end
            local timeTilVisit = readScreenPlayData(pTarget, "VillageJediProgression", "FsIntroDelay") - os.time()

            if (not PlayerObject(pGhost):isOnline()) then
                promptBuf = promptBuf .. " \\#pcontrast1 " .. "Time until visit:" .. " \\#pcontrast2 Player Offline\n"
            elseif (timeTilVisit > 0) then
                promptBuf = promptBuf .. " \\#pcontrast1 " .. "Time until visit:" .. " \\#pcontrast2 " .. VillageGmSui:getTimeString(timeTilVisit * 1000) .. "\n"
            else
                promptBuf = promptBuf .. " \\#pcontrast1 " .. "Time until visit:" .. " \\#pcontrast2 Soon\n"
            end
            local totalVisits = tonumber(readScreenPlayData(pTarget, "VillageJediProgression", "FsIntroOldManVisits"))

            if (totalVisits == nil) then
                totalVisits = 0
            end
            promptBuf = promptBuf .. " \\#pcontrast1 " .. "Old Man Visits So Far:" .. " \\#pcontrast2 " .. totalVisits .. "\n"
        elseif (curStep == FsIntro.OLDMANMEET) then
            promptBuf = promptBuf .. "Intro (Old Man Visit)\n"
        elseif (curStep == FsIntro.SITHWAIT) then
            promptBuf = promptBuf .. "Intro (Waiting for Sith Attack)\n"
            if (FsIntro:hasDelayPassed(pTarget)) then
                promptBuf = promptBuf .. " \\#pcontrast1 " .. "Has Delay Passed: \\#pcontrast2 YES \n"
            else
                promptBuf = promptBuf .. " \\#pcontrast1 " .. "Has Delay Passed: \\#pcontrast2 NO \n"
            end
            local timeTilAttack = readScreenPlayData(pTarget, "VillageJediProgression", "FsIntroDelay") - os.time()

            if (not PlayerObject(pGhost):isOnline()) then
                promptBuf = promptBuf .. " \\#pcontrast1 " .. "Time Until Attack:" .. " \\#pcontrast2 Player Offline\n"
            elseif (timeTilAttack > 0) then
                promptBuf = promptBuf .. " \\#pcontrast1 " .. "Time Until Attack:" .. " \\#pcontrast2 " .. VillageGmSui:getTimeString(timeTilAttack * 1000) .. "\n"
            else
                promptBuf = promptBuf .. " \\#pcontrast1 " .. "Time Until Attack:" .. " \\#pcontrast2 Soon\n"
            end
        elseif (curStep == FsIntro.SITHATTACK) then
            promptBuf = promptBuf .. "Intro (Sith Attack)\n"
        elseif (curStep == FsIntro.USEDATAPADONE) then
            promptBuf = promptBuf .. "Intro (First Datapad Looted)\n"
        elseif (curStep == FsIntro.SITHTHEATER) then
            promptBuf = promptBuf .. "Intro (Sith Camp)\n"
        elseif (curStep == FsIntro.USEDATAPADTWO) then
            promptBuf = promptBuf .. "Intro (Second Datapad Looted)\n"
        elseif (curStep == FsIntro.VILLAGE) then
            promptBuf = promptBuf .. "Intro (Sent to Village)\n"
        end

        if (curStep == FsIntro.OLDMANWAIT or curStep == FsIntro.OLDMANMEET or curStep == FsIntro.SITHWAIT or curStep == FsIntro.SITHATTACK) then
            promptBuf = promptBuf .. " -- Encounter Checks --\n" .. "\\#pcontrast1 " .. "InPositionForEncounter:" .. " \\#pcontrast2 " .. tostring(Encounter:isPlayerInPositionForEncounter(pTarget)) .. "\n"

            if (not Encounter:isPlayerInPositionForEncounter(pTarget)) then
                promptBuf = promptBuf .. " \\#pcontrast1 " .. "Player Online:" .. " \\#pcontrast2 " .. tostring(Encounter:isPlayerOnline(pTarget)) .. "\n"
                if (PlayerObject(pGhost):isOnline()) then
                    promptBuf = promptBuf .. " \\#pcontrast1 " .. "Player In a Building:" .. " \\#pcontrast2 " .. tostring(Encounter:isPlayerInABuilding(pTarget)) .. "\n"
                    promptBuf = promptBuf .. " \\#pcontrast1 " .. "Player In NPC City:" .. " \\#pcontrast2 " .. tostring(Encounter:isPlayerInNpcCity(pTarget)) .. "\n"
                end
            end
            promptBuf = promptBuf .. " ----\n"
        end
    elseif (Glowing:isGlowing(pTarget)) then
        promptBuf = promptBuf .. "Glowing\n"
    else
        promptBuf = promptBuf .. "Not Glowing\n"
    end

    if (VillageJediManagerCommon.hasActiveQuestThisPhase(pTarget)) then
        promptBuf = promptBuf .. " \\#pcontrast1 " .. "Has Active Quest This Phase:" .. " \\#pcontrast2 YES\n"
    else
        promptBuf = promptBuf .. " \\#pcontrast1 " .. "Has Active Quest This Phase:" .. " \\#pcontrast2 NO\n"
    end

    if (VillageJediManagerCommon.hasCompletedQuestThisPhase(pTarget)) then
        promptBuf = promptBuf .. " \\#pcontrast1 " .. "Has Completed Quest This Phase:" .. " \\#pcontrast2 YES\n"
    else
        promptBuf = promptBuf .. " \\#pcontrast1 " .. "Has Completed Quest This Phase:" .. " \\#pcontrast2 NO\n"
    end

    if
        (VillageJediManagerCommon.hasJediProgressionScreenPlayState(pTarget, VILLAGE_JEDI_PROGRESSION_HAS_VILLAGE_ACCESS) or
            VillageJediManagerCommon.getUnlockedBranchCount(pTarget) > 0)
     then
        promptBuf = promptBuf .. " \\#pcontrast1 " .. "Unlocked Branches:" .. " \\#pcontrast2 " .. VillageJediManagerCommon.getUnlockedBranchCount(pTarget) .. "\n"
    end

    local sui = SuiListBox.new("VillageGmSui", "mainCallback")
    sui.setTitle("Village GM Panel")
    sui.setPrompt(promptBuf)

    sui.add("FS Branch Management", "branchManagement" .. targetID)

    if
        (jediState >= 4 or CreatureObject(pTarget):hasSkill("force_title_jedi_rank_03") or
            CreatureObject(pTarget):hasSkill("force_rank_dark_novice") or
            CreatureObject(pTarget):hasSkill("force_rank_light_novice"))
     then
        sui.add("Manage Player FRS", "frsManagement" .. targetID)
    else
        sui.add("Make Glowy", "makeGlowy" .. targetID)
        sui.add("Unlock Village", "unlockVillage" .. targetID)
        sui.add("Complete Village", "completeVillage" .. targetID)
        sui.add("Unlock Jedi Padawan", "unlockPadawan" .. targetID)
        sui.add("Unlock Light FRS", "unlockLightFrs" .. targetID)
        sui.add("Unlock Dark FRS", "unlockDarkFrs" .. targetID)
    end

    if (VillageJediManagerCommon.hasActiveQuestThisPhase(pTarget)) then
        sui.add("Manage Active Village Quest", "manageActiveVillageQuest" .. targetID)
        sui.add("Drop Current Village Quest", "dropActiveQuest" .. targetID)
    end

    if (VillageJediManagerCommon.hasCompletedQuestThisPhase(pTarget)) then
        sui.add("Reset Completed Quest This Phase", "resetCompletedQuest" .. targetID)
    end

    if (FsIntro:isOnIntro(pTarget)) then
        local curStep = FsIntro:getCurrentStep(pTarget)

        if (curStep == FsIntro.SITHWAIT or curStep == FsIntro.SITHATTACK) then
            sui.add("Force Start Sith Attack Intro Encounter", "forceIntroSithAttackEvent" .. targetID)
        elseif (curStep == FsIntro.OLDMANWAIT or curStep == FsIntro.OLDMANMEET) then
            sui.add("Force Start Old Man Intro Encounter", "forceIntroOldManEvent" .. targetID)
        end
        sui.add("Reset FS Intro (back to Old Man visit)", "resetFsIntro" .. targetID)
    elseif (FsOutro:isOnOutro(pTarget)) then
        local curStep = FsOutro:getCurrentStep(pTarget)

        if (curStep == FsOutro.OLDMANWAIT or curStep == FsIntro.OLDMANMEET) then
            sui.add("Force Start Old Man Outro Encounter", "forceOutroOldManEvent" .. targetID)
        end
        sui.add("Reset FS Outro (back to Old Man visit)", "resetFsOutro" .. targetID)
    elseif (JediTrials:isOnKnightTrials(pTarget)) then
        sui.add("Reset Knight Trials", "resetKnightTrials" .. targetID)
    end

    -- Infinity (#1452): Reset for players stuck with completedTrials flag (not actively on trials)
    if CreatureObject(pTarget):hasSkill("force_title_jedi_rank_03") and
       not CreatureObject(pTarget):hasSkill("force_rank_light_novice") and
       not CreatureObject(pTarget):hasSkill("force_rank_dark_novice") and
       not JediTrials:isOnKnightTrials(pTarget) and
       tonumber(readScreenPlayData(pTarget, "KnightTrials", "completedTrials") or 0) == 1 then
        sui.add("Reset Knight Trials (Completed)", "resetKnightTrials" .. targetID)
    end

    sui.add("Manage Visibility", "manageVisibility" .. targetID)

    if (CreatureObject(pTarget):hasSkill("combat_bountyhunter_investigation_03") or CreatureObject(pTarget):hasSkill("mandalorian_master")) then
        sui.add("Cancel Player Bounty Mission", "cancelPlayerBountyMission" .. targetID)
    end

    sui.sendTo(pPlayer)
end

function VillageGmSui.manageActiveVillageQuest(pPlayer, targetID)
    local pTarget = getSceneObject(targetID)

    if (pTarget == nil) then
        return
    end

    local pGhost = CreatureObject(pTarget):getPlayerObject()

    if (pGhost == nil) then
        return
    end

    if (not VillageJediManagerCommon.hasActiveQuestThisPhase(pTarget) or VillageJediManagerCommon.hasCompletedQuestThisPhase(pTarget)) then
        CreatureObject(pPlayer):sendSystemMessage("Player does not have an active quest this phase.")
        return
    end

    local sui = SuiListBox.new("VillageGmSui", "manageVillageQuestCallback")
    sui.setTitle("Village Quest Management")

    local curQuest = VillageJediManagerCommon.getActiveQuestIdThisPhase(pTarget)
    local questGiver = ""
    local promptAdd = ""

    if (curQuest == VILLAGE_PHASE1_SARGUILLO) then
        questGiver = "Sarguillo (Phase 1)"
        sui.add("Reset Current Patrol", "resetSarguilloPatrol" .. targetID)
        sui.add("Complete Current Patrol", "completeSarguilloPatrol" .. targetID)
        sui.add("Complete Patrol Quest (full)", "forcePatrolComplete" .. targetID)
    elseif (curQuest == VILLAGE_PHASE1_QUHAREK) then
        questGiver = "Quharek (Phase 1)"
        sui.add("Complete Crafting Quest (full)", "forceQuharekComplete" .. targetID)
    elseif (curQuest == VILLAGE_PHASE1_SIVARRA) then
        questGiver = "Sivarra (Phase 1)"
        sui.add("Complete Medic Quest (full)", "forceSivarraComplete" .. targetID)
    elseif (curQuest == VILLAGE_PHASE1_WHIP) then
        questGiver = "Whip (Phase 1)"
        sui.add("Complete Current Escort", "completeWhipEscort" .. targetID)
        sui.add("Complete Whip Quest (full)", "forceWhipComplete" .. targetID)
    elseif (curQuest == VILLAGE_PHASE2_DAGEERIN) then
        questGiver = "Dageerin (Phase 2)"
        sui.add("Reset task limit", "resetDageerinTaskLimit" .. targetID)
        promptAdd = " \\#pcontrast1 " .. "Tasks towards limit:" .. " \\#pcontrast2 " .. readScreenPlayData(pTarget, "VillageJediProgression", "FsSadTasksSinceLastTimestamp") .. "\n"
    elseif (curQuest == VILLAGE_PHASE2_QUHAREK) then
        questGiver = "Quharek (Phase 2)"
        sui.add("No current available functions for this quest", "noAvailableFunctions" .. targetID)
    elseif (curQuest == VILLAGE_PHASE2_WHIP) then
        questGiver = "Whip (Phase 2)"
        sui.add("No current available functions for this quest", "noAvailableFunctions" .. targetID)
    elseif (curQuest == VILLAGE_PHASE2_SURVEYOR) then
        questGiver = "Surveyor (Phase 2)"
        sui.add("No current available functions for this quest", "noAvailableFunctions" .. targetID)
    elseif (curQuest == VILLAGE_PHASE3_DAGEERIN) then
        questGiver = "Dageerin (Phase 3)"
        sui.add("Reset task limit", "resetDageerinTaskLimit" .. targetID)
        promptAdd = " \\#pcontrast1 " .. "Tasks towards limit:" .. " \\#pcontrast2 " .. readScreenPlayData(pTarget, "VillageJediProgression", "FsSad2TasksSinceLastTimestamp") .. "\n"
    elseif (curQuest == VILLAGE_PHASE3_QUHAREK) then
        questGiver = "Quharek (Phase 3)"
        sui.add("No current available functions for this quest", "noAvailableFunctions" .. targetID)
    elseif (curQuest == VILLAGE_PHASE3_SARGUILLO) then
        questGiver = "Sarguillo (Phase 3)"
        sui.add("No current available functions for this quest", "noAvailableFunctions" .. targetID)
    elseif (curQuest == VILLAGE_PHASE3_SURVEYOR) then
        questGiver = "Surveyor (Phase 3)"
        sui.add("No current available functions for this quest", "noAvailableFunctions" .. targetID)
    elseif (curQuest == VILLAGE_PHASE4_ENGINEER) then
        questGiver = "Chief Engineer (Phase 4)"
        sui.add("No current available functions for this quest", "noAvailableFunctions" .. targetID)
    elseif (curQuest == VILLAGE_PHASE4_SARGUILLO_CP) then
        questGiver = "Sarguillo - Combat Prowess (Phase 4)"
        sui.add("No current available functions for this quest", "noAvailableFunctions" .. targetID)
    elseif (curQuest == VILLAGE_PHASE4_SARGUILLO_ER) then
        questGiver = "Sarguillo - Enhanced Reflexes (Phase 4)"
        sui.add("No current available functions for this quest", "noAvailableFunctions" .. targetID)
    elseif (curQuest == VILLAGE_PHASE4_SIVARRA) then
        questGiver = "Sivarra (Phase 4)"
        sui.add("No current available functions for this quest", "noAvailableFunctions" .. targetID)
    end

    local promptBuf = " \\#pcontrast1 " .. "Player:" .. " \\#pcontrast2 " .. SceneObject(pTarget):getDisplayedName() .. " (" .. targetID .. ")\n"
    promptBuf = promptBuf .. " \\#pcontrast1 " .. "Current Quest:" .. " \\#pcontrast2 " .. questGiver .. "\n"
    promptBuf = promptBuf .. promptAdd

    sui.setPrompt(promptBuf)

    sui.sendTo(pPlayer)
end

function VillageGmSui:manageVillageQuestCallback(pPlayer, pSui, eventIndex, args)
    local cancelPressed = (eventIndex == 1)

    if (cancelPressed) then
        return
    end

    local pPageData = LuaSuiBoxPage(pSui):getSuiPageData()

    if (pPageData == nil) then
        return
    end

    local suiPageData = LuaSuiPageData(pPageData)
    local menuOption = suiPageData:getStoredData(tostring(args))

    local targetID, pTarget
    local curPhase = VillageJediManagerTownship:getCurrentPhase()

    if (string.find(menuOption, "%d")) then
        targetID = string.match(menuOption, "%d+")
        menuOption = string.gsub(menuOption, targetID, "")

        if (menuOption == "noAvailableFunctions") then
            return
        end

        pTarget = getSceneObject(targetID)

        if (pTarget == nil) then
            Logger:log("Unable to find player for VillageGmSui function " .. menuOption .. " using oid " .. targetID, LT_ERROR)
            return
        end
    end

    if (curPhase == 1) then
        if (menuOption == "resetSarguilloPatrol") then
            FsPatrol:resetFsPatrol(pTarget)
            CreatureObject(pPlayer):sendSystemMessage("Player's current Sarguillo Phase 1 patrol has been reset.")
        elseif (menuOption == "completeSarguilloPatrol") then
            CreatureObject(pPlayer):sendSystemMessage("Player's current Sarguillo Phase 1 patrol has been completed.")
            villageSarguilloPhase1ConvoHandler:completeCurrentPatrol(pTarget)
        elseif (menuOption == "completeWhipEscort") then
            CreatureObject(pPlayer):sendSystemMessage("Player's current Whip Phase 1 escort has been completed.")
            FsReflex1:completeVillagerEscort(pTarget)
        elseif (menuOption == "forcePatrolComplete") then
            FsPatrol:forceComplete(pTarget)
            CreatureObject(pPlayer):sendSystemMessage("Player's Sarguillo Phase 1 patrol quest has been force-completed.")
        elseif (menuOption == "forceWhipComplete") then
            FsReflex1:forceComplete(pTarget)
            CreatureObject(pPlayer):sendSystemMessage("Player's Whip Phase 1 escort quest has been force-completed.")
        elseif (menuOption == "forceSivarraComplete") then
            FsMedicPuzzle:forceComplete(pTarget)
            CreatureObject(pPlayer):sendSystemMessage("Player's Sivarra Phase 1 medic quest has been force-completed.")
        elseif (menuOption == "forceQuharekComplete") then
            SuiPhase1AccessTerminal:forceComplete(pTarget)
            CreatureObject(pPlayer):sendSystemMessage("Player's Quharek Phase 1 crafting quest has been force-completed.")
        end
    elseif (curPhase == 2) then
        if (menuOption == "resetDageerinTaskLimit") then
            FsSad:setTasksSinceLastTimestamp(pTarget, 0)
            CreatureObject(pPlayer):sendSystemMessage("Player's SAD task count towards limit has been reset back to 0.")
        end
    elseif (curPhase == 3) then
        if (menuOption == "resetDageerinTaskLimit") then
            FsSad2:setTasksSinceLastTimestamp(pTarget, 0)
            CreatureObject(pPlayer):sendSystemMessage("Player's SAD2 task count towards limit has been reset back to 0.")
        end
    end
end

function VillageGmSui.forceIntroSithAttackEvent(pPlayer, targetID)
    local pTarget = getSceneObject(targetID)

    if (pTarget == nil) then
        return
    end

    local curStep = FsIntro:getCurrentStep(pTarget)
    local playerName = CreatureObject(pTarget):getFirstName()
    if (not FsIntro:isOnIntro(pTarget) or (curStep ~= FsIntro.SITHWAIT and curStep ~= FsIntro.SITHATTACK)) then
        CreatureObject(pPlayer):sendSystemMessage("Unable to force the sith attack intro event for " .. playerName .. ", they are not on the correct step.")
        return
    end

    CreatureObject(pPlayer):sendSystemMessage("Now forcing the sith attack intro event to start for " .. playerName .. ".")
    FsIntro:startSithAttack(pTarget)
end

function VillageGmSui.resetFsIntro(pPlayer, targetID)
    local pTarget = getSceneObject(targetID)

    if (pTarget == nil) then
        return
    end

    local playerName = CreatureObject(pTarget):getFirstName()

    if (not FsIntro:isOnIntro(pTarget)) then
        CreatureObject(pPlayer):sendSystemMessage("Unable to reset FS intro for " .. playerName .. ", they are not on the intro.")
        return
    end

    -- Finish any active theater/encounter
    if (SithShadowIntroTheater:hasTaskStarted(pTarget)) then
        SithShadowIntroTheater:finish(pTarget)
    end

    -- Remove datapads if they have them
    if (FsIntro:hasFirstDatapad(pTarget)) then
        FsIntro:removeFirstDatapad(pTarget)
    end
    if (FsIntro:hasSecondDatapad(pTarget)) then
        FsIntro:removeSecondDatapad(pTarget)
    end

    -- Reset all intro quest states
    QuestManager.resetQuest(pTarget, QuestManager.quests.OLD_MAN_INITIAL)
    QuestManager.resetQuest(pTarget, QuestManager.quests.OLD_MAN_FORCE_CRYSTAL)
    QuestManager.resetQuest(pTarget, QuestManager.quests.TWO_MILITARY)
    QuestManager.resetQuest(pTarget, QuestManager.quests.LOOT_DATAPAD_1)
    QuestManager.resetQuest(pTarget, QuestManager.quests.GOT_DATAPAD)
    QuestManager.resetQuest(pTarget, QuestManager.quests.FS_THEATER_CAMP)
    QuestManager.resetQuest(pTarget, QuestManager.quests.LOOT_DATAPAD_2)
    QuestManager.resetQuest(pTarget, QuestManager.quests.GOT_DATAPAD_2)

    -- Clear screenplay data
    writeScreenPlayData(pTarget, "VillageJediProgression", "FsIntroStep", "")
    writeScreenPlayData(pTarget, "VillageJediProgression", "FsIntroDelay", "")
    writeScreenPlayData(pTarget, "VillageJediProgression", "FsIntroOldManVisits", "")

    -- Set back to step 1 (OLDMANWAIT) with immediate delay so old man spawns on next login
    FsIntro:setCurrentStep(pTarget, FsIntro.OLDMANWAIT)
    writeScreenPlayData(pTarget, "VillageJediProgression", "FsIntroDelay", os.time())

    CreatureObject(pPlayer):sendSystemMessage("Reset FS intro for " .. playerName .. ". Old man will spawn on next login.")
end

function VillageGmSui.resetFsOutro(pPlayer, targetID)
    local pTarget = getSceneObject(targetID)

    if (pTarget == nil) then
        return
    end

    local playerName = CreatureObject(pTarget):getFirstName()

    if (not FsOutro:isOnOutro(pTarget)) then
        CreatureObject(pPlayer):sendSystemMessage("Unable to reset FS outro for " .. playerName .. ", they are not on the outro.")
        return
    end

    -- Finish any active Mellichae theater
    if (MellichaeOutroTheater:hasTaskStarted(pTarget)) then
        MellichaeOutroTheater:finish(pTarget)
    end

    -- Reset outro quest states
    QuestManager.resetQuest(pTarget, QuestManager.quests.OLD_MAN_FINAL)
    QuestManager.resetQuest(pTarget, QuestManager.quests.FS_THEATER_FINAL)
    QuestManager.resetQuest(pTarget, QuestManager.quests.FS_THEATER_CAMP)

    -- Clear screenplay data
    writeScreenPlayData(pTarget, "VillageJediProgression", "FsOutroStep", "")
    writeScreenPlayData(pTarget, "VillageJediProgression", "FsOutroDelay", "")

    -- Set back to step 1 (OLDMANWAIT) with immediate delay
    FsOutro:setCurrentStep(pTarget, FsOutro.OLDMANWAIT)
    writeScreenPlayData(pTarget, "VillageJediProgression", "FsOutroDelay", os.time())

    CreatureObject(pPlayer):sendSystemMessage("Reset FS outro for " .. playerName .. ". Old man will spawn on next login.")
end

function VillageGmSui.resetKnightTrials(pPlayer, targetID)
    local pTarget = getSceneObject(targetID)

    if (pTarget == nil) then
        return
    end

    local playerName = CreatureObject(pTarget):getFirstName()

    -- Infinity (#1452): surrenderSkill breaks the jedi_trials.lua:onPlayerLoggedIn() re-fire
    -- cycle that re-sets completedTrials=1 on every login when rank_03 is present
    if CreatureObject(pTarget):hasSkill("force_title_jedi_rank_03") then
        CreatureObject(pTarget):surrenderSkill("force_title_jedi_rank_03")
    end

    KnightTrials:resetCompletedTrialsToStart(pTarget)

    CreatureObject(pPlayer):sendSystemMessage("Reset Knight Trials for " .. playerName .. ". Knight title surrendered, trial data cleared. Player can use frog to re-grant skills.")
end

function VillageGmSui.forceIntroOldManEvent(pPlayer, targetID)
    local pTarget = getSceneObject(targetID)

    if (pTarget == nil) then
        return
    end

    local curStep = FsIntro:getCurrentStep(pTarget)
    local playerName = CreatureObject(pTarget):getFirstName()
    if (not FsIntro:isOnIntro(pTarget) or (curStep ~= FsIntro.OLDMANWAIT and curStep ~= FsIntro.OLDMANMEET)) then
        CreatureObject(pPlayer):sendSystemMessage("Unable to force the old man intro event for " .. playerName .. ", they are not on the correct step.")
        return
    end

    CreatureObject(pPlayer):sendSystemMessage("Now forcing the old man event intro to start for " .. playerName .. ".")
    FsIntro:startOldMan(pTarget)
end

function VillageGmSui.forceOutroOldManEvent(pPlayer, targetID)
    local pTarget = getSceneObject(targetID)

    if (pTarget == nil) then
        return
    end

    local curStep = FsOutro:getCurrentStep(pTarget)

    if (not FsOutro:isOnOutro(pTarget) or (curStep ~= FsOutro.OLDMANWAIT and curStep ~= FsOutro.OLDMANMEET)) then
        CreatureObject(pPlayer):sendSystemMessage("Unable to force the old man outro event for " .. CreatureObject(pTarget):getFirstName() .. ", they are not on the correct step.")
        return
    end

    CreatureObject(pPlayer):sendSystemMessage("Now forcing the old man event outro to start for " .. CreatureObject(pTarget):getFirstName() .. ".")
    FsOutro:doOldManSpawn(pTarget)
end

function VillageGmSui.dropActiveQuest(pPlayer, targetID)
    local pTarget = getSceneObject(targetID)

    if (pTarget == nil) then
        return
    end

    local sui = SuiMessageBox.new("VillageGmSui", "dropActiveQuestCallback")

    sui.setTitle("Drop Current Village Quest")
    sui.setPrompt("Drop the player's current village quest? This tears the quest down and leaves them with NO quest chosen for this phase (they can pick a new one). Earned Force branches are kept. This does NOT restart the quest.")
    sui.setOkButtonText("Yes")
    sui.setCancelButtonText("No")
    sui.setTargetNetworkId(targetID)

    sui.sendTo(pPlayer)
end

-- Shared "drop the subject's active village quest" primitive — used by the GM SUI Drop button
-- (and, once all phases are built, the player-facing Paemos option). Resolves the active quest
-- FIRST (getActiveQuestIdThisPhase keys off the flag we are about to clear), tears it down via the
-- owning quest module's forceDrop, then clears the active-quest-this-phase flag so the subject has
-- NO quest chosen for the phase. Returns:
--   "full"     - quest fully torn down (Phase 1, or the Phase 4 Engineer special-case)
--   "flagonly" - flag cleared, but this quest's engine journal can't be fully torn down yet
--                (un-migrated phase 2-3 / phase-4 non-Engineer quest; its forceDrop isn't built)
--   "none"     - no active (incomplete) quest to drop
function VillageGmSui.dropActiveVillageQuest(pSubject)
    if (pSubject == nil) then
        return "none"
    end

    if (not VillageJediManagerCommon.hasActiveQuestThisPhase(pSubject) or VillageJediManagerCommon.hasCompletedQuestThisPhase(pSubject)) then
        return "none"
    end

    local activeQuest = VillageJediManagerCommon.getActiveQuestIdThisPhase(pSubject)

    -- Phase 1 - full per-quest drop (teardown + journal reset + flag clear inside each forceDrop).
    if (activeQuest == VILLAGE_PHASE1_SARGUILLO) then
        FsPatrol:forceDrop(pSubject)
        return "full"
    elseif (activeQuest == VILLAGE_PHASE1_WHIP) then
        FsReflex1:forceDrop(pSubject)
        return "full"
    elseif (activeQuest == VILLAGE_PHASE1_SIVARRA) then
        FsMedicPuzzle:forceDrop(pSubject)
        return "full"
    elseif (activeQuest == VILLAGE_PHASE1_QUHAREK) then
        SuiPhase1AccessTerminal:forceDrop(pSubject)
        return "full"
    end

    -- Phase 4 Engineer - the one later-phase quest with a known journal + item teardown.
    if (activeQuest == VILLAGE_PHASE4_ENGINEER) then
        for i = 0, 7 do
            local questName = "fs_crafting4_quest_0" .. i
            local questID = getPlayerQuestID(questName)
            QuestManager.resetQuest(pSubject, questID)
        end

        local pInventory = CreatureObject(pSubject):getSlottedObject("inventory")

        if (pInventory ~= nil) then
            local questItems = {
                "object/tangible/item/quest/force_sensitive/fs_crafting4_computer_core.iff",
                "object/tangible/loot/collectible/collectible_rewards/fs_tracking_device.iff",
                "object/tangible/loot/collectible/kits/fs_tracking_device_kit.iff"
            }

            for _, template in ipairs(questItems) do
                local pItem = getContainerObjectByTemplate(pInventory, template, true)

                if (pItem ~= nil) then
                    SceneObject(pItem):destroyObjectFromWorld()
                    SceneObject(pItem):destroyObjectFromDatabase()
                end
            end
        end

        deleteScreenPlayData(pSubject, "VillageJediProgression", "FsCrafting4TooLateBox")
        deleteData(SceneObject(pSubject):getObjectID() .. ":returningForCraftingKit")
        VillageJediManagerCommon.clearActiveQuestThisPhase(pSubject)
        return "full"
    end

    -- Fallback: no forceDrop built for this quest yet (phases 2-3 + phase-4 non-Engineer). Clear
    -- the village flag so the GM menu / quest-givers stay consistent; the engine journal entry for
    -- this quest can't be fully torn down until its own forceDrop is built (phases 2-4 follow-up).
    VillageJediManagerCommon.clearActiveQuestThisPhase(pSubject)
    return "flagonly"
end

function VillageGmSui:dropActiveQuestCallback(pPlayer, pSui, eventIndex, args)
    local pPageData = LuaSuiBoxPage(pSui):getSuiPageData()

    if (pPageData == nil) then
        return
    end

    local suiPageData = LuaSuiPageData(pPageData)

    local targetID = suiPageData:getTargetNetworkId()

    local pTarget = getSceneObject(targetID)

    if (pTarget == nil) then
        return
    end

    local cancelPressed = (eventIndex == 1)

    if (cancelPressed) then
        VillageGmSui.playerInfo(pPlayer, targetID)
        return
    end

    local result = VillageGmSui.dropActiveVillageQuest(pTarget)

    if (result == "full") then
        CreatureObject(pPlayer):sendSystemMessage("Player's current village quest has been dropped - no quest chosen for this phase.")
    elseif (result == "flagonly") then
        CreatureObject(pPlayer):sendSystemMessage("Cleared the player's village quest flag, but full journal teardown is not built for this quest's phase yet - verify their quest journal manually.")
    else
        CreatureObject(pPlayer):sendSystemMessage("Player has no active quest to drop this phase.")
    end

    VillageGmSui.playerInfo(pPlayer, targetID)
end

function VillageGmSui.resetCompletedQuest(pPlayer, targetID)
    local pTarget = getSceneObject(targetID)

    if (pTarget == nil) then
        return
    end

    local sui = SuiMessageBox.new("VillageGmSui", "resetCompletedQuestCallback")

    sui.setTitle("Reset Completed Quest")
    sui.setPrompt("Are you sure you want to reset the player's completed quest status? This will allow the player to complete a second quest this phase.")
    sui.setOkButtonText("Yes")
    sui.setCancelButtonText("No")
    sui.setTargetNetworkId(targetID)

    sui.sendTo(pPlayer)
end

function VillageGmSui:resetCompletedQuestCallback(pPlayer, pSui, eventIndex, args)
    local pPageData = LuaSuiBoxPage(pSui):getSuiPageData()

    if (pPageData == nil) then
        return
    end

    local suiPageData = LuaSuiPageData(pPageData)

    local targetID = suiPageData:getTargetNetworkId()

    local pTarget = getSceneObject(targetID)

    if (pTarget == nil) then
        return
    end

    local cancelPressed = (eventIndex == 1)

    if (cancelPressed) then
        VillageGmSui.playerInfo(pPlayer, targetID)
        return
    end

    local phaseID = VillageJediManagerTownship:getCurrentPhaseID()
    removeQuestStatus(targetID .. ":village:lastCompletedQuest")

    CreatureObject(pPlayer):sendSystemMessage("Player has had their completed quest status for this phase reset.")

    VillageGmSui.playerInfo(pPlayer, targetID)
end

function VillageGmSui.manageVisibility(pPlayer, targetID)
    local pTarget = getSceneObject(targetID)

    if (pTarget == nil) then
        return
    end

    local pGhost = CreatureObject(pTarget):getPlayerObject()

    if (pGhost == nil) then
        return
    end

    includeFile("../managers/jedi/visibility_manager.lua")

    local sui = SuiInputBox.new("VillageGmSui", "manageVisibilityCallback")
    sui.setTitle("Visibility Management")

    local promptBuf = " \\#pcontrast1 " .. "Player:" .. " \\#pcontrast2 " .. SceneObject(pTarget):getDisplayedName() .. " (" .. targetID .. ")\n"
    promptBuf = promptBuf .. " \\#pcontrast1 " .. "Current Visibility: " .. " \\#pcontrast2 " .. PlayerObject(pGhost):getVisibility() .. "\n"
    promptBuf = promptBuf .. " \\#pcontrast1 (Cap: " .. maxVisibility .. ") \n"

    sui.setPrompt(promptBuf)
    sui.setTargetNetworkId(targetID)
    sui.sendTo(pPlayer)
end

function VillageGmSui:manageVisibilityCallback(pPlayer, pSui, eventIndex, args)
    local cancelPressed = (eventIndex == 1)

    if (cancelPressed) then
        return
    end
    local pPageData = LuaSuiBoxPage(pSui):getSuiPageData()

    if (pPageData == nil) then
        --util:cw("page data null", 1)
        return
    end

    local suiPageData = LuaSuiPageData(pPageData)
    local targetID = suiPageData:getTargetNetworkId()

    local pTarget = getSceneObject(targetID)
    local pGhost = CreatureObject(pTarget):getPlayerObject()

    if (pGhost == nil) then
        return
    end
    local newVis = tonumber(args)
    if newVis ~= nil and newVis >= 0 and newVis <= 8000 then
        util:cout(pPlayer, "Visibility set to " .. tostring(newVis) .. " on " .. CreatureObject(pTarget):getFirstName(), "green")
        PlayerObject(pGhost):setVisibility(newVis)
    end

    VillageGmSui.playerInfo(pPlayer, targetID)
end

--[[

	FRS Management

]]
function VillageGmSui.frsManagement(pPlayer, targetID)
    local pTarget = getSceneObject(targetID)

    if (pTarget == nil) then
        return
    end

    local pGhost = CreatureObject(pTarget):getPlayerObject()

    if (pGhost == nil) then
        return
    end

    local councilType = PlayerObject(pGhost):getFrsCouncil()
    local luaCouncil = JediTrials:getJediCouncil(pTarget)
    local councilRank = PlayerObject(pGhost):getFrsRank()

    local sui = SuiListBox.new("VillageGmSui", "frsManageCallback")
    sui.setTitle("FRS Info")

    local promptBuf = " \\#pcontrast1 " .. "Player:" .. " \\#pcontrast2 " .. SceneObject(pTarget):getDisplayedName() .. " (" .. targetID .. ")\n"
    promptBuf = promptBuf .. " \\#pcontrast1 " .. "Council:" .. " \\#pcontrast2 "

    if (councilType == JediTrials.COUNCIL_LIGHT) then
        promptBuf = promptBuf .. "Light\n"
    elseif (councilType == JediTrials.COUNCIL_DARK) then
        promptBuf = promptBuf .. "Dark\n"
    else
        promptBuf = promptBuf .. "Invalid\n"
    end

    if (luaCouncil == nil or councilType == nil or luaCouncil ~= councilType) then
        promptBuf = promptBuf .. " \\#pcontrast1 " .. "WARNING:" .. " \\#pcontrast2 Trials council choice does not match council type stored on player. Use Change/Fix Council Type menu option below to fix.\n"
    end

    promptBuf = promptBuf .. " \\#pcontrast1 " .. "Rank:" .. " \\#pcontrast2 " .. councilRank .. "\n"
    promptBuf = promptBuf .. " \\#pcontrast1 " .. "FRS XP:" .. " \\#pcontrast2 " .. withCommas(PlayerObject(pGhost):getExperience("force_rank_xp")) .. "\n"

    sui.setPrompt(promptBuf)

    sui.add("Set FRS Rank", "setFrsRank" .. targetID)
    sui.add("Set FRS XP", "setFrsXp" .. targetID)
    sui.add("Change/Fix Council Type", "changeCouncilType" .. targetID)
    sui.add("Remove from FRS", "removeFromFrs" .. targetID)

    sui.sendTo(pPlayer)
end

function VillageGmSui:frsManageCallback(pPlayer, pSui, eventIndex, args)

    if not pPlayer or not SceneObject(pPlayer):isPlayerCreature() then
        return
    end

    local cancelPressed = (eventIndex == 1)

    if cancelPressed or args == nil or tonumber(args) < 0 then
        return
    end

    local pPageData = LuaSuiBoxPage(pSui):getSuiPageData()

    if not pPageData then
        return
    end

    local suiPageData = LuaSuiPageData(pPageData)
    local menuOption = suiPageData:getStoredData(tostring(args))
    local targetID, pTarget

    if (string.find(menuOption, "%d")) then
        targetID = string.match(menuOption, "%d+")
        menuOption = string.gsub(menuOption, targetID, "")

        pTarget = getSceneObject(targetID)
    end

    if not pTarget or not SceneObject(pTarget):isPlayerCreature() then
        Logger:log("Unable to find player for VillageGmSui function frsManageCallback - " .. menuOption .. " using oid " .. targetID, LT_ERROR)
        return
    end

    local targetSceno = LuaSceneObject(pTarget)
    local staffPlayerName = SceneObject(pPlayer):getDisplayedName()

    if (menuOption == "setFrsRank") then
        local sui = SuiInputBox.new("VillageGmSui", "suiSetFrsRankCallback")

        sui.setTargetNetworkId(targetSceno:getObjectID())

        local suiBody = "\r\\#FFFFFF Target Player: \r\\#pcontrast1 " .. CreatureObject(pTarget):getFirstName() .. "\r\\#FFFFFF \n"
        suiBody = suiBody .. " Target Player ID: \r\\#pcontrast1 " .. targetID .. "\r\\#FFFFFF \n\n"
        suiBody = suiBody .. " Enter the FRS Rank you wish to set on the target:"

        sui.setTitle("Set Player FRS Rank")
        sui.setPrompt(suiBody)

        sui.sendTo(pPlayer)
    elseif (menuOption == "setFrsXp") then
        local sui = SuiInputBox.new("VillageGmSui", "suiSetFrsXpCallback")

        sui.setTargetNetworkId(targetSceno:getObjectID())

        local suiBody = "Enter the amount of FRS XP you wish to grant:"
        sui.setTitle("Set Player FRS XP")
        sui.setPrompt(suiBody)

        sui.sendTo(pPlayer)
    elseif (menuOption == "changeCouncilType") then
        local sui = SuiListBox.new("VillageGmSui", "changeCouncilTypeCallback")
        sui.setTitle("Change Council Type")
        sui.setPrompt("Choose " .. targetSceno:getDisplayedName() .. "'s new council type below. Note: Changing a player's council will set them back to rank 0.")

        sui.setTargetNetworkId(targetSceno:getObjectID())

        sui.add("Light Side", "lightSide" .. targetID)
        sui.add("Dark Side", "darkSide" .. targetID)

        sui.sendTo(pPlayer)
    elseif (menuOption == "removeFromFrs") then
        -- Infinity:  Remove from FRS

        local pGhost = CreatureObject(pTarget):getPlayerObject()
        if not pGhost then
            return
        end

        PlayerObject(pGhost):removeFromFrs(CreatureObject(pTarget))

        -- Fixed in FRS Manager now, but double-checking here
        if PlayerObject(pGhost):getJediState() > 2 then
            PlayerObject(pGhost):setJediState(2)
        end

        local msg = SceneObject(pPlayer):getDisplayedName() .. " ID: " .. targetSceno:getObjectID() .. " used GMFSVillage:suiSetFrsRankCallback on "
        msg = msg .. targetSceno:getDisplayedName() .. " ID: " .. targetID .. " removing them from the FRS."
        Logger:log(msg, LT_ERROR)

        CreatureObject(pPlayer):sendSystemMessage(msg)

        VillageGmSui.playerInfo(pPlayer, targetID)
    end
end

function VillageGmSui:suiSetFrsRankCallback(pPlayer, pSui, eventIndex, args)
    if (pPlayer == nil) then
        return
    end

    local cancelPressed = (eventIndex == 1)

    if (cancelPressed or args == "") then
        return
    end

    local pPageData = LuaSuiBoxPage(pSui):getSuiPageData()

    if (pPageData == nil) then
        return
    end

    local suiPageData = LuaSuiPageData(pPageData)
    local targetID = suiPageData:getTargetNetworkId()

    local pTarget = getSceneObject(targetID)

    if (pTarget == nil or not SceneObject(pTarget):isPlayerCreature()) then
        Logger:log(SceneObject(pPlayer):getDisplayedName() .. " ID: " .. SceneObject(pPlayer):getObjectID() .. " attempted to use GMFSVillage:suiSetFrsRankCallback on an improper target.", LT_ERROR)

        CreatureObject(pPlayer):sendSystemMessage("Please select a proper target to set their FRS Rank.")
        return
    end

    local pGhost = CreatureObject(pTarget):getPlayerObject()
    if (pGhost == nil) then
        return
    end

    local rank = tonumber(args)

    if (rank == nil or rank < 0 or rank > 11) then
        Logger:log(SceneObject(pPlayer):getDisplayedName() .. " ID: " .. SceneObject(pPlayer):getObjectID() .. " attempted to use GMFSVillage:suiSetFrsRankCallback and input an improper rank number.", LT_ERROR)

        CreatureObject(pPlayer):sendSystemMessage("Please input a FRS Rank ranging from 0 to 11.")
        return
    end

    -- Infinity:  Set the rank and adjust FRS xp
    PlayerObject(pGhost):setFrsRank(rank, true)

    local msg = SceneObject(pPlayer):getDisplayedName() .. " ID: " .. SceneObject(pPlayer):getObjectID() .. " used GMFSVillage:suiSetFrsRankCallback on " .. SceneObject(pTarget):getDisplayedName() .. " ID: " .. targetID
    msg = msg .. " granting them Rank " .. rank
    Logger:log(msg, LT_ERROR)

    CreatureObject(pPlayer):sendSystemMessage(msg)

    VillageGmSui.frsManagement(pPlayer, targetID)
end

function VillageGmSui:suiSetFrsXpCallback(pPlayer, pSui, eventIndex, args)
    if (pPlayer == nil) then
        return
    end

    local cancelPressed = (eventIndex == 1)
    if (cancelPressed or args == "") then
        return
    end

    local pPageData = LuaSuiBoxPage(pSui):getSuiPageData()
    if (pPageData == nil) then
        return
    end

    local suiPageData = LuaSuiPageData(pPageData)
    local targetID = suiPageData:getTargetNetworkId()
    local pTarget = getSceneObject(targetID)

    if (pTarget == nil or not SceneObject(pTarget):isPlayerCreature()) then
        Logger:log(SceneObject(pPlayer):getDisplayedName() .. " ID: " .. SceneObject(pPlayer):getObjectID() .. " attempted to use GMFSVillage:suiSetFrsRankCallback on an improper target.", LT_ERROR)

        CreatureObject(pPlayer):sendSystemMessage("Please select a proper target to set their FRS Rank.")
        return
    end

    local pTargetGhost = CreatureObject(pTarget):getPlayerObject()
    if (pTargetGhost == nil) then
        return
    end

    local amount = tonumber(args)

    if (amount == nil) then
        return
    end

    local currentXp = PlayerObject(pTargetGhost):getExperience("force_rank_xp")

    local delta = amount - currentXp

    CreatureObject(pTarget):awardExperience("force_rank_xp", delta, true, false) -- Don't use modifiers

    if (delta < 0) then
        local messageString = LuaStringIdChatParameter("@base_player:prose_revoke_xp")
        messageString:setTO(getStringId("@exp_n:force_rank_xp"))
        messageString:setDI(delta * -1)
        CreatureObject(pTarget):sendSystemMessage(messageString:_getObject())
    end

    local msg = SceneObject(pPlayer):getDisplayedName() .. " ID: " .. SceneObject(pPlayer):getObjectID() .. " used GMFSVillage:suiSetFrsXpCallback on " .. SceneObject(pTarget):getDisplayedName() .. " ID: " .. targetID
    msg = msg .. " setting them to " .. amount .. " Force Rank Experience"
    Logger:log(msg, LT_ERROR)

    CreatureObject(pPlayer):sendSystemMessage(msg)

    VillageGmSui.frsManagement(pPlayer, targetID)
end

function VillageGmSui.unlockLightFrs(pPlayer, targetID)
    if not pPlayer or not targetID then
        return
    end

    local pTarget = getSceneObject(targetID)

    if not pTarget or not SceneObject(pTarget):isPlayerCreature() then
        return
    end

    local targetPlayer = LuaCreatureObject(pTarget)

    local pGhost = targetPlayer and targetPlayer:getPlayerObject()

    if not pGhost then
        return
    end

    local staffPlayer = LuaCreatureObject(pPlayer)
    local targetSceno = LuaSceneObject(pTarget)
    local targetPlayer = LuaCreatureObject(pTarget)
    local targetGhost = LuaPlayerObject(pGhost)

    local staffPlayerName = SceneObject(pPlayer):getDisplayedName()
    local staffPlayerOID = SceneObject(pPlayer):getObjectID()
    local targetPlayerName = targetSceno:getDisplayedName()

    if not targetPlayer:villageKnightPrereqsMet("") then
        staffPlayer:sendSystemMessage(targetPlayerName .. " ID: " .. targetID .. " is not eligible to complete the Knight Trials and needs more skillpoints spent in Jedi skills.")
        VillageGmSui.playerInfo(pPlayer, targetID)
        return
    end

    local jediState = targetGhost:getJediState()

    -- If they aleady have Jedi state 4 or 8, throw an error
    if jediState >= 4 then
        local msg = staffPlayerName .. " ID: " .. staffPlayerOID .. " attempted to use GMFSVillage:unlockLightFrs on Player: " .. targetPlayerName .. " ID: " .. targetID .. " with an invalid Jedi State of " .. jediState
        Logger:log(msg, LT_ERROR)

        staffPlayer:sendSystemMessage(targetPlayerName .. " ID: " .. targetID .. " must not have Jedi state 4 or 8.")
        return
    end

    -- Unlock and set to Light Side FRS
	JediTrials:completeKnightForTesting(pTarget, 1)

    targetSceno:switchZone(targetSceno:getZoneName(), targetSceno:getPositionX(), targetSceno:getPositionZ(), targetSceno:getPositionY(), targetSceno:getParentID())

    local msg = staffPlayerName .. " ID: " .. staffPlayerOID .. " used GMFSVillage:unlockLightFrs on Player: " .. targetPlayerName .. " ID: " .. targetID
    Logger:log(msg, LT_ERROR)

    CreatureObject(pPlayer):sendSystemMessage(targetPlayerName .. " ID: " .. targetID .. " is now a member of the Light Jedi Enclave.")

    VillageGmSui.playerInfo(pPlayer, targetID)
end

function VillageGmSui.unlockDarkFrs(pPlayer, targetID)
    if not pPlayer or not targetID then
        return
    end

    local pTarget = getSceneObject(targetID)

    if not pTarget or not SceneObject(pTarget):isPlayerCreature() then
        return
    end

    local targetPlayer = LuaCreatureObject(pTarget)

    local pGhost = targetPlayer and targetPlayer:getPlayerObject()

    if not pGhost then
        return
    end

    local staffPlayer = LuaCreatureObject(pPlayer)
    local targetSceno = LuaSceneObject(pTarget)
    local targetPlayer = LuaCreatureObject(pTarget)
    local targetGhost = LuaPlayerObject(pGhost)

    local staffPlayerName = SceneObject(pPlayer):getDisplayedName()
    local staffPlayerOID = SceneObject(pPlayer):getObjectID()
    local targetPlayerName = targetSceno:getDisplayedName()

    if not targetPlayer:villageKnightPrereqsMet("") then
        staffPlayer:sendSystemMessage(targetPlayerName .. " ID: " .. targetID .. " is not eligible to complete the Knight Trials and needs more skillpoints spent in Jedi skills.")
        VillageGmSui.playerInfo(pPlayer, targetID)
        return
    end

    local jediState = targetGhost:getJediState()

    -- If they aleady have Jedi state 4 or 8, throw an error
    if jediState >= 4 then
        local msg = staffPlayerName .. " ID: " .. staffPlayerOID .. " attempted to use GMFSVillage:unlockDarkFrs on Player: " .. targetPlayerName .. " ID: " .. targetID .. " with an invalid Jedi State of " .. jediState
        Logger:log(msg, LT_ERROR)

        staffPlayer:sendSystemMessage(targetPlayerName .. " ID: " .. targetID .. " must not have Jedi state 4 or 8.")
        return
    end

    -- Unlock and set to Dark Side FRS
	JediTrials:completeKnightForTesting(pTarget, 2)

    targetSceno:switchZone(targetSceno:getZoneName(), targetSceno:getPositionX(), targetSceno:getPositionZ(), targetSceno:getPositionY(), targetSceno:getParentID())

    local msg = staffPlayerName .. " ID: " .. staffPlayerOID .. " used GMFSVillage:unlockDarkFrs on Player: " .. targetPlayerName .. " ID: " .. targetID
    Logger:log(msg, LT_ERROR)

    CreatureObject(pPlayer):sendSystemMessage(targetPlayerName .. " ID: " .. targetID .. " is now a member of the Dark Jedi Enclave.")

    VillageGmSui.playerInfo(pPlayer, targetID)
end

function VillageGmSui.showLightRanks(pPlayer)
    if (pPlayer == nil or not SceneObject(pPlayer):isPlayerCreature()) then
        return
    end

    local pGhost = CreatureObject(pPlayer):getPlayerObject()

    if (pGhost == nil) then
        return
    end

    PlayerObject(pGhost):showCouncilRank(JediTrials.COUNCIL_LIGHT)

    local msg = SceneObject(pPlayer):getDisplayedName() .. " ID: " .. SceneObject(pPlayer):getObjectID() .. " used GMFSVillage:showLightRanks."
    Logger:log(msg, LT_ERROR)
end

function VillageGmSui.showDarkRanks(pPlayer)
    if (pPlayer == nil or not SceneObject(pPlayer):isPlayerCreature()) then
        return
    end

    local pGhost = CreatureObject(pPlayer):getPlayerObject()

    if (pGhost == nil) then
        return
    end

    PlayerObject(pGhost):showCouncilRank(JediTrials.COUNCIL_DARK)

    local msg = SceneObject(pPlayer):getDisplayedName() .. " ID: " .. SceneObject(pPlayer):getObjectID() .. " used GMFSVillage:showDarkRanks."
    Logger:log(msg, LT_ERROR)
end

--[[

	End of FRS Management

]]
function VillageGmSui.branchManagement(pPlayer, targetID)
    local pTarget = getSceneObject(targetID)

    if (pTarget == nil) then
        return
    end

    local sui = SuiListBox.new("VillageGmSui", "branchManagementCallback")
    sui.setTitle("Village Branch Management")
    sui.setPrompt("The statuses of " .. SceneObject(pTarget):getDisplayedName() .. "'s force sensitive branches are listed below. Branches can be locked and unlocked from this window."
    )

    for i = 1, #VillageJediManagerCommon.forceSensitiveBranches, 1 do
        local rowString = getStringId("@quest/force_sensitive/utils:" .. VillageJediManagerCommon.forceSensitiveBranches[i])

        if (VillageJediManagerCommon.hasUnlockedBranch(pTarget, VillageJediManagerCommon.forceSensitiveBranches[i])) then
            rowString = rowString .. " \\#pcontrast1 (UNLOCKED)"
        else
            rowString = rowString .. " \\#pcontrast2 (Locked)"
        end

        sui.add(rowString, VillageJediManagerCommon.forceSensitiveBranches[i] .. targetID)
    end

    sui.setOkButtonText("Lock/Unlock")
    sui.sendTo(pPlayer)
end

function VillageGmSui:branchManagementCallback(pPlayer, pSui, eventIndex, args)
    local cancelPressed = (eventIndex == 1)

    if (cancelPressed) then
        return
    end

    local pPageData = LuaSuiBoxPage(pSui):getSuiPageData()

    if (pPageData == nil) then
        return
    end

    local suiPageData = LuaSuiPageData(pPageData)
    local menuOption = suiPageData:getStoredData(tostring(args))

    local targetID = string.match(menuOption, "%d+")
    local branchName = string.gsub(menuOption, targetID, "")

    local pTarget = getSceneObject(targetID)

    if (pTarget == nil) then
        Logger:log("Unable to find player in VillageGmSui:branchManagementCallback using oid " .. targetID, LT_ERROR)
        return
    end

    if (VillageJediManagerCommon.hasUnlockedBranch(pTarget, branchName)) then
        CreatureObject(pPlayer):sendSystemMessage(SceneObject(pTarget):getDisplayedName() .. "'s " .. getStringId("@quest/force_sensitive/utils:" .. branchName) .. " has been LOCKED.")
        CreatureObject(pTarget):sendSystemMessage("Your unlock of the branch " .. getStringId("@quest/force_sensitive/utils:" .. branchName) .. " has been removed by a GM.")
        CreatureObject(pTarget):removeScreenPlayState(2, "VillageUnlockScreenPlay:" .. branchName)
    else
        CreatureObject(pPlayer):sendSystemMessage(SceneObject(pTarget):getDisplayedName() .. "'s " .. getStringId("@quest/force_sensitive/utils:" .. branchName) .. " has been UNLOCKED.")
        VillageJediManagerCommon.unlockBranch(pTarget, branchName)
    end

    VillageGmSui.branchManagement(pPlayer, targetID)
end

function VillageGmSui.manageCounterStrikeBases(pPlayer)
    if (pPlayer == nil) then
        return
    end

    local curPhase = VillageJediManagerTownship:getCurrentPhase()

    if (curPhase ~= 3) then
        return
    end

    local sui = SuiListBox.new("VillageGmSui", "manageCounterStrikeBasesCallback")
    sui.setTitle("Village CounterStrike Bases")
    sui.setPrompt("Below are the currently spawned CounterStrike bases. Select a base to get more detailed information.")

    local campList = FsCounterStrike:getPhaseCampList()
    local campTable = HelperFuncs:splitString(campList, ",")

    for i = 1, #campTable, 1 do
        local campNum = tonumber(campTable[i])
        local campLoc = FsCounterStrike.campSpawns[campNum]
        local campName = campLoc[1]

        local suiText = campName
        local theaterID = readData("VillageCounterStrikeCampID:" .. campName)
        local pTheater = getSceneObject(theaterID)

        if (pTheater == nil) then
            suiText = suiText .. " \\#pcontrast1 (CAMP OBJECT MISSING)"
        else
            if (not FsCsBaseControl:isShieldPoweredDown(pTheater)) then
                suiText = suiText .. " \\#pcontrast1 (SHIELD UP)"
            else
                suiText = suiText .. " \\#pcontrast2 (SHIELD DOWN)"
            end
        end

        sui.add(suiText, campNum)
    end

    sui.sendTo(pPlayer)
end

function VillageGmSui:manageCounterStrikeBasesCallback(pPlayer, pSui, eventIndex, args)
    local curPhase = VillageJediManagerTownship:getCurrentPhase()

    if (curPhase ~= 3) then
        return
    end

    local cancelPressed = (eventIndex == 1)

    if (cancelPressed or args == nil or tonumber(args) < 0) then
        return
    end

    local pPageData = LuaSuiBoxPage(pSui):getSuiPageData()

    if (pPageData == nil) then
        return
    end

    local suiPageData = LuaSuiPageData(pPageData)
    local campNum = tonumber(suiPageData:getStoredData(tostring(args)))

    local campLoc = FsCounterStrike.campSpawns[campNum]

    if (campLoc == nil) then
        Logger:log("Invalid camp info grabbed in VillageGmSui:manageCounterStrikeBasesCallback using camp number " .. campNum, LT_ERROR)
        return
    end

    local campName = campLoc[1]

    local sui = SuiListBox.new("VillageGmSui", "manageCounterStrikeBaseCallback")
    sui.setTitle("Village CounterStrike Base - " .. campName)

    local suiPrompt = " \\#pcontrast1 " .. "Base Name:" .. " \\#pcontrast2 " .. campName .. "\n"

    local theaterID = readData("VillageCounterStrikeCampID:" .. campName)
    local pTheater = getSceneObject(theaterID)

    if (pTheater == nil) then
        suiPrompt = suiPrompt .. " \\#pcontrast1 " .. "Theater Object:" .. " \\#pcontrast2 MISSING\n"
        sui.setPrompt(suiPrompt)
        sui.sendTo(pPlayer)
    else
        suiPrompt = suiPrompt .. " \\#pcontrast1 " .. "Theater Object:" .. " \\#pcontrast2 " .. theaterID .. "\n"
    end

    if (not FsCsBaseControl:isShieldPoweredDown(pTheater)) then
        suiPrompt = suiPrompt .. " \\#pcontrast1 " .. "Shield Status:" .. " \\#pcontrast2 UP\n"
    else
        suiPrompt = suiPrompt .. " \\#pcontrast1 " .. "Shield Status:" .. " \\#pcontrast2 DOWN\n"

        local powerDownTime = readData(theaterID .. ":shieldPowerDownTime")
        local powerDownDiff = os.time() - powerDownTime
        local diffString = self:getTimeString(powerDownDiff * 1000)
        suiPrompt = suiPrompt .. " \\#pcontrast1 " .. "Shield Taken Down At:" .. " \\#pcontrast2 " .. os.date("%c", powerDownTime) .. " (" .. diffString .. " ago)\n"
        local storedAttackerID = readData(theaterID .. ":attackerID")

        local pAttacker = getCreatureObject(storedAttackerID)

        if (pAttacker == nil) then
            suiPrompt = suiPrompt .. " \\#pcontrast1 " .. "Current Attacker:" .. " \\#pcontrast2 UNKNOWN\n"
        else
            suiPrompt = suiPrompt .. " \\#pcontrast1 " .. "Current Attacker:" .. " \\#pcontrast2 " .. CreatureObject(pAttacker):getFirstName() .. "\n"
        end
    end

    sui.add("Reset Base", "resetCounterStrikeBase" .. theaterID)
    sui.setPrompt(suiPrompt)
    sui.sendTo(pPlayer)
end

function VillageGmSui:manageCounterStrikeBaseCallback(pPlayer, pSui, eventIndex, args)
    local cancelPressed = (eventIndex == 1)

    if (cancelPressed) then
        return
    end

    local pPageData = LuaSuiBoxPage(pSui):getSuiPageData()

    if (pPageData == nil) then
        return
    end

    local suiPageData = LuaSuiPageData(pPageData)
    local menuOption = suiPageData:getStoredData(tostring(args))

    local targetID, pTheater
    local curPhase = VillageJediManagerTownship:getCurrentPhase()

    if (curPhase ~= 3) then
        return
    end

    if (string.find(menuOption, "%d")) then
        targetID = string.match(menuOption, "%d+")
        menuOption = string.gsub(menuOption, targetID, "")

        if (menuOption == nil) then
            return
        end

        pTheater = getSceneObject(targetID)

        if (pTheater == nil) then
            Logger:log("Unable to find theater for VillageGmSui function " .. menuOption .. " using oid " .. targetID, LT_ERROR)
            return
        end

        if (menuOption == "resetCounterStrikeBase") then
            FsCsBaseControl:resetCamp(pTheater, 0, true)
            CreatureObject(pPlayer):sendSystemMessage("Base reset.")
        end
    end
end

function VillageGmSui:getPhaseDuration()
    local eventID = getServerEventID("VillagePhaseChange")

    if (eventID == nil) then
        return
    end

    return self:getTimeString(getServerEventTimeLeft(eventID))
end

function VillageGmSui:getTimeString(miliTime)
    local timeLeft = miliTime / 1000
    local daysLeft = math.floor(timeLeft / (24 * 60 * 60))
    local hoursLeft = math.floor((timeLeft / 3600) % 24)
    local minutesLeft = math.floor((timeLeft / 60) % 60)
    local secondsLeft = math.floor(timeLeft % 60)

    return daysLeft .. "d " .. hoursLeft .. "h " .. minutesLeft .. "m " .. secondsLeft .. "s"
end

-- Infinity custom functions below

function VillageGmSui.cancelPlayerBountyMission(pPlayer, targetID)
    local pTarget = getSceneObject(targetID)

    if not pTarget or not SceneObject(pTarget):isPlayerCreature() then
        return
    end

    local targetCreature = LuaCreatureObject(pTarget)

    local pGhost = targetCreature:getPlayerObject()

    if not pGhost then
        return
    end

    if targetCreature:hasSkill("combat_bountyhunter_investigation_03") or targetCreature:hasSkill("mandalorian_master") then
        util:cout(pPlayer, "Cancelling any player bounty mission for " .. SceneObject(pTarget):getDisplayedName(), "green")
        targetCreature:failPlayerBountyMission()
    end
end

function VillageGmSui:changeCouncilTypeCallback(pPlayer, pSui, eventIndex, args)
    if (pPlayer == nil) then
        return
    end

    local cancelPressed = (eventIndex == 1)
    if (cancelPressed or args == "" or args == nil) then
        return
    end

    local pPageData = LuaSuiBoxPage(pSui):getSuiPageData()
    if (pPageData == nil) then
        return
    end

    local suiPageData = LuaSuiPageData(pPageData)
    local menuOption = suiPageData:getStoredData(tostring(args))

    local targetID = string.match(menuOption, "%d+")
    local newType = string.gsub(menuOption, targetID, "")

    local councilType = 0

    if (newType == "lightSide") then
        councilType = 1
    elseif (newType == "darkSide") then
        councilType = 2
    end

    if councilType ~= JediTrials.COUNCIL_LIGHT and councilType ~= JediTrials.COUNCIL_DARK then
        Logger:log("Invalid council type " .. newType .. " sent to VillageGmSui:changeCouncilTypeCallback", LT_ERROR)
        return
    end

    local pTarget = getSceneObject(targetID)
    if pTarget == nil then
        Logger:log("Unable to find player in VillageGmSui:changeCouncilTypeCallback using oid " .. targetID, LT_ERROR)
        return
    end

    local pGhost = CreatureObject(pTarget):getPlayerObject()
    if (pGhost == nil) then
        return
    end

    if (councilType == JediTrials.COUNCIL_LIGHT) then
        CreatureObject(pTarget):surrenderSkill("force_title_jedi_master")
        CreatureObject(pTarget):surrenderSkill("force_title_jedi_rank_04")
        CreatureObject(pTarget):surrenderSkill("force_title_jedi_rank_03") -- This should remove them from FRS and clear FRS XP

        local factionStanding = PlayerObject(pGhost):getFactionStanding("rebel")
        if factionStanding < 200 then
            PlayerObject(pGhost):setFactionStanding("rebel", 200)
        end
        CreatureObject(pTarget):setFaction(FACTIONREBEL)
        CreatureObject(pTarget):setFactionStatus(2)
        PlayerObject(pGhost):setJediState(4)
        PlayerObject(pGhost):setFrsCouncil(councilType)
        JediTrials:setJediCouncil(pTarget, councilType)
        awardSkill(pTarget, "force_title_jedi_rank_03")
        awardSkill(pTarget, "force_rank_light_novice")
        PlayerObject(pGhost):setFrsRank(0)
        CreatureObject(pPlayer):sendSystemMessage("Council type has been set to Light Side.")
    else
        CreatureObject(pTarget):surrenderSkill("force_title_jedi_master")
        CreatureObject(pTarget):surrenderSkill("force_title_jedi_rank_04")
        CreatureObject(pTarget):surrenderSkill("force_title_jedi_rank_03") -- This should remove them from FRS and clear FRS XP

        local factionStanding = PlayerObject(pGhost):getFactionStanding("imperial")
        if factionStanding < 200 then
            PlayerObject(pGhost):setFactionStanding("imperial", 200)
        end
        CreatureObject(pTarget):setFaction(FACTIONIMPERIAL)
        CreatureObject(pTarget):setFactionStatus(2)
        PlayerObject(pGhost):setJediState(8)
        PlayerObject(pGhost):setFrsCouncil(councilType)
        JediTrials:setJediCouncil(pTarget, councilType)
        awardSkill(pTarget, "force_title_jedi_rank_03")
        awardSkill(pTarget, "force_rank_dark_novice")
        PlayerObject(pGhost):setFrsRank(0)
        CreatureObject(pPlayer):sendSystemMessage("Council type has been set to Dark Side.")
    end

    VillageGmSui.frsManagement(pPlayer, targetID)
end

function VillageGmSui.makeGlowy(pPlayer, targetID)
    if not pPlayer or not targetID then
        return
    end

    local pTarget = getSceneObject(targetID)

    if not pTarget or not SceneObject(pTarget):isPlayerCreature() then
        return
    end

    local targetPlayer = LuaCreatureObject(pTarget)

    local pGhost = CreatureObject(pTarget):getPlayerObject()

    if not pGhost then
        return
    end

    local staffPlayer = LuaCreatureObject(pPlayer)
    local targetPlayer = LuaCreatureObject(pTarget)
    local targetGhost = LuaPlayerObject(pGhost)

    local staffPlayerName = SceneObject(pPlayer):getDisplayedName()
    local staffPlayerOID = SceneObject(pPlayer):getObjectID()
    local targetPlayerName = SceneObject(pTarget):getDisplayedName()

    -- Make glowy
    local glowyBadgeIds = {12, 14, 15, 16, 17, 19, 20, 21, 23, 30, 38, 39, 108, 45, 48, 56, 65, 69}

    for i = 1, #glowyBadgeIds do
        targetGhost:awardBadge(glowyBadgeIds[i])
    end

    Logger:log(staffPlayerName .. " ID: " .. staffPlayerOID .. " used GMFSVillage:makeGlowy on Player: " .. targetPlayerName .. " ID: " .. targetID, LT_ERROR)

    staffPlayer:sendSystemMessage(targetPlayerName .. " ID: " .. targetID .. " has been granted the appropriate badges and is now glowy.")

    VillageGmSui.playerInfo(pPlayer, targetID)

end

function VillageGmSui.unlockVillage(pPlayer, targetID)
    if not pPlayer or not targetID then
        return
    end

    local pTarget = getSceneObject(targetID)

    if not pTarget or not SceneObject(pTarget):isPlayerCreature() then
        return
    end

    local targetPlayer = LuaCreatureObject(pTarget)

    local pGhost = CreatureObject(pTarget):getPlayerObject()

    if not pGhost then
        return
    end

    local staffPlayer = LuaCreatureObject(pPlayer)
    local targetPlayer = LuaCreatureObject(pTarget)
    local targetGhost = LuaPlayerObject(pGhost)

    local staffPlayerName = SceneObject(pPlayer):getDisplayedName()
    local staffPlayerOID = SceneObject(pPlayer):getObjectID()
    local targetPlayerName = SceneObject(pTarget):getDisplayedName()

    -- Make glowy
    local glowyBadgeIds = {12, 14, 15, 16, 17, 19, 20, 21, 23, 30, 38, 39, 108, 45, 48, 56, 65, 69}

    for i = 1, #glowyBadgeIds do
        targetGhost:awardBadge(glowyBadgeIds[i])
    end

    -- Complete FS Intro
    FsIntro:completeVillageIntroFrog(pTarget)

    Logger:log(staffPlayerName .. " ID: " .. staffPlayerOID .. " used GMFSVillage:unlockVillage on Player: " .. targetPlayerName .. " ID: " .. targetID, LT_ERROR)

    staffPlayer:sendSystemMessage(targetPlayerName .. " ID: " .. targetID .. " has been granted access to the Village and all prerequisite skills and quest states.")

    VillageGmSui.playerInfo(pPlayer, targetID)

end

function VillageGmSui.completeVillage(pPlayer, targetID)
    if not pPlayer or not targetID then
        return
    end

    local pTarget = getSceneObject(targetID)

    if not pTarget or not SceneObject(pTarget):isPlayerCreature() then
        return
    end

    local targetPlayer = LuaCreatureObject(pTarget)

    local pGhost = CreatureObject(pTarget):getPlayerObject()

    if not pGhost then
        return
    end

    local staffPlayer = LuaCreatureObject(pPlayer)
    local targetPlayer = LuaCreatureObject(pTarget)
    local targetGhost = LuaPlayerObject(pGhost)

    local staffPlayerName = SceneObject(pPlayer):getDisplayedName()
    local staffPlayerOID = SceneObject(pPlayer):getObjectID()
    local targetPlayerName = SceneObject(pTarget):getDisplayedName()

    -- Make glowy
    local glowyBadgeIds = {12, 14, 15, 16, 17, 19, 20, 21, 23, 30, 38, 39, 108, 45, 48, 56, 65, 69}

    for i = 1, #glowyBadgeIds do
        targetGhost:awardBadge(glowyBadgeIds[i])
    end

    -- Complete FS Intro
    FsIntro:completeVillageIntroFrog(pTarget)

    -- Award FS Skills and unlock
    local branches = {      
        "force_sensitive_combat_prowess_melee_accuracy",
        "force_sensitive_combat_prowess_melee_speed",
        "force_sensitive_enhanced_reflexes_ranged_defense",
        "force_sensitive_enhanced_reflexes_melee_defense"
    }

    -- Unlock branches and award FS skills
    for i = 1, #branches do
        local branch = branches[i]
        targetPlayer:setScreenPlayState(2, "VillageUnlockScreenPlay:" .. branch)
        awardSkill(pTarget, branch .. "_04")
    end

    -- Complete FS Outro
    FsOutro:completeVillageOutroFrog(pTarget)

    Logger:log(staffPlayerName .. " ID: " .. staffPlayerOID .. " used GMFSVillage:completeVillage on Player: " .. targetPlayerName .. " ID: " .. targetID, LT_ERROR)

    staffPlayer:sendSystemMessage(targetPlayerName .. " ID: " .. targetID .. " has completed the Village and has all prerequisite skills and quest states.")

    VillageGmSui.playerInfo(pPlayer, targetID)

end

function VillageGmSui.unlockPadawan(pPlayer, targetID)
    if not pPlayer or not targetID then
        return
    end

    local pTarget = getSceneObject(targetID)

    if not pTarget or not SceneObject(pTarget):isPlayerCreature() then
        return
    end

    local targetPlayer = LuaCreatureObject(pTarget)

    local pGhost = CreatureObject(pTarget):getPlayerObject()

    if not pGhost then
        return
    end

    local staffPlayer = LuaCreatureObject(pPlayer)
    local targetPlayer = LuaCreatureObject(pTarget)
    local targetGhost = LuaPlayerObject(pGhost)

    local staffPlayerName = SceneObject(pPlayer):getDisplayedName()
    local staffPlayerOID = SceneObject(pPlayer):getObjectID()
    local targetPlayerName = SceneObject(pTarget):getDisplayedName()

    local jediState = targetGhost:getJediState()

    -- If they aleady have padawan, throw an error
    if jediState >= 2 then
        local msg = staffPlayerName .. " ID: " .. staffPlayerOID .. " attempted to use GMFSVillage:unlockPadawan on Player: " .. targetPlayerName .. " ID: " .. targetID .. " with an invalid Jedi State of " .. jediState
        Logger:log(msg, LT_ERROR)

        staffPlayer:sendSystemMessage(targetPlayerName .. " ID: " .. targetID .. " must not have Jedi state 2 or higher.")
        return
    end

    -- Surrender All skills
    targetPlayer:surrenderAllSkills()

    -- Make glowy
    local glowyBadgeIds = {12, 14, 15, 16, 17, 19, 20, 21, 23, 30, 38, 39, 108, 45, 48, 56, 65, 69}

    for i = 1, #glowyBadgeIds do
        targetGhost:awardBadge(glowyBadgeIds[i])
    end

    -- Complete FS Intro
    FsIntro:completeVillageIntroFrog(pTarget)

    -- Award FS Skills and unlock
    local branches = {      
        "force_sensitive_combat_prowess_melee_accuracy",
        "force_sensitive_combat_prowess_melee_speed",
        "force_sensitive_enhanced_reflexes_ranged_defense",
        "force_sensitive_enhanced_reflexes_melee_defense"
    }

    -- Unlock branches and award FS skills
    for i = 1, #branches do
        local branch = branches[i]
        targetPlayer:setScreenPlayState(2, "VillageUnlockScreenPlay:" .. branch)
        awardSkill(pTarget, branch .. "_04")
    end

    -- Complete FS Outro
    FsOutro:completeVillageOutroFrog(pTarget)

	-- Complete Padwan Trials
	JediTrials:completePadawanForTesting(pTarget)

    Logger:log(staffPlayerName .. " ID: " .. staffPlayerOID .. " used GMFSVillage:unlockPadawan on Player: " .. targetPlayerName .. " ID: " .. targetID, LT_ERROR)

    staffPlayer:sendSystemMessage(targetPlayerName .. " ID: " .. targetID .. " has been granted Jedi Padawan and all prerequisite skills and quest states.")

    VillageGmSui.playerInfo(pPlayer, targetID)

end
