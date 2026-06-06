/*
 * WearableObjectImplementation.cpp
 *
 *  Created on: 02/08/2009
 *      Author: victor
 */

#include "server/zone/objects/tangible/wearables/WearableObject.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/objects/manufactureschematic/craftingvalues/CraftingValues.h"
#include "server/zone/objects/manufactureschematic/ManufactureSchematic.h"
#include "server/zone/objects/draftschematic/DraftSchematic.h"
#include "server/zone/objects/tangible/attachment/Attachment.h"
#include "server/zone/managers/skill/SkillModManager.h"
#include "server/zone/objects/tangible/wearables/ModSortingHelper.h"
#include "server/zone/objects/transaction/TransactionLog.h"

void WearableObjectImplementation::initializeTransientMembers() {
	TangibleObjectImplementation::initializeTransientMembers();

	// Wearable has too many attachments on it for the allowed socket count
	while (usedSocketCount > socketCount) {
		wearableSkillMods.removeElementAt(wearableSkillMods.size() - 1);

		usedSocketCount--;
	}
}

void WearableObjectImplementation::fillAttributeList(AttributeListMessage* alm, CreatureObject* object) {
	TangibleObjectImplementation::fillAttributeList(alm, object);

	for(int i = 0; i < wearableSkillMods.size(); ++i) {
		String key = wearableSkillMods.elementAt(i).getKey();
		String statname = "cat_skill_mod_bonus.@stat_n:" + key;
		int value = wearableSkillMods.get(key);

		if (value > 0)
			alm->insertAttribute(statname, value);
	}

	//Anti Decay Kit
	if (hasAntiDecayKit() && !isArmorObject()){
		alm->insertAttribute("@veteran_new:antidecay_examine_title", "@veteran_new:antidecay_examine_text");
	}

}

void WearableObjectImplementation::updateCraftingValues(CraftingValues* values, bool initialUpdate) {
	/*
	 * Values available:	Range:
	 * sockets				0-0(novice artisan) (Don't use)
	 * hitpoints			1000-1000 (Don't Use)
	 */
	if (initialUpdate) {
		if(values->hasExperimentalAttribute("sockets") && values->getCurrentValue("sockets") >= 0)
			generateSockets(values);
	}
}

void WearableObjectImplementation::generateSockets(CraftingValues* craftingValues) {
	if (socketsGenerated) {
		return;
	}

	int skill = 0;
	int luck = 0;

	if (craftingValues != nullptr) {
		ManagedReference<ManufactureSchematic*> manuSchematic = craftingValues->getManufactureSchematic();

		if (manuSchematic != nullptr) {
			ManagedReference<DraftSchematic*> draftSchematic = manuSchematic->getDraftSchematic();
			ManagedReference<CreatureObject*> player = manuSchematic->getCrafter().get();

			if (player != nullptr && draftSchematic != nullptr) {
				String assemblySkill = draftSchematic->getAssemblySkill();

				skill = player->getSkillMod(assemblySkill) + player->getSkillMod("force_assembly");  //Infinity:  Include force assembly

				if (MIN_SOCKET_MOD > skill)
					return;

                skill *= 3.4483;  //Infinity: 0 to 500 max  (345 for master crafter, no tapes, no force assembly, 500 for all mods)

                //Infinity: Let's cap the chance of 4-sockets at 90% before luck
				if (skill > 450)
					skill = 450;

				luck = System::random(player->getSkillMod("luck") + player->getSkillMod("force_luck"));
			}
		}
	}

	int random = (System::random(500)) - 100; //Infinity: -100 to 400 90% chance of 4 sockets w/ master, +25 assembly tapes, & +20 force assembly, 69% chance of 4 sockets with master only, 13.8% at novice   
	float roll = skill + random + luck;
    float generatedCount = roll / 100.0f;

	if (generatedCount > MAXSOCKETS)
		generatedCount = MAXSOCKETS;
	if (generatedCount < 0)
		generatedCount = 0;

	usedSocketCount = 0;
	socketCount = (int)generatedCount;

	socketsGenerated = true;

	return;
}

void WearableObjectImplementation::applyAttachment(CreatureObject* player, Attachment* attachment) {
	if (attachment == nullptr || !isASubChildOf(player))
		return;

	if (getRemainingSockets() > 0 && wearableSkillMods.size() < 6) {
		if (isEquipped()) {
			removeSkillModsFrom(player);
		}

		Locker clocker(attachment, player);

		VectorMap<String, int>* mods = attachment->getSkillMods();

		SortedVector< ModSortingHelper > sortedMods;
		for (int i = 0; i < mods->size(); i++) {
			String statName = mods->elementAt(i).getKey();
			int newValue = mods->elementAt(i).getValue();
			sortedMods.put(ModSortingHelper(statName, newValue));
		}

		// Select the next mod in the SEA, sorted high-to-low. If that skill mod is already on the
		// wearable, with higher or equal value, don't apply and continue. Break once one mod
		// is applied.
		bool modApplied = false;

		for (int i = 0; i < sortedMods.size(); i++ ) {
			String modName = sortedMods.elementAt(i).getKey();
			int modValue = sortedMods.elementAt(i).getValue();

			int existingValue = -26;
			if (wearableSkillMods.contains(modName))
				existingValue = wearableSkillMods.get(modName);

			if (modValue > existingValue) {
				wearableSkillMods.put(modName, modValue);
				modApplied = true;
				break;
			}
		}

		if (modApplied) {
			usedSocketCount++;
			addMagicBit(true);

			TransactionLog trx(player, asSceneObject(), attachment, TrxCode::APPLYATTACHMENT);

			if (trx.isVerbose()) {
				// Force a synchronous export because the object will be deleted before we can export it!
				trx.addRelatedObject(attachment, true);
				trx.setExportRelatedObjects(true);
				trx.exportRelated();
			}

			trx.addState("subjectSkillModMap", sortedMods);
			trx.addState("dstSkillModMap", wearableSkillMods);

			attachment->destroyObjectFromWorld(true);
			attachment->destroyObjectFromDatabase(true);
		} else {
			player->sendSystemMessage("The attachment had no applicable skill mods for this item.");  // Infinity: Don't consume socket or destroy SEA if no mod applied
		}

		if (isEquipped()) {
			applySkillModsTo(player);
		}
	}
	else {
		player->sendSystemMessage("The wearable item cannot hold additional SEAs.");   //Custom error message
	}
}

void WearableObjectImplementation::applySkillModsTo(CreatureObject* creature) const {
	if (creature == nullptr) {
		return;
	}

	for (int i = 0; i < wearableSkillMods.size(); ++i) {
		String name = wearableSkillMods.elementAt(i).getKey();
		int value = wearableSkillMods.get(name);

		if (!SkillModManager::instance()->isWearableModDisabled(name))
		{
			creature->addSkillMod(SkillModManager::WEARABLE, name, value, true);
			creature->updateSpeedAndAccelerationMods();
		}
	}

	SkillModManager::instance()->verifyWearableSkillMods(creature);
}

void WearableObjectImplementation::removeSkillModsFrom(CreatureObject* creature) {
	if (creature == nullptr) {
		return;
	}

	for (int i = 0; i < wearableSkillMods.size(); ++i) {
		String name = wearableSkillMods.elementAt(i).getKey();
		int value = wearableSkillMods.get(name);

		if (!SkillModManager::instance()->isWearableModDisabled(name))
		{
			creature->removeSkillMod(SkillModManager::WEARABLE, name, value, true);
			creature->updateSpeedAndAccelerationMods();
		}
	}

	SkillModManager::instance()->verifyWearableSkillMods(creature);
}

bool WearableObjectImplementation::isEquipped() {
	ManagedReference<SceneObject*> parent = getParent().get();
	if (parent != nullptr && parent->isPlayerCreature())
		return true;

	return false;
}

String WearableObjectImplementation::repairAttempt(int repairChance) {
	String message = "@error_message:";

	if(repairChance < 25) {
		message += "sys_repair_failed";
		setMaxCondition(1, true);
		setConditionDamage(0, true);
	} else if(repairChance < 50) {
		message += "sys_repair_imperfect";
		setMaxCondition(getMaxCondition() * .65f, true);
		setConditionDamage(0, true);
	} else if(repairChance < 75) {
		setMaxCondition(getMaxCondition() * .80f, true);
		setConditionDamage(0, true);
		message += "sys_repair_slight";
	} else {
		setMaxCondition(getMaxCondition() * .95f, true);
		setConditionDamage(0, true);
		message += "sys_repair_perfect";
	}

	return message;
}

