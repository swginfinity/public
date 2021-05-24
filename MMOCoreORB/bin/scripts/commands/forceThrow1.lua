ForceThrow1Command = {
	name = "forcethrow1",

	minDamage = 375,
	maxDamage = 500,
	speed = 1.0,
	forceCost = 28,
	visMod = 25,
	accuracyBonus = 95,

	stateEffects = {
	  StateEffect(
		STUN_EFFECT,
		{},
		{ "stun_defense", "resistance_states" }, 
		{ "jedi_state_defense" },
		65,
		0,
		10
	  ),
	    StateEffect(
		DIZZY_EFFECT,
		{},
		{ "dizzy_defense", "resistance_states" }, 
		{ "jedi_state_defense" }, 
		65,
		0,
		10
	  ),
	  	StateEffect(
		BLIND_EFFECT,
		{}, 
		{ "blind_defense", "resistance_states" }, 
		{ "jedi_state_defense" }, 
		65,
		0,
		10
	  )
	},

	animation = "force_throw_1_particle_level_1",
	animType = GENERATE_INTENSITY,

	combatSpam = "forcethrow1",

	poolsToDamage = HEALTH_ATTRIBUTE + ACTION_ATTRIBUTE + MIND_ATTRIBUTE,

	forceAttack = false,
	damageType = BLAST_DAMAGE,

	range = 32
}

AddCommand(ForceThrow1Command)
