local ObjectManager = require("managers.object.object_manager")

HeroOfTatooineScreenPlay = ScreenPlay:new {
    screenplayName = "HeroOfTatooineScreenPlay",

    courageSpawns = {
        {-861, 4557}, {-159, 5040}, {-1264, 5379}, {-695, 4506}, {16, 4068},
        {-533, 4314}, {-1023, 5214}, {-318, 4477}, {-443, 4970}, {-1183, 4647},
        {-575, 4985}, {-1618, 5272}, {-97, 5025}, {-471, 4715}, {-991, 5887},
        {-998, 5240}, {-500, 4600}, {-1131, 6185}
    },

    altruismSpawns = {
        {2123, 1291}, {2470, 1394}, {2511, 1357}, {2439, 1294}, {3196, 1272},
        {3303, 1107}, {2123, 1291}, {2494, 1714}, {2545, 1741}, {2778, 1507},
        {5100, 800}, {2168, 1293}, {2488, 1676}, {3640, -44}, {1839, 966},
        {2570, 1164}, {2478, 687}, {1003, 1240}, {2560, 1324}, {2437, 1421}
    },

    intellectSpawns = {
        { x1 = 59.5, z1 = 52, y1 = -5372.23, x2 = 57.56, z2 = 52, y2 = -5366.43, bhX = 59.4, bhZ = 52, bhY = -5368.9, bhAngle = 84 },
        { x1 = -1370.11, z1 = 12, y1 = -3639.83, x2 = -1363.3, z2 = 12, y2 = -3642.07, bhX = -1367, bhZ = 12, bhY = -3642.2, bhAngle = -168 },
        { x1 = 3481.75, z1 = 5, y1 = -4641.05, x2 = 3486.69, z2 = 5, y2 = -4645.42, bhX = 3483.8, bhZ = 5, bhY = -4644.1, bhAngle = -145 },
        { x1 = 1541.44, z1 = 7, y1 = 3126.77, x2 = 1551.83, z2 = 7, y2 = 3124.25, bhX = 1546.2, bhZ = 7, bhY = 3124.3, bhAngle = -165 },
        { x1 = -2890.04, z1 = 5, y1 = 2198.77, x2 = -2893.69, z2 = 5, y2 = 2204.23, bhX = -2890.9, bhZ = 5, bhY = 2201.9, bhAngle = 57 },
        { x1 = 3793.97, z1 = 11.62, y1 = 2389.19, x2 = 3794.33, z2 = 11.66, y2 = 2396.27, bhX = 3795.4, bhZ = 11.9, bhY = 2392.4, bhAngle = 91 },
        { x1 = -5166.3, z1 = 75, y1 = -6620.24, x2 = -5171.38, z2 = 75, y2 = -6624.75, bhX = -5169.4, bhZ = 75, bhY = -6621.8, bhAngle = -42 }
    },

    honorSpawns = {
        {4650, -3860}, {4091, -4310}, {4835, -4845}, {5946, -5233}, {6390, -5220},
        {5966, -5224}, {5513, -5362}, {4467, -5192}, {5267, -4349}, {6377, -4623},
        {5912, -5135}, {4704, -4399}, {4612, -3969}, {4614, -4978}, {6337, -5325},
        {4373, -3956}, {5930, -5933}, {6144, -4800}
    },

    honorFailPoints = {
        pirate1 = {{-6.53, 0.03, -3.21, 4005941}, {-6.20, 0.57, 10.92, 4005939}},
        pirate2 = {{-5.3, 0.03, -1.4, 4005941}, {-6.20, 0.57, 10.92, 4005939}}
    },

    honorSuccessPoints = {
        pirate1 = {{-5.4, 0.31, -3.2, 4005941}, {-6.20, 0.57, 10.92, 4005939}},
        pirate2 = {{-6.20, 0.57, 10.92, 4005939}},
        rancher = {{-6.6, 0.03, -1.8, 4005941}},
        trooper1 = {{-7.86, -3.97, -8.18, 4005942}, {-6.1, 0.03, -4.2, 4005941}, {-6.20, 0.57, 10.92, 4005939}},
        trooper2 = {{-5.53, -3.97, -8.51, 4005942}, {-6.20, 0.57, 10.92, 4005939}}
    },

    spawnTimers = {
        initial = {20, 30}, -- Time in minutes for first spawn after server start
        respawn = {60, 180}, -- Time in minutes before respawning after despawn
        life = {15, 60} -- Time in minutes to keep mob in world before despawn
    },
    
    markOfTheHeroRing = "object/tangible/wearables/ring/ring_mark_hero.iff",
    markOfAltruism = "object/tangible/loot/quest/hero_of_tatooine/mark_altruism.iff",
    markOfCourage = "object/tangible/loot/quest/hero_of_tatooine/mark_courage.iff",
    markOfHonor = "object/tangible/loot/quest/hero_of_tatooine/mark_honor.iff",
    markOfIntellect = "object/tangible/loot/quest/hero_of_tatooine/mark_intellect.iff"
}

registerScreenPlay("HeroOfTatooineScreenPlay", true)

function HeroOfTatooineScreenPlay:start()
    if (isZoneEnabled("tatooine")) then
        self:spawnMobiles()
        self:initEvents()
        self:initRanchHouse()

        local pRanchHouse = getSceneObject(4005938)

        if pRanchHouse then
            createObserver(ENTEREDBUILDING, self.screenplayName, "onEnteredRanchHouse", pRanchHouse)
        else
            Logger:logEvent("Hero of Tatooine -- unable to find ranch house.", LT_ERROR)
        end

        local pAltCave = getSceneObject(9995371)

        if pAltCave then
            createObserver(ENTEREDBUILDING, self.screenplayName, "onEnteredAltruismCave", pAltCave)
            createObserver(EXITEDBUILDING, self.screenplayName, "onExitedAltruismCave", pAltCave)
        else
            Logger:logEvent("Hero of Tatooine -- unable to find Altruism cave.", LT_ERROR)
        end
    end
end

--[[ Infinity Custom allowing players to reset ]]
function HeroOfTatooineScreenPlay:reset(pPlayer)
    if isPlayer(pPlayer) then
        local states = {
            "1 - Started squill quest",
            "2 - Completed squill quest",
            "4 - Completed Altruism",
            "8 - Completed Intellect",
            "16 - Completed Courage",
            "32 - Completed Honor",
            "64 - Completed Mark of Hero"
        }
        for i = 1, #states do
            if util:hasQuestState(pPlayer, "hero_of_tatooine", QS[i]) then
                util:removeQuestState(pPlayer, "hero_of_tatooine", QS[i])
            end
        end
        local others = {
            {"Bounty Hunter", "hero_of_tatooine_intellect"},
            {"Farmer", "hero_of_tatooine_altruism"},
            {"Rancher", "hero_of_tatooine_honor"}
        }
        for j = 1, #others do
            for i = 1, 2 do
                if util:hasQuestState(pPlayer, others[j][2], QS[i]) then
                    util:removeQuestState(pPlayer, others[j][2], QS[i])
                end
            end
        end
        local marks = {
            "1 - Altruism",
            "2 - Intellect",
            "4 - Courage",
            "8 - Honor",
            "16 - Ring given"
        }
        for i = 1, #marks do
            if util:hasQuestState(pPlayer, "hero_of_tatooine_missing_marks", QS[i]) then
                util:removeQuestState(pPlayer, "hero_of_tatooine_missing_marks", QS[i])
            end
        end
        deleteData(CreatureObject(pPlayer):getObjectID() .. ":hero_of_tat:failedIntellect")
    end
end

function HeroOfTatooineScreenPlay:spawnMobiles()
    local pHermit = spawnMobile("tatooine", "hero_of_tat_hermit", 0, 36.5, -69, -103.7, 0, 7125586)

    if pHermit then
        writeData("hero_of_tat:hermit_id", SceneObject(pHermit):getObjectID())
    else
        Logger:logEvent("Hero of Tatooine: spawnMobiles -- unable to find hermit object ", LT_ERROR)
    end
end

function HeroOfTatooineScreenPlay:getEventTimer(event)
    if util:isTC() then
        return 15 * 60 * 1000 -- 15 Minute respawn on non live servers
    end

    local timer = self.spawnTimers[event]

    if timer then
        return getRandomNumber(timer[1], timer[2]) * 60 * 1000
    else
        Logger:logEvent("Hero of Tatooine: getEventTimer -- nil timer for event: " .. event .. " so using default", LT_ERROR)
        return getRandomNumber(20, 30) * 60 * 1000
    end
end

function HeroOfTatooineScreenPlay:spawnAltruismObjects()
    self:despawnAltruismObjects()

    --local pCrate = spawnSceneObject("tatooine", "object/tangible/item/quest/hero_of_tatooine/explosives_crate.iff", 76.87,-46.24,-136.9, 5995575, .985, 0, .1714, 0)
    -- infinity moved, conflict with mando sawtooth queen
    local pCrate = spawnSceneObject("tatooine", "object/tangible/item/quest/hero_of_tatooine/explosives_crate.iff", 95.4, -45.7, -95.9, 5995569, 0.985, 0, 0.1714, 0)

    if pCrate then
        writeData("hero_of_tat:altruismCrate", SceneObject(pCrate):getObjectID())
        SceneObject(pCrate):setObjectMenuComponent("explosivesCrateMenuComponent")
    else
        Logger:logEvent("Could not spawn crate for Altruism subquest", LT_ERROR)
    end

    local pWall = spawnSceneObject("tatooine", "object/static/structure/general/cave_wall_damprock_style_01.iff", 162.5, -66.8, -97.7, 5995573, 0, 0, 0, 0)

    if pWall then
        writeData("hero_of_tat:altruismWall", SceneObject(pWall):getObjectID())
    else
        Logger:logEvent("Could not spawn wall for Altruism subquest", LT_ERROR)
    end

    local pCrevice = spawnSceneObject("tatooine", "object/tangible/container/quest/hero_of_tatooine/rock_crevice.iff", 156.767, -66.46, -93.84, 5995572, 0.95, 0, 0, 0)

    if pCrevice then
        crevice = LuaSceneObject(pCrevice)
        writeData("hero_of_tat:altruismCrevice", crevice:getObjectID())
        crevice:setContainerComponent("heroOfTatooineRockCrevice")
        crevice:setContainerInheritPermissionsFromParent(false)
        crevice:setContainerDefaultAllowPermission(MOVEIN)
        crevice:setContainerDefaultAllowPermission(OPEN)
    else
        Logger:logEvent("Could not spawn crevice for Altruism subquest", LT_ERROR)
    end

    local pWife = spawnMobile("tatooine", "hero_of_tat_farmers_wife", 0, 190.8, -66.6, -104.6, -70, 5995573)

    if isAiAgent(pWife) then
        CreatureObject(pWife):setPvpStatusBitmask(0)
        writeData("hero_of_tat:farmerWifeId", SceneObject(pWife):getObjectID())
    else
        Logger:logEvent("Could not spawn wife for Altruism subquest", LT_ERROR)
    end

    local pDaughter = spawnMobile("tatooine", "hero_of_tat_farmers_child", 0, 192.1, -66.8, -105.9, -70, 5995573)

    if isAiAgent(pDaughter) then
        CreatureObject(pDaughter):setPvpStatusBitmask(0)
        writeData("hero_of_tat:farmerChildId", SceneObject(pDaughter):getObjectID())
    else
        Logger:logEvent("Could not spawn daughter for Altruism subquest", LT_ERROR)
    end

    createEvent(30 * 60 * 1000, self.screenplayName, "validateAltruismCave", nil, "")
end

function HeroOfTatooineScreenPlay:validateAltruismCave()
    local count = self:getCavePlayerWithQuestCount()

    if count == 0 then
        self:despawnAltruismObjects()
    else
        createEvent(30 * 60 * 1000, self.screenplayName, "validateAltruismCave", nil, "")
    end
end

function HeroOfTatooineScreenPlay:initEvents()
    self:createCourageEvent("initial")
    self:createAltruismEvent("initial")
    self:createIntellectEvent("initial")
    self:createHonorEvent("initial")
end

function HeroOfTatooineScreenPlay:createCourageEvent(event)
    local timer = self:getEventTimer(event)

    if hasServerEvent("HeroOfTatCourage") then
        rescheduleServerEvent("HeroOfTatCourage", timer)

        Logger:logEvent("Hero of Tatooine: Rescheduling EXISTING Event for Courage to spawn in " .. timer .. " Event Type: " .. event, LT_INFO)
    else
        createServerEvent(timer, self.screenplayName, "doCourageChange", "HeroOfTatCourage")

        Logger:logEvent("Hero of Tatooine: Scheduling NEW Event for Courage spawn in " .. timer .. " Event Type: " .. event, LT_INFO)
    end
end

function HeroOfTatooineScreenPlay:createAltruismEvent(event)
    local timer = self:getEventTimer(event)

    if hasServerEvent("HeroOfTatAltruism") then
        rescheduleServerEvent("HeroOfTatAltruism", timer)

        Logger:logEvent(
            "Hero of Tatooine: Rescheduling EXISTING Event for Altruism spawn in " .. timer .. " Event Type: " .. event, LT_INFO)
    else
        createServerEvent(timer, self.screenplayName, "doAltruismChange", "HeroOfTatAltruism")

        Logger:logEvent("Hero of Tatooine: Scheduling NEW Event for Altruism spawn in " .. timer .. " Event Type: " .. event, LT_INFO)
    end
end

function HeroOfTatooineScreenPlay:createIntellectEvent(event)
    local timer = self:getEventTimer(event)

    if hasServerEvent("HeroOfTatIntellect") then
        rescheduleServerEvent("HeroOfTatIntellect", timer)

        Logger:logEvent("Hero of Tatooine: Rescheduling EXISTING Event for Intellect spawn in " .. timer .. " Event Type: " .. event, LT_INFO)
    else
        createServerEvent(timer, self.screenplayName, "doIntellectSpawn", "HeroOfTatIntellect")

        Logger:logEvent("Hero of Tatooine: Scheduling NEW Event for Intellect spawn in " .. timer .. " Event Type: " .. event, LT_INFO)
    end
end

function HeroOfTatooineScreenPlay:createHonorEvent(event)
    local timer = self:getEventTimer(event)

    if hasServerEvent("HeroOfTatHonor") then
        rescheduleServerEvent("HeroOfTatHonor", timer)

        Logger:logEvent("Hero of Tatooine: Rescheduling EXISTING Event for Honor spawn in " .. timer .. " Event Type: " .. event, LT_INFO)
    else
        createServerEvent(timer, self.screenplayName, "doHonorChange", "HeroOfTatHonor")

        Logger:logEvent("Hero of Tatooine: Scheduling NEW Event for Honor spawn in " .. timer .. " Event Type: " .. event, LT_INFO)
    end
end

function HeroOfTatooineScreenPlay:doCourageChange()
    if not isZoneEnabled("tatooine") then
        return 0
    end

    local hermitId = readData("hero_of_tat:hermit_id")
    local pHermit = getSceneObject(hermitId)

    if not pHermit then
        Logger:logEvent("Hero of Tatooine: doCourageChange -- unable to find hermit object ", LT_ERROR)
        return 0
    end

    local mobId = readData("hero_of_tat:courage_mob_id")
    local mobLoc = readData("hero_of_tat:courage_mob_loc")
    local pBoar = getSceneObject(mobId)

    -- Reschedule respawn if boar is in combat or dead
    if isAiAgent(pBoar) then
        if AiAgent(pBoar):isInCombat() then
            Logger:logEvent("Hero of Tatooine: doCourageChange - Boar is in Combat, rescheduling.", LT_INFO)

            self:createCourageEvent("life")
        else
            Logger:logEvent("Hero of Tatooine: doCourageChange - Boar is spawned, despawning and rescheduling respawn.", LT_INFO)

            AiAgent(pBoar):doDespawn()

            deleteData("hero_of_tat:courage_mob_id")

            self:createCourageEvent("respawn")
        end

        return 0
    end

    local newLoc = getRandomNumber(1, #self.courageSpawns)

    if newLoc == mobLoc then
        if newLoc == #self.courageSpawns then
            newLoc = newLoc - 1
        else
            newLoc = newLoc + 1
        end
    end

    writeData("hero_of_tat:courage_mob_loc", newLoc)

    local x = self.courageSpawns[newLoc][1]
    local y = self.courageSpawns[newLoc][2]

    -- 	{zoneName, x, y, minDist, maxDist, force}
    local spawnPoint = getSpawnPoint("tatooine", x, y, 10, 128, true)

    if not spawnPoint then
        createEvent(30 * 1000, "HeroOfTatooineScreenPlay", "doCourageChange", nil, "")
        return 0
    end

    local z = getWorldFloor(spawnPoint[1], spawnPoint[3], "tatooine")
    pBoar = spawnMobile("tatooine", "wild_bladeback_boar", 0, spawnPoint[1], z, spawnPoint[3], getRandomNumber(360) - 180, 0)

    if pBoar then
        AiAgent(pBoar):addCreatureFlag(AI_NOAIAGGRO)
        createObserver(OBJECTDESTRUCTION, self.screenplayName, "notifyDefeatedBoar", pBoar)
        writeData("hero_of_tat:courage_mob_id", SceneObject(pBoar):getObjectID())

        Logger:logEvent("Hero of Tatooine: doCourageChange complete. Boar Spawned at: " .. spawnPoint[1] .. ", " .. spawnPoint[3], LT_INFO)
    else
        Logger:logEvent("HeroOfTatooineScreenPlay:doCourageChange, unable to spawn boar.", LT_ERROR)
    end

    self:createCourageEvent("life")
end

function HeroOfTatooineScreenPlay:notifyDefeatedBoar(pBoar, pAttacker)
    if not isAiAgent(pBoar) then
        return 1
    end

    if not isPlayerOrPet(pAttacker) then
        self:clearInventory(pBoar)
        return 1
    end

    local player = toLuaCreatureObject(petOwner(pAttacker))
    if not player or not player:hasScreenPlayState(2, "hero_of_tatooine") or player:hasScreenPlayState(16, "hero_of_tatooine") then
        if player and player:hasScreenPlayState(16, "hero_of_tatooine") then
            player:sendSystemMessage("@quest/hero_of_tatooine/system_messages:courage_already_have_mark")
        end
        self:clearInventory(pBoar)
        return 1
    end

    local pInventory = CreatureObject(pBoar):getSlottedObject("inventory")

    if not pInventory then
        return 1
    end

    local pItem = getContainerObjectByTemplate(pInventory, self.markOfCourage, false)

    if pItem then
        SceneObject(pInventory):setContainerOwnerID(player:getObjectID())
        createObserver(ITEMLOOTED, self.screenplayName, "notifyMarkOfCourageLooted", pItem)
        player:sendSystemMessage("@quest/hero_of_tatooine/system_messages:courage_notice_object")
    end

    Logger:logEvent("Hero of Tatooine: notifyDefeatedBoar - Boar has been killled.", LT_INFO)
    return 1
end

function HeroOfTatooineScreenPlay:clearInventory(pCreature)
    if not isCreatureObject(pCreature) then
        return 0
    end

    local pInventory = CreatureObject(pCreature):getSlottedObject("inventory")

    if not pInventory then
        return 0
    end

    -- Infinity:  Converted to easier while loop rather than for loop
    while SceneObject(pInventory):getContainerObjectsSize() > 0 do
        local pItem = SceneObject(pInventory):getContainerObject(0)
        if pItem then
            SceneObject(pItem):destroyObjectFromWorld()
            SceneObject(pItem):destroyObjectFromDatabase()
        end
    end
end

function HeroOfTatooineScreenPlay:notifyMarkOfCourageLooted(pItem, pLooter)
    if not pItem or not isPlayer(pLooter) then
        return 1
    end

    local player = LuaCreatureObject(pLooter)

    if not player:hasScreenPlayState(2, "hero_of_tatooine") or player:hasScreenPlayState(16, "hero_of_tatooine") then
        return 1
    end

    local pGhost = player:getPlayerObject()

    if not pGhost then
        return 1
    end

    if not PlayerObject(pGhost):hasBadge(POI_RABIDBEAST) then
        PlayerObject(pGhost):awardBadge(POI_RABIDBEAST)
    end
    player:setScreenPlayState(16, "hero_of_tatooine")

    return 1
end

function HeroOfTatooineScreenPlay:doAltruismChange()
    if not isZoneEnabled("tatooine") then
        return 0
    end

    local hermitId = readData("hero_of_tat:hermit_id")
    local pHermit = getSceneObject(hermitId)

    if not pHermit then
        Logger:logEvent("Hero of Tatooine: doAltruismChange -- unable to find hermit object ", LT_ERROR)
        return 0
    end

    local mobId = readData("hero_of_tat:altruism_mob_id")
    local mobLoc = readData("hero_of_tat:altruism_mob_loc")
    local pFarmer = getSceneObject(mobId)

    if isAiAgent(pFarmer) then
        AiAgent(pFarmer):doDespawn()

        deleteData("hero_of_tat:altruism_mob_id")

        Logger:logEvent("Hero of Tatooine: doAltruismChange - Moisture Farmer already spawned, despawning and rescheduling.", LT_INFO)

        self:createAltruismEvent("respawn")
        return 0
    end

    local newLoc = getRandomNumber(1, #self.altruismSpawns)

    if newLoc == mobLoc then
        if (newLoc == #self.altruismSpawns) then
            newLoc = newLoc - 1
        else
            newLoc = newLoc + 1
        end
    end

    writeData("hero_of_tat:altruism_mob_loc", newLoc)

    local x = self.altruismSpawns[newLoc][1]
    local y = self.altruismSpawns[newLoc][2]

    -- 	{zoneName, x, y, minDist, maxDist, force}
    local spawnPoint = getSpawnPoint("tatooine", x, y, 10, 128)

    if not spawnPoint then
        createEvent(30 * 1000, "HeroOfTatooineScreenPlay", "doAltruismChange", nil, "")
        return
    end

    local z = getWorldFloor(spawnPoint[1], spawnPoint[3], "tatooine")
    pFarmer = spawnMobile("tatooine", "hero_of_tat_farmer", 0, spawnPoint[1], z, spawnPoint[3], getRandomNumber(360) - 180, 0)

    if pFarmer then
        writeData("hero_of_tat:altruism_mob_id", SceneObject(pFarmer):getObjectID())
        CreatureObject(pFarmer):setPvpStatusBitmask(0)
        AiAgent(pFarmer):addCreatureFlag(AI_STATIC)

        Logger:logEvent("Hero of Tatooine: doAltruismChange - Spawned Moisture Farmer at " .. spawnPoint[1] .. ", " .. spawnPoint[3] .. " Tatooine using base Spawn Point - X: " .. x .. " Y: " .. y, LT_INFO)
    else
        Logger:logEvent("HeroOfTatooineScreenPlay:doAltruismChange, unable to spawn farmer.", LT_ERROR)
    end

    self:createAltruismEvent("life")
end

function HeroOfTatooineScreenPlay:doIntellectSpawn()
    if not isZoneEnabled("tatooine") then
        return 0
    end

    local hermitId = readData("hero_of_tat:hermit_id")
    local pHermit = getSceneObject(hermitId)

    if not pHermit then
        Logger:logEvent("Hero of Tatooine: doIntellectSpawn -- unable to find hermit object ", LT_ERROR)
        return 0
    end

    local mobLoc = readData("hero_of_tat:intellect_mob_loc")

    self:destroyIntellectMobs()

    local newLoc = getRandomNumber(1, #self.intellectSpawns)

    if newLoc == mobLoc then
        if (newLoc == #self.intellectSpawns) then
            newLoc = newLoc - 1
        else
            newLoc = newLoc + 1
        end
    end

    writeData("hero_of_tat:intellect_mob_loc", newLoc)

    local pBountyHunter = spawnMobile("tatooine", "hero_of_tat_bh", 0, self.intellectSpawns[newLoc]["bhX"], self.intellectSpawns[newLoc]["bhZ"], self.intellectSpawns[newLoc]["bhY"], self.intellectSpawns[newLoc]["bhAngle"], 0)

    if not pBountyHunter then
        Logger:logEvent("HeroOfTatooineScreenPlay, unable to spawn bounty hunter.", LT_ERROR)
        return
    end

    writeData("hero_of_tat:intellect_mob_id", SceneObject(pBountyHunter):getObjectID())
    CreatureObject(pBountyHunter):setPvpStatusBitmask(0)
    AiAgent(pBountyHunter):addCreatureFlag(AI_STATIC)

    Logger:logEvent("Hero of Tatooine Intellect: doIntellectSpawn -- Bounty Hunter Spawned - New Loc #" .. newLoc .. " with Coordinates X: " .. self.intellectSpawns[newLoc]["bhX"] .. 
        " Z: " .. self.intellectSpawns[newLoc]["bhZ"] .. " Y: " .. self.intellectSpawns[newLoc]["bhY"], LT_INFO)

    self:spawnIntellectLiars(pBountyHunter)
end

function HeroOfTatooineScreenPlay:getIntellectLiarName(liarNum)
    local liarId = readData("hero_of_tat:liar_" .. liarNum + 1)
    local pLiar = getSceneObject(liarId)

    if not pLiar then
        return "unknown"
    end

    return string.match(SceneObject(pLiar):getCustomObjectName(), "%a+")
end

function HeroOfTatooineScreenPlay:sendImplicateSui(pPlayer, pNpc)
    if not isPlayer(pPlayer) or not pNpc then
        return
    end

    local liarTable = {}

    for i = 1, 5, 1 do
        local liarId = readData("hero_of_tat:liar_" .. i)
        local pLiar = getSceneObject(liarId)

        if pLiar then
            local option = {self:getIntellectLiarName(i), 0}
            table.insert(liarTable, option)
        end
    end

    local suiManager = LuaSuiManager()
    suiManager:sendListBox(pNpc, pPlayer, "@quest/hero_of_tatooine/intellect_liar:sui_title", "@quest/hero_of_tatooine/intellect_liar:sui_prompt", 2, 
        "@quest/hero_of_tatooine/intellect_liar:sui_btn_cancel", "", "@quest/hero_of_tatooine/intellect_liar:sui_btn_ok", self.screenplayName, "handleSuiImplication", 32, liarTable)

end

function HeroOfTatooineScreenPlay:handleSuiImplication(pPlayer, pSui, eventIndex, arg0)
    local cancelPressed = (eventIndex == 1)

    if not isPlayer(pPlayer) then
        return
    end

    if cancelPressed then
        return
    end

    local player = LuaCreatureObject(pPlayer)

    local pGhost = player:getPlayerObject()

    if pGhost == nil then
        return
    end

    local mobId = readData("hero_of_tat:intellect_mob_id")
    local pBountyHunter = getSceneObject(mobId)

    if not pBountyHunter then
        return
    end

    local liarNum = arg0 + 1

    if liarNum == 3 then
        local pInventory = SceneObject(pPlayer):getSlottedObject("inventory")

        if not pInventory then
            return
        end

        if (SceneObject(pInventory):isContainerFullRecursive()) then
            player:setScreenPlayState(2, "hero_of_tatooine_missing_marks") -- 1 - Altruism, 2 - Intellect, 4 - Courage, 8 - Honor
            player:sendSystemMessage("@quest/hero_of_tatooine/system_messages:intellect_inv_full")
        else
            local pMark = giveItem(pInventory, self.markOfIntellect, -1)

            if not pMark then
                player:setScreenPlayState(2, "hero_of_tatooine_missing_marks") -- 1 - Altruism, 2 - Intellect, 4 - Courage, 8 - Honor
                player:sendSystemMessage("Error creating Mark of Intellect object. Please create a support ticket.")
            else
                player:sendSystemMessage("@quest/hero_of_tatooine/system_messages:intellect_success")
            end
        end

        player:setScreenPlayState(8, "hero_of_tatooine")
        player:setScreenPlayState(2, "hero_of_tatooine_intellect")

        if not PlayerObject(pGhost):hasBadge(POI_TWOLIARS) then
            PlayerObject(pGhost):awardBadge(POI_TWOLIARS)
        end

        spatialChat(pBountyHunter, "@quest/hero_of_tatooine/intellect_liar:bh_win")
        createEvent(10 * 1000, self.screenplayName, "doIntellectSpawn", pBountyHunter, "")
    else
        spatialChat(pBountyHunter, "@quest/hero_of_tatooine/intellect_liar:bh_lose")
        writeData(player:getObjectID() .. ":hero_of_tat:failedIntellect", mobId)

        -- 10 minute despawn
        createEvent(10 * 60 * 1000, "HeroOfTatooineScreenPlay", "destroyIntellectMobs", nil, "")

        self:createIntellectEvent("respawn")
    end
end

function HeroOfTatooineScreenPlay:destroyIntellectMobs()
    Logger:logEvent("Hero of Tatooine Intellect: destroyIntellectMobs called", LT_INFO)

    local mobId = readData("hero_of_tat:intellect_mob_id")
    local mobLoc = readData("hero_of_tat:intellect_mob_loc")
    local pBountyHunter = getSceneObject(mobId)

    if isAiAgent(pBountyHunter) then
        AiAgent(pBountyHunter):doDespawn()
    end

    for i = 1, 6, 1 do
        local liarId = readData("hero_of_tat:liar_" .. i)
        local pLiar = getSceneObject(liarId)

        if isAiAgent(pLiar) then
            deleteData(SceneObject(pLiar):getObjectID() .. ":liarId")
            AiAgent(pLiar):doDespawn()
        end

        deleteData("hero_of_tat:liar_" .. i)
    end
end

function HeroOfTatooineScreenPlay:spawnIntellectLiars(pBountyHunter)
    local mobLoc = readData("hero_of_tat:intellect_mob_loc")
    local mobId = readData("hero_of_tat:intellect_mob_id")

    for i = 1, 6, 1 do
        local x, y, z

        if (i < 4) then
            x = self.intellectSpawns[mobLoc]["x1"]
            z = self.intellectSpawns[mobLoc]["z1"]
            y = self.intellectSpawns[mobLoc]["y1"]
        else
            x = self.intellectSpawns[mobLoc]["x2"]
            z = self.intellectSpawns[mobLoc]["z2"]
            y = self.intellectSpawns[mobLoc]["y2"]
        end

        if i == 1 or i == 4 then
            x = x - .75
            y = y + 1.3
        elseif i == 2 or i == 5 then
            x = x - .75
            y = y - 1.3
        elseif i == 3 or i == 6 then
            x = x + 1.5
        end

        local pLiar

        if i == 1 then
            pLiar = spawnMobile("tatooine", "hero_of_tat_smuggler_captain", 0, x, z, y, getRandomNumber(360) - 180, 0)
        else
            pLiar = spawnMobile("tatooine", "hero_of_tat_smuggler", 0, x, z, y, getRandomNumber(360) - 180, 0)
        end

        if not pLiar then
            Logger:logEvent("Unable to spawn Mark of Intellect smugglers.", LT_ERROR)
            self:destroyIntellectMobs() -- If not all were able to spawn, destroy them all until next spawn attempt
            return
        end


        CreatureObject(pLiar):setPvpStatusBitmask(0)
        AiAgent(pLiar):addCreatureFlag(AI_STATIC)

        Logger:logEvent("Hero of Tatooine Intellect: spawnIntellectLiars -- Liar: " .. SceneObject(pLiar):getDisplayedName() .. " ID: " .. SceneObject(pLiar):getObjectID() .. 
            " spawned at X: " .. x .. " Z: " .. z .. " Y: " .. y, LT_INFO)

        writeData("hero_of_tat:liar_" .. i, SceneObject(pLiar):getObjectID())
        writeData(SceneObject(pLiar):getObjectID() .. ":liarId", i)
    end
end

function HeroOfTatooineScreenPlay:onEnteredAltruismCave(pCave, pCreature)
    if not isPlayer(pCreature) or SceneObject(pCreature):getParentID() == 5995573 then
        return 0
    end

    local count = self:getCavePlayerWithQuestCount()

    if count == 1 then
        self:spawnAltruismObjects()
    end

    return 0
end

function HeroOfTatooineScreenPlay:onExitedAltruismCave(pCave, pCreature)
    if not isPlayer(pCreature) then
        return 0
    end

    local escorterID = readData("hero_of_tat:altruismEscorterID")

    if SceneObject(pCreature):getObjectID() == escorterID then
        self:completeEscort(pCreature)
        return 0
    end

    local count = self:getCavePlayerWithQuestCount()

    if count == 0 then
        self:despawnAltruismObjects()
    end

    return 0
end

function HeroOfTatooineScreenPlay:despawnAltruismObjects()
    local objectIDs = {
        readData("hero_of_tat:altruismWall"),
        readData("hero_of_tat:altruismCrevice"),
        readData("hero_of_tat:farmerWifeId"),
        readData("hero_of_tat:farmerChildId"),
        readData("hero_of_tat:altruismCrate")
    }

    for i = 1, #objectIDs, 1 do
        if objectIDs[i] ~= 0 then
            local pObject = getSceneObject(objectIDs[i])

            if pObject then
                if isAiAgent(pObject) then
                    AiAgent(pObject):doDespawn()
                else
                    SceneObject(pObject):destroyObjectFromWorld()
                end
            end
        end
    end
    deleteData("hero_of_tat:altruismWall")
    deleteData("hero_of_tat:altruismCrevice")
    deleteData("hero_of_tat:farmerWifeId")
    deleteData("hero_of_tat:farmerChildId")
    deleteData("hero_of_tat:altruismCrate")
    deleteData("hero_of_tat:altruismEscorterID") -- No escorter
    deleteData("hero_of_tat:altruismEscortStatus") -- No one doing escort
end

function HeroOfTatooineScreenPlay:getCavePlayerWithQuestCount()
    local pAltCave = getSceneObject(9995371) -- Sennex cave for Altruism quest

    if not pAltCave then
        return 0
    end

    local playerCount = 0
    local AltruismCave = LuaBuildingObject(pAltCave)

    for i = 1, AltruismCave:getTotalCellNumber(), 1 do
        local pCell = AltruismCave:getCell(i)
        if pCell then
            local cell = LuaSceneObject(pCell)
            for j = cell:getContainerObjectsSize() - 1, 0, -1 do
                local pObject = cell:getContainerObject(j)

                -- If object exists, is a player, and has started but not completed Altruism quest
                if
                    isPlayer(pObject) and CreatureObject(pObject):hasScreenPlayState(1, "hero_of_tatooine_altruism") and
                        not CreatureObject(pObject):hasScreenPlayState(2, "hero_of_tatooine_altruism")
                 then
                    playerCount = playerCount + 1
                end
            end
        end
    end

    return playerCount
end

function HeroOfTatooineScreenPlay:giveAltruismWaypoint(pPlayer)
    if not isPlayer(pPlayer) then
        return
    end

    local pGhost = CreatureObject(pPlayer):getPlayerObject()

    if not pGhost then
        return
    end

    local playerID = CreatureObject(pPlayer):getObjectID()
    local oldWaypointID = tonumber(getQuestStatus(playerID .. ":altruismWaypointID"))

    if oldWaypointID ~= 0 then
        PlayerObject(pGhost):removeWaypoint(oldWaypointID, true)
        removeQuestStatus(playerID .. ":altruismWaypointID")
    end

    local waypointID = PlayerObject(pGhost):addWaypoint("tatooine", "Kidnapped Family", "", 6555, 0, -1311, WAYPOINT_PURPLE, true, true, 0, 1)
    setQuestStatus(playerID .. ":altruismWaypointID", waypointID)
end

function HeroOfTatooineScreenPlay:removeAltruismWaypoint(pPlayer)
    if not isPlayer(pPlayer) then
        return
    end

    local pGhost = CreatureObject(pPlayer):getPlayerObject()

    if not pGhost then
        return
    end

    local playerID = CreatureObject(pPlayer):getObjectID()
    local oldWaypointID = tonumber(getQuestStatus(playerID .. ":altruismWaypointID"))

    if oldWaypointID ~= 0 then
        PlayerObject(pGhost):removeWaypoint(oldWaypointID, true)
        removeQuestStatus(playerID .. ":altruismWaypointID")
    end
end

function HeroOfTatooineScreenPlay:destroyCaveWall(pCrevice)
    local wallID = readData("hero_of_tat:altruismWall")
    local pWall = getSceneObject(wallID)

    if not pCrevice or not pWall then
        return
    end

    local planterId = readData("hero_of_tat:explosivePlanterID")
    local pPlanter = getSceneObject(planterId)

    if pPlanter then
        playClientEffectLoc(pPlanter, "clienteffect/lair_damage_heavy_shake.cef", "tatooine", 162.5, -66.8, -97.7, 5995573)
        playClientEffectLoc(pPlanter, "clienteffect/lair_damage_heavy_shake.cef", "tatooine", 150.96, -65.83, -97.66, 5995573)
        deleteData("hero_of_tat:explosivePlanterID")
    end

    SceneObject(pCrevice):destroyObjectFromWorld()
    SceneObject(pWall):destroyObjectFromWorld()
end

function HeroOfTatooineScreenPlay:doGiverDespawn(pGiver)
    if not isAiAgent(pGiver) then
        return
    end

    giver = LuaAiAgent(pGiver)

    giver:addCreatureFlag(AI_NOAIAGGRO)
    giver:addCreatureFlag(AI_FOLLOW)
    giver:setMovementState(AI_PATROLLING)
    giver:generatePatrol(1, 30)
    createObserver(DESTINATIONREACHED, self.screenplayName, "giverDespawnDestinationReached", pGiver)
end

function HeroOfTatooineScreenPlay:giverDespawnDestinationReached(pGiver)
    if not isAiAgent(pGiver) then
        return 1
    end
    createEvent(2000, self.screenplayName, "destroyGiver", pGiver, "")
    return 1
end

function HeroOfTatooineScreenPlay:destroyGiver(pGiver)
    if isAiAgent(pGiver) then
        deleteData(CreatureObject(pGiver):getObjectID() .. ":gaveQuest")
        AiAgent(pGiver):doDespawn()
    end
end

function HeroOfTatooineScreenPlay:failEscort(pPlayer)
    if isPlayer(pPlayer) then
        CreatureObject(pPlayer):sendSystemMessage("@quest/hero_of_tatooine/system_messages:altruism_quest_fail")
    end
    self:despawnAltruismObjects()
end

function HeroOfTatooineScreenPlay:escortRangeCheckEvent(pPlayer)
    if not isPlayer(pPlayer) then
        return
    end

    local pWife = getSceneObject(readData("hero_of_tat:farmerWifeId"))

    if not isAiAgent(pWife) and readData("hero_of_tat:altruismEscortStatus") == 1 then
        self:failEscort(pPlayer)
        return
    end

    if readData("hero_of_tat:altruismEscortStatus") == 0 or SceneObject(pPlayer):getParentID() == 0 then
        return
    end

    if isAiAgent(pWife) and CreatureObject(pPlayer):isInRangeWithObject(pWife, 32) then
        createEvent(1000, self.screenplayName, "escortRangeCheckEvent", pPlayer, "")
    else
        self:failEscort(pPlayer)
    end
end

function HeroOfTatooineScreenPlay:completeEscort(pPlayer)
    if not isPlayer(pPlayer) then
        return
    end

    local player = LuaCreatureObject(pPlayer)

    local pGhost = player:getPlayerObject()

    if not pGhost then
        return
    end

    local pWife = getSceneObject(readData("hero_of_tat:farmerWifeId"))

    if isAiAgent(pWife) then
        spatialChat(pWife, "@quest/hero_of_tatooine/system_messages:altruism_npc_farewell")
        AiAgent(pWife):addCreatureFlag(AI_NOAIAGGRO)
        AiAgent(pWife):addCreatureFlag(AI_FOLLOW)
    end

    deleteData("hero_of_tat:altruismEscortStatus")
    player:sendSystemMessage("@quest/hero_of_tatooine/system_messages:altruism_quest_success")

    local pInventory = SceneObject(pPlayer):getSlottedObject("inventory")

    if not pInventory then
        return 0
    end

    if SceneObject(pInventory):isContainerFullRecursive() then
        player:setScreenPlayState(1, "hero_of_tatooine_missing_marks") -- 1 - Altruism, 2 - Intellect, 4 - Courage, 8 - Honor
        player:sendSystemMessage("@quest/hero_of_tatooine/system_messages:altruism_inv_full")
    else
        local pMark = giveItem(pInventory, self.markOfAltruism, -1)

        if not pMark then
            player:setScreenPlayState(1, "hero_of_tatooine_missing_marks") -- 1 - Altruism, 2 - Intellect, 4 - Courage, 8 - Honor
            player:sendSystemMessage("Error creating Mark of Altruism object. Please create a support ticket.")
        else
            player:sendSystemMessage("@quest/hero_of_tatooine/system_messages:altruism_success")
        end
    end

    if not PlayerObject(pGhost):hasBadge(POI_FACTORYLIBERATION) then
        PlayerObject(pGhost):awardBadge(POI_FACTORYLIBERATION)
    end

    player:setScreenPlayState(4, "hero_of_tatooine")
    player:setScreenPlayState(2, "hero_of_tatooine_altruism")
    self:removeAltruismWaypoint(pPlayer)
    createEvent(3000, self.screenplayName, "despawnAltruismObjects", pPlayer, "")
end

function HeroOfTatooineScreenPlay:doHonorChange()
    if not isZoneEnabled("tatooine") then
        return
    end

    local hermitId = readData("hero_of_tat:hermit_id")
    local pHermit = getSceneObject(hermitId)

    if not pHermit then
        Logger:logEvent("Hero of Tatooine: doHonorChange -- unable to find hermit object ", LT_ERROR)
        return
    end

    local leaderId = readData("hero_of_tat:honor_leader_id")
    local pirate1Id = readData("hero_of_tat:honor_pirate_1_id")
    local pirate2Id = readData("hero_of_tat:honor_pirate_2_id")
    local mobLoc = readData("hero_of_tat:honor_leader_loc")
    local pLeader = getSceneObject(leaderId)
    local pPirate1 = getSceneObject(pirate1Id)
    local pPirate2 = getSceneObject(pirate2Id)

    -- Reschedule respawn if leader OR either pirate is in combat (Infinity GH #1653: guard previously only checked the leader)
    local leaderInCombat = pLeader and AiAgent(pLeader):isInCombat()
    local pirate1InCombat = pPirate1 and AiAgent(pPirate1):isInCombat()
    local pirate2InCombat = pPirate2 and AiAgent(pPirate2):isInCombat()

    if leaderInCombat or pirate1InCombat or pirate2InCombat then
        self:createHonorEvent("life")
        return
    end

    if isAiAgent(pPirate1) then
        AiAgent(pPirate1):doDespawn()
        deleteData("hero_of_tat:honor_pirate_1_id")
    end

    if isAiAgent(pPirate2) then
        AiAgent(pPirate2):doDespawn()
        deleteData("hero_of_tat:honor_pirate_2_id")
    end

    if isAiAgent(pLeader) then
        AiAgent(pLeader):doDespawn()
        deleteData("hero_of_tat:honor_leader_id")
        self:createHonorEvent("respawn")
        return
    end

    local newLoc = getRandomNumber(1, #self.honorSpawns)

    if newLoc == mobLoc then
        if newLoc == #self.honorSpawns then
            newLoc = newLoc - 1
        else
            newLoc = newLoc + 1
        end
    end

    writeData("hero_of_tat:honor_leader_loc", newLoc)

    local x = self.honorSpawns[newLoc][1]
    local y = self.honorSpawns[newLoc][2]

    -- {zoneName, x, y, minDist, maxDist}
    local spawnPoint = getSpawnPoint("tatooine", x, y, 10, 512)

    if not spawnPoint then
        createEvent(30 * 1000, "HeroOfTatooineScreenPlay", "doHonorChange", nil, "")
        return
    end

    local z = getWorldFloor(spawnPoint[1], spawnPoint[3], "tatooine")

    pLeader = spawnMobile("tatooine", "hero_of_tat_pirate_leader", 0, spawnPoint[1], z, spawnPoint[3], getRandomNumber(360) - 180, 0)

    if not pLeader then
        Logger:logEvent("Failed to create leader in HeroOfTatooineScreenPlay:doHonorChange().", LT_ERROR)
        return
    end


    Logger:logEvent("Hero of Tatooine: doHonorChange - Spawned Pirate Leader at " .. round(spawnPoint[1]) .. ", " .. round(spawnPoint[3]) .. " Tatooine using base Spawn Point - X: " .. x .. " Y: " .. y, LT_INFO)

    AiAgent(pLeader):addCreatureFlag(AI_NOAIAGGRO)
    writeData("hero_of_tat:honor_leader_id", SceneObject(pLeader):getObjectID())

    pPirate1 = spawnMobile("tatooine", "pirate", 0, spawnPoint[1], z, spawnPoint[3], getRandomNumber(360) - 180, 0)

    if pPirate1 then
        AiAgent(pPirate1):addCreatureFlag(AI_NOAIAGGRO)
        writeData("hero_of_tat:honor_pirate_1_id", SceneObject(pPirate1):getObjectID())
    end

    pPirate2 = spawnMobile("tatooine", "pirate", 0, spawnPoint[1], z, spawnPoint[3], getRandomNumber(360) - 180, 0)

    if pPirate2 then
        AiAgent(pPirate2):addCreatureFlag(AI_NOAIAGGRO)
        writeData("hero_of_tat:honor_pirate_2_id", SceneObject(pPirate2):getObjectID())
    end

    createObserver(DAMAGERECEIVED, self.screenplayName, "pirateLeaderDamage", pLeader)

    self:createHonorEvent("life")
end

function HeroOfTatooineScreenPlay:pirateLeaderDamage(pLeader, pPlayer, damage)
    if not isAiAgent(pLeader) or not isPlayerOrPet(pPlayer) then
        return 1
    end

    local player = toLuaCreatureObject(petOwner(pPlayer))

    if
        not player or not player:hasScreenPlayState(2, "hero_of_tatooine") or player:hasScreenPlayState(32, "hero_of_tatooine") or
            player:hasScreenPlayState(1, "hero_of_tatooine_honor") or
            player:hasScreenPlayState(2, "hero_of_tatooine_honor")
     then
        return 0
    end

    local pirateLeader = LuaCreatureObject(pLeader)

    -- Infinity:  HAM threshold is now 50% of total HAM
    local totalHamThreshold = round((pirateLeader:getMaxHAM(HEALTH) + pirateLeader:getMaxHAM(ACTION) + pirateLeader:getMaxHAM(MIND)) * 0.50)
    local currentTotalHam = pirateLeader:getHAM(HEALTH) + pirateLeader:getHAM(ACTION) + pirateLeader:getHAM(MIND)

    if (currentTotalHam > totalHamThreshold) then
        return 0
    end

    local spawnLoc = {
        x = pirateLeader:getPositionX(),
        z = pirateLeader:getPositionZ(),
        y = pirateLeader:getPositionY(),
        cell = pirateLeader:getParentID(),
        angle = pirateLeader:getDirectionAngle()
    }

    player:forcePeace()
    pirateLeader:forcePeace()

    pirateLeader:setPvpStatusBitmask(0)
    pirateLeader:setOptionBit(CONVERSABLE)
    pirateLeader:setPosture(UPRIGHT)
    AiAgent(pLeader):setHomeLocation(spawnLoc.x, spawnLoc.z, spawnLoc.y, 0)

    AiAgent(pLeader):addCreatureFlag(AI_STATIONARY)
    AiAgent(pLeader):setAITemplate()

    spatialChat(pLeader, "@quest/pirates:dont_hurt_us")

    Logger:logEvent("Hero of Tatooine: pirateLeaderDamage - Pirate Leader was defeated at location: " .. spawnLoc.x .. ", " .. spawnLoc.y .. " Tatooine.", LT_INFO)

    return 1
end

function HeroOfTatooineScreenPlay:spawnAggroLeader(pOldLeader, pPlayer, screenID)
    if not isAiAgent(pOldLeader) or not isPlayer(pPlayer) then
        return
    end

    local oldLeader = LuaCreatureObject(pOldLeader)

    local spawnLoc = {x = oldLeader:getPositionX(), z = oldLeader:getPositionZ(), y = oldLeader:getPositionY(), cell = oldLeader:getParentID(), angle = oldLeader:getDirectionAngle()}

    local spawnHam = {h = oldLeader:getHAM(HEALTH), a = oldLeader:getHAM(ACTION), m = oldLeader:getHAM(MIND)}
    local leaderName = SceneObject(pOldLeader):getCustomObjectName()

    AiAgent(pOldLeader):doDespawn()

    local pNewLeader = spawnMobile("tatooine", "hero_of_tat_pirate_leader", 0, spawnLoc.x, spawnLoc.z, spawnLoc.y, spawnLoc.angle, spawnLoc.cell)

    if not pNewLeader then
        return
    end

    local newLeader = LuaCreatureObject(pNewLeader)

    SceneObject(pNewLeader):setCustomObjectName(leaderName)
    newLeader:setHAM(HEALTH, spawnHam.h)
    newLeader:setHAM(ACTION, spawnHam.a)
    newLeader:setHAM(MIND, spawnHam.m)

    if (screenID == "lets_get_em") then
        spatialChat(pNewLeader, "@conversation/quest_hero_of_tatooine_pirate_leader:s_cacfa6a2")
    elseif (screenID == "is_that_so") then
        spatialChat(pNewLeader, "@conversation/quest_hero_of_tatooine_pirate_leader:s_5a3d905f")
    elseif (screenID == "teach_a_lesson") then
        spatialChat(pNewLeader, "@conversation/quest_hero_of_tatooine_pirate_leader:s_d9706ae2")
    end

    createObserver(DAMAGERECEIVED, self.screenplayName, "pirateLeaderDamage", pNewLeader)

    newLeader:engageCombat(pPlayer)
end

function HeroOfTatooineScreenPlay:giveHonorWaypoint(pPlayer)
    if not isPlayer(pPlayer) then
        return
    end

    local pGhost = CreatureObject(pPlayer):getPlayerObject()

    if not pGhost then
        return
    end

    local playerID = CreatureObject(pPlayer):getObjectID()
    local oldWaypointID = tonumber(getQuestStatus(playerID .. ":honorWaypointID"))

    if oldWaypointID ~= 0 then
        PlayerObject(pGhost):removeWaypoint(oldWaypointID, true)
        removeQuestStatus(playerID .. ":honorWaypointID")
    end

    local waypointID = PlayerObject(pGhost):addWaypoint("tatooine", "Ranch House", "", 4993, 0, -4682, WAYPOINT_PURPLE, true, true, 0, 1)
    setQuestStatus(playerID .. ":honorWaypointID", waypointID)
end

function HeroOfTatooineScreenPlay:initRanchHouse()
    local pBedroom = getSceneObject(4005944) -- Ranch House bedroom

    if not pBedroom then
        Logger:logEvent("Hero of Tatooine:  initRanchHouse -- unable to find ranch house bedroom", LT_ERROR)
        return
    end

    local bedroom = LuaSceneObject(pBedroom)

    bedroom:setContainerInheritPermissionsFromParent(false)
    bedroom:clearContainerDefaultDenyPermission(WALKIN)
    bedroom:clearContainerDefaultAllowPermission(WALKIN)
    bedroom:setContainerAllowPermission("heroOfTatRanchBedroom", WALKIN)
    bedroom:setContainerDenyPermission("heroOfTatRanchBedroom", MOVEIN)

    --TODO: Droid temporary until we can come up with a way to make TANO's conversible
    --local pObject = spawnSceneObject("tatooine", "object/tangible/terminal/terminal_elevator_up.iff", -6.33, -3.57, -7.27, 4005942, 0.0436, 0, -0.999, 0)
    local pDroid = spawnMobile("tatooine", "hero_of_tat_intercom_droid", 0, -6.33, -4.0, -7.9, 180, 4005942)

    if pDroid then
        TangibleObject(pDroid):setPvpStatusBitmask(0)
        writeData("hero_of_tat:ranch_house_speaker_box", SceneObject(pDroid):getObjectID())
    else
        Logger:logEvent("Hero of Tatooine:  initRanchHouse -- unable to spawn speaker box", LT_ERROR)
    end

    local pWife = spawnMobile("tatooine", "hero_of_tat_ranchers_wife", 0, -6.53, 0.03, -3.21, 0, 4005941)

    if pWife then
        writeData("hero_of_tat:ranch_house_wife", SceneObject(pWife):getObjectID())
        CreatureObject(pWife):setPvpStatusBitmask(0)
    else
        Logger:logEvent("Hero of Tatooine:  initRanchHouse -- unable to spawn rancher's wife", LT_ERROR)
    end

    Logger:logEvent("Hero of Tatooine: initRanchHouse - Rancher house initialized.", LT_INFO)
end

function HeroOfTatooineScreenPlay:doHonorFail(pPlayer)
    if not isPlayer(pPlayer) then
        return
    end

    local pPirate1 = spawnMobile("tatooine", "fugitive", 0, -9.0, -4.0, -8.6, 0, 4005942)
    local pPirate2 = spawnMobile("tatooine", "fugitive", 0, -7.3, -4.0, -9.4, 0, 4005942)

    if not pPirate1 or not pPirate2 then
        return
    end

    CreatureObject(pPirate1):setPvpStatusBitmask(0)
    CreatureObject(pPirate2):setPvpStatusBitmask(0)

    deleteData("hero_of_tat:honor_result") -- 0 = fail, 1 = success

    writeData("hero_of_tat:honor_pirate1", SceneObject(pPirate1):getObjectID())
    writeData("hero_of_tat:honor_pirate1_step", 1) -- First step, spawn mobile
    writeData("hero_of_tat:honor_pirate2", SceneObject(pPirate2):getObjectID())
    writeData("hero_of_tat:honor_pirate2_step", 1) -- First step, spawn mobile

    createObserver(DESTINATIONREACHED, self.screenplayName, "honorDestReached", pPirate1)
    createObserver(DESTINATIONREACHED, self.screenplayName, "honorDestReached", pPirate2)

    local pIntercom = getSceneObject(readData("hero_of_tat:ranch_house_speaker_box"))
    local pWife = getSceneObject(readData("hero_of_tat:ranch_house_wife"))

    if pIntercom then
        createEvent(500, self.screenplayName, "setNotConversable", pIntercom, "")
    end

    if isAiAgent(pWife) then
        createEvent(500, self.screenplayName, "setNotConversable", pWife, "")
    end

    --Explosion effect
    playClientEffectLoc(pPlayer, "clienteffect/combat_grenade_thermal_detonator.cef", "tatooine", -4.8, 0.3, -2.3, 4005941)
    --House shake effect
    playClientEffectLoc(pPlayer, "clienteffect/cr_bodyfall_huge.cef", "tatooine", -4.8, 0.3, -2.3, 4005941)

    createEvent(500, self.screenplayName, "doStartPatrol", pPirate1, "")
    createEvent(500, self.screenplayName, "doStartPatrol", pPirate2, "")

    createEvent(3500, self.screenplayName, "doFailHonorPhase", pPlayer, "")
    createEvent(120000, self.screenplayName, "doRanchHouseCleanup", pPlayer, "")
end

function HeroOfTatooineScreenPlay:setNotConversable(pNpc)
    if isCreatureObject(pNpc) then
        CreatureObject(pNpc):clearOptionBit(CONVERSABLE) -- No longer conversable
    end
end

function HeroOfTatooineScreenPlay:doStartPatrol(pNpc)
    if isAiAgent(pNpc) then
        local agent = LuaAiAgent(pNpc)
        agent:addCreatureFlag(AI_NOAIAGGRO)
        agent:addCreatureFlag(AI_ESCORT)
        agent:setMovementState(AI_PATROLLING)
        HeroOfTatooineScreenPlay:doHonorStep(pNpc)
    end
end

function HeroOfTatooineScreenPlay:doFailHonorPhase(pPlayer)
    if not isPlayer(pPlayer) then
        return
    end

    CreatureObject(pPlayer):sendSystemMessage("@quest/hero_of_tatooine/system_messages:failed")
    CreatureObject(pPlayer):removeScreenPlayState(1, "hero_of_tatooine_honor")
end

function HeroOfTatooineScreenPlay:doSuccessHonorPhase(pPlayer)
    if not isPlayer(pPlayer) then
        return
    end

    local player = LuaCreatureObject(pPlayer)

    local pGhost = player:getPlayerObject()

    if not pGhost then
        return
    end

    player:setScreenPlayState(2, "hero_of_tatooine_honor")

    if not PlayerObject(pGhost):hasBadge(POI_PRISONBREAK) then
        PlayerObject(pGhost):awardBadge(POI_PRISONBREAK)
    end

    player:setScreenPlayState(32, "hero_of_tatooine")

    local pInventory = player:getSlottedObject("inventory")

    if not pInventory then
        return
    end

    if SceneObject(pInventory):isContainerFullRecursive() then
        player:setScreenPlayState(8, "hero_of_tatooine_missing_marks") -- 1 - Altruism, 2 - Intellect, 4 - Courage, 8 - Honor
        player:sendSystemMessage("@quest/hero_of_tatooine/system_messages:honor_inv_full")
    else
        local pMark = giveItem(pInventory, self.markOfHonor, -1)

        if not pMark then
            player:setScreenPlayState(8, "hero_of_tatooine_missing_marks") -- 1 - Altruism, 2 - Intellect, 4 - Courage, 8 - Honor
            player:sendSystemMessage("Error creating Mark of Honor object. Please create a support ticket.")
        else
            player:sendSystemMessage("@quest/hero_of_tatooine/system_messages:honor_success")
        end
    end
end

function HeroOfTatooineScreenPlay:doRanchHouseCleanup(pPlayer)
    local pirate1ID = readData("hero_of_tat:honor_pirate1")
    local pirate2ID = readData("hero_of_tat:honor_pirate2")
    local intercomID = readData("hero_of_tat:ranch_house_speaker_box")
    local wifeID = readData("hero_of_tat:ranch_house_wife")
    local trooper1ID = readData("hero_of_tat:honor_trooper1")
    local trooper2ID = readData("hero_of_tat:honor_trooper2")
    local rancherID = readData("hero_of_tat:honor_rancher")

    local pObject = getSceneObject(pirate1ID)
    self:destroyRanchHouseObject(pObject)
    pObject = getSceneObject(pirate2ID)
    self:destroyRanchHouseObject(pObject)
    pObject = getSceneObject(intercomID)
    self:destroyRanchHouseObject(pObject)
    pObject = getSceneObject(wifeID)
    self:destroyRanchHouseObject(pObject)
    pObject = getSceneObject(trooper1ID)
    self:destroyRanchHouseObject(pObject)
    pObject = getSceneObject(trooper2ID)
    self:destroyRanchHouseObject(pObject)
    pObject = getSceneObject(rancherID)
    self:destroyRanchHouseObject(pObject)

    deleteData("hero_of_tat:honor_pirate1")
    deleteData("hero_of_tat:honor_pirate2")
    deleteData("hero_of_tat:honor_trooper1")
    deleteData("hero_of_tat:honor_trooper2")
    deleteData("hero_of_tat:honor_rancher")
    deleteData("hero_of_tat:ranch_house_speaker_box")
    deleteData("hero_of_tat:ranch_house_wife")
    deleteData("hero_of_tat:honor_pirate1_step")
    deleteData("hero_of_tat:honor_pirate2_step")
    deleteData("hero_of_tat:ranch_player_id")
    deleteData("hero_of_tat:honor_result")

    local playerID = CreatureObject(pPlayer):getObjectID()

    deleteData(playerID .. ":hero_of_tat_honor:accepted")
    deleteData(playerID .. ":hero_of_tat_honor:distracting_wife")
    deleteData(playerID .. ":hero_of_tat_honor:spoke_to_wife")

    self:initRanchHouse()
end

function HeroOfTatooineScreenPlay:destroyRanchHouseObject(pObject)
    if isAiAgent(pObject) then
        AiAgent(pObject):doDespawn()
    end
end

function HeroOfTatooineScreenPlay:honorDestReached(pAgent)
    if not isAiAgent(pAgent) then
        return 0
    end

    local delay = 1500

    local honorResult = readData("hero_of_tat:honor_result")
    local pirate1ID = readData("hero_of_tat:honor_pirate1")
    local pirate2ID = readData("hero_of_tat:honor_pirate2")
    local trooper1ID = readData("hero_of_tat:honor_trooper1")
    local trooper2ID = readData("hero_of_tat:honor_trooper2")
    local rancherID = readData("hero_of_tat:honor_rancher")

    local agentID = SceneObject(pAgent):getObjectID()

    if agentID == trooper1ID then
        local curStep = readData("hero_of_tat:honor_trooper1_step")

        if curStep == 2 then
            createEvent(5500, self.screenplayName, "doHonorSuccessPirates", pAgent, "")
            delay = 5000
        end
    elseif agentID == trooper2ID then
        local curStep = readData("hero_of_tat:honor_trooper2_step")

        if curStep == 2 then
            delay = 6500
        end
    elseif agentID == pirate1ID then
        local curStep = readData("hero_of_tat:honor_pirate1_step")

        if curStep == 2 then
            spatialChat(pAgent, "@quest/pirates:ruin_life")

            if honorResult == 0 then
                local pWife = getSceneObject(readData("hero_of_tat:ranch_house_wife"))

                if pWife then
                    CreatureObject(pWife):setPosture(13)
                end

                CreatureObject(pAgent):doAnimation("cheer")
            else
                CreatureObject(pAgent):doAnimation("wave_finger_warning")
            end
        end
    elseif agentID == rancherID then
        local curStep = readData("hero_of_tat:honor_rancher_step")

        if curStep == 2 then
            spatialChat(pAgent, "@quest/hero_of_tatooine/rancher:you_okay")

            local pWife = getSceneObject(readData("hero_of_tat:ranch_house_wife"))

            if pWife then
                spatialChat(pWife, "@quest/hero_of_tatooine/rancher:glad_here")
            end
        end
    end

    createEvent(delay, self.screenplayName, "doHonorStep", pAgent, "")
    return 0
end

function HeroOfTatooineScreenPlay:doHonorStep(pAgent)
    if not isAiAgent(pAgent) then
        return
    end

    local honorResult = readData("hero_of_tat:honor_result")
    local coordTable

    if honorResult == 0 then
        coordTable = self.honorFailPoints
    else
        coordTable = self.honorSuccessPoints
    end

    local agentID = SceneObject(pAgent):getObjectID()

    local pirate1ID = readData("hero_of_tat:honor_pirate1")
    local pirate2ID = readData("hero_of_tat:honor_pirate2")
    local trooper1ID = readData("hero_of_tat:honor_trooper1")
    local trooper2ID = readData("hero_of_tat:honor_trooper2")
    local rancherID = readData("hero_of_tat:honor_rancher")

    local curStep
    local patrolPoints

    if agentID == pirate1ID then -- Pirate1, success or fail
        curStep = readData("hero_of_tat:honor_pirate1_step")
        writeData("hero_of_tat:honor_pirate1_step", curStep + 1)
        patrolPoints = coordTable.pirate1
    elseif agentID == pirate2ID then -- Pirate2, success or fail
        curStep = readData("hero_of_tat:honor_pirate2_step")
        writeData("hero_of_tat:honor_pirate2_step", curStep + 1)
        patrolPoints = coordTable.pirate2
    elseif agentID == trooper1ID then -- Trooper 1, success only
        curStep = readData("hero_of_tat:honor_trooper1_step")
        writeData("hero_of_tat:honor_trooper1_step", curStep + 1)
        patrolPoints = coordTable.trooper1
    elseif agentID == trooper2ID then -- Trooper 2, success only
        curStep = readData("hero_of_tat:honor_trooper2_step")
        writeData("hero_of_tat:honor_trooper2_step", curStep + 1)
        patrolPoints = coordTable.trooper2
    elseif agentID == rancherID then -- Rancher, success only
        curStep = readData("hero_of_tat:honor_rancher_step")
        writeData("hero_of_tat:honor_rancher_step", curStep + 1)
        patrolPoints = coordTable.rancher
    else
        return
    end

    local nextPoint = patrolPoints[curStep]

    if not nextPoint then
        if agentID ~= rancherID then
            AiAgent(pAgent):doDespawn()
        end
        return
    end

    AiAgent(pAgent):stopWaiting()
    AiAgent(pAgent):setNextPosition(nextPoint[1], nextPoint[2], nextPoint[3], nextPoint[4])
    AiAgent(pAgent):executeBehavior()
end

function HeroOfTatooineScreenPlay:doHonorStart(pPlayer)
    if not isPlayer(pPlayer) then
        return
    end

    local playerID = SceneObject(pPlayer):getObjectID()
    writeData(playerID .. ":hero_of_tat_honor:accepted", 1)
    writeData("hero_of_tat:ranch_player_id", playerID)

    createEvent(7 * 60 * 1000, self.screenplayName, "doHonorTimeout", pPlayer, "") -- 7 Minute timeout
end

function HeroOfTatooineScreenPlay:doHonorTimeout(pPlayer)
    if not isPlayer(pPlayer) then
        return
    end

    local curRanchPlayer = readData("hero_of_tat:ranch_player_id")

    if SceneObject(pPlayer):getObjectID() ~= curRanchPlayer then
        return
    end

    CreatureObject(pPlayer):sendSystemMessage("@quest/hero_of_tatooine/system_messages:restart")
    self:doRanchHouseCleanup(pPlayer)
end

function HeroOfTatooineScreenPlay:doHonorSuccess(pPlayer)
    if not isPlayer(pPlayer) then
        return
    end

    local pTrooper1 = spawnMobile("tatooine", "elite_sand_trooper", 0, -2.0, 0.6, 10.2, 0, 4005939)
    local pTrooper2 = spawnMobile("tatooine", "elite_sand_trooper", 0, -4.0, 0.6, 10.2, 0, 4005939)
    local pRancher = spawnMobile("tatooine", "hero_of_tat_rancher", 0, 0, 0.6, 10.2, 0, 4005939)

    if not pTrooper1 or not pTrooper2 or not pRancher then
        return
    end

    writeData("hero_of_tat:honor_result", 1) -- 0 = fail, 1 = success

    CreatureObject(pTrooper1):setPvpStatusBitmask(0)
    CreatureObject(pTrooper2):setPvpStatusBitmask(0)
    CreatureObject(pRancher):setPvpStatusBitmask(0)

    writeData("hero_of_tat:honor_trooper1", SceneObject(pTrooper1):getObjectID())
    writeData("hero_of_tat:honor_trooper1_step", 1) -- First step, spawn mobile
    writeData("hero_of_tat:honor_trooper2", SceneObject(pTrooper2):getObjectID())
    writeData("hero_of_tat:honor_trooper2_step", 1) -- First step, spawn mobile
    writeData("hero_of_tat:honor_rancher", SceneObject(pRancher):getObjectID())
    writeData("hero_of_tat:honor_rancher_step", 1) -- First step, spawn mobile

    createObserver(DESTINATIONREACHED, self.screenplayName, "honorDestReached", pTrooper1)
    createObserver(DESTINATIONREACHED, self.screenplayName, "honorDestReached", pTrooper2)
    createObserver(DESTINATIONREACHED, self.screenplayName, "honorDestReached", pRancher)

    local pIntercom = getSceneObject(readData("hero_of_tat:ranch_house_speaker_box"))
    local pWife = getSceneObject(readData("hero_of_tat:ranch_house_wife"))

    if pIntercom then
        createEvent(500, self.screenplayName, "setNotConversable", pIntercom, "")
    end

    if pWife then
        createEvent(500, self.screenplayName, "setNotConversable", pWife, "")
    end

    createEvent(500, self.screenplayName, "doStartPatrol", pRancher, "")
    createEvent(500, self.screenplayName, "doStartPatrol", pTrooper1, "")
    createEvent(500, self.screenplayName, "doStartPatrol", pTrooper2, "")

    createEvent(120000, self.screenplayName, "doRanchHouseCleanup", pPlayer, "")
end

function HeroOfTatooineScreenPlay:doHonorSuccessPirates()
    local pPirate1 = spawnMobile("tatooine", "fugitive", 0, -9.0, -4.0, -8.6, 0, 4005942)
    local pPirate2 = spawnMobile("tatooine", "fugitive", 0, -7.3, -4.0, -9.4, 0, 4005942)

    if not pPirate1 or not pPirate2 then
        return
    end

    CreatureObject(pPirate1):setPvpStatusBitmask(0)
    CreatureObject(pPirate2):setPvpStatusBitmask(0)

    writeData("hero_of_tat:honor_pirate1", SceneObject(pPirate1):getObjectID())
    writeData("hero_of_tat:honor_pirate1_step", 1) -- First step, spawn mobile
    writeData("hero_of_tat:honor_pirate2", SceneObject(pPirate2):getObjectID())
    writeData("hero_of_tat:honor_pirate2_step", 1) -- First step, spawn mobile

    createObserver(DESTINATIONREACHED, self.screenplayName, "honorDestReached", pPirate1)
    createObserver(DESTINATIONREACHED, self.screenplayName, "honorDestReached", pPirate2)

    createEvent(500, self.screenplayName, "doStartPatrol", pPirate1, "")
    createEvent(500, self.screenplayName, "doStartPatrol", pPirate2, "")
end

function HeroOfTatooineScreenPlay:onEnteredRanchHouse(pHouse, pObject)
    if not isPlayer(pObject) then
        return 0
    end

    local player = LuaCreatureObject(pObject)

    if player:hasScreenPlayState(1, "hero_of_tatooine_honor") and not player:hasScreenPlayState(2, "hero_of_tatooine_honor") then
        player:sendSystemMessage("@quest/hero_of_tatooine/system_messages:enter")
    end

    return 0
end

function HeroOfTatooineScreenPlay:isMissingMark(pPlayer)
    if not isPlayer(pPlayer) then
        return false
    end

    local player = LuaCreatureObject(pPlayer)

    return player:hasScreenPlayState(1, "hero_of_tatooine_missing_marks") or player:hasScreenPlayState(2, "hero_of_tatooine_missing_marks") or player:hasScreenPlayState(4, "hero_of_tatooine_missing_marks") or
        player:hasScreenPlayState(8, "hero_of_tatooine_missing_marks") or player:hasScreenPlayState(16, "hero_of_tatooine_missing_marks")
end

function HeroOfTatooineScreenPlay:giveMissingMarks(pPlayer)
    if not isPlayer(pPlayer) then
        return false
    end

    local pInventory = SceneObject(pPlayer):getSlottedObject("inventory")

    if not pInventory then
        return false
    end

    local player = LuaCreatureObject(pPlayer)

    if player:hasScreenPlayState(1, "hero_of_tatooine_missing_marks") then
        local pMark = giveItem(pInventory, self.markOfAltruism, -1)
        if pMark then
            player:removeScreenPlayState(1, "hero_of_tatooine_missing_marks") -- 1 - Altruism, 2 - Intellect, 4 - Courage, 8 - Honor, 16 - Ring
            player:sendSystemMessage(SceneObject(pMark):getDisplayedName() .. " has been placed in your inventory.")
        else
            return false
        end
    end

    if player:hasScreenPlayState(2, "hero_of_tatooine_missing_marks") then
        local pMark = giveItem(pInventory, self.markOfIntellect, -1)
        if pMark then
            player:removeScreenPlayState(2, "hero_of_tatooine_missing_marks") -- 1 - Altruism, 2 - Intellect, 4 - Courage, 8 - Honor, 16 - Ring
            player:sendSystemMessage(SceneObject(pMark):getDisplayedName() .. " has been placed in your inventory.")
        else
            return false
        end
    end

    if player:hasScreenPlayState(4, "hero_of_tatooine_missing_marks") then
        local pMark = giveItem(pInventory, self.markOfCourage, -1)
        if pMark then
            player:removeScreenPlayState(4, "hero_of_tatooine_missing_marks") -- 1 - Altruism, 2 - Intellect, 4 - Courage, 8 - Honor, 16 - Ring
            player:sendSystemMessage(SceneObject(pMark):getDisplayedName() .. " has been placed in your inventory.")
        else
            return false
        end
    end

    if player:hasScreenPlayState(8, "hero_of_tatooine_missing_marks") then
        local pMark = giveItem(pInventory, self.markOfHonor, -1)
        if pMark then
            player:removeScreenPlayState(8, "hero_of_tatooine_missing_marks") -- 1 - Altruism, 2 - Intellect, 4 - Courage, 8 - Honor, 16 - Ring
            player:sendSystemMessage(SceneObject(pMark):getDisplayedName() .. " has been placed in your inventory.")
        else
            return false
        end
    end

    if player:hasScreenPlayState(16, "hero_of_tatooine_missing_marks") then
        local pMark = giveItem(pInventory, self.markOfTheHeroRing, -1)
        if pMark then
            player:removeScreenPlayState(16, "hero_of_tatooine_missing_marks") -- 1 - Altruism, 2 - Intellect, 4 - Courage, 8 - Honor, 16 - Ring
            player:sendSystemMessage(SceneObject(pMark):getDisplayedName() .. " has been placed in your inventory.")
            TangibleObject(pMark):setOptionBit(INSURED)
        else
            return false
        end
    end

    return true
end
