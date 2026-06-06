NabooMordranScreenPlay = ScreenPlay:new {
	numberOfActs = 1,

	screenplayName = "NabooMordranScreenPlay",

}

registerScreenPlay("NabooMordranScreenPlay", true)

function NabooMordranScreenPlay:start()
	if (isZoneEnabled("naboo")) then
		self:spawnMobiles()

	end
end

function NabooMordranScreenPlay:spawnMobiles()

	--mordran respawns between 15-20 minutes
	--local pNpc = spawnMobile("naboo", "mordran", 1200, -1969, 6.4, -5295, 45, 0)
	self:mordranSpawn()
	spawnMobile("naboo", "outcast_gungan", 300, -1967, 6.4, -5296, 45, 0)
	spawnMobile("naboo", "outcast_gungan", 300, -1964, 6.4, -5292, 45, 0)
	spawnMobile("naboo", "outcast_gungan", 300, -1966, 6.4, -5301, 45, 0)
	spawnMobile("naboo", "gungan_reject", 360, -1969, 6.4, -5291, 45, 0)
	spawnMobile("naboo", "gungan_reject", 360, -1969, 6.4, -5297, 45, 0)
	spawnMobile("naboo", "exiled_gungan", 360, -1966, 6.4, -5295, 45, 0)
	spawnMobile("naboo", "exiled_gungan", 360, -1963, 6.4, -5299, 45, 0)
end

function NabooMordranScreenPlay:mordranSpawn()
	--local pNpc = spawnMobile("naboo", "mordran", math.random(21600, 23400), -1969, 6.4, -5295, 45, 0)
	local pNpc = spawnMobile("naboo", "mordran", 0, -1969, 6.4, -5295, 45, 0)
	if pNpc ~= nil then
		createObserver(OBJECTDESTRUCTION, "NabooMordranScreenPlay", "mordranKilled", pNpc)
	end
end
function NabooMordranScreenPlay:mordranKilled(pBoss, pPlayer)
	if pPlayer ~= nil then
		local killCredit = false
		local pKiller = petOwner(pPlayer)
		if pKiller ~= nil then
			if isPet(pPlayer) then			
				if util:hasProfession(pKiller, "outdoors_creaturehandler_novice") then
					killCredit = true
				end
			else
				killCredit = true
			end
			if killCredit and util:hasProfession(pKiller, "big_game_hunter_novice") then
				if not util:hasQuestState(pKiller, "bg_quest:co4", 1) then
					util:setQuestState(pKiller, "bg_quest:co4", 1)
				end
			end
		end
	end
	local t1 = 45 * 60
	local t2 = 75 * 60

	createEvent(getRandomNumber(t1, t2) * 1000, "NabooMordranScreenPlay", "mordranSpawn", nil, "")
	return 1
end
