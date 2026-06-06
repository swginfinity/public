/*
 * ForcePowersQueueCommand.h
 *
 *  Created on: 24/05/2010
 *      Author: Valkyra
 */

#ifndef FORCEPOWERSQUEUECOMMAND_H_
#define FORCEPOWERSQUEUECOMMAND_H_

#include"server/zone/ZoneServer.h"
#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/managers/combat/CombatManager.h"
#include "server/zone/managers/combat/CreatureAttackData.h"
#include "server/zone/managers/collision/CollisionManager.h"
#include "CombatQueueCommand.h"
#include "server/zone/managers/visibility/VisibilityManager.h"

class ForcePowersQueueCommand : public CombatQueueCommand {
public:

	ForcePowersQueueCommand(const String& name, ZoneProcessServer* server) : CombatQueueCommand(name, server) {
		visMod = 25;
	}

	int doCombatAction(CreatureObject* creature, const uint64& target, const UnicodeString& arguments = "") const {
		ManagedReference<SceneObject*> targetObject = server->getZoneServer()->getObject(target);

		if (targetObject == nullptr || !targetObject->isTangibleObject() || targetObject == creature)
			return INVALIDTARGET;

		float checkRange = range;

		if (creature->isProne())
			return NOPRONE;

		if(!checkDistance(creature, targetObject, checkRange))
			return TOOFAR;

		if (!CollisionManager::checkLineOfSight(creature, targetObject)) {
			creature->sendSystemMessage("@cbt_spam:los_fail");// "You lost sight of your target."
			// Infinity: Record LOS fail hint for /unstickTarget. See CombatQueueCommand.h.
			StringBuffer losKeyBuf;
			losKeyBuf << "los_fail_" << targetObject->getObjectID();
			creature->addCooldown(losKeyBuf.toString(), 60 * 1000);
			return GENERALERROR;
		}

		if (!playerEntryCheck(creature, targetObject->asTangibleObject())) {
			return GENERALERROR;
		}

		ManagedReference<PlayerObject*> ghost = creature->getPlayerObject();

		if (ghost != nullptr && ghost->getForcePower() < forceCost) {
			creature->sendSystemMessage("@jedi_spam:no_force_power"); //"You do not have enough Force Power to peform that action.
			return GENERALERROR;
		}

		CombatManager* combatManager = CombatManager::instance();

		if (combatManager == nullptr)
			return GENERALERROR;

		try {


			ManagedReference<WeaponObject*> weapon = creature->getWeapon();

			if (weapon == nullptr) {

				StringBuffer msg;

				msg << "Nullptr weapon in ForcePowersQueueCommand" << endl;
				msg << "Attacker Name:  " << creature->getFirstName() << endl;
				msg << "Attacker OID:  " << creature->getObjectID() << endl;

				if (creature->isAiAgent()) {

					ManagedReference<AiAgent*> agent = creature->asAiAgent();

					if (agent != nullptr) {
						const CreatureTemplate* creatureTemplate = agent->getCreatureTemplate();
						if (creatureTemplate!= nullptr) {
							msg << "AI template:  " << creatureTemplate->getTemplateName();
						}
					}
				}
				else if (creature->isPlayerCreature()) {
					msg << "Player:  Yes";
				}

				Logger::console.error(msg.toString());

				return GENERALERROR;
			}

			int res = combatManager->doCombatAction(creature, weapon, cast<TangibleObject*>(targetObject.get()), CreatureAttackData(arguments, this, target));

			switch (res) {
			case -1:
				return INVALIDTARGET;
			case -3:
				return GENERALERROR;
			}

			if (ghost != nullptr) {
				//creature->sendSystemMessage(name + " command executed, Reducing force by " + String::valueOf(forceCost));
				ghost->setForcePower(ghost->getForcePower() - forceCost);
			}

		} catch (Exception& e) {
			error("unreported exception caught in ForcePowersQueueCommand::doCombatAction");
			error(e.getMessage());
			e.printStackTrace();
		}

		// Increase Visibility for Force Power.
		if (ghost != nullptr)
			VisibilityManager::instance()->increaseVisibility(creature, visMod);

		return SUCCESS;
	}

	float getCommandDuration(CreatureObject *object, const UnicodeString& arguments) const {

		float combatHaste = object->getSkillMod("combat_haste") / 2.0f;   //Infinity:  combat haste was too effective for powers post rework, reducing by 50% for powers
		
		if (combatHaste > 0)
			return Math::max(speed * (1.f - (combatHaste / 100.f)), 1.0f);   // Speed cannot be lower than 1.0 seconds
		else
			return speed;
	}

	virtual bool isJediCombatQueueCommand() {
		return true;
	}

};

#endif /* FORCEPOWERSQUEUECOMMAND_H_ */
