PlayerTriggers = { }

function PlayerTriggers:playerLoggedIn(pPlayer)
	if (pPlayer == nil) then
		return
	end

	ServerEventAutomation:playerLoggedIn(pPlayer)
	BestineElection:playerLoggedIn(pPlayer)

	--Infinity Custom
	--util:cw("run custom player triggers", 1)
	playerTriggers_infinity:playerLoggedIn(pPlayer)

end

function PlayerTriggers:playerLoggedOut(pPlayer)
	if (pPlayer == nil) then
		return
	end
	-- Infinity Custom
	playerTriggers_infinity:playerLoggedOut(pPlayer)

	ServerEventAutomation:playerLoggedOut(pPlayer)
end
