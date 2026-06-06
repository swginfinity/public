-- Infinity: shared client template for the Room Designer blueprint disk.
-- Visually a disk asset (room_recording.apt) but inherits container behavior
-- from shared_base_container_volume.iff so the SWG client renders the
-- chest-style Open UI on double-click. See ROOM_DESIGNER.md.

object_tangible_container_blueprint_shared_blueprint_disk = SharedTangibleObjectTemplate:new {
	clientTemplateFileName = "object/tangible/container/blueprint/shared_blueprint_disk.iff"
}
ObjectTemplates:addClientTemplate(object_tangible_container_blueprint_shared_blueprint_disk,
	"object/tangible/container/blueprint/shared_blueprint_disk.iff")
