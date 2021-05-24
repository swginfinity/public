death_watch_s_battle_droid = Creature:new {
	objectName = "@mob/creature_names:mand_bunker_super_battle_droid",
	socialGroup = "death_watch",
	faction = "",
	level = 300,
	chanceHit = 1.8,
	damageMin = 1080,
	damageMax = 1890,
	baseXp = 19000,
	baseHAM = 172500,
	baseHAMmax = 323000,
	armor = 2,
	resists = {65,75,80,50,80,25,40,75,15},--kinetic,energy,blast,heat,cold,electric,acid,stun,ls
	meatType = "",
	meatAmount = 0,
	hideType = "",
	hideAmount = 0,
	boneType = "",
	boneAmount = 0,
	milk = 0,
	tamingChance = 0,
	ferocity = 0,
	pvpBitmask = AGGRESSIVE + ATTACKABLE + ENEMY,
	creatureBitmask = KILLER,
	optionsBitmask = AIENABLED,
	diet = NONE,
	scale = 1.40,

	templates = {
		"object/mobile/death_watch_s_battle_droid.iff",
		"object/mobile/death_watch_s_battle_droid_02.iff",
		"object/mobile/death_watch_s_battle_droid_03.iff"},
	lootGroups = {
		{
			groups = {
				{group = "death_watch_bunker_commoners", chance = 6500000},
				{group = "stap_speeder", chance = 3500000}     -- 25% * 35% = 8.75%
			},
			lootChance = 2500000    -- 25% chance of this group
		},
		{
			groups = {
				{group = "armor_attachments", chance = 10000000},
			},
			lootChance = 1000000
		}

	},
	conversationTemplate = "",
	defaultWeapon = "object/weapon/ranged/droid/droid_droideka_ranged.iff",
	defaultAttack = "attack"
}

CreatureTemplates:addCreatureTemplate(death_watch_s_battle_droid, "death_watch_s_battle_droid")
