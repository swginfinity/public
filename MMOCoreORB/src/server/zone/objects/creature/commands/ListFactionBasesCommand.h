/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions. */

// Thanks to SWG Remastered for the initial code

#ifndef LISTFACTIONBASES_H_
#define LISTFACTIONBASES_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/objects/building/BuildingObject.h"
#include "server/zone/Zone.h"
#include "server/zone/ZoneServer.h"
#include "server/zone/managers/gcw/GCWManager.h"



class ListFactionBasesCommand : public QueueCommand {
public:

	ListFactionBasesCommand(const String& name, ZoneProcessServer* server)
	: QueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		ManagedReference<PlayerObject*> ghost = creature->getPlayerObject();

		if (ghost == nullptr)
			return GENERALERROR;
		
		if (!ghost->isAdmin())
			return GENERALERROR;

		StringTokenizer args(arguments.toString());
		String zoneName = "";

		if (!args.hasMoreTokens())  {
			creature->sendSystemMessage("SYNTAX: /listFactionBases <zoneName> [DELETE]");
			return INVALIDPARAMETERS;
		}
		args.getStringToken(zoneName);

		zoneName = zoneName.toLowerCase();

		Zone* planetZone = creature->getZoneServer()->getZone(zoneName);

		zoneName[0] = toupper(zoneName[0]);

		if (planetZone == nullptr) {
			creature->sendSystemMessage("Zone \"" + zoneName + "\" cannot be found.  Invalid zone.");
			return INVALIDPARAMETERS;
		}

		GCWManager* gcwMan = planetZone->getGCWManager();

		if (gcwMan == nullptr) {
			creature->sendSystemMessage("There is no GCW manager on " + zoneName + ".");
			return GENERALERROR;
		}
		
		if (!creature->checkCooldownRecovery("listFactionBases"))  {
			const Time* cooldownTime = creature->getCooldownTime("listFactionBases");
			if (cooldownTime != nullptr) {
				float timeLeft = fabs(cooldownTime->miliDifference()) / 1000.f;
				creature->sendSystemMessage("You can use the listFactionBases command again in " + String::format("%.1f", timeLeft) + " second" + ((timeLeft == 1.0f) ? "." : "s."));
				return GENERALERROR;
			} 
			else {
				creature->sendSystemMessage("You can only use the listFactionBases command once every 5 seconds.");
                return GENERALERROR;
			}
		}
		creature->updateCooldownTimer("listFactionBases", 5000);

		creature->sendSystemMessage("Scanning all of the faction base data may take a while, so please be patient.");

		bool destroy = false;
		bool pveOnly = false;
		if (args.hasMoreTokens()) {
			String destroyArg = "";
			args.getStringToken(destroyArg);
			if (destroyArg == "DESTROY") {
				destroy = true;
				if (args.hasMoreTokens()) {
					String pveArg = "";
					args.getStringToken(pveArg);
					pveArg = pveArg.toLowerCase();
					if (pveArg == "pveonly") {
						pveOnly = true;
					}
				}
			}
		}

		gcwMan->listBases(creature, destroy, pveOnly);

		return SUCCESS;

	}

};

#endif //ListFactionBasesCOMMAND_H_
