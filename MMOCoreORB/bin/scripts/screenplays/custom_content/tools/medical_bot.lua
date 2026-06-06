-- Thanks to SWG Awakening for the Buff Bot Code
medicalBotScreenPlay = ScreenPlay:new {
	numberOfACts = 1,
	screenPlayName = "customName"
}
registerScreenPlay("medicalBotScreenPlay", true)

function medicalBotScreenPlay:start()
	if (isZoneEnabled("corellia")) then
		self:spawnMobiles()
	end
end

function medicalBotScreenPlay:spawnMobiles()
	--Corellia
	--spawnMobile("corellia", "buff_bot", 0, -136, 28, -4730, 0, 0)

end

function medicalBotScreenPlay:healWounds(pPlayer)
	for i = 0, 8 do
		CreatureObject(pPlayer):setWounds(i, 0)
	end
	CreatureObject(pPlayer):setShockWounds(0)
	CreatureObject(pPlayer):sendSystemMessage("Your wounds and battle fatigue have been healed.")
end


function medicalBotScreenPlay:clearDots(pPlayer)
	CreatureObject(pPlayer):clearDots()
	CreatureObject(pPlayer):sendSystemMessage("All DOT effects have been cleared.")
end
