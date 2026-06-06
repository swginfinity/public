/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef CITYSTATUSCOMMAND_H_
#define CITYSTATUSCOMMAND_H_

class CityStatusCommand : public QueueCommand {
public:
	CityStatusCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {
		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		if (!creature->isPlayerCreature())
			return GENERALERROR;

		byte rank = 0;
		StringTokenizer args(arguments.toString());
		String planet;

		if(args.hasMoreTokens()) {
			args.getStringToken(planet);
		} else {
			sendSyntax(creature);
			return INVALIDSYNTAX;
		}

		if(creature->getZoneServer() == nullptr)
			return GENERALERROR;

		CityManager* cityManager = creature->getZoneServer()->getCityManager();

		if(cityManager == nullptr)
			return GENERALERROR;

		if (!creature->checkCooldownRecovery("cityStatus"))  {
			const Time* cooldownTime = creature->getCooldownTime("cityStatus");
			if (cooldownTime != nullptr) {
				float timeLeft = fabs(cooldownTime->miliDifference()) / 1000.0f;
				creature->sendSystemMessage("You can use the cityStatus command again in " + String::format("%.1f", timeLeft) + " second" + ((timeLeft == 1.0f) ? "." : "s."));
				return GENERALERROR;
			} 
			else {
				creature->sendSystemMessage("You can only use the cityStatus command once every 5 seconds.");
                return GENERALERROR;
			}
		}
		creature->updateCooldownTimer("cityStatus", 60 * 1000);

		cityManager->sendCityStatusReport(creature, planet);
		return SUCCESS;
	}

	static void sendSyntax(CreatureObject* creature) {
		String syntax = "/citystatus [planetname]";
		creature->sendSystemMessage(syntax);
	}
};

#endif //CITYSTATUSCOMMAND_H_
