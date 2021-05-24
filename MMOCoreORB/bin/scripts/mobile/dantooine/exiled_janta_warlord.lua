exiled_janta_warlord = Creature:new {
    customName = "an Exiled Janta Warlord",
    randomNameType = NAME_GENERIC,
    randomNameTag = true,
    socialGroup = "janta_tribe",
    faction = "janta_tribe",
    level = 100,
    chanceHit = 7.5,
    damageMin = 250,
    damageMax = 450,
    baseXp = 10000,
    baseHAM = 35000,
    baseHAMmax = 45000,
    armor = 2,
    resists = {20,30,30,60,30,20,60,30,10},
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
    creatureBitmask = PACK + HERD + KILLER,
    optionsBitmask = AIENABLED,
    diet = HERBIVORE,

    templates = {
        "object/mobile/dantari_male.iff",
        "object/mobile/dantari_female.iff"},
        lootGroups = {
            {
              groups = {
                    {group = "armor_attachments", chance = 2500000},
                    {group = "clothing_attachments", chance = 2500000},
                    {group = "janta_common", chance = 5000000},
                  }
            		}
            	},
    weapons = {"rebel_weapons_medium", "pirate_weapons_light"},
    conversationTemplate = "",
    attacks = merge(marksmanmaster,pistoleermaster,riflemanmaster,brawlermaster,fencermaster,swordsmanmaster)
}

CreatureTemplates:addCreatureTemplate(exiled_janta_warlord, "exiled_janta_warlord")
