/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions. */


#include "templates/params/creature/CreatureAttribute.h"
#include "templates/params/creature/CreatureState.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/objects/creature/commands/effect/CommandEffect.h"
#include "DamageOverTime.h"
#include "server/zone/packets/object/ShowFlyText.h"
#include "server/zone/managers/combat/CombatManager.h"
#include "server/zone/managers/skill/SkillManager.h"
#include "server/zone/ZoneServer.h"

DamageOverTime::DamageOverTime() {
	setAttackerID(0);
	setType(CreatureState::BLEEDING);
	setAttribute(CreatureAttribute::HEALTH);
	strength = 0;
	setDuration(0);
	setExpires(Time((uint32) 0));
	setSecondaryStrength(0);
	addSerializableVariables();
}

DamageOverTime::DamageOverTime(CreatureObject* attacker,
							   uint64 tp,
							   uint8 attrib,
							   uint32 str,
							   uint32 dur,
							   int secondaryStrength) {

	if (attacker != NULL)
		setAttackerID(attacker->getObjectID());

	setType(tp);
	setAttribute(attrib);
	strength = str;
	setDuration(dur);
	setSecondaryStrength(secondaryStrength);
	applied.updateToCurrentTime();
	activate();

	addSerializableVariables();
}

DamageOverTime::DamageOverTime(const DamageOverTime& dot) : Object(), Serializable() {
	addSerializableVariables();

	attackerID = dot.attackerID;
	type = dot.type;
	attribute = dot.attribute;
	strength = dot.strength;
	duration = dot.duration;
	applied = dot.applied;
	expires = dot.expires;
	nextTick = dot.nextTick;
	secondaryStrength = dot.secondaryStrength;

}

DamageOverTime& DamageOverTime::operator=(const DamageOverTime& dot) {
	if (this == &dot)
		return *this;

	attackerID = dot.attackerID;
	type = dot.type;
	attribute = dot.attribute;
	strength = dot.strength;
	duration = dot.duration;
	applied = dot.applied;
	expires = dot.expires;
	nextTick = dot.nextTick;
	secondaryStrength = dot.secondaryStrength;


	return *this;
}

void DamageOverTime::addSerializableVariables() {
	addSerializableVariable("attackerID", &attackerID);
	addSerializableVariable("type", &type);
	addSerializableVariable("attribute", &attribute);
	addSerializableVariable("strength", &strength);
	addSerializableVariable("duration", &duration);
	addSerializableVariable("applied", &applied);
	addSerializableVariable("expires", &expires);
	addSerializableVariable("nextTick", &nextTick);
	addSerializableVariable("secondaryStrength", &secondaryStrength);

}

void DamageOverTime::activate() {
	expires.updateToCurrentTime();
	expires.addMiliTime(duration * 1000);
}

uint32 DamageOverTime::applyDot(CreatureObject* victim) {
	if (expires.isPast() || !nextTick.isPast())
		return 0;

	nextTick.updateToCurrentTime();

	uint32 power = 0;
	ManagedReference<CreatureObject*> attacker = victim->getZoneServer()->getObject(attackerID).castTo<CreatureObject*>();

	if (attacker == NULL)
		attacker = victim;

	switch(type) {
	case CreatureState::BLEEDING:
		power = doBleedingTick(victim, attacker);
		nextTick.addMiliTime(20000);
		break;
	case CreatureState::POISONED:
		power = doPoisonTick(victim, attacker);
		nextTick.addMiliTime(10000);
		break;
	case CreatureState::DISEASED:
		power = doDiseaseTick(victim, attacker);
		nextTick.addMiliTime(40000);
		break;
	case CreatureState::ONFIRE:
		power = doFireTick(victim, attacker);
		nextTick.addMiliTime(10000);
		break;
	case CommandEffect::FORCECHOKE:
		power = doForceChokeTick(victim, attacker);
		nextTick.addMiliTime(6000);
		break;
	case CommandEffect::FORCEBURN:
		power = doForceBurnTick(victim, attacker);
		nextTick.addMiliTime(10000);
		break;

	}

	return power;
}

uint32 DamageOverTime::initDot(CreatureObject* victim, CreatureObject* attacker) {
	uint32 power = 0;
	int absorptionMod = 0;
	nextTick.updateToCurrentTime();

	switch(type) {
	case CreatureState::BLEEDING:
		absorptionMod = Math::max(0, Math::min(50, victim->getSkillMod("absorption_bleeding")));
		nextTick.addMiliTime(20000);
		break;
	case CreatureState::ONFIRE:
		absorptionMod = Math::max(0, Math::min(50, victim->getSkillMod("absorption_fire")));
		nextTick.addMiliTime(10000);
		break;
	case CommandEffect::FORCEBURN:
		absorptionMod = Math::max(0, Math::min(50, victim->getSkillMod("absorption_fire")));
		nextTick.addMiliTime(10000);
		break;
	case CreatureState::POISONED:
		absorptionMod = Math::max(0, Math::min(50, victim->getSkillMod("absorption_poison")));
		nextTick.addMiliTime(10000);
		break;
	case CreatureState::DISEASED:
		absorptionMod = Math::max(0, Math::min(50, victim->getSkillMod("absorption_disease")));
		nextTick.addMiliTime(40000);
		break;
	case CommandEffect::FORCECHOKE:
		nextTick.addMiliTime(6000);
		strength = (float)(strength * 0.01f) + (strength * (System::random(100) * 0.01f));
		victim->showFlyText("combat_effects", "choke", 0xFF, 0, 0);

		break;
	}

	power = (uint32)(strength * (1.f - absorptionMod / 100.f));

	//victim->addDamage(attacker,1);

	return power;
}

uint32 DamageOverTime::doBleedingTick(CreatureObject* victim, CreatureObject* attacker) {
	// TODO: Do we try to resist again?
	// we need to allow dots to tick while incapped, but not do damage
	if (victim->isIncapacitated() && victim->isFeigningDeath() == false)
		return 0;

	uint32 attr = victim->getHAM(attribute);
	int absorptionMod = Math::max(0, Math::min(50, victim->getSkillMod("absorption_bleeding")));

	// absorption reduces the strength of a dot by the given %.
	int damage = (int)(strength * (1.f - absorptionMod / 100.f));
	if (attr < damage) {
		//System::out << "setting strength to " << attr -1 << endl;
		damage = attr - 1;
	}

	Reference<CreatureObject*> attackerRef = attacker;
	Reference<CreatureObject*> victimRef = victim;
	auto attribute = this->attribute;

	Core::getTaskManager()->executeTask([victimRef, attackerRef, attribute, damage] () {
		Locker locker(victimRef);

		Locker crossLocker(attackerRef, victimRef);

		victimRef->inflictDamage(attackerRef, attribute, damage, false);

		if (victimRef->hasAttackDelay())
			victimRef->removeAttackDelay();

		victimRef->playEffect("clienteffect/dot_bleeding.cef","");
	}, "BleedTickLambda");

	return damage;
}

uint32 DamageOverTime::doFireTick(CreatureObject* victim, CreatureObject* attacker) {
	// we need to allow dots to tick while incapped, but not do damage
	if (victim->isIncapacitated() && victim->isFeigningDeath() == false)
		return 0;

	uint32 attr = victim->getHAM(attribute);
	int absorptionMod = Math::max(0, Math::min(50, victim->getSkillMod("absorption_fire")));

	// absorption reduces the strength of a dot by the given %.
	int damage = (int)(strength * (1.f - absorptionMod / 100.f));
	if (attr < damage) {
		//System::out << "setting strength to " << attr -1 << endl;
		damage = attr - 1;
	}

	int woundsToApply = (int)(secondaryStrength * (1.f + victim->getShockWounds() / 100.0f));
	int maxWoundsToApply = victim->getBaseHAM(attribute) - 1 - victim->getWounds(attribute);

	woundsToApply = Math::min(woundsToApply, maxWoundsToApply);

	Reference<CreatureObject*> attackerRef = attacker;
	Reference<CreatureObject*> victimRef = victim;
	auto attribute = this->attribute;
	auto secondaryStrength = this->secondaryStrength;

	Core::getTaskManager()->executeTask([victimRef, attackerRef, attribute, woundsToApply, secondaryStrength, damage] () {
		Locker locker(victimRef);

		Locker crossLocker(attackerRef, victimRef);

		if (woundsToApply > 0) {
			// need to do damage to account for wounds first, or it will possibly get
			// applied twice
			if (attribute % 3 == 0)
				victimRef->inflictDamage(attackerRef, attribute, woundsToApply, true);

			victimRef->addWounds(attribute, woundsToApply, true, false);
		}

		victimRef->addShockWounds((int)(secondaryStrength * 0.075f));

		victimRef->inflictDamage(attackerRef, attribute - attribute % 3, damage, true);
		if (victimRef->hasAttackDelay())
			victimRef->removeAttackDelay();

		victimRef->playEffect("clienteffect/dot_fire.cef","");
	}, "FireTickLambda");

	return damage;
}

uint32 DamageOverTime::doPoisonTick(CreatureObject* victim, CreatureObject* attacker) {
	// we need to allow dots to tick while incapped, but not do damage
	if (victim->isIncapacitated() && victim->isFeigningDeath() == false)
		return 0;

	uint32 attr = victim->getHAM(attribute);
	int absorptionMod = Math::max(0, Math::min(50, victim->getSkillMod("absorption_poison")));

	// absorption reduces the strength of a dot by the given %.
	int damage = (int)(strength * (1.f - absorptionMod / 100.f));
	if (attr < damage) {
		//System::out << "setting strength to " << attr -1 << endl;
		damage = attr - 1;
	}

	Reference<CreatureObject*> attackerRef = attacker;
	Reference<CreatureObject*> victimRef = victim;
	auto attribute = this->attribute;

	Core::getTaskManager()->executeTask([victimRef, attackerRef, attribute, damage] () {
		Locker locker(victimRef);

		Locker crossLocker(attackerRef, victimRef);

		victimRef->inflictDamage(attackerRef, attribute, damage, false);
		if (victimRef->hasAttackDelay())
			victimRef->removeAttackDelay();

		victimRef->playEffect("clienteffect/dot_poisoned.cef","");
	}, "PoisonTickLambda");

	return damage;
}

uint32 DamageOverTime::doDiseaseTick(CreatureObject* victim, CreatureObject* attacker) {
	// we need to allow dots to tick while incapped, but not do damage
	if (victim->isIncapacitated() && victim->isFeigningDeath() == false)
		return 0;

	int absorptionMod = Math::max(0, Math::min(50, victim->getSkillMod("absorption_disease")));

	// absorption reduces the strength of a dot by the given %.
	// make sure that the CM dots modify the strength
	int damage = (int)(strength * (1.f - absorptionMod / 100.f) * (1.f + victim->getShockWounds() / 100.0f));
	int maxDamage = victim->getBaseHAM(attribute) - 1 - victim->getWounds(attribute);

	damage = Math::min(damage, maxDamage);

	Reference<CreatureObject*> attackerRef = attacker;
	Reference<CreatureObject*> victimRef = victim;
	auto attribute = this->attribute;
	auto strength = this->strength;

	Core::getTaskManager()->executeTask([victimRef, attackerRef, attribute, damage, strength] () {
		Locker locker(victimRef);
		Locker crossLocker(attackerRef, victimRef);

		if ((int)damage > 0) {
			// need to do damage to account for wounds first, or it will possibly get
			// applied twice
			if (attribute % 3 == 0)
				victimRef->inflictDamage(attackerRef, attribute, damage, true);

			victimRef->addWounds(attribute, damage, true, false);
		}

		victimRef->addShockWounds((int)(strength * 0.075f));

		if (victimRef->hasAttackDelay())
			victimRef->removeAttackDelay();

		victimRef->playEffect("clienteffect/dot_diseased.cef","");
	}, "DiseaseTickLambda");

	return damage;
}

uint32 DamageOverTime::doForceChokeTick(CreatureObject* victim, CreatureObject* attacker) {
	static uint8 legLocations[] = {4,5};
	static uint8 bodyLocations[] = {1, 1, 2, 3};

	// we need to allow dots to tick while incapped, but not do damage
			if (victim->isIncapacitated() && victim->isFeigningDeath() == false)
		return 0;

	if (!victim->getZoneServer())
		return 0;

	SkillManager* skillManager =  victim->getZoneServer()->getSkillManager();

	Reference<CreatureObject*> attackerRef = attacker;
	Reference<CreatureObject*> victimRef = victim;
	auto attribute = this->attribute;
	auto strength = this->strength;


		 if (!victim->isPlayerCreature() && attacker->isPlayerCreature()) { //Ticks much harder on NPCS
			strength *= 2.25;
		 }

		 if (attacker->isPlayerCreature()) {
			if (victim->isProne() || victim->isKnockedDown()) {
				strength *= 1.5;
			} else if (victim->isKneeling()) {
				strength *= 1.25;
			}
		 }

		int targetHamTotal = victim->getHAM(CreatureAttribute::HEALTH); //Force choke ticks on the currently lowest stat
			attribute = CreatureAttribute::HEALTH;

		if (victim->getHAM(CreatureAttribute::ACTION) < targetHamTotal){
			attribute = CreatureAttribute::ACTION;
			targetHamTotal = victim->getHAM(CreatureAttribute::ACTION);

		}
		if (victim->getHAM(CreatureAttribute::MIND) < targetHamTotal){
			attribute = CreatureAttribute::MIND;
		}


                if (attackerRef->isPlayerCreature()){ //player is using force powers, apply cloak of pain buff
                        uint32 buffCRC = BuffCRC::JEDI_CLOAKOFPAIN;
                        Buff* buff = attackerRef->getBuff(buffCRC);
                                if (buff != NULL){ //buff currently active, extend
																				Locker plocker(attackerRef);
                                        attackerRef->renewBuff(buffCRC, 8, true);
                                }
                                else{ //New buff

                                        ManagedReference<Buff*> cloakBuff = new Buff(attackerRef, buffCRC, 8, BuffType::JEDI);
                                        int cloakStrength = 7;
                                        cloakStrength += 2*skillManager->getSpecificSkillCount(attackerRef,"force_discipline_powers"); //Cloak strength scales with boxes purchased in powers
					if (cloakStrength == 43)  // Give 2 points of bonus at master box
							cloakStrength = 45;
                                        Locker locker(cloakBuff);
                                        Locker plocker(attackerRef);
                                        cloakBuff->setSkillModifier("cloakofpain", cloakStrength);
                                        attackerRef->addBuff(cloakBuff);
                                        attackerRef->showFlyText("combat_effects", "cloakofpain", 0, 255, 0); // +Cloak of Pain+
                                }
                }


		CombatManager* combatManager = CombatManager::instance(); 

		uint8 hitLocation;
		
			if (attribute == CreatureAttribute::HEALTH){
				hitLocation = bodyLocations[System::random(3)];
			}
			else if (attribute == CreatureAttribute::ACTION){
				hitLocation = legLocations[System::random(1)];
			}
			else {
				hitLocation = 6;
			}

			ManagedReference<ArmorObject*> psg = combatManager->getPSGArmor(victim);
			

			if (psg != NULL && !psg->isVulnerable(SharedWeaponObjectTemplate::LIGHTSABER) ) {
			float armorReduction =  combatManager->checkArmorObjectReduction(psg, SharedWeaponObjectTemplate::LIGHTSABER);
			float dmgAbsorbed = strength;
			if (armorReduction > 45)
					armorReduction = 45; //Hardcap for legacy PSGS

				if (victim->hasBuff(BuffCRC::JEDI_FORCEBREACH))
						armorReduction *= .75;
				if (armorReduction > 0) strength *= 1.f - (armorReduction / 100.f);

				dmgAbsorbed -= strength;
				if (dmgAbsorbed > 0)
					combatManager->sendMitigationCombatSpam(victim, psg, (int)dmgAbsorbed, 0x1);
			}
			
				// Standard Armor
			ManagedReference<ArmorObject*> armor = NULL;

			armor = combatManager->getArmorObject(victim, hitLocation);
			if (armor != NULL && !armor->isVulnerable(SharedWeaponObjectTemplate::LIGHTSABER)) {
				float armorReduction = combatManager->checkArmorObjectReduction(armor, SharedWeaponObjectTemplate::LIGHTSABER);
				float dmgAbsorbed = strength;
				float preArmorDamage = strength;
				int armorPiercing = 2;	
				
				if (victim->hasBuff(BuffCRC::JEDI_FORCEBREACH))
					armorPiercing++;
				
				if (armorReduction > 80)
					armorReduction = 80;
				strength *= combatManager->checkArmorPiercing(armor, armorPiercing);
				if (preArmorDamage < strength && victim->isPlayerCreature()) //players cannot take more damage than you would have unarmored.
					strength = preArmorDamage;
						if (armorReduction > 0) {
					strength *= (1.f - (armorReduction / 100.f));
					dmgAbsorbed -= strength;
					combatManager->sendMitigationCombatSpam(victim, armor, (int)dmgAbsorbed, 0x6);
				}
			}
			

	Core::getTaskManager()->executeTask([victimRef, attackerRef, attribute, strength] () {
		Locker locker(victimRef);

		Locker crossLocker(attackerRef, victimRef);

		int strMod = strength;
		if (!attackerRef->isPlayerCreature())
			strMod *= .5;
	
		victimRef->inflictDamage(attackerRef, attribute, strMod, true);

		victimRef->playEffect("clienteffect/pl_force_choke.cef", "");
		victimRef->sendSystemMessage("@combat_effects:choke_single");
		victimRef->showFlyText("combat_effects", "choke", 0xFF, 0, 0);
	}, 	"ForceChokeTickLambda");

	return strength;

}


uint32 DamageOverTime::doForceBurnTick(CreatureObject* victim, CreatureObject* attacker) {
	uint32 attr = 0;

	// we need to allow dots to tick while incapped, but not do damage
	if (victim->isIncapacitated() && victim->isFeigningDeath() == false)
		return 0;

	//Burn a random Stat
	switch (System::random(2)){
		case 0:
		attr = victim->getHAM(CreatureAttribute::HEALTH);
		break;
		case 1:
		attr = victim->getHAM(CreatureAttribute::ACTION);
		break;
		case 2:
		attr = victim->getHAM(CreatureAttribute::MIND);
		break;
	}


	int absorptionMod = Math::max(0, Math::min(50, victim->getSkillMod("absorption_fire")));


		 if (!victim->isPlayerCreature() && attacker->isPlayerCreature()) { //Ticks much harder on NPCS
			strength *= 2.25;
		 }

		 if (attacker->isPlayerCreature()) {
			if (victim->isProne() || victim->isKnockedDown()) {
				strength *= 1.5;
			} else if (victim->isKneeling()) {
				strength *= 1.25;
			}
		 }

	// absorption reduces the strength of a dot by the given %.
	int damage = (int)(strength * (1.f - absorptionMod / 100.f));
	damage += victim->getMaxHAM(attribute) * .01; //Bonus damage = 1% max health

	attacker->sendSystemMessage("Burn tick on victim for " + String::valueOf(damage));

	if (attr < damage) {
		damage = attr - 1;
	}

	int woundsToApply = (int)((damage * (1.f + victim->getShockWounds() / 100.0f)))/2;
	attacker->sendSystemMessage("Burn wounds on victim for " + String::valueOf(woundsToApply));
	int maxWoundsToApply = victim->getBaseHAM(attribute) - 1 - victim->getWounds(attribute);

	woundsToApply = Math::min(woundsToApply, maxWoundsToApply);

	Reference<CreatureObject*> attackerRef = attacker;
	Reference<CreatureObject*> victimRef = victim;
	auto attribute = this->attribute;
	auto secondaryStrength = this->secondaryStrength;

	Core::getTaskManager()->executeTask([victimRef, attackerRef, attribute, woundsToApply, secondaryStrength, damage] () {
		Locker locker(victimRef);

		Locker crossLocker(attackerRef, victimRef);

		if (woundsToApply > 0) {
			if (attribute % 3 == 0)
				victimRef->inflictDamage(attackerRef, attribute, woundsToApply, true);

			victimRef->addWounds(attribute, woundsToApply, true, false);
		}

		victimRef->addShockWounds((int)(secondaryStrength * 0.075f));

		victimRef->inflictDamage(attackerRef, attribute - attribute % 3, damage, true);
		if (victimRef->hasAttackDelay())
			victimRef->removeAttackDelay();

		victimRef->playEffect("clienteffect/dot_fire.cef","");
	}, "FireTickLambda");

	return damage;
}

float DamageOverTime::reduceTick(float reduction) {
	//System::out << "reducing tick with reduction " << reduction << endl;
	if (reduction < 0.f) // this ensures we can't increase a dot strength
		return reduction;

	if (reduction >= strength) {
		expireTick();
		return reduction - strength;
	} else {
		//System::out << "strength before dotRed " << strength << endl;
		strength -= reduction;
		//System::out << "strength after dotRed " << strength << endl;
	}

	return 0.f;
}

void DamageOverTime::multiplyDuration(float multiplier) {
	Time newTime;
	uint64 timeToAdd = (expires.getMiliTime() - newTime.getMiliTime()) * multiplier;
	newTime.addMiliTime(timeToAdd);
	expires = newTime;

}
