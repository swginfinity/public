-- Custom-content spawn groups that vanilla does NOT define.
-- These are referenced by planet region files (managers/planet/<planet>_regions.lua)
-- but their definitions live only here. Without this include, the region
-- spawn-group lookups silently fail at runtime and nothing spawns in those areas.
--
-- IMPORTANT: This file deliberately does NOT include the *_world.lua overrides.
-- Those files would clobber the vanilla world spawn groups (loaded via
-- spawn/serverobjects.lua) and strip out the global_imperial_*, global_rebel_*,
-- and global_dark_jedi_* military/jedi spawns that the vanilla world groups
-- carry. See plans/jtl-spawn-quest-regressions.md for the full audit.

includeFile("custom_content/spawn/tatooine_jawa_krayt_traders.lua")
includeFile("custom_content/spawn/tatooine_krayt_valley.lua")
includeFile("custom_content/spawn/tatooine_pelgo_surveyors.lua")
includeFile("custom_content/spawn/peko_albatross.lua")
