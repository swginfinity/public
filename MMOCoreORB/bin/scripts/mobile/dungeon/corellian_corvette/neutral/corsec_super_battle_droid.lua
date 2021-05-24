corsec_super_battle_droid = Creature:new {
  objectName = "@mob/creature_names:rebel_super_battle_droid",
  socialGroup = "corsec",
  faction = "corsec",
  level = 200,
  chanceHit = 18,
  damageMin = 1200,
  damageMax = 2300,
  baseXp = 19000,
  baseHAM = 230000,
  baseHAMmax = 230000,
  armor = 2,
  resists = {85,95,100,60,100,25,40,85,-1},--kinetic,energy,blast,heat,cold,electric,acid,stun,ls
  meatType = "",
  meatAmount = 0,
  hideType = "",
  hideAmount = 0,
  boneType = "",
  boneAmount = 0,
  milk = 2 * 0,
  tamingChance = 0,
  ferocity = 0,
  pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
  creatureBitmask = KILLER,
  optionsBitmask = AIENABLED,
  diet = NONE,
  scale = 1.40,

  templates = {
    "object/mobile/super_battle_droid.iff",
  },
	lootGroups = {
		{
			groups = {
				{group = "death_watch_bunker_commoners", chance = 6000000},   -- 25% * 60% = 15%
				{group = "stap_speeder", chance = 4000000}     -- 25% * 40% = 10%
			},
			lootChance = 2500000    -- 25% chance of this group
		}
	},
  conversationTemplate = "",
  defaultWeapon = "object/weapon/ranged/droid/droid_droideka_ranged.iff",
  defaultAttack = "creaturerangedattack"
}

CreatureTemplates:addCreatureTemplate(corsec_super_battle_droid, "corsec_super_battle_droid")
