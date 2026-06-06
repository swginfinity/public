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

hoth_regions = {
	-- No Build Zones
	{"fullplanet", -1, 1, {CIRCLE, 12000}, NOBUILDZONEAREA},

	-- outposts
	{"@hoth_region_names:hothstarport", 0, -2000, {CIRCLE, 512}, CITY + NOSPAWNAREA + NAVAREA},
	{"hoth_zone_1", -5900, 3300, {CIRCLE, 720}, NOSPAWNAREA},
	{"hoth_zone_2_nav", 4800, -500, {RECTANGLE, 6200, 2400}, NAVAREA},
	{"hoth_zone_2_nospawn", 5200, 400, {CIRCLE, 2000}, NOSPAWNAREA},

	-- spawns
	-- Northwest Corner: Quest Area - Toughest creatures, minimal GCW
	{"hoth_northwest_quest_area", -7680, 1000, {RECTANGLE, -1000, 7680}, SPAWNAREA, {"hoth_northwest_elite"}, 512},

	-- Northeast Corner: Second toughest, moderate GCW
	{"hoth_northeast_hard", 1000, 1000, {RECTANGLE, 7680, 7680}, SPAWNAREA, {"hoth_northeast_hard"}, 512},

	-- General World: Easier creatures, heavy GCW
	{"hoth_world_spawner_01", 0, 0, {RECTANGLE, 0, 0}, SPAWNAREA + WORLDSPAWNAREA, {"hoth_world"}, 1024},
}
