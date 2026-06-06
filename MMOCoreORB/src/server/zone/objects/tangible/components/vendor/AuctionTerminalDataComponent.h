/*
 * AuctionTerminalDataComponent.h
 *
 *  Created on: 5/27/2012
 *      Author: kyle
 */

#ifndef AUCTIONTERMINALDATACOMPONENT_H_
#define AUCTIONTERMINALDATACOMPONENT_H_

#include "server/zone/objects/scene/components/DataObjectComponent.h"
#include "server/zone/managers/auction/AuctionManager.h"
#include "server/zone/objects/region/CityRegion.h"
#include "server/zone/Zone.h"

class AuctionTerminalDataComponent : public DataObjectComponent {
protected:

	ManagedWeakReference<AuctionManager*> auctionMan;
	SerializableString uid;

	// Vendor Pack-Up (Phase 1): when true, updateUID() is a no-op so
	// the vendor's UID stays pinned at its pre-pack value while the vendor
	// is dormant inside a VendorControlDevice. Without this guard, the
	// vendor's getZone()/getWorldPositionX walks resolve up through the
	// player creature when packed, producing garbage UIDs and corrupting
	// AuctionsMap entries.
	bool suppressUIDRebuild = false;

public:
	AuctionTerminalDataComponent() {
		uid = "";
		addSerializableVariables();
	}

	virtual ~AuctionTerminalDataComponent() {

	}

	void writeJSON(nlohmann::json& j) const {
		DataObjectComponent::writeJSON(j);

		SERIALIZE_JSON_MEMBER(uid);
		SERIALIZE_JSON_MEMBER(suppressUIDRebuild);
	}

	void initializeTransientMembers() {
		ManagedReference<SceneObject*> strongParent = parent.get();
		if(strongParent != nullptr && strongParent->getZoneServer() != nullptr) {
			auctionMan = strongParent->getZoneServer()->getAuctionManager();
			if(uid.isEmpty())
				updateUID();
		}
	}

	void updateUID() {
		// Vendor Pack-Up (Phase 1): suppress while dormant. Must be checked
		// BEFORE any container moves during pack so the cascade of
		// setInitialized/region-change/notifyContainerChange events that
		// fire during the pack flow do not rewrite AuctionsMap entries
		// using the player's parent-chain zone.
		if (suppressUIDRebuild)
			return;

		ManagedReference<SceneObject*> strongParent = parent.get();
		ManagedReference<AuctionManager*> auctionManager = auctionMan.get();

		if(auctionManager == nullptr || strongParent == nullptr || strongParent->getZone() == nullptr)
			return;

		String olduid = uid;

		uid = strongParent->getZone()->getZoneName() + ".";

		String region = "@planet_n:" + strongParent->getZone()->getZoneName();
		ManagedReference<CityRegion*> cityRegion = strongParent->getCityRegion().get();
		if(cityRegion != nullptr)
			region = cityRegion->getCityRegionName();

		uid += region + "." + strongParent->getDisplayedName() + ".";
		uid += String::valueOf(strongParent->getObjectID()) + "#";
		uid += String::valueOf(((int)strongParent->getWorldPositionX())) + "," + String::valueOf(((int)strongParent->getWorldPositionY()));

		if(olduid != uid)
			auctionManager->updateVendorUID(strongParent, olduid, uid);
	}

	String getUID() {
		return uid;
	}

	bool isAuctionTerminalData() {
		return true;
	}

	// Vendor Pack-Up (Phase 1): toggled by AuctionManager::prePackVendor /
	// postUnpackVendor. Persisted via addSerializableVariables so a packed
	// vendor that's saved + reloaded doesn't lose its dormant state.
	void setSuppressUIDRebuild(bool suppress) {
		suppressUIDRebuild = suppress;
	}

	bool isSuppressUIDRebuild() const {
		return suppressUIDRebuild;
	}

private:
	void addSerializableVariables() {
		addSerializableVariable("uid", &uid);
		addSerializableVariable("suppressUIDRebuild", &suppressUIDRebuild);
	}
};


#endif /* AUCTIONTERMINALDATACOMPONENT_H_ */
