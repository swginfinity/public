--Big thanks to Tarkins Revenge for their Star Tours Code as a base

includeFile("tools/infinity_travel_table.lua")

infinity_travel = ScreenPlay:new {
	numberOfActs = 1,
	travelFee = 0, -- This is the cost of travel using the Infinity Special Event Travel Service

	droidModel = "object/tangible/travel/ticket_collector/infinity_travel.iff",
	droids = {
		--Corellia
		--{planetName = "corellia", x = -159.377, z = 28.0279, y = -4942.14, ow = 0.998257, oy = 0.0588108, cellID = 0}, --Coronet starport
		--{planetName = "corellia", x = 3372.79, z = 308, y = 5623.05, ow = 0.290803, oy = -0.956783, cellID = 0}, --Doaba Guerfel starport
		--{planetName = "corellia", x = -3147.2, z = 31, y = 2907.89, ow = -0.0561584, oy = 0.998422, cellID = 0}, --Kor Vella starport
		--{planetName = "corellia", x = -4974.41, z = 21, y = -2211.71, ow = 0.346712, oy = -0.937972, cellID = 0},  --Tyrena starport

		--Naboo
		--{planetName = "naboo", x = -13.5802, z = 7.9918, y = 14.2038, ow = 0.0270844, oy = 0.999633, cellID = 1692104}, --Theed starport
		--{planetName = "naboo", x = 5310.75, z = -192, y = 6674.53, ow = -0.687696, oy = 0.725999, cellID = 0}, --Kaadara starport
		--{planetName = "naboo", x = 1339.42, z = 13, y = 2755.31, ow = 0.725999, oy = 0.687696, cellID = 0}, --Keren starport
		--{planetName = "naboo", x = 4708.84, z = 4.17, y = -4652, ow = 0.475684, oy = 0.879617, cellID = 0}, --Moenia starport

		--Rori
		--{planetName = "rori", x = -5390.72, z = 80, y = -2160.53, ow = 0.5136, oy = 0.85803, cellID = 0}, --Narmle starport
		--{planetName = "rori", x = 5351.81, z = 80, y = 5765.4, ow = 0.0270844, oy = 0.999633, cellID = 0}, --Restuss starport

		--Talus
		--{planetName = "talus", x = 231.606, z = 6, y = -2943.15, ow = 0.718174, oy = 0.695864, cellID = 0}, --Dearic starport
		--{planetName = "talus", x = 4472.12, z = 2, y = 5381.11, ow = -0.146912, oy = 0.98915, cellID = 0}, --Nashal starport

		--Tatooine
		--{planetName = "tatooine", x = 1042.07, z = 7.02263, y = 2991.87, ow = 0.654793, oy = 0.75581, cellID = 0}, --Mos Entha
		--{planetName = "tatooine", x = -2811.65, z = 5, y = 2082.73, ow = -0.852166, oy = 0.523273, cellID = 0}, --Mos Espa
		--{planetName = "tatooine", x = -1390.49, z = 12, y = -3586.34, ow = 0.725999, oy = 0.687696, cellID = 0}, --Bestine
		--{planetName = "tatooine", x = 3632.02, z = 5, y = -4788.63, ow = 0.695864, oy = -0.718174, cellID = 0}, --Mos Eisley
	}
}

registerScreenPlay("infinity_travel", true)

function infinity_travel:start()
	-- Spawn droids
	for i = 1, #self.droids, 1 do
		local pDroid = spawnSceneObject(self.droids[i].planetName, self.droidModel , self.droids[i].x, self.droids[i].z, self.droids[i].y, self.droids[i].cellID, self.droids[i].ow, 0, self.droids[i].oy, 0)
		if (pDroid ~= nil) then
			-- Add menu
			SceneObject(pDroid):setObjectMenuComponent("InfinityTravelMenuComponent")
		end	
	end
end

function infinity_travel:openWindow(pPlayer)
	if (pPlayer == nil) then
		return
	end

	self:showCategories(pPlayer)
end

function infinity_travel:showCategories(pPlayer)

	if CreatureObject(pPlayer):isInCombat() then
		CreatureObject(pPlayer):sendSystemMessage("Travel services are not available while you are in combat.")
		return 0
	end

	local sui = SuiListBox.new("infinity_travel", "showSubCategories") -- calls showSubCategories on SUI window event

	sui.setTargetNetworkId(SceneObject(pPlayer):getObjectID())

	sui.setTitle("Infinity Special Event Travel")

	local promptText = "Using this droid will allow you to travel to various destinations instantly."
	sui.setPrompt(promptText)

	for i = 1, #InfinityTravelTable, 1 do
		sui.add(InfinityTravelTable[i].catName, "")
	end

	sui.sendTo(pPlayer)
end

function infinity_travel:showSubCategories(pPlayer, pSui, eventIndex, args)
	local cancelPressed = (eventIndex == 1)

	if (cancelPressed) then
		return
	end


	if (args == "-1") then
		CreatureObject(pPlayer):sendSystemMessage("No category was selected...")
		return
	end

	local selectedIndex = tonumber(args) + 1
	writeScreenPlayData(pPlayer, "infinity_travel", "categorySelected", selectedIndex) 

	local sui = SuiListBox.new("infinity_travel", "showDestinations") -- calls showDestinations on SUI window event

	sui.setTargetNetworkId(SceneObject(pPlayer):getObjectID())

	sui.setTitle("Infinity Special Event Travel")

	local promptText = "Using this droid will allow you to travel to various interesting destinations instantly."
	sui.setPrompt(promptText)

	for i = 1, #InfinityTravelTable[selectedIndex].subCategory, 2 do
		sui.add(InfinityTravelTable[selectedIndex].subCategory[i], InfinityTravelTable[selectedIndex].subCategory[i+1])
	end

	sui.sendTo(pPlayer)
end

function infinity_travel:showDestinations(pPlayer, pSui, eventIndex, args)
	local cancelPressed = (eventIndex == 1)

	if (cancelPressed) then
		return
	end


	if (args == "-1") then
		CreatureObject(pPlayer):sendSystemMessage("No category was selected...")
		return
	end

	local selectedSubCatIndex = tonumber(args) + 1

	local catIndex = tonumber(readScreenPlayData(pPlayer, "infinity_travel", "categorySelected"))

	local sui = SuiListBox.new("infinity_travel", "destinationSelection") -- calls destinationSelection on SUI window event

	sui.setTargetNetworkId(SceneObject(pPlayer):getObjectID())

	sui.setTitle("Infinity Special Event Travel")
	sui.setPrompt(InfinityTravelTable[catIndex].catName .. "\n\n Press Cancel to return to the main menu.")

	local n = selectedSubCatIndex
	repeat 
		if (InfinityTravelDestinationTable[n].subCatName ~= InfinityTravelTable[catIndex].subCategory[selectedSubCatIndex*2]) then
		n = n +1;
		end
	until InfinityTravelDestinationTable[n].subCatName == InfinityTravelTable[catIndex].subCategory[selectedSubCatIndex*2]
	
	selectedSubCatIndex = n
	writeScreenPlayData(pPlayer, "infinity_travel", "subCategorySelected", selectedSubCatIndex) 

	for i = 1, #InfinityTravelDestinationTable[selectedSubCatIndex].destinations, 6 do
		sui.add(InfinityTravelDestinationTable[selectedSubCatIndex].destinations[i], InfinityTravelDestinationTable[selectedSubCatIndex].destinations[i+1])
	end

	sui.sendTo(pPlayer)
end

function infinity_travel:destinationSelection(pPlayer, pSui, eventIndex, args)

	local cancelPressed = (eventIndex == 1)

	if (cancelPressed) then
		return self:showCategories(pPlayer)
	end

	if (args == "-1") then
		CreatureObject(pPlayer):sendSystemMessage("No destination was selected...")
		return
	end

	local selectedDestinationIndex = tonumber(args) + 1
	local subCatIndex = tonumber(readScreenPlayData(pPlayer, "infinity_travel", "subCategorySelected"))

	local price = self.travelFee
	--First, determine whether player has enough money for the trip

	local cashCredits = CreatureObject(pPlayer):getCashCredits()
	local bankCredits = CreatureObject(pPlayer):getBankCredits()
	local playerCredits = cashCredits + bankCredits

	if playerCredits < price then
		CreatureObject(pPlayer):sendSystemMessage("Insufficient Funds: You require at least " .. tostring(price) .. " total credits in your bank or cash to travel to your destination.")
			return
	end

	--Use cash first
	if (price <= cashCredits) then
			CreatureObject(pPlayer):subtractCashCredits(price)

	--Pay remainder from bank
	else
		price = price - cashCredits
		CreatureObject(pPlayer):subtractCashCredits(cashCredits)
		CreatureObject(pPlayer):setBankCredits(bankCredits - price)
	end
	
	--Second, travel to selected destination
		SceneObject(pPlayer):switchZone(InfinityTravelDestinationTable[subCatIndex].destinations[((selectedDestinationIndex*6)-4)], InfinityTravelDestinationTable[subCatIndex].destinations[((selectedDestinationIndex*6)-3)], InfinityTravelDestinationTable[subCatIndex].destinations[((selectedDestinationIndex*6)-2)], InfinityTravelDestinationTable[subCatIndex].destinations[((selectedDestinationIndex*6)-1)], InfinityTravelDestinationTable[subCatIndex].destinations[(selectedDestinationIndex*6)])
	
	-- Third, send system message to player to announce that they have paid

	if (price > 0) then
		createEvent(2*1000, "infinity_travel", "announcePayment", pPlayer, "")
	end

end

function infinity_travel:announcePayment(pPlayer)
		CreatureObject(pPlayer):sendSystemMessage("You have paid the " .. tostring(self.travelFee) .. " credit travel fee.")

end

-- Radial Menu Functions
InfinityTravelMenuComponent = { }

function InfinityTravelMenuComponent:fillObjectMenuResponse(pSceneObject, pMenuResponse, pPlayer)
	local menuResponse = LuaObjectMenuResponse(pMenuResponse)
	menuResponse:addRadialMenuItem(20, 3, "Travel")
	menuResponse:addRadialMenuItem(120, 3, "About Infinity Special Event Travel")
end

function InfinityTravelMenuComponent:handleObjectMenuSelect(pObject, pPlayer, selectedID)
	if (pPlayer == nil or pObject == nil) then
		return 0
	end
	
	if CreatureObject(pPlayer):isInCombat() then
		CreatureObject(pPlayer):sendSystemMessage("Travel services are not available while you are in combat.")
		return 0
	end
	
	if (selectedID == 20) then
		infinity_travel:openWindow(pPlayer, pObject)
	elseif (selectedID == 120) then
		local suiManager = LuaSuiManager()
		local helpMessage = "This droid will provide travel to many varied destinations around the galaxy.  Just select your destination from the menu and off you go!"

		suiManager:sendMessageBox(pObject, pPlayer, "About Infinity Special Event Travel", helpMessage, "@ok", "InfinityTravelMenuComponent", "noCallback")
	end 
	
	return 0
end

function InfinityTravelMenuComponent:noCallback(pPlayer, pSui, eventIndex)
	-- do nothing
end
