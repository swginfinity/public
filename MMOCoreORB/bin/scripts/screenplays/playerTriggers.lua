PlayerTriggers = { }

function PlayerTriggers:playerLoggedIn(pPlayer)
	if (pPlayer == nil) then
		return
	end

	local pGhost = CreatureObject(pPlayer):getPlayerObject()
	local playerID = CreatureObject(pPlayer):getObjectID()
	local pGhost2 = CreatureObject(pPlayer)
	if (pGhost == nil) then
		return nil
	end
	
	-- if a player fully logs out in the tutorial, they are stuck in space, this sends them to HG
	if (PlayerObject(pGhost):hasGodMode() or PlayerObject(pGhost):isPrivileged()) then
		-- do nothing for admins
	else
		local planet = SceneObject(pPlayer):getZoneName()
		if planet ~= nil then
			if planet == "tutorial" then
				local pBuilding = SceneObject(pPlayer):getParent()
				if pBuilding == nil then
					-- missing: hud fix
					CreatureObject(pPlayer):sendNewbieTutorialEnableHudElement("all", 1, 0)
					-- teleport to shuttle in HG near new player term
					SceneObject(pPlayer):switchZone("naboo",-5027,8,2872,0)
					includeFile("../managers/player_creation_manager.lua")
					if pGhost2 ~= nil then
						local startProf = pGhost2:getScreenPlayState("tutorialprofession")
						if startProf ~= nil then
							CreatureObject(pPlayer):sendSystemMessage("your starter prof: " .. startProf)
							if startProf == 1 then
								-- combat_brawler_novice
								awardSkill(pPlayer, "combat_brawler_novice")
								self:giveItem(pPlayer, brawlerOneHander)
								self:giveItem(pPlayer, brawlerTwoHander)
								self:giveItem(pPlayer, brawlerPolearm)
								self:giveItem(pPlayer, brawlerVK)
							elseif startProf == 2 then
								-- combat_marksman_novice
								awardSkill(pPlayer, "combat_marksman_novice")
								self:giveItem(pPlayer, marksmanPistol)
								self:giveItem(pPlayer, marksmanRifle)
								self:giveItem(pPlayer, marksmanCarbine)
							elseif startProf == 4 then
								-- outdoors_scout_novice
								awardSkill(pPlayer, "outdoors_scout_novice")
							elseif startProf == 8 then
								-- science_medic_novice
								awardSkill(pPlayer, "science_medic_novice")
								self:giveItem(pPlayer, foodTool)
							elseif startProf == 16 then
								-- social_entertainer_novice
								awardSkill(pPlayer, "social_entertainer_novice")
								self:giveItem(pPlayer, slitherhorn)
							elseif startProf == 32 then
								-- crafting_artisan_novice
								awardSkill(pPlayer, "crafting_artisan_novice")
								self:giveItem(pPlayer, foodTool)
								self:giveItem(pPlayer, mineralTool)
								self:giveItem(pPlayer, chemicalTool)
							end
							-- backpack
							if backPack ~= nil then
								self:giveItem(pPlayer, backPack)
							end
							-- survival knife
							self:giveItem(pPlayer, survivalKnife)

							-- give everyone generic tool
							self:giveItem(pPlayer, genericTool)

							-- give bike
							self:giveItem(pPlayer, SwoopBike)

							-- give melon
							self:giveItem(pPlayer, marojMelon)

							-- give armor
							self:giveItem(pPlayer, paddedbelt)
							self:giveItem(pPlayer, paddedbicepl)
							self:giveItem(pPlayer, paddedbicepr)
							self:giveItem(pPlayer, paddedboots)
							self:giveItem(pPlayer, paddedbracerl)
							self:giveItem(pPlayer, paddedbracerr)
							self:giveItem(pPlayer, paddedchest)
							self:giveItem(pPlayer, paddedhelmet)
							self:giveItem(pPlayer, paddedgloves)
							self:giveItem(pPlayer, paddedleggings)
						else
							CreatureObject(pPlayer):sendSystemMessage("failed to store starter prof")
						end
					else
						print("pghost2 was null...hmmm")
					end
				end
			end
			
			
		end
	end
	-- take away varactyls if they did not do the quest
	if pGhost2 ~= nil then
		local state = pGhost2:hasScreenPlayState(64, "varactylquest")
		if state ~= nil then
			if not state then
				self:checkVaractyls(pPlayer)
			end
		end
	end
	-- One-time per player check for legacy FRS skills	
	local skillCheckCompleted = readScreenPlayData(pPlayer,"login_skill_check", "Completed")
	
	if (skillCheckCompleted ~= "true") then

		if (PlayerObject(pGhost):hasAbility("cert_onehandlightsaber_gen4")) and not (CreatureObject(pPlayer):hasSkill("force_discipline_light_saber_master")) then
			PlayerObject(pGhost):removeAbility("cert_onehandlightsaber_gen4")
			PlayerObject(pGhost):removeAbility("cert_polearmlightsaber_gen4")
			PlayerObject(pGhost):removeAbility("cert_twohandlightsaber_gen4")
		end

		if (PlayerObject(pGhost):hasAbility("healAllSelf2")) and not (CreatureObject(pPlayer):hasSkill("force_discipline_healing_damage_04")) then
			PlayerObject(pGhost):removeAbility("healAllSelf2")
		end

		if (PlayerObject(pGhost):hasAbility("avoidIncapacitation")) and not (CreatureObject(pPlayer):hasSkill("force_discipline_defender_master")) then
			PlayerObject(pGhost):removeAbility("avoidIncapacitation")
		end

		if (PlayerObject(pGhost):hasAbility("regainConsciousness")) and not (CreatureObject(pPlayer):hasSkill("force_discipline_enhancements_master")) then
			PlayerObject(pGhost):removeAbility("regainConsciousness")
		end

		local skillManager = LuaSkillManager()

		local result = skillManager:awardForceFromSkills(pPlayer)

		writeScreenPlayData(pPlayer, "login_skill_check", "Completed", "true") 

	end

	--[[ Quarterly check for FRS reset, reserved for potential future use
	local FRSResetCompleted = readScreenPlayData(pPlayer,"FRSReset20200401", "Completed")

	if (FRSResetCompleted ~= "true") then

		-- Reset FRS xp to zero regardless of Jedi state / skills
		local xpAmount = PlayerObject(pGhost):getExperience("force_rank_xp")
		CreatureObject(pPlayer):awardExperience("force_rank_xp", xpAmount * -1, false)

		if ((CreatureObject(pPlayer):hasSkill("force_rank_light_novice")) or (CreatureObject(pPlayer):hasSkill("force_rank_dark_novice"))) then  

			local rankResult = PlayerObject(pGhost):setFrsRank(pGhost,0)

		end

		writeScreenPlayData(pPlayer, "FRSReset20200401", "Completed", "true") -- Save screenplay state for current quarter

	end
	--]]


	BestineElection:playerLoggedIn(pPlayer)

end
function PlayerTriggers:checkVaractyls(pPlayer)
	-- remove any varactyls they have
	local playerID = SceneObject(pPlayer):getObjectID()
	local objects = {
		{ name = "Azure",	template = "object/intangible/pet/azure_belsavis_varactyl_hue.iff"},
		{ name = "Gilded",	template = "object/intangible/pet/gilded_belsavis_varactyl_hue.iff"},
		{ name = "Ruby",	template = "object/intangible/pet/ruby_belsavis_varactyl_hue.iff"},
	}
	for i=1,#objects do
		local run = 1
		local runaway = 1
		while run == 1 do
			local pInventory = SceneObject(pPlayer):getSlottedObject("datapad")
			if pInventory ~= nil then
				local pItem = getContainerObjectByTemplate(pInventory, objects[i].template, false)
				if pItem ~= nil then
					printLuaError("PlayerTriggers:remove illegal Varactyl: " .. objects[i].name .. ", player ID: " .. playerID .. " name: " ..  CreatureObject(pPlayer):getFirstName())
					SceneObject(pItem):destroyObjectFromWorld()
					SceneObject(pItem):destroyObjectFromDatabase()
				else
					run = 0
				end
			end
			runaway = runaway + 1
			if runaway > 50 then
				run = 0
			end
		end
	end
end

-- give item
function PlayerTriggers:giveItem(pPlayer, itemTemplate)
	local playerID = SceneObject(pPlayer):getObjectID()
	local pInventory = SceneObject(pPlayer):getSlottedObject("inventory")
	local pCheck = getContainerObjectByTemplate(pInventory, itemTemplate, true)
	if (pCheck == nil) then
		local pItem = giveItem(pInventory, itemTemplate, -1)
		if (pItem ~= nil) then
			SceneObject(pItem):sendTo(pPlayer)
		end
	end
end
function PlayerTriggers:playerLoggedOut(pPlayer)
	if (pPlayer == nil) then
		return
	end
end