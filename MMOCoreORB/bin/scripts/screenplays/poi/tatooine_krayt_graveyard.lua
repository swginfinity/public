KraytGraveyardScreenPlay = ScreenPlay:new {
	numberOfActs = 1,

}

registerScreenPlay("KraytGraveyardScreenPlay", true)

function KraytGraveyardScreenPlay:start()
	if (isZoneEnabled("tatooine")) then
		self:spawnMobiles()
	end
end

function KraytGraveyardScreenPlay:spawnMobiles()

	-- Group at entrance
	spawnMobile("tatooine", "canyon_krayt_dragon", 780, 6967.97, 13.6389, 4378.2, -110, 0)
	spawnMobile("tatooine", "giant_canyon_krayt_dragon", 900, 6988.49, 18.5257, 4345.14, -69, 0)

	-- Duo at First Krayt Skeleton in Entrance Canyon
	spawnMobile("tatooine", "krayt_dragon_adolescent", 660, 7182.92, 21.8043, 4451.46, 64, 0)
	spawnMobile("tatooine", "giant_canyon_krayt_dragon", 900, 7213.06, 25.197, 4466.2, -122, 0)

	-- Second Skeleton at entrance canyon
	spawnMobile("tatooine", "canyon_krayt_dragon", 780, 7271.48, 25.7028, 4469.77, -137, 0)
	spawnMobile("tatooine", "krayt_dragon_adolescent", 660, 7250.68, 25.5734, 4462.58, 46, 0)

	-- On hill up from the second Skeleton near entrance
	spawnMobile("tatooine", "juvenile_canyon_krayt_dragon", 600, 7287.19, 35.1549, 4429.04, -97, 0)

	-- First main mob guarding entrance
	spawnMobile("tatooine", "krayt_dragon_grand", 1200, 7323.71, 19.0917, 4476.63, -107, 0)

	-- First group inside main Canyon area
	spawnMobile("tatooine", "juvenile_canyon_krayt_dragon", 600, 7396.22, 8.82042, 4563.7, 26, 0)
	spawnMobile("tatooine", "canyon_krayt_dragon", 780, 7406.37, 9.50683, 4609.91, 151, 0)
	spawnMobile("tatooine", "juvenile_canyon_krayt_dragon", 600, 7442.19, 7.46604, 4595.92, -112, 0)

	-- Flat area NE of First Group
	spawnMobile("tatooine", "krayt_dragon_ancient", 1000, 7514.03, 8.2983, 4565.24, -114, 0)

	-- Queen, Random respawn of 40minutes to 60minutes
	spawnMobile("tatooine", "krayt_dragon_queen", 1200, 7501.67, 8.16027, 4738.07, -168, 0)

	--Near Queen, in Ridges
	spawnMobile("tatooine", "juvenile_canyon_krayt_dragon", 600, 7521.98, 2.79673, 4738.25, -114, 0)
	spawnMobile("tatooine", "krayt_dragon_adolescent", 660, 7467.26, 1.27853, 4766.53, 173, 0)
	spawnMobile("tatooine", "juvenile_canyon_krayt_dragon", 600, 7519.91, 3.0781, 4826.45, -114, 0)

	--Left of Queen
	spawnMobile("tatooine", "krayt_dragon_adolescent", 660, 7571.65, 16.7122, 4720.35, 109, 0)

	--Behind Queen
	spawnMobile("tatooine", "krayt_dragon_ancient", 1000, 7535.58, 7.61307, 4789.4, 128, 0)

	--In front of Queen
	spawnMobile("tatooine", "krayt_dragon_adolescent", 660, 7463.36, 5.4316, 4692, 119, 0)

	--Adolescent Feeding Group
	spawnMobile("tatooine", "krayt_dragon_adolescent", 660, 7489.27, 7.11111, 4508.67, -28, 0)
	spawnMobile("tatooine", "krayt_dragon_adolescent", 660, 7461.5, 6.72755, 4516.87, 39, 0)
	spawnMobile("tatooine", "juvenile_canyon_krayt_dragon", 600, 7429.2, 6.07662, 4544.86, 83, 0)

	--Back Wall
	spawnMobile("tatooine", "krayt_dragon_ancient", 1000, 7548.54, 19.0047, 4440.38, -41, 0)
	spawnMobile("tatooine", "krayt_dragon_grand", 1200, 7493.09, 26.8857, 4400.18, -17, 0)

	--Guarding Juves and Adoles
	spawnMobile("tatooine", "giant_canyon_krayt_dragon", 900, 7372.06, 21.7032, 4606.04, 11, 0)
	spawnMobile("tatooine", "krayt_dragon_ancient", 1000, 7382, 28.6055, 4432.14, 107, 0)
	spawnMobile("tatooine", "giant_canyon_krayt_dragon", 900, 7632.54, 56.2281, 4566.58, 6, 0)
	spawnMobile("tatooine", "giant_canyon_krayt_dragon", 900, 7649.13, 44.8928, 4748.37, -168, 0)

	-- Juvies hiding
	spawnMobile("tatooine", "juvenile_canyon_krayt_dragon", 600, 7510.05, 0.288336, 4586.63, -87, 0)
	spawnMobile("tatooine", "juvenile_canyon_krayt_dragon", 600, 7468.03, -0.534591, 4627.72, 107, 0)
	spawnMobile("tatooine", "canyon_krayt_dragon", 780, 7521.24, 0.397643, 4615.58, 158, 0)

	--Giant Canyon Krayts
	spawnMobile("tatooine", "giant_canyon_krayt_dragon", 900, 7444.37, 7.44919, 4754.94, -149, 0)
	spawnMobile("tatooine", "giant_canyon_krayt_dragon", 900, 7550.78, 10.2386, 4654.99, -3, 0)
	spawnMobile("tatooine", "giant_canyon_krayt_dragon", 900, 7477.68, 4.1576, 4643.93, 84, 0)

	-- Ancient Watching over kids
	spawnMobile("tatooine", "krayt_dragon_ancient", 1000, 7442.93, 7.30235, 4574.65, -126, 0)

	-- Rear Entrance Guard
	spawnMobile("tatooine", "giant_canyon_krayt_dragon", 900, 7506.3, 13.606, 4897.76, 15, 0)
	spawnMobile("tatooine", "krayt_dragon_grand", 1200, 7535.71, 11.9171, 4883.34, 4, 0)
	spawnMobile("tatooine", "giant_canyon_krayt_dragon", 900, 7577.5, 20.9797, 4857.89, -7, 0)

end
