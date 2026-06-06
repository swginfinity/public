/*
 * UnlearnSchematicCommand.h
 *
 * Infinity custom GM command to remove a learned draft schematic from the targeted player
 * (or self if no target).  Useful for testing schematic iteration — the client caches
 * schematic metadata on learn, so re-testing after server-side schematic/ACM changes
 * requires unlearning + re-learning the schematic to pick up the fresh state.
 *
 * Syntax:
 *   /unlearnschematic <draft_schematic_iff_path>
 *
 * Example:
 *   /unlearnschematic object/draft_schematic/clothing/clothing_hat_m_hood.iff
 */

#ifndef UNLEARNSCHEMATICCOMMAND_H_
#define UNLEARNSCHEMATICCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/managers/crafting/schematicmap/SchematicMap.h"
#include "server/zone/objects/draftschematic/DraftSchematic.h"

class UnlearnSchematicCommand : public QueueCommand {
public:

	UnlearnSchematicCommand(const String& name, ZoneProcessServer* server) : QueueCommand(name, server) {
	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {
		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		StringTokenizer args(arguments.toString());

		if (!args.hasMoreTokens()) {
			creature->sendSystemMessage("Syntax: /unlearnschematic <draft_schematic_iff_path>");
			creature->sendSystemMessage("Example: /unlearnschematic object/draft_schematic/clothing/clothing_hat_m_hood.iff");
			return INVALIDTARGET;
		}

		String schematicPath;
		args.getStringToken(schematicPath);

		// Resolve the target player — default to self, or the targeted player if one is selected
		ManagedReference<CreatureObject*> targetPlayer = creature;

		auto zoneServer = server->getZoneServer();
		if (zoneServer != nullptr && target != 0) {
			ManagedReference<SceneObject*> object = zoneServer->getObject(target);
			if (object != nullptr && object->isPlayerCreature()) {
				targetPlayer = cast<CreatureObject*>(object.get());
			}
		}

		ManagedReference<PlayerObject*> ghost = targetPlayer->getPlayerObject();
		if (ghost == nullptr) {
			creature->sendSystemMessage("Target is not a player.");
			return GENERALERROR;
		}

		DraftSchematic* schematic = SchematicMap::instance()->get(schematicPath.hashCode());
		if (schematic == nullptr) {
			creature->sendSystemMessage("Unknown schematic: " + schematicPath);
			return GENERALERROR;
		}

		Locker locker(targetPlayer);
		ghost->removeRewardedSchematic(schematic, true);

		creature->sendSystemMessage("Removed schematic \"" + schematicPath + "\" from " + targetPlayer->getFirstName() + ".");
		return SUCCESS;
	}
};

#endif //UNLEARNSCHEMATICCOMMAND_H_
