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
		ONFIRE,
		{"resistance_fire", "fire_resist"},
		HEALTH,
		true,
		30,
		60,
		100,
		60,
		10,
		2
	  )
	},
	stateEffects = {
	  StateEffect(
		DIZZY_EFFECT,
		{},
		{},
		{ "jedi_state_defense" },
		65,
		0,
		10
	  )
	}
}

AddCommand(ForceLightningCone2Command)
