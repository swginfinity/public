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


object_weapon_melee_special_shared_blasterfist = SharedWeaponObjectTemplate:new {
	clientTemplateFileName = "object/weapon/melee/special/shared_blasterfist.iff"}

ObjectTemplates:addClientTemplate(object_weapon_melee_special_shared_blasterfist, "object/weapon/melee/special/shared_blasterfist.iff")

object_weapon_melee_sword_shared_sword_acid = SharedWeaponObjectTemplate:new {
	clientTemplateFileName = "object/weapon/melee/sword/shared_sword_acid.iff"}

ObjectTemplates:addClientTemplate(object_weapon_melee_sword_shared_sword_acid, "object/weapon/melee/sword/shared_sword_acid.iff")

object_weapon_melee_2h_sword_shared_2h_sword_sith = SharedWeaponObjectTemplate:new {
	clientTemplateFileName = "object/weapon/melee/2h_sword/shared_2h_sword_sith.iff"}

ObjectTemplates:addClientTemplate(object_weapon_melee_2h_sword_shared_2h_sword_sith, "object/weapon/melee/2h_sword/shared_2h_sword_sith.iff")

object_weapon_melee_baton_shared_mandoviol_smasher = SharedWeaponObjectTemplate:new {
	clientTemplateFileName = "object/weapon/melee/baton/shared_mandoviol_smasher.iff"}

ObjectTemplates:addClientTemplate(object_weapon_melee_baton_shared_mandoviol_smasher, "object/weapon/melee/baton/shared_mandoviol_smasher.iff")

object_weapon_melee_polearm_shared_lance_cryo = SharedWeaponObjectTemplate:new {
	clientTemplateFileName = "object/weapon/melee/polearm/shared_lance_cryo.iff"}

ObjectTemplates:addClientTemplate(object_weapon_melee_polearm_shared_lance_cryo, "object/weapon/melee/polearm/shared_lance_cryo.iff")
