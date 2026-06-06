/*
 * LightsaberCrystalComponentImplementation.cpp
 *
 *      Author: Katherine
 */

#include "engine/engine.h"
#include "server/zone/objects/player/sui/messagebox/SuiMessageBox.h"
#include "server/zone/objects/tangible/component/lightsaber/LightsaberCrystalComponent.h"
#include "server/zone/packets/object/ObjectMenuResponse.h"
#include "server/zone/objects/tangible/wearables/WearableContainerObject.h"
#include "server/zone/packets/scene/AttributeListMessage.h"
#include "server/zone/objects/player/PlayerObject.h"
#include "server/zone/objects/player/sui/callbacks/LightsaberCrystalTuneSuiCallback.h"
#include "server/zone/objects/tangible/weapon/WeaponObject.h"
#include "server/zone/managers/stringid/StringIdManager.h"
#include "server/zone/managers/loot/CrystalData.h"
#include "server/zone/managers/loot/LootManager.h"
#include "server/zone/ZoneServer.h"

void LightsaberCrystalComponentImplementation::initializeTransientMembers() {
	ComponentImplementation::initializeTransientMembers();

	setLoggingName("LightsaberCrystalComponent");
}

void LightsaberCrystalComponentImplementation::notifyLoadFromDatabase() {
	// Randomize item level and stats for existing crystals based on original quality value
	// TODO: Remove this on a server wipe when old variables are removed
	if (color == 31 && (minimumDamage != maximumDamage || itemLevel == 0)) {
		if (quality == POOR)
			itemLevel = 1 + System::random(38); // 1-39
		else if (quality == FAIR)
			itemLevel = 40 + System::random(29); // 40-69
		else if (quality == GOOD)
			itemLevel = 70 + System::random(29); // 70-99
		else if (quality == QUALITY)
			itemLevel = 100 + System::random(39); // 100-139
		else if (quality == SELECT)
			itemLevel = 140 + System::random(79); // 140-219
		else if (quality == PREMIUM)
			itemLevel = 220 + System::random(109); // 220-329
		else
			itemLevel = 330 + System::random(50); // 330-380

		attackSpeed = 0.0;
		minimumDamage = 0;
		maximumDamage = 0;
		sacHealth = 0;
		sacAction = 0;
		sacMind = 0;
		woundChance = 0;
		forceCost = 0;
		floatForceCost = 0.0;
		crystalExcMod = 0.0;//Infinity
		

		generateCrystalStats();
	}

	TangibleObjectImplementation::notifyLoadFromDatabase();
}

void LightsaberCrystalComponentImplementation::generateCrystalStats() {
	ManagedReference<LootManager*> lootManager = getZoneServer()->getLootManager();

	if (lootManager == nullptr)
		return;

	const CrystalData* crystalData = lootManager->getCrystalData(getObjectTemplate()->getTemplateFileName());

	if (crystalData == nullptr) {
		error("Unable to find crystal stats for " + getObjectTemplate()->getTemplateFileName());
		return;
	}

	int minStat = crystalData->getMinHitpoints();
	int maxStat = crystalData->getMaxHitpoints();
	setMaxCondition(hardCapStat(minStat, maxStat, getRandomizedStat(minStat, maxStat, itemLevel)));

	if (color == 31) {
		minStat = crystalData->getMinDamage();
		maxStat = crystalData->getMaxDamage();
		damage = hardCapStat(minStat, maxStat, getRandomizedStat(minStat, maxStat, itemLevel));

		minStat = crystalData->getMinHealthSac();
		maxStat = crystalData->getMaxHealthSac();
		sacHealth = hardCapStat(minStat, maxStat, getRandomizedStat(minStat, maxStat, itemLevel));

		minStat = crystalData->getMinActionSac();
		maxStat = crystalData->getMaxActionSac();
		sacAction = hardCapStat(minStat, maxStat, getRandomizedStat(minStat, maxStat, itemLevel));

		minStat = crystalData->getMinMindSac();
		maxStat = crystalData->getMaxMindSac();
		sacMind = hardCapStat(minStat, maxStat, getRandomizedStat(minStat, maxStat, itemLevel));

		minStat = crystalData->getMinWoundChance();
		maxStat = crystalData->getMaxWoundChance();
		woundChance = hardCapStat(minStat, maxStat, getRandomizedStat(minStat, maxStat, itemLevel));

		float minFloatStat = crystalData->getMinForceCost();
		float maxFloatStat = crystalData->getMaxForceCost();
		floatForceCost = hardCapStat(minFloatStat, maxFloatStat, getRandomizedStat(minFloatStat, maxFloatStat, itemLevel));

		minFloatStat = crystalData->getMinAttackSpeed();
		maxFloatStat = crystalData->getMaxAttackSpeed();
		attackSpeed = Math::getPrecision(hardCapStat(minFloatStat, maxFloatStat, getRandomizedStat(minFloatStat, maxFloatStat, itemLevel)), 2);
	}

	quality = getCrystalQuality();
}

void LightsaberCrystalComponentImplementation::validateCrystalStats() {
	ManagedReference<LootManager*> lootManager = getZoneServer()->getLootManager();

	if (lootManager == nullptr)
		return;

	const CrystalData* crystalData = lootManager->getCrystalData(getObjectTemplate()->getTemplateFileName());

	if (crystalData == nullptr) {
		error("Unable to find crystal stats for " + getObjectTemplate()->getTemplateFileName());
		return;
	}

	int minStat = crystalData->getMinHitpoints();
	int maxStat = crystalData->getMaxHitpoints();
	bool inverted = minStat > maxStat;
	int maxCondition = getMaxCondition();

	if ((!inverted && (maxCondition > maxStat || maxCondition < minStat)) || (inverted && (maxCondition < maxStat || maxCondition > minStat))) {
		setMaxCondition(hardCapStat(minStat, maxStat, getRandomizedStat(minStat, maxStat, itemLevel)));
	}

	if (color == 31) {
		minStat = crystalData->getMinDamage();
		maxStat = crystalData->getMaxDamage();
		inverted = minStat > maxStat;

		if ((!inverted && (damage > maxStat || damage < minStat)) || (inverted && (damage < maxStat || damage > minStat))) {
			damage = hardCapStat(minStat, maxStat, getRandomizedStat(minStat, maxStat, itemLevel));
		}

		minStat = crystalData->getMinHealthSac();
		maxStat = crystalData->getMaxHealthSac();
		inverted = minStat > maxStat;

		if ((!inverted && (sacHealth > maxStat || sacHealth < minStat)) || (inverted && (sacHealth < maxStat || sacHealth > minStat))) {
			sacHealth = hardCapStat(minStat, maxStat, getRandomizedStat(minStat, maxStat, itemLevel));
		}

		minStat = crystalData->getMinActionSac();
		maxStat = crystalData->getMaxActionSac();
		inverted = minStat > maxStat;

		if ((!inverted && (sacAction > maxStat || sacAction < minStat)) || (inverted && (sacAction < maxStat || sacAction > minStat))) {
			sacAction = hardCapStat(minStat, maxStat, getRandomizedStat(minStat, maxStat, itemLevel));
		}

		minStat = crystalData->getMinMindSac();
		maxStat = crystalData->getMaxMindSac();
		inverted = minStat > maxStat;

		if ((!inverted && (sacMind > maxStat || sacMind < minStat)) || (inverted && (sacMind < maxStat || sacMind > minStat))) {
			sacMind = hardCapStat(minStat, maxStat, getRandomizedStat(minStat, maxStat, itemLevel));
		}

		minStat = crystalData->getMinWoundChance();
		maxStat = crystalData->getMaxWoundChance();
		inverted = minStat > maxStat;

		if ((!inverted && (woundChance > maxStat || woundChance < minStat)) || (inverted && (woundChance < maxStat || woundChance > minStat))) {
			woundChance = hardCapStat(minStat, maxStat, getRandomizedStat(minStat, maxStat, itemLevel));
		}

		float minFloatStat = crystalData->getMinForceCost();
		float maxFloatStat = crystalData->getMaxForceCost();
		inverted = minFloatStat > maxFloatStat;

		if ((!inverted && (floatForceCost > maxFloatStat || floatForceCost < minFloatStat)) || (inverted && (floatForceCost < maxFloatStat || floatForceCost > minFloatStat))) {
			floatForceCost = hardCapStat(minFloatStat, maxFloatStat, getRandomizedStat(minFloatStat, maxFloatStat, itemLevel));
		}

		minFloatStat = crystalData->getMinAttackSpeed();
		maxFloatStat = crystalData->getMaxAttackSpeed();
		inverted = minFloatStat > maxFloatStat;

		if ((!inverted && (attackSpeed > maxFloatStat || attackSpeed < minFloatStat)) || (inverted && (attackSpeed < maxFloatStat || attackSpeed > minFloatStat))) {
			attackSpeed = Math::getPrecision(hardCapStat(minFloatStat, maxFloatStat, getRandomizedStat(minFloatStat, maxFloatStat, itemLevel)), 2);
		}

		// Do Exceptional Chances modifiers here!
		if (crystalExcMod == 1.5) {
			damage += 10;
			woundChance += 1;
			floatForceCost -= 0.50;
		}else if (crystalExcMod == 2.0){
			damage += 15;
			woundChance += 2;
			floatForceCost -= 0.75;
		}else if (crystalExcMod == 3.0){
			damage += 20;
			woundChance += 3;
			floatForceCost -= 1.25;
		}
	}
}

int LightsaberCrystalComponentImplementation::getCrystalQuality() {
	if (itemLevel < 40)
		return POOR;
	else if (itemLevel < 70)
		return FAIR;
	else if (itemLevel < 100)
		return GOOD;
	else if (itemLevel < 140)
		return QUALITY;
	else if (itemLevel < 220)
		return SELECT;
	else if (itemLevel < 330)
		return PREMIUM;
	else
		return FLAWLESS;
}

int LightsaberCrystalComponentImplementation::getRandomizedStat(int min, int max, int itemLevel) {

	bool invertedValues = false;
	int invertedMin = min;
	int invertedMax = max;

	if (min > max) {
		int temp = min;
		min = max;
		max = temp;

		invertedValues = true;
	}

	int minGap = (min >= 500) ? min * 0.015f : min * 0.05f;
	int maxGap = (max >= 500) ? max * 0.015f : max * 0.05f;

	float avgLevel = (itemLevel + 40) / 220.0f;
	float midLevel = min + ((max - min) * avgLevel);

	if (midLevel < min) {
		max += (midLevel - min);
		midLevel = min;
	}

	if (midLevel > max) {
		min += (midLevel - max);
		midLevel = max;
	}

	midLevel = round(midLevel);

	int roll = (midLevel - min > minGap) ? System::random(midLevel - min) : System::random(minGap);
	int randMin = min + roll;

	roll = (max - midLevel > maxGap) ? System::random(max - midLevel) : System::random(maxGap);
	int randMax = midLevel + roll;

	roll = (randMax - randMin > maxGap) ? System::random(randMax - randMin) : System::random(maxGap);
	int result = randMin + roll;

	if (invertedValues)
		result = invertedMin + (invertedMax - result);

	return result;
}

float LightsaberCrystalComponentImplementation::getRandomizedStat(float min, float max, int itemLevel) {  //Infinity:  This is used for damage and force cost only
	bool invertedValues = false;
	float invertedMin = min;
	float invertedMax = max;

	if (min > max) {
		float temp = min;
		min = max;
		max = temp;

		invertedValues = true;
	}

	float minGap = (min >= 500) ? min * 0.015f : min * 0.05f;
	float maxGap = (max >= 500) ? max * 0.015f : max * 0.05f;

	float avgLevel = (itemLevel + 20) / 220.f;
	float midLevel = min + ((max - min) * avgLevel);

	if (midLevel < min) {
		max += (midLevel - min);
		midLevel = min;
	}

	if (midLevel > max) {
		min += (midLevel - max);
		midLevel = max;
	}

	float roll = (midLevel - min > minGap) ? System::getMTRand()->rand(midLevel - min) : System::getMTRand()->rand(minGap);
	float randMin = min + roll;

	roll = (max - midLevel > maxGap) ? System::getMTRand()->rand(max - midLevel) : System::getMTRand()->rand(maxGap);
	float randMax = midLevel + roll;

	roll = (randMax - randMin > maxGap) ? System::random(randMax - randMin) : System::random(maxGap);
	float result = randMin + roll;

	if (invertedValues)
		result = invertedMin + (invertedMax - result);

	return result;
}

void LightsaberCrystalComponentImplementation::fillAttributeList(AttributeListMessage* alm, CreatureObject* object) {
	TangibleObjectImplementation::fillAttributeList(alm, object);

	if (object == nullptr) {
		return;
	}

	PlayerObject* player = object->getPlayerObject();
	if (object->hasSkill("force_title_jedi_rank_01") || player->isPrivileged()) {
		if (ownerID == 0) {
			StringBuffer str;
			str << "\\#pcontrast2 UNTUNED";
			alm->insertAttribute("crystal_owner", str);
		} else {
			alm->insertAttribute("crystal_owner", ownerName);
		}

		if (getColor() != 31) {
			StringBuffer str3;
			str3 << "@jedi_spam:saber_color_" << getColor();
			alm->insertAttribute("color", str3);
		} else {
			if (ownerID != 0 || player->isPrivileged()) {
				alm->insertAttribute("mindamage", damage);
				alm->insertAttribute("maxdamage", damage);
				alm->insertAttribute("wpn_attack_speed", attackSpeed);
				alm->insertAttribute("wpn_wound_chance", woundChance);
				alm->insertAttribute("wpn_attack_cost_health", sacHealth);
				alm->insertAttribute("wpn_attack_cost_action", sacAction);
				alm->insertAttribute("wpn_attack_cost_mind", sacMind);
				//alm->insertAttribute("forcecost", (int)getForceCost());
				alm->insertAttribute("forcecost", round(getForceCost() * 100.f) / 100.0f);   //Infinity: Show crystal force cost to two decimals
	
				// For debugging
				if (player->isPrivileged()) {
					StringBuffer str;
					str << "@jedi_spam:crystal_quality_" << getQuality();
					alm->insertAttribute("challenge_level", itemLevel);
					alm->insertAttribute("crystal_quality", str);
				}
			} else {
				StringBuffer str;
				str << "@jedi_spam:crystal_quality_" << getQuality();
				alm->insertAttribute("crystal_quality", str);
			}
		}
	}
}

void LightsaberCrystalComponentImplementation::fillObjectMenuResponse(ObjectMenuResponse* menuResponse, CreatureObject* player) {
	if (ownerID == 0 && player->hasSkill("force_title_jedi_rank_01") && hasPlayerAsParent(player)) {
		String text = "@jedi_spam:tune_crystal";
		menuResponse->addRadialMenuItem(128, 3, text);
	}

	PlayerObject* ghost = player->getPlayerObject();
	if (ghost != nullptr && ghost->isPrivileged()) {
		menuResponse->addRadialMenuItem(129, 3, "Staff Commands");

		if (getColor() == 31)
			menuResponse->addRadialMenuItemToRadialID(129, 130, 3, "Recalculate Stats");

		if (ownerID != 0)
			menuResponse->addRadialMenuItemToRadialID(129, 131, 3, "Untune Crystal");
	}

	ComponentImplementation::fillObjectMenuResponse(menuResponse, player);
}

int LightsaberCrystalComponentImplementation::handleObjectMenuSelect(CreatureObject* player, byte selectedID) {
	if (selectedID == 128 && player->hasSkill("force_title_jedi_rank_01") && hasPlayerAsParent(player) && ownerID == 0) {
		ManagedReference<SuiMessageBox*> suiMessageBox = new SuiMessageBox(player, SuiWindowType::TUNE_CRYSTAL);

		suiMessageBox->setPromptTitle("@jedi_spam:confirm_tune_title");
		suiMessageBox->setPromptText("@jedi_spam:confirm_tune_prompt");
		suiMessageBox->setCancelButton(true, "Cancel");
		suiMessageBox->setUsingObject(_this.getReferenceUnsafeStaticCast());
		suiMessageBox->setCallback(new LightsaberCrystalTuneSuiCallback(player->getZoneServer()));

		player->getPlayerObject()->addSuiBox(suiMessageBox);
		player->sendMessage(suiMessageBox->generateMessage());
	}

	PlayerObject* ghost = player->getPlayerObject();
	if (ghost != nullptr && ghost->isPrivileged()){
		if (selectedID == 130 && getColor() == 31) {
			generateCrystalStats();
		} else if (selectedID == 131 && ownerID != 0) {
			ownerID = 0;

			String tuneName = StringIdManager::instance()->getStringId(objectName.getFullPath().hashCode()).toString();
			if (getCustomObjectName().toString().contains("(Exceptional)"))
				tuneName = tuneName + " (Exceptional)\\#.";
			else if (getCustomObjectName().toString().contains("(Legendary)"))
				tuneName = tuneName + " (Legendary)\\#.";
			else
				tuneName = tuneName + "\\#.";
		}
	}

	return 0;
}

bool LightsaberCrystalComponentImplementation::hasPlayerAsParent(CreatureObject* player) {
	ManagedReference<SceneObject*> wearableParent = getParentRecursively(SceneObjectType::WEARABLECONTAINER);
	SceneObject* inventory = player->getSlottedObject("inventory");
	SceneObject* bank = player->getSlottedObject("bank");

	// Check if crystal is inside a wearable container in bank or inventory
	if (wearableParent != nullptr) {
		ManagedReference<WearableContainerObject*> wearable = cast<WearableContainerObject*>(wearableParent.get());

		if (wearable != nullptr) {
			SceneObject* parentOfWearableParent = wearable->getParent().get();

			if (parentOfWearableParent == inventory || parentOfWearableParent == bank)
				return true;
		}
	} else {
		// Check if crystal is in inventory or bank
		SceneObject* thisParent = getParent().get();

		if (thisParent == inventory || thisParent == bank)
			return true;
	}
	return false;
}

void LightsaberCrystalComponentImplementation::tuneCrystal(CreatureObject* player) {
	if(!player->hasSkill("force_title_jedi_rank_01") || !hasPlayerAsParent(player)) {
		return;
	}

	if (getColor() == 31) {
		ManagedReference<PlayerObject*> ghost = player->getPlayerObject();

		if (ghost == nullptr)
			return;

		int tuningCost = 100 + (quality * 75);

		if (ghost->getForcePower() <= tuningCost) {
			player->sendSystemMessage("@jedi_spam:no_force_power");
			return;
		}

		ghost->setForcePower(ghost->getForcePower() - tuningCost);
	}

	if (ownerID == 0) {
		validateCrystalStats();

		auto zoneServer = player->getZoneServer();
		if (zoneServer != nullptr) {
			if (zoneServer->getGalaxyName() == "SWG Infinity TC") { //Let's randomize crystal tuning on TC only so people can't cheat the system
				generateCrystalStats();
				validateCrystalStats();
			}
		}

		ownerID = player->getObjectID();
		ownerName = player->getDisplayedName();

		// Color code is lime green.
		String tuneName = StringIdManager::instance()->getStringId(objectName.getFullPath().hashCode()).toString();
		if (getCustomObjectName().toString().contains("(Exceptional)"))
			tuneName = "\\#00FF00" + tuneName + " (Exceptional) (tuned)\\#.";
		else if (getCustomObjectName().toString().contains("(Legendary)"))
			tuneName = "\\#00FF00" + tuneName + " (Legendary) (tuned)\\#.";
		else
			tuneName = "\\#00FF00" + tuneName + " (tuned)\\#.";

		setCustomObjectName(tuneName, true);
		player->notifyObservers(ObserverEventType::TUNEDCRYSTAL, _this.getReferenceUnsafeStaticCast(), 0);
		player->sendSystemMessage("@jedi_spam:crystal_tune_success");
	}
}

void LightsaberCrystalComponentImplementation::updateCrystal(int value){
	byte type = 0x02;
	setCustomizationVariable(type, value, true);
}

void LightsaberCrystalComponentImplementation::updateCraftingValues(CraftingValues* values, bool firstUpdate) {
	int colorMax = values->getMaxValue("color");
	int color = values->getCurrentValue("color");

	if (colorMax != 31) {
		int finalColor = Math::min(color, 63);  //Infinity:  Custom crystal colors, change from 11 to 63
		setColor(finalColor);
		updateCrystal(finalColor);
	} else {
		setColor(31);
		updateCrystal(31);
	}

	generateCrystalStats();

	ComponentImplementation::updateCraftingValues(values, firstUpdate);
}

int LightsaberCrystalComponentImplementation::inflictDamage(TangibleObject* attacker, int damageType, float damage, bool destroy, bool notifyClient) {
	if (isDestroyed()) {
		return 0;
	}

	TangibleObjectImplementation::inflictDamage(attacker, damageType, damage, destroy, notifyClient);

	if (isDestroyed()) {
		ManagedReference<WeaponObject*> weapon = cast<WeaponObject*>(_this.getReferenceUnsafeStaticCast()->getParent().get()->getParent().get().get());

		if (weapon != nullptr) {
			if (getColor() == 31) {
				weapon->setAttackSpeed(weapon->getAttackSpeed() - getAttackSpeed());
				weapon->setMinDamage(weapon->getMinDamage() - getDamage());
				weapon->setMaxDamage(weapon->getMaxDamage() - getDamage());
				weapon->setHealthAttackCost(weapon->getHealthAttackCost() - getSacHealth());
				weapon->setActionAttackCost(weapon->getActionAttackCost() - getSacAction());
				weapon->setMindAttackCost(weapon->getMindAttackCost() - getSacMind());
				weapon->setWoundsRatio(weapon->getWoundsRatio() - getWoundChance());
				weapon->setForceCost(Math::max(1.0f, weapon->getForceCost() - getForceCost()));  //Minimum 1.0 force cost on sabers
			}

			if (getColor() != 31) {
				weapon->setBladeColor(31);
				weapon->setCustomizationVariable("/private/index_color_blade", 31, true);

				if (weapon->isEquipped()) {
					ManagedReference<CreatureObject*> parent = cast<CreatureObject*>(weapon->getParent().get().get());
					ManagedReference<SceneObject*> inventory = parent->getSlottedObject("inventory");
					inventory->transferObject(weapon, -1, true, true);
					parent->sendSystemMessage("@jedi_spam:lightsaber_no_color"); //That lightsaber can not be used until it has a color-modifying Force crystal installed.
				}
			}
		}
	}

	return 0;
}

// Infinity:  Added this to add even more checks against insane stats from CombatLevel being different than expected.
// This allows for any CombatLevel to be used moving forward. Even 99999!

float LightsaberCrystalComponentImplementation::hardCapStat(float minValue,float maxValue, float initialValue){
	
	float result = initialValue;
	bool inverted = minValue > maxValue;

	if (!inverted) { //Normal stat, higher is better

		if (result > maxValue) 
			result = maxValue;
		else if (result < minValue)
			result = minValue;
	}
	else { //Inverted stat, lower is better

		if (result < maxValue) 
			result = maxValue;
		else if (result > minValue)
			result = minValue;
	}

	return result;
}


int LightsaberCrystalComponentImplementation::hardCapStat(int minValue,int maxValue, int initialValue){
	
	int result = initialValue;
	bool inverted = minValue > maxValue;

	if (!inverted) { //Normal stat, higher is better

		if (result > maxValue) 
			result = maxValue;
		else if (result < minValue)
			result = minValue;
	}
	else { //Inverted stat, lower is better

		if (result < maxValue) 
			result = maxValue;
		else if (result > minValue)
			result = minValue;
	}

	return result;
}
