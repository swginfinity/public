ForceLightningCone2Command = {
	name = "forcelightningcone2",
	minDamage = 4200,
	maxDamage = 5600,
	speed = 1,
	forceCost = 200,
	accuracyBonus = 95,
	coneAngle = 60,
	coneAction = true,
	visMod = 25,
	animation = "force_lightning_1_arc_particle_level_3",
	animType = GENERATE_INTENSITY,
	combatSpam = "forcelightningcone2",
	poolsToDamage = RANDOM_ATTRIBUTE,
	forceAttack = true,
	damageType = ELECTRICITY_DAMAGE,
	range = 32,
	dotEffects = {
		DotEffect(
			ONFIRE,                                             -- DOT Type
			{"resistance_fire", "fire_resist"},                 -- defenderStateDefenseModifiers
			HEALTH,                                             -- DOT Pool
			true,                                               -- dotDamageOfHit
			30,                                                 -- dotPotency
			60,                                                 -- dotChance
			0,                                                  -- dotStrength (not used if dotDamageOfHit = true)
			60,                                                 -- dotDuration
			10,                                                 -- primaryPercent, this is primary damage DOT on health pool
			2                                                   -- secondaryPercent, this is the secondary effect on wounds, which is only used by fire DOT
		)
	},
	stateEffects = {
		StateEffect(
			DIZZY_EFFECT,                                       -- Effect Type
			{},                                                 -- defenderExclusionTimers
			{"dizzy_defense"},                                -- defenderStateDefenseModifiers
			{"jedi_state_defense"},      						-- defenderJediStateDefenseModifiers
			100,                                                -- stateChance
			0,                                                  -- stateStrength
			10                                                  -- stateLength
		),
	}
}

AddCommand(ForceLightningCone2Command)
