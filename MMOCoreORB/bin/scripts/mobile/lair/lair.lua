Lair = {
	mobiles = {}, -- mobile template, weighting
	bossMobiles = {}, -- mobile template, number to spawn
	bossMobileChance = 0,  -- Infinity:   Allows for setting bossMobile spawn chance per lair.  Note this value will be reduced by 50% for Destroy mission lairs.
	
	spawnLimit = 0,
	
	buildingsVeryEasy = {},
	buildingsEasy = {},
	buildingsMedium = {},
	buildingsHard = {},
	buildingsVeryHard = {},

	faction = "neutral",
	mobType = "creature",
	buildingType = "lair"
}

function Lair:new (o)
	o = o or { }
	setmetatable(o, self)
    self.__index = self
    return o
end
