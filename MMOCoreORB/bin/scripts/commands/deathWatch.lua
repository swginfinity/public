-- Death Watch Faction Ability Command
-- Hidden faction-specific utility command available to Death Watch members with Box 3 skills
-- Infinity Custom Content

DeathWatchCommand = {
	name = "deathwatch",

	-- Instant command, no animation needed
	defaultTime = 0,

	-- Non-combat command
	addToCombatQueue = false,
}

AddCommand(DeathWatchCommand)
