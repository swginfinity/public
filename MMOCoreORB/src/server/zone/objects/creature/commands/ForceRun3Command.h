/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions. */

#ifndef FORCERUN3COMMAND_H_
#define FORCERUN3COMMAND_H_

#include "server/zone/objects/creature/buffs/PrivateSkillMultiplierBuff.h"
#include "JediQueueCommand.h"

class ForceRun3Command : public JediQueueCommand {
public:
	ForceRun3Command(const String& name, ZoneProcessServer* server) : JediQueueCommand(name, server) {
		// BuffCRC's, first one is used.
		buffCRC = BuffCRC::JEDI_FORCE_RUN_3;

		// If these are active they will block buff use
		blockingCRCs.add(BuffCRC::JEDI_FORCE_RUN_1);
		blockingCRCs.add(BuffCRC::JEDI_FORCE_RUN_2);
		blockingCRCs.add(STRING_HASHCODE("luaSetSpeed"));

		skillMods.put("force_run", 3);
		skillMods.put("slope_move", 99);
	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		// Infinity: Block if speed is being controlled by Lua (e.g. screenplay)
		if (creature->hasBuff(STRING_HASHCODE("luaSetSpeed"))) {
			creature->sendSystemMessage("You cannot use Force Run 3 while your speed is being controlled by an unknown entity.");
			return GENERALERROR;
		}

		// Infinity: Toggle force run off if already active, with 10s cooldown scaled by FRS
		if (creature->hasBuff(buffCRC)) {
			creature->removeBuff(buffCRC);
			creature->updateCooldownTimer("forcerun3", 10000/creature->getFrsMod("manipulation"));
			return SUCCESS;
		}

		// Infinity: FR3 cannot be activated in combat
		if (creature->isInCombat()) {
			creature->sendSystemMessage("You can't focus enough to use Force Run 3 in combat.");
			return GENERALERROR;
		}

		// Infinity: FR3 cannot be used with active PvP/BH TEF
		ManagedReference<PlayerObject*> ghost = creature->getPlayerObject();

		if (ghost != nullptr) {
			if (ghost->hasPvpTef() || ghost->hasBhTef()) {
				creature->sendSystemMessage("You cannot use Force Run 3 so soon after being in PVP combat.");
				return GENERALERROR;
			}
		}

		if (!creature->checkCooldownRecovery("forcerun3")) {
			creature->sendSystemMessage("You cannot use Force Run 3 again yet.");
			return GENERALERROR;
		}

		int res = doJediSelfBuffCommand(creature);

		if (res != SUCCESS) {
			return res;
		}

		// Need to apply the damage reduction in a separate buff so that the multiplication and division applies right
		Buff* buff = creature->getBuff(BuffCRC::JEDI_FORCE_RUN_3);
		if (buff == nullptr) {
			return GENERALERROR;
		}

		ManagedReference<PrivateSkillMultiplierBuff*> multBuff = new PrivateSkillMultiplierBuff(creature, name.hashCode(), duration, BuffType::JEDI);

		Locker locker(multBuff);

		multBuff->setSkillModifier("private_damage_divisor_forcerun3", 20);

		creature->addBuff(multBuff);

		locker.release();

		Locker blocker(buff);

		buff->addSecondaryBuffCRC(multBuff->getBuffCRC());

		// Cancel all other speed buffs
		if (creature->hasBuff(STRING_HASHCODE("burstrun"))) {
			creature->removeBuff(STRING_HASHCODE("burstrun"));
		}

		if (creature->hasBuff(STRING_HASHCODE("gallop"))) {
			creature->removeBuff(STRING_HASHCODE("gallop"));
		}

		if (creature->hasBuff(STRING_HASHCODE("retreat"))) {
			creature->removeBuff(STRING_HASHCODE("retreat"));
		}

		if (creature->hasBuff(STRING_HASHCODE("dash"))) {
			creature->removeBuff(STRING_HASHCODE("dash"));
		}

		if (creature->hasBuff(STRING_HASHCODE("rocketboots"))) {
			creature->removeBuff(STRING_HASHCODE("rocketboots"));
		}

		creature->notifyObservers(ObserverEventType::ABILITYUSED, nullptr, STRING_HASHCODE("forcerun3"));

		return SUCCESS;
	}

};

#endif //FORCERUN3COMMAND_H_
