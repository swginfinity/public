ForceBreachCommand = {
	name = "forcebreach",

	damageMultiplier = 0,
	speed = 1,
	forceCost = 125,
	accuracyBonus = 295,
	forceAttack = true,
	visMod = 100,

	stateEffects = {
	  StateEffect(
		FORCE_BREACH_EFFECT,
		{},
		{},
		{},
		200,
		0,
		10
	  )
	},

	animation = "force_intimidate",

	combatSpam = "forcebreach",

	poolsToDamage = NO_ATTRIBUTE,

	range = 32
}

AddCommand(ForceBreachCommand)
