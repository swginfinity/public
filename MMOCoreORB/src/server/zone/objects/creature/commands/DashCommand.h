/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef DASHCOMMAND_H_
#define DASHCOMMAND_H_

class DashCommand : public QueueCommand {
public:

	DashCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!checkStateMask(creature)) {
			if (creature->isRidingMount()) {
				creature->sendSystemMessage("@cbt_spam:no_burst");
			}
			return INVALIDSTATE;
		}

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		PlayerManager* playerManager = server->getZoneServer()->getPlayerManager();

		if (!playerManager->doDash(creature, 0.f, 0.f))
			return GENERALERROR;

		return SUCCESS;
	}

};

#endif //DASHCOMMAND_H_
