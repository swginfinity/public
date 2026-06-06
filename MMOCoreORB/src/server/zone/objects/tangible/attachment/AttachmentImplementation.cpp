/*
 * AttachmentImplementation.cpp
 *
 *  Created on: Mar 10, 2011
 *      Author: polonel
 */

#include "server/zone/objects/tangible/attachment/Attachment.h"
#include "server/zone/ZoneServer.h"
#include "server/zone/ZoneProcessServer.h"
#include "server/zone/packets/scene/AttributeListMessage.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/managers/loot/LootManager.h"

void AttachmentImplementation::initializeTransientMembers() {
	TangibleObjectImplementation::initializeTransientMembers();

	setLoggingName("AttachmentObject");

	// Infinity: Migrate old skillModMap (HashTable) data to skillModifiers (VectorMap)
	// Pre-JTL attachments stored mods in skillModMap; post-merge code reads skillModifiers
	if (skillModifiers.size() == 0 && skillModMap.size() > 0) {
		HashTableIterator<String, int> iterator = skillModMap.iterator();

		while (iterator.hasNext()) {
			String key;
			int value;

			iterator.getNextKeyAndValue(key, value);
			skillModifiers.put(key, value);
		}
	}
}

void AttachmentImplementation::updateCraftingValues(CraftingValues* values, bool firstUpdate, const String& lootTemplateName) { // Infinity: Added lootTemplateName to support Jedi SEA group
	int level = values->getMaxValue("creatureLevel");
	int roll = System::random(100);
	int modCount = 1;

	/* Infinity:  All AAs and CAs to have only one mod
	if (roll > 99) {
		modCount += 2;
	}

	if (roll < 5) {
		modCount += 1;
	}
	*/

	for (int i = 0; i < modCount; ++i) {
		// Mods can't be lower than -1 or greater than 25
		int max = (int)Math::max(-1.f, Math::min(25.f, (float)round(0.1f * level + 3)));
		int min = (int)Math::max(-1.f, Math::min(25.f, (float)round(0.075f * level - 1)));

		int mod = System::random(max - min) + min;

		if (mod == 0) {
			mod = 1;
		}

		String modName = server->getZoneServer()->getLootManager()->getRandomLootableMod(gameObjectType, lootTemplateName); // Infinity:  Added lootTemplateName

		if (modName.isEmpty()) {
			// LootManager::getRandomLootableMod has no branch for this
			// (gameObjectType, lootTemplateName) pair — most often a custom
			// loot template (e.g. "attachment_low_clothing") added in a Lua
			// loot config without the matching C++ wiring. Skip writing a
			// "" key to skillModifiers (which would produce a blank, no-op
			// attachment) and log so the gap is visible in core3.log.
			error() << "getRandomLootableMod returned empty for gameObjectType=" << gameObjectType
			        << " lootTemplateName=" << lootTemplateName
			        << " — attachment will have no mod (add the missing branch in LootManagerImplementation::getRandomLootableMod)";
			continue;
		}

		skillModifiers.put(modName, mod);
	}
}

void AttachmentImplementation::initializeMembers() {
	if (gameObjectType == SceneObjectType::CLOTHINGATTACHMENT) {
		setOptionsBitmask(32, true);
		attachmentType = CLOTHINGTYPE;

	} else if (gameObjectType == SceneObjectType::ARMORATTACHMENT) {
		setOptionsBitmask(32, true);
		attachmentType = ARMORTYPE;
	}
}

void AttachmentImplementation::fillAttributeList(AttributeListMessage* msg, CreatureObject* object) {
	TangibleObjectImplementation::fillAttributeList(msg, object);

	StringBuffer name;

	for (int i = 0; i < skillModifiers.size(); ++i) {
		const String& key = skillModifiers.elementAt(i).getKey();
		int value = skillModifiers.elementAt(i).getValue();

		name << "cat_skill_mod_bonus.@stat_n:" << key;

		msg->insertAttribute(name.toString(), value);

		name.deleteAll();
	}
}

void AttachmentImplementation::updateAttachmentValues(const String& modName, int value) {
	skillModifiers.put(modName, value);
}

// Infinity: Single source of truth for primary mod selection — used for naming and application
String AttachmentImplementation::getPrimarySkillMod(int& outValue) {
	// VectorMap iterates alphabetically by key. Pick the first mod with the highest value.
	// This matches the application order used by ModSortingHelper (value DESC, alpha tiebreak).
	String primaryName;
	int highest = -26;

	for (int i = 0; i < skillModifiers.size(); ++i) {
		const String& key = skillModifiers.elementAt(i).getKey();
		int value = skillModifiers.elementAt(i).getValue();

		if (value > highest) {
			highest = value;
			primaryName = key;
		}
	}

	outValue = highest;
	return primaryName;
}
