LokBattlefield = ScreenPlay:new {
	screenplayName = "LokBattlefield",
	planet = "lok",
	numberOfActs = 1
}

registerScreenPlay("LokBattlefield", true)

function LokBattlefield:start()
	if (isZoneEnabled(self.planet)) then
		self:spawnSceneObjects()
	end
end
function LokBattlefield:spawnSceneObjects()
	--This is the load in point to the battlefield, Need to set the point of landing for the shuttleport. Walls adjusted to 128Mx96M.
	spawnSceneObject(self.planet, "object/building/tatooine/shuttleport_tatooine.iff",  671.518, 11.5, 2161.06, 0, math.rad(180) )
	spawnSceneObject(self.planet, "object/building/tatooine/junkshop_watto.iff", 667.567, 12, 2095.38, 0, math.rad(180) )
	--Cloner for Battlefield.
	spawnSceneObject(self.planet, "object/building/tatooine/cloning_facility_tatooine.iff", 613.387, 12, 2183.62, 0, math.rad(90))
	--Arena Walls and Columns.
	spawnSceneObject(self.planet, "object/static/structure/tatooine/bridge_tatooine_small_style_03.iff", 759, 2, 2146.425, 0, math.rad(93) )
	spawnSceneObject(self.planet, "object/static/structure/tatooine/bridge_tatooine_small_style_03.iff", 790, 2, 2144.80, 0, math.rad(93) )
	
	spawnSceneObject(self.planet, "object/static/structure/corellia/corl_imprv_wall_4x32_s02.iff", 934.136, 11, 2153.14, 0, math.rad(180))
	--spawnSceneObject(self.planet, "object/static/structure/corellia/corl_imprv_wall_4x32_s02.iff", 902.348, 11, 2153.14, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/static/structure/corellia/corl_imprv_wall_4x32_s02.iff", 870.348, 11, 2153.14, 0, math.rad(180))
	
	spawnSceneObject(self.planet, "object/static/structure/corellia/corl_imprv_wall_4x32_s02.iff", 934.348, 11, 2025.14, 0, math.rad(180))
	--spawnSceneObject(self.planet, "object/static/structure/corellia/corl_imprv_wall_4x32_s02.iff", 902.348, 11, 2025.14, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/static/structure/corellia/corl_imprv_wall_4x32_s02.iff", 870.348, 11, 2025.14, 0, math.rad(180))

	spawnSceneObject(self.planet, "object/static/structure/corellia/corl_imprv_column_s01.iff", 950.877-32, 11, 2153.14, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/static/structure/corellia/corl_imprv_column_s01.iff", 950.877-64, 11, 2153.14, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/static/structure/corellia/corl_imprv_column_s01.iff", 950.877-32, 11, 2025.14, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/static/structure/corellia/corl_imprv_column_s01.iff", 950.877-64, 11, 2025.14, 0, math.rad(180))

	spawnSceneObject(self.planet, "object/static/structure/corellia/corl_imprv_column_s01.iff", 950.877, 11, 2025.14, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/static/structure/corellia/corl_imprv_column_s01.iff", 950.877, 11, 2153.14, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/static/structure/corellia/corl_imprv_column_s01.iff", 853.877, 11, 2025.14, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/static/structure/corellia/corl_imprv_column_s01.iff", 853.877, 11, 2153.14, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/static/structure/corellia/corl_imprv_wall_4x32_s02.iff", 853.877, 11, 2137.14, 0, math.rad(270))
	spawnSceneObject(self.planet, "object/static/structure/corellia/corl_imprv_wall_4x32_s02.iff", 853.877, 11, 2105.14, 0, math.rad(270))
	spawnSceneObject(self.planet, "object/static/structure/corellia/corl_imprv_wall_4x32_s02.iff", 853.877, 11, 2073.14, 0, math.rad(270))
	spawnSceneObject(self.planet, "object/static/structure/corellia/corl_imprv_wall_4x32_s02.iff", 853.877, 11, 2041.14, 0, math.rad(270))
	spawnSceneObject(self.planet, "object/static/structure/corellia/corl_imprv_wall_4x32_s02.iff", 950.877, 11, 2137.14, 0, math.rad(270))
	spawnSceneObject(self.planet, "object/static/structure/corellia/corl_imprv_wall_4x32_s02.iff", 950.877, 11, 2105.14, 0, math.rad(270))
	spawnSceneObject(self.planet, "object/static/structure/corellia/corl_imprv_wall_4x32_s02.iff", 950.877, 11, 2073.14, 0, math.rad(270))
	spawnSceneObject(self.planet, "object/static/structure/corellia/corl_imprv_wall_4x32_s02.iff", 950.877, 11, 2041.14, 0, math.rad(270))
	--Battlefield Observer Towers, need to add 4x4 on corners and guard posts 4+ across in between the Towers.
	spawnSceneObject(self.planet, "object/building/military/military_outpost_guard_house_imperial.iff", 858.892, 11, 2018.38, 0, math.rad(90))
	spawnSceneObject(self.planet, "object/building/military/military_outpost_guard_house_imperial.iff", 944.892, 11, 2018.38, 0, math.rad(360))
	spawnSceneObject(self.planet, "object/building/military/military_outpost_guard_house_rebel.iff", 858.892, 11, 2160.2, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/building/military/military_outpost_guard_house_rebel.iff", 944.892, 11, 2160.2, 0, math.rad(270))
	-- north and south wall gate houses
	--spawnSceneObject(self.planet, "object/building/military/military_base_gate_house_rebel.iff", 881.40, 11, 2155.64, 0, math.rad(180))
	--spawnSceneObject(self.planet, "object/building/military/military_base_gate_house_rebel.iff", 895.40, 11, 2155.64, 0, math.rad(180))
	--spawnSceneObject(self.planet, "object/building/military/military_base_gate_house_rebel.iff", 909.40, 11, 2155.64, 0, math.rad(180))
	--spawnSceneObject(self.planet, "object/building/military/military_base_gate_house_rebel.iff", 923.40, 11, 2155.64, 0, math.rad(180))
	--spawnSceneObject(self.planet, "object/building/military/military_base_gate_house_imperial.iff", 881.40, 11, 2022.45, 0, math.rad(360))
	--spawnSceneObject(self.planet, "object/building/military/military_base_gate_house_imperial.iff", 895.40, 11, 2022.45, 0, math.rad(360))
	--spawnSceneObject(self.planet, "object/building/military/military_base_gate_house_imperial.iff", 909.40, 11, 2022.45, 0, math.rad(360))
	--spawnSceneObject(self.planet, "object/building/military/military_base_gate_house_imperial.iff", 923.40, 11, 2022.45, 0, math.rad(360))
	--Decoration around the battleground.
	spawnSceneObject(self.planet, "object/static/structure/tatooine/jawa/tent_jawa_01_large.iff", 679, 11.5, 2131, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/building/poi/tatooine_jawa_large_1.iff", 692, 11.5, 2196, 0, math.rad(45))
--	spawnSceneObject(self.planet, "object/tangible/event_perk/rebel_recruiting_station.iff", 902.535, 12, 2193.53, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/building/poi/yavin4_imperial_patrol_camp_large4.iff", 902.151, 12, 1979.21, 0, math.rad(360))
	spawnSceneObject(self.planet, "object/building/tatooine/filler_building_block_64x16_style_01.iff", 693.655, 11, 2231.91, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/building/tatooine/filler_building_block_64x16_style_02.iff", 627.655, 11, 2231.91, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/building/tatooine/filler_building_block_64x32_style_01.iff", 578.271, 11, 2180.6, 0, math.rad(90))
	spawnSceneObject(self.planet, "object/building/tatooine/filler_building_block_64x32_style_02.iff", 578.271, 11, 2114.6, 0, math.rad(90))
	spawnSceneObject(self.planet, "object/building/tatooine/filler_building_block_64x32_style_01.iff", 609.873, 12, 2043.06, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/event_perk/audience_seating.iff", 616, 12, 2145, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/event_perk/fire_pit.iff", 902.167, 12, 2200.13, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/event_perk/fire_pit.iff", 905.502, 12, 1977.87, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/event_perk/imperial_landing_party_lambda_theater.iff", 847.917, 12, 1939.81, 0, math.rad(360))
	spawnSceneObject(self.planet, "object/tangible/event_perk/imperial_landing_party_lambda_theater.iff", 947.917, 12, 1939.81, 0, math.rad(360))
	spawnSceneObject(self.planet, "object/tangible/event_perk/xwing.iff", 861.39, 8, 2252.34, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/event_perk/xwing.iff", 941.39, 12, 2252.34, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/event_perk/rebel_1x10_honorguard.iff", 865.283, 9, 2241.45, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/event_perk/rebel_1x10_honorguard.iff", 865.283, 9, 2238.45, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/event_perk/rebel_1x10_honorguard.iff", 865.283, 9, 2235.45, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/event_perk/rebel_1x10_honorguard.iff", 865.283, 9, 2232.45, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/event_perk/rebel_1x10_honorguard.iff", 944.996, 12, 2241.45, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/event_perk/rebel_1x10_honorguard.iff", 944.996, 12, 2238.45, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/event_perk/rebel_1x10_honorguard.iff", 944.996, 12, 2235.45, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/event_perk/rebel_1x10_honorguard.iff", 944.996, 12, 2232.45, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/event_perk/party_stage_theater.iff", 616.137, 12, 2120.02, 0, math.rad(360))
	spawnSceneObject(self.planet, "object/tangible/event_perk/picnic_pavillion.iff", 616.137, 12, 2098.54, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/event_perk/jawas_and_droids.iff", 693.298, 12, 2194.29, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/statue_tatooine_sunball.iff", 743.373, 2, 2151.5, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/statue_tatooine_sunball.iff", 743.373, 2, 2142.97, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/statue_tatooine_sunball.iff", 805.607, 2, 2140.26, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/statue_tatooine_sunball.iff", 805.988, 2, 2147.61, 0, math.rad(180))
	--Arena Starting Points for Team A and B.
	-- looking to spawn new start spots
	--spawnSceneObject(self.planet, "object/static/structure/general/poi_all_corral_half_32x32_s05.iff", 902, 11, 2040, 0, math.rad(180))
	--spawnSceneObject(self.planet, "object/static/structure/general/poi_all_rebl_corral_32x32_s01.iff", 902, 11, 2040, 0, math.rad(180))
	--spawnSceneObject(self.planet, "object/static/structure/general/poi_all_corral_half_32x32_s05.iff", 902, 11, 2138, 0, math.rad(360))
	--spawnSceneObject(self.planet, "object/static/structure/general/poi_all_rebl_corral_32x32_s01.iff", 902, 11, 2138, 0, math.rad(360))
--	--Line of Sight Breaks for competitors to use to escape ranged or distract.
	spawnSceneObject(self.planet, "object/building/corellia/filler_slum_16x16_s01.iff", 879.523, 12, 2106.3, 0, math.rad(135))
	--spawnSceneObject(self.planet, "object/static/structure/dathomir/mtn_clan_tent_house_s01.iff", 879.523, 12, 2106.3, 0, math.rad(270))
	spawnSceneObject(self.planet, "object/building/corellia/filler_slum_16x16_s01.iff", 924.706, 12, 2073.15, 0, math.rad(17))
	--spawnSceneObject(self.planet, "object/static/structure/dathomir/nsister_tent_house_style_01.iff", 924.706, 12, 2073.15, 0, math.rad(270))
	
	spawnSceneObject(self.planet, "object/building/poi/creature_lair_imperial_destroy.iff", 903, 12, 2088, 0, math.rad(180))
	--spawnSceneObject(self.planet, "object/building/endor/endor_tiki_torch.iff", 911.087, 12, 2050.97, 0, math.rad(180))
	--spawnSceneObject(self.planet, "object/building/endor/endor_tiki_torch.iff", 892.943, 12, 2050.97, 0, math.rad(180))
	--spawnSceneObject(self.planet, "object/building/endor/endor_tiki_torch.iff", 892.943, 12, 2127.02, 0, math.rad(180))
	--spawnSceneObject(self.planet, "object/building/endor/endor_tiki_torch.iff", 911.087, 12, 2127.02, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/event_perk/crashed_tie_bomber_theater.iff", 935, 11, 2125, 0, math.rad(270))
	spawnSceneObject(self.planet, "object/static/structure/general/debris_deathstar_turbolaser.iff", 896, 12, 2091, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/event_perk/crashed_xwing_theater.iff", 873.269, 12, 2051.61, 0, math.rad(90))
	--Buff Centers for both sides.
	spawnSceneObject(self.planet, "object/building/corellia/ply_corl_house_m_s01_fp1.iff", 903.306, 11.75, 2249.8, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/building/corellia/ply_corl_house_m_s01_fp1.iff", 902.312, 12, 1932.23, 0, math.rad(360))
	--Tavern Lighting.
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_red_02.iff", 897.869, 11, 2241.22, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_red_02.iff", 893.579, 11, 2241.22, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_blue_02.iff", 907.785, 12, 1940.81, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_blue_02.iff", 911.799, 12, 1940.81, 0, math.rad(180))
	--Battleground Complete Area Lighting
	spawnSceneObject(self.planet, "object/tangible/furniture/city/fountain_brazier_round.iff", 694.287, 11.5, 2144.76, 0, math.rad(90))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/fountain_brazier_round.iff", 694.287, 11.5, 2175.76, 0, math.rad(90))

	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_red_02.iff", 853.212, 9, 2167.88, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_red_02.iff", 862.212, 10, 2167.88, 0, math.rad(180))
	
	--[[
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_red_02.iff", 877.212, 11, 2167.88, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_red_02.iff", 880.212, 11, 2167.88, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_red_02.iff", 891.212, 11, 2167.88, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_red_02.iff", 894.212, 11, 2167.88, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_red_02.iff", 905.212, 11, 2167.88, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_red_02.iff", 908.212, 11, 2167.88, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_red_02.iff", 919.212, 11, 2167.88, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_red_02.iff", 922.876, 11, 2167.88, 0, math.rad(180))
	]]
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_red_02.iff", 951.392, 11, 2157.07, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_red_02.iff", 951.392, 11, 2165.07, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_red_02.iff", 909.959, 11, 2185.07, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_red_02.iff", 896.296, 11, 2185.07, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_red_02.iff", 909.959, 11, 2202.28, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_red_02.iff", 896.296, 11, 2202.28, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_red_02.iff", 858.379, 8, 2242.25, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_red_02.iff", 864.38, 9, 2242.25, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_red_02.iff", 937.564, 11, 2242.25, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_red_02.iff", 944.619, 11, 2242.25, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_blue_02.iff", 941.378, 12, 2011.88, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_blue_02.iff", 950.098, 12, 2011.88, 0, math.rad(180))

	--[[
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_blue_02.iff", 926.619, 12, 2010.21, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_blue_02.iff", 923.619, 12, 2010.21, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_blue_02.iff", 912.619, 12, 2010.21, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_blue_02.iff", 909.619, 12, 2010.21, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_blue_02.iff", 898.899, 12, 2010.21, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_blue_02.iff", 895.619, 12, 2010.21, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_blue_02.iff", 885.014, 12, 2010.21, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_blue_02.iff", 881.924, 12, 2010.21, 0, math.rad(180))
	]]

	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_blue_02.iff", 852.392, 12, 2021.88, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_blue_02.iff", 852.392, 12, 2013.88, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_blue_02.iff", 944.619, 12, 2021.88, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_blue_02.iff", 888.92, 12, 1996.39, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_blue_02.iff", 919.449, 12, 1966.02, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_blue_02.iff", 904.96, 12, 1966.05, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_blue_02.iff", 889.077, 12, 1982.03, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_blue_02.iff", 846.674, 12, 1950.91, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_blue_02.iff", 855.539, 12, 1950.91, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_blue_02.iff", 946.879, 12, 1950.91, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_blue_02.iff", 955.098, 12, 1950.91, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_green_02.iff", 675.315, 11, 2111.25, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_green_02.iff", 670.762, 11, 2112.64, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_blue_02.iff", 955.098, 12, 1950.91, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_blue_02.iff", 610.348, 12, 2148.82, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_blue_02.iff", 621.348, 12, 2148.82, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_blue_02.iff", 610.348, 12, 2141.86, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_blue_02.iff", 621.348, 12, 2141.86, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_blue_02.iff", 608.472, 13, 2126.57, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_blue_02.iff", 623.472, 13, 2126.57, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_blue_02.iff", 608.472, 13, 2113.83, 0, math.rad(180))
	spawnSceneObject(self.planet, "object/tangible/furniture/city/streetlamp_large_blue_02.iff", 623.472, 13, 2113.83, 0, math.rad(180))

end
