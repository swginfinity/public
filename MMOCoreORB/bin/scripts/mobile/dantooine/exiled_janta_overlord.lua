exiled_janta_overlord = Creature:new {
    customName = "an Exiled Janta Overlord",
    randomNameType = NAME_GENERIC,
    randomNameTag = true,
    socialGroup = "janta_tribe",
    faction = "janta_tribe",
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

CreatureTemplates:addCreatureTemplate(exiled_janta_overlord, "exiled_janta_overlord")
