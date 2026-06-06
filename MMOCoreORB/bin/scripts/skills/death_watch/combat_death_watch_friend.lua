--[[
	Friends of Greef Skill

	Granted to non-Death Watch service providers (doctors, entertainers, weaponsmiths)
	who pay Greef for the privilege of serving Death Watch members.

	This skill:
	- Grants access to all 4 faction HQs (like Death Watch members)
	- Provides the title "a Friend of Greef"
	- Blocks faction recruiter access (Imperial/Rebel)
	- Blocks Jedi shrine meditation
	- Does NOT grant DW combat abilities or bounty hunting access
]]

combat_death_watch_friend = {
	skillName = "combat_death_watch_friend",
	parentName = "",
	graphType = 0,
	godOnly = 0,
	title = 1,
	profession = 0,
	hidden = 1,
	moneyRequired = 0,
	pointsRequired = 0,
	skillsRequiredCount = 0,
	skillsRequired = {},
	preclusionSkills = {
		"combat_death_watch_novice"  -- Cannot be both DW member and Friend
	},
	xpType = "",
	xpCost = 0,
	xpCap = 0,
	missionsRequired = {},
	apprenticeshipsRequired = 0,
	statsRequired = {},
	speciesRequired = {},
	jediStateRequired = 0,
	skillAbility = {},
	commands = {},
	skillModifiers = {},
	schematicsGranted = {},
	schematicsRevoked = {},
	searchable = 0
}

addSkill(combat_death_watch_friend)
