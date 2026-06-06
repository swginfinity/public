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


-- Server-side template for the Vendor Control Device intangible used by the
-- Vendor Pack-Up feature (Phase 1). The corresponding shared template
-- `object/intangible/vendor/shared_generic_vendor_control_device.iff` is
-- already shipped in the client TRE files (CRC 0x87087846).

object_intangible_vendor_generic_vendor_control_device = object_intangible_vendor_shared_generic_vendor_control_device:new {
	-- VCD holds one vendor TANO at a time as a list-style container child.
	-- containerType = 1 (LIST), volumeLimit = 1 (one vendor). Set directly
	-- on the server-side template since this is a new object — server
	-- properties belong here, not on the client-shared.
	containerType = 1,
	containerVolumeLimit = 1,
}

ObjectTemplates:addTemplate(object_intangible_vendor_generic_vendor_control_device, "object/intangible/vendor/generic_vendor_control_device.iff")
