/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef CITYPARDONCOMMAND_H_
#define CITYPARDONCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/objects/transaction/TransactionLog.h"

class CitypardonCommand : public QueueCommand {
public:

	CitypardonCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {
		if (!checkStateMask(creature)) {
			creature->sendSystemMessage("You cannot use /citypardon in your current state.");
			return INVALIDSTATE;
		}

		if (!checkInvalidLocomotions(creature)) {
			creature->sendSystemMessage("You cannot use /citypardon in your current posture.");
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
				creature->sendSystemMessage("SYNTAX: /cityPardon <playerName> or target a player");
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

		if (!city->isBanned(targetCreature->getObjectID())) {
			creature->sendSystemMessage("That player is not banned from this city.");
			return INVALIDTARGET;
		}

		if (!city->isMilitiaMember(creature->getObjectID())) {
			creature->sendSystemMessage("@city/city:not_militia"); //You must be a member of the city militia to use this command.
			return GENERALERROR;
		}

		city->removeBannedPlayer(targetCreature->getObjectID());
		city->applySpecializationModifiers(targetCreature);

		TransactionLog trx(creature, targetCreature, city->getCityHall(), TrxCode::CITYPARDON);
		trx.addState("cityName", city->getCityRegionName());
		trx.addState("cityRegionID", city->getObjectID());
		trx.addState("zone", city->getZone() != nullptr ? city->getZone()->getZoneName() : "unknown");

		StringIdChatParameter params("city/city", "city_pardoned"); //You have been pardoned by the city %TO and are once again able to use city services.
		params.setTO(city->getCityRegionName());
		targetCreature->sendSystemMessage(params);

		StringIdChatParameter params1("city/city", "city_pardon_done"); //%TT has been pardoned and is now able to use city services.
		params1.setTT(targetCreature->getDisplayedName());
		creature->sendSystemMessage(params1);

		return SUCCESS;
	}

};

#endif //CITYPARDONCOMMAND_H_
