/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef ROCKETBOOTSCOMMAND_H_
#define ROCKETBOOTSCOMMAND_H_

class RocketBootsCommand : public QueueCommand {
public:

	RocketBootsCommand(const String& name, ZoneProcessServer* server)
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

		if (!playerManager->doRocketBoots(creature, 0.f, 0.f))
			return GENERALERROR;

		return SUCCESS;
	}

};

#endif //ROCKETBOOTSCOMMAND_H_
