exiled_janta_overlord = Creature:new {
    customName = "an Exiled Janta Overlord",
    randomNameType = NAME_GENERIC,
    randomNameTag = true,
    socialGroup = "janta_tribe",
    faction = "janta_tribe",
    mobType = MOB_NPC,
    level = 150,
    chanceHit = 7,
    damageMin = 450,
    damageMax = 850,
    baseXp = 15000,
    baseHAM = 50000,
    baseHAMmax = 65000,
    armor = 2,
    resists = {50,35,50,35,35,50,35,40,10},
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
    creatureBitmask = PACK + HERD + KILLER,
    optionsBitmask = AIENABLED,
    diet = HERBIVORE,

    templates = {
        "object/mobile/dantari_male.iff",
        "object/mobile/dantari_female.iff"
    },
    lootGroups = {
        {
            groups = {
                {group = "armor_attachments", chance = 2500000},
                {group = "clothing_attachments", chance = 2500000},
                {group = "janta_common", chance = 5000000},
            }
        }
    },
	-- Primary and secondary weapon should be different types (rifle/carbine, carbine/pistol, rifle/unarmed, etc)
	-- Unarmed should be put on secondary unless the mobile doesn't use weapons, in which case "unarmed" should be put primary and "none" as secondary
	primaryWeapon = "pirate_weapons_light",
	secondaryWeapon = "none",
	conversationTemplate = "",
	
	-- primaryAttacks and secondaryAttacks should be separate skill groups specific to the weapon type listed in primaryWeapon and secondaryWeapon
	-- Use merge() to merge groups in creatureskills.lua together. If a weapon is set to "none", set the attacks variable to empty brackets
	primaryAttacks = merge(marksmanmaster,pistoleermaster,riflemanmaster,brawlermaster,fencermaster,swordsmanmaster),
	secondaryAttacks = { },
}

CreatureTemplates:addCreatureTemplate(exiled_janta_overlord, "exiled_janta_overlord")
