--Copyright (C) 2007 <SWGEmu>
--Infinity: self-service NPC unstick command. See UnstickTargetCommand.h.

UnstickTargetCommand = {
    name = "unsticktarget",
    -- Tunables — edit and /loadluascripts, no rebuild required.
    -- Staff "force" mode inside the C++ command ignores these.
    cooldown = 60000,                                              -- 60 seconds, enforced by command queue
    cooldownString = "You can only unstick a creature once every 60 seconds.",
    maxRangeToTarget = 6                                           -- 6 meters, rejected before reaching C++
}

AddCommand(UnstickTargetCommand)
