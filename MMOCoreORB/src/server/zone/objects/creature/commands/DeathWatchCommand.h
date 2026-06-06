/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.

	Infinity: Death Watch Faction Ability Command
	Hidden faction-specific utility command available to DW members with Box 3 skills

	Effects by faction:
	- Zann Consortium: Espionage Matrix (deep cover infiltration)
	- Black Krrsantan: Savage Sprint (2.5x speed buff)
	- Black Sun: Shadow Protocol (stealth + attack capability)
	- Xonti Brothers: Genetic Harvest (special forage)

	Admin Commands:
	- /deathWatch on  - Enable Death Watch (deactivate kill switch)
	- /deathWatch off - Disable Death Watch (activate kill switch)
	- /deathWatch <faction> - Test any faction's ability (sun, krr, xon, zan)
*/

#ifndef DEATHWATCHCOMMAND_H_
#define DEATHWATCHCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/objects/player/PlayerObject.h"
#include "server/zone/managers/director/DirectorManager.h"

class DeathWatchCommand : public QueueCommand {
public:

	DeathWatchCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {
	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {
		if (!checkStateMask(creature)) {
			return INVALIDSTATE;
		}

		if (!checkInvalidLocomotions(creature)) {
			return INVALIDLOCOMOTION;
		}

		// Must be a player
		if (!creature->isPlayerCreature()) {
			return GENERALERROR;
		}

		// Get the arguments
		String args = arguments.toString().trim().toLowerCase();

		// Check if admin
		PlayerObject* ghost = creature->getPlayerObject();
		bool isAdmin = (ghost != nullptr && ghost->isPrivileged());

		// Admin-only: Kill switch commands (on/off)
		if (args == "on" || args == "off") {
			if (!isAdmin) {
				creature->sendSystemMessage("\\#FF6666This command requires administrator privileges.");
				return GENERALERROR;
			}

			// Call Lua handler for kill switch
			Lua* lua = DirectorManager::instance()->getLuaInstance();
			Reference<LuaFunction*> killSwitchFunc = lua->createFunction("dw_killswitch", "handleCommand", 0);

			*killSwitchFunc << creature;
			*killSwitchFunc << args;

			killSwitchFunc->callFunction();

			return SUCCESS;
		}

		// Admin-only: Faction override for testing (sun, krr, xon, zan)
		bool hasFactionArg = (args == "sun" || args == "krr" || args == "xon" || args == "zan");

		if (isAdmin && hasFactionArg) {
			// Pass through to Lua - it will handle the override
		} else {
			// Normal player - must have Death Watch Novice skill
			if (!creature->hasSkill("combat_death_watch_novice")) {
				creature->sendSystemMessage("\\#FF6666Command not available yet.");
				return GENERALERROR;
			}

			// Check for Box 3 skill in any faction to determine if command is available
			bool hasBox3 = creature->hasSkill("combat_death_watch_sun_3") ||
						   creature->hasSkill("combat_death_watch_krr_3") ||
						   creature->hasSkill("combat_death_watch_xon_3") ||
						   creature->hasSkill("combat_death_watch_zan_3");

			if (!hasBox3) {
				creature->sendSystemMessage("\\#FF6666Command not available yet.");
				return GENERALERROR;
			}
		}

		// Call Lua handler for faction-specific ability execution
		Lua* lua = DirectorManager::instance()->getLuaInstance();
		Reference<LuaFunction*> abilityFunc = lua->createFunction("dw_faction_ability", "executeCommand", 0);

		*abilityFunc << creature;
		*abilityFunc << args;

		abilityFunc->callFunction();

		return SUCCESS;
	}

};

#endif //DEATHWATCHCOMMAND_H_
