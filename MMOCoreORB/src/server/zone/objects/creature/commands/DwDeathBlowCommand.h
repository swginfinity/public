/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.

	Infinity: Death Watch DeathBlow Command
	Allows deathblow while mounted on DW jetpack
	Only works on enemy Death Watch players/pets who are incapacitated
	Requires ranged weapon, 70m range, accuracy curve favoring 30-40m
*/

#ifndef DWDEATHBLOWCOMMAND_H_
#define DWDEATHBLOWCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/managers/creature/PetManager.h"
#include "server/zone/managers/player/PlayerManager.h"
#include "server/zone/managers/death_watch/DeathWatchManager.h"
#include "server/zone/managers/collision/CollisionManager.h"
#include "server/zone/objects/tangible/weapon/WeaponObject.h"
#include "templates/faction/DeathWatchFactions.h"

class DwDeathBlowCommand : public QueueCommand {
public:

	DwDeathBlowCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {

	}

	/**
	 * Calculate deathblow accuracy based on distance
	 * Sweet spot is 30-40m (95% hit chance)
	 * Closer or further reduces accuracy
	 */
	int calculateDeathblowAccuracy(float distance) const {
		// Sweet spot: 30-40m = 95% accuracy
		// 0-10m: 60% (too close for a clean shot)
		// 10-30m: scales 60% to 95%
		// 30-40m: 95% (optimal range)
		// 40-60m: scales 95% to 70%
		// 60-70m: scales 70% to 50%

		if (distance < 10.0f) {
			return 60;  // Too close
		} else if (distance < 30.0f) {
			// Linear interpolation from 60% at 10m to 95% at 30m
			float t = (distance - 10.0f) / 20.0f;
			return 60 + (int)(t * 35.0f);
		} else if (distance <= 40.0f) {
			return 95;  // Sweet spot
		} else if (distance <= 60.0f) {
			// Linear interpolation from 95% at 40m to 70% at 60m
			float t = (distance - 40.0f) / 20.0f;
			return 95 - (int)(t * 25.0f);
		} else if (distance <= 70.0f) {
			// Linear interpolation from 70% at 60m to 50% at 70m
			float t = (distance - 60.0f) / 10.0f;
			return 70 - (int)(t * 20.0f);
		} else {
			return 30;  // Beyond max range
		}
	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {
		// Note: We intentionally skip checkStateMask and checkInvalidLocomotions
		// because this command is specifically designed to work while mounted

		// Must be a player
		if (!creature->isPlayerCreature()) {
			return GENERALERROR;
		}

		// Must have Death Watch novice skill
		if (!creature->hasSkill("combat_death_watch_novice")) {
			creature->sendSystemMessage("You must be Death Watch to use this command.");
			return GENERALERROR;
		}

		auto zoneServer = server->getZoneServer();
		if (zoneServer == nullptr) {
			return GENERALERROR;
		}

		// Must be on a Death Watch jetpack
		DeathWatchManager* dwManager = zoneServer->getDeathWatchManager();
		if (dwManager == nullptr) {
			return GENERALERROR;
		}

		if (!dwManager->isOnDeathWatchJetpack(creature)) {
			creature->sendSystemMessage("This command can only be used while mounted on a Death Watch jetpack.");
			return GENERALERROR;
		}

		// Must have a ranged weapon equipped
		ManagedReference<WeaponObject*> weapon = creature->getWeapon();
		if (weapon == nullptr || !weapon->isRangedWeapon() || weapon->isUnarmedWeapon()) {
			creature->sendSystemMessage("Deathblow requires a ranged weapon.");
			return GENERALERROR;
		}

		// Get target - fall back to look-at target if command target is 0
		// (Client may not send target for custom commands not in TRE)
		uint64 targetId = target;
		if (targetId == 0) {
			targetId = creature->getTargetID();
		}

		ManagedReference<SceneObject*> targetObject = zoneServer->getObject(targetId);

		if (targetObject == nullptr || targetObject == creature) {
			creature->sendSystemMessage("You must have a valid target.");
			return INVALIDTARGET;
		}

		// If target is a vehicle/mount, get the rider instead
		if (targetObject->isVehicleObject() || targetObject->isMount()) {
			ManagedReference<SceneObject*> rider = targetObject->getSlottedObject("rider");
			if (rider != nullptr && rider->isPlayerCreature()) {
				targetObject = rider;
			} else {
				creature->sendSystemMessage("Invalid target - no rider on vehicle.");
				return INVALIDTARGET;
			}
		}

		if (!targetObject->isCreatureObject()) {
			creature->sendSystemMessage("Invalid target type.");
			return INVALIDTARGET;
		}

		// Check range (70 meters for deathblow)
		float distance = creature->getDistanceTo(targetObject);
		if (distance > 70.0f) {
			creature->sendSystemMessage("Your target is out of range. Maximum: 70m");
			return TOOFAR;
		}

		// Check line of sight
		if (!CollisionManager::checkLineOfSight(creature, targetObject)) {
			creature->sendSystemMessage("@combat_effects:cansee_fail"); // You cannot see your target.
			return GENERALERROR;
		}

		// Validate target is an enemy Death Watch player or their pet
		TangibleObject* targetTano = targetObject->asTangibleObject();
		if (targetTano == nullptr) {
			creature->sendSystemMessage("Invalid target.");
			return INVALIDTARGET;
		}

		if (!dwManager->isValidDwJetpackTarget(creature, targetTano)) {
			creature->sendSystemMessage("You can only use this on enemy Death Watch targets.");
			return INVALIDTARGET;
		}

		// Calculate accuracy based on distance (sweet spot 30-40m)
		int accuracy = calculateDeathblowAccuracy(distance);

		// Apply angle modifier - must be facing target
		float facingOffset = dwManager->getFacingOffsetAngle(creature, targetObject);
		if (facingOffset > 45.0f) {
			// Significant penalty for not facing target
			accuracy -= (int)((facingOffset - 45.0f) / 2.0f);
		} else if (facingOffset <= 5.0f) {
			// Bonus for dead-on aim
			accuracy += 5;
		}

		// Apply dw_acc skill bonus
		int dwAcc = creature->getSkillMod("dw_acc");
		accuracy += dwAcc / 4;  // Up to +25% at max skill

		// Clamp accuracy
		if (accuracy < 20) accuracy = 20;
		if (accuracy > 95) accuracy = 95;

		// Roll for hit
		int roll = System::random(99) + 1;  // 1-100
		if (roll > accuracy) {
			creature->sendSystemMessage("Your deathblow shot missed!");
			return SUCCESS;  // Command executed, just missed
		}

		// Handle player target
		if (targetObject->isPlayerCreature()) {
			ManagedReference<CreatureObject*> player = cast<CreatureObject*>(targetObject.get());
			if (player == nullptr) {
				return INVALIDTARGET;
			}

			Locker clocker(player, creature);

			if (player->isDead()) {
				StringIdChatParameter params("error_message", "prose_target_already_dead"); // But %TT is already dead!
				params.setTT(player->getDisplayedName());
				creature->sendSystemMessage(params);
				return GENERALERROR;
			}

			if (!player->isIncapacitated() || player->isFeigningDeath()) {
				creature->sendSystemMessage("@error_message:target_not_incapacitated"); // You cannot perform the death blow. Your target is not incapacitated.
				return GENERALERROR;
			}

			if (player->isAttackableBy(creature)) {
				auto playerManager = zoneServer->getPlayerManager();
				if (playerManager == nullptr) {
					return GENERALERROR;
				}

				playerManager->killPlayer(creature, player, 1);

				StringIdChatParameter params("base_player", "prose_target_dead"); // %TT is no more.
				params.setTT(player->getDisplayedName());
				creature->sendSystemMessage(params);
			} else {
				creature->sendSystemMessage("@error_message:target_not_attackable"); // You cannot attack this target
				return INVALIDTARGET;
			}
		}
		// Handle pet target
		else if (targetObject->isPet()) {
			ManagedReference<AiAgent*> pet = cast<AiAgent*>(targetObject.get());
			if (pet == nullptr) {
				return INVALIDTARGET;
			}

			Locker clocker(pet, creature);

			if (pet->isDead()) {
				StringIdChatParameter params("error_message", "prose_target_already_dead"); // But %TT is already dead!
				params.setTT(pet->getDisplayedName());
				creature->sendSystemMessage(params);
				return GENERALERROR;
			}

			if (!pet->isIncapacitated()) {
				creature->sendSystemMessage("@error_message:target_not_incapacitated"); // You cannot perform the death blow. Your target is not incapacitated.
				return GENERALERROR;
			}

			ManagedReference<CreatureObject*> owner = pet->getLinkedCreature().get();
			if (owner == nullptr) {
				return INVALIDTARGET;
			}

			if (owner->isAttackableBy(creature)) {
				PetManager* petManager = zoneServer->getPetManager();
				if (petManager == nullptr) {
					return GENERALERROR;
				}

				petManager->killPet(creature, pet);

				StringIdChatParameter params("base_player", "prose_target_dead"); // %TT is no more.
				params.setTT(pet->getDisplayedName());
				creature->sendSystemMessage(params);
			} else {
				creature->sendSystemMessage("@error_message:target_not_attackable"); // You cannot attack this target
				return INVALIDTARGET;
			}
		} else {
			creature->sendSystemMessage("Invalid target - must be a player or pet.");
			return INVALIDTARGET;
		}

		return SUCCESS;
	}

};

#endif //DWDEATHBLOWCOMMAND_H_
