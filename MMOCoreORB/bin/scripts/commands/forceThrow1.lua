ForceThrow1Command = {
    name = "forcethrow1",
    minDamage = 1500,
    maxDamage = 2500,
    speed = 1.0,
    forceCost = 28,
    visMod = 25,
    accuracyBonus = 95,
    stateEffects = {
        StateEffect(
            STUN_EFFECT,                                       	-- Effect Type
            {},                                                	-- defenderExclusionTimers
            {"stun_defense"},                                	-- defenderStateDefenseModifiers
            {"jedi_state_defense"},     						-- defenderJediStateDefenseModifiers
            80,                                                	-- stateChance
            0,                                                 	-- stateStrength
            10                                                 	-- stateLength
        ),
        StateEffect(
            DIZZY_EFFECT,                                       -- Effect Type
            {},                                                 -- defenderExclusionTimers
            {"dizzy_defense"},                                  -- defenderStateDefenseModifiers
            {"jedi_state_defense"},        						-- defenderJediStateDefenseModifiers
            80,                                                 -- stateChance
            0,                                                  -- stateStrength
            10                                                  -- stateLength
        ),
        StateEffect(
            BLIND_EFFECT,                                       -- Effect Type
            {},                                                 -- defenderExclusionTimers
            {"blind_defense"},                                -- defenderStateDefenseModifiers
            {"jedi_state_defense"},      						-- defenderJediStateDefenseModifiers
            80,                                                 -- stateChance
            0,                                                  -- stateStrength
            10                                                  -- stateLength
        ),
    },
    animation = "force_throw_1_particle_level_1",
    animType = GENERATE_INTENSITY,
    combatSpam = "forcethrow1",
    poolsToDamage = HEALTH_ATTRIBUTE + ACTION_ATTRIBUTE + MIND_ATTRIBUTE,
    forceAttack = true,
    damageType = BLAST_DAMAGE,
    range = 32
}

AddCommand(ForceThrow1Command)
