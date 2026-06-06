local ObjectManager = require("managers.object.object_manager")

local mapStringName = { 
		"pirate1", 
		"pirate2", 
		"bountyhunter1", 
		"hedon1", 
		"pirate3", 
		"pirate4", 
		"pirate5", 
		"pirate6", 
		"pirate7", 
		"pirate8", 
		"pirate9",
		"pirate10",
		"pirate11",
		"pirate12",
		"pirate13",
		"pirate14",
		"pirate15",
		"pirate16",
		"pirate17",
		"pirate18",
		"pirate19",
	}

local treasureMapData = {
	{ planet = "corellia", x = 3400, y = -1400, bark = true }, -- Pirate 1
	{ planet = "naboo", x = -1900, y = -4000, bark = true }, -- Pirate 2
	{ planet = "talus", x = 3800, y = -1000, bark = true }, -- Bounty Hunter
	{ planet = "tatooine", x = 6700, y = 4240, bark = false }, -- Scythe Schematic
	{ planet = "dathomir", x = -2158, y = -4409, bark = true }, -- Pirate 3
	{ planet = "endor", x = -1265, y = -1234, bark = true }, -- Pirate 4
	{ planet = "yavin4", x = -221, y = -4248, bark = false }, -- Pirate 5
	{ planet = "tatooine", x = 1760, y = -1024, bark = true }, -- Pirate 6
	{ planet = "rori", x = -1854, y = 611, bark = true }, -- Pirate 7
	{ planet = "lok", x = -5132, y = 5113, bark = true }, -- Pirate 8
	{ planet = "dantooine", x = 3210, y = 2437, bark = true }, -- Pirate 9

	{ planet = "rori", x = 4900, y = 1600, bark = false }, -- Pirate 10
	{ planet = "naboo", x = 5345, y = 4433, bark = true }, -- Pirate 11
	{ planet = "tatooine", x = -2000, y = 4400, bark = false }, -- Pirate 12
	{ planet = "endor", x = 3800, y = 5400, bark = false }, -- Pirate 13
	{ planet = "talus", x = -5400, y = -2000, bark = false }, -- Pirate 14
	{ planet = "corellia", x = -1800, y = 5900, bark = true }, -- Pirate 15
	{ planet = "dathomir", x = 4700, y = 3250, bark = false }, -- Pirate 16
	{ planet = "yavin4", x = 4800, y = -3300, bark = false }, -- Pirate 17
	{ planet = "lok", x = -490, y = -5800, bark = false }, -- Pirate 18
	{ planet = "dantooine", x = -1300, y = -3000, bark = true }, -- Pirate 19
}

local treasureMapMobs = {
	-- moblist, loot level low, high, big game points
	{{"pirate_leader","pirate_armsman","pirate_armsman"},25,50, 1100, 10 * 60 * 1000},-- cor
	{{"pirate_leader","pirate_armsman","pirate_armsman"},25,50, 1100, 10 * 60 * 1000},-- nab
	{{"bounty_hunter_thug_tm"},35,60, 1300, 10 * 60 * 1000},-- talus
	{{"canyon_krayt_dragon"},25,50, 1300, 30 * 60 * 1000},-- legacy - NOT USED, PLACEHOLDER (5-field shape preserved)
	{{"nightsister_sentinel","nightsister_ranger","nightsister_ranger"},110,150,2500, 30 * 60 * 1000},-- dath
	{{"panshee_elder_tm","panshee_elder_tm","panshee_elder_tm","panshee_elder_tm"},64,92, 2000, 30 * 60 * 1000},-- endor
	{{"kliknik_shredder_guardian_tm","kliknik_shredder_guardian_tm","kliknik_shredder_guardian_tm"},85,135, 2350, 30 * 60 * 1000},-- yavin4
	{{"tusken_carnage_champion","tusken_death_hunter","tusken_death_hunter"},35,80, 1500, 30 * 60 * 1000},-- tat
	{{"pirate_leader","pirate_armsman","pirate_armsman"},40,80, 1650, 10 * 60 * 1000},-- rori
	{{"nym_patrol_elite_tm","nym_patrol_elite_tm","nym_droideka","nym_droideka"},78,104, 1850, 20 * 60 * 1000},-- lok
	{{"force_crystal_hunter","force_sensitive_crypt_crawler","force_sensitive_crypt_crawler"},85,115, 2100, 30 * 60 * 1000},-- dant

	{{"legendary_belsavis_varactyl_wild","legendary_belsavis_varactyl_wild"},150,200, 12500, 45 * 60 * 1000},-- rori varactyls
	{{"gungan_artifact_protector","gungan_artifact_protector","gungan_artifact_protector"},90,145, 2000, 30 * 60 * 1000},-- naboo gungans
	{{"krayt_king_crimzon"},250,300, 25000, 60 * 60 * 1000},-- tat king
	{{"gorax_tm","gorax_tm"},125,175, 9500, 30 * 60 * 1000},-- endor gorax
	{{"wolf_kima","wolf_kima","wolf_kima","wolf_kima","wolf_kima","wolf_kima","wolf_kima"},125,225, 4500, 20 * 60 * 1000},-- talus wolfies
	{{"nyax_extremist_tm","nyax_extremist_tm","disciple_of_lord_nyax","disciple_of_lord_nyax","disciple_of_lord_nyax"},119,159, 3250, 30 * 60 * 1000},-- corellia nyax
	{{"rancor_tm","ancient_bull_rancor","ancient_bull_rancor"},150,300, 7500, 60 * 60 * 1000},-- dathomir big ranc
	{{"enhanced_force_spider_tm","enhanced_force_spider_tm","enhanced_force_spider_tm","enhanced_force_spider_tm","enhanced_force_spider_tm"},225,275, 5750, 30 * 60 * 1000},-- yavin4 spiders
	{{"perlek_ravager_tm","perlek_ravager_tm","perlek_ravager_tm","perlek_ravager_tm"},135,165, 6870, 30 * 60 * 1000},-- lok perleks
	--{{"janta_bloodletter","janta_bloodletter","janta_bloodletter","janta_bloodletter","janta_bloodletter"},125,225, 3750},-- dantooine cavemen
	{{"janta_clan_leader","janta_clan_leader","janta_clan_leader","janta_clan_leader","janta_clan_leader"},125,225, 3750, 40 * 60 * 1000},-- dantooine cavemen

}

local TREASURE_CHEST_LIFESPAN = 10 * 60 * 1000 --10 minutes

TreasureMapMenuComponent = { }

function TreasureMapMenuComponent:fillObjectMenuResponse(pSceneObject, pMenuResponse, pPlayer)
	local menuResponse = LuaObjectMenuResponse(pMenuResponse)

	menuResponse:addRadialMenuItem(120, 3, "@treasure_map/treasure_map:use") -- Read
	menuResponse:addRadialMenuItem(121, 3, "@treasure_map/treasure_map:search_area") -- Search Area
	menuResponse:addRadialMenuItem(122, 3, "@treasure_map/treasure_map:extract_treasure") -- Extract Treasure
end

function TreasureMapMenuComponent:handleObjectMenuSelect(pObject, pPlayer, selectedID)
	if (pPlayer == nil or pObject == nil) then
		return 0
	end

	if (SceneObject(pObject):isASubChildOf(pPlayer) == false) then
		CreatureObject(pPlayer):sendSystemMessage("@treasure_map/treasure_map:sys_not_in_inv") -- The treasure map must be in your inventory for you to use it!
		return 0
	end

	if (TreasureMapMenuComponent:getMapType(pObject) == 0) then
		CreatureObject(pPlayer):sendSystemMessage("@treasure_map/treasure_map:map_fail") -- This map is obviously a fake.
		return 0
	end

	if (selectedID == 120) then
		TreasureMapMenuComponent:doReadMap(pObject, pPlayer)
	elseif (selectedID == 121) then
		if util:validateCheck(pPlayer, 13) then
			util:cout(pPlayer, "You have some unfinished business to tend to, and if you kill me with a pet, I will just keep coming for you!!!", "pink")
			TreasureMapMenuComponent:attackPlayer(pPlayer)
		else
			TreasureMapMenuComponent:doSearchArea(pObject, pPlayer)
		end
	elseif (selectedID == 122) then
		TreasureMapMenuComponent:doExtractTreasure(pObject, pPlayer)
	end
	return 0
end

function TreasureMapMenuComponent:doSearchArea(pObject, pPlayer)
	if (pPlayer == nil or pObject == nil) then
		return 0
	end

	local pGhost = CreatureObject(pPlayer):getPlayerObject()

	if (pGhost == nil) then
		return 0
	end

	local mapType = TreasureMapMenuComponent:getMapType(pObject)

	local mapData = treasureMapData[mapType]
	local playerID = CreatureObject(pPlayer):getObjectID()
	local waypointID = readData(playerID .. ":treasureMapSearchAreaWaypointID")
	local searchAreaID = readData(playerID .. ":treasureMapSearchAreaActiveAreaID")

	if (waypointID == 0 or searchAreaID == 0) then
		CreatureObject(pPlayer):sendSystemMessage("@treasure_map/treasure_map:sys_no_waypoint") -- You must store the treasure's waypoint in your datapad before you can search for it!
		return 0
	end

	if (TangibleObject(pPlayer):hasActiveArea(searchAreaID) == false) then
		CreatureObject(pPlayer):sendSystemMessage("@treasure_map/treasure_map:sys_cant_pinpoint") -- You are not close enough to pinpoint the treasure's location.
		return 0
	end

	local pWaypoint = getSceneObject(waypointID)

	if (pWaypoint == nil) then
		CreatureObject(pPlayer):sendSystemMessage("@treasure_map/treasure_map:sys_no_waypoint") -- You must store the treasure's waypoint in your datapad before you can search for it!
		return 0
	end

	local pActiveArea = getSceneObject(searchAreaID)
	SceneObject(pActiveArea):destroyObjectFromWorld()
	local zoneName = SceneObject(pPlayer):getZoneName()

	local spawnPoint

	if (mapType == 4) then
		spawnPoint = getSpawnPoint(zoneName, SceneObject(pWaypoint):getWorldPositionX(), SceneObject(pWaypoint):getWorldPositionY(), 15, 30, true)
	else
		spawnPoint = getSpawnPoint(zoneName, SceneObject(pWaypoint):getWorldPositionX(), SceneObject(pWaypoint):getWorldPositionY(), 30, 60)
	end
	if spawnPoint ~= nil then
	CreatureObject(pPlayer):sendSystemMessage("@treasure_map/treasure_map:sys_pinpoint") -- You have successfully pinpointed the exact location of the treasure!
	local waypointID = PlayerObject(pGhost):addWaypoint(mapData.planet, "@treasure_map/treasure_map:waypoint_name", "", spawnPoint[1], 0, spawnPoint[3], WAYPOINT_GREEN, true, true, WAYPOINTTREASUREMAP, 0)
	writeData(playerID .. ":treasureMapExactWaypointID", waypointID)
	deleteData(playerID .. ":treasureMapSearchAreaWaypointID")
	deleteData(playerID .. ":treasureMapSearchAreaActiveAreaID")
	else
		util:cout(pPlayer, "Failed to locate the location of the treasure, move around and try again", "yellow")
	end
end

function TreasureMapMenuComponent:doExtractTreasure(pObject, pPlayer)
	if (pObject == nil or pPlayer == nil) then
		return 0
	end

	local mapType = TreasureMapMenuComponent:getMapType(pObject)
	local playerID = SceneObject(pPlayer):getObjectID()

	local mapData = treasureMapData[mapType]
	local waypointID = readData(playerID .. ":treasureMapExactWaypointID")

	local pWaypoint = getSceneObject(waypointID)

	if (pWaypoint == nil) then
		CreatureObject(pPlayer):sendSystemMessage("@treasure_map/treasure_map:sys_no_waypoint") -- You must store the treasure's waypoint in your datapad before you can search for it!
		return 0
	end

	local x = SceneObject(pWaypoint):getWorldPositionX()
	local y = SceneObject(pWaypoint):getWorldPositionY()
	local px = SceneObject(pPlayer):getWorldPositionX()
	local py = SceneObject(pPlayer):getWorldPositionY()
	--local z = getTerrainHeight(pPlayer, x, y) -- this breaks if there is a lair nearby or some terrain height oddity
	local z = SceneObject(pPlayer):getWorldPositionZ()
	local dist = util:distanceObjects(pWaypoint,pPlayer)
	if (dist > 9) then
		CreatureObject(pPlayer):sendSystemMessage("@treasure_map/treasure_map:sys_dist_far") -- You aren't close enough to extract the treasure.
		return 0
	end

	local treasureChestID = readData(playerID .. ":treasureChestID")

	if (treasureChestID ~= nil) then
		local pExistingChest = getSceneObject(treasureChestID)
		if (pExistingChest ~= nil) then
			self:removeTreasureChest(pExistingChest)
		end
	end

	CreatureObject(pPlayer):sendSystemMessage("@treasure_map/treasure_map:sys_found") -- You successfully extract the treasure!

	local pChest = spawnSceneObject(mapData.planet, "object/tangible/container/drum/treasure_drum.iff", x, z, y, 0, 0, 0, 0, 0)

	if (pChest ~= nil) then
		local chestID = SceneObject(pChest):getObjectID()
		writeData(playerID .. ":treasureChestID", chestID)
		writeData(chestID .. ":ownerID", playerID)
		SceneObject(pChest):setContainerOwnerID(playerID)
		createObserver(OPENCONTAINER, "TreasureMapMenuComponent", "openChestEvent", pChest)
		createObserver(CONTAINERCONTENTSCHANGED, "TreasureMapMenuComponent", "chestLootedEvent", pChest)
		TreasureMapMenuComponent:spawnTreasureLoot(pChest, pPlayer, mapType)
		createEvent(TREASURE_CHEST_LIFESPAN, "TreasureMapMenuComponent", "removeTreasureChest", pChest, "")
	end

	TreasureMapMenuComponent:spawnTreasureDefenders(pObject, pPlayer, x, z, y, mapType)
	SceneObject(pObject):destroyObjectFromWorld()
	SceneObject(pObject):destroyObjectFromDatabase(true)
end

function TreasureMapMenuComponent:chestLootedEvent(pChest, pCreature)
	if (pChest == nil) then
		return 0
	end

	if (SceneObject(pChest):getContainerObjectsSize() == 0) then
		TreasureMapMenuComponent:removeTreasureChest(pChest)
	end

	return 0
end

function TreasureMapMenuComponent:openChestEvent(pChest, pCreature)
	if pCreature == nil or pChest == nil or not SceneObject(pCreature):isCreatureObject() then
		return 0
	end

	local chestOwnerID = readData(SceneObject(pChest):getObjectID() .. ":ownerID")
	local playerID = CreatureObject(pCreature):getObjectID()

	if (chestOwnerID ~= playerID) then
		CreatureObject(pCreature):sendSystemMessage("@treasure_map/treasure_map:sys_not_yours") -- That treasure chest doesn't belong to you.
		return 0
	end

	local hasOpenedChest = readData(playerID .. ":hasOpenedChest")

	if (hasOpenedChest ~= 1) then
		local credits = getRandomNumber(55000, 175000)
		CreatureObject(pCreature):addCashCredits(credits, true)
		CreatureObject(pCreature):sendSystemMessage("You find " .. credits .. " credits in the chest.")
		writeData(playerID .. ":hasOpenedChest", 1)
	end

	return 0
end

function TreasureMapMenuComponent:spawnTreasureLoot(pChest, pPlayer, mapType)
	if (pPlayer == nil or pChest == nil) then
		return
	end
	if mapType == 4 then
		createLoot(pChest, "hedon_istee_treasure", 10, false)
	else
		local playerLevelRange = getRandomNumber(CreatureObject(pPlayer):getLevel() + treasureMapMobs[mapType][2], CreatureObject(pPlayer):getLevel() + treasureMapMobs[mapType][3])
		util:cw("Chest loot level" .. playerLevelRange, 1)
		for i = 1, 5 do
			createLoot(pChest, "treasure_map_group", playerLevelRange, false)
		end
	end
	--[[if (mapType == 1 or mapType == 2 or mapType == 3) then
		local playerLevelRange = getRandomNumber(CreatureObject(pPlayer):getLevel() - 5, CreatureObject(pPlayer):getLevel() + 5)

		for i = 1, 5 do
			createLoot(pChest, "treasure_map_group", playerLevelRange, false)
		end
	else
		createLoot(pChest, "hedon_istee_treasure", 10, false)
	end]]
end

function TreasureMapMenuComponent:spawnTreasureDefenders(pObject, pPlayer, x, z, y, mapType)
	if (pObject == nil or pPlayer == nil) then
		return
	end

	local zoneName = SceneObject(pPlayer):getZoneName()
	local mapType = TreasureMapMenuComponent:getMapType(pObject)
	local mapData = treasureMapData[mapType]
	local firstSpawnPoint, secondSpawnPoint, thirdSpawnPoint
	--[[if (mapType ~= 4) then
		firstSpawnPoint = getSpawnPoint(zoneName, x, y, 10, 20)
	end]]
	big_game_hunter:contentXP(pPlayer, treasureMapMobs[mapType][4])

	if mapType == 4 then
		spawnMobile(mapData.planet, "canyon_krayt_dragon", 0, x, z, y, 0, 0)
	else
		for i=1,#treasureMapMobs[mapType][1] do
			local sp = getSpawnPoint(zoneName, x, y, 10, 20)
			if sp ~= nil then
				local pMobile = spawnMobile(mapData.planet, treasureMapMobs[mapType][1][i], 0, sp[1], sp[2], sp[3], 0, 0)
				if pMobile ~= nil then
					CreatureObject(pMobile):engageCombat(pPlayer)
					local despawnTimer = treasureMapMobs[mapType][5]
					if util:isDev() then
						despawnTimer = 30 * 1000
					end
					createEvent(despawnTimer, "TreasureMapMenuComponent", "despawnAttackers", pPlayer, tostring(SceneObject(pMobile):getObjectID()))
					if i == 1 and treasureMapData[mapType].bark then
						spatialChat(pMobile, "@treasure_map/treasure_map:bark_" .. mapStringName[mapType])
					end
				end
			end
		end
	end
	
end
function TreasureMapMenuComponent:despawnAttackers(pPlayer, pMobileID)

	local pMobile = getSceneObject(tonumber(pMobileID))

	if pMobile ~= nil and SceneObject(pMobile):isAiAgent() and not SceneObject(pMobile):isPet() then

		if pPlayer ~= nil then

			if CreatureObject(pMobile):isDead() or CreatureObject(pMobile):isIncapacitated() then
				util:cw("player killed it", 1)
			else
				util:cw("player cheated", 1)
				-- set a quest state to attack them on next search area
				-- has a 3 hour expiration
				util:setCheck(pPlayer, 13, 3 * 60 * 60)
			end
		end

		AiAgent(pMobile):doDespawn()

	end

end
function TreasureMapMenuComponent:attackPlayer(pPlayer)
	if pPlayer ~= nil then
		local planet = SceneObject(pPlayer):getZoneName()
		local cellid = SceneObject(pPlayer):getParentID()
		local x = SceneObject(pPlayer):getWorldPositionX()
		local z = SceneObject(pPlayer):getWorldPositionZ()
		local y = SceneObject(pPlayer):getWorldPositionY()
		local pa = SceneObject(pPlayer):getDirectionAngle()
        local pa1 = math.rad(pa)
		local attackType = 1
		local atObject = pPlayer
		if cellid ~= nil and cellid > 0 then
			local cellobj = getSceneObject(cellid)
			if cellobj ~= nil then
				local objType = SceneObject(cellobj):getGameObjectType()
				if objType ~= nil then
					if objType == 11 then -- building
						attackType = -1
					elseif objType == 1024 then -- pet mount						
						attackType = 2
					elseif objType == 65537 then -- vehicle mount
						attackType = 3
					end
				end  
			end  
		end
		if cellid ~= nil and attackType == 1 then
			local sp = "bounty_hunter_thug_tm"
			local pGhost = CreatureObject(pPlayer):getPlayerObject()
			if pGhost ~= nil then
				if PlayerObject(pGhost):isJedi() then
					sp = "bounty_hunter_thug_revenge"
				end
			end
			local pMobile = spawnMobile(planet, sp, 0, x + 2, z, y, 0, 0)
			if pMobile ~= nil then
				createObserver(OBJECTDESTRUCTION, "TreasureMapMenuComponent", "bhKilled", pMobile)
				CreatureObject(pMobile):engageCombat(pPlayer)
				createEvent(20 * 60 * 1000, "util", "despawnMobile", pMobile, "TreasureMapMenuComponent_394")
			end
		else
			createEvent(getRandomNumber(900) * 1000, "TreasureMapMenuComponent", "attackPlayer", pPlayer, "")
		end
	end
end
function TreasureMapMenuComponent:bhKilled(pBoss, pPlayer)
	util:cw("bh killed", 1)
    if (pPlayer == nil) then
		return 0
	end
    if (pBoss == nil) then
		return 0
	end
	util:removeCheck(pPlayer, 13)
	--util:removeQuestState(pPlayer, "treasuremap:cheat", 1)
	return 1
end
function TreasureMapMenuComponent:setDefenderAggro(pCreature, pPlayer)
	if (pCreature == nil) then
		return
	end

	AiAgent(pCreature):setDefender(pPlayer)
end

function TreasureMapMenuComponent:removeTreasureChest(pChest)
	if (pChest == nil) then
		return
	end

	local chestID = SceneObject(pChest):getObjectID()
	local chestOwnerID = readData(chestID .. ":ownerID")
	local pOwner = getSceneObject(chestOwnerID)

	if (pOwner ~= nil) then
		local pGhost = CreatureObject(pOwner):getPlayerObject()

		if (pGhost ~= nil) then
			PlayerObject(pGhost):removeWaypointBySpecialType(WAYPOINTTREASUREMAP)
		end
	end

	deleteData(chestOwnerID .. ":treasureChestID")
	deleteData(chestID .. ":ownerID")
	SceneObject(pChest):destroyObjectFromWorld()

	deleteData(chestOwnerID .. ":hasOpenedChest")
	deleteData(chestOwnerID .. ":treasureMapExactWaypointID")
end

function TreasureMapMenuComponent:doReadMap(pObject, pPlayer)
	if (pObject == nil or pPlayer == nil) then
		return
	end

	local mapType = TreasureMapMenuComponent:getMapType(pObject)
	local suiManager = LuaSuiManager()
	suiManager:sendMessageBox(pObject, pPlayer, "@treasure_map/treasure_map:title_" .. mapStringName[mapType], "@treasure_map/treasure_map:text_" .. mapStringName[mapType], "@treasure_map/treasure_map:store_waypoint", "TreasureMapMenuComponent", "handleTreasureMapSuiCallback")
end

function TreasureMapMenuComponent:handleTreasureMapSuiCallback(pCreature, pSui, eventIndex)
	local cancelPressed = (eventIndex == 1)

	if (cancelPressed or pCreature == nil) then
		return 0
	end

	local pGhost = CreatureObject(pCreature):getPlayerObject()

	if (pGhost == nil) then
		return 0
	end

	local suiBox = LuaSuiBox(pSui)
	local pUsingObject = suiBox:getUsingObject()

	if (pUsingObject == nil) then
		return 0
	end

	local mapType = TreasureMapMenuComponent:getMapType(pUsingObject)

	local mapData = treasureMapData[mapType]
	if (CreatureObject(pCreature):getZoneName() ~= mapData.planet) then
		CreatureObject(pCreature):sendSystemMessage("@treasure_map/treasure_map:wrong_planet") -- The coordinates stored in the map data do not appear to be for this planet.
		return 0
	end
	local playerID = CreatureObject(pCreature):getObjectID()

	local currentWaypointID = readData(playerID .. ":treasureMapSearchAreaWaypointID")
	local exactWaypointID = readData(playerID .. ":treasureMapExactWaypointID")
	local pExactWaypoint = getSceneObject(exactWaypointID)
	local pWaypoint = getSceneObject(currentWaypointID)

	if (pWaypoint ~= nil or pExactWaypoint ~= nil) then
		CreatureObject(pCreature):sendSystemMessage("@treasure_map/treasure_map:sys_waypoint_exists") -- A waypoint to this location already exists in your datapad.
		return 0
	end

	local spawnPoint
	local zoneName = SceneObject(pCreature):getZoneName()

	if (mapType == 4) then
		spawnPoint = getSpawnPoint(zoneName, mapData.x, mapData.y, 1, 50, true)
	else
		spawnPoint = getSpawnPoint(zoneName, mapData.x, mapData.y, 1, 2000)
	end

	if spawnPoint == nil then return 0 end

	local waypointID = PlayerObject(pGhost):addWaypoint(mapData.planet, "@treasure_map/treasure_map:waypoint_name", "", spawnPoint[1], 0, spawnPoint[3], WAYPOINT_GREEN, true, true, WAYPOINTTREASUREMAP, 0)
	local activeAreaID = self:spawnSearchArea(mapType, pCreature, spawnPoint[1], spawnPoint[3])
	writeData(playerID .. ":treasureMapSearchAreaWaypointID", waypointID)
	writeData(playerID .. ":treasureMapSearchAreaActiveAreaID", activeAreaID)
end

function TreasureMapMenuComponent:spawnSearchArea(mapType, pCreature, x, y)
	if (pCreature == nil) then
		return 0
	end

	local mapData = treasureMapData[mapType]
	local z = getTerrainHeight(pCreature, x, y)
	local pActiveArea = spawnActiveArea(mapData.planet, "object/active_area.iff", x, z, y, 64, 0)

	if pActiveArea ~= nil then
		return SceneObject(pActiveArea):getObjectID()
	else
		return 0
	end
end

function TreasureMapMenuComponent:getMapType(pObject)
	if (pObject == nil) then
		return 0
	end

	local objectTemplate = SceneObject(pObject):getTemplateObjectPath()

	if (objectTemplate == "object/tangible/treasure_map/treasure_map_pirate1.iff") then
		return 1
	elseif (objectTemplate == "object/tangible/treasure_map/treasure_map_pirate2.iff") then
		return 2
	elseif (objectTemplate == "object/tangible/treasure_map/treasure_map_bh.iff") then
		return 3
	elseif (objectTemplate == "object/tangible/loot/quest/treasure_map_hedon.iff") then
		return 4
	elseif (objectTemplate == "object/tangible/treasure_map/treasure_map_pirate3.iff") then
		return 5
	elseif (objectTemplate == "object/tangible/treasure_map/treasure_map_pirate4.iff") then
		return 6
	elseif (objectTemplate == "object/tangible/treasure_map/treasure_map_pirate5.iff") then
		return 7
	elseif (objectTemplate == "object/tangible/treasure_map/treasure_map_pirate6.iff") then
		return 8
	elseif (objectTemplate == "object/tangible/treasure_map/treasure_map_pirate7.iff") then
		return 9
	elseif (objectTemplate == "object/tangible/treasure_map/treasure_map_pirate8.iff") then
		return 10
	elseif (objectTemplate == "object/tangible/treasure_map/treasure_map_pirate9.iff") then
		return 11
	elseif (objectTemplate == "object/tangible/treasure_map/treasure_map_pirate10.iff") then
		return 12
	elseif (objectTemplate == "object/tangible/treasure_map/treasure_map_pirate11.iff") then
		return 13
	elseif (objectTemplate == "object/tangible/treasure_map/treasure_map_pirate12.iff") then
		return 14
	elseif (objectTemplate == "object/tangible/treasure_map/treasure_map_pirate13.iff") then
		return 15
	elseif (objectTemplate == "object/tangible/treasure_map/treasure_map_pirate14.iff") then
		return 16
	elseif (objectTemplate == "object/tangible/treasure_map/treasure_map_pirate15.iff") then
		return 17
	elseif (objectTemplate == "object/tangible/treasure_map/treasure_map_pirate16.iff") then
		return 18
	elseif (objectTemplate == "object/tangible/treasure_map/treasure_map_pirate17.iff") then
		return 19
	elseif (objectTemplate == "object/tangible/treasure_map/treasure_map_pirate18.iff") then
		return 20
	elseif (objectTemplate == "object/tangible/treasure_map/treasure_map_pirate19.iff") then
		return 21
	else
		return 0
	end
end
