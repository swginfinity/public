#include "server/zone/objects/tangible/pharmaceutical/EnhancePack.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/managers/skill/SkillModManager.h"
#include "server/zone/objects/building/BuildingObject.h"
//Infinity: Custom includes
#include "server/zone/objects/region/CityRegion.h"
#include "server/zone/objects/player/FactionStatus.h"

uint32 EnhancePackImplementation::calculatePower(CreatureObject* healer, CreatureObject* patient, bool applyBattleFatigue) {
		float power = getEffectiveness();

		if (applyBattleFatigue)
			power *= patient->calculateBFRatio();

		int mod = healer->getSkillModOfType("private_medical_rating", SkillModManager::CITY);

		int droidBuff = healer->getSkillModOfType("private_medical_rating",SkillModManager::DROID);
		int bldBuff = healer->getSkillModOfType("private_medical_rating", SkillModManager::STRUCTURE);

		mod += droidBuff > bldBuff ? droidBuff : bldBuff;  //Infinity: Reverse Mantis #8885

		// Infinity: faction HQ medical rating bonus disabled — INFINITY_FACTION_MED_ENHANCE_DISABLED
		// int factionPerk = healer->getSkillMod("private_faction_medical_rating");
		// if (building != nullptr && factionPerk > 0 && ...) mod += factionPerk;

		float modEnvironment = ((float) mod / 100);
		float modSkill = (float) healer->getSkillMod("healing_wound_treatment");

		return power * modEnvironment * (100 + modSkill) / 100;
}
