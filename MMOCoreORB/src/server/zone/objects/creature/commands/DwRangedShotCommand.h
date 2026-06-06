/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.

	Infinity: Death Watch Ranged Shot Command
	Basic ranged attack while mounted on DW jetpack

	Architecture: ALL combat logic in C++ for consistency
	- Uses weapon harmonic mean damage: 2*min*max/(min+max)
	  This rewards balanced weapons and penalizes "monster weapons"
	- Uses weapon attack speed for cooldown (min 1 second)
	- Lua stub exists only for future extensibility (effects, etc.)
*/

#ifndef DWRANGEDSHOTCOMMAND_H_
#define DWRANGEDSHOTCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/managers/director/DirectorManager.h"
#include "server/zone/managers/death_watch/DeathWatchManager.h"
#include "server/zone/objects/tangible/weapon/WeaponObject.h"
#include "server/zone/objects/player/PlayerObject.h"
#include "templates/SharedObjectTemplate.h"

class DwRangedShotCommand : public QueueCommand {
public:

	DwRangedShotCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {

	}

	// Override to use weapon attack speed for queue timing
	float getCommandDuration(CreatureObject* creature, const UnicodeString& arguments) const override {
		if (creature == nullptr) {
			return defaultTime;
		}

		ManagedReference<WeaponObject*> weapon = creature->getWeapon();
		if (weapon == nullptr) {
			return defaultTime;
		}

		// Use weapon attack speed, minimum 1 second
		float speed = weapon->getAttackSpeed();
		return Math::max(1.0f, speed);
	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const override {
		auto zoneServer = server->getZoneServer();
		if (zoneServer == nullptr) {
			return GENERALERROR;
		}

		DeathWatchManager* dwManager = zoneServer->getDeathWatchManager();
		if (dwManager == nullptr) {
			return GENERALERROR;
		}

		// Centralized validation (skill, jetpack, speed, target, range)
		uint64 targetId = 0;
		int validationResult = dwManager->validateJetpackAttack(creature, target, targetId, true); // true = ranged
		if (validationResult != 0) {
			return GENERALERROR;
		}

		// Note: Cooldown is handled by getCommandDuration() and the combat queue system

		// Execute attack
		// Parameters: damageMultiplier=1.0, accuracyMod=0 (normal), damageType=2 (split H/A)
		int damage = dwManager->executeDwRangedAttack(creature, targetId, 1.0f, 0, 2);

		if (damage < 0) {
			// Attack failed - error message already sent by executeDwRangedAttack
			return GENERALERROR;
		}

		// Send combat feedback
		if (damage > 0) {
			ManagedReference<SceneObject*> finalTargetObj = zoneServer->getObject(targetId);
			String targetName = "target";
			if (finalTargetObj != nullptr && finalTargetObj->isCreatureObject()) {
				CreatureObject* targetCreature = finalTargetObj->asCreatureObject();
				// If target is a vehicle/mount, get the rider's name instead
				if (targetCreature->isVehicleObject() || targetCreature->isMount()) {
					ManagedReference<CreatureObject*> rider = targetCreature->getLinkedCreature();
					if (rider != nullptr) {
						targetName = rider->getFirstName();
					}
				} else {
					targetName = targetCreature->getFirstName();
				}
			}

			StringBuffer msg;
			msg << "You hit " << targetName << " for " << damage << " damage.";
			creature->sendSystemMessage(msg.toString());
		}

		return SUCCESS;
	}

};

#endif //DWRANGEDSHOTCOMMAND_H_
