CorsecBaseScreenPlay = ScreenPlay:new {
    numberOfActs = 1,

    screenplayName = "CorsecBaseScreenPlay",

    lootContainers = {
        6575944,
        6575945,
    
    },

    lootLevel = 250,    

    lootGroups = {
        {
            groups = {
                {group = "color_crystals", chance = 200000},
                {group = "clothing_attachments", chance = 400000},
                {group = "armor_attachments", chance = 400000}
            },
            lootChance = 10000000
        }                    
    },

    lootContainerRespawn = 900 -- 15 minutes
}

registerScreenPlay("CorsecBaseScreenPlay", true)

function CorsecBaseScreenPlay:start()
    if (isZoneEnabled("corellia")) then
        self:spawnSceneObjects()
        self:spawnMobiles()
        self:initializeLootContainers()
    end
end

function CorsecBaseScreenPlay:spawnSceneObjects()
    --Scenery
    spawnSceneObject("corellia", "object/static/structure/general/transport_debris_01.iff", 5167.82, 3, 1434.74, 0, math.rad(270))
    spawnSceneObject("corellia", "object/static/structure/general/transport_debris_02.iff", 5200.15, 2, 1434.74, 0, math.rad(270))
    --Wall pathway created to make it more maze like and direct players to certain clearing points.
    spawnSceneObject("corellia", "object/static/structure/corellia/corl_imprv_wall_4x16_s04.iff", 5208, 5, 1516, 0, math.rad(90))
    spawnSceneObject("corellia", "object/static/structure/corellia/corl_imprv_wall_4x16_s04.iff", 5293, 5, 1557.50, 0, math.rad(180))
    spawnSceneObject("corellia", "object/static/structure/corellia/corl_imprv_wall_4x16_s04.iff", 5277, 5, 1557.50, 0, math.rad(180))
    spawnSceneObject("corellia", "object/static/structure/corellia/corl_imprv_wall_4x16_s04.iff", 5267, 5, 1557.50, 0, math.rad(180))
    spawnSceneObject("corellia", "object/static/structure/corellia/corl_imprv_wall_4x16_s04.iff", 5267, 5, 1557.50, 0, math.rad(180))
    spawnSceneObject("corellia", "object/installation/battlefield/destructible/bfield_column_rebl_weak.iff", 5208, 5, 1507.50, 0, math.rad(180))
    spawnSceneObject("corellia", "object/static/structure/corellia/corl_imprv_wall_4x16_s04.iff", 5235, 5, 1558, 0, math.rad(145))
    spawnSceneObject("corellia", "object/static/structure/corellia/corl_imprv_wall_4x16_s04.iff", 5205, 5, 1584, 0, math.rad(145))
    spawnSceneObject("corellia", "object/installation/battlefield/destructible/bfield_column_rebl_weak.iff", 5211.96, 5, 1588.93, 0, math.rad(180))
    spawnSceneObject("corellia", "object/static/structure/corellia/corl_imprv_wall_4x16_s04.iff", 5217, 5, 1582, 0, math.rad(55))
    spawnSceneObject("corellia", "object/installation/battlefield/destructible/bfield_column_rebl_weak.iff", 5221.75, 5, 1576, 0, math.rad(170))
    spawnSceneObject("corellia", "object/static/structure/corellia/corl_imprv_wall_4x16_s04.iff", 5228, 5, 1580, 0, math.rad(145))
    spawnSceneObject("corellia", "object/static/structure/corellia/corl_imprv_wall_4x16_s04.iff", 5222.50, 5, 1609, 0, math.rad(55))
    spawnSceneObject("corellia", "object/static/structure/corellia/corl_imprv_wall_4x16_s04.iff", 5226, 5, 1604, 0, math.rad(55))
    spawnSceneObject("corellia", "object/static/structure/corellia/corl_imprv_wall_4x16_s04.iff", 5195.59, 5, 1587.87, 0, math.rad(55))
    spawnSceneObject("corellia", "object/static/structure/corellia/corl_imprv_wall_4x16_s04.iff", 5192.79, 5, 1591.60, 0, math.rad(55))
    spawnSceneObject("corellia", "object/static/structure/corellia/corl_imprv_wall_4x16_s04.iff", 5178, 5, 1642, 0, math.rad(145))
end

function CorsecBaseScreenPlay:spawnMobiles()

    --Raid Boss<<Laen Pieweto>> Boss 45-55min Respawn, NPCs on 45 Minute as respawn indicator for boss soon to respawn!
    spawnMobile("corellia", "laen_pieweto",math.random(2700, 3300),1.5,-20.8,33.4,-179,1855730,true) --Raid Boss, 45-55 min respawn rate
    spawnMobile("corellia", "chiss_poacher_bodyguard",2700,4.8,-20.7,2.8,-179,1855729)--Boss Bodyguard
    spawnMobile("corellia", "chiss_poacher_bodyguard",2700,-2.0,-20.7,4.7,-57,1855729)--Boss Bodyguard
    spawnMobile("corellia", "chiss_poacher_bodyguard",2700,1.6,-20.8,18.9,0,1855730) --Boss Bodyguard
    spawnMobile("corellia", "chiss_poacher_bodyguard",2700,-0.9,-20.8,31.1,89,1855730) --Boss Bodyguard
    spawnMobile("corellia", "chiss_poacher_bodyguard",2700,4.5,-20.8,30.9,-93,1855730) --Boss Bodyguard
    --Trash Mobs inside Bunker <<20 min respawn.>>
    spawnMobile("corellia", "chiss_poacher_assailant",1200,-7.4,-17.3,-4.3,42,1855728) -- <<Ramps>>
    spawnMobile("corellia", "chiss_poacher_thug",1200,-5.9,-17.3,-2.6,-137,1855728) -- <<Ramps>>
    spawnMobile("corellia", "chiss_poacher_supervisor",1200,-7.1,-13.8,7.1,90,1855727) --<<Room3>>
    spawnMobile("corellia", "chiss_poacher_hunter",1200,0.2,-13.8,7.9,-92,1855727) --<<Room3>>
    spawnMobile("corellia", "chiss_poacher_thug",1200,-0.5,-13.8,5.8,-146,1855727) --<<Room3>>
    spawnMobile("corellia", "chiss_poacher_hunter",1200,7.4,-13.8,8.2,54,1855727) --<<Room3>>
    spawnMobile("corellia", "chiss_poacher_hunter",1200,-4.6,-10.3,-5.3,147,1855725) --<<Ramp2>>
    spawnMobile("corellia", "chiss_poacher_director",1200,4.3,0.3,-4.2,91,1855723) --<<Hall2>>
	spawnMobile("corellia", "chiss_poacher_thug",1200,-3.7,0.3,-2.5,177,1855722) --<<Hall1>>
	spawnMobile("corellia", "chiss_poacher_assailant",1200,-3.7,0.3,-3.7,0,1855722) --<<Hall1>>
	spawnMobile("corellia", "chiss_poacher_supervisor",1200,-1.8,0.3,2.3,179,1855722) --<<Hall1>>
	--Mid Raid Boss Nabek Naga Dark Jedi with Acolytes
	spawnMobile("corellia", "nabek_naga",math.random(2700, 3300),5226.8,5.0,1580.8,-34,0,true) --<<Boss before Bunker entrance>> Random Respawn 45-55min
	spawnMobile("corellia", "nabek_acolyte2",2700,5224.4,5.0,1578.7,-34,0) --Apok
	spawnMobile("corellia", "nabek_acolyte1",2700,5229.6,5.0,1582.4,-34,0) --Euyer
	--3rd building
	spawnMobile("corellia", "chiss_poacher_director",1200,-16.0,0.2,7.5,-90,1855738) --
	spawnMobile("corellia", "chiss_poacher_supervisor",1200,-9.7,0.2,-10.3,164,1855738) --
	spawnMobile("corellia", "chiss_poacher_hunter",1200,-2.5,0.2,0.3,-92,1855738) --
	spawnMobile("corellia", "chiss_poacher_assailant",1200,-4.5,0.2,-1.3,0,1855738) --
	spawnMobile("corellia", "chiss_poacher_director",1200,15.9,0.2,8.5,127,1855738) --
	spawnMobile("corellia", "chiss_poacher_supervisor",1200,15.8,0.2,6.5,58,1855738) --
	spawnMobile("corellia", "chiss_poacher_director",1200,-4.3,0.2,10.6,-2,1855738) 
	spawnMobile("corellia", "chiss_poacher_hunter",1200,11.2,0.2,5.3,-170,1855738) --
	spawnMobile("corellia", "chiss_poacher_thug",1200,10.6,0.2,1.3,9,1855738) --
	spawnMobile("corellia", "chiss_poacher_director",1200,4.3,0.2,-10.4,179,1855738) --
	--After first building
	spawnMobile("corellia", "drosso_zraccu",2700,5204.0,5.0,1511.5,-39,0) --Optional Boss	
	spawnMobile("corellia", "chiss_poacher_supervisor",1200,5191.4,5.0,1535.9,-107,0)	
	spawnMobile("corellia", "chiss_poacher_director",1200,5185.3,5.0,1533.3,56,0)
	--By Shuttle Ship
	spawnMobile("corellia", "vash_dlarku",2700,5177.7,5.0,1584.6,-37,0) --Optional Boss
	spawnMobile("corellia", "chiss_poacher_director",1200,5154.5,5.0,1610.6,130,0)
	spawnMobile("corellia", "chiss_poacher_hunter",1200,5157.1,5.0,1608.4,-56,0)
	spawnMobile("corellia", "chiss_poacher_thug",1200,5168.9,5.0,1604.1,173,0)
	spawnMobile("corellia", "chiss_poacher_supervisor",1200,5165.7,5.0,1601.9,55,0)
	--Behind Lambda Shuttle <<Outside>>
	spawnMobile("corellia", "master_tamer_xerthan",math.rad(2700, 3300),5214.9,5.0,1630.0,15,0,true)
	spawnMobile("corellia", "tamers_prized_wrix",2700,5216.2,5.0,1627.8,8,0)
	spawnMobile("corellia", "tamers_prized_wrix2",2700,5213.0,5.0,1632.0,19,0)
	spawnMobile("corellia", "chiss_poacher_assailant",1200,5224.9,5,1612.7,38,0) --Trash mobs before boss
    spawnMobile("corellia", "chiss_poacher_hunter",1200,5226.9,5,1616.6,-159,0) --Trash mobs before boss
    spawnMobile("corellia", "chiss_poacher_thug",1200,5229.2,5,1613.3,-85,0) --Trash mobs before boss
    spawnMobile("corellia", "chiss_poacher_director",1200,5290.3,5.0,1574.7,-19,0)	--Trash Mobs outside of 2nd building
	spawnMobile("corellia", "chiss_poacher_assailant",1200,5285.0,5.0,1578.4,127,0)	--Trash Mobs outside of 2nd building
	spawnMobile("corellia", "chiss_poacher_hunter",1200,5289.8,5.0,1581.8,-177,0)	--Trash Mobs outside of 2nd building
	--2nd Building
	spawnMobile("corellia", "chiss_poacher_director",1200,1.6,0.2,1.0,179,1855742) -- Looking at lootbox
	spawnMobile("corellia", "chiss_poacher_supervisor",1200,-1.5,0.2,-10.3,167,1855742)	--by droids
	spawnMobile("corellia", "chiss_poacher_thug",1200,-5.9,0.2,-1.0,-30,1855742)
	spawnMobile("corellia", "chiss_poacher_thug",1200,-4.1,0.2,1.8,-93,1855742)
    spawnMobile("corellia", "chiss_poacher_thug",1200,-10,0.2,5.8,137,1855742)
    spawnMobile("corellia", "chiss_poacher_hunter",1200,-10.1,0.2,-1.3,40,1855742)
	--1st building
	spawnMobile("corellia", "chiss_poacher_thug",1200,11.1,0.2,6.8,-89,1855746)
	spawnMobile("corellia", "chiss_poacher_hunter",1200,6.8,0.2,7.4,94,1855746)
	spawnMobile("corellia", "chiss_poacher_director",1200,-11.3,0.2,-5.7,-179,1855746)
	spawnMobile("corellia", "chiss_poacher_assailant",1200,-8.9,0.2,-5.8,-90,1855746)
	spawnMobile("corellia", "chiss_poacher_supervisor",1200,-15.1,0.2,6.9,-98,1855746)
	spawnMobile("corellia", "chiss_poacher_assailant",1200,14.8,0.2,-5.8,89,1855746)
	--Entrance trash mobs
	spawnMobile("corellia", "chiss_poacher_hunter",1200,5237.7,5.0,1530.7,38,0)
	spawnMobile("corellia", "chiss_poacher_assailant",1200,5240.5,5.0,1533.5,-129,0)
	spawnMobile("corellia", "chiss_poacher_director",1200,5216.9,5.0,1521.1,83,0)
	spawnMobile("corellia", "chiss_poacher_thug",1200,5219.4,5.0,1520.1,-72,0)
end
