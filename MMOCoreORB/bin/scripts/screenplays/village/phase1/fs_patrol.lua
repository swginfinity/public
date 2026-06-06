local ObjectManager = require("managers.object.object_manager")
local QuestManager = require("managers.quest.quest_manager")
require("utils.helpers")

FsPatrol = Patrol:new {
	-- Task properties
	taskName = "FsPatrol",
	-- Patrol properties
	waypointName = "@quest/force_sensitive/fs_patrol:patrol_point",
	numPoints = 8,
	areaSize = 48,
	originX = 5313,
	originY = -4161,
	forceSpawn = true,
	enemyList = { "sith_shadow_mercenary", "sith_shadow_thug", "sith_shadow_pirate", "sith_shadow_outlaw" }
}

function FsPatrol:spawnEnemies(pPlayer, numEnemies, x, y)
	local playerID = SceneObject(pPlayer):getObjectID()
	writeData(playerID .. self.taskName .. "numEnemies", numEnemies)
	writeData(playerID .. self.taskName .. "totalEnemies", numEnemies)

	for i = 1, numEnemies, 1 do
		local npcType

		if (numEnemies < 3) then
			npcType = getRandomNumber(1,2)
		else
			npcType = getRandomNumber(3,4)
		end

		local offsetX = getRandomNumber(-5, 5)
		local offsetY = getRandomNumber(-5, 5)

		local spawnX = x + offsetX
		local spawnY = y + offsetY
		local spawnZ = getTerrainHeight(pPlayer, spawnX, spawnY)

		local pMobile = spawnMobile(SceneObject(pPlayer):getZoneName(), self.enemyList[npcType], 0, spawnX, spawnZ, spawnY, 0, 0)

		if (pMobile ~= nil) then
			local mobileID = SceneObject(pMobile):getObjectID()
			writeData(playerID .. self.taskName .. "enemyNum" .. i, mobileID)
			createEvent(600 * 1000, self.taskName, "destroyMobile", pMobile, "")
			writeData(mobileID .. self.taskName .. "ownerID", playerID)
			if (numEnemies <= 3) then
				createObserver(OBJECTDESTRUCTION, self.taskName, "notifyKilledGoodTarget", pMobile)
			else
				createObserver(OBJECTDESTRUCTION, self.taskName, "notifyKilledBadTarget", pMobile)
			end
		end
	end
end

function FsPatrol:clearLastPoint(pPlayer)
	local playerID = SceneObject(pPlayer):getObjectID()
	local numEnemies = readData(playerID .. self.taskName .. "totalEnemies")

	for i = 1, numEnemies, 1 do
		local mobileID = readData(playerID .. self.taskName .. "enemyNum" .. i)
		local pMobile = getSceneObject(mobileID)

		if (pMobile ~= nil) then
			if (CreatureObject(pMobile):isInCombat()) then
				createEvent(30 * 1000, self.taskName, "destroyMobile", pMobile, "")
			else
				if SceneObject(pMobile):isAiAgent() then
					AiAgent(pMobile):doDespawn()
				else
					SceneObject(pMobile):destroyObjectFromWorld()
				end
				deleteData(mobileID .. self.taskName .. "ownerID")
			end

			deleteData(playerID .. self.taskName .. "enemyNum" .. i)
		end
	end
end

function FsPatrol:destroyMobile(pMobile)
	if (pMobile == nil) then
		return
	end

	if (CreatureObject(pMobile):isInCombat()) then
		createEvent(60 * 1000, self.taskName, "destroyMobile", pMobile, "")
		return
	end

	deleteData(SceneObject(pMobile):getObjectID() .. self.taskName .. "ownerID")

	if SceneObject(pMobile):isAiAgent() then
		AiAgent(pMobile):doDespawn()
	else
		SceneObject(pMobile):destroyObjectFromWorld()
	end

end

function FsPatrol:notifyKilledGoodTarget(pVictim, pAttacker)
	local ownerID = readData(SceneObject(pVictim):getObjectID() .. self.taskName .. "ownerID")

	local numEnemies = readData(ownerID .. self.taskName .. "numEnemies")

	numEnemies = numEnemies - 1

	if (numEnemies <= 0) then
		writeData(ownerID .. ":completedCurrentPoint", 1)
	end

	deleteData(SceneObject(pVictim):getObjectID() .. self.taskName .. "ownerID")
	writeData(ownerID .. self.taskName .. "numEnemies", numEnemies)
	return 1
end

function FsPatrol:notifyKilledBadTarget(pVictim, pAttacker)
	local ownerID = readData(SceneObject(pVictim):getObjectID() .. self.taskName .. "ownerID")
	if (SceneObject(pAttacker):getObjectID() == ownerID) then
		CreatureObject(pAttacker):sendSystemMessage("@quest/force_sensitive/fs_patrol:failed_killed_dont_kill_npc")
		self:failPatrol(pAttacker)
	end

	deleteData(SceneObject(pVictim):getObjectID() .. self.taskName .. "ownerID")
	return 1
end

function FsPatrol:onPlayerKilled(pPlayer)
	local completedCount = tonumber(QuestManager.getStoredVillageValue(pPlayer, "FsPatrolCompletedCount"))
	local playerID = SceneObject(pPlayer):getObjectID()

	if (completedCount >= 0 and completedCount < 20 and readData(playerID .. ":failedPatrol") ~= 1) then
		CreatureObject(pPlayer):sendSystemMessage("@quest/force_sensitive/fs_patrol:failed_death")
		self:failPatrol(pPlayer)
	end
end

function FsPatrol:onEnteredActiveArea(pPlayer, pActiveArea)
	if (pPlayer == nil) then
		return 0
	end

	local playerID = SceneObject(pPlayer):getObjectID()
	self:clearLastPoint(pPlayer)

	if (readData(playerID .. ":completedCurrentPoint") == -1) then
		CreatureObject(pPlayer):sendSystemMessage("@quest/force_sensitive/fs_patrol:failed_previous_point")
		self:failPatrol(pPlayer)
		deleteData(playerID .. ":completedCurrentPoint")
		return 1
	end

	local spawnChance = getRandomNumber(1,100)

	if (spawnChance >= 70) then
		writeData(playerID .. ":completedCurrentPoint", -1)
		local numEnemies = getRandomNumber(1,3)
		CreatureObject(pPlayer):sendSystemMessage("@quest/force_sensitive/fs_patrol:small_group" .. numEnemies)
		self:spawnEnemies(pPlayer, numEnemies, SceneObject(pActiveArea):getWorldPositionX(), SceneObject(pActiveArea):getWorldPositionY())
	elseif (spawnChance >= 60 and spawnChance < 70) then
		writeData(playerID .. ":completedCurrentPoint", 1)
		local numEnemies = getRandomNumber(5,8)
		CreatureObject(pPlayer):sendSystemMessage("@quest/force_sensitive/fs_patrol:large_group" .. numEnemies)
		self:spawnEnemies(pPlayer, numEnemies, SceneObject(pActiveArea):getWorldPositionX(), SceneObject(pActiveArea):getWorldPositionY())
	else
		writeData(playerID .. ":completedCurrentPoint", 1)
		CreatureObject(pPlayer):sendSystemMessage("@quest/force_sensitive/fs_patrol:no_objective")
	end

	return 1
end

function FsPatrol:resetFsPatrol(pPlayer)
	local playerID = SceneObject(pPlayer):getObjectID()
	deleteData(playerID .. ":patrolWaypointsReached")
	deleteData(playerID .. ":failedPatrol")
	deleteData(playerID .. ":completedCurrentPoint")

	local numEnemies = readData(playerID .. self.taskName .. "totalEnemies")

	for i = 1, numEnemies, 1 do
		local mobileID = readData(playerID .. self.taskName .. "enemyNum" .. i)
		local pMobile = getSceneObject(mobileID)

		if (pMobile ~= nil) then
			if SceneObject(pMobile):isAiAgent() then
				AiAgent(pMobile):doDespawn()
			else
				SceneObject(pMobile):destroyObjectFromWorld()
			end
		end

		deleteData(playerID .. self.taskName .. "enemyNum" .. i)
	end

	deleteData(playerID .. self.taskName .. "numEnemies")
	deleteData(playerID .. self.taskName .. "totalEnemies")
	self:waypointCleanup(pPlayer)
	self:setupPatrolPoints(pPlayer)
end

function FsPatrol:completeFsPatrol(pPlayer)
	local playerID = SceneObject(pPlayer):getObjectID()
	deleteData(playerID .. ":completedCurrentPoint")
	deleteData(playerID .. ":failedPatrol")
	deleteData(playerID .. ":patrolWaypointsReached")

	local numEnemies = readData(playerID .. self.taskName .. "totalEnemies")

	for i = 1, numEnemies, 1 do
		local mobileID = readData(playerID .. self.taskName .. "enemyNum" .. i)
		local pMobile = getSceneObject(mobileID)

		if (pMobile ~= nil) then
			if SceneObject(pMobile):isAiAgent() then
				AiAgent(pMobile):doDespawn()
			else
				SceneObject(pMobile):destroyObjectFromWorld()
			end
		end

		deleteData(playerID .. self.taskName .. "enemyNum" .. i)
	end

	deleteData(playerID .. self.taskName .. "numEnemies")
	deleteData(playerID .. self.taskName .. "totalEnemies")
	self:finish(pPlayer)
end

function FsPatrol:doPhaseChangeFail(pPlayer)
	if (not self:hasTaskStarted(pPlayer)) then
		return
	end

	CreatureObject(pPlayer):sendSystemMessage("@fs_quest_village:combat_quest_failed_timeout");
	self:finish(pPlayer)
end

-- GM tool: drive the player to the exact legit Patrol finish-state (the completedCount == 20
-- path in villageSarguilloPhase1ConvoHandler:completeCurrentPatrol). Each step is guarded so a
-- re-run is a safe no-op. Mirrors the legit finish; does NOT complete FS_PATROL_QUEST_START
-- (legit play leaves _START lingering in the journal).
function FsPatrol:forceComplete(pPlayer)
	if (pPlayer == nil) then
		return
	end

	-- Despawn any active patrol mobs + clear waypoints/data via the legit teardown primitive.
	if (self:hasTaskStarted(pPlayer)) then
		self:completeFsPatrol(pPlayer)
	end

	if (not QuestManager.hasCompletedQuest(pPlayer, QuestManager.quests.FS_PATROL_QUEST_10)) then
		QuestManager.completeQuest(pPlayer, QuestManager.quests.FS_PATROL_QUEST_10)
	end

	-- count == 10 milestone branch — must fire even when GM-completing from below 10.
	if (not VillageJediManagerCommon.hasUnlockedBranch(pPlayer, "force_sensitive_combat_prowess_ranged_accuracy")) then
		VillageJediManagerCommon.unlockBranch(pPlayer, "force_sensitive_combat_prowess_ranged_accuracy")
	end

	if (not QuestManager.hasCompletedQuest(pPlayer, QuestManager.quests.FS_PATROL_QUEST_20)) then
		QuestManager.completeQuest(pPlayer, QuestManager.quests.FS_PATROL_QUEST_20)
	end

	if (not QuestManager.hasCompletedQuest(pPlayer, QuestManager.quests.FS_PATROL_QUEST_FINISH)) then
		QuestManager.completeQuest(pPlayer, QuestManager.quests.FS_PATROL_QUEST_FINISH)
	end

	QuestManager.setStoredVillageValue(pPlayer, "FsPatrolCompletedCount", 20)
	QuestManager.setCurrentVillageQuestID(pPlayer, 0)

	local pInventory = SceneObject(pPlayer):getSlottedObject("inventory")

	if (pInventory ~= nil and getContainerObjectByTemplate(pInventory, "object/tangible/item/quest/force_sensitive/fs_village_bannerpole_s01.iff", true) == nil) then
		local pPole = giveItem(pInventory, "object/tangible/item/quest/force_sensitive/fs_village_bannerpole_s01.iff", -1, true)

		if (pPole == nil) then
			CreatureObject(pPlayer):sendSystemMessage("Error: Unable to generate item.")
		end
	end

	VillageJediManagerCommon.setCompletedQuestThisPhase(pPlayer)
end

-- GM/player tool: DROP the quest entirely. Tear down any in-progress patrol, zero progress,
-- reset every patrol sub-quest, and clear the active-quest-this-phase flag so the player has no
-- quest chosen for the phase (free to pick fresh). NOT restart-in-place: does NOT re-activate, and
-- FS branches are NOT re-locked (earned branches persist).
function FsPatrol:forceDrop(pPlayer)
	if (pPlayer == nil) then
		return
	end

	-- Tear down any in-progress patrol. completeFsPatrol despawns mobs + finish()es the task
	-- (clearing waypoints/active-areas, the started flag, and the observer). Guard on hasTaskStarted
	-- so finish() is never called on an inactive task.
	local playerID = SceneObject(pPlayer):getObjectID()

	if (self:hasTaskStarted(pPlayer)) then
		self:completeFsPatrol(pPlayer)
	else
		deleteData(playerID .. ":completedCurrentPoint")
		deleteData(playerID .. ":failedPatrol")
		deleteData(playerID .. ":patrolWaypointsReached")
	end

	QuestManager.setStoredVillageValue(pPlayer, "FsPatrolCompletedCount", 0)

	-- FS_PATROL_QUEST_1 (81) .. FS_PATROL_QUEST_20 (100) are contiguous and cover every
	-- iteration plus the _10/_20 milestones.
	for i = QuestManager.quests.FS_PATROL_QUEST_1, QuestManager.quests.FS_PATROL_QUEST_20, 1 do
		QuestManager.resetQuest(pPlayer, i)
	end

	QuestManager.resetQuest(pPlayer, QuestManager.quests.FS_PATROL_QUEST_FINISH)
	QuestManager.resetQuest(pPlayer, QuestManager.quests.FS_PATROL_QUEST_START)
	QuestManager.setCurrentVillageQuestID(pPlayer, 0)

	VillageJediManagerCommon.clearActiveQuestThisPhase(pPlayer)
end

return FsPatrol
