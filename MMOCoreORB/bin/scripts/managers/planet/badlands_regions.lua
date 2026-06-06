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
--
-- TIER FLAGS:
--   UNDEFINEDAREA = 0x00000000
--   SPAWNAREA = 0x00000001
--   WORLDSPAWNAREA = 0x00000002
--   NOSPAWNAREA = 0x00000004
--   NOWORLDSPAWNAREA = 0x00000008
--   NOBUILDZONEAREA = 0x00000010
--   GEOAREA = 0x00000020
--   CITY = 0x00000040
--   NAVAREA = 0x00000080
--   NAMEDREGION = 0x00000100
--
-- You can combine flags with +, for example:
--   NOSPAWNAREA + NOBUILDZONEAREA + NAMEDREGION + NAVAREA

require("scripts.managers.planet.regions")

badlands_regions = {
	-- No Build Zones
	{"fullplanet", -1, 1, {CIRCLE, 12000}, NOBUILDZONEAREA},
	-- ═══════════════════════════════════════════════════════════════════════════
	-- WORLD BOUNDARIES (No Build Zones at edges)
	-- ═══════════════════════════════════════════════════════════════════════════
	--{"northedge_wasteland_nobuild", -7980, 7640, {RECTANGLE, 8000, 8000}, NOBUILDZONEAREA},
	--{"westedge_wasteland_nobuild", -7980, -7641, {RECTANGLE, -7640, 7640}, NOBUILDZONEAREA},
	--{"southedge_wasteland_nobuild", -8000, -8000, {RECTANGLE, 8000, -7640}, NOBUILDZONEAREA},
	--{"eastedge_wasteland_nobuild", 7640, -7640, {RECTANGLE, 8000, 7640}, NOBUILDZONEAREA},

	-- ═══════════════════════════════════════════════════════════════════════════
	-- NAMED REGIONS
	-- ═══════════════════════════════════════════════════════════════════════════
	{"the_proving_sands", 5051, 4058, {CIRCLE, 1500}, NAMEDREGION},
	{"the_sepulcher_vale", 5000, -2300, {CIRCLE, 1000}, NAMEDREGION},

	-- ═══════════════════════════════════════════════════════════════════════════
	-- SPAWN AREAS
	-- ═══════════════════════════════════════════════════════════════════════════

	-- THE SEPULCHER VALE - Krayt Heavy Zone (5000, 4000 - 1000m radius)
	-- High-tier krayts, elite spawns, larger lair sizes
	{"wasteland_krayt_zone", 5000, 4000, {CIRCLE, 1000}, SPAWNAREA, {"badlands_krayt_zone"}, 512},

	-- THE PROVING SANDS - Mixed Zone (-2000, 2000 - 1500m radius)
	-- Balanced mix of all creature types
	{"wasteland_proving_sands", 5051, 4058, {CIRCLE, 1500}, SPAWNAREA, {"badlands_mixed"}, 512},

	-- GENERAL WASTELAND - World Spawner (covers entire planet)
	-- Emphasis on mutated beasts and regular tatooine creatures
	{"wasteland_world_spawner", 0, 0, {RECTANGLE, 0, 0}, SPAWNAREA + WORLDSPAWNAREA, {"badlands_world"}, 1024},

	-- ═══════════════════════════════════════════════════════════════════════════
	-- NO BUILD / NO SPAWN ZONES
	-- ═══════════════════════════════════════════════════════════════════════════
	-- Add protected areas here (POIs, dungeons, etc.)
	-- Example:
	-- {"wasteland_dungeon_entrance", 0, 0, {CIRCLE, 100}, NOSPAWNAREA + NOBUILDZONEAREA + NAVAREA},
	{"sidrona_chanting_grounds_1", 800, 800, {CIRCLE, 256}, NOSPAWNAREA + NOBUILDZONEAREA + NAVAREA},
	{"sidrona_chanting_grounds_2", 3241.92, -6524.40, {CIRCLE, 256}, NOSPAWNAREA + NOBUILDZONEAREA + NAVAREA},
	{"sidrona_chanting_grounds_3", 5916.58, -4729.29, {CIRCLE, 256}, NOSPAWNAREA + NOBUILDZONEAREA + NAVAREA},
	{"sidrona_chanting_grounds_4", 5930.69, -4004.39, {CIRCLE, 256}, NOSPAWNAREA + NOBUILDZONEAREA + NAVAREA},
	{"sidrona_chanting_grounds_5", 7129.80, -1792.40, {CIRCLE, 256}, NOSPAWNAREA + NOBUILDZONEAREA + NAVAREA},
	{"sidrona_chanting_grounds_6", 6052.67, -343.22, {CIRCLE, 256}, NOSPAWNAREA + NOBUILDZONEAREA + NAVAREA},
	{"sidrona_chanting_grounds_7", 4253.23, 1193.44, {CIRCLE, 256}, NOSPAWNAREA + NOBUILDZONEAREA + NAVAREA},
	{"sidrona_chanting_grounds_8", 4488.19, 2909.79, {CIRCLE, 256}, NOSPAWNAREA + NOBUILDZONEAREA + NAVAREA},
	{"sidrona_chanting_grounds_9", 6697.53, 6371.29, {CIRCLE, 256}, NOSPAWNAREA + NOBUILDZONEAREA + NAVAREA},
	{"sidrona_chanting_grounds_10", 2846.25, 5454.80, {CIRCLE, 256}, NOSPAWNAREA + NOBUILDZONEAREA + NAVAREA},
	{"sidrona_chanting_grounds_11", -969.31, 4567.74, {CIRCLE, 256}, NOSPAWNAREA + NOBUILDZONEAREA + NAVAREA},
	{"sidrona_chanting_grounds_12", -2294.74, 1976.83, {CIRCLE, 256}, NOSPAWNAREA + NOBUILDZONEAREA + NAVAREA},
	{"sidrona_chanting_grounds_13", 848.91, 1284.04, {CIRCLE, 256}, NOSPAWNAREA + NOBUILDZONEAREA + NAVAREA},
	{"sidrona_chanting_grounds_14", -2688.92, -1093.51, {CIRCLE, 256}, NOSPAWNAREA + NOBUILDZONEAREA + NAVAREA},
	{"sidrona_chanting_grounds_15", -6881.49, 295.99, {CIRCLE, 256}, NOSPAWNAREA + NOBUILDZONEAREA + NAVAREA},
	{"sidrona_chanting_grounds_16", -6814.30, 6961.02, {CIRCLE, 256}, NOSPAWNAREA + NOBUILDZONEAREA + NAVAREA},
	{"sidrona_chanting_grounds_17", -6066, -7233, {CIRCLE, 256}, NOSPAWNAREA + NOBUILDZONEAREA + NAVAREA},
	-- ═══════════════════════════════════════════════════════════════════════════
	-- CITIES / OUTPOSTS
	-- ═══════════════════════════════════════════════════════════════════════════
	-- Add city regions here
	-- Example:
	{"wasteland_sidrona", -7326, -5259, {CIRCLE, 512}, NOSPAWNAREA + NOBUILDZONEAREA + NAVAREA},

	-- ═══════════════════════════════════════════════════════════════════════════
	-- Death Watch
	-- ═══════════════════════════════════════════════════════════════════════════
	{"death_watch_black_sun",  3187, -6755, {CIRCLE, 256}, NOSPAWNAREA + NOBUILDZONEAREA + NAVAREA},
	{"death_watch_zann",        659,   249, {CIRCLE, 256}, NOSPAWNAREA + NOBUILDZONEAREA + NAVAREA},
	{"death_watch_xonti",     -2079,  4848, {CIRCLE, 256}, NOSPAWNAREA + NOBUILDZONEAREA + NAVAREA},
	{"death_watch_krrsantan", -7307,  4308, {CIRCLE, 256}, NOSPAWNAREA + NOBUILDZONEAREA + NAVAREA},

}


