/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef FORCESHIELD2COMMAND_H_
#define FORCESHIELD2COMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"

class ForceShield2Command : public JediQueueCommand {
public:

	ForceShield2Command(const String& name, ZoneProcessServer* server)
		: JediQueueCommand(name, server) {

		buffCRC = BuffCRC::JEDI_FORCE_SHIELD_2;

		overrideableCRCs.add(BuffCRC::JEDI_FORCE_SHIELD_1);

		singleUseEventTypes.add(ObserverEventType::FORCEBUFFSHIELDHIT);

		skillMods.put("force_shield", 45);

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {
		return doJediSelfBuffCommand(creature);
	}

	void handleBuff(SceneObject* creature, ManagedObject* object, int64 param) {

		ManagedReference<CreatureObject*> creo = cast<CreatureObject*>( creature);
		if (creo == nullptr)
			return;

		// Client Effect upon hit (needed)
		creo->playEffect("clienteffect/pl_force_shield_hit.cef", "");

		ManagedReference<PlayerObject*> playerObject = creo->getPlayerObject();
		if (playerObject == nullptr)
			return;

		// TODO: Force Rank modifiers.
		int forceCost = param * 0.095;
		forceCost /= creo->getFrsMod("control");
		if (playerObject->getForcePower() <= forceCost) { // Remove buff if not enough force.
			Buff* buff = creo->getBuff(BuffCRC::JEDI_FORCE_SHIELD_2);
			if (buff != nullptr) {
				Locker locker(buff);

				creo->removeBuff(buff);
			}
		} 
		else {
			playerObject->setForcePower(playerObject->getForcePower() - forceCost);
			//creo->sendSystemMessage("*TC* Force Cost of Absorbing Hit on Force Shield  = " + String::valueOf(forceCost));
		}
	}

};

#endif //FORCESHIELD2COMMAND_H_
