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

etyyy_regions = {
	-- No Build Zones
	{"fullplanet", -1, 1, {CIRCLE, 12000}, NOBUILDZONEAREA},

	-- outposts
	{"@etyyy_region_names:outpost_main", 272, 502, {CIRCLE, 250}, CITY + NOSPAWNAREA + NAVAREA},
	{"@etyyy_region_names:outpost_rodian", 559, 533, {CIRCLE, 128}, CITY  + NAVAREA},
	{"@etyyy_region_names:outpost_arcona", -597, 873, {CIRCLE, 128}, CITY  + NAVAREA},
	{"@etyyy_region_names:outpost_bandit", 39, -490, {CIRCLE, 128}, CITY  + NAVAREA},
	{"@etyyy_region_names:outpost_chiss", -1211, -115, {CIRCLE, 128}, CITY  + NAVAREA},
	{"@etyyy_region_names:outpost_blackscale", 36, -192, {CIRCLE, 128}, CITY  + NAVAREA},

	-- special areas
	-- bird taming
	--{"outlook1", -711, 100, {CIRCLE, 160}, NOSPAWNAREA + NAVAREA},

	-- spawns
	--{"world1", 0, 0, {CIRCLE, 3000}, SPAWNAREA, {"etyyy_world"}, 3},
	-- mouf
	{"mouf1", -1025, 674, {CIRCLE, 296}, SPAWNAREA, {"etyyy_mouf"}, 64},
	{"mouf2", -903, 520, {CIRCLE, 256}, SPAWNAREA, {"etyyy_mouf"}, 64},
	{"mouf3", -887, 868, {CIRCLE, 134}, SPAWNAREA, {"etyyy_mouf"}, 64},

	-- bantha
	{"bantha1", -296, -32, {CIRCLE, 300}, SPAWNAREA, {"etyyy_bantha"}, 64},
	{"bantha2", -393, 261, {CIRCLE, 300}, SPAWNAREA, {"etyyy_bantha"}, 64},

	-- walluga
	{"walluga1", -552, -574, {CIRCLE, 266}, SPAWNAREA, {"etyyy_walluga"}, 64},
	{"walluga2", -835, -666, {CIRCLE, 194}, SPAWNAREA, {"etyyy_walluga"}, 64},
	{"walluga3", -494, -765, {CIRCLE, 186}, SPAWNAREA, {"etyyy_walluga"}, 64},

	-- uller
	{"uller1", 672, -70, {CIRCLE, 240}, SPAWNAREA, {"etyyy_uller"}, 64},
	{"uller2", 873, 169, {CIRCLE, 300}, SPAWNAREA, {"etyyy_uller"}, 64},
	{"uller3", 1016, 503, {CIRCLE, 240}, SPAWNAREA, {"etyyy_uller"}, 64},

	-- webweaver
	{"webweaver1", 795, -367, {CIRCLE, 226}, SPAWNAREA, {"etyyy_webweaver"}, 64},
	{"webweaver2", 1083, -463, {CIRCLE, 344}, SPAWNAREA, {"etyyy_webweaver"}, 64},

	-- jedi punishment
	{"punish1", -777, -197, {CIRCLE, 64}, NAVAREA + NOSPAWNAREA},
}
