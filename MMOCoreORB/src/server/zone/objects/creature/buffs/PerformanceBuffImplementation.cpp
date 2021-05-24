/*
 * PerformanceBuffImplementation.cpp
 *
 *  Created on: 04/03/2011
 *      Author: Itac
 */

#include "server/zone/objects/creature/buffs/PerformanceBuff.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "templates/params/creature/CreatureAttribute.h"
#include "server/zone/objects/creature/buffs/PerformanceBuffType.h"
#include <string>
#include <iostream>

void PerformanceBuffImplementation::activate(bool applyModifiers) {
	// Change ent buffs to be based on a species max potential buff.
	// This is to avoid a bug with "Migrate Stats Anywhere" which we want to keep.
	int speciesMindValue = 1000;
	int speciesFocusValue = 500;
	int speciesWillValue = 500;
	
	int species = creature.get()->getSpecies();
	
	// Species detection and value changes
	switch (species) {
	case CreatureObject::HUMAN: 
		speciesMindValue = 1100;
		speciesFocusValue = 1100;
		speciesWillValue = 1100;
		break;
	case CreatureObject::RODIAN:
		speciesMindValue = 1100;
		speciesFocusValue = 1100;
		speciesWillValue = 1100;
		break;
	case CreatureObject::TRANDOSHAN:
		speciesMindValue = 1100;
		speciesFocusValue = 1100;
		speciesWillValue = 1100;
		break;
	case CreatureObject::MONCAL:
		speciesMindValue = 1300;
		speciesFocusValue = 1100;
		speciesWillValue = 1100;
		break;
	case CreatureObject::WOOKIE:
		speciesMindValue = 1100;
		speciesFocusValue = 1100;
		speciesWillValue = 1100;
		break;
	case CreatureObject::BOTHAN:
		speciesMindValue = 1100;
		speciesFocusValue = 1100;
		speciesWillValue = 1100;
		break;
	case CreatureObject::TWILEK:
		speciesMindValue = 1100;
		speciesFocusValue = 1100;
		speciesWillValue = 1100;
		break;
	case CreatureObject::ZABRAK:
		speciesMindValue = 1100;
		speciesFocusValue = 1100;
		speciesWillValue = 1100;
		break;
	case CreatureObject::ITHORIAN:
		speciesMindValue = 1300;
		speciesFocusValue = 1100;
		speciesWillValue = 1100;
		break;
	case CreatureObject::SULLUSTAN:
		speciesMindValue = 1200;
		speciesFocusValue = 1100;
		speciesWillValue = 1100;
		break;
	case CreatureObject::CHISS: 
		speciesMindValue = 1100;
		speciesFocusValue = 1100;
		speciesWillValue = 1100;
		break;
	default:
		speciesMindValue = 1100;
		speciesFocusValue = 1100;
		speciesWillValue = 1100;
		break;
	}
	
	// If the entertainer has ANY skill mod bonuses for enhancing musician/dancer buffs then we apply this here.
	if (strength > 1.0f){
		speciesMindValue = round(strength * (float)speciesMindValue);
		speciesFocusValue = round(strength * (float)speciesFocusValue);
		speciesWillValue = round(strength * (float)speciesWillValue);
	}

// Kept original values in case we want to revert this easily later~
	if(type == PerformanceBuffType::DANCE_MIND) {
		//int mindStrength = round(strength * (float)creature.get()->getBaseHAM(CreatureAttribute::MIND));
		int mindStrength = speciesMindValue;
		setAttributeModifier(CreatureAttribute::MIND, mindStrength);
		creature.get()->sendSystemMessage("@healing:performance_enhance_dance_mind_d");		
	}
	else if(type == PerformanceBuffType::MUSIC_FOCUS) {
		//int focusStrength = round(strength * (float)creature.get()->getBaseHAM(CreatureAttribute::FOCUS));
		int focusStrength = speciesFocusValue;
		setAttributeModifier(CreatureAttribute::FOCUS, focusStrength);
		creature.get()->sendSystemMessage("@healing:performance_enhance_music_focus_d");		
	}
	else if(type == PerformanceBuffType::MUSIC_WILLPOWER) {
		//int willStrength = round(strength * (float)creature.get()->getBaseHAM(CreatureAttribute::WILLPOWER));
		int willStrength = speciesWillValue;
		setAttributeModifier(CreatureAttribute::WILLPOWER, willStrength);
		creature.get()->sendSystemMessage("@healing:performance_enhance_music_willpower_d");
	}

	BuffImplementation::activate(true);
}

void PerformanceBuffImplementation::deactivate(bool removeModifiers) {
	BuffImplementation::deactivate(true);
}
