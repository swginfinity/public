-- Dedicated dynamic albatross herd spawn group - rare CL140 flock.
-- weighting 3 = rare relative to region neighbors; spawn cap of 8 lives on the
-- lair template (naboo_peko_albatross_herd_neutral_none, spawnLimit = 8).
-- Bound to Naboo region(s) via managers/planet/naboo_regions.lua.
peko_albatross = {

	lairSpawns = {
		{
			lairTemplateName = "naboo_peko_albatross_herd_neutral_none",
			spawnLimit = -1,
			minDifficulty = 140,
			maxDifficulty = 200,
			numberToSpawn = 8,
			weighting = 3,
			size = 25
		}
	}
}

addSpawnGroup("peko_albatross", peko_albatross);
