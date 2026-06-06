
#ifndef RECHARGEDROIDCOMMAND_H_
#define RECHARGEDROIDCOMMAND_H_

#include "server/zone/objects/creature/commands/QueueCommand.h"
#include "server/zone/objects/creature/ai/DroidObject.h"

class RechargeDroidCommand : public QueueCommand {
public:
	RechargeDroidCommand(const String& name, ZoneProcessServer* server) : QueueCommand(name, server) {
	}

	int doQueueCommand(CreatureObject* player, const uint64& target, const UnicodeString& arguments) const {
		if (player == nullptr) {
			return GENERALERROR;
		}

		if (player->isDead() || player->isIncapacitated()) {
			return INVALIDSTATE;
		}

		auto playerGhost = player->getPlayerObject();

		if (playerGhost == nullptr) {
			return GENERALERROR;
		}

		ManagedReference<CreatureObject*> droidCreature = nullptr;
		ManagedReference<PetControlDevice*> controlDevice = nullptr;

		for (int i = 0; i < playerGhost->getActivePetsSize(); i++) {
			ManagedReference<AiAgent*> targetPet = playerGhost->getActivePet(i);

			if (targetPet == nullptr) {
				continue;
			}

			controlDevice = targetPet->getControlDevice().get().castTo<PetControlDevice*>();

			if (controlDevice == nullptr) {
				continue;
			}

			if (controlDevice->getPetType() == PetManager::DROIDPET) {
				droidCreature = targetPet->asCreatureObject();
				break;
			}
		}

		if (droidCreature == nullptr || controlDevice == nullptr || droidCreature->isDead() || droidCreature->isIncapacitated()) {
			return GENERALERROR;
		}

		if (!checkDistance(player, droidCreature, 16.0f)) {
			player->sendSystemMessage("You must be within 16 meters of your droid to recharge.");
			return GENERALERROR;
		}

		auto droidPet = droidCreature.castTo<DroidObject*>();

		if (droidPet == nullptr) {
			return GENERALERROR;
		}

		int batteryThreshold = 99; // 99% default threshold
		int percentPower = floor((droidPet->getPower() / 1000.f) * 100.f);
		bool hasArg = false;

		StringTokenizer args(arguments.toString());

		try {
			if (args.hasMoreTokens()) {
				int customThreshold = args.getIntToken();
				if (customThreshold > 0 and customThreshold <= 100) {
					batteryThreshold = customThreshold;
					hasArg = true;
				}
			}

		} catch (Exception& e) {
			return INVALIDPARAMETERS;
		}

		if (hasArg) {
			if (percentPower > batteryThreshold) {
				player->sendSystemMessage("Your droid is already above your requested " + String::valueOf(batteryThreshold) + "% power threshold.");
				return GENERALERROR;
			}
		} else {
			if (percentPower > batteryThreshold) {
				player->sendSystemMessage("Your droid is already at 100% power.");
				return GENERALERROR;
			}
		}

		// Recharge
		Locker clocker(droidPet, player);
		droidPet->rechargeFromBattery(player);

		return SUCCESS;
	}
};

#endif /* RECHARGEDROIDCOMMAND_H_ */
