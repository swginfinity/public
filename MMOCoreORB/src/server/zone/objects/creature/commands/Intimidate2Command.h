/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef INTIMIDATE2COMMAND_H_
#define INTIMIDATE2COMMAND_H_

#include "server/zone/packets/object/CombatSpam.h"
#include "CombatQueueCommand.h"

class Intimidate2Command : public CombatQueueCommand {
public:

	Intimidate2Command(const String& name, ZoneProcessServer* server)
		: CombatQueueCommand(name, server) {
	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		Reference<TangibleObject*> targetObject = server->getZoneServer()->getObject(target).castTo<TangibleObject*>();

		if (targetObject == nullptr || !targetObject->isCreatureObject())
			return INVALIDTARGET;

		//Infinity:  Custom checks through cooldown
		ManagedReference<WeaponObject*> weapon = creature->getWeapon();
		if (weapon != nullptr && weapon->isJediWeapon())  // Jedi can't use brawler intimidate
			return INVALIDWEAPON;

		CreatureObject* tarCreo = targetObject->asCreatureObject();
		if (tarCreo == nullptr)
			return INVALIDTARGET;

		if (creature->isPlayerCreature() && (tarCreo->isPlayerCreature() || tarCreo->isPet()) && !tarCreo->checkCooldownRecovery("intimidate")) {
			creature->sendSystemMessage("Your target has been recently intimidated and cannot be intimidated again at this time.");
			return GENERALERROR;
		}

		int res = doCombatAction(creature, target);

		if (res == TOOFAR && creature->isPlayerCreature()) {
			CombatSpam* msg = new CombatSpam(creature, targetObject, creature, nullptr, 0, "cbt_spam", "intim_out_of_range", 0);
			creature->sendMessage(msg);
		}
		
		if (res == SUCCESS && creature->isPlayerCreature()) {
			ManagedReference<PlayerObject*> ghost = creature->getPlayerObject();

			if (ghost != nullptr && !ghost->getCommandMessageString(STRING_HASHCODE("intimidate2")).isEmpty() && creature->checkCooldownRecovery("command_message")) {
				UnicodeString shout(ghost->getCommandMessageString(STRING_HASHCODE("intimidate2")));
				server->getChatManager()->broadcastChatMessage(creature, shout, 0, 80, creature->getMoodID(), 0, ghost->getLanguageID());
				creature->updateCooldownTimer("command_message", 30 * 1000);
			}
		}

		//Infinity:  Check for PVP targets in earea
		if (res != TOOFAR && res != INVALIDTARGET && (tarCreo->isPlayerCreature() || tarCreo->isPet()) && creature->isPlayerCreature()) {
			Locker clocker(tarCreo, creature);
			ThreatMap* threatMap = tarCreo->getThreatMap();
			clocker.release();
			if (threatMap != nullptr) {
				int attackerCount = 0;
				ThreatMap copyThreatMap(*threatMap);
				for (int i = 0; i < copyThreatMap.size(); ++i) {
					ThreatMapEntry* entry = &copyThreatMap.elementAt(i).getValue();
					TangibleObject* attacker = copyThreatMap.elementAt(i).getKey();

					if (entry == nullptr || attacker == nullptr || !attacker->isPlayerCreature())
						continue;

					if (attacker->isInRange(tarCreo, 100.0f)) {
						attackerCount++;
					}
				}

				if (attackerCount > 1) {
					//Infinity:   Intimidate Cooldown
					tarCreo->updateCooldownTimer("intimidate", 6000);
				}
			}
		}

		return res;
	}

};

#endif //INTIMIDATE2COMMAND_H_
