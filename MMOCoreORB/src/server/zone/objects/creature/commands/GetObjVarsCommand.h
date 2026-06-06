/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef GETOBJVARSCOMMAND_H_
#define GETOBJVARSCOMMAND_H_

#include "server/zone/objects/scene/SceneObject.h"
#include "server/chat/ChatManager.h"
#include "server/zone/objects/group/GroupObject.h"
#include "server/zone/objects/player/sui/messagebox/SuiMessageBox.h"
#include "templates/params/creature/ObjectFlag.h"
// Infinity: Custom includes
#include "server/zone/managers/stringid/StringIdManager.h"
#include "server/zone/managers/faction/FactionManager.h"
#include "server/zone/objects/tangible/weapon/WeaponObject.h"
#include "server/zone/objects/tangible/TangibleObject.h"
#include "server/zone/objects/tangible/Container.h"
#include "server/zone/objects/intangible/PetControlDevice.h"

class GetObjVarsCommand : public QueueCommand {
public:
	GetObjVarsCommand(const String& name, ZoneProcessServer* server) : QueueCommand(name, server) {
	}

	int doQueueCommand(CreatureObject* creature, const uint64& target, const UnicodeString& arguments) const {
		if (!checkStateMask(creature)) {
			return INVALIDSTATE;
		}

		if (!checkInvalidLocomotions(creature)) {
			return INVALIDLOCOMOTION;
		}

		if (!creature->isPlayerCreature()) {
			return GENERALERROR;
		}

		auto ghost = creature->getPlayerObject();

		if (ghost == nullptr) {
			return GENERALERROR;
		}

		uint64 objectID = 0;
		UnicodeTokenizer tokenizer(arguments);
		tokenizer.setDelimeter(" ");

		// if we have an argument passed, use it
		if (tokenizer.hasMoreTokens()) {
			try {
				objectID = tokenizer.getLongToken();
			} catch (Exception& err) {
				creature->sendSystemMessage("INVALID OBJECT.  Please specify a valid object name or objectid");
				return INVALIDPARAMETERS;
			}
		} else {
			objectID = target;
		}

		if (objectID == 0) {
			creature->sendSystemMessage("You need to target an object or specify an object id: /getobjvars <objectID> ");
		}

		auto zoneServer = server->getZoneServer();

		if (zoneServer == nullptr) {
			return GENERALERROR;
		}

		ManagedReference<SceneObject*> object = zoneServer->getObject(objectID, false);

		if (object == nullptr) {
			creature->sendSystemMessage("ERROR GETTING OBJECT - nullptr " + String::valueOf(objectID));
			return INVALIDTARGET;
		}

		ManagedReference<SuiMessageBox*> box = new SuiMessageBox(creature, SuiWindowType::NONE);

		if (box == nullptr) {
			return GENERALERROR;
		}

		/*
		*	General Object Information
		*/

		String nameString = object->getObjectNameStringIdName();
		String strDescription = object->getDetailedDescription();
		bool bMarkedForDelete = object->_isMarkedForDeletion();

		bool bIsUpdated = object->_isUpdated();
		int rCount = object.get()->getReferenceCount();
		auto containmentType = object->getContainmentType();

		auto parent = object->getParent().get();
		auto rootParent = object->getRootParent();

		uint32 covSize = 0;
		auto closeObjectsVector = object->getCloseObjects();

		if (closeObjectsVector != nullptr) {
			covSize = closeObjectsVector->size();
		}

		StringBuffer msg;
		msg << endl << "General Object Information:" << endl << endl <<
		"Object ID: " << objectID << endl << endl <<
		"Class Name: " << object->_getClassName() << endl <<
		"Child Objects Size: " << object->getChildObjects()->size() << endl <<
		"Close Objects Vector COV Size: " << covSize << endl <<
		"ContainmentType: " << containmentType << endl;

		if (parent != nullptr) {
			msg << "Parent: " << parent->getDisplayedName() << " ID: " << parent->getObjectID() << " Parent Class: " << parent->_getClassName() << endl;
		} else {
			msg << "Parent: null" << endl;
		}

		if (rootParent != nullptr) {
			msg << "Root Parent: " << rootParent->getDisplayedName() << " ID: " << rootParent->getObjectID() << " Root Parent Class: " << rootParent->_getClassName() << endl;
		} else {
			msg << "Root Parent: null" << endl;
		}

		msg << endl << // Spacer

		"Object Type: " << object->getGameObjectType() << endl <<
		"Object Name String: " << nameString << endl <<
		"Template Path: " << object->getObjectTemplate()->getFullTemplateString() << endl <<

		endl << // Spacer

		"Reference Count: " << rCount << endl <<
		"Marked for deletion: " << bMarkedForDelete << endl <<
		"IsUpdated: " << bIsUpdated << endl << endl <<
		"TreeNode is null: " << (object->getNode() == nullptr ? "true" : "false") << endl;

		auto zone = object->getZone();
		msg << "Zone: " << (zone != nullptr ? zone->getZoneName() : "nullptr") << endl;

		// Infinity: Faction info for tangible objects
		if (object->isTangibleObject()) {
			auto tano = object->asTangibleObject();
			if (tano != nullptr) {
				unsigned faction = tano->getFaction();
				String factionString = "NEUTRAL";
				if (faction == Factions::FACTIONIMPERIAL) {
					factionString = "IMPERIAL";
				} else if (faction == Factions::FACTIONREBEL) {
					factionString = "REBEL";
				}

				int factionStatus = tano->getFactionStatus();
				String statusString = "N/A";
				if (factionStatus == FactionStatus::ONLEAVE) {
					statusString = "ONLEAVE";
				} else if (factionStatus == FactionStatus::COVERT) {
					statusString = "COVERT";
				} else if (factionStatus == FactionStatus::OVERT) {
					statusString = "OVERT";
				}
				msg << "Faction: " << factionString << "  Faction Status: " << statusString << endl;
			}
		}

		// Infinity: PetControlDevice — show linked creature OID
		if (object->_getClassName() == "PetControlDevice") {
			auto pcd = dynamic_cast<PetControlDevice*>(object.get());

			if (pcd != nullptr) {
				auto controlledObj = pcd->getControlledObject();

				msg << endl << "Pet Control Device Information:" << endl;

				if (controlledObj != nullptr) {
					msg << "Linked Creature OID: " << controlledObj->getObjectID() << endl;
					msg << "Linked Creature Name: " << controlledObj->getDisplayedName() << endl;
					msg << "Pet Type: " << pcd->getPetType() << endl;
				} else {
					msg << "Linked Creature: nullptr (MISSING)" << endl;
				}
			}
		}

		// Infinity: Container contents
		if (object->isContainerObject()) {
			auto container = dynamic_cast<Container*>(object.get());

			if (container != nullptr) {
				msg << "Number of Objects in Container: " << container->getContainerObjectsSize() << endl;

				for (int i = 0; i < container->getContainerObjectsSize(); ++i) {
					auto child = container->getContainerObject(i, "GetObjVars:container");

					if (child != nullptr) {
						msg << "  Child: " << i + 1 << "   ObjectID: " << child->getObjectID() << "   Name: " << child->getDisplayedName() << endl;
					}
				}
			}
		}

		if (object->isCreatureObject()) {
			auto creoObject = object->asCreatureObject();

			if (creoObject != nullptr) {
				/*
				*	CreatureObject Information
				*/

				msg << endl << "Creature Object Information:" << endl << endl <<
				"Displayed Name: " << creoObject->getDisplayedName() << endl;

				if (!creoObject->isVehicleObject()) {
					msg << "Creature First Name: " << creoObject->getFirstName() << endl;
				}

				// Infinity: Pet/vehicle owner info
				if (creoObject->isPet() || creoObject->isVehicleObject()) {
					auto pcd = creoObject->getControlDevice().get().castTo<PetControlDevice*>();
					if (pcd != nullptr) {
						auto owner = pcd->getParentRecursively(SceneObjectType::PLAYERCREATURE).castTo<CreatureObject*>();
						msg << "Owner: " << (owner != nullptr ? owner->getFirstName() : "*Not Found*") << endl;
					}
				}

				msg << "PvP Status Bitmask: " << creoObject->getPvpStatusBitmask() << endl <<
				"Options Bitmask: " << creoObject->getOptionsBitmask() << endl;

				if (creoObject->isPlayerCreature()) {
					auto playerManager = server->getPlayerManager();

					if (playerManager != nullptr) {
						int playerLevel = playerManager->calculatePlayerLevel(creoObject);
						msg << "Player Level: " << playerLevel << endl;
					}

					auto tarGhost = creoObject->getPlayerObject();

					if (tarGhost != nullptr) {
						int pilotSquadron = tarGhost->getPilotSquadron();
						int pilotTier = tarGhost->getPilotTier();
						String pilotFaction = FactionManager::instance()->getSpaceFactionBySquadron(pilotSquadron, pilotTier);

						msg <<
						"Pilot Tier: " << pilotTier << endl <<
						"Pilot Squadron: " << pilotSquadron << " (Defined in PlayerManager.idl)" << endl <<
						"Pilot Faction: " << pilotFaction << endl;
					}
				} else {
					// Infinity: Creature level for non-players
					int creatureLevel = creoObject->getLevel();
					msg << "Creature Level: " << creatureLevel << endl;
				}

				if (creoObject->isGrouped()) {
					GroupObject* group = creoObject->getGroup();

					if (group != nullptr) {
						msg << "Group Level: " << group->getGroupLevel() << endl;
					}
				}

				msg << endl;

				/*
				*	AiAgentObject Information
				*/
				if (creoObject->isAiAgent()) {
					auto objectAgent = creoObject->asAiAgent();

					if (objectAgent != nullptr) {
						msg << endl << "AiAgent Object Information:" << endl << endl;

						String aiEnabled = ((objectAgent->getOptionsBitmask() & OptionBitmask::AIENABLED) ? "True" : "False");
						msg << "AI Enabled: " << aiEnabled << endl;

						auto creatureBitmask = objectAgent->getCreatureBitmask();

						msg << "Creature Bitmask: " << creatureBitmask << endl;

						// Display individual ObjectFlags
						msg << "Creature Bitmask Flags:" << endl;
						if (creatureBitmask & ObjectFlag::NPC) { msg << "  - NPC" << endl; }
						if (creatureBitmask & ObjectFlag::PACK) { msg << "  - PACK" << endl; }
						if (creatureBitmask & ObjectFlag::HERD) { msg << "  - HERD" << endl; }
						if (creatureBitmask & ObjectFlag::KILLER) { msg << "  - KILLER" << endl; }
						if (creatureBitmask & ObjectFlag::STALKER) { msg << "  - STALKER" << endl; }
						if (creatureBitmask & ObjectFlag::BABY) { msg << "  - BABY" << endl; }
						if (creatureBitmask & ObjectFlag::LAIR) { msg << "  - LAIR" << endl; }
						if (creatureBitmask & ObjectFlag::HEALER) { msg << "  - HEALER" << endl; }
						if (creatureBitmask & ObjectFlag::SCOUT) { msg << "  - SCOUT" << endl; }
						if (creatureBitmask & ObjectFlag::PET) { msg << "  - PET" << endl; }
						if (creatureBitmask & ObjectFlag::DROID_PET) { msg << "  - DROID_PET" << endl; }
						if (creatureBitmask & ObjectFlag::FACTION_PET) { msg << "  - FACTION_PET" << endl; }
						if (creatureBitmask & ObjectFlag::ESCORT) { msg << "  - ESCORT" << endl; }
						if (creatureBitmask & ObjectFlag::FOLLOW) { msg << "  - FOLLOW" << endl; }
						if (creatureBitmask & ObjectFlag::STATIC) { msg << "  - STATIC" << endl; }
						if (creatureBitmask & ObjectFlag::STATIONARY) { msg << "  - STATIONARY" << endl; }
						if (creatureBitmask & ObjectFlag::NOAIAGGRO) { msg << "  - NOAIAGGRO" << endl; }
						if (creatureBitmask & ObjectFlag::SCANNING_FOR_CONTRABAND) { msg << "  - SCANNING_FOR_CONTRABAND" << endl; }
						if (creatureBitmask & ObjectFlag::IGNORE_FACTION_STANDING) { msg << "  - IGNORE_FACTION_STANDING" << endl; }
						if (creatureBitmask & ObjectFlag::SQUAD) { msg << "  - SQUAD" << endl; }
						if (creatureBitmask & ObjectFlag::EVENTCONTROL) { msg << "  - EVENTCONTROL" << endl; }
						if (creatureBitmask & ObjectFlag::NOINTIMIDATE) { msg << "  - NOINTIMIDATE" << endl; }
						if (creatureBitmask & ObjectFlag::NODOT) { msg << "  - NODOT" << endl; }
						if (creatureBitmask & ObjectFlag::NOBLIND) { msg << "  - NOBLIND" << endl; }
						if (creatureBitmask & ObjectFlag::NODIZZY) { msg << "  - NODIZZY" << endl; }
						if (creatureBitmask & ObjectFlag::NOKNOCKDOWN) { msg << "  - NOKNOCKDOWN" << endl; }
						if (creatureBitmask & ObjectFlag::NOPOSTUREDOWN) { msg << "  - NOPOSTUREDOWN" << endl; }
						if (creatureBitmask & ObjectFlag::NOSTUN) { msg << "  - NOSTUN" << endl; }
						if (creatureBitmask & ObjectFlag::ADULTONLY) { msg << "  - ADULTONLY" << endl; }
						if (creatureBitmask == ObjectFlag::NONE) { msg << "  - NONE" << endl; }

						// Infinity: MobType display
						int mobType = objectAgent->getMobType();
						msg << "MobType: ";
						switch (mobType) {
							case AiAgent::MOB_HERBIVORE: msg << "MOB_HERBIVORE" << endl; break;
							case AiAgent::MOB_NPC: msg << "MOB_NPC" << endl; break;
							case AiAgent::MOB_DROID: msg << "MOB_DROID" << endl; break;
							case AiAgent::MOB_ANDROID: msg << "MOB_ANDROID" << endl; break;
							case AiAgent::MOB_VEHICLE: msg << "MOB_VEHICLE" << endl; break;
							default: msg << "UNKNOWN" << endl; break;
						}

						// Display movement state with readable name
						unsigned int movementState = objectAgent->getMovementState();
						String movementStateName;

						switch (movementState) {
							case 0: movementStateName = "OBLIVIOUS"; break;
							case 1: movementStateName = "WATCHING"; break;
							case 2: movementStateName = "STALKING"; break;
							case 3: movementStateName = "FOLLOWING"; break;
							case 4: movementStateName = "PATROLLING"; break;
							case 5: movementStateName = "FLEEING"; break;
							case 6: movementStateName = "LEASHING"; break;
							case 7: movementStateName = "EVADING"; break;
							case 8: movementStateName = "PATHING_HOME"; break;
							case 9: movementStateName = "FOLLOW_FORMATION"; break;
							case 10: movementStateName = "MOVING_TO_HEAL"; break;
							case 11: movementStateName = "NOTIFY_ALLY"; break;
							case 12: movementStateName = "CRACKDOWN_SCANNING"; break;
							case 13: movementStateName = "HARVESTING"; break;
							case 14: movementStateName = "RESTING"; break;
							case 15: movementStateName = "CONVERSING"; break;
							case 16: movementStateName = "LAIR_HEALING"; break;
							default: movementStateName = "UNKNOWN"; break;
						}

						msg << "Creature Movement State: " << movementState << " (" << movementStateName << ")" << endl;
						msg << "Aggro Radius: " << objectAgent->getAggroRadius() << endl;
						msg << "Respawn Timer: " << (int)objectAgent->getRespawnTimer() << " seconds" << endl;

						ManagedReference<SceneObject*> followCopy = objectAgent->getFollowObject();
						StringBuffer hasFollow;

						if (followCopy != nullptr) {
							hasFollow << "True - " << " OID: " << followCopy->getObjectID();
						} else {
							hasFollow << "False";
						}

						msg << "Has Follow Object: " << hasFollow.toString() << endl;
						msg << "Current total Patrol Points: " << objectAgent->getPatrolPointSize() << endl;
						msg << "In Navmesh: " << (objectAgent->isInNavMesh() ? "True" : "False") << endl;
						msg << "Special Percentage: " << objectAgent->getSpecialPercentage() << "%" << endl;

						// Infinity: Lego chance and credit multiplier
						if (!objectAgent->isPet()) {
							msg << "Lego Chance Multiplier: " << String::format("%.1f", objectAgent->getLegoChanceMultiplier()) << endl;
						}

						if (objectAgent->isNonPlayerCreatureObject()) {
							msg << "Credit Multiplier: " << String::format("%.1f", objectAgent->getCreditMultiplier()) << endl;
						}

						msg << endl;

						// Weapon Information
						msg << "Weapon Information" << endl;

						auto currentWeapon = objectAgent->getCurrentWeapon();
						if (currentWeapon != nullptr) {
							msg << "  Current Weapon: " << currentWeapon->getObjectTemplate()->getTemplateFileName() << "  OID: " << currentWeapon->getObjectID() << endl;
							msg << "    Min Damage: " << currentWeapon->getMinDamage() << "  Max Damage: " << currentWeapon->getMaxDamage() << endl;
						}

						auto primaryWeapon = objectAgent->getPrimaryWeapon();
						if (primaryWeapon != nullptr) {
							msg << "  Primary Weapon: " << primaryWeapon->getObjectTemplate()->getTemplateFileName() << "  OID: " << primaryWeapon->getObjectID() << endl;
						}

						auto secondaryWeapon = objectAgent->getSecondaryWeapon();
						if (secondaryWeapon != nullptr) {
							msg << "  Secondary Weapon: " << secondaryWeapon->getObjectTemplate()->getTemplateFileName() << "  OID: " << secondaryWeapon->getObjectID() << endl;
						}

						auto defaultWeapon = objectAgent->getDefaultWeapon();
						if (defaultWeapon != nullptr) {
							msg << "  Default Weapon: " << defaultWeapon->getObjectTemplate()->getTemplateFileName() << "  OID: " << defaultWeapon->getObjectID() << endl;
						}

						msg << "  AI Agent (Template) Min Damage: " << objectAgent->getMinDamage() << "  Max Damage: " << objectAgent->getMaxDamage() << "  Weapon Speed: " << objectAgent->getWeaponSpeed() << endl;

						// Infinity: Attack Maps
						auto attackMap = objectAgent->getPrimaryAttackMap();
						String mapName = "Primary";

						if (attackMap != nullptr && attackMap->size() > 0) {
							msg << mapName << " Weapon Attack Map" << endl;
							for (int i = 0; i < attackMap->size(); i++) {
								String cmd = attackMap->getCommand(i);
								String formattedName = "";
								if (cmd.isEmpty()) {
									formattedName = "none";
								} else {
									String lookup = "@combat_effects:" + cmd;
									formattedName = StringIdManager::instance()->getStringId(lookup.hashCode()).toString();
								}
								msg << "  Attack #" << i + 1 << ":  " << (formattedName != "" ? formattedName : cmd) << endl;
							}
						}

						// NOTE: getSecondaryAttackMap/getDefaultAttackMap removed in JTL upstream

						msg << endl;

						// Home Object - Lairs
						uint64 homeID = 0;
						String homeName = "none";
						ManagedReference<SceneObject*> homeLair = objectAgent->getHomeObject().get();

						if (homeLair != nullptr) {
							homeID = homeLair->getObjectID();
							homeName = homeLair->getObjectNameStringIdName();
						}

						msg << "Home Object: " << homeName << " ID: " << homeID << endl;

						PatrolPoint* homeLocation = objectAgent->getHomeLocation();

						if (homeLocation != nullptr) {
							msg << "Home Location: X:" << homeLocation->getPositionX() << ", Z:" << homeLocation->getPositionZ() << ", Y:" << homeLocation->getPositionY() << endl;
						}

						// Infinity: DNA/milk state for creatures
						if (objectAgent->isCreature()) {
							auto critter = cast<Creature*>(objectAgent);

							if (critter != nullptr) {
								msg << "DNA Sample count: " << critter->getDnaSampleCount() << "  DNA State: " << critter->getDnaState() << "  Milk State: " << critter->getMilkState() << endl;
							}
						}

						// Inventory Contents
						msg << endl << "Inventory" << endl;
						auto inventory = objectAgent->getInventory();

						if (inventory != nullptr) {
							if (inventory->getContainerObjectsSize() > 0) {
								for (int i = 0; i < inventory->getContainerObjectsSize(); ++i) {
									auto invObject = inventory->getContainerObject(i, "GetObjVars:inventory");

									if (invObject != nullptr) {
										msg << "  Item #" << i + 1 << ": " << invObject->getDisplayedName() << " - " << invObject->getObjectTemplate()->getTemplateFileName() << " - OID: " << invObject->getObjectID() << endl;
									}
								}
							} else {
								msg << "  Inventory is empty" << endl;
							}
						} else {
							msg << "  Inventory is nullptr" << endl;
						}

						msg << endl; // Spacing
					}
				}

				// Infinity: Child objects listing
				auto childObjects = creoObject->getChildObjects();

				for (int i = 0; i < childObjects->size(); ++i) {
					auto child = childObjects->get(i);

					if (child != nullptr) {
						msg << "  Child: " << i + 1 << "   ObjectID: " << child->getObjectID() << "   Name: " << child->getDisplayedName() << endl;
					}
				}

				// Movement Modifiers
				msg << "Walk Speed: " << creoObject->getWalkSpeed() << endl;
				msg << "Run Speed: " << creoObject->getRunSpeed() << endl;
				msg << "Current Speed: " << creoObject->getCurrentSpeed() << endl;
				msg << "Speed Multi Base: " << creoObject->getSpeedMultiplierBase() << endl;
				msg << "Speed Multi Mod: " << creoObject->getSpeedMultiplierMod() << endl;
				msg << "Creature Height: " << creoObject->getHeight() << endl;

				msg << "Walk Acceleration: " << creoObject->getWalkAcceleration() << endl;
				msg << "Run Acceleration: " << creoObject->getRunAcceleration() << endl;
				msg << "Acceleration Multi Base: " << creoObject->getAccelerationMultiplierBase() << endl;
				msg << "Acceleration Multi Mod: " << creoObject->getAccelerationMultiplierMod() << endl;

				// Slope Modifiers
				msg << "Slope Angle: " << creoObject->getSlopeModAngle() << endl;
				msg << "Slope Mod Percent: " << creoObject->getSlopeModPercent() << endl;
				msg << "Water Mod Percent: " << creoObject->getWaterModPercent() << endl;
				msg << "Turn Scale: " << creoObject->getTurnScale() << endl;

				msg << endl;

				// List the active areas
				SortedVector<ManagedReference<ActiveArea*>>* areas = creoObject->getActiveAreas();

				if (areas != nullptr) {
					msg << endl << endl << "Current Active Areas:" << endl << endl;

					for (int i = 0; i < areas->size(); i++) {
						ActiveArea* area = areas->get(i);

						if (area == nullptr) {
							continue;
						}

						msg << "Area #" << i << " -- " << area->getAreaName() << endl;
					}
				}
			}
		/*
			ShipObject Information
		*/
		} else if (object->isShipObject()) {
			ShipObject* ship = object->asShipObject();

			if (ship != nullptr) {
				String aiEnabled = (ship->getOptionsBitmask() & OptionBitmask::AIENABLED ? "True" : "False");
				msg << "AI Enabled: " << aiEnabled << endl;
				msg << "PvP Status Bitmask: " << ship->getPvpStatusBitmask() << endl;
				msg << "Options Bitmask: " << ship->getOptionsBitmask() << endl;

				if (ship->isShipAiAgent()) {
					ShipAiAgent* shipAgent = ship->asShipAiAgent();

					if (shipAgent != nullptr) {
						msg << "Ship Agent Movement State: " << shipAgent->getMovementState() << endl;

						ManagedReference<ShipObject*> followCopy = shipAgent->getFollowShipObject();
						StringBuffer hasFollow;

						if (followCopy != nullptr) {
							hasFollow << "True - " << " OID: " << followCopy->getObjectID();
						} else {
							hasFollow << "False";
						}

						msg << "Has Follow Object: " << hasFollow.toString() << endl;

						ManagedReference<ShipObject*> targetCopy = shipAgent->getTargetShipObject();
						StringBuffer hasTarget;

						if (targetCopy != nullptr) {
							hasTarget << "True - " << " OID: " << targetCopy->getObjectID();
						} else {
							hasTarget << "False";
						}

						msg << "Has Target Object: " << hasTarget.toString() << endl;
						msg << "Current total Patrol Points: " << shipAgent->getPatrolPointSize() << endl;
					}
				}
			}
		}

		// Infinity: Weapon object details
		if (object->isWeaponObject()) {
			auto weapon = cast<WeaponObject*>(object.get());
			if (weapon != nullptr) {
				msg << endl << "Weapon Information:" << endl;
				msg << "Weapon Min Damage: " << weapon->getMinDamage() << endl;
				msg << "Weapon Max Damage: " << weapon->getMaxDamage() << endl;
				msg << "Weapon Speed: " << weapon->getAttackSpeed() << endl;
				msg << "Force Cost: " << weapon->getForceCost() << endl;
			}
		}

		if (object->getZone() != nullptr) {
			msg << "Location: " << object->getPosition().toString() << " " << object->getZone()->getZoneName() << endl;
			msg << "Direction Angle - Degrees: " << object->getDirectionAngle() << endl;
		}

		auto city = object->getCityRegion().get();

		if (city != nullptr) {
			msg << "City Region oid: " << city->getObjectID() << ", name: " << city->getRegionDisplayedName() << endl;
		}

		// Send information in a system message
		creature->sendSystemMessage(msg.toString());

		StringBuffer titleStr;
		titleStr << "GetObjVars: " << objectID;

		ChatManager* chatManager = zoneServer->getChatManager();

		if (chatManager != nullptr) {
			// Send information as in game mail
			chatManager->sendMail("System", titleStr.toString(), msg.toString(), creature->getFirstName());
		}

		box->setPromptTitle(titleStr.toString());
		box->setPromptText(msg.toString());

		ghost->addSuiBox(box);

		// Send information via sui box
		creature->sendMessage(box->generateMessage());

		return SUCCESS;
	}
};

#endif // GETOBJVARSCOMMAND_H_
