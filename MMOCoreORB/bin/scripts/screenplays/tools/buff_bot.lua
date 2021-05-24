-- Thanks to SWG Awakening for the Buff Bot Code
buffBotScreenPlay = ScreenPlay:new {
	numberOfACts = 1,
	screenPlayName = "customName"
}
registerScreenPlay("buffBotScreenPlay", true)

function buffBotScreenPlay:start()
	if (isZoneEnabled("corellia")) then
		self:spawnMobiles()
	end
end

function buffBotScreenPlay:spawnMobiles()
	--Corellia
	--spawnMobile("corellia", "buff_bot", 0, -136, 28, -4730, 0, 0)

end

function buffBotScreenPlay:healWounds(pPlayer)
	for i = 0, 8 do
		CreatureObject(pPlayer):setWounds(i, 0)
	end
	CreatureObject(pPlayer):setShockWounds(0)
	CreatureObject(pPlayer):sendSystemMessage("Your wounds and battle fatigue have been healed.")
end

function buffBotScreenPlay:clearEnhancements(pPlayer)
	CreatureObject(pPlayer):resetBuffs()
	CreatureObject(pPlayer):sendSystemMessage("The effects of your previous enhancements have been cleared.")
end

function buffBotScreenPlay:clearDots(pPlayer)
	CreatureObject(pPlayer):clearDots()
	CreatureObject(pPlayer):sendSystemMessage("All DOT effects have been cleared.")
end

function buffBotScreenPlay:applyEnhancements(pPlayer)
	CreatureObject(pPlayer):gmenhanceCharacter(pPlayer)
end