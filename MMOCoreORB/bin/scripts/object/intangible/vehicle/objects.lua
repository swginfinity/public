--Copyright (C) 2009 <SWGEmu>

--This File is part of Core3.

--This program is free software; you can redistribute
--it and/or modify it under the terms of the GNU Lesser
--General Public License as published by the Free Software
--Foundation; either version 2 of the License,
--or (at your option) any later version.

--This program is distributed in the hope that it will be useful,
--but WITHOUT ANY WARRANTY; without even the implied warranty of
--MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
--See the GNU Lesser General Public License for
--more details.

--You should have received a copy of the GNU Lesser General
--Public License along with this program; if not, write to
--the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

--Linking Engine3 statically or dynamically with other modules
--is making a combined work based on Engine3.
--Thus, the terms and conditions of the GNU Lesser General Public License
--cover the whole combination.

--In addition, as a special exception, the copyright holders of Engine3
--give you permission to combine Engine3 program with free software
--programs or libraries that are released under the GNU LGPL and with
--code included in the standard release of Core3 under the GNU LGPL
--license (or modified versions of such code, with unchanged license).
--You may copy and distribute such a system following the terms of the
--GNU LGPL for Engine3 and the licenses of the other code concerned,
--provided that you include the source code of that other code when
--and as the GNU LGPL requires distribution of source code.

--Note that people who make modified versions of Engine3 are not obligated
--to grant this special exception for their modified versions;
--it is their choice whether to do so. The GNU Lesser General Public License
--gives permission to release a modified version without this exception;
--this exception also makes it possible to release a modified version
--which carries forward this exception.


object_intangible_vehicle_shared_jetpack_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_jetpack_pcd.iff"
	--Data below here is deprecated and loaded from the tres, keeping for easy lookups
--[[
	appearanceFilename = "appearance/jetpack_mandalorian_s01.apt",
	arrangementDescriptorFilename = "",

	clearFloraRadius = 0,
	clientDataFile = "",
	clientGameObjectType = 2056,
	collisionActionBlockFlags = 0,
	collisionActionFlags = 0,
	collisionActionPassFlags = 0,
	collisionMaterialBlockFlags = 0,
	collisionMaterialFlags = 0,
	collisionMaterialPassFlags = 0,
	containerType = 0,
	containerVolumeLimit = 0,

	detailedDescription = "@monster_detail:jetpack",

	gameObjectType = 2056,

	locationReservationRadius = 0,
	lookAtText = "string_id_table",

	noBuildRadius = 0,

	objectName = "@monster_name:jetpack",
	onlyVisibleInTools = 0,

	portalLayoutFilename = "",

	scale = 0,
	scaleThresholdBeforeExtentTest = 0.5,
	sendToClient = 1,
	slotDescriptorFilename = "abstract/slot/descriptor/mount_pcd.iff",
	snapToTerrain = 0,
	surfaceType = 0,

	totalCellNumber = 0,

	clientObjectCRC = 2096609656,
	derivedFromTemplates = {"object/object/base/shared_base_object.iff", "object/intangible/base/shared_base_intangible.iff", "object/intangible/vehicle/shared_vehicle_pcd_base.iff"}
]]
}

ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_jetpack_pcd, "object/intangible/vehicle/shared_jetpack_pcd.iff")

object_intangible_vehicle_shared_landspeeder_av21_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_landspeeder_av21_pcd.iff"
	--Data below here is deprecated and loaded from the tres, keeping for easy lookups
--[[
	appearanceFilename = "appearance/landspeeder_av21.apt",
	arrangementDescriptorFilename = "",

	clearFloraRadius = 0,
	clientDataFile = "",
	clientGameObjectType = 2056,
	collisionActionBlockFlags = 0,
	collisionActionFlags = 0,
	collisionActionPassFlags = 0,
	collisionMaterialBlockFlags = 0,
	collisionMaterialFlags = 0,
	collisionMaterialPassFlags = 0,
	containerType = 0,
	containerVolumeLimit = 0,

	detailedDescription = "@monster_detail:landspeeder_av21",

	gameObjectType = 2056,

	locationReservationRadius = 0,
	lookAtText = "string_id_table",

	noBuildRadius = 0,

	objectName = "@monster_name:landspeeder_av21",
	onlyVisibleInTools = 0,

	portalLayoutFilename = "",

	scale = 0,
	scaleThresholdBeforeExtentTest = 0.5,
	sendToClient = 1,
	slotDescriptorFilename = "abstract/slot/descriptor/mount_pcd.iff",
	snapToTerrain = 0,
	surfaceType = 0,

	totalCellNumber = 0,

	clientObjectCRC = 3762629621,
	derivedFromTemplates = {"object/object/base/shared_base_object.iff", "object/intangible/base/shared_base_intangible.iff", "object/intangible/vehicle/shared_vehicle_pcd_base.iff"}
]]
}

ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_landspeeder_av21_pcd, "object/intangible/vehicle/shared_landspeeder_av21_pcd.iff")

object_intangible_vehicle_shared_landspeeder_x31_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_landspeeder_x31_pcd.iff"
	--Data below here is deprecated and loaded from the tres, keeping for easy lookups
--[[
	appearanceFilename = "appearance/landspeeder_luke.apt",
	arrangementDescriptorFilename = "",

	clearFloraRadius = 0,
	clientDataFile = "",
	clientGameObjectType = 2056,
	collisionActionBlockFlags = 0,
	collisionActionFlags = 0,
	collisionActionPassFlags = 0,
	collisionMaterialBlockFlags = 0,
	collisionMaterialFlags = 0,
	collisionMaterialPassFlags = 0,
	containerType = 0,
	containerVolumeLimit = 0,

	detailedDescription = "@monster_detail:landspeeder_x31",

	gameObjectType = 2056,

	locationReservationRadius = 0,
	lookAtText = "string_id_table",

	noBuildRadius = 0,

	objectName = "@monster_name:landspeeder_x31",
	onlyVisibleInTools = 0,

	portalLayoutFilename = "",

	scale = 0,
	scaleThresholdBeforeExtentTest = 0.5,
	sendToClient = 1,
	slotDescriptorFilename = "abstract/slot/descriptor/mount_pcd.iff",
	snapToTerrain = 0,
	surfaceType = 0,

	totalCellNumber = 0,

	clientObjectCRC = 3148893162,
	derivedFromTemplates = {"object/object/base/shared_base_object.iff", "object/intangible/base/shared_base_intangible.iff", "object/intangible/vehicle/shared_vehicle_pcd_base.iff"}
]]
}

ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_landspeeder_x31_pcd, "object/intangible/vehicle/shared_landspeeder_x31_pcd.iff")

object_intangible_vehicle_shared_landspeeder_x34_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_landspeeder_x34_pcd.iff"
	--Data below here is deprecated and loaded from the tres, keeping for easy lookups
--[[
	appearanceFilename = "appearance/landspeeder_luke.apt",
	arrangementDescriptorFilename = "",

	clearFloraRadius = 0,
	clientDataFile = "",
	clientGameObjectType = 2056,
	collisionActionBlockFlags = 0,
	collisionActionFlags = 0,
	collisionActionPassFlags = 0,
	collisionMaterialBlockFlags = 0,
	collisionMaterialFlags = 0,
	collisionMaterialPassFlags = 0,
	containerType = 0,
	containerVolumeLimit = 0,

	detailedDescription = "@monster_detail:landspeeder_x34",

	gameObjectType = 2056,

	locationReservationRadius = 0,
	lookAtText = "string_id_table",

	noBuildRadius = 0,

	objectName = "@monster_name:landspeeder_x34",
	onlyVisibleInTools = 0,

	portalLayoutFilename = "",

	scale = 0,
	scaleThresholdBeforeExtentTest = 0.5,
	sendToClient = 1,
	slotDescriptorFilename = "abstract/slot/descriptor/mount_pcd.iff",
	snapToTerrain = 0,
	surfaceType = 0,

	totalCellNumber = 0,

	clientObjectCRC = 2364632781,
	derivedFromTemplates = {"object/object/base/shared_base_object.iff", "object/intangible/base/shared_base_intangible.iff", "object/intangible/vehicle/shared_vehicle_pcd_base.iff"}
]]
}

ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_landspeeder_x34_pcd, "object/intangible/vehicle/shared_landspeeder_x34_pcd.iff")

object_intangible_vehicle_shared_speederbike_flash_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_speederbike_flash_pcd.iff"
	--Data below here is deprecated and loaded from the tres, keeping for easy lookups
--[[
	appearanceFilename = "appearance/flash_speeder.apt",
	arrangementDescriptorFilename = "",

	clearFloraRadius = 0,
	clientDataFile = "",
	clientGameObjectType = 2056,
	collisionActionBlockFlags = 0,
	collisionActionFlags = 0,
	collisionActionPassFlags = 0,
	collisionMaterialBlockFlags = 0,
	collisionMaterialFlags = 0,
	collisionMaterialPassFlags = 0,
	containerType = 0,
	containerVolumeLimit = 0,

	detailedDescription = "@monster_detail:speederbike_flash",

	gameObjectType = 2056,

	locationReservationRadius = 0,
	lookAtText = "string_id_table",

	noBuildRadius = 0,

	objectName = "@monster_name:speederbike_flash",
	onlyVisibleInTools = 0,

	portalLayoutFilename = "",

	scale = 0,
	scaleThresholdBeforeExtentTest = 0.5,
	sendToClient = 1,
	slotDescriptorFilename = "abstract/slot/descriptor/mount_pcd.iff",
	snapToTerrain = 0,
	surfaceType = 0,

	totalCellNumber = 0,

	clientObjectCRC = 1448247515,
	derivedFromTemplates = {"object/object/base/shared_base_object.iff", "object/intangible/base/shared_base_intangible.iff", "object/intangible/vehicle/shared_vehicle_pcd_base.iff"}
]]
}

ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_speederbike_flash_pcd, "object/intangible/vehicle/shared_speederbike_flash_pcd.iff")

object_intangible_vehicle_shared_speederbike_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_speederbike_pcd.iff"
	--Data below here is deprecated and loaded from the tres, keeping for easy lookups
--[[
	appearanceFilename = "appearance/speeder_bike.apt",
	arrangementDescriptorFilename = "",

	clearFloraRadius = 0,
	clientDataFile = "",
	clientGameObjectType = 2056,
	collisionActionBlockFlags = 0,
	collisionActionFlags = 0,
	collisionActionPassFlags = 0,
	collisionMaterialBlockFlags = 0,
	collisionMaterialFlags = 0,
	collisionMaterialPassFlags = 0,
	containerType = 0,
	containerVolumeLimit = 0,

	detailedDescription = "@monster_detail:speederbike",

	gameObjectType = 2056,

	locationReservationRadius = 0,
	lookAtText = "string_id_table",

	noBuildRadius = 0,

	objectName = "@monster_name:speederbike",
	onlyVisibleInTools = 0,

	portalLayoutFilename = "",

	scale = 0,
	scaleThresholdBeforeExtentTest = 0.5,
	sendToClient = 1,
	slotDescriptorFilename = "abstract/slot/descriptor/mount_pcd.iff",
	snapToTerrain = 0,
	surfaceType = 0,

	totalCellNumber = 0,

	clientObjectCRC = 3659586517,
	derivedFromTemplates = {"object/object/base/shared_base_object.iff", "object/intangible/base/shared_base_intangible.iff", "object/intangible/vehicle/shared_vehicle_pcd_base.iff"}
]]
}

ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_speederbike_pcd, "object/intangible/vehicle/shared_speederbike_pcd.iff")

object_intangible_vehicle_shared_speederbike_swoop_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_speederbike_swoop_pcd.iff"
	--Data below here is deprecated and loaded from the tres, keeping for easy lookups
--[[
	appearanceFilename = "appearance/swoop_bike.apt",
	arrangementDescriptorFilename = "",

	clearFloraRadius = 0,
	clientDataFile = "",
	clientGameObjectType = 2056,
	collisionActionBlockFlags = 0,
	collisionActionFlags = 0,
	collisionActionPassFlags = 0,
	collisionMaterialBlockFlags = 0,
	collisionMaterialFlags = 0,
	collisionMaterialPassFlags = 0,
	containerType = 0,
	containerVolumeLimit = 0,

	detailedDescription = "@monster_detail:speederbike_swoop",

	gameObjectType = 2056,

	locationReservationRadius = 0,
	lookAtText = "string_id_table",

	noBuildRadius = 0,

	objectName = "@monster_name:speederbike_swoop",
	onlyVisibleInTools = 0,

	portalLayoutFilename = "",

	scale = 0,
	scaleThresholdBeforeExtentTest = 0.5,
	sendToClient = 1,
	slotDescriptorFilename = "abstract/slot/descriptor/mount_pcd.iff",
	snapToTerrain = 0,
	surfaceType = 0,

	totalCellNumber = 0,

	clientObjectCRC = 2233671002,
	derivedFromTemplates = {"object/object/base/shared_base_object.iff", "object/intangible/base/shared_base_intangible.iff", "object/intangible/vehicle/shared_vehicle_pcd_base.iff"}
]]
}

ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_speederbike_swoop_pcd, "object/intangible/vehicle/shared_speederbike_swoop_pcd.iff")

object_intangible_vehicle_shared_vehicle_pcd_base = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_vehicle_pcd_base.iff"
	--Data below here is deprecated and loaded from the tres, keeping for easy lookups
--[[
	appearanceFilename = "appearance/pv_speeder_bike.sat",
	arrangementDescriptorFilename = "",

	clearFloraRadius = 0,
	clientDataFile = "",
	clientGameObjectType = 2056,
	collisionActionBlockFlags = 0,
	collisionActionFlags = 0,
	collisionActionPassFlags = 0,
	collisionMaterialBlockFlags = 0,
	collisionMaterialFlags = 0,
	collisionMaterialPassFlags = 0,
	containerType = 0,
	containerVolumeLimit = 0,

	detailedDescription = "",

	gameObjectType = 2056,

	locationReservationRadius = 0,
	lookAtText = "string_id_table",

	noBuildRadius = 0,

	objectName = "",
	onlyVisibleInTools = 0,

	portalLayoutFilename = "",

	scale = 0,
	scaleThresholdBeforeExtentTest = 0.5,
	sendToClient = 1,
	slotDescriptorFilename = "abstract/slot/descriptor/mount_pcd.iff",
	snapToTerrain = 0,
	surfaceType = 0,

	totalCellNumber = 0,

	clientObjectCRC = 2853503513,
	derivedFromTemplates = {"object/object/base/shared_base_object.iff", "object/intangible/base/shared_base_intangible.iff"}
]]
}

ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_vehicle_pcd_base, "object/intangible/vehicle/shared_vehicle_pcd_base.iff")


--***MTG STUFF

object_intangible_vehicle_shared_swoopracer_black_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_swoopracer_black_pcd.iff"
}

ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_swoopracer_black_pcd, "object/intangible/vehicle/shared_swoopracer_black_pcd.iff")
------------------------------------------------------------------------------------------------------------------------------------


object_intangible_vehicle_shared_swoopracer_blue_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_swoopracer_blue_pcd.iff"
}

ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_swoopracer_blue_pcd, "object/intangible/vehicle/shared_swoopracer_blue_pcd.iff")
------------------------------------------------------------------------------------------------------------------------------------


object_intangible_vehicle_shared_swoopracer_gold_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_swoopracer_gold_pcd.iff"
}

ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_swoopracer_gold_pcd, "object/intangible/vehicle/shared_swoopracer_gold_pcd.iff")
------------------------------------------------------------------------------------------------------------------------------------


object_intangible_vehicle_shared_swoopracer_green_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_swoopracer_green_pcd.iff"
}

ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_swoopracer_green_pcd, "object/intangible/vehicle/shared_swoopracer_green_pcd.iff")
------------------------------------------------------------------------------------------------------------------------------------


object_intangible_vehicle_shared_swoopracer_purple_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_swoopracer_purple_pcd.iff"
}

ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_swoopracer_purple_pcd, "object/intangible/vehicle/shared_swoopracer_purple_pcd.iff")
------------------------------------------------------------------------------------------------------------------------------------


object_intangible_vehicle_shared_swoopracer_red_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_swoopracer_red_pcd.iff"
}

ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_swoopracer_red_pcd, "object/intangible/vehicle/shared_swoopracer_red_pcd.iff")
------------------------------------------------------------------------------------------------------------------------------------


object_intangible_vehicle_shared_swoopracer_silver_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_swoopracer_silver_pcd.iff"
}

ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_swoopracer_silver_pcd, "object/intangible/vehicle/shared_swoopracer_silver_pcd.iff")
------------------------------------------------------------------------------------------------------------------------------------

object_intangible_vehicle_shared_atat_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_atat_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_atat_pcd, "object/intangible/vehicle/shared_atat_pcd.iff")


object_intangible_vehicle_shared_atpt_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_atpt_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_atpt_pcd, "object/intangible/vehicle/shared_atpt_pcd.iff")


object_intangible_vehicle_shared_atrt_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_atrt_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_atrt_pcd, "object/intangible/vehicle/shared_atrt_pcd.iff")


object_intangible_vehicle_shared_atst_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_atst_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_atst_pcd, "object/intangible/vehicle/shared_atst_pcd.iff")


object_intangible_vehicle_shared_atxt_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_atxt_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_atxt_pcd, "object/intangible/vehicle/shared_atxt_pcd.iff")


object_intangible_vehicle_shared_a1_deluxe_floater_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_a1_deluxe_floater_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_a1_deluxe_floater_pcd, "object/intangible/vehicle/shared_a1_deluxe_floater_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_air2_swoop_speeder_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_air2_swoop_speeder_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_air2_swoop_speeder_pcd, "object/intangible/vehicle/shared_air2_swoop_speeder_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_barc_speeder_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_barc_speeder_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_barc_speeder_pcd, "object/intangible/vehicle/shared_barc_speeder_pcd.iff")
--**********************************************************************

object_intangible_vehicle_shared_barc_speeder_imperial_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_barc_speeder_imperial_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_barc_speeder_imperial_pcd, "object/intangible/vehicle/shared_barc_speeder_imperial_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_barc_speeder_rebel_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_barc_speeder_rebel_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_barc_speeder_rebel_pcd, "object/intangible/vehicle/shared_barc_speeder_rebel_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_basilisk_war_droid = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_basilisk_war_droid.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_basilisk_war_droid, "object/intangible/vehicle/shared_basilisk_war_droid.iff")

--**********************************************************************

object_intangible_vehicle_shared_fg_8t8_podracer_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_fg_8t8_podracer_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_fg_8t8_podracer_pcd, "object/intangible/vehicle/shared_fg_8t8_podracer_pcd.iff")


--**********************************************************************

object_intangible_vehicle_shared_flare_s_swoop_crafted_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_flare_s_swoop_crafted_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_flare_s_swoop_crafted_pcd, "object/intangible/vehicle/shared_flare_s_swoop_crafted_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_flare_s_swoop_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_flare_s_swoop_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_flare_s_swoop_pcd, "object/intangible/vehicle/shared_flare_s_swoop_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_geonosian_speeder_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_geonosian_speeder_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_geonosian_speeder_pcd, "object/intangible/vehicle/shared_geonosian_speeder_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_gift_stap_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_gift_stap_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_gift_stap_pcd, "object/intangible/vehicle/shared_gift_stap_pcd.iff")

object_intangible_vehicle_shared_atat_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_atat_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_atat_pcd, "object/intangible/vehicle/shared_atat_pcd.iff")


object_intangible_vehicle_shared_atpt_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_atpt_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_atpt_pcd, "object/intangible/vehicle/shared_atpt_pcd.iff")


object_intangible_vehicle_shared_atrt_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_atrt_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_atrt_pcd, "object/intangible/vehicle/shared_atrt_pcd.iff")


object_intangible_vehicle_shared_atst_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_atst_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_atst_pcd, "object/intangible/vehicle/shared_atst_pcd.iff")


object_intangible_vehicle_shared_atxt_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_atxt_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_atxt_pcd, "object/intangible/vehicle/shared_atxt_pcd.iff")


object_intangible_vehicle_shared_a1_deluxe_floater_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_a1_deluxe_floater_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_a1_deluxe_floater_pcd, "object/intangible/vehicle/shared_a1_deluxe_floater_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_air2_swoop_speeder_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_air2_swoop_speeder_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_air2_swoop_speeder_pcd, "object/intangible/vehicle/shared_air2_swoop_speeder_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_barc_speeder_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_barc_speeder_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_barc_speeder_pcd, "object/intangible/vehicle/shared_barc_speeder_pcd.iff")
--**********************************************************************

object_intangible_vehicle_shared_barc_speeder_imperial_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_barc_speeder_imperial_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_barc_speeder_imperial_pcd, "object/intangible/vehicle/shared_barc_speeder_imperial_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_barc_speeder_rebel_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_barc_speeder_rebel_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_barc_speeder_rebel_pcd, "object/intangible/vehicle/shared_barc_speeder_rebel_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_fg_8t8_podracer_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_fg_8t8_podracer_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_fg_8t8_podracer_pcd, "object/intangible/vehicle/shared_fg_8t8_podracer_pcd.iff")


--**********************************************************************

object_intangible_vehicle_shared_flare_s_swoop_crafted_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_flare_s_swoop_crafted_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_flare_s_swoop_crafted_pcd, "object/intangible/vehicle/shared_flare_s_swoop_crafted_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_flare_s_swoop_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_flare_s_swoop_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_flare_s_swoop_pcd, "object/intangible/vehicle/shared_flare_s_swoop_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_geonosian_speeder_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_geonosian_speeder_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_geonosian_speeder_pcd, "object/intangible/vehicle/shared_geonosian_speeder_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_gift_stap_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_gift_stap_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_gift_stap_pcd, "object/intangible/vehicle/shared_gift_stap_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_grievous_wheel_bike = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_grievous_wheel_bike.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_grievous_wheel_bike, "object/intangible/vehicle/shared_grievous_wheel_bike.iff")

--**********************************************************************
object_intangible_vehicle_shared_grievous_wheel_bike_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_grievous_wheel_bike_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_grievous_wheel_bike_pcd, "object/intangible/vehicle/shared_grievous_wheel_bike_pcd.iff")

--**********************************************************************
object_intangible_vehicle_shared_hoth_at_st_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_hoth_at_st_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_hoth_at_st_pcd, "object/intangible/vehicle/shared_hoth_at_st_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_hover_chair_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_hover_chair_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_hover_chair_pcd, "object/intangible/vehicle/shared_hover_chair_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_hoverlifter_speeder_crafted_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_hoverlifter_speeder_crafted_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_hoverlifter_speeder_crafted_pcd, "object/intangible/vehicle/shared_hoverlifter_speeder_crafted_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_hoverlifter_speeder_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_hoverlifter_speeder_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_hoverlifter_speeder_pcd, "object/intangible/vehicle/shared_hoverlifter_speeder_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_koro2_speeder_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_koro2_speeder_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_koro2_speeder_pcd, "object/intangible/vehicle/shared_koro2_speeder_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_landspeeder_ab1_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_landspeeder_ab1_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_landspeeder_ab1_pcd, "object/intangible/vehicle/shared_landspeeder_ab1_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_landspeeder_desert_skiff_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_landspeeder_desert_skiff_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_landspeeder_desert_skiff_pcd, "object/intangible/vehicle/shared_landspeeder_desert_skiff_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_landspeeder_lava_skiff_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_landspeeder_lava_skiff_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_landspeeder_lava_skiff_pcd, "object/intangible/vehicle/shared_landspeeder_lava_skiff_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_landspeeder_organa_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_landspeeder_organa_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_landspeeder_organa_pcd, "object/intangible/vehicle/shared_landspeeder_organa_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_landspeeder_tantive4_adv_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_landspeeder_tantive4_adv_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_landspeeder_tantive4_adv_pcd, "object/intangible/vehicle/shared_landspeeder_tantive4_adv_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_landspeeder_tantive4_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_landspeeder_tantive4_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_landspeeder_tantive4_pcd, "object/intangible/vehicle/shared_landspeeder_tantive4_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_landspeeder_usv5_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_landspeeder_usv5_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_landspeeder_usv5_pcd, "object/intangible/vehicle/shared_landspeeder_usv5_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_landspeeder_usv5_s02_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_landspeeder_usv5_s02_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_landspeeder_usv5_s02_pcd, "object/intangible/vehicle/shared_landspeeder_usv5_s02_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_landspeeder_v35_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_landspeeder_v35_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_landspeeder_v35_pcd, "object/intangible/vehicle/shared_landspeeder_v35_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_landspeeder_xp38_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_landspeeder_xp38_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_landspeeder_xp38_pcd, "object/intangible/vehicle/shared_landspeeder_xp38_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_light_bending_barc = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_light_bending_barc.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_light_bending_barc, "object/intangible/vehicle/shared_light_bending_barc.iff")

--**********************************************************************

object_intangible_vehicle_shared_mechno_chair_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_mechno_chair_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_mechno_chair_pcd, "object/intangible/vehicle/shared_mechno_chair_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_mustafar_panning_droid = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_mustafar_panning_droid.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_mustafar_panning_droid, "object/intangible/vehicle/shared_mustafar_panning_droid.iff")

--**********************************************************************

object_intangible_vehicle_shared_pod_racer_balta_podracer_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_pod_racer_balta_podracer_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_pod_racer_balta_podracer_pcd, "object/intangible/vehicle/shared_pod_racer_balta_podracer_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_pod_racer_ipg_longtail_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_pod_racer_ipg_longtail_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_pod_racer_ipg_longtail_pcd, "object/intangible/vehicle/shared_pod_racer_ipg_longtail_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_pod_racer_light_bending = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_pod_racer_light_bending.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_pod_racer_light_bending, "object/intangible/vehicle/shared_pod_racer_light_bending.iff")

--**********************************************************************

object_intangible_vehicle_shared_pod_racer_one_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_pod_racer_one_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_pod_racer_one_pcd, "object/intangible/vehicle/shared_pod_racer_one_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_pod_racer_two_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_pod_racer_two_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_pod_racer_two_pcd, "object/intangible/vehicle/shared_pod_racer_two_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_podracer_anakin_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_podracer_anakin_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_podracer_anakin_pcd, "object/intangible/vehicle/shared_podracer_anakin_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_senate_pod_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_senate_pod_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_senate_pod_pcd, "object/intangible/vehicle/shared_senate_pod_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_sith_speeder_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_sith_speeder_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_sith_speeder_pcd, "object/intangible/vehicle/shared_sith_speeder_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_snowspeeder = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_snowspeeder.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_snowspeeder, "object/intangible/vehicle/shared_snowspeeder.iff")

--**********************************************************************

object_intangible_vehicle_shared_speeder_ric_920_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_speeder_ric_920_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_speeder_ric_920_pcd, "object/intangible/vehicle/shared_speeder_ric_920_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_stap_speeder_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_stap_speeder_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_stap_speeder_pcd, "object/intangible/vehicle/shared_stap_speeder_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_swamp_speeder_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_swamp_speeder_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_swamp_speeder_pcd, "object/intangible/vehicle/shared_swamp_speeder_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_tcg_8_air_speeder = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_tcg_8_air_speeder.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_tcg_8_air_speeder, "object/intangible/vehicle/shared_tcg_8_air_speeder.iff")

--**********************************************************************

object_intangible_vehicle_shared_tcg_8_air_speeder_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_tcg_8_air_speeder_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_tcg_8_air_speeder_pcd, "object/intangible/vehicle/shared_tcg_8_air_speeder_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_tcg_at_pt_walker_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_tcg_at_pt_walker_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_tcg_at_pt_walker_pcd, "object/intangible/vehicle/shared_tcg_at_pt_walker_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_tcg_hk47_jetpack_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_tcg_hk47_jetpack_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_tcg_hk47_jetpack_pcd, "object/intangible/vehicle/shared_tcg_hk47_jetpack_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_tcg_merr_sonn_jt12_jetpack_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_tcg_merr_sonn_jt12_jetpack_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_tcg_merr_sonn_jt12_jetpack_pcd, "object/intangible/vehicle/shared_tcg_merr_sonn_jt12_jetpack_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_tcg_military_transport_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_tcg_military_transport_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_tcg_military_transport_pcd, "object/intangible/vehicle/shared_tcg_military_transport_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_tcg_republic_gunship_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_tcg_republic_gunship_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_tcg_republic_gunship_pcd, "object/intangible/vehicle/shared_tcg_republic_gunship_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_temp_walker1_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_temp_walker1_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_temp_walker1_pcd, "object/intangible/vehicle/shared_temp_walker1_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_walker_at_rt_camo_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_walker_at_rt_camo_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_walker_at_rt_camo_pcd, "object/intangible/vehicle/shared_walker_at_rt_camo_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_walker_at_rt_reg_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_walker_at_rt_reg_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_walker_at_rt_reg_pcd, "object/intangible/vehicle/shared_walker_at_rt_reg_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_walker_at_st_walker_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_walker_at_st_walker_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_walker_at_st_walker_pcd, "object/intangible/vehicle/shared_walker_at_st_walker_pcd.iff")


--**********************************************************************

object_intangible_vehicle_shared_walker_at_xt_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_walker_at_xt_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_walker_at_xt_pcd, "object/intangible/vehicle/shared_walker_at_xt_pcd.iff")

--**********************************************************************

object_intangible_vehicle_shared_xj6_air_speeder_pcd = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vehicle/shared_xj6_air_speeder_pcd.iff"
}
ObjectTemplates:addClientTemplate(object_intangible_vehicle_shared_xj6_air_speeder_pcd, "object/intangible/vehicle/shared_xj6_air_speeder_pcd.iff")
