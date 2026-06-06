local ObjectManager = require("managers.object.object_manager")

function printf(...) io.write(string.format(table.unpack({...}))) end

function writeData(key, data)
	writeSharedMemory(string.format(key), data)
end

function writeStringData(key, data)
	writeStringSharedMemory(string.format(key), string.format(data))
end

function deleteData(key)
	deleteSharedMemory(string.format(key))
end

function deleteStringData(key)
	deleteStringSharedMemory(string.format(key))
end

function readData(key)
	return readSharedMemory(string.format(key))
end

function readStringData(key)
	return readStringSharedMemory(string.format(key))
end

function writeVector3Data(key, x, z, y)
	writeVector3SharedMemory(string.format(key), x, z, y)
end

function readVector3Data(key)
	return readVector3SharedMemory(string.format(key))
end

function deleteVector3Data(key)
	deleteVector3SharedMemory(string.format(key))
end



ScreenPlay = Object:new {
	screenplayName = "",
	numberOfActs = 0,
	startingEvent = nil,

	lootContainers = {},
	lootLevel = 0,
	lootGroups = {},
	lootContainerRespawn = 1800 -- 30 minutes
}

-- Custom content - Loads last to allow for overrides
includeFile("../custom_scripts/screenplays/screenplay.lua")

function ScreenPlay:initializeLootContainers()
	for k,v in pairs(self.lootContainers) do
		local pContainer = getSceneObject(v)

        if pContainer and SceneObject(pContainer):isContainerObject() then
            if self.screenplayName ~= "" then
                createObserver(OPENCONTAINER, self.screenplayName, "spawnContainerLoot", pContainer)
            else
                Logger:log("self.screenplayName is empty in ScreenPlay:initializeLootContainers.  Container OID: " .. SceneObject(pContainer):getObjectID() .. ".  Caller info follows:" .. getLuaCallStack(), LT_ERROR)
            end

            self:spawnContainerLoot(pContainer)
            SceneObject(pContainer):setContainerDefaultAllowPermission(MOVEOUT + OPEN)
            SceneObject(pContainer):setContainerComponent("PlaceableLootContainerComponent")
        else
            Logger:log("pContainer is nil or invalid.  Container ID (from screenplay): " .. v .. ".  Caller info follows:" .. getLuaCallStack(), LT_ERROR) 
        end
	end
end

function ScreenPlay:spawnContainerLoot(pContainer)

    if type(pContainer) ~= "userdata" or not SceneObject(pContainer):isContainerObject() then
        Logger:log("pContainer is nil or invalid.  Caller info follows:" .. getLuaCallStack(), LT_ERROR) 
        return 0
    end

    local containerOID = SceneObject(pContainer):getObjectID()
    local nextRespawnTime = readData(containerOID)
    local currentTime = getTimestamp()

    if currentTime < nextRespawnTime then
        return 0
    end

    writeData(containerOID, currentTime + self.lootContainerRespawn)

    --If it has loot already, then exit.
    if SceneObject(pContainer):getContainerObjectsSize() > 0 then
        return 0
    end

    createLootFromCollection(pContainer, self.lootGroups, self.lootLevel)

    return 0
end

function ScreenPlay:setMoodString(pNpc, mood)
	if type(pNpc) == "userdata" and SceneObject(pNpc):isCreatureObject() and type(mood) == "string" then
        CreatureObject(pNpc):setMoodString(mood)
	end
end

function ScreenPlay:setCustomName(pObj, name)
	if type(pObj) == "userdata" and type(name) == "string" then
        SceneObject(pObj):setCustomObjectName(name)
	end
end

Act = Object:new {

}

return ScreenPlay
