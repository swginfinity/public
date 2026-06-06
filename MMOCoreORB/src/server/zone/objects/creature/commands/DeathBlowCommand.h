/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef DEATHBLOWCOMMAND_H_
#define DEATHBLOWCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/managers/creature/PetManager.h"
#include "server/zone/managers/player/PlayerManager.h"
#include "server/zone/managers/collision/CollisionManager.h"

class DeathBlowCommand : public QueueCommand {
public:

	DeathBlowCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		if (!creature->isPlayerCreature())
			return GENERALERROR;

		auto zoneServer = server->getZoneServer();
		if (zoneServer == nullptr)
			return GENERALERROR;

		ManagedReference<SceneObject*> targetObject = zoneServer->getObject(target);

		if (targetObject == nullptr || targetObject == creature) {
			return GENERALERROR;
		}

		if (!targetObject->isCreatureObject()) {
			return INVALIDTARGET;
		}

		if (!checkDistance(creature, targetObject, 5.0f)) {
			creature->sendSystemMessage("You must be within 5 meters of your target to perform a deathblow.");
			return TOOFAR;
		}

		if (!CollisionManager::checkLineOfSight(creature, targetObject)) {
			creature->sendSystemMessage("@combat_effects:cansee_fail");// You cannot see your target.
			return GENERALERROR;
		}

		//TODO: play coup_de_grace combat animations - ranged_coup_de_grace, melee_coup_de_grace, unarmed_coup_de_grace

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

			if (!playerEntryCheck(creature, player)) {
				return GENERALERROR;
			}

			if (!player->isIncapacitated() || player->isFeigningDeath()){
				creature->sendSystemMessage("@error_message:target_not_incapacitated"); //You cannot perform the death blow. Your target is not incapacitated.
				return GENERALERROR;
			}

			if (player->isAttackableBy(creature)) {

				auto playerManager = zoneServer->getPlayerManager();
				if (playerManager == nullptr)
					return GENERALERROR;

				playerManager->killPlayer(creature, player, 1);
			}
			else {
				creature->sendSystemMessage("@error_message:target_not_attackable"); //You cannot attack this target;
				return INVALIDTARGET;
			}
		}
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

			if (!playerEntryCheck(creature, pet)) {
				return GENERALERROR;
			}

			if (!pet->isIncapacitated()) {
				creature->sendSystemMessage("@error_message:target_not_incapacitated"); //You cannot perform the death blow. Your target is not incapacitated.
				return GENERALERROR;
			}

			ManagedReference<CreatureObject*> owner = pet->getLinkedCreature().get();
			if (owner == nullptr)
				return INVALIDTARGET;

			if (owner->isAttackableBy(creature)) {
				PetManager* petManager = zoneServer->getPetManager();
				if (petManager == nullptr)
					return GENERALERROR;

				petManager->killPet(creature, pet);
			}
			else {
				creature->sendSystemMessage("@error_message:target_not_attackable"); //You cannot attack this target;
				return INVALIDTARGET;
			}
		}
		else {
			return INVALIDTARGET;
		}

		if (targetObject->isPlayerCreature()) {
			StringIdChatParameter params("base_player", "prose_target_dead"); // %TT is no more.
			params.setTT(targetObject->getDisplayedName());
			creature->sendSystemMessage(params);
		}

		return SUCCESS;
	}

};

#endif //DEATHBLOWCOMMAND_H_
