/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef SETPVPCOMMAND_H_
#define SETPVPCOMMAND_H_

#include "templates/faction/Factions.h"
#include "server/zone/objects/player/FactionStatus.h"
#include "server/zone/managers/director/DirectorManager.h"

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

                if (creature->getFutureFactionStatus() != -1) {
                		creature->sendSystemMessage("You are already in the process of changing your faction status.");
                        return GENERALERROR;	
                }

				if (!creature->checkCooldownRecovery("setpvp")){
					creature->sendSystemMessage("You have already used this command recently.");
					return GENERALERROR;
				}					

                int curStatus = creature->getFactionStatus();

				ManagedReference<PlayerObject*> player = creature->getPlayerObject();
		
				if (player != nullptr){
					if (player->hasPvpTef()){
						creature->sendSystemMessage("You cannot use this while currently engaged in PVP combat.");
						return GENERALERROR;
					}
				}			
					
                if (curStatus == FactionStatus::OVERT) {

                    if (creature->hasSkill("force_title_jedi_rank_03") || creature->hasSkill("force_rank_dark_novice") || creature->hasSkill("force_rank_light_novice") ){
                        creature->sendSystemMessage("@faction_recruiter:jedi_cant_go_covert");
                        return GENERALERROR;
		            }

                    creature->sendSystemMessage("You will be flagged as a Combatant in 5 minutes.");

                    Lua* lua = DirectorManager::instance()->getLuaInstance();
                    Reference<LuaFunction*> luaSetPVPgoCovert = lua->createFunction("SetPVPCommand", "goCovert", 0);
                    *luaSetPVPgoCovert << creature;
                    luaSetPVPgoCovert->callFunction();
                }
                else {
                    
                    creature->sendSystemMessage("You will be flagged as Special Forces in 30 seconds.");

                    Lua* lua = DirectorManager::instance()->getLuaInstance();
                    Reference<LuaFunction*> luaSetPVPgoOvert = lua->createFunction("SetPVPCommand", "goOvert", 0);
                    *luaSetPVPgoOvert << creature;
                    luaSetPVPgoOvert ->callFunction();                                
				}
                creature->updateCooldownTimer("setpvp", 30000); // 30s cooldown
                return SUCCESS;
	}

};

#endif //SETPVPCOMMAND_H_
