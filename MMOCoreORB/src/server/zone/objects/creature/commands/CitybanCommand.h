/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef CITYBANCOMMAND_H_
#define CITYBANCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/objects/transaction/TransactionLog.h"

class CitybanCommand : public QueueCommand {
public:

	CitybanCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!checkStateMask(creature)) {
			creature->sendSystemMessage("You cannot use /cityban in your current state.");
			return INVALIDSTATE;
		}

		if (!checkInvalidLocomotions(creature)) {
			creature->sendSystemMessage("You cannot use /cityban in your current posture.");
			return INVALIDLOCOMOTION;
		}

		ZoneServer* zoneServer = creature->getZoneServer();
		if (zoneServer == nullptr)
			return GENERALERROR;

		ManagedReference<CreatureObject*> targetCreature = nullptr;

		// Infinity (#1463): Try target first, then fall through to name lookup
		if (target != 0) {
			ManagedReference<SceneObject*> targetObject = zoneServer->getObject(target);

			if (targetObject != nullptr && targetObject->isPlayerCreature() && targetObject != creature) {
				targetCreature = cast<CreatureObject*>(targetObject.get());
			}
		}

		// If no valid player target, try name from arguments
		if (targetCreature == nullptr) {
			String targetName;

			try {
				UnicodeTokenizer args(arguments);
				args.getStringToken(targetName);
			} catch (Exception& e) {
				creature->sendSystemMessage("SYNTAX: /cityBan <playerName> or target a player");
				return INVALIDPARAMETERS;
			}

			ManagedReference<PlayerManager*> playerManager = server->getPlayerManager();
			if (playerManager == nullptr)
				return GENERALERROR;

			targetCreature = playerManager->getPlayer(targetName);

			if (targetCreature == nullptr) {
				creature->sendSystemMessage("The specified player does not exist.");
				return INVALIDTARGET;
			}
		}

		ManagedReference<CityRegion*> city = creature->getCityRegion().get();

		if (city == nullptr) {
			creature->sendSystemMessage("@city/city:not_in_city"); //You must be in a city to use this command.
			return GENERALERROR;
		}

		Locker lock(city, creature);

		if (city->isBanned(targetCreature->getObjectID())) {
			creature->sendSystemMessage("That player is already banned from this city.");
			return INVALIDTARGET;
		}

		if (!city->isMilitiaMember(creature->getObjectID())) {
			creature->sendSystemMessage("@city/city:not_militia"); //You must be a member of the city militia to use this command.
			return GENERALERROR;
		}

		ManagedReference<PlayerObject*> ghost = targetCreature->getPlayerObject();

		if (ghost != nullptr && ghost->hasGodMode()) {
			//Can't ban a CSR
			creature->sendSystemMessage("@city/city:not_csr_ban"); //You cannot ban a Customer Service Representative from the city!

			StringIdChatParameter params("city/city", "csr_ban_attempt_msg");
			params.setTT(creature->getDisplayedName());
			params.setTO(city->getCityRegionName());

			targetCreature->sendSystemMessage(params); //%TT tried to /cityBan you from %TO!
			return GENERALERROR;
		}

		if (city->isCitizen(targetCreature->getObjectID())) {
			creature->sendSystemMessage("@city/city:not_citizen_ban"); //You can't city ban a citizen of the city!
			return GENERALERROR;
		}

		city->addBannedPlayer(targetCreature->getObjectID());
		city->removeSpecializationModifiers(targetCreature);

		TransactionLog trx(creature, targetCreature, city->getCityHall(), TrxCode::CITYBAN);
		trx.addState("cityName", city->getCityRegionName());
		trx.addState("cityRegionID", city->getObjectID());
		trx.addState("zone", city->getZone() != nullptr ? city->getZone()->getZoneName() : "unknown");

		StringIdChatParameter params("city/city", "city_banned");
		params.setTO(city->getCityRegionName());
		targetCreature->sendSystemMessage(params); //You have been banned from the city %TO.  You may no longer use any city services. 

		StringIdChatParameter params1("city/city", "city_ban_done");
		params1.setTT(targetCreature->getDisplayedName());
		creature->sendSystemMessage(params1); //%TT has been banned from the city and is no longer able to access city services.

		return SUCCESS;
	}

};

#endif //CITYBANCOMMAND_H_
