/*
 * PetDeedImplementation.cpp
 *
 *  Created on: Aug 17, 2013
 *      Author: washu
 */

#include "server/zone/objects/tangible/deed/pet/PetDeed.h"
#include "server/zone/ZoneServer.h"
#include "server/zone/packets/object/ObjectMenuResponse.h"
#include "server/zone/objects/manufactureschematic/ManufactureSchematic.h"
#include "server/zone/objects/manufactureschematic/ingredientslots/IngredientSlot.h"
#include "server/zone/objects/manufactureschematic/ingredientslots/ComponentSlot.h"
#include "templates/tangible/PetDeedTemplate.h"
#include "server/zone/objects/creature/ai/CreatureTemplate.h"
#include "server/zone/managers/player/PlayerManager.h"
#include "server/zone/managers/creature/CreatureTemplateManager.h"
#include "server/zone/objects/intangible/PetControlDevice.h"
#include "server/zone/managers/creature/PetManager.h"
#include "server/zone/objects/player/PlayerObject.h"
#include "server/zone/objects/creature/ai/AiAgent.h"
#include "server/zone/objects/creature/ai/Creature.h"
#include "server/zone/objects/tangible/component/genetic/GeneticComponent.h"
#include "server/zone/managers/creature/CreatureManager.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "templates/customization/CustomizationIdManager.h"
#include "server/zone/objects/scene/variables/CustomizationVariables.h"
#include "server/zone/managers/stringid/StringIdManager.h"
#include "server/zone/managers/creature/DnaManager.h"
#include "server/zone/objects/creature/events/SampleDeedTask.h"
#include "server/zone/managers/crafting/labratories/Genetics.h"

void PetDeedImplementation::loadTemplateData(SharedObjectTemplate* templateData) {
	DeedImplementation::loadTemplateData(templateData);
	PetDeedTemplate* deedData = dynamic_cast<PetDeedTemplate*>(templateData);

	if (deedData == nullptr)
		return;

	controlDeviceObjectTemplate = deedData->getControlDeviceObjectTemplate();
	mobileTemplate = deedData->getMobileTemplate();
}

void PetDeedImplementation::fillAttributeList(AttributeListMessage* alm, CreatureObject* object) {
	DeedImplementation::fillAttributeList(alm, object);

	alm->insertAttribute("challenge_level", level);
	alm->insertAttribute("creature_health", health);
	alm->insertAttribute("creature_action", action);
	alm->insertAttribute("creature_mind", mind);

	// Armor Rating
	if (armor == 0)
		alm->insertAttribute("armor_rating", "None");
	else if (armor == 1)
		alm->insertAttribute("armor_rating", "Light");
	else if (armor == 2)
		alm->insertAttribute("armor_rating", "Medium");
	else if (armor == 3)
		alm->insertAttribute("armor_rating", "Heavy");

	// Resistances
	if (kinResist < 0)
		alm->insertAttribute("dna_comp_armor_kinetic", "Vulnerable");
	else
		alm->insertAttribute("dna_comp_armor_kinetic", kinResist);

	if (energyResist < 0)
		alm->insertAttribute("dna_comp_armor_energy", "Vulnerable");
	else
		alm->insertAttribute("dna_comp_armor_energy", energyResist);

	if (blastResist < 0)
		alm->insertAttribute("dna_comp_armor_blast", "Vulnerable");
	else
		alm->insertAttribute("dna_comp_armor_blast", blastResist);

	if (heatResist < 0)
		alm->insertAttribute("dna_comp_armor_heat", "Vulnerable");
	else
		alm->insertAttribute("dna_comp_armor_heat", heatResist);

	if (coldResist < 0)
		alm->insertAttribute("dna_comp_armor_cold", "Vulnerable");
	else
		alm->insertAttribute("dna_comp_armor_cold", coldResist);

	if (elecResist < 0)
		alm->insertAttribute("dna_comp_armor_electric", "Vulnerable");
	else
		alm->insertAttribute("dna_comp_armor_electric", elecResist);

	if (acidResist < 0)
		alm->insertAttribute("dna_comp_armor_acid", "Vulnerable");
	else
		alm->insertAttribute("dna_comp_armor_acid", acidResist);

	if (stunResist < 0)
		alm->insertAttribute("dna_comp_armor_stun", "Vulnerable");
	else
		alm->insertAttribute("dna_comp_armor_stun", stunResist);

	if (saberResist < 0)
		alm->insertAttribute("dna_comp_armor_saber", "Vulnerable");
	else
		alm->insertAttribute("dna_comp_armor_saber", saberResist);

	StringBuffer attdisplayValue;
	attdisplayValue << Math::getPrecision(attackSpeed, 2);

	StringBuffer hitdisplayValue;
	hitdisplayValue << Math::getPrecision(chanceHit, 2);
	alm->insertAttribute("creature_attack", attdisplayValue);
	alm->insertAttribute("creature_tohit", hitdisplayValue);
	alm->insertAttribute("creature_damage", String::valueOf(damageMin) + " - " + String::valueOf(damageMax));

	if (special1 != "none" && special1 != "defaultattack") {
		String str = StringIdManager::instance()->getStringId(("@combat_effects:" + special1).hashCode()).toString();
		alm->insertAttribute("spec_atk_1", str);
	} else
		alm->insertAttribute("spec_atk_1", "---");

	if (special2 != "none" && special2 != "defaultattack") {
		String str = StringIdManager::instance()->getStringId(("@combat_effects:" + special2).hashCode()).toString();
		alm->insertAttribute("spec_atk_2", str);
	} else
		alm->insertAttribute("spec_atk_2", "---");

	CreatureTemplateManager* creatureTemplateManager = CreatureTemplateManager::instance();
	Reference<CreatureTemplate*> petTemplate = creatureTemplateManager->getTemplate(mobileTemplate.hashCode());

	bool allowRanged = false;
	if (petTemplate != nullptr) {//Infinity:  This is the correct check
		String primary = petTemplate->getPrimaryWeapon();
		String secondary = petTemplate->getSecondaryWeapon();
		if (primary.contains("ranged") || primary.contains("spit") || secondary.contains("ranged") || secondary.contains("spit")) {  
			allowRanged = true;
		}
	}

	if (ranged && allowRanged)
		alm->insertAttribute("dna_comp_ranged_attack", "Yes");
	else
		alm->insertAttribute("dna_comp_ranged_attack", "No");

	alm->insertAttribute("sampling_attempts", getSampleCount());	

}

void PetDeedImplementation::initializeTransientMembers() {
	DeedImplementation::initializeTransientMembers();
	setupAttacks();
	setLoggingName("PetDeed");
}

void PetDeedImplementation::setupAttacks() {
	attacks.removeAll();
	if (special1 != "none" && special1 != "defaultattack") {
		String args = "";
		if (special1.contains("creatureareaattack") || special1.contains("creatureareableeding") || special1.contains("poison") || special1.contains("disease")){
			attacks.addAttack(special1,args);
		} else if(special1.contains("blind")) {
			attacks.addAttack(special1,"blindChance=100");
		}else if(special1.contains("dizzy")) {
			attacks.addAttack(special1,"dizzyChance=100");
		}else if(special1.contains("intimidation")) {
			attacks.addAttack(special1,"intimidationChance=100");
		}else if(special1.contains("stun")) {
			attacks.addAttack(special1,"stunChance=100");
		}else if(special1.contains("knockdown")) {
			attacks.addAttack(special1,"knockdownChance=100");
		}else if(special1.contains("posturedown")) {
			attacks.addAttack(special1,"postureDownChance=100");
		}else if(special1.contains("postureup")) {
			attacks.addAttack(special1,"postureUpChance=100");
		}else if(special1.contains("creatureareaknockdown")) {
			attacks.addAttack(special1,"stateAccuracyBonus=25");
		}else if(special1.contains("creatureareacombo")) {
			attacks.addAttack(special1,"stateAccuracyBonus=25");
		}
	}
	if (special2 != "none" && special2 != "defaultattack") {
		String args = "";
		if (special2.contains("creatureareaattack") || special2.contains("creatureareableeding") || special2.contains("poison") || special2.contains("disease")){
			attacks.addAttack(special2,args);
		} else if(special2.contains("blind")) {
			attacks.addAttack(special2,"blindChance=100");
		}else if(special2.contains("dizzy")) {
			attacks.addAttack(special2,"dizzyChance=100");
		}else if(special2.contains("intimidation")) {
			attacks.addAttack(special2,"intimidationChance=100");
		}else if(special2.contains("stun")) {
			attacks.addAttack(special2,"stunChance=100");
		}else if(special2.contains("knockdown")) {
			attacks.addAttack(special2,"knockdownChance=100");
		}else if(special2.contains("posturedown")) {
			attacks.addAttack(special2,"postureDownChance=100");
		}else if(special2.contains("postureup")) {
			attacks.addAttack(special2,"postureUpChance=100");
		}else if(special2.contains("creatureareaknockdown")) {
			attacks.addAttack(special2,"stateAccuracyBonus=25");
		}else if(special2.contains("creatureareacombo")) {
			attacks.addAttack(special2,"stateAccuracyBonus=25");
		}
	}
}

const CreatureAttackMap* PetDeedImplementation::getAttacks() const {
	return &attacks;
}

String PetDeedImplementation::getTemplateName() const {
	CreatureTemplateManager* creatureTemplateManager = CreatureTemplateManager::instance();
	Reference<CreatureTemplate*> petTemplate = creatureTemplateManager->getTemplate(mobileTemplate.hashCode());

	if (petTemplate == nullptr) {
		return "";
	}

	String name = petTemplate->getObjectName();
	return name;
}

CreatureTemplate* PetDeedImplementation::getCreatureTemplate() const {
	CreatureTemplateManager* creatureTemplateManager = CreatureTemplateManager::instance();

	if (creatureTemplateManager == nullptr)
		return nullptr;

	Reference<CreatureTemplate*> petTemplate = creatureTemplateManager->getTemplate(mobileTemplate.hashCode());

	return petTemplate;
}

int PetDeedImplementation::calculatePetLevel() {
	// Regenerate the LEvel
	int effective = (int)(((fortitude - (armor * 500)) / 50) * 5);
	int dps = ((damageMax + damageMin) / 2.0f) / attackSpeed;
	int avgHam = (health + action + mind) / 3;

	if (regen == 0) {
		regen = avgHam / 10;
	}
	return Genetics::calculateAgentLevel(avgHam, dps, chanceHit, regen, armor, effective, kinResist, energyResist, blastResist, heatResist, coldResist, elecResist, acidResist, stunResist,saberResist);
}

void PetDeedImplementation::updateCraftingValues(CraftingValues* values, bool firstUpdate) {
	ManagedReference<ManufactureSchematic*> manufact = values->getManufactureSchematic();
	float clFactor = 0;

	if (manufact == nullptr) {
		String templateName;
		SharedObjectTemplate* templ = getObjectTemplate();

		if (templ != nullptr) {
			templateName = templ->getFullTemplateString();
		}

		error("null manufacture schematic in PetDeedImplementation::updateCraftingValues for " + templateName);

		return;
	}

	// info(true) << "PetDeedImplementation::updateCraftingValues with Ingedient Slot Count: " << manufact->getSlotCount();

	for (int i = 0; i < manufact->getSlotCount(); ++i) {
		// Dna Component Slots
		Reference<IngredientSlot*> ingredSlot = manufact->getSlot(i);

		if (!ingredSlot->isComponentSlot())
			continue;

		ComponentSlot* componentSlot = cast<ComponentSlot*>(ingredSlot.get());

		if (componentSlot == nullptr)
			continue;

		ManagedReference<TangibleObject*> tanO = componentSlot->getPrototype();

		if (tanO == nullptr || tanO->getGameObjectType() != SceneObjectType::GENETICCOMPONENT)
			continue;

		ManagedReference<GeneticComponent*> component = cast<GeneticComponent*>(tanO.get());

		if (component == nullptr)
			continue;

		// Now we can input the values from the Genetic Component
		clFactor = component->getLevel();

		quality = component->getQuality();
		chanceHit = component->getHit();
		attackSpeed = component->getSpeed();

		damageMin = component->getMinDamage();
		damageMax = component->getMaxDamage();

		// Armor
		armor = component->getArmor();

		// Resistances
		kinResist = round(component->getKinetic());
		energyResist = round(component->getEnergy());
		blastResist = round(component->getBlast());
		coldResist = round(component->getCold());
		heatResist = round(component->getHeat());
		elecResist = round(component->getElectrical());
		acidResist = round(component->getAcid());
		stunResist = round(component->getStun());
		saberResist = round(component->getSaber());

		// HAM
		health = component->getHealth();
		action = component->getAction();
		mind = component->getMind();

		// Special Attacks
		special1 = component->getSpecial1();
		special2 = component->getSpecial2();
		ranged = component->getRanged();

		// Attributes
		cleverness = component->getCleverness();
		endurance = component->getEndurance();
		fierceness = component->getFierceness();
		power = component->getPower();
		intellect = component->getIntellect();
		courage = component->getCourage();
		dependability = component->getDependability();
		dexterity = component->getDexterity();
		fortitude = component->getFortitude();
		hardiness = component->getHardiness();

		if (component->isSpecialResist(SharedWeaponObjectTemplate::STUN)) {
			setSpecialResist(SharedWeaponObjectTemplate::STUN);
			// info(true) << "setting special resist STUN";
		}

		if (component->isSpecialResist(SharedWeaponObjectTemplate::KINETIC)) {
			setSpecialResist(SharedWeaponObjectTemplate::KINETIC);
			// info(true) << "setting special resist KINETIC";
		}

		if (component->isSpecialResist(SharedWeaponObjectTemplate::ENERGY)) {
			setSpecialResist(SharedWeaponObjectTemplate::ENERGY);
			//info(true) << "setting special resist ENERGY";
		}

		if (component->isSpecialResist(SharedWeaponObjectTemplate::BLAST)) {
			setSpecialResist(SharedWeaponObjectTemplate::BLAST);
			// info(true) << "setting special resist BLAST";
		}

		if (component->isSpecialResist(SharedWeaponObjectTemplate::HEAT)) {
			setSpecialResist(SharedWeaponObjectTemplate::HEAT);
			// info(true) << "setting special resist HEAT";
		}

		if (component->isSpecialResist(SharedWeaponObjectTemplate::COLD)) {
			setSpecialResist(SharedWeaponObjectTemplate::COLD);
			// info(true) << "setting special resist COLD";
		}

		if (component->isSpecialResist(SharedWeaponObjectTemplate::ELECTRICITY)) {
			setSpecialResist(SharedWeaponObjectTemplate::ELECTRICITY);
			// info(true) << "setting special resist ELECTRICITY";
		}

		if (component->isSpecialResist(SharedWeaponObjectTemplate::ACID)) {
			setSpecialResist(SharedWeaponObjectTemplate::ACID);
			// info(true) << "setting special resist ACID";
		}
		if (component->isSpecialResist(SharedWeaponObjectTemplate::LIGHTSABER)) {
			setSpecialResist(SharedWeaponObjectTemplate::LIGHTSABER);
			// info(true) << "setting special resist LIGHTSABER";
		}

		// Calculate and set level
		level = Genetics::calculatePetLevel(component);
	}

	CreatureTemplateManager* creatureTemplateManager = CreatureTemplateManager::instance();
	Reference<CreatureTemplate*> petTemplate = creatureTemplateManager->getTemplate(mobileTemplate.hashCode());

	if (petTemplate != nullptr) {
		// get min CL from the template
		int skinFactor = petTemplate->getLevel();

		if (level < skinFactor) {
			level = skinFactor;
		}
	}

	if (level > 95) {   //Allows for amazing/amazing to generate level 97 creature
		level = 95; 
	} 

	// setup attack map
	setupAttacks();
}

void PetDeedImplementation::fillObjectMenuResponse(ObjectMenuResponse* menuResponse, CreatureObject* player) {
	DeedImplementation::fillObjectMenuResponse(menuResponse, player);

	if (isASubChildOf(player))
		menuResponse->addRadialMenuItem(20, 3, "@pet/pet_menu:menu_tame");
	// Bio engineers can sample a deed
	if (player->hasSkill("outdoors_bio_engineer_novice") && isASubChildOf(player))
		menuResponse->addRadialMenuItem(21, 3, "@sui:harvest_dna");
}

int PetDeedImplementation::handleObjectMenuSelect(CreatureObject* player, byte selectedID) {
	if (selectedID == 21) {
		if (generated || !player->hasSkill("outdoors_bio_engineer_novice") || !isASubChildOf(player))
			return 1;

		if (player->isRidingMount()) {
			player->sendSystemMessage("You cannot sample DNA while mounted");
			return 1;
		}

		if (player->getPendingTask("sampledeed") != nullptr) {
			player->sendSystemMessage("@bio_engineer:harvest_dna_already_harvesting");
			return 1;
		}
		int skillMod = player->getSkillMod("dna_harvesting");
		if (skillMod < 1 || level > skillMod + 15) {
			player->sendSystemMessage("@bio_engineer:harvest_dna_skill_too_low");
			return 1;
		}

		Locker clocker(_this.getReferenceUnsafeStaticCast());

		Reference<SampleDeedTask*> task = new SampleDeedTask(_this.getReferenceUnsafeStaticCast(), player);

		player->addPendingTask("sampledeed", task, 0);
		return 0;
	} else if (selectedID == 20) {
		if (generated || !isASubChildOf(player))
			return 1;

		auto zone = player->getZone();

		if (zone == nullptr)
			return 1;

		if (player->isInCombat() || player->getParentRecursively(SceneObjectType::BUILDING) != nullptr) {
			player->sendSystemMessage("@pet/pet_menu:cant_call"); // You can only unpack vehicles while Outside and not in Combat.
			return 1;
		}

		ManagedReference<SceneObject*> datapad = player->getSlottedObject("datapad");

		if (datapad == nullptr) {
			player->sendSystemMessage("Datapad doesn't exist when trying to call pet");
			return 1;
		}
		if (datapad->getContainerObjectsSize() >= datapad->getContainerVolumeLimit()) {
			player->sendSystemMessage("@faction_recruiter:datapad_full"); // Your datapad is full. You must first free some space.
			return 1;
		}
		// Does the player have room
		ManagedReference<PlayerManager*> playerManager = player->getZoneServer()->getPlayerManager();

		int numberStored = 0;
		int maxStoredPets = playerManager->getBaseStoredCreaturePets() + player->getSkillMod("stored_pets");

		for (int i = 0; i < datapad->getContainerObjectsSize(); ++i) {
			ManagedReference<SceneObject*> object = datapad->getContainerObject(i);

			if (object != nullptr && object->isPetControlDevice()) {
				PetControlDevice* device = cast<PetControlDevice*>(object.get());

				if (device->getPetType() == PetManager::CREATUREPET) {
					if (++numberStored >= maxStoredPets) {
						player->sendSystemMessage("@pet/pet_menu:sys_too_many_stored"); // There are too many pets stored in this container. Release some of them to make room for more.
						return 1;
					}
				}
			}
		}

		// Can the player control it
		ManagedReference<PlayerObject*> ghost = player->getPlayerObject();

		int currentlySpawned = 0;
		int spawnedLevel = 0;
		int cLevel = level;
		int maxPets = player->getSkillMod("keep_creature");
		int maxLevelofPets = player->getSkillMod("tame_level");

		for (int i = 0; i < ghost->getActivePetsSize(); ++i) {
			ManagedReference<AiAgent*> object = ghost->getActivePet(i);

			if (object != nullptr && object->isCreature()) {
				if (++currentlySpawned >= maxPets) {
					player->sendSystemMessage("@pet/pet_menu:too_many"); // You can't control any more pets. Store one first
					return 1;
				}

				spawnedLevel += object->getLevel();

				if ((spawnedLevel + cLevel) > maxLevelofPets) {
					player->sendSystemMessage("Taming this pet would exceed your control level ability.");
					return 1;
				}
			}
		}

		Reference<CreatureManager*> creatureManager = zone->getCreatureManager();

		if (creatureManager == nullptr) {
			player->sendSystemMessage("Internal Pet Deed Error #307");
			return 1;
		}

		CreatureTemplateManager* creatureTemplateManager = CreatureTemplateManager::instance();
		Reference<CreatureTemplate*> petTemplate = creatureTemplateManager->getTemplate(mobileTemplate.hashCode());

		if (petTemplate == nullptr) {
			player->sendSystemMessage("wrong pet template;mobileTemplate=[" + mobileTemplate + "]");
			return 1;
		}

		bool isVicious = petTemplate->getPvpBitmask() & ObjectFlag::AGGRESSIVE;

		if (level > 10 || isVicious) {
			if (!player->hasSkill("outdoors_creaturehandler_novice") || (level > maxLevelofPets)) {
				player->sendSystemMessage("@pet/pet_menu:sys_lack_skill"); // You lack the skill to be able to tame that creature.
				return 1;
			}

			if (isVicious && player->getSkillMod("tame_aggro") < 1) {
				player->sendSystemMessage("@pet/pet_menu:sys_lack_skill"); // You lack the skill to be able to tame that creature.
				return 1;
			}
		}

		// All checks complete, lets setup the control device and do it.
		// Infinity: guard against deed templates with an empty controlDeviceObjectTemplate
		// field — hashCode("") == 0 produces TemplateManager "unknown template key 0x0"
		// + ObjectManager "could not create object CRC = 0x0" ERROR + stack trace,
		// and the player just sees nothing happen. Log enough to identify the
		// offending deed template so Wicked can patch the IFF.
		if (controlDeviceObjectTemplate.isEmpty()) {
			if (player != nullptr) {
				warning() << "PetDeed has empty controlDeviceObjectTemplate; cannot create PetControlDevice."
						  << " deedOID=" << getObjectID()
						  << " mobileTemplate=[" << mobileTemplate << "]"
						  << " player=" << player->getFirstName() << "(" << player->getObjectID() << ")";
			} else {
				warning() << "PetDeed has empty controlDeviceObjectTemplate; cannot create PetControlDevice."
						  << " deedOID=" << getObjectID()
						  << " mobileTemplate=[" << mobileTemplate << "]"
						  << " player=<null>";
			}
			return 1;
		}

		ManagedReference<PetControlDevice*> controlDevice = (server->getZoneServer()->createObject(controlDeviceObjectTemplate.hashCode(), 1)).castTo<PetControlDevice*>();

		if (controlDevice == nullptr) {
			player->sendSystemMessage("wrong pet control device;controlDevice=[" + controlDeviceObjectTemplate + "]");
			return 1;
		}

		Locker locker(controlDevice);

		String templateToSpawn = creatureManager->getTemplateToSpawn(mobileTemplate.hashCode());
		ManagedReference<CreatureObject*> creatureObject = creatureManager->createCreature(templateToSpawn.hashCode(), true, 0);

		if (creatureObject == nullptr) {
			controlDevice->destroyObjectFromDatabase(true);
			player->sendSystemMessage("wrong pet template;mobileTemplate=[" + mobileTemplate + "]");
			return 1;
		}

		Locker clocker(creatureObject, player);

		ManagedReference<Creature*> pet = creatureObject.castTo<Creature*>();

		if (pet == nullptr) {
			controlDevice->destroyObjectFromDatabase(true);
			creatureObject->destroyObjectFromDatabase(true);
			player->sendSystemMessage("Internal Pet Deed Error #348");
			return 1;
		}

		pet->setPetDeed(_this.getReferenceUnsafeStaticCast());
		pet->loadTemplateData(petTemplate);
		pet->setCustomObjectName(StringIdManager::instance()->getStringId(*pet->getObjectName()), true);
		pet->createChildObjects();
		pet->setBaby(false);

		// Equip the pets primary weapon in order to show their attacks on the control device
		pet->equipPrimaryWeapon();

		// update base stats on the pet now
		// We will store the deed pointer to the aiagent before serialization

		// then this is complete
		StringId s;
		s.setStringId(pet->getObjectName()->getFullPath());
		controlDevice->setControlledObject(pet);
		controlDevice->setObjectName(s, false);
		controlDevice->setPetType(PetManager::CREATUREPET);
		controlDevice->setMaxVitality(100);
		controlDevice->setVitality(100);
		controlDevice->setGrowthStage(1);
		controlDevice->updateStatus(1);

		if (!datapad->transferObject(controlDevice, -1)) {
			controlDevice->destroyObjectFromDatabase(true);
			return 1;
		}

		datapad->broadcastObject(controlDevice, true);
		controlDevice->growPet(player, true);

		// Infinity: Apply crafted colors BEFORE calling pet into world so the client
		// sees the correct hue on first spawn, not a random one from loadTemplateData.
		// Copy EACH index_color palette slider directly so multi-palette creatures
		// (e.g. distinct mane + body colors) keep every color — setHue alone only sets
		// the first palette slider, collapsing a 2-color pet to a single hue.
		CustomizationVariables* customVars = getCustomizationVariables();

		if (customVars != nullptr) {
			int dominantHue = -1;

			// pass 1: find the dominant crafted hue (used only to stamp hueValue)
			for (int i = 0; i < customVars->size(); ++i) {
				uint8 id = customVars->elementAt(i).getKey();
				String name = CustomizationIdManager::instance()->getCustomizationVariable(id);

				if (!name.contains("index_color")) {
					continue;
				}

				int16 val = customVars->elementAt(i).getValue();

				if (val > dominantHue) {
					dominantHue = val;
				}
			}

			// Stamp hueValue (>= 0) so the creature is not re-randomized on reload
			// (AiAgentImplementation re-rolls the hue only while getHueValue() == -1)
			// and so the call/recall path treats it as a colored pet. setHue also sets
			// the first slider; pass 2 below corrects every slider afterward.
			if (dominantHue >= 0) {
				pet->setHue(dominantHue);
			}

			// pass 2: apply each crafted slider so every palette keeps its own color
			for (int i = 0; i < customVars->size(); ++i) {
				uint8 id = customVars->elementAt(i).getKey();
				String name = CustomizationIdManager::instance()->getCustomizationVariable(id);

				if (!name.contains("index_color")) {
					continue;
				}

				pet->setCustomizationVariable(name, customVars->elementAt(i).getValue(), true);
			}
		}

		controlDevice->callObject(player);

		// Remove the deed from it's container.
		ManagedReference<SceneObject*> deedContainer = getParent().get();

		if (deedContainer != nullptr) {
			destroyObjectFromWorld(true);
		}

		generated = true;
		player->sendSystemMessage("@pet/pet_menu:device_added"); // "A control device has been added to your datapad."
		return 0;
	}

	return DeedImplementation::handleObjectMenuSelect(player, selectedID);
}

bool PetDeedImplementation::isSpecialResist(unsigned int type) const {
	return specialResists & type;
}

void PetDeedImplementation::setSpecialResist(unsigned int type) {
	if (!(specialResists & type))
		specialResists |= type;
}

void PetDeedImplementation::adjustPetLevel(CreatureObject* player, CreatureObject* pet) {
	int newLevel = calculatePetLevel();

	if (newLevel < 1 || newLevel > 95) {
		player->sendSystemMessage("@bio_engineer:pet_sui_fix_error");
		return;
	}

	level = newLevel;
	pet->reloadTemplate();
	player->sendSystemMessage("@bio_engineer:pet_sui_level_fixed");
}

bool PetDeedImplementation::adjustPetStats(CreatureObject* player, CreatureObject* pet) {
	int oldLevel = level;
	if (oldLevel < 1) {
		player->sendSystemMessage("@bio_engineer:pet_sui_fix_error");
		return false;
	}

	if (oldLevel > 95) {
		oldLevel = 95;
	}

	int ham = DnaManager::instance()->valueForLevel(DnaManager::HAM_LEVEL, oldLevel);
	health = ham;
	action = ham;
	mind = ham;

	regen = DnaManager::instance()->valueForLevel(DnaManager::REG_LEVEL, oldLevel);
	float dps = DnaManager::instance()->valueForLevel(DnaManager::DPS_LEVEL, oldLevel);

	damageMin = round((dps * 2.0) * 0.5);
	attackSpeed = 2.0;
	damageMax = round((dps * 2.0) * 1.5);
	chanceHit = DnaManager::instance()->valueForLevel(DnaManager::HIT_LEVEL, oldLevel);

	// Adjust Armor Now
	fortitude = DnaManager::instance()->valueForLevel(DnaManager::ARM_LEVEL, oldLevel);
	armor = fortitude / 500;
	// Infinity: Removed * 5
	float effectiveness = (int)((fortitude - (armor * 500)) / 50);
	if (!isSpecialResist(SharedWeaponObjectTemplate::KINETIC) && kinResist > 0)
		kinResist = effectiveness;
	if (!isSpecialResist(SharedWeaponObjectTemplate::ACID) && acidResist > 0)
		acidResist = effectiveness;
	if (!isSpecialResist(SharedWeaponObjectTemplate::BLAST) && blastResist > 0)
		blastResist = effectiveness;
	if (!isSpecialResist(SharedWeaponObjectTemplate::COLD) && coldResist > 0)
		coldResist = effectiveness;
	if (!isSpecialResist(SharedWeaponObjectTemplate::ELECTRICITY) && elecResist > 0)
		elecResist = effectiveness;
	if (!isSpecialResist(SharedWeaponObjectTemplate::ENERGY) && energyResist > 0)
		energyResist = effectiveness;
	if (!isSpecialResist(SharedWeaponObjectTemplate::HEAT) && heatResist > 0)
		heatResist = effectiveness;
	if (!isSpecialResist(SharedWeaponObjectTemplate::LIGHTSABER) && saberResist > 0)
		saberResist = effectiveness;
	if (!isSpecialResist(SharedWeaponObjectTemplate::STUN) && stunResist > 0)
		stunResist = effectiveness;

	// ensure the stats are set
	pet->reloadTemplate();

	player->sendSystemMessage("@bio_engineer:pet_sui_stats_fixed");
	return true;
}
