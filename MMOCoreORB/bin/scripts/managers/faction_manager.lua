--Factional Relationships determine enemies and allies of factions.
--If you kill a factional NPC, and it is allied with another faction, then you also lose faction with the ally.
--If you kill a factional NPC, and it is enemies with another faction, then you gain faction with the enemy.

-- { faction,playerAllowed,enemies,allies,adjustFactor },
--	faction       - This is the string key faction that the relationship describes.
-- playerAllowed - Player's are able to earn or lose points with this faction.
-- enemies       - This is a comma delimited list of string key factions that this faction is enemies with.
-- allies        - This is a comma delimited list of string key factions that this faction is allies with.
-- adjustFactor  - This factor affects the amount of faction gained/lost when killing a mobile

factionList = {
	{ "rebel", true, "imperial", "", 1.0 },
	{ "imperial", true, "rebel", "", 1.0 },
	{ "aakuans", true, "binayre,fed_dub,liberation_party", "", 1.0 },
	{ "afarathu", true, "corsec", "", 1.0 },
	{ "alkhara", true, "tusken_raider", "", 1.0 },
	{ "bandit", true, "townsperson,infinite_llc", "", 1.0 },
	{ "beldonnas_league", true, "followers_of_lord_nyax,lost_aqualish", "corsec", 1.0 },
	{ "binayre", true, "aakuans,corsec,fed_dub", "", 1.0 },
	{ "bloodrazor", true, "canyon_corsair,nym", "", 1.0 },
	{ "borvo", true, "gungan,jabba,trade_federation", "", 1.0 },
	{ "canyon_corsair", true, "bloodrazor,nym", "", 1.0 },
	{ "cobral", true, "restuss", "", 1.0 },
	{ "cor_swoop", true, "smashball", "", 1.0 },
	{ "corsec", true, "afarathu,followers_of_lord_nyax,monumenter,rogue_corsec,lost_aqualish", "beldonnas_league", 1.0 },
	{ "dantari_raiders", true, "kunga_tribe,mokk_tribe,janta_tribe", "", 1.0 },
	{ "desert_demon", true, "swoop", "", 1.0 },
	{ "donkuwah_tribe", true, "gondula_tribe,panshee_tribe", "", 1.0 },
	{ "drall", true, "corsec,fed_dub", "", 1.0 },
	{ "endor_marauder", true, "gondula_tribe,panshee_tribe", "", 1.0 },
	{ "fed_dub", true, "binayre,drall,liberation_party,lost_aqualish", "", 1.0 },
	{ "flail", true, "hidden_daggers", "", 1.0 },
	{ "followers_of_lord_nyax", true, "beldonnas_league,corsec", "", 1.0 },
	{ "fs_villager", true, "sith_shadow,sith_shadow_nonaggro", "", 0.0 },
	{ "garyn", true, "restuss,nym", "", 1.0 },
	{ "gondula_tribe", true, "donkuwah_tribe,korga_tribe,pubam,endor_marauder", "panshee_tribe", 1.0 },
	{ "gungan", true, "borvo,plasma_thief,swamp_rat", "", 1.0 },
	{ "hidden_daggers", true, "beldonnas_league,corsec,flail", "rogue_corsec", 1.0 },
	{ "hutt", true, "naboo_security_force,narmle,nym,corsec,imperial", "jabba,borvo", 1.0 },
	{ "jabba", true, "borvo,valarian", "hutt", 1.0 },
	{ "janta_tribe", true, "kunga_tribe,dantari_raiders", "", 1.0 },
	{ "jawa", true, "tusken_raider", "", 1.0 },
	{ "kobola", true, "narmle,spice_collective", "", 1.0 },
	{ "korga_tribe", true, "gondula_tribe,panshee_tribe", "", 1.0 },
	{ "kunga_tribe", true, "dantari_raiders,mokk_tribe", "", 1.0 },
	{ "liberation_party", true, "corsec,fed_dub", "", 1.0 },
	{ "lok_mercenaries", true, "bloodrazor,canyon_corsair", "", 1.0 },
	{ "lost_aqualish", true, "beldonnas_league,corsec,fed_dub", "", 1.0 },
	{ "meatlump", true, "beldonnas_league,corsec,rogue_corsec", "", 1.0 },
	{ "mokk_tribe", true, "dantari_raiders,janta_tribe", "", 1.0 },
	{ "monumenter", true, "beldonnas_league,corsec,infinite_llc", "", 1.0 },
	{ "mtn_clan", true, "nightsister", "", 1.0 },
	{ "naboo", true, "borvo", "", 1.0 },
	{ "naboo_pirate", true, "naboo_security_force", "", 1.0 },
	{ "naboo_security_force", true, "borvo,naboo_pirate,plasma_thief,swamp_rat,trade_federation", "", 1.0 },
	{ "narmle", true, "kobola,spice_collective", "restuss", 1.0 },
	{ "nightsister", true, "mtn_clan,imperial", "", 1.0 },
	{ "nym", true, "bloodrazor,canyon_corsair,garyn", "", 1.0 },
	{ "olag_greck", true, "beldonnas_league,corsec", "", 1.0 },
	{ "panshee_tribe", true, "donkuwah_tribe,korga_tribe,pubam,endor_marauder", "gondula_tribe", 1.0 },
	{ "pirate", true, "infinite_llc", "", 1.0 },
	{ "plasma_thief", true, "gungan", "", 1.0 },
	{ "pubam", true, "gondula_tribe,panshee_tribe", "", 1.0 },
	{ "restuss", true, "cobral,garyn", "narmle", 1.0 },
	{ "rogue_corsec", true, "corsec", "hidden_daggers", 1.0 },
	{ "rorgungan", true, "spice_collective", "", 1.0 },
	{ "sif", true, "rebel,imperial", "hutt", 1.0 },
	{ "sith_shadow", true, "fs_villager", "rebel,imperial", 0.0 },
	{ "sith_shadow_nonaggro", true, "fs_villager", "rebel,imperial", 0.0 },
	{ "smashball", true, "cor_swoop,corsec", "", 1.0 },
	{ "spice_collective", true, "narmle,kobola,rorgungan", "", 1.0 },
	{ "spider_nightsister", true, "mtn_clan", "", 1.0 },
	{ "swamp_rat", true, "gungan", "", 1.0 },
	{ "swoop", true, "desert_demon,infinite_llc", "", 1.0 },
	{ "thug", true, "townsperson", "", 1.0 },
	{ "townsperson", true, "bandit,thug", "rebel,imperial", 1.0 },
	{ "trade_federation", true, "borvo,naboo_security_force", "", 1.0 },
	{ "tusken_raider", true, "alkhara,jawa", "", 1.0 },
	{ "valarian", true, "jabba", "", 1.0 },
	
	-- etyyy
	{ "arcona_hunters", true, "hracca,rodian_hunters", ",sinisters,blackscale_elite", 1.0 },
	{ "blackscale_elite", true, "hracca,sinisters,", "rodian_hunters,arcona_hunters, blackscale", 1.0 },
	{ "hracca", true, "arcona_hunters,blackscale_elite", "rodian_hunters,sinisters", 1.0 },
	{ "rodian_hunters", true, "sinisters,arcona_hunters", "blackscale_elite,hracca", 1.0 },
	{ "sinisters", true, "rodian_hunters,blackscale_elite", "arcona_hunters,hracca", 1.0 },
	
	-- custom
	{ "gorax", true, "gondula_tribe,panshee_tribe", "", 1.0 },
	{ "infinite_llc", true, "pirate,swoop,monumenter", "", 1.0 },


	-- mandalorian
	{ "mandalorian", true, "ohnaka_clan,dust_vipers", "omera_fisherman", .1 },
	{ "klatoonian", true, "", "", .1 },
	{ "night_owls", true, "blackscale,dust_vipers", "", .1 },
	{ "miners_guild", true, "", "", .1 },
	{ "bounty_hunter_guild", true, "", "", .1 },
	{ "blackscale", true, "night_owls", "", .1 },
	{ "dust_vipers", true, "night_owls", "", .1 },
	{ "omera_fisherman", true, "ohnaka_clan", "mandalorian", .1 },
	{ "ohnaka_clan", true, "omera_fisherman,mandalorian", "dust_vipers", 1 },
	
	-- do not, under any circumstance, edit the map below
	-- big 4
	{ "black_krrsantans", true, "deathwatchhunter,xonti_brothers,kleef,darkwalkers", "kanjiklub,corellian_exchange,crymorah_syndicate,pyke_syndicate", .4 },
	{ "xonti_brothers", true, "deathwatchhunter,black_krrsantans,corellian_exchange,crymorah_syndicate", "darkwalkers,guavian_death_gang,crimson_dawn,kleef", .4 },

	{ "blacksun", true, "deathwatchhunter,zann_consortium,pyke_syndicate,crimson_dawn", "kanjiklub,guavian_death_gang,crymorah_syndicate,kleef", .4 },
	{ "zann_consortium", true, "deathwatchhunter,blacksun,guavian_death_gang,kanjiklub", "darkwalkers,corellian_exchange,crimson_dawn,pyke_syndicate", .4 },
	-- lesser 8
	{ "darkwalkers", true, "black_krrsantans,crimson_dawn,crymorah_syndicate", "xonti_brothers,kanjiklub", .60 },	
	{ "kanjiklub", true, "zann_consortium,crimson_dawn,crymorah_syndicate", "blacksun,darkwalkers", .60 },
	
	{ "guavian_death_gang", true, "zann_consortium,kleef,pyke_syndicate", "blacksun,corellian_exchange", .60 },
	{ "corellian_exchange", true, "xonti_brothers,kleef,pyke_syndicate", "black_krrsantans,guavian_death_gang", .60 },
	
	{ "crymorah_syndicate", true, "xonti_brothers,darkwalkers,kanjiklub", "black_krrsantans,crimson_dawn", .60 },
	{ "crimson_dawn", true, "blacksun,darkwalkers,kanjiklub", "zann_consortium,crymorah_syndicate", .60 },

	{ "pyke_syndicate", true, "blacksun,guavian_death_gang,corellian_exchange", "zann_consortium,kleef", .60 },
	{ "kleef", true, "black_krrsantans,guavian_death_gang,corellian_exchange", "xonti_brothers,pyke_syndicate", .60 },

	{"deathwatchhunter", true, "black_krrsantans,xonti_brothers,blacksun,zann_consortium", "dread_ridge,dust_vipers", 1.0 },
	-- Kashyyyk
	-- trandoshan
	{ "rekkaluradan", true, "liakevgeurra,kittatha", "fesssellis", .20 },
	{ "fesssellis", true, "liakevgeurra,kittatha", "rekkaluradan", .20 },
	-- wookiee
	{ "liakevgeurra", true, "rekkaluradan,fesssellis", "kittatha", .20 },
	{ "kittatha", true, "rekkaluradan,fesssellis", "liakevgeurra", .20 },

	-- future dev
	{ "dread_ridge", true, "bloodrazor", "canyon_corsair,rekkaluradan", .12 },

	{ "mtn_clan_hoth", true, "", "rebel,imperial", 1.0 },
}
