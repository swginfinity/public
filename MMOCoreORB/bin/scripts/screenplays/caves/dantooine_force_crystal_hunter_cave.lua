ForceCrystalCaveScreenPlay = ScreenPlay:new {
	numberOfActs = 1,

	screenplayName = "ForceCrystalCaveScreenPlay",

	lootContainers = {
		200335,
		200336,		
		8535511			
	},
	
	lootLevel = 110,	

	-- Create chance that three items can drop from a single chest every 15minutes!
	lootGroups = {
		{
			groups = {
				{group = "junk", chance = 4000000},
				{group = "armor_attachments", chance = 3000000},
				{group = "color_crystals", chance = 2000000},
				{group = "clothing_attachments", chance = 1000000}
			},
			lootChance = 8500000
		},
		{
			groups = {
				{group = "junk", chance = 4000000},
				{group = "armor_attachments", chance = 3000000},
				{group = "color_crystals", chance = 2000000},
				{group = "clothing_attachments", chance = 1000000}
			},
			lootChance = 8500000
		},
		{
			groups = {
				{group = "junk", chance = 4000000},
				{group = "armor_attachments", chance = 3000000},
				{group = "color_crystals", chance = 2000000},
				{group = "clothing_attachments", chance = 1000000}
			},
			lootChance = 8500000
		},			
	},
	
	lootContainerRespawn = 900 -- 15 minutes
}

registerScreenPlay("ForceCrystalCaveScreenPlay", true)

function ForceCrystalCaveScreenPlay:start()
	if (isZoneEnabled("dantooine")) then
		self:spawnMobiles()
		self:initializeLootContainers()
	end
end

function ForceCrystalCaveScreenPlay:spawnMobiles()   

	--[8535483]
	spawnMobile("dantooine", "untrained_wielder_of_the_dark_side",360, -14.9517, -4.82356, -8.96425, 145, 8535483)
	spawnMobile("dantooine", "force_crystal_hunter",360, -12.737, -5.57787, -10.5169, -80, 8535483)
	spawnMobile("dantooine", "untrained_wielder_of_the_dark_side",360, 8.78807, -20.3257, -5.01239, 80, 8535483)
	spawnMobile("dantooine", "force_crystal_hunter",360, 12.3659, -22.2289, -5.27407, -68, 8535483)
	spawnMobile("dantooine", "untrained_wielder_of_the_dark_side",360, 26.8569, -27.144, -7.27365, -19, 8535483)
	spawnMobile("dantooine", "force_crystal_hunter",360, 23.5215, -25.8269, -5.08516, 88, 8535483)
	spawnMobile("dantooine", "force_crystal_hunter",360, 20.5951, -28.6512, -12.5748, -167, 8535483)
	spawnMobile("dantooine", "untrained_wielder_of_the_dark_side",360, 20.5951, -28.6512, -12.5748, 98, 8535483)

	--[8535484]
	spawnMobile("dantooine", "untrained_wielder_of_the_dark_side",360, 21.4909, -39.3121, -41.5174, 2, 8535484)
	spawnMobile("dantooine", "force_crystal_hunter",360, 26.0456, -39.3523, -40.1884, -2, 8535484)
	spawnMobile("dantooine", "untrained_wielder_of_the_dark_side",360, 27.0579, -41.7332, -59.7803, 170, 8535484)
	spawnMobile("dantooine", "untrained_wielder_of_the_dark_side",360, 20.1977, -41.056, -70.6279, 0, 8535484)
	spawnMobile("dantooine", "untrained_wielder_of_the_dark_side",360, 49.0744, -48.7665, -62.4055, 67, 8535484)
	spawnMobile("dantooine", "untrained_wielder_of_the_dark_side",360, 52.3704, -48.7408, -63.8327, -31, 8535484)
	spawnMobile("dantooine", "force_crystal_hunter",360, 47.2703, -48.1271, -57.1334, 141, 8535484)
	spawnMobile("dantooine", "force_crystal_hunter_captain",360, 51.635, -48.8996, -55.2345, 97, 8535484) --make strong!
	spawnMobile("dantooine", "force_crystal_hunter_captain",360, 46.0542, -47.1159, -8.22541, -14, 8535484) --make strong
	spawnMobile("dantooine", "force_crystal_hunter",360, 48.3683, -47.0387, -7.30779, -114, 8535484)
	spawnMobile("dantooine", "force_crystal_hunter",360, 47.5006, -47.2927, -14.2866, -39, 8535484)
	spawnMobile("dantooine", "untrained_wielder_of_the_dark_side",360, 53.9746, -49.1282, -7.57905, 20, 8535484)
	spawnMobile("dantooine", "untrained_wielder_of_the_dark_side",360, 56.7722, -68.5288, -41.6701, -174, 8535484)
	spawnMobile("dantooine", "force_crystal_hunter",360, 52.413, -67.8657, -46.8491, 167, 8535484)
	spawnMobile("dantooine", "force_crystal_hunter",360, 53.1561, -68.1378, -39.9971, -9, 8535484)
	spawnMobile("dantooine", "untrained_wielder_of_the_dark_side",360, 54.039, -68.0285, -36.4378, -134, 8535484)

	--[8535485]
	spawnMobile("dantooine", "force_crystal_hunter",360, 88.531, -60.6509, -6.90809, 74, 8535485)
	spawnMobile("dantooine", "untrained_wielder_of_the_dark_side",360, 91.5983, -61.7219, -9.5336, 24, 8535485)
	spawnMobile("dantooine", "force_crystal_hunter",360, 94.2485, -61.5897, -10.4592, -5, 8535485)
	spawnMobile("dantooine", "untrained_wielder_of_the_dark_side",360, 96.0409, -63.2504, -26.9507, 147, 8535485)


	--[8535486]
	spawnMobile("dantooine", "force_crystal_hunter",360, 88.6484, -76.8239, -63.7984, -50, 8535486)
	spawnMobile("dantooine", "untrained_wielder_of_the_dark_side",360, 82.1368, -76.4444, -64.9824, 173, 8535486)
	spawnMobile("dantooine", "force_crystal_hunter_captain",360, 64.3964, -75.3098, -58.6522, -70, 8535486) -- make strong!
	spawnMobile("dantooine", "force_crystal_hunter",360, 65.9405, -75.3107, -61.9422, 22, 8535486)
	spawnMobile("dantooine", "force_crystal_hunter",360, 67.0266, -75.4522, -59.1309, -156, 8535486)
	spawnMobile("dantooine", "untrained_wielder_of_the_dark_side",360, 89.7134, -76.3508, -85.9711, 35, 8535486)
	spawnMobile("dantooine", "force_crystal_hunter",360, 93.4897, -76.0432, -83.2506, 125, 8535486)
	spawnMobile("dantooine", "force_crystal_hunter",360, 64.6952, -76.7732, -88.2343, -91, 8535486)
	spawnMobile("dantooine", "force_crystal_hunter",360, 61.0722, -76.0911, -81.4163, -150, 8535486)
end