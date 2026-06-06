/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#include "QueueCommand.h"

//Infinity: Custom includes
#include "server/zone/managers/collision/CollisionManager.h"
#include "server/zone/managers/planet/PlanetManager.h"
#include "server/zone/objects/building/BuildingObject.h"
#include "server/zone/objects/player/PlayerObject.h"

#ifndef UNSTICKCOMMAND_H_
#define UNSTICKCOMMAND_H_

class UnstickCommand : public QueueCommand {
public:

	UnstickCommand(const String& name, ZoneProcessServer* server)
		: QueueCommand(name, server) {

	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {

		if (creature == nullptr)
			return GENERALERROR;

		if (!checkStateMask(creature))
			return INVALIDSTATE;

		if (!checkInvalidLocomotions(creature))
			return INVALIDLOCOMOTION;

		if (!creature->checkCooldownRecovery("unstick")) {

			const Time* cooldownTime = creature->getCooldownTime("unstick");
			if (cooldownTime != nullptr) {
				float timeLeft = fabs(cooldownTime->miliDifference()) / 1000.f /60.f;
				creature->sendSystemMessage("You can use the /unstick command again in " + String::format("%.1f", timeLeft) + " minute" + ((timeLeft == 1.0f) ? "." : "s."));
				return GENERALERROR;
			} 
			else {
				creature->sendSystemMessage("You can only use the /unstick command once every 5 minutes.");
                return GENERALERROR;
			}
		}

        auto zone = creature->getZone();

	    if (zone == nullptr) {
            creature->sendSystemMessage("The /unstick command could not find player zone.  Please create a support ticket for assistance.");
			return GENERALERROR;
		}

        uint32 cellID = 0;
        Vector3 worldPosition = creature->getWorldPosition();
        float x = worldPosition.getX();
        float y = worldPosition.getY();
        float z = worldPosition.getZ();

        auto parent = creature->getParent().get();

        if (parent != nullptr) {

            if (parent->isCellObject()) {
                cellID = parent->getObjectID();
            }
        }

        if (cellID == 0) { // Outside

            auto planetManager = zone->getPlanetManager();

            if (planetManager != nullptr) {
                auto terrainManager = planetManager->getTerrainManager();

                if (terrainManager != nullptr) {

                    bool found = false;

                    float randomX = x;
                    float randomY = y;

                    for (int i = 0; i < 20; i++) {

                        randomX = x + System::random(10) - 5;
                        randomY = y + System::random(10) - 5;
                        float collisionZ = CollisionManager::getWorldFloorCollision(randomX, randomY, zone, false);
                        float terrainZ = terrainManager->getHeight(randomX, randomY);

                        /*
                        StringBuffer msg;
                        msg << "X:" << x << "  Y:" << y << "  Z:" << z << "  ranX:" << randomX << "  ranY:" << randomY << "  collisionZ:" << collisionZ << "  terrainZ:" << terrainZ << "  deltaZ:" << fabs(collisionZ - z);
                        creature->sendSystemMessage(msg.toString());
                        */

                        if (collisionZ - z < 3) {  //Can't go up more than 3 meters, can go down farther if stuck on roof
                            x = randomX;
                            y = randomY;
                            z = collisionZ;
                            found = true;
                            break;
                        }
                    }

                    if (!found) {
                        creature->sendSystemMessage("Could not find a new location Z coordinate due to collisions.  Attempting teleport to current Z coordinate to unstick.");
                        x = randomX;
                        y = randomY;
                    }

                }
                else {
                    creature->sendSystemMessage("The /unstick command could not find a new position.  Please create a support ticket for assistance.");
                    return GENERALERROR;
                }
            }           
        }
        else { //Inside — Infinity #1620: route to the building's ejection point so
               // players stuck in PB elevator rooms or other cell pockets can recover
               // without needing CSR intervention. Guard against PvP / PvE escape
               // abuse with combat + static-building checks. We deliberately do NOT
               // teleport within the building (that would enable internal geometry
               // hacks); ejectObject sends the player to the building's pre-set
               // exterior ejection point (front step), reusing the same path
               // /eject uses for roof-stuck players. Player must walk back in
               // normally and re-enter through the front door.
            ManagedReference<SceneObject*> cellParent = parent->getParent().get();
            if (cellParent == nullptr || !cellParent->isBuildingObject()) {
                creature->sendSystemMessage("The /unstick command could not resolve your building.  Please create a support ticket for assistance.");
                return GENERALERROR;
            }

            BuildingObject* building = cast<BuildingObject*>(cellParent.get());
            if (building == nullptr) {
                creature->sendSystemMessage("The /unstick command could not resolve your building.  Please create a support ticket for assistance.");
                return GENERALERROR;
            }

            // Guard #1: refuse if the building is static world data (dungeon, POI,
            // themepark, NPC structure, world cantina, etc.). Closes the
            // "caught fighting deep in a dungeon, /unstick to escape" vector.
            if (building->isStaticBuilding()) {
                creature->sendSystemMessage("The /unstick command cannot be used inside a structure.  Please create a support ticket for assistance.");
                return GENERALERROR;
            }

            // Guard #2: refuse if the building is a GCW / faction base. These
            // are player-owned but are PvP-relevant — ejecting from one during
            // a defense or assault would be an exploit. Even the owner of a
            // faction base is refused here.
            if (building->isGCWBase()) {
                creature->sendSystemMessage("The /unstick command cannot be used inside a faction base.  Please create a support ticket for assistance.");
                return GENERALERROR;
            }

            // Guard #3: confirm this is actually a player-owned structure. A
            // non-static building should have an ownerObjectID set; if it's 0,
            // something unusual is going on (orphaned structure, mid-transfer,
            // edge-case world data) — refuse rather than guess.
            if (building->getOwnerObjectID() == 0) {
                creature->sendSystemMessage("The /unstick command cannot be used inside this structure.  Please create a support ticket for assistance.");
                return GENERALERROR;
            }

            // Guard #4: refuse if the player is in combat or has any active
            // engagement flag. This is the load-bearing PvP/BH escape guard.
            // It applies regardless of which player-built building they're in
            // (own house, friend's house, vendor stall, guild hall, etc.).
            PlayerObject* ghost = creature->getPlayerObject();
            bool inCombat = creature->isInCombat();
            bool hasEngagementFlag = false;
            if (ghost != nullptr) {
                hasEngagementFlag = ghost->hasPvpTef() || ghost->hasBhTef() || ghost->hasCrackdownTef();
            }
            if (inCombat || hasEngagementFlag) {
                creature->sendSystemMessage("The /unstick command cannot be used while in combat or with an active TEF.  Please create a support ticket for assistance.");
                return GENERALERROR;
            }

            building->ejectObject(creature);

            creature->addCooldown("unstick", 15 * 60 * 1000); // Wait 15 mins before they can try again
            creature->info("Player " + creature->getFirstName() + " used /unstick from inside building OID=" + String::valueOf(building->getObjectID()) + " — ejected to exterior", true);
            creature->sendSystemMessage("@cmd_err:unstick_request_complete"); // Unstick complete
            return SUCCESS;
        }

        creature->teleport(x, z, y, cellID);

		creature->addCooldown("unstick", 15 * 60 * 1000); // Wait 15 mins before they can try again

		creature->info("Player " + creature->getFirstName() + " used /unstick " + arguments.toString(), true);

		creature->sendSystemMessage("@cmd_err:unstick_request_complete"); // Unstick complete

		    /*
            string/en/cmd_err.stf	7	unstick_in_progress	Unstick in progress
            string/en/cmd_err.stf	8	unstick_request_complete	Unstick complete
            string/en/cmd_err.stf	9	unstick_request_cancelled	Unstick request was cancelled
		    */

		return SUCCESS;
	}

};

#endif //UNSTICKCOMMAND_H_

