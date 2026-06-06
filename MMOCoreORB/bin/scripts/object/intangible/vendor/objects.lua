--Copyright (C) 2026 <Infinity>


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


-- Client-shared template registration for the Vendor Control Device.
-- The corresponding shared IFF ships in the client TRE files
-- (object/intangible/vendor/shared_generic_vendor_control_device.iff,
-- CRC 0x87087846). This file creates the Lua global the server-side
-- generic_vendor_control_device.lua template inherits from.

object_intangible_vendor_shared_generic_vendor_control_device = SharedIntangibleObjectTemplate:new {
	clientTemplateFileName = "object/intangible/vendor/shared_generic_vendor_control_device.iff"
}

ObjectTemplates:addClientTemplate(object_intangible_vendor_shared_generic_vendor_control_device, "object/intangible/vendor/shared_generic_vendor_control_device.iff")
