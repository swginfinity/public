/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef FIREHEAVYWEAPONCOMMAND_H_
#define FIREHEAVYWEAPONCOMMAND_H_

#include "server/zone/managers/death_watch/DeathWatchManager.h"

class FireHeavyWeaponCommand : public CombatQueueCommand {
public:

	FireHeavyWeaponCommand(const String& name, ZoneProcessServer* server)
		: CombatQueueCommand(name, server) {
	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		StringTokenizer tokenizer(arguments.toString());
		if (!tokenizer.hasMoreTokens())
			return INVALIDPARAMETERS;

		try {

			uint64 weaponID = tokenizer.getLongToken();
			Reference<WeaponObject*> weapon = server->getZoneServer()->getObject(weaponID).castTo<WeaponObject*>();
			if (weapon == nullptr || !weapon->isHeavyWeapon())
				return INVALIDPARAMETERS;

			if (!weapon->isASubChildOf(creature))
				return GENERALERROR;

			ManagedReference<TangibleObject*> targetObject = server->getZoneServer()->getObject(target).castTo<TangibleObject*>();
			if (targetObject == nullptr)
				return GENERALERROR;

			if (!(targetObject->isAttackableBy(creature)))
				return GENERALERROR;

			// =====================================================================
			// DEATH WATCH HEAVY WEAPON INTERCEPTION
			// =====================================================================
			// Check if this is DW-vs-DW combat with target on jetpack
			// Supports: Rocket Launcher, Acid Beam, Lightning Beam
			// Target can be player directly OR their jetpack (damage transfers to rider)
			// If so, use DW combat system instead of normal CombatManager
			{
				ZoneServer* zoneServer = server->getZoneServer();
				DeathWatchManager* dwManager = zoneServer->getDeathWatchManager();

				if (dwManager != nullptr && dwManager->shouldInterceptHeavyWeapon(creature, targetObject, weapon)) {
					// Resolve the actual defender (handles jetpack targeting)
					CreatureObject* dwDefender = dwManager->resolveHeavyWeaponTarget(targetObject);

					if (dwDefender != nullptr) {
						// Use DW combat system for this attack
						int dwResult = dwManager->processHeavyWeaponHit(creature, dwDefender, weapon);

						// Consume weapon use regardless of hit/miss (rocket was fired)
						Core::getTaskManager()->scheduleTask([weapon] {
							Locker lock(weapon);
							weapon->decreaseUseCount();
						}, "FireHeavyWeaponDwDecrementTask", 100);

						// Play firing animation
						SharedObjectTemplate* animTemplate = TemplateManager::instance()->getTemplate(weapon->getServerObjectCRC());
						if (animTemplate != nullptr) {
							SharedWeaponObjectTemplate* animData = cast<SharedWeaponObjectTemplate*>(animTemplate);
							if (animData != nullptr) {
								String animName = "fire_heavy_" + animData->getAnimationType() + "_light";
								creature->doAnimation(animName);
							}
						}

						return (dwResult >= 0) ? SUCCESS : GENERALERROR;
					}
				}
			}
			// =====================================================================
			// END DEATH WATCH INTERCEPTION - Continue with normal combat
			// =====================================================================

			SharedObjectTemplate* templateData = TemplateManager::instance()->getTemplate(weapon->getServerObjectCRC());

			if (templateData == nullptr)
				return GENERALERROR;

			SharedWeaponObjectTemplate* weaponData = cast<SharedWeaponObjectTemplate*>(templateData);

			if (weaponData == nullptr)
				return GENERALERROR;

			UnicodeString args = "combatSpam=" + weaponData->getCombatSpam() + ";";

			int result = doCombatAction(creature, target, args, weapon);

			if (result == SUCCESS) {
				// We need to give some time for the combat animation to start playing before destroying the tano
				Core::getTaskManager()->scheduleTask([weapon] {
					Locker lock(weapon);
					weapon->decreaseUseCount();
				}, "FireHeavyWeaponTanoDecrementTask", 100);
			}

			return result;

		} catch (Exception& e) {

		}

		return GENERALERROR;
	}

	String getAnimation(TangibleObject* attacker, TangibleObject* defender, WeaponObject* weapon, uint8 hitLocation, int damage) const {
		SharedWeaponObjectTemplate* weaponData = cast<SharedWeaponObjectTemplate*>(weapon->getObjectTemplate());

		if (weaponData == nullptr) {
			warning("Null weaponData in FireHeavyWeapon::getAnimation");
			return "";
		}

		return "fire_heavy_" + weaponData->getAnimationType() + getIntensity(weapon->getMaxDamage() / 2.0f, damage);
	}

	float getCommandDuration(CreatureObject *object, const UnicodeString& arguments) const {
		StringTokenizer tokenizer(arguments.toString());
		uint64 weaponID = tokenizer.getLongToken();
		ManagedReference<WeaponObject*> weapon = server->getZoneServer()->getObject(weaponID).castTo<WeaponObject*>();

		if (weapon != nullptr)
			return CombatManager::instance()->calculateWeaponAttackSpeed(object, weapon, speedMultiplier);
		else
			return defaultTime * speedMultiplier;
	}

};

#endif //FIREHEAVYWEAPONCOMMAND_H_
