/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef SETPRIVACYCOMMAND_H_
#define SETPRIVACYCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/objects/scene/components/DataObjectComponentReference.h"
#include "server/zone/objects/tangible/components/vendor/VendorDataComponent.h"
#include "server/zone/managers/vendor/VendorManager.h"
#include "server/zone/ZoneProcessServer.h"

class SetprivacyCommand : public QueueCommand {
public:

	SetprivacyCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {
		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		ManagedReference<SceneObject*> obj = creature->getRootParent();

		if (obj == NULL || !obj->isBuildingObject()) {
			creature->sendSystemMessage("@player_structure:must_be_in_building"); //You must be in a building to do that.
			return INVALIDTARGET;
		}

		BuildingObject* building = cast<BuildingObject*>( obj.get());

		Locker clocker(building, creature);

		if (!building->isOnAdminList(creature)) {
			creature->sendSystemMessage("@player_structure:must_be_admin"); //You must be a building admin to do that.
			return INSUFFICIENTPERMISSION;
		}

		if (building->isGCWBase()) {
			creature->sendSystemMessage("@player_structure:no_privacy_faction_hq"); // You may not alter privacy of a factional headquarters.
			return GENERALERROR;
		}

		Reference<SharedBuildingObjectTemplate*> ssot = dynamic_cast<SharedBuildingObjectTemplate*>(building->getObjectTemplate());

		if (ssot == NULL || ssot->isAlwaysPublic()) {
			creature->sendSystemMessage("@player_structure:force_public"); //This structure is always public.
			return GENERALERROR;
		}


//We now allow vendors in private structures but disable the search
		for (int i = 1; i < building->getTotalCellNumber(); ++i) {
			ManagedReference<CellObject*> cell = building->getCell(i);

			if(cell == NULL)
				continue;

			for(int j = 0; j < cell->getContainerObjectsSize(); ++j) {
				ManagedReference<SceneObject*> obj = cell->getContainerObject(j);

				if(obj != NULL && obj->isVendor()) {
					DataObjectComponentReference* data = obj->getDataObjectComponent();
					if(data == NULL || data->get() == NULL || !data->get()->isVendorData()) {
						error("No vendor data found");
						return GENERALERROR;
					}
					VendorDataComponent* vendorData = cast<VendorDataComponent*>(data->get());
					if(vendorData == NULL) {
						error("Null vendor data");
						return GENERALERROR;
					}
					vendorData->setVendorSearchEnabled(false); //disable vendor search for all vendors in the now private structre
				}
			}
		}


		if (building->togglePrivacy()) {
			creature->sendSystemMessage("@player_structure:structure_now_public"); //This structure is now public
		} else {
			creature->sendSystemMessage("@player_structure:structure_now_private"); //This structure is now private
		}

		building->broadcastCellPermissions();

		return SUCCESS;
	}

};

#endif //SETPRIVACYCOMMAND_H_
