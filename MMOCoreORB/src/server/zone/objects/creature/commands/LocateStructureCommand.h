/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions. */

// Thanks to SWG Remastered for the initial code

#ifndef LOCATESTRUCTURECOMMAND_H_
#define LOCATESTRUCTURECOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/objects/player/sui/messagebox/SuiMessageBox.h"


class LocateStructureCommand : public QueueCommand {
public:

	LocateStructureCommand(const String& name, ZoneProcessServer* server)
	: QueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		PlayerObject* ghost = creature->getPlayerObject();

		if (ghost == nullptr)
			return GENERALERROR;

		if (!creature->checkCooldownRecovery("locateStructure"))  {
			Time* cooldownTime = creature->getCooldownTime("locateStructure");
			if (cooldownTime != nullptr) {
				float timeLeft = round(fabs(cooldownTime->miliDifference() / 1000.f));
				creature->sendSystemMessage("You can use the locateStructure command again in " + String::valueOf(timeLeft) + " second" + ((timeLeft == 1.0f) ? "." : "s."));
				return GENERALERROR;
			} 
			else {
				creature->sendSystemMessage("You can only use the locateStructure command once per minute.");
                return GENERALERROR;
			}
		}
		creature->updateCooldownTimer("locateStructure", 60000);

		int lotsRemaining = ghost->getLotsRemaining();

		StringBuffer body;
		String zoneName;
		String structureName;

		body << "Unused Lots:\t" << String::valueOf(lotsRemaining) << endl << endl;
		body << "Player Structures:";

		for (int i = 0; i < ghost->getTotalOwnedStructureCount(); i++) {
			ManagedReference<StructureObject*> structure = creature->getZoneServer()->getObject(ghost->getOwnedStructure(i)).castTo<StructureObject*>();

			int num = i + 1;
			body << endl << String::valueOf(num) << ".\t";

			if (structure == nullptr) {
				body << "NULL Structure" << endl << endl;
				continue;
			}

			if (structure->getCustomObjectName().isEmpty())
				structureName = structure->getDisplayedName();
			else
				structureName = structure->getCustomObjectName().toString();
			if (num < 10)
				body << "\tName:\t" << structureName << endl;
			else
				body << "Name:\t" << structureName << endl;
			
			body << "\t\tType:\t\t" << structure->getObjectTemplate()->getTemplateFileName() << endl;
			body << "\t\tLots:\t\t" << String::valueOf(structure->getLotSize()) << endl;

			body << "\t\tPlanet:\t";
			Zone* zone = structure->getZone();
			if (zone == nullptr) {
				body << "NULL" << endl;
			} else {
				zoneName = zone->getZoneName();
				zoneName[0] = toupper(zoneName[0]);
				body << zoneName << endl;
				body << "\t\tLocation:\t" << ceil(structure->getWorldPositionX()) << ", " << ceil(structure->getWorldPositionY()) << endl;
			}
		}

		ManagedReference<SuiMessageBox*> box = new SuiMessageBox(creature, 0);
		box->setPromptTitle("Player Structure Info");
		box->setPromptText(body.toString());
		box->setUsingObject(creature);
		box->setForceCloseDisabled();

		ghost->addSuiBox(box);
		creature->sendMessage(box->generateMessage());

		ChatManager* chatManager = server->getZoneServer()->getChatManager();
			if (chatManager != nullptr)
				chatManager->sendMail("System", "Lot and Structure Information for " + creature->getFirstName(), body.toString(), creature->getFirstName());

		return SUCCESS;
	}


};

#endif //LocateStructureCOMMAND_H_
