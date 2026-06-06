/*
 * BlueprintDiskObjectImplementation.cpp
 *
 * Sealed container for the Room Designer v2 system.
 *
 * Contents don't count against player inventory cap (getCountableObjectsRecursive
 * returns 1). Drags in and out are denied via container permissions; burn and
 * deploy flows toggle a "burn_in_progress" LuaStringData flag that
 * canAddObject consults to permit internal server moves.
 *
 * See /workspace/ROOM_DESIGNER.md for design rationale.
 *
 * Infinity: Custom class
 */

#include "server/zone/objects/blueprint/BlueprintDiskObject.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/objects/player/PlayerObject.h"
#include "server/zone/packets/scene/AttributeListMessage.h"
#include "server/zone/objects/scene/variables/ContainerPermissions.h"

void BlueprintDiskObjectImplementation::initializeTransientMembers() {
	TangibleObjectImplementation::initializeTransientMembers();

	setLoggingName("BlueprintDiskObject");
}

void BlueprintDiskObjectImplementation::loadTemplateData(SharedObjectTemplate* templateData) {
	TangibleObjectImplementation::loadTemplateData(templateData);
}

void BlueprintDiskObjectImplementation::sendBaselinesTo(SceneObject* player) {
	TangibleObjectImplementation::sendBaselinesTo(player);
}

void BlueprintDiskObjectImplementation::fillAttributeList(AttributeListMessage* alm, CreatureObject* player) {
	if (alm == nullptr || player == nullptr)
		return;

	// Skip TangibleObjectImplementation::fillAttributeList intentionally — its
	// auto volume/contents block ("0/250") would lie because we override
	// getCountableObjectsRecursive() to 0 for inventory-cap-bypass. We
	// re-add the TangibleObject attributes that actually apply to a sealed
	// container (serial, crafter, no-trade) below, plus a real-time
	// "contents: N/maxCapacity" using the direct-children count.
	SceneObjectImplementation::fillAttributeList(alm, player);

	if (!objectSerial.isEmpty())
		alm->insertAttribute("serial_number", objectSerial);

	if (!craftersName.isEmpty())
		alm->insertAttribute("crafter", craftersName);

	StringBuffer contentsString;
	contentsString << getContainerObjectsSize() << "/" << maxCapacity;
	alm->insertAttribute("contents", contentsString);

	// Surface the scene metadata on inspect so buyers can verify before paying.
	String sceneName = getLuaStringData("scene_name");
	if (!sceneName.isEmpty())
		alm->insertAttribute("blueprint_scene_name", sceneName);

	String state = getLuaStringData("disk_state");
	if (state.isEmpty())
		state = "blank";
	alm->insertAttribute("blueprint_state", state);
}

void BlueprintDiskObjectImplementation::fillObjectMenuResponse(ObjectMenuResponse* menuResponse, CreatureObject* player) {
	// Delegates entirely to the BlueprintDiskMenuComponent attached via the
	// object template. We intentionally do not add radial items here.
	TangibleObjectImplementation::fillObjectMenuResponse(menuResponse, player);
}

int BlueprintDiskObjectImplementation::handleObjectMenuSelect(CreatureObject* player, byte selectedID) {
	return TangibleObjectImplementation::handleObjectMenuSelect(player, selectedID);
}

int BlueprintDiskObjectImplementation::canAddObject(SceneObject* object, int containmentType, String& errorDescription) {
	// First: the base TangibleObject checks (same-object, no-trade rules, etc.)
	int baseResult = TangibleObjectImplementation::canAddObject(object, containmentType, errorDescription);
	if (baseResult != 0) {
		return baseResult;
	}

	// GM always bypasses.
	ManagedReference<SceneObject*> objParent = object->getParent().get();
	if (objParent != nullptr && objParent->isPlayerCreature()) {
		CreatureObject* actor = cast<CreatureObject*>(objParent.get());
		if (actor != nullptr && actor->getPlayerObject() != nullptr && actor->getPlayerObject()->isPrivileged()) {
			return 0;
		}
	}

	// Internal burn/deploy path: the Lua burn code sets "burn_in_progress"
	// on the disk before calling transferObject, and clears it after.
	// Deploy is structured not to call canAddObject (it removes items TO a
	// cell), but leaving this as a single gate means one check for all server
	// moves into the disk.
	String burnFlag = getLuaStringData("burn_in_progress");
	if (burnFlag == "1") {
		return 0;
	}

	// Reject player drag-ins.
	errorDescription = "This disk is sealed. Items cannot be added manually.";
	return TransferErrorCode::CANTADD;
}

byte BlueprintDiskObjectImplementation::checkContainerPermission(CreatureObject* player, uint16 permission) {
	// Privileged staff get full bypass for support / GM recovery. The
	// framework's built-in admin bypass (ContainerComponent.cpp) only
	// auto-grants OPEN and WALKIN — MOVEOUT and MOVEIN need an explicit
	// override here, otherwise admins can't drag stuck items out.
	if (player != nullptr) {
		PlayerObject* ghost = player->getPlayerObject();

		if (ghost != nullptr && ghost->isPrivileged())
			return true;
	}

	// Anyone can OPEN the disk to browse contents (chest-style preview before
	// purchase / before deploy). Same as a tech chest.
	if (permission == ContainerPermissions::OPEN)
		return true;

	// Players cannot drag items in or out — only the burn (Lua-privileged
	// transferObject in) and deploy (Lua-privileged transferObject out)
	// flows can move items. Same gate as canAddObject, just restated for
	// the MOVEOUT direction the framework doesn't auto-bypass.
	if (permission == ContainerPermissions::MOVEIN || permission == ContainerPermissions::MOVEOUT)
		return false;

	return SceneObjectImplementation::checkContainerPermission(player, permission);
}

// Note: getCountableObjectsRecursive() and getSizeOnVendorRecursive() are
// provided by the IDL `abstract` default bodies (return 0 / return 1). Defining
// them again here would produce duplicate-symbol link errors. See
// BlueprintDiskObject.idl.

void BlueprintDiskObjectImplementation::destroyObjectFromWorld(bool sendSelfDestroy) {
	// Refuse destroy while the disk still holds items. Every legitimate path
	// that destroys a loaded disk (deploy, recall, GM force-empty) removes
	// the contents first. This catches the radial "Destroy", drag-to-trash,
	// /destroy command, and any bulk-inventory-clear path with a single
	// check at the lowest common denominator.
	if (getContainerObjectsSize() > 0) {
		ManagedReference<SceneObject*> parent = getParent().get();
		if (parent != nullptr && parent->isPlayerCreature()) {
			CreatureObject* player = cast<CreatureObject*>(parent.get());
			if (player != nullptr) {
				player->sendSystemMessage("This blueprint disk still holds items. Deploy or recall it first, or use [MGMT] Force Empty.");
			}
		}
		info() << "Refused destroy on non-empty BlueprintDisk (OID: " << getObjectID()
		       << ", contents: " << getContainerObjectsSize() << ")";
		return;
	}

	TangibleObjectImplementation::destroyObjectFromWorld(sendSelfDestroy);
}

