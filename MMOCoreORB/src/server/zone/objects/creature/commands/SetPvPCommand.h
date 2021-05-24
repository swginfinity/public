/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef SETPVPCOMMAND_H_
#define SETPVPCOMMAND_H_

#include "templates/faction/Factions.h"
#include "server/zone/objects/player/FactionStatus.h"

class SetPvPCommand : public QueueCommand {
public:

	SetPvPCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

                unsigned int faction = creature->getFaction();

                if (faction != Factions::FACTIONIMPERIAL && faction != Factions::FACTIONREBEL)
                        return GENERALERROR;

                if (creature->hasSkill("force_title_jedi_rank_03")) {
                    creature->sendSystemMessage("You must leave the FRS first.");
                        return GENERALERROR;
		}
		
                if (creature->getFutureFactionStatus() != -1)
                        return GENERALERROR;	

				if (!creature->checkCooldownRecovery("setpvp")){
					creature->sendSystemMessage("You have already used this command recently.");
					return GENERALERROR;
				}					

                int curStatus = creature->getFactionStatus();

				ManagedReference<PlayerObject*> player = creature->getPlayerObject();
		
				if (player != NULL){
					if (player->hasPvpTef()){
						creature->sendSystemMessage("You cannot use this while currently engaged in GCW combat.");
								return GENERALERROR;

					}
				}			
					
                if (curStatus == FactionStatus::OVERT)
                {
                        creature->sendSystemMessage("You will be flagged as Combatant in 5 minutes.");
                        creature->setFutureFactionStatus(FactionStatus::COVERT);

                        ManagedReference<CreatureObject*> creo = creature->asCreatureObject();

                        Core::getTaskManager()->scheduleTask([creo]{
                                ManagedReference<PlayerObject*> playerDelayed = creo->getPlayerObject();

							if (playerDelayed != NULL){
								if (playerDelayed->hasPvpTef()){
									creo->sendSystemMessage("You have recently engaged in GCW combat, request to leave special forces has been denied.");	
								}
								
                                if(creo != NULL && !playerDelayed->hasPvpTef()) {
                                    Locker locker(creo);
                                    creo->setFactionStatus(FactionStatus::COVERT);
                                }
							}	
                        }, "UpdateFactionStatusTask", 300000);
						creature->updateCooldownTimer("setpvp", 30000); // 30s cooldown
                }
                else
                {

                        creature->sendSystemMessage("You will be flagged as Special Forces in 30 seconds.");
                        creature->setFutureFactionStatus(FactionStatus::OVERT);

                        ManagedReference<CreatureObject*> creo = creature->asCreatureObject();

                        Core::getTaskManager()->scheduleTask([creo]{
                                if(creo != NULL) {
                                        Locker locker(creo);

                                        creo->setFactionStatus(FactionStatus::OVERT);
                                }
                        }, "UpdateFactionStatusTask", 30000);
						creature->updateCooldownTimer("setpvp", 30000); // 30s cooldown
				}

                return SUCCESS;
	}

};

#endif //SETPVPCOMMAND_H_
