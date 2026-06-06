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

dungeon2_regions = {
	-- No Build Zones
	--{"myyydril_cavern", 0, 500, {CIRCLE, 600}, NAVAREA},
	-- dungeons 
	-- {"northedge_corellia_nobuild", -8000, 7640, {RECTANGLE, 8000, 8000}, NOBUILDZONEAREA},
	--{"sdungeon_rogue_corsec_bunker", 5225, 1567, {CIRCLE, 128}, NOSPAWNAREA + NOBUILDZONEAREA},
	--{"sdungeon_imperial_outpost", 4664, -5784, {CIRCLE, 128}, NOBUILDZONEAREA},
	{"axkva01", -7500, -7500, {CIRCLE, 256}, NAVAREA},
	{"axkva02", -7500, -7250, {CIRCLE, 256}, NAVAREA},
	{"axkva03", -7500, -7000, {CIRCLE, 256}, NAVAREA},
	{"axkva04", -7500, -6750, {CIRCLE, 256}, NAVAREA},
	{"axkva05", -7500, -6500, {CIRCLE, 256}, NAVAREA},
	{"axkva06", -7500, -6250, {CIRCLE, 256}, NAVAREA},
	{"axkva07", -7250, -7500, {CIRCLE, 256}, NAVAREA},
	{"axkva08", -7250, -7250, {CIRCLE, 256}, NAVAREA},
	{"axkva09", -7250, -7000, {CIRCLE, 256}, NAVAREA},
	{"axkva10", -7250, -6750, {CIRCLE, 256}, NAVAREA},
	{"axkva11", -7250, -6500, {CIRCLE, 256}, NAVAREA},
	{"axkva12", -7250, -6250, {CIRCLE, 256}, NAVAREA},
}
