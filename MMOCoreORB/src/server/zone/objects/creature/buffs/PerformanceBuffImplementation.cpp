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

void PerformanceBuffImplementation::activate(bool applyModifiers) {

	if(type == PerformanceBuffType::DANCE_MIND) {
		//Infinity:  Min buff strength based on human mind stats of 1100
		int mindStrength = round(strength * Math::max(1100.0f,(float)creature.get()->getBaseHAM(CreatureAttribute::MIND)));
		setAttributeModifier(CreatureAttribute::MIND, mindStrength);
		creature.get()->sendSystemMessage("@healing:performance_enhance_dance_mind_d");

	}
	else if(type == PerformanceBuffType::MUSIC_FOCUS) {
		//Infinity:  Min buff strength based on human mind stats of 1100
		int focusStrength = round(strength * Math::max(1100.0f,(float)creature.get()->getBaseHAM(CreatureAttribute::FOCUS)));
		setAttributeModifier(CreatureAttribute::FOCUS, focusStrength);
		creature.get()->sendSystemMessage("@healing:performance_enhance_music_focus_d");

	}
	else if(type == PerformanceBuffType::MUSIC_WILLPOWER) {
		//Infinity:  Min buff strength based on human mind stats of 1100
		int willStrength = round(strength * Math::max(1100.0f,(float)creature.get()->getBaseHAM(CreatureAttribute::WILLPOWER)));
		setAttributeModifier(CreatureAttribute::WILLPOWER, willStrength);
		creature.get()->sendSystemMessage("@healing:performance_enhance_music_willpower_d");
	}

	BuffImplementation::activate(true);

}

void PerformanceBuffImplementation::deactivate(bool removeModifiers) {
	BuffImplementation::deactivate(true);
}
