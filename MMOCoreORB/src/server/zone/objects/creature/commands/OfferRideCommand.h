/*
			Copyright <SWGEmu>
	See file COPYING for copying conditions.*/

#ifndef OFFERRIDECOMMAND_H_
#define OFFERRIDECOMMAND_H_

// Infinity: stub implementation — offerRide is in the game command tables
// but not yet implemented. Return SUCCESS to prevent error spam.
class OfferRideCommand : public QueueCommand {
public:

	OfferRideCommand(const String& name, ZoneProcessServer* server) : QueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		return SUCCESS;
	}

};

#endif //OFFERRIDECOMMAND_H_
