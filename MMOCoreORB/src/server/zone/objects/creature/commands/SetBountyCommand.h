/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef SETBOUNTYCOMMAND_H_
#define SETBOUNTYCOMMAND_H_


#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/ZoneServer.h"
#include "server/zone/managers/mission/MissionManager.h"
#include "server/zone/objects/player/sui/callbacks/BountyHuntSuiCallback.h"
#include "server/zone/objects/player/sui/inputbox/SuiInputBox.h"

class SetBountyCommand : public QueueCommand {
public:

	SetBountyCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		if (creature == nullptr)
			return GENERALERROR;

		ManagedReference<PlayerObject*> ghost = creature->getPlayerObject();

		if (ghost == nullptr)
			return GENERALERROR;

		String lastDeathTimeString = ghost->getScreenPlayData("pvp", "last_death_timestamp");
		uint64 lastDeathTimeMili;
		if (lastDeathTimeString != "") {
			lastDeathTimeMili = Long::unsignedvalueOf(lastDeathTimeString);
		}
		else {
			creature->sendSystemMessage("You have no PVP death on record.");
			return GENERALERROR;
		}

		String bountyPlacedTimeString = ghost->getScreenPlayData("pvp", "bounty_placed_timestamp");
		uint64 bountyPlacedTimeMili = 0;
		if (bountyPlacedTimeString != "") {
			bountyPlacedTimeMili = Long::unsignedvalueOf(bountyPlacedTimeString);
		}

		if (bountyPlacedTimeMili >= lastDeathTimeMili) {
			creature->sendSystemMessage("You have already placed a bounty for your last PVP death.");
			return GENERALERROR;
		}

		Time currentTime;
		uint64 deathDelta = currentTime.getMiliTime() - lastDeathTimeMili;

		if (deathDelta >= 1000 * 60 * 30) { // 30 minutes
			creature->sendSystemMessage("Too much time has elapsed since your PVP death. You are unable to place a bounty.");
			return GENERALERROR;
		}

		String killerName = ghost->getScreenPlayData("pvp", "last_killer");
		ManagedReference<CreatureObject*> killerCreature = server->getPlayerManager()->getPlayer(killerName);

		if (killerCreature == nullptr) {
			creature->sendSystemMessage("Unable to identify player killer.  Please report the issue in support channel on Discord.");
			return INVALIDPARAMETERS;
		}

		auto killerGhost = killerCreature->getPlayerObject();

		if (killerGhost == nullptr) {
			creature->sendSystemMessage("Unable to identify player killer.  Please report the issue in support channel on Discord.");
			return INVALIDPARAMETERS;
		}

		if (!killerGhost->hasGodMode()) {
			ManagedReference<SuiInputBox*> box = new SuiInputBox(creature, SuiWindowType::OBJECT_NAME);
			box->setPromptTitle("You have died in PVP");
			box->setPromptText("Place a bounty on your killer, " + killerName + ". Enter an amount between 25,000 and 2,500,000 credits. The Bounty Hunter Guild will take 20% for its fee and your target will be added to our boards immediately.");
			box->setMaxInputSize(128);
			box->setCancelButton(true, "@no");
			box->setOkButton(true, "@yes");
			box->setUsingObject(creature);
			box->setCallback(new BountyHuntSuiCallback(creature->getZoneServer()));
			ghost->addSuiBox(box);
			creature->sendMessage(box->generateMessage());
		}
		else {
			creature->sendSystemMessage("You cannot place a bounty on a staff character.");
		}
		
		return SUCCESS;
	}

};

#endif //SETBOUNTYCOMMAND_H_
