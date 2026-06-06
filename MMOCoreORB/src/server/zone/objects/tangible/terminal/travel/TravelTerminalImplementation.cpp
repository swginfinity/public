/*
 * TravelTerminalImplementation.cpp
 *
 *  Created on: 31/05/2010
 *      Author: victor
 *  Updated on: Thu Oct 13 08:34:42 PDT 2011 by lordkator - use getPlanetTravelPoint() instead of trying to resolve here
 */

#include "server/zone/objects/tangible/terminal/travel/TravelTerminal.h"
#include "server/zone/packets/player/EnterTicketPurchaseModeMessage.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/objects/player/PlayerObject.h"
#include "server/zone/objects/player/sui/SuiWindowType.h"

int TravelTerminalImplementation::handleObjectMenuSelect(CreatureObject* player, byte selectedID) {
	if (selectedID != 20) {
		return 0;
	}

	if (player == nullptr) {
		return 0;
	}

	auto ghost = player->getPlayerObject();

	if (ghost == nullptr) {
		return 0;
	}

	// Prevent duplicate SUI — client crashes on overlapping ticket purchase windows
	if (ghost->hasSuiBoxWindowType(SuiWindowType::TICKET_PURCHASE_MESSAGE)) {
		return 0;
	}

	Reference<PlanetTravelPoint*> ptp = getPlanetTravelPoint();

	if (ptp == nullptr) {
		error() << "TravelTerminalImplementation::handleObjectMenuSelect(" << getObjectID()
				<< ") Player: " << player->getObjectID() << " - Could not determine related PlanetTravelPoint";
		return 0;
	}

	EnterTicketPurchaseModeMessage* etpm = new EnterTicketPurchaseModeMessage(ptp);
	player->sendMessage(etpm);

	return 0;
}

void TravelTerminalImplementation::notifyInsertToZone(Zone* zone) {
	TerminalImplementation::notifyInsertToZone(zone);
}
