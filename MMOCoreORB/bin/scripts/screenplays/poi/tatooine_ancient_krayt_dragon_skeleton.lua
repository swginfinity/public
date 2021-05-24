AncientKraytSkeletonScreenPlay = ScreenPlay:new {
	numberOfActs = 1,

}

registerScreenPlay("AncientKraytSkeletonScreenPlay", true)

function AncientKraytSkeletonScreenPlay:start()
	if (isZoneEnabled("tatooine")) then
		self:spawnMobiles()
	end
end

function AncientKraytSkeletonScreenPlay:spawnMobiles()

	spawnMobile("tatooine", "krayt_dragon_grand", 800, -4555.0, 49.3, -4459.2, -116, 0)
	spawnMobile("tatooine", "krayt_dragon_grand", 800, -4840.0, 49, -4379.36, -116, 0)
	spawnMobile("tatooine", "krayt_dragon_ancient", 1000, -4713.1, 46.5, -4288.3, 50, 0)
	spawnMobile("tatooine", "krayt_dragon_ancient", 1000, -4762.78, 49, -4328.23, 50, 0)
	spawnMobile("tatooine", "giant_canyon_krayt_dragon", 800, -4669.8, 30.1, -4477.7, 5, 0)
	spawnMobile("tatooine", "canyon_krayt_dragon", 800, -4528.8, 28.3, -4302.4, 144, 0)
	spawnMobile("tatooine", "juvenile_canyon_krayt_dragon", 600, -4521.3, 27.2, -4298.2, 144, 0)
	spawnMobile("tatooine", "juvenile_canyon_krayt_dragon", 600, -4851.14, 45, -4402.69, 144, 0)
	spawnMobile("tatooine", "krayt_dragon_queen", 1200, -4754.9, 47, -4273.39, 144, 0)
	spawnMobile("tatooine", "krayt_dragon_adolescent", 600, -4747.2, 32.5, -4424.8, -91, 0)

end
