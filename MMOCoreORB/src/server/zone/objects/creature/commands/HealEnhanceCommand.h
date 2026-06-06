/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef HEALENHANCECOMMAND_H_
#define HEALENHANCECOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/objects/tangible/pharmaceutical/EnhancePack.h"
#include "server/zone/ZoneServer.h"
#include "server/zone/managers/player/PlayerManager.h"
#include "server/zone/objects/creature/events/InjuryTreatmentTask.h"
#include "server/zone/objects/creature/buffs/Buff.h"
#include "server/zone/objects/creature/BuffAttribute.h"
#include "server/zone/objects/creature/buffs/DelayedBuff.h"
#include "server/zone/managers/collision/CollisionManager.h"

class HealEnhanceCommand : public QueueCommand {
	float mindCost;
	float range;

public:
	HealEnhanceCommand(const String& name, ZoneProcessServer* server) : QueueCommand(name, server) {
		mindCost = 150;
		range = 15;  //Infinity;  Range increased from 7 to 15
	}

	void deactivateWoundTreatment(CreatureObject* creature) const {
		float modSkill = (float)creature->getSkillMod("healing_wound_speed");

		int delay = (int)round((modSkill * -(2.0f / 25.0f)) + 13.0f);   //Infinity: 5 seconds for master doc, 3 seconds with +25 Wound treatment speed or Havla (3 second minimum)

		if (creature->hasBuff(BuffCRC::FOOD_HEAL_RECOVERY)) {
			DelayedBuff* buff = cast<DelayedBuff*>(creature->getBuff(BuffCRC::FOOD_HEAL_RECOVERY));

			if (buff != nullptr) {
				float percent = buff->getSkillModifierValue("heal_recovery");

				delay = round(delay * (100.0f - percent) / 100.0f);
			}
		}

		// Force the delay to be at least 3 seconds.
		delay = (delay < 3) ? 3 : delay;

		StringIdChatParameter message("healing_response", "healing_response_59"); // You are now ready to heal more wounds or apply more enhancements.
		Reference<InjuryTreatmentTask*> task = new InjuryTreatmentTask(creature, message, "woundTreatment");
		creature->addPendingTask("woundTreatment", task, delay * 1000);
	}

	EnhancePack* findEnhancePack(CreatureObject* enhancer, uint8 attribute) const {
		SceneObject* inventory = enhancer->getSlottedObject("inventory");

		int medicineUse = enhancer->getSkillMod("healing_ability");

		if (inventory != nullptr) {
			for (int i = 0; i < inventory->getContainerObjectsSize(); ++i) {
				SceneObject* object = inventory->getContainerObject(i);

				if (!object->isPharmaceuticalObject())
					continue;

				PharmaceuticalObject* pharma = cast<PharmaceuticalObject*>(object);

				if (pharma->isEnhancePack()) {
					EnhancePack* enhancePack = cast<EnhancePack*>(pharma);

					if (enhancePack->getMedicineUseRequired() <= medicineUse && enhancePack->getAttribute() == attribute)
						return enhancePack;
				}
			}
		}

		return nullptr;
	}

	bool canPerformSkill(CreatureObject* enhancer, CreatureObject* patient, EnhancePack* enhancePack, int mindCostNew) const {
		if (patient->isDead())
			return false;

		if (!enhancer->canTreatWounds()) {
			enhancer->sendSystemMessage("@healing_response:enhancement_must_wait"); // You must wait before you can heal wounds or apply enhancements again.
			return false;
		}

		if (enhancePack == nullptr) {
			enhancer->sendSystemMessage("@healing_response:healing_response_60"); // No valid medicine found.
			return false;
		}

		int medicalRatingNotIncludingCityBonus = enhancer->getSkillMod("private_medical_rating") - enhancer->getSkillModOfType("private_medical_rating", SkillModManager::CITY);

		if (medicalRatingNotIncludingCityBonus <= 0) {
			enhancer->sendSystemMessage("@healing_response:must_be_near_droid"); // You must be in a hospital, at a campsite, or near a surgical droid to do that.
			return false;
		}
		else {//Infinity:  Reverse Mantis #8884/8885
			// are we in a cantina? we have a private medical rating so either thats form a droid or camp or hospital
			ManagedReference<SceneObject*> root = enhancer->getRootParent();
			if (root != nullptr && root->isClientObject()) {
				uint32 gameObjectType = root->getGameObjectType();
				switch (gameObjectType) {
						case SceneObjectType::RECREATIONBUILDING:
						case SceneObjectType::HOTELBUILDING:
						case SceneObjectType::THEATERBUILDING:
							enhancer->sendSystemMessage("@healing_response:must_be_in_hospital"); // You must be in a hospital or at a campsite to do that.
							return false;
				}
			}
		}

		if (enhancer->isInCombat()) {
			enhancer->sendSystemMessage("You cannot HealEnhance yourself while in Combat.");
			return false;
		}

		if (!patient->isPlayerCreature() && !(patient->isCreature() && patient->isPet())) {
			enhancer->sendSystemMessage("@healing_response:healing_response_77"); // Target must be a player or a creature pet in order to apply enhancements.
			return false;
		}

		if (patient->isPet()) {
			auto pcd = patient->getControlDevice().get().castTo<PetControlDevice*>();

			if (pcd != nullptr && pcd->getPetType() != PetManager::CREATUREPET) {
				enhancer->sendSystemMessage("@healing_response:healing_response_77"); // Target must be a player or a creature pet in order to apply enhancements.
				return false;
			}
		}

		if (patient->isInCombat()) {
			enhancer->sendSystemMessage("You cannot HealEnhance your target while they are still in Combat.");
			return false;
		}

		if (enhancer->getHAM(CreatureAttribute::MIND) < mindCostNew) {
			enhancer->sendSystemMessage("@healing_response:not_enough_mind"); // You do not have enough mind to do that.
			return false;
		}

		if (enhancer != patient && !CollisionManager::checkLineOfSight(enhancer, patient)) {
			enhancer->sendSystemMessage("@healing:no_line_of_sight"); // You cannot see your target.
			return false;
		}

		if (!playerEntryCheck(enhancer, patient)) {
			return false;
		}

		return true;
	}

	void parseModifier(const String& modifier, uint8& attribute, uint64& objectId) const {
		if (!modifier.isEmpty()) {
			StringTokenizer tokenizer(modifier);
			tokenizer.setDelimeter("|");

			String attributeName;

			tokenizer.getStringToken(attributeName);
			attribute = BuffAttribute::getAttribute(attributeName);

			if (tokenizer.hasMoreTokens()) {
				objectId = tokenizer.getLongToken();
			}
		} else {
			attribute = BuffAttribute::UNKNOWN;
			objectId = 0;
		}
	}

	void sendEnhanceMessage(CreatureObject* enhancer, CreatureObject* patient, uint8 attribute, uint32 buffApplied) const {
		if (enhancer == nullptr || patient == nullptr || !enhancer->isPlayerCreature())
			return;

		String enhancerName = enhancer->getFirstName();
		String patientName;
		String attributeName = BuffAttribute::getName(attribute, true);

		if (patient->isPlayerCreature())
			patientName = patient->getFirstName();
		else
			patientName = patient->getDisplayedName();

		if (BuffAttribute::isProtection(attribute))
			attributeName += " Resistance";

		StringBuffer msgPlayer, msgTarget, msgBuff;

		if (buffApplied == 0) {
			if (enhancer == patient) {
				msgPlayer << "You re-apply your ";
			} else {
				msgPlayer << "You re-apply " << patientName << "'s ";
				msgTarget << enhancerName << " re-applies your ";
			}
			msgBuff << attributeName << " enhancement.";
		} else {
			if (enhancer == patient) {
				msgPlayer << "You enhance your ";
			} else {
				msgPlayer << "You enhance " << patientName << "'s ";
				msgTarget << enhancerName << " enhances your ";
			}
			msgBuff << attributeName << " by " << buffApplied << ".";
		}

		msgPlayer << msgBuff.toString();
		enhancer->sendSystemMessage(msgPlayer.toString());

		if (enhancer != patient) {
			msgTarget << msgBuff.toString();
			patient->sendSystemMessage(msgTarget.toString());
		}
	}

	uint32 getEnhancePackStrength(EnhancePack* enhancePack, CreatureObject* enhancer, CreatureObject* patient) const {
		uint32 buffPower = 0;

		if (BuffAttribute::isProtection(enhancePack->getAttribute())) { // If it's a protection enhancement, wound treatment has no effect
			buffPower = enhancePack->getEffectiveness();
			buffPower = buffPower * patient->calculateBFRatio();
		} else
			buffPower = enhancePack->calculatePower(enhancer, patient);

		return buffPower;
	}

	uint32 getBuffStrength(Buff* existingbuff, int attribute) const {
		if (existingbuff != nullptr) {
			if (BuffAttribute::isProtection(attribute))
				return existingbuff->getSkillModifierValue(BuffAttribute::getProtectionString(attribute));
			else
				return existingbuff->getAttributeModifierValue(attribute);
		}
		return 0;
	}

	void awardXp(CreatureObject* creature, const String& type, int power) const {
		if (!creature->isPlayerCreature())
			return;

		CreatureObject* player = cast<CreatureObject*>(creature);

		int amount = (int)round((float)power * 2.5f);

		if (amount <= 0)
			return;

		PlayerManager* playerManager = server->getZoneServer()->getPlayerManager();
		playerManager->awardExperience(player, type, amount, true);
	}

	void doAnimations(CreatureObject* enhancer, CreatureObject* patient) const {
		patient->playEffect("clienteffect/healing_healenhance.cef", "");

		if (enhancer == patient)
			enhancer->doAnimation("heal_self");
		else
			enhancer->doAnimation("heal_other");
	}

	int doQueueCommand(CreatureObject* enhancer, const uint64& target, const UnicodeString& arguments) const {
		int result = doCommonMedicalCommandChecks(enhancer);

		if (result != SUCCESS)
			return result;

		ManagedReference<SceneObject*> object = server->getZoneServer()->getObject(target);

		if (object != nullptr) {
			if (!object->isCreatureObject()) {
				TangibleObject* tangibleObject = dynamic_cast<TangibleObject*>(object.get());

				if (tangibleObject != nullptr && tangibleObject->isAttackableBy(enhancer)) {
					object = enhancer;
				} else {
					enhancer->sendSystemMessage("@healing_response:healing_response_77"); // Target must be a player or a creature pet in order to apply enhancements.
					return GENERALERROR;
				}
			}
		} else {
			object = enhancer;
		}

		CreatureObject* patient = object->asCreatureObject();

		if (patient == nullptr)
			return GENERALERROR;

		uint8 attribute = BuffAttribute::UNKNOWN;
		uint64 objectId = 0;

		if (!checkDistance(enhancer, patient, range))
			return TOOFAR;

		parseModifier(arguments.toString(), attribute, objectId);

		Locker clocker(patient, enhancer);

		ManagedReference<EnhancePack*> enhancePack = nullptr;

		if (objectId != 0) {  //Infinity:  Enhance used from item or from toolbar.  Check containers.
			SceneObject* inventory = enhancer->getSlottedObject("inventory");
			SceneObject* backpack = enhancer->getSlottedObject("back");

			if (inventory != nullptr) {	//First, check inventory
				enhancePack = inventory->getContainerObject(objectId).castTo<EnhancePack*>();
			}

			if (enhancePack == nullptr) { //Second, check backpack
				if (backpack != nullptr && backpack->isContainerObject()) {
					enhancePack = backpack->getContainerObject(objectId).castTo<EnhancePack*>();
				}
			}

			if (enhancePack == nullptr) { //Third, check for containers in inventory or backpack
				SceneObject* medicalPack = server->getZoneServer()->getObject(objectId);
				if (medicalPack != nullptr) {
					ManagedReference<SceneObject*> packParent = medicalPack->getParent().get();
					if (packParent != nullptr) {
						ManagedReference<SceneObject*> packParentParent = packParent->getParent().get();
						if (packParentParent != nullptr && (packParentParent == inventory || packParentParent == backpack)) {
							enhancePack = enhancePack = cast<EnhancePack*>(medicalPack);
						}
					}
				}
			}

			if (enhancePack == nullptr) {
				enhancer->sendSystemMessage("@healing_response:healing_response_76"); // That item does not provide attribute enhancement.
				return GENERALERROR;
			}

			int medicineUse = enhancer->getSkillMod("healing_ability");

			if (enhancePack->getMedicineUseRequired() > medicineUse) {
				enhancer->sendSystemMessage("@error_message:insufficient_skill"); // You lack the skill to use this item.
				return GENERALERROR;
			}

			if (enhancePack->getAttribute() != attribute) {
				attribute = enhancePack->getAttribute();
			}
		} else {
			if (attribute == BuffAttribute::UNKNOWN) {
				SortedVector<Buff*> currentBuffs; // values are sorted by duration
				VectorMap<Buff*, int> attributeMap;

				for (int i = 0; i < CreatureAttribute::MIND; i++) {
					uint32 buffCRC = BuffCRC::getMedicalBuff(i);
					if (patient->hasBuff(buffCRC)) {
						Buff* buff = patient->getBuff(buffCRC);
						currentBuffs.put(buff);
						attributeMap.put(buff, i);

						continue;
					}

					attribute = i;
					enhancePack = findEnhancePack(enhancer, i);
					if (enhancePack != nullptr)
						break;
				}

				if (enhancePack == nullptr) {
					// We couldn't find any enhance packs for non-applied buffs.
					// Loop through the applied buffs and see if one matches our criteria
					// This should be sorted by time applied - This ensures we don't overwrite the same buff repeatedly
					for (int i = 0; i < currentBuffs.size(); i++) {
						Buff* buff = currentBuffs.get(i);

						attribute = attributeMap.get(buff);
						enhancePack = findEnhancePack(enhancer, attribute);
						if (enhancePack != nullptr) {
							uint32 currentBuff = getBuffStrength(buff, attribute);
							uint32 newBuff = getEnhancePackStrength(enhancePack, enhancer, patient);

							if (newBuff < currentBuff) {
								enhancePack = nullptr;
								attribute = BuffAttribute::UNKNOWN;
							} else {
								break;
							}
						}
					}

					if (attribute == BuffAttribute::UNKNOWN) {
						StringIdChatParameter stringId("healing", "no_attrib_to_buff"); // %TT has no attribute that you can enhance.
						stringId.setTT(patient->getDisplayedName());
						enhancer->sendSystemMessage(stringId);
						return GENERALERROR;
					}
				}
			} else {
				enhancePack = findEnhancePack(enhancer, attribute);
			}
		}

		if (patient->isDead() || patient->isRidingMount())
			patient = enhancer;

		int mindCostNew = enhancer->calculateCostAdjustment(CreatureAttribute::FOCUS, mindCost);

		if (!canPerformSkill(enhancer, patient, enhancePack, mindCostNew))
			return GENERALERROR;

		uint32 currentBuff = 0;
		uint32 buffcrc = BuffCRC::getMedicalBuff(attribute);

		if (patient->hasBuff(buffcrc)) {
			Buff* existingbuff = patient->getBuff(buffcrc);

			currentBuff = getBuffStrength(existingbuff, attribute);
		}

		// Applies battle fatigue
		uint32 buffPower = getEnhancePackStrength(enhancePack, enhancer, patient);

		// Infinity: Allow rebuff if existing buff is within 73 points of new power.
		// This lets doctors refresh buffs that are close in strength without getting
		// the "too powerful" error (QoL improvement for doctor buffs).
		if (buffPower < currentBuff) {
			if (currentBuff - buffPower <= 73) {
				Buff* existingBuff = patient->getBuff(buffcrc);
				if (existingBuff != nullptr) {
					Locker blocker(existingBuff);
					patient->removeBuff(existingBuff);
				}
			} else {
				if (patient == enhancer) {
					enhancer->sendSystemMessage("Your current enhancements are of greater power and cannot be re-applied.");
				} else {
					enhancer->sendSystemMessage("Your target's current enhancements are of greater power and cannot be re-applied.");
				}
				return GENERALERROR;
			}
		}

		auto zoneServer = server->getZoneServer();

		if (zoneServer == nullptr)
			return GENERALERROR;

		auto playerMan = zoneServer->getPlayerManager();

		if (playerMan == nullptr)
			return GENERALERROR;

		uint32 amountEnhanced = playerMan->healEnhance(enhancer, patient, attribute, buffPower, enhancePack->getDuration(), enhancePack->getAbsorption());

		if (enhancer->isPlayerCreature() && patient->isPlayerCreature()) {
			playerMan->sendBattleFatigueMessage(enhancer, patient);
		}

		sendEnhanceMessage(enhancer, patient, attribute, amountEnhanced);

		enhancer->inflictDamage(enhancer, CreatureAttribute::MIND, mindCostNew, false);

		deactivateWoundTreatment(enhancer);

		if (enhancePack != nullptr) {
			Locker locker(enhancePack);
			enhancePack->decreaseUseCount();
		}

		if (patient->getObjectID() != enhancer->getObjectID())
			awardXp(enhancer, "medical", amountEnhanced); // No experience for healing yourself.

		doAnimations(enhancer, patient);

		enhancer->notifyObservers(ObserverEventType::MEDPACKUSED);

		return SUCCESS;
	}
};

#endif // HEALENHANCECOMMAND_H_
