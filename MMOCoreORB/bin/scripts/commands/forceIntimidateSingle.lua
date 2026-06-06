ForceIntimidateSingleCommand = {
    name = "forceintimidatesingle",
    damageMultiplier = 0,
    speed = 0,
    forceCost = 20,
    accuracySkillMod = "intimidate",
    forceAttack = true,
    visMod = 25,
    stateEffects = {
        StateEffect(
            INTIMIDATE_EFFECT,                      -- Effect Type
            {},                                     -- defenderExclusionTimers
            {"intimidate_defense"},                 -- defenderStateDefenseModifiers
            {},                                     -- defenderJediStateDefenseModifiers
            100,                                    -- stateChance
            0,                                      -- stateStrength
            25                                      -- stateLength
        )
    },
    animation = "force_intimidate",
    combatSpam = "forceintimidatesingle",
    poolsToDamage = NO_ATTRIBUTE,
    range = 32
}

AddCommand(ForceIntimidateSingleCommand)
