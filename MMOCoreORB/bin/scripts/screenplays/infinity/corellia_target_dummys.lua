local ObjectManager = require("managers.object.object_manager")

corelliaTargetDummys = ScreenPlay:new {
	numberOfActs = 1,

	screenplayName = "corelliaTargetDummys",

	coronetDummys = {
		{"tcg_target_dummy", 0, -187.303, 28, 4747.72, 270, 0 },
		{"tcg_target_dummy", 0, -187.303, 28, 4752.72, 270, 0 },
		{"tcg_target_dummy", 0, -187.303, 28, 4757.72, 270, 0 }
	}
}

registerScreenPlay("corelliaTargetDummys", true)

function corelliaTargetDummys:start()
	if (isZoneEnabled("corellia")) then
		self:spawnMobiles()
	end
end

function corelliaTargetDummys:spawnMobiles()
	local mobileTable = self.coronetDummys
		for i = 1, #mobileTable, 1 do
			local pMobile = spawnMobile("corellia", mobileTable[i][1], mobileTable[i][2], mobileTable[i][3], 0)
				if (pMobile ~= nil and CreatureObject(pMobile):isAiAgent()) then
					AiAgent(pMobile):setAiTemplate("targetdummy")
				end
	end
end
