-- Planet Region Definitions
--
-- {"regionName", x, y, shape and size, tier, {"spawnGroup1", ...}, maxSpawnLimit}
-- For circle and ring, x and y are the center point
-- For rectangles, x and y are the bottom left corner. x2 and y2 (see below) are the upper right corner
-- Shape and size is a table with the following format depending on the shape of the area:
--   - Circle: {CIRCLE, radius}
--   - Rectangle: {RECTANGLE, x2, y2}
--   - Ring: {RING, inner radius, outer radius}
-- Tier is a bit mask with the following possible values where each hexadecimal position is one possible configuration.
-- That means that it is not possible to have both a spawn area and a no spawn area in the same region, but
-- a spawn area that is also a no build zone is possible.

require("scripts.managers.planet.regions")

kashyyyk_regions = {
	-- No Build Zones
	{"basic", -1, 1, {CIRCLE, 50}, NOBUILDZONEAREA},
	-- world spawns
	{"kashyyyk_world1", -2240, -3232, {CIRCLE, 1600}, SPAWNAREA, {"kashyyyk_world"}, 2048},
	{"kashyyyk_world2", -1248, -8000, {RECTANGLE, 8000, -3008}, SPAWNAREA, {"kashyyyk_world"}, 2048},
	{"kashyyyk_world3", -8000, -3008, {RECTANGLE, 8000, 8000}, SPAWNAREA, {"kashyyyk_world"}, 2048},
	-- hard spawn areas
	{"kashyyyk_hard1", -5000, 5000, {CIRCLE, 2300}, SPAWNAREA, {"kashyyyk_hard"}, 1024},
	{"kashyyyk_hard2", 64, -5216, {CIRCLE, 1568}, SPAWNAREA, {"kashyyyk_hard"}, 512},
	{"kashyyyk_hard3", 6880, 4864, {CIRCLE, 1000}, SPAWNAREA, {"kashyyyk_hard"}, 1024},
	{"kashyyyk_hard4", 4320, -6048, {CIRCLE, 1184}, SPAWNAREA, {"kashyyyk_hard"}, 1024},
	{"kashyyyk_hard5", 2048, -1952, {CIRCLE, 1216}, SPAWNAREA, {"kashyyyk_hard"}, 1024},

	-- boss spawn areas
	{"kashyyyk_boss_bolataur1", 5248, -1952, {CIRCLE, 608}, SPAWNAREA, {"kashyyyk_boss_bolataur"}, 2},
	{"kashyyyk_boss_bolataur2", 7312, -2384, {CIRCLE, 496}, SPAWNAREA, {"kashyyyk_boss_bolataur"}, 2},
	
	{"kashyyyk_boss_jyykle1", -5448, -312, {CIRCLE, 184}, SPAWNAREA, {"kashyyyk_boss_jyykle"}, 2},
	{"kashyyyk_boss_jyykle2", -6264, 600, {CIRCLE, 248}, SPAWNAREA, {"kashyyyk_boss_jyykle"}, 2},
	{"kashyyyk_boss_jyykle3", -2984, -936, {CIRCLE, 168}, SPAWNAREA, {"kashyyyk_boss_jyykle"}, 2},
	{"kashyyyk_boss_jyykle4", -5208, 1752, {CIRCLE, 472}, SPAWNAREA, {"kashyyyk_boss_jyykle"}, 2},

	{"kashyyyk_boss_katarn1", -3672, 744, {CIRCLE, 344}, SPAWNAREA, {"kashyyyk_boss_katarn"}, 2},
	{"kashyyyk_boss_katarn2", 432, -3136, {CIRCLE, 576}, SPAWNAREA, {"kashyyyk_boss_katarn"}, 2},
	{"kashyyyk_boss_katarn3", 1936, -5552, {CIRCLE, 432}, SPAWNAREA, {"kashyyyk_boss_katarn"}, 2},
	{"kashyyyk_boss_katarn4", 6080, -5952, {CIRCLE, 208}, SPAWNAREA, {"kashyyyk_boss_katarn"}, 2},

	{"kashyyyk_boss_kklyyytt1", -1704, 2184, {CIRCLE, 696}, SPAWNAREA, {"kashyyyk_boss_kklyyytt"}, 2},
	{"kashyyyk_boss_kklyyytt2", -648, 3736, {CIRCLE, 344}, SPAWNAREA, {"kashyyyk_boss_kklyyytt"}, 2},
	{"kashyyyk_boss_kklyyytt3", -3632, 4000, {CIRCLE, 688}, SPAWNAREA, {"kashyyyk_boss_kklyyytt"}, 2},
	{"kashyyyk_boss_kklyyytt4", -6416, 2384, {CIRCLE, 832}, SPAWNAREA, {"kashyyyk_boss_kklyyytt"}, 2},
	{"kashyyyk_boss_kklyyytt5", -1248, -16, {CIRCLE, 1568}, SPAWNAREA, {"kashyyyk_boss_kklyyytt"}, 2},
	{"kashyyyk_boss_kklyyytt6", -1424, 6000, {CIRCLE, 1104}, SPAWNAREA, {"kashyyyk_boss_kklyyytt"}, 2},

	{"kashyyyk_boss_minstyngar1", -2512, -2640, {CIRCLE, 1100}, SPAWNAREA, {"kashyyyk_boss_minstyngar"}, 2},

	{"kashyyyk_boss_mouf1", 760, -1320, {CIRCLE, 456}, SPAWNAREA, {"kashyyyk_boss_mouf"}, 2},
	{"kashyyyk_boss_mouf2", 1736, -2072, {CIRCLE, 232}, SPAWNAREA, {"kashyyyk_boss_mouf"}, 2},
	{"kashyyyk_boss_mouf3", 2184, -3096, {CIRCLE, 152}, SPAWNAREA, {"kashyyyk_boss_mouf"}, 2},
	{"kashyyyk_boss_mouf4", 2304, -3440, {CIRCLE, 144}, SPAWNAREA, {"kashyyyk_boss_mouf"}, 2},
	{"kashyyyk_boss_mouf5", 1120, 272, {CIRCLE, 368}, SPAWNAREA, {"kashyyyk_boss_mouf"}, 2},
	{"kashyyyk_boss_mouf6", 784, 4416, {CIRCLE, 720}, SPAWNAREA, {"kashyyyk_boss_mouf"}, 2},
	{"kashyyyk_boss_mouf7", -7648, 5504, {CIRCLE, 160}, SPAWNAREA, {"kashyyyk_boss_mouf"}, 2},
	{"kashyyyk_boss_mouf8", -7720, 3608, {CIRCLE, 136}, SPAWNAREA, {"kashyyyk_boss_mouf"}, 2},
	{"kashyyyk_boss_mouf9", -7640, 2696, {CIRCLE, 248}, SPAWNAREA, {"kashyyyk_boss_mouf"}, 2},
	{"kashyyyk_boss_mouf10", -7616, 1360, {CIRCLE, 304}, SPAWNAREA, {"kashyyyk_boss_mouf"}, 2},
	{"kashyyyk_boss_mouf11", -3528, 2264, {CIRCLE, 488}, SPAWNAREA, {"kashyyyk_boss_mouf"}, 2},
	{"kashyyyk_boss_mouf12", 2024, 2040, {CIRCLE, 504}, SPAWNAREA, {"kashyyyk_boss_mouf"}, 2},

	{"kashyyyk_boss_pug1", 3088, 4208, {CIRCLE, 608}, SPAWNAREA, {"kashyyyk_boss_pug"}, 2},
	{"kashyyyk_boss_pug2", 5288, 5304, {CIRCLE, 568}, SPAWNAREA, {"kashyyyk_boss_pug"}, 2},
	{"kashyyyk_boss_pug3", 6608, 6544, {CIRCLE, 576}, SPAWNAREA, {"kashyyyk_boss_pug"}, 2},
	{"kashyyyk_boss_pug4", 5552, 4448, {CIRCLE, 576}, SPAWNAREA, {"kashyyyk_boss_pug"}, 2},
	{"kashyyyk_boss_pug5", 4368, 4432, {CIRCLE, 432}, SPAWNAREA, {"kashyyyk_boss_pug"}, 2},
	{"kashyyyk_boss_pug6", 6520, 5368, {CIRCLE, 488}, SPAWNAREA, {"kashyyyk_boss_pug"}, 2},
	{"kashyyyk_boss_pug7", 6272, 3216, {CIRCLE, 256}, SPAWNAREA, {"kashyyyk_boss_pug"}, 2},
	{"kashyyyk_boss_pug8", 5208, -6040, {CIRCLE, 360}, SPAWNAREA, {"kashyyyk_boss_pug"}, 2},
	
	{"kashyyyk_boss_roroo1", 3336, -4200, {CIRCLE, 440}, SPAWNAREA, {"kashyyyk_boss_roroo"}, 2},
	{"kashyyyk_boss_roroo2", 4880, -3760, {CIRCLE, 288}, SPAWNAREA, {"kashyyyk_boss_roroo"}, 2},
	{"kashyyyk_boss_roroo3", 5584, -4848, {CIRCLE, 336}, SPAWNAREA, {"kashyyyk_boss_roroo"}, 2},
	{"kashyyyk_boss_roroo4", 3840, -2240, {CIRCLE, 336}, SPAWNAREA, {"kashyyyk_boss_roroo"}, 2},
	{"kashyyyk_boss_roroo5", 6728, -4488, {CIRCLE, 392}, SPAWNAREA, {"kashyyyk_boss_roroo"}, 2},

	{"kashyyyk_boss_sathog1", 2128, -7232, {CIRCLE, 432}, SPAWNAREA, {"kashyyyk_boss_sathog"}, 2},
	{"kashyyyk_boss_sathog2", 864, -7248, {CIRCLE, 400}, SPAWNAREA, {"kashyyyk_boss_sathog"}, 2},
	{"kashyyyk_boss_sathog3", -448, -5600, {CIRCLE, 448}, SPAWNAREA, {"kashyyyk_boss_sathog"}, 2},
	{"kashyyyk_boss_sathog4", 328, -4984, {CIRCLE, 488}, SPAWNAREA, {"kashyyyk_boss_sathog"}, 2},
	{"kashyyyk_boss_sathog5", 3216, -5424, {CIRCLE, 432}, SPAWNAREA, {"kashyyyk_boss_sathog"}, 2},
	
	{"kashyyyk_boss_uller1", 672, -6304, {CIRCLE, 240}, SPAWNAREA, {"kashyyyk_boss_uller"}, 2},
	{"kashyyyk_boss_uller2", 1368, -6392, {CIRCLE, 152}, SPAWNAREA, {"kashyyyk_boss_uller"}, 2},
	{"kashyyyk_boss_uller3", 2112, -6512, {CIRCLE, 160}, SPAWNAREA, {"kashyyyk_boss_uller"}, 2},
	{"kashyyyk_boss_uller4", 4000, -7152, {CIRCLE, 240}, SPAWNAREA, {"kashyyyk_boss_uller"}, 2},
	{"kashyyyk_boss_uller5", -7056, -2064, {CIRCLE, 544}, SPAWNAREA, {"kashyyyk_boss_uller"}, 2},
	{"kashyyyk_boss_uller6", -7176, -824, {CIRCLE, 504}, SPAWNAREA, {"kashyyyk_boss_uller"}, 2},
	
	{"kashyyyk_boss_walluga1", 3888, -32, {CIRCLE, 416}, SPAWNAREA, {"kashyyyk_boss_walluga"}, 2},
	{"kashyyyk_boss_walluga2", 3296, -3088, {CIRCLE, 432}, SPAWNAREA, {"kashyyyk_boss_walluga"}, 2},
	{"kashyyyk_boss_walluga3", 4400, -4336, {CIRCLE, 256}, SPAWNAREA, {"kashyyyk_boss_walluga"}, 2},
	{"kashyyyk_boss_walluga4", 3352, -4232, {CIRCLE, 392}, SPAWNAREA, {"kashyyyk_boss_walluga"}, 2},
	
	{"kashyyyk_boss_webweaver1", 1296, -3568, {CIRCLE, 192}, SPAWNAREA, {"kashyyyk_boss_webweaver"}, 2},
	{"kashyyyk_boss_webweaver2", 1536, -4112, {CIRCLE, 176}, SPAWNAREA, {"kashyyyk_boss_webweaver"}, 2},
	{"kashyyyk_boss_webweaver3", 2076, -4548, {CIRCLE, 204}, SPAWNAREA, {"kashyyyk_boss_webweaver"}, 2},
	{"kashyyyk_boss_webweaver4", 1900, -5412, {CIRCLE, 324}, SPAWNAREA, {"kashyyyk_boss_webweaver"}, 2},
	{"kashyyyk_boss_webweaver5", 2936, -5408, {CIRCLE, 80}, SPAWNAREA, {"kashyyyk_boss_webweaver"}, 2},
	
	{"kashyyyk_boss_ysalamiri1", -4528, -2016, {CIRCLE, 368}, SPAWNAREA, {"kashyyyk_boss_ysalamiri"}, 2},
	{"kashyyyk_boss_ysalamiri2", -4616, -424, {CIRCLE, 356}, SPAWNAREA, {"kashyyyk_boss_ysalamiri"}, 2},
	{"kashyyyk_boss_ysalamiri3", -1048, -2040, {CIRCLE, 216}, SPAWNAREA, {"kashyyyk_boss_ysalamiri"}, 2},
	{"kashyyyk_boss_ysalamiri4", 6832, -3184, {CIRCLE, 672}, SPAWNAREA, {"kashyyyk_boss_ysalamiri"}, 2},
	{"kashyyyk_boss_ysalamiri5", 6800, 1968, {CIRCLE, 480}, SPAWNAREA, {"kashyyyk_boss_ysalamiri"}, 2},
	{"kashyyyk_boss_ysalamiri6", 4760, 3032, {CIRCLE, 648}, SPAWNAREA, {"kashyyyk_boss_ysalamiri"}, 2},
	{"kashyyyk_boss_ysalamiri7", -128, 6976, {CIRCLE, 640}, SPAWNAREA, {"kashyyyk_boss_ysalamiri"}, 2},
	{"kashyyyk_boss_ysalamiri8", 904, 7256, {CIRCLE, 440}, SPAWNAREA, {"kashyyyk_boss_ysalamiri"}, 2},
	{"kashyyyk_boss_ysalamiri9", 1592, 6344, {CIRCLE, 432}, SPAWNAREA, {"kashyyyk_boss_ysalamiri"}, 2},
	{"kashyyyk_boss_ysalamiri10", 7320, 3160, {CIRCLE, 472}, SPAWNAREA, {"kashyyyk_boss_ysalamiri"}, 2},
	
	-- Forest
	{"kashyyyk_forest1", 5024, 7424, {CIRCLE, 448}, SPAWNAREA, {"kashyyyk_forest"}, 2},
	{"kashyyyk_forest2", -1568, 7456, {CIRCLE, 224}, SPAWNAREA, {"kashyyyk_forest"}, 2},
	{"kashyyyk_forest3", 144, 5456, {CIRCLE, 272}, SPAWNAREA, {"kashyyyk_forest"}, 2},
	{"kashyyyk_forest4", -3584, 1600, {CIRCLE, 472}, SPAWNAREA, {"kashyyyk_forest"}, 2},
	{"kashyyyk_forest5", -2512, -48, {CIRCLE, 368}, SPAWNAREA, {"kashyyyk_forest"}, 2},
	{"kashyyyk_forest6", 704, 1824, {CIRCLE, 224}, SPAWNAREA, {"kashyyyk_forest"}, 2},
	{"kashyyyk_forest7", 5280, -256, {CIRCLE, 192}, SPAWNAREA, {"kashyyyk_forest"}, 2},
	{"kashyyyk_forest8", 3904, -1568, {CIRCLE, 384}, SPAWNAREA, {"kashyyyk_forest"}, 2},
	{"kashyyyk_forest9", -7408, 4912, {CIRCLE, 304}, SPAWNAREA, {"kashyyyk_forest"}, 2},
	{"kashyyyk_forest10", -6672, 4112, {CIRCLE, 240}, SPAWNAREA, {"kashyyyk_forest"}, 2},
	{"kashyyyk_forest11", -6400, 3104, {CIRCLE, 194}, SPAWNAREA, {"kashyyyk_forest"}, 2},
	{"kashyyyk_forest12", -4704, 3616, {CIRCLE, 384}, SPAWNAREA, {"kashyyyk_forest"}, 2},

	-- rryatt
	{"kashyyyk_rryatt1", -3704, 712, {CIRCLE, 207}, SPAWNAREA, {"kashyyyk_rryatt"}, 24},
	{"kashyyyk_rryatt2", -7176, 4400, {CIRCLE, 120}, SPAWNAREA, {"kashyyyk_rryatt"}, 24},
	{"kashyyyk_rryatt3", 5700, 2996, {CIRCLE, 148}, SPAWNAREA, {"kashyyyk_rryatt"}, 24},
	{"kashyyyk_rryatt4", 6716, 3340, {CIRCLE, 108}, SPAWNAREA, {"kashyyyk_rryatt"}, 24},
	{"kashyyyk_rryatt5", 7104, 3432, {CIRCLE, 72}, SPAWNAREA, {"kashyyyk_rryatt"}, 24},
	{"kashyyyk_rryatt6", 6188, 2916, {CIRCLE, 140}, SPAWNAREA, {"kashyyyk_rryatt"}, 24},
	{"kashyyyk_rryatt7", 148, -3188, {CIRCLE, 164}, SPAWNAREA, {"kashyyyk_rryatt"}, 24},
	{"kashyyyk_rryatt8", 580, -2612, {CIRCLE, 132}, SPAWNAREA, {"kashyyyk_rryatt"}, 24},
	{"kashyyyk_rryatt9", 1424, -2612, {CIRCLE, 272}, SPAWNAREA, {"kashyyyk_rryatt"}, 24},
	{"kashyyyk_rryatt10", 1936, -4376, {CIRCLE, 232}, SPAWNAREA, {"kashyyyk_rryatt"}, 24},
	

	-- rebel base
	{"kashyyyk_rebelbase1", 800, 4296, {CIRCLE, 196}, NOSPAWNAREA + NOBUILDZONEAREA},
	{"kashyyyk_rebelbase2", 800, 4296, {CIRCLE, 1024}, NOBUILDZONEAREA},

	-- imperial base
	{"kashyyyk_imperialbase1", 1889, -7554, {CIRCLE, 196}, NOSPAWNAREA + NOBUILDZONEAREA},
	{"kashyyyk_imperialbase2", 1889, -7554, {CIRCLE, 1024}, NOBUILDZONEAREA},

	-- wawaatt
	{"kashyyyk_wawa1", 3416, 1384, {CIRCLE, 500}, NOSPAWNAREA + NOBUILDZONEAREA},
	{"kashyyyk_wawa2", 3358, 1815, {CIRCLE, 256}, NOSPAWNAREA + NOBUILDZONEAREA},
	{"kashyyyk_wawa3", 3416, 1384, {CIRCLE, 1024}, NOBUILDZONEAREA},
	{"@kashyyyk_region_names:wawaatt", 3204, 1035, {CIRCLE, 400}, CITY + NOSPAWNAREA + NOBUILDZONEAREA + NAVAREA},

	-- New Trandosha
	{"@kashyyyk_region_names:new_trandosha", 7319, -4460, {CIRCLE, 750}, CITY + NOSPAWNAREA + NAVAREA + NOBUILDZONEAREA},
	-- graveyard
	{"kashyyyk_graveyard1", 2500, -1200, {RECTANGLE, 2800, -1000}, NOSPAWNAREA + NOBUILDZONEAREA + NAVAREA},
	{"kashyyyk_graveyard2", 2641, -1102, {CIRCLE, 300}, NOSPAWNAREA + NOBUILDZONEAREA + NAVAREA},
	
	-- crash sites
	{"kashyyyk_crash1", 3965, -6265, {CIRCLE, 300}, NOSPAWNAREA + NOBUILDZONEAREA + NAVAREA},
	{"kashyyyk_crash2", -5804, -1733, {CIRCLE, 500}, NOBUILDZONEAREA}, -- Greenbark space station
	
	-- arena
	{"kashyyyk_arena1", 6000, -4000, {CIRCLE, 600}, NOBUILDZONEAREA},
	{"kashyyyk_arena2", 6000, -4000, {CIRCLE, 400}, NOSPAWNAREA + NAVAREA},

	-- dungeons 
	{"kashyyyk_tree_cave", -3700, 700, {CIRCLE, 1024}, NOBUILDZONEAREA},
	{"kashyyyk_tree_cave_nav", -3700, 700, {CIRCLE, 256}, NAVAREA},
	{"kashyyyk_new_trando_cave", 7098, -4565, {CIRCLE, 128}, NOBUILDZONEAREA + NAVAREA},
	{"@kashyyyk_region_names:azure_veil_mine", -2992, -2250, {CIRCLE, 256}, CITY + NOSPAWNAREA + NOBUILDZONEAREA + NAVAREA},

	-- {"northedge_corellia_nobuild", -8000, 7640, {RECTANGLE, 8000, 8000}, NOBUILDZONEAREA},
	--{"sdungeon_rogue_corsec_bunker", 5225, 1567, {CIRCLE, 128}, NOSPAWNAREA + NOBUILDZONEAREA},
	--{"sdungeon_imperial_outpost", 4664, -5784, {CIRCLE, 128}, NOBUILDZONEAREA},
}
