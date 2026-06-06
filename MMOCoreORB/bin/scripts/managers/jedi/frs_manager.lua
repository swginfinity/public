frsEnabled = 1

-- Object ID of Enclave buildings
lightEnclaveID = 8525417
darkEnclaveID = 3435626

petitionInterval = 86400000 -- 1 day ** Not used
votingInterval = 86400000 -- 1 day ** Not used
acceptanceInterval = 86400000 -- 1 day ** Not used
-- Infinity:  Weekly maintenance
maintenanceInterval = 604800000 -- 7 days

requestDemotionDuration = 604800000 -- 7 days ** Not used
voteChallengeDuration = 604800000 -- 7 days ** Not used

arenaOpenInterval = 5400000 -- 90 mins ** Not used
arenaClosedInterval = 108000000 -- 30 hours ** Not used
arenaChallengeDuration = 3600000 -- 60 mins ** Not used
arenaChallengeCooldown = 90000000 -- 25 hours ** Not used

-- Costs are in FRS experience
-- Infinity:  1000 base per week, 11000 at rank 11
baseMaintCost = 1000

-- Not used on Infinity
requestDemotionCost = 0
voteChallengeCost = 0
maxPetitioners = 0
maxChallenges = 0
missedVotePenalty = 0

-- Restrict players from voting for and challenging other characters on their account
sameAccountEnclaveRestrictions = 1 -- ** Not used

-- Infinity:  Custom XP Values and no limits per rank
-- { rank, skillName, requiredExperience, playerCap, robe }
lightRankingData = {
	{ 0, "force_rank_light_novice", 0, -1, "object/tangible/wearables/robe/robe_jedi_light_s01.iff" },
	{ 1, "force_rank_light_rank_01", 10000, -1, "object/tangible/wearables/robe/robe_jedi_light_s02.iff" },
	{ 2, "force_rank_light_rank_02", 20000, -1, "object/tangible/wearables/robe/robe_jedi_light_s02.iff" },
	{ 3, "force_rank_light_rank_03", 30000, -1, "object/tangible/wearables/robe/robe_jedi_light_s02.iff" },
	{ 4, "force_rank_light_rank_04", 65000, -1, "object/tangible/wearables/robe/robe_jedi_light_s02.iff" },
	{ 5, "force_rank_light_rank_05", 150000, -1, "object/tangible/wearables/robe/robe_jedi_light_s03.iff"},
	{ 6, "force_rank_light_rank_06", 225000, -1, "object/tangible/wearables/robe/robe_jedi_light_s03.iff" },
	{ 7, "force_rank_light_rank_07", 325000, -1, "object/tangible/wearables/robe/robe_jedi_light_s03.iff" },
	{ 8, "force_rank_light_rank_08", 450000, -1, "object/tangible/wearables/robe/robe_jedi_light_s04.iff" },
	{ 9, "force_rank_light_rank_09", 600000, -1, "object/tangible/wearables/robe/robe_jedi_light_s04.iff" },
	{ 10, "force_rank_light_rank_10", 775000, -1, "object/tangible/wearables/robe/robe_jedi_light_s05.iff" },
	{ 11, "force_rank_light_master", 1000000, -1, "object/tangible/wearables/robe/robe_jedi_light_s05.iff" },
}

darkRankingData = {
	{ 0, "force_rank_dark_novice", 0, -1, "object/tangible/wearables/robe/robe_jedi_dark_s01.iff" },
	{ 1, "force_rank_dark_rank_01", 10000, -1, "object/tangible/wearables/robe/robe_jedi_dark_s02.iff" },
	{ 2, "force_rank_dark_rank_02", 20000, -1, "object/tangible/wearables/robe/robe_jedi_dark_s02.iff" },
	{ 3, "force_rank_dark_rank_03", 30000, -1, "object/tangible/wearables/robe/robe_jedi_dark_s02.iff" },
	{ 4, "force_rank_dark_rank_04", 65000, -1, "object/tangible/wearables/robe/robe_jedi_dark_s02.iff" },
	{ 5, "force_rank_dark_rank_05", 150000, -1, "object/tangible/wearables/robe/robe_jedi_dark_s03.iff" },
	{ 6, "force_rank_dark_rank_06", 225000, -1, "object/tangible/wearables/robe/robe_jedi_dark_s03.iff" },
	{ 7, "force_rank_dark_rank_07", 325000, -1, "object/tangible/wearables/robe/robe_jedi_dark_s03.iff" },
	{ 8, "force_rank_dark_rank_08", 450000, -1, "object/tangible/wearables/robe/robe_jedi_dark_s04.iff" },
	{ 9, "force_rank_dark_rank_09", 600000, -1, "object/tangible/wearables/robe/robe_jedi_dark_s04.iff" },
	{ 10, "force_rank_dark_rank_10", 775000, -1, "object/tangible/wearables/robe/robe_jedi_dark_s05.iff" },
	{ 11, "force_rank_dark_master", 1000000, -1, "object/tangible/wearables/robe/robe_jedi_dark_s05.iff" },
}

enclaveRoomRequirements = {
	-- Light enclave
	{ 8525444, -1 }, -- entrancehall1
	{ 8525421, 1 }, -- tier1hall1
	{ 8525420, 1 }, -- tier1room
	{ 8525419, 1 }, -- tier1hall2
	{ 8525424, 5 }, -- tier2hall1
	{ 8525423, 5 }, -- tier2room
	{ 8525422, 5 }, -- tier2hall2
	{ 8525433, 8 }, -- tier3hall1
	{ 8525432, 8 }, -- tier3room
	{ 8525431, 8 }, -- tier3hall2
	{ 8525430, 10 }, -- tier4hall1
	{ 8525429, 10 }, -- tier4room
	{ 8525428, 10 }, -- tier4hall2
	{ 8525427, 11 }, -- tier5hall1
	{ 8525426, 11 }, -- tier5room
	{ 8525425, 11 }, -- tier5hall2
	-- Dark enclave
	{ 3435626, -1 }, -- ramp1
	{ 3435644, 1 }, -- hallwayb1
	{ 3435650, 1 }, -- council1
	{ 3435637, 5 }, -- hallwaya1
	{ 3435652, 5 }, -- council3
	{ 3435638, 8 }, -- hallwaya2
	{ 3435653, 8 }, -- council4
	{ 3435645, 8 }, -- hallwayb2
	{ 3435639, 10 }, -- hallwaya3
	{ 3435646, 10 }, -- hallwayb3
	{ 3435651, 10 }, -- council2
	{ 3435640, 11 }, -- hallwaya4
	{ 3435647, 11 }, -- hallwayb4
	{ 3435641, 11 }, -- chamberramp
	{ 3435642, 11 }, -- chamber
}

-- Key followed by values for player rank 0 through 11
-- Key references the player's target
-- Ex: Rank 5 loses to BH, see "bh_lose" key and 6th integer value in same row
-- Infinity:  Values not used in our system
frsExperienceValues = {
}
