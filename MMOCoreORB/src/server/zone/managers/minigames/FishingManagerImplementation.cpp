/*
 *	FishingManagerImplementation.cpp
 *
 *	Created on: Apr 11, 2010
 *	Refactor on: Aug. 3, 2021 - By: Hakry
 *	Author: swgemu
 */

#include "server/zone/managers/minigames/FishingManager.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/objects/player/PlayerObject.h"
#include "server/zone/objects/player/sui/listbox/SuiListBox.h"
#include "server/zone/objects/tangible/fishing/FishingPoleObject.h"
#include "server/zone/objects/tangible/fishing/FishingBaitObject.h"
#include "server/zone/objects/tangible/fishing/FishObject.h"
#include "server/zone/objects/resource/ResourceContainer.h"
#include "terrain/manager/TerrainManager.h"
#include "server/zone/managers/resource/ResourceManager.h"
#include "server/zone/Zone.h"
#include "server/zone/managers/planet/PlanetManager.h"
#include "server/zone/managers/player/PlayerManager.h"
#include "server/zone/ZoneServer.h"
#include "server/chat/StringIdChatParameter.h"
#include "server/zone/managers/minigames/events/FishingEvent.h"
#include "server/zone/managers/minigames/events/FishingSplashEvent.h"
#include "server/zone/managers/minigames/FishingSession.h"
#include "server/zone/managers/loot/LootManager.h"
#include "system/util/VectorMap.h"

// #define DEBUG_FISHING

// Infinity: Named constants for tuning values (extracted from magic numbers)
namespace FishingConstants {
	// Distance
	static constexpr float MIN_CAST_DISTANCE = 2.1f;
	static constexpr float MAX_CAST_DISTANCE = 10.0f;
	static constexpr float CATCH_RANGE = 2.0f;
	static constexpr float MAX_LINE_RANGE = 20.0f;

	// Timers (ms)
	static constexpr int NORMAL_TIMER_MS = 5000;
	static constexpr int FAST_TIMER_MIN_MS = 2500;
	static constexpr int FAST_TIMER_RANGE_MS = 1000;
	static constexpr int SPLASH_DURATION_MS = 1000;

	// Probability / thresholds
	static constexpr int CHANCE_ROLL_MAX = 30;
	static constexpr int BAIT_TOSS_THRESHOLD = 15;
	static constexpr int NIBBLE_DOWNGRADE_THRESHOLD = 10;
	static constexpr int BITE_DENSITY_OFFSET = 10;
	static constexpr int LOST_BAIT_THRESHOLD = 3;
	static constexpr int REEL_MISHAP_OFFSET = 5;
	static constexpr int BAIT_DEGRADE_PERCENT = 5;
	static constexpr int BAD_LUCK_PERCENT = 5;
	static constexpr int FISH_VS_ITEM_THRESHOLD = 85;
	static constexpr int RARE_LOOT_ROLL_MAX = 200;
	static constexpr int RARE_LOOT_THRESHOLD = 190;
	static constexpr int FISH_TYPE_COUNT = 7;

	// Marker movement rates
	static constexpr float MARKER_APPROACH_RATE = 0.75f;
	static constexpr float FISH_CHARGE_RATE = 0.65f;

	// Resource/reward
	static constexpr int RESOURCE_HARVEST_MULTIPLIER = 50;
	static constexpr int XP_BASE = 90;
}

void FishingManagerImplementation::initialize() {
	info("Loading configuration.");

	if (!loadConfigData() && fishingEnabled) {
		info("Failed to load configuration lua.");
	}
}

bool FishingManagerImplementation::loadConfigData() {
	Lua* lua = new Lua();
	lua->init();

	if (!lua->runFile("scripts/managers/fishing_manager.lua")) {
		delete lua;
		return false;
	}

	bool forceLog = true;

	fishingEnabled = lua->getGlobalBoolean("fishingEnabled");

	if (!fishingEnabled) {
		info("Fishing is disabled.");
		return false;
	}

	LuaObject fishTypes = lua->getGlobalObject("fishTypes");

	if (fishTypes.isValidTable()) {
		for (int i = 1; i <= fishTypes.getTableSize(); ++i) {
			String fishName = fishTypes.getStringAt(i);
			fishType.add(fishName);
		}
	}
	fishTypes.pop();

	info("Loaded a total of " + String::valueOf(fishType.size()) + " Fish Species.", forceLog);

	LuaObject colors = lua->getGlobalObject("fishColors");

	if (colors.isValidTable()) {
		for (int i = 1; i <= colors.getTableSize(); ++i) {
			LuaObject colorsByPlanet = colors.getObjectAt(i);

			if (colorsByPlanet.isValidTable()) {
				String planet = colorsByPlanet.getStringAt(1);
				int colorInt = colorsByPlanet.getIntAt(2);

				fishColors.put(planet, colorInt);
			}
			colorsByPlanet.pop();
		}

		fishColors.setNullValue(1); // Infinity:  Set the nullvalue for planet not found to 1
	}
	colors.pop();

	info("Loaded Fish Colors for " + String::valueOf(fishColors.size()) + " Planets.", forceLog);

	LuaObject lengths = lua->getGlobalObject("fishLengths");

	if (lengths.isValidTable()) {
		for (int i = 1; i <= lengths.getTableSize(); ++i) {
			LuaObject lengthsByFish = lengths.getObjectAt(i);

			if (lengthsByFish.isValidTable()) {
				String fishSpecies = lengthsByFish.getStringAt(1);
				float length = lengthsByFish.getFloatAt(2);
				fishLengths.put(fishSpecies, length);
			}
			lengthsByFish.pop();
		}
	}
	lengths.pop();

	info("Loaded " + String::valueOf(fishLengths.size()) + " fish lengths.", forceLog);

	LuaObject actions = lua->getGlobalObject("fishingActions");

	if (actions.isValidTable()) {
		for (int i = 1; i <= actions.getTableSize(); ++i) {
			String action = actions.getStringAt(i);

			fishingActions.add(action);
		}
	}
	actions.pop();

	info("Loaded " + String::valueOf(fishingActions.size()) + " Fishing Actions.", forceLog);

	LuaObject states = lua->getGlobalObject("fishingStates");

	if (states.isValidTable()) {
		for (int i = 1; i <= states.getTableSize(); ++i) {
			String state = states.getStringAt(i);

			fishingStates.add(state);
		}
	}
	states.pop();

	info("Loaded " + String::valueOf(fishingStates.size()) + " Fishing States.", forceLog);

	LuaObject baitStat = lua->getGlobalObject("baitStatus");

	if (baitStat.isValidTable()) {
		for (int i = 1; i <= baitStat.getTableSize(); ++i) {
			String status = baitStat.getStringAt(i);

			baitStatus.add(status);
		}
	}
	baitStat.pop();

	info("Loaded " + String::valueOf(baitStatus.size()) + " Bait Status Types.", forceLog);

	delete lua;
	lua = nullptr;

	return true;
}

// Infinity: Session helper — eliminates 16 copy-paste getActiveSession patterns
static ManagedReference<FishingSession*> getSession(CreatureObject* player) {
	if (player == nullptr)
		return nullptr;

	return player->getActiveSession(SessionFacadeType::FISHING).castTo<FishingSession*>();
}

int FishingManagerImplementation::startFishing(CreatureObject* player) {
	if (!fishingEnabled || player == nullptr) {
		return NOFISHING;
	}

	ManagedReference<FishingPoleObject*> pole = getPole(player);

	if (pole == nullptr) {
		player->sendSystemMessage("@fishing:must_have_pole");
		return NOFISHING;
	}

	ManagedReference<FishingBaitObject*> bait = getBait(player);

	if (bait == nullptr) {
		player->sendSystemMessage("@fishing:bait_your_pole");
		return NOFISHING;
	}

	auto zone = player->getZone();

	if (zone == nullptr) {
		return NOFISHING;
	}

	if (player->isSwimming()) {
		player->sendSystemMessage("@fishing:player_swimming");
		return NOFISHING;
	}

	Vector3 location;
	int locationCheck = checkLocation(player, pole->getQuality(), location);

	switch(locationCheck) {
		case NOFISHING:
			return locationCheck; // Player was nullptr
		case BADCAST:
			player->sendSystemMessage("@fishing:too_far"); // Your cast goes astray and lands at a non-fishable spot.
			return locationCheck;
		case NOWATER:
			player->sendSystemMessage("@fishing:invalid_location");
			return locationCheck;
		default:
			break;
	}

	ManagedReference<SceneObject*> markerObject = createMarker(location, zone);

	if (markerObject == nullptr) {
		return NOFISHING;
	}

	String moodString = player->getMoodString();

	createFishingSession(player, createFishingEvent(player, WAITING), markerObject, DONOTHING, 0, 0, WAITING, moodString);

	ManagedReference<FishingSession*> fishingSession = getSession(player);

	if (fishingSession == nullptr) {
		return NOFISHING;
	}

	uint32 boxID = createWindow(player, 0);

	fishingSession->setFishBoxID(boxID);

	player->doAnimation("fishing_cast");
	player->setMoodString("fishing");
	player->sendSystemMessage("@fishing:start_fishing");

	player->registerObserver(ObserverEventType::POSITIONCHANGED, _this.getReferenceUnsafeStaticCast());
	player->registerObserver(ObserverEventType::PLAYERKILLED, _this.getReferenceUnsafeStaticCast());
	player->registerObserver(ObserverEventType::STARTCOMBAT, _this.getReferenceUnsafeStaticCast());
	player->registerObserver(ObserverEventType::LOGGEDOUT, _this.getReferenceUnsafeStaticCast());

	createSplash(location.getX(), location.getY(), location.getZ(), zone, player);

	return STARTFISHING;
}

int FishingManagerImplementation::checkLocation(CreatureObject* player, int quality, Vector3& location) {
	if (player == nullptr) {
		return NOFISHING;
	}

	auto zone = player->getZone();

	if (zone == nullptr || !zone->isGroundZone()) {
		return NOFISHING;
	}

	auto planetManager = zone->getPlanetManager();

	if (planetManager == nullptr) {
		return NOFISHING;
	}

	auto terrainManager = planetManager->getTerrainManager();

	if (terrainManager == nullptr) {
		return NOFISHING;
	}

	float angle = player->getDirectionAngle();

	if (angle > 360) {
		angle = angle - 360;
	}

	float randVal = (float)System::random(2);
	float distance = Math::min((Math::max(fabs(1.0 - (quality / 10.f)), (double)FishingConstants::MIN_CAST_DISTANCE) + randVal), (double)FishingConstants::MAX_CAST_DISTANCE);

	angle = 2 * M_PI * angle / 360;

	location = player->getWorldPosition();

	float x = location.getX() + sin(angle) * (distance);
	float y = location.getY() + cos(angle) * (distance);
	float z = location.getZ();

	location.setX(x);
	location.setY(y);

	if (!terrainManager->getWaterHeight(x, y, z)) {
		return BADCAST;
	}

	location.setZ(z);

	if (z < zone->getHeight(x, y)) {
		return NOWATER;
	}

	return STARTFISHING;
}

void FishingManagerImplementation::createFishingSession(CreatureObject* player, FishingEvent* event, SceneObject* marker, int nextAction, int fish, uint32 boxID, int fishingState, String& mood) {
	if (player == nullptr || event == nullptr || marker == nullptr) {
		return;
	}

	// Fishing Session is created and dropped in stopFishing or notifyCloseContainer
	player->addActiveSession(SessionFacadeType::FISHING, new FishingSession(event, marker, nextAction, fish, boxID, fishingState, mood));
}

void FishingManagerImplementation::stopFishing(CreatureObject* player, uint32 boxID, bool destroyMarker) {
	if (player == nullptr) {
		return;
	}

#ifdef DEBUG_FISHING
	player->info(true) << "FishingManagerImplementation::stopFishing -- called for " << player->getDisplayedName();
#endif

	// Remove the observers from the player
	player->dropObserver(ObserverEventType::POSITIONCHANGED, _this.getReferenceUnsafeStaticCast());
	player->dropObserver(ObserverEventType::PLAYERKILLED, _this.getReferenceUnsafeStaticCast());
	player->dropObserver(ObserverEventType::STARTCOMBAT, _this.getReferenceUnsafeStaticCast());
	player->dropObserver(ObserverEventType::LOGGEDOUT, _this.getReferenceUnsafeStaticCast());

	// Stop the fishing Event
	stopFishingEvent(player);

	// Get the session
	ManagedReference<FishingSession*> fishingSession = getSession(player);
	ManagedReference<SceneObject*> marker = nullptr;

	String moodString = "none";

	if (fishingSession != nullptr) {
		// Get the store mood string
		moodString = fishingSession->getMoodString();

		// Clear the session fishing state
		fishingSession->setFishingState(NOTFISHING);

		fishingSession->setEvent(nullptr);

		marker = fishingSession->getMarker();
	}

	// Clear the fishing mood off of the player
	player->setMoodString(moodString);

	player->sendSystemMessage("@fishing:stop_fishing");

	if (!player->isInCombat() && !player->isIncapacitated() && !player->isDead()) {
		player->doAnimation("fishing_reel");
	}

	uint32 id = boxID;

	if (boxID == 0) {
		id = getFishBoxID(player);
	}

	// We can now drop the session off the player
	player->dropActiveSession(SessionFacadeType::FISHING);

	// Fishing Window
	closeMenu(player, id);

	// The only instance when the marker is not automatically destroyed is when there is loot in it the player must retrieve
	if (destroyMarker && marker != nullptr) {
		Locker locker(marker, player);
		removeMarker(player, marker);
	}
}

// Player is pre-locked upon function call
void FishingManagerImplementation::continueFishing(CreatureObject* player) {
	if (player == nullptr) {
		return;
	}

#ifdef DEBUG_FISHING
	player->info(true) << "FishingManagerImplementation::continueFishing -- called for " << player->getDisplayedName();
#endif

	ManagedReference<FishingSession*> fishingSession = getSession(player);

	if (fishingSession == nullptr) {
		return;
	}

	uint32 boxID = fishingSession->getFishBoxID();

	// Player needs a pole to continue fishing
	ManagedReference<FishingPoleObject*> pole = getPole(player);

	if (pole == nullptr) {
		stopFishing(player, boxID, true);
		return;
	}

	// Make sure the player is alive and notincapacitated or in combat
	if (player->isInCombat() || player->isDead() || player->isIncapacitated()) {
		stopFishing(player, boxID, true);
		return;
	}

	// Get the fishing marker
	ManagedReference<SceneObject*> marker = fishingSession->getMarker();

	int nextAction = fishingSession->getNextAction();
	int fish = fishingSession->getFish();
	int state = fishingSession->getFishingState();

	String moodString = fishingSession->getMoodString();

	// Close Current Window
	if (state != REELGAME) {
		closeMenu(player, boxID);
	}

	// Random Event & Mishap
	int chance = System::random(FishingConstants::CHANCE_ROLL_MAX);
	int vegStat = vegetation(marker);
	int fishDensity = density(marker);
	int poleMod = 0;

	if (pole->getQuality() != 0) {
		poleMod = (int)ceil((float)pole->getQuality() / 10);
	}

	// Accounting for bad Bait
	ManagedReference<FishingBaitObject*> bait = getBait(player);

	if (bait != nullptr) {
		int baitState = bait->getFreshness();

		if (state < CATCH && baitState > SOGGY && System::random(FishingConstants::BAIT_TOSS_THRESHOLD) < baitState) {
			mishapEvent("@fishing:toss_bait", player, marker, boxID, true, moodString);
			return; // NO BAIT
		}
	}

#ifdef DEBUG_FISHING
	player->info(true) << "State: " << state << " Chance: " << chance << " Mishap: " << String::valueOf(MISHAP) << " Next Action: " << nextAction << " Pole Mod: " << poleMod << " Vegetation: " << vegStat;
#endif

	// Play Fishing Animation
	animate(player, nextAction);

	// createFishingEvent(player, state);  //Infinity:  Reschedule the task since we moved from repeatable task of 7000 ms

	switch (state) {
		case WAITING: {
			int newFish = getFish(player, marker);

			if (chance + poleMod <= MISHAP + vegStat) { // Pole decreases chance of Snag
				fishingProceed(player, nextAction, marker, newFish, boxID, SNAGGED, false, moodString);
			} else {
				if (chance - poleMod <= fishDensity * 1.5) { // Nibble
					if (nextAction >= TUGUP && nextAction <= REEL) {
						ManagedReference<SceneObject*> newMarker = updateMarker(player, marker, boxID, false);

						if (newMarker != nullptr) {
							fishingProceed(player, nextAction, newMarker, newFish, boxID, NIBBLE, true, moodString);
						}
					} else {
						fishingProceed(player, nextAction, marker, newFish, boxID, NIBBLE, true, moodString);
					}
				} else { // Waiting
					if (nextAction >= TUGUP && nextAction <= REEL) {
						ManagedReference<SceneObject*> newMarker = updateMarker(player, marker, boxID, false);

						if (newMarker != nullptr) {
							fishingProceed(player, nextAction, newMarker, newFish, boxID, WAITING, false, moodString);
						}
					} else {
						fishingProceed(player, nextAction, marker, newFish, boxID, WAITING, false, moodString);
					}
				}
			}
			break;
		}
		case SNAGGED: {
			if (nextAction >= TUGUP && nextAction <= REEL && chance + poleMod - (vegStat / 2) >= PROCEED) { // Pole Mod increase chance to not snag and vegetation increase chance to snag
				fishingProceed(player, nextAction, marker, fish, boxID, WAITING, false, moodString);
			} else {
				fishingProceed(player, nextAction, marker, fish, boxID, SNAGGED, false, moodString);
			}
			break;
		}
		case NIBBLE: {
			if (chance + poleMod <= MISHAP) { // Pole decreases chance of mishap
				mishapEvent("@fishing:line_snap", player, marker, boxID, true, moodString);
				break;
			} else if ((chance + poleMod >= (PROCEED + FishingConstants::BITE_DENSITY_OFFSET - (fishDensity * 2))) && (nextAction >= TUGUP && nextAction <= REEL)) { // BITE
				ManagedReference<SceneObject*> newMarker = updateMarker(player, marker, boxID, false);

				if (newMarker != nullptr) {
					fishingProceed(player, nextAction, newMarker, fish, boxID, BITE, true, moodString);
				} else {
					fishingProceed(player, nextAction, marker, fish, boxID, BITE, true, moodString);
				}
				break;
			} else if (nextAction >= TUGUP && nextAction <= REEL) { // Action entered & BITE calculation failed
				ManagedReference<SceneObject*> newMarker = updateMarker(player, marker, boxID, false);

				if (newMarker != nullptr) {
					fishingProceed(player, nextAction, newMarker, fish, boxID, NIBBLE, true, moodString);
				} else {
					// Infinity: null marker = zombie session fix. Old code passed null to
					// fishingProceed which silently returned (no new event, no cleanup).
					stopFishing(player, boxID, true);
				}
			} else {
				if (chance <= FishingConstants::NIBBLE_DOWNGRADE_THRESHOLD) {
					fishingProceed(player, nextAction, marker, fish, boxID, WAITING, true, moodString);
				} else {
					fishingProceed(player, nextAction, marker, fish, boxID, NIBBLE, true, moodString);
				}
			}
			break;
		}
		case BITE: {
			if (chance + poleMod <= MISHAP) { // Pole decreases chance of MISHAP
				if (chance < FishingConstants::LOST_BAIT_THRESHOLD) { // Lost Bait
					mishapEvent("@fishing:lost_bait", player, marker, boxID, true, moodString);
					break;
				} else { // Fish pulled out the line, but the player has no penalty
					mishapEvent("@fishing:line_spooled", player, marker, boxID, false, moodString);
					break;
				}
			} else if (chance + poleMod >= PROCEED && nextAction >= TUGUP && nextAction <= REEL) {
				ManagedReference<SceneObject*> newMarker = updateMarker(player, marker, boxID, false);

				if (newMarker != nullptr) {
					fishingProceed(player, nextAction, newMarker, fish, boxID, CATCH, true, moodString);
				} else {
					fishingProceed(player, nextAction, marker, fish, boxID, BITE, true, moodString);
				}
			} else {
				fishingProceed(player, nextAction, marker, fish, boxID, NIBBLE, false, moodString);
			}
			break;
		}
		case CATCH: {
			if (chance + poleMod <= MISHAP) {
				mishapEvent("@fishing:lost_catch", player, marker, boxID, false, moodString);
				break;
			} else if (nextAction == DONOTHING || nextAction == REEL) {
				fishingProceed(player, nextAction, marker, fish, boxID, REELING, true, moodString);
			} else {
				fishingProceed(player, nextAction, marker, fish, boxID, CATCH, true, moodString);
			}
			break;
		}
		case REELING: {
			ManagedReference<SceneObject*> newMarker = updateMarker(player, marker, boxID, true);

			if (newMarker != nullptr)
				fishingProceed(player, nextAction, newMarker, fish, boxID, REELING, true, moodString);
			break;
		}
		case REELGAME: {
			if (chance + poleMod <= MISHAP - FishingConstants::REEL_MISHAP_OFFSET) { // Pole decreases chance of mishap
				mishapEvent("@fishing:tore_bait", player, marker, boxID, true, moodString);
			} else {
				if (player->isInRange(marker, FishingConstants::CATCH_RANGE)) {
					success(player, fish, marker, boxID);
				} else {
					ManagedReference<SceneObject*> newMarker = updateMarker(player, marker, boxID, true);

					if (newMarker != nullptr) {
						fishingProceed(player, nextAction, newMarker, fish, boxID, REELGAME, true, moodString);
					} else {
						// Infinity: updateMarker returned null — old marker was destroyed and new one
						// couldn't be created. Previously both branches called fishingProceed identically
						// (copy-paste bug), passing null marker which caused fishingProceed to silently
						// return — no new event, no cleanup, no feedback. Now properly end the session.
						stopFishing(player, boxID, false);
					}
				}
			}
			break;
		}
	}
}

void FishingManagerImplementation::success(CreatureObject* player, int fish, SceneObject* marker, uint32 boxID) {
	if (player == nullptr) {
		return;
	}

#ifdef DEBUG_FISHING
	player->info(true) << "FishingManagerImplementation::success -- called for " << player->getDisplayedName();
#endif

	// Drop the POSITIONCHANGED observer immediately so position ticks during loot
	// creation cannot race with stopFishing() and trigger a double stop (#1494).
	player->dropObserver(ObserverEventType::POSITIONCHANGED, _this.getReferenceUnsafeStaticCast());

	auto zone = player->getZone();

	if (zone == nullptr) {
		return;
	}

	auto zoneServer = player->getZoneServer();

	if (zoneServer == nullptr) {
		return;
	}

	auto lootManager = zoneServer->getLootManager();
	if (lootManager == nullptr) {
		return;
	}

	TransactionLog trx(TrxCode::FISHING, player);

	if (fish >= FishingConstants::FISH_VS_ITEM_THRESHOLD) { // Non-fish Item Caught
		int chance = System::random(FishingConstants::RARE_LOOT_ROLL_MAX) + player->getSkillMod("luck");
		uint64 lootID = 0;

		if (chance > FishingConstants::RARE_LOOT_THRESHOLD) { // Rare Loot
			if (System::random(4) > 1) {
				lootID = lootManager->createLoot(trx, marker, "weapons_all", 40);
			} else {
				lootID = lootManager->createLoot(trx, marker, "armor_all", 40);
			}
		} else { // Junk Loot Drop
			lootID = lootManager->createLoot(trx, marker, "junk", 40);
		}

		if (lootID > 0) {
			trx.commit();
			sendReward(player, marker, nullptr);
		} else {
			trx.abort() << "FishingManagerImplementation -- Loot Object ID == 0";
		}
	} else {
		// Chance to toss fish away
		int badLuck = System::random(99) + 1;

		if (badLuck <= FishingConstants::BAD_LUCK_PERCENT) { // chance of a bad catch
			player->sendSystemMessage("@fishing:size_matters");
			stopFishing(player, boxID, true);

			trx.abort() << "@fishing:size_matters";
			return;
		} else {
			// Infinity #1618: bounds-check the fish index before fishType.get().
			// FishingSession persistence + session reload after restart can put a
			// stale or corrupted index into the session's `fish` field; if it falls
			// outside [0, fishType.size()) we'd OOB. Capture the bad value, stop
			// the session cleanly, and return — minigame is retry-friendly so the
			// player can just cast again.
			if (fish < 0 || fish >= fishType.size()) {
				player->warning() << "FishingManager::success — OOB fish=" << fish
					<< " fishType.size()=" << fishType.size()
					<< " player=" << player->getDisplayedName();
				stopFishing(player, boxID, true);
				trx.abort() << "OOB fish index";
				return;
			}

			String fishName = fishType.get(fish);

			// Infinity #1618: verify fishLengths contains this name before .get()
			// further down. Defensive — the data sets should always match, but if
			// they ever diverge (custom Lua data, mid-config swap) this prevents
			// a second OOB class on the same call path.
			if (!fishLengths.contains(fishName)) {
				player->warning() << "FishingManager::success — fishLengths missing entry for '"
					<< fishName << "' (fish index " << fish << ") player=" << player->getDisplayedName();
				stopFishing(player, boxID, true);
				trx.abort() << "missing fishLengths entry";
				return;
			}

			String lootFish = "object/tangible/fishing/fish/" + fishName + ".iff";
			ManagedReference<FishObject*> lootFishObject = zoneServer->createObject(lootFish.hashCode(), 2).castTo<FishObject*>();

			if (lootFishObject != nullptr) {
				Locker lootLocker(lootFishObject, player);

				String time = getTime();
				String name = player->getFirstName() + " " + player->getLastName();

				// Determine fish length
				float length = fishLengths.get(fishName);
				float totalLength = length;
				float harvestingMod = player->getSkillMod("creature_harvesting") + 5.f;
				float randBonus = System::frandom(length * 0.5f) - System::frandom(length * 0.5f);

				totalLength += randBonus;

				// Bonus for creature harvesting mod
				if (harvestingMod > 0) {
					harvestingMod = (System::frandom(harvestingMod) / 100.0f) + 1.f;
					totalLength *= harvestingMod;
				}

				String zoneName = zone->getZoneName();

				lootFishObject->setAttributes(name, zoneName, time, (totalLength / 100.f));

				int xp = (totalLength / length) * FishingConstants::XP_BASE;

				ManagedReference<PlayerManager*> playerManager = zoneServer->getPlayerManager();

				if (playerManager != nullptr) {
					playerManager->awardExperience(player, "camp", xp, true);
				}

				int color = fishColors.get(zoneName);

				//error("Player " + name + " caught a " + fishName + " fish in zone " + zoneName + ".  Color: " + String::valueOf(color));

				lootFishObject->setCustomizationVariable("/private/index_color_1", color);

				String baitString = "object/tangible/fishing/bait/bait_chum.iff";
				ManagedReference<TangibleObject*> baitObject = zoneServer->createObject(baitString.hashCode(), 2).castTo<TangibleObject*>();

				if (baitObject != nullptr) {
					Locker baitLocker(baitObject, player);

					TransactionLog trxBait(TrxCode::FISHING, player);

					int useCount = System::random(5) + 1;

					if (useCount >= 1)
						baitObject->setUseCount(useCount, true);

					if (lootFishObject->transferObject(baitObject, -1, true)) {
						trxBait.setSubject(baitObject);
						trxBait.addState("subjectObjectID", baitObject->getObjectID());
						trxBait.addState("subjectUseCount", useCount);
						trxBait.groupWith(trx);

						lootFishObject->broadcastObject(baitObject, true);
					} else {
						baitObject->destroyObjectFromDatabase(true);
					}
				}

				String resourceString = "seafood_fish_" + zoneName;
				int amount = harvestingMod * FishingConstants::RESOURCE_HARVEST_MULTIPLIER;

				ManagedReference<ResourceManager*> resourceManager = zoneServer->getResourceManager();
				ManagedReference<SceneObject*> resource = cast<SceneObject*>(resourceManager->harvestResource(player, resourceString, amount));

				if (resource != nullptr) {
					Locker resourceLocker(resource, player);

					if (lootFishObject->transferObject(resource, -1, true)) {
						TransactionLog trxResource(TrxCode::FISHING, player);
						trxResource.setSubject(resource);
						trxResource.addState("subjectResourceType", resourceString);
						trxResource.addState("subjectResourceQuantity", amount);
						trxResource.groupWith(trx);

						lootFishObject->broadcastObject(resource, true);
					} else {
						resource->destroyObjectFromDatabase(true);
					}
				}

				sendReward(player, marker, cast<SceneObject*>(lootFishObject.get()));

				trx.setSubject(lootFishObject);
				trx.addState("subjectFishType", fishName);
				trx.addState("subjectFishLength", length);
				trx.addState("experienceAwarded", xp);
				trx.commit();
			}
		}
	}

	stopFishing(player, boxID, false);
}

void FishingManagerImplementation::sendReward(CreatureObject* player, SceneObject* marker, SceneObject* loot) {
	if (player != nullptr && marker != nullptr) {
		Locker markerLocker(marker);

		ContainerPermissions* permissions = marker->getContainerPermissionsForUpdate();
		permissions->setOwner(player->getObjectID());

		if (loot != nullptr) {
			if (marker->transferObject(loot, -1, true)) {
				String itemName = "";
				loot->getObjectName()->getFullPath(itemName);

				StringIdChatParameter body("fishing", "prose_notify_catch");
				body.setTT(itemName);
				player->sendSystemMessage(body);
			} else {
				loot->destroyObjectFromDatabase(true);
			}
		} else {
			player->sendSystemMessage("@fishing:reel_loot");
		}
		marker->openContainerTo(player);
	}
}

uint32 FishingManagerImplementation::createWindow(CreatureObject* player, uint32 boxID) {
	if (player == nullptr)
		return -1;

	ManagedReference<SceneObject*> marker = getFishMarker(player);

	if (marker == nullptr)
		return -1;

	// clear old one
	closeMenu(player, boxID);

	int veg = vegetation(marker);
	int den = density(marker);

	ManagedReference<FishingBaitObject*> bait = getBait(player);

	if (bait == nullptr)
		return -1;

	int freshness = bait->getFreshness();

	if (freshness >= MUSH) {
		freshness = 2;
	} else if (freshness >= SOGGY) {
		freshness = 1;
	} else {
		freshness = 0;
	}

	int nextAct = getNextAction(player);

	if (nextAct >= fishingActions.size())
		nextAct = 0;

	int stateIdx = getFishingState(player);
	if (stateIdx < 0 || stateIdx >= fishingStates.size())
		stateIdx = 0;

	String prompt = "STATUS : " + fishingStates.get(stateIdx) + "\nBAIT STATUS : " + baitStatus.get(freshness % baitStatus.size()) + "\nLINE RANGE : " + String::valueOf(ceil(player->getDistanceTo(marker) * 100) / 100) + "\nFISH DENSITY : " + getPropertyString(den % 6) +
					"\nVEGETATION : " + getPropertyString(veg % 6) + "\nNEXT ACTION : " + fishingActions.get(nextAct);

	// create new window
	ManagedReference<SuiListBox*> box = new SuiListBox(player, SuiWindowType::FISHING, 0);

	box->setPromptTitle("@fishing:sui_title");
	box->setPromptText(prompt);

	box->addMenuItem("@fishing:fa_tug_up", 0);
	box->addMenuItem("@fishing:fa_tug_right", 1);
	box->addMenuItem("@fishing:fa_tug_left", 2);
	box->addMenuItem("@fishing:fa_small_reel", 3);
	box->addMenuItem("@fishing:fa_stop", 4);

	player->getPlayerObject()->addSuiBox(box);
	player->sendMessage(box->generateMessage());

	return box->getBoxID();
}

void FishingManagerImplementation::closeMenu(CreatureObject* player, uint32 boxID) {
	PlayerObject* ghost = player->getPlayerObject();

	if (ghost != nullptr) {
		ghost->removeSuiBoxType(SuiWindowType::FISHING);
	}
}

int FishingManagerImplementation::vegetation(SceneObject* marker) {
	if (marker == nullptr)
		return 0;

	return (int)ceil(fabs(marker->getPositionX() / 100)) % 6;
}

int FishingManagerImplementation::density(SceneObject* marker) {
	if (marker == nullptr)
		return 0;

	return (int)ceil(fabs(marker->getPositionY() / 100)) % 6;
}

int FishingManagerImplementation::getFish(CreatureObject* player, SceneObject* marker) {
	int chance = System::random(99);

#ifdef DEBUG_FISHING
	StringBuffer msg;
	msg << " Starting getFish chance = " << chance << "  ";
#endif

	if (player == nullptr || marker == nullptr) {
		return chance;
	}

	int vegStat = vegetation(marker);

	if (vegStat > 0 && chance < FishingConstants::FISH_VS_ITEM_THRESHOLD) {
		chance += vegStat * 5;
	}

#ifdef DEBUG_FISHING
	msg << " chance after vegetation bonus = " << chance << "  ";
#endif

	if (chance < FishingConstants::FISH_VS_ITEM_THRESHOLD) {
		ManagedReference<FishingPoleObject*> pole = getPole(player);
		if (pole == nullptr)
			return chance;

		ManagedReference<FishingBaitObject*> bait = getBait(player);
		if (bait == nullptr)
			return chance;

		chance = (int)((pole->getQuality() * 0.06)			// ACCOUNT FOR POLE QUALITY - quality is 50 at init
					   + (vegStat * -0.75)					// Vegetation Decrease chance to catch a fish
					   + (density(marker) * 0.75)			// ACCOUNT FOR FISH DENSITY - Infinity: was density(player), wrong coordinates
					   + ((3 - bait->getFreshness()) * 3.0) // ACCOUNT FOR BAIT STATUS - freshness is 0 at init
					   + player->getSkillMod("luck")		// ACCOUNT FOR LUCK
					   + (System::random(20)))				// RANDOM BIAS
				 % FishingConstants::FISH_TYPE_COUNT;		// MUX NUMBER TO FISH 0-6
	}

#ifdef DEBUG_FISHING
	msg << " Final Chance: " << chance;
	player->info(true) << msg.toString();
#endif

	return chance;
}

int FishingManagerImplementation::getNextAction(CreatureObject* player) {
	if (player == nullptr)
		return DONOTHING;

	ManagedReference<FishingSession*> fishingSession = getSession(player);

	if (fishingSession == nullptr)
		return DONOTHING;

	return fishingSession->getNextAction();
}

void FishingManagerImplementation::setNextAction(CreatureObject* player, int next) {
	if (player == nullptr)
		return;

	ManagedReference<FishingSession*> fishingSession = getSession(player);

	if (fishingSession == nullptr)
		return;

	if ((next >= DONOTHING) && (next <= STOPFISHING)) {
		fishingSession->setNextAction(next);
	}
}

FishingPoleObject* FishingManagerImplementation::getPole(CreatureObject* player) {
	if (player == nullptr) {
		return nullptr;
	}

	SceneObject* poleSceneO = player->getSlottedObject("hold_r");

	if (poleSceneO == nullptr || !poleSceneO->isFishingPoleObject()) {
		return nullptr;
	}

	ManagedReference<FishingPoleObject*> fishingPole = cast<FishingPoleObject*>(poleSceneO);

	if (fishingPole == nullptr) {
		return nullptr;
	}

	return fishingPole;
}

FishingBaitObject* FishingManagerImplementation::getBait(CreatureObject* player) {
	if (player == nullptr)
		return nullptr;

	// Infinity: Removed unnecessary _this lock — getBait accesses no mutable manager state
	ManagedReference<FishingPoleObject*> pole = getPole(player);

	if (pole != nullptr) {
		if (pole->isFishingPoleObject() && pole->isContainerFull()) {
			auto containerObject = pole->getContainerObject(0);

			if (containerObject != nullptr) {
				ManagedReference<FishingBaitObject*> bait = containerObject.castTo<FishingBaitObject*>();

				if (bait != nullptr) {
					if (bait->isFishingBait()) {
						return bait;
					}
				}
			}
		}
	}

	return nullptr;
}

uint32 FishingManagerImplementation::getFishBoxID(CreatureObject* player) {
	if (player == nullptr)
		return -1;

	ManagedReference<FishingSession*> fishingSession = getSession(player);

	if (fishingSession != nullptr) {
		return fishingSession->getFishBoxID();
	}

	return -1;
}

void FishingManagerImplementation::setFishBoxID(CreatureObject* player, uint32 boxID) {
	if (player == nullptr)
		return;

	ManagedReference<FishingSession*> fishingSession = getSession(player);

	if (fishingSession != nullptr) {
		fishingSession->setFishBoxID(boxID);
	}
}

int FishingManagerImplementation::getFishingState(CreatureObject* player) {
	if (player == nullptr)
		return NOTFISHING;

	ManagedReference<FishingSession*> fishingSession = getSession(player);

	if (fishingSession == nullptr) {
		return NOTFISHING;
	}

	return fishingSession->getFishingState();
}

void FishingManagerImplementation::setFishingState(CreatureObject* player, int state) {
	if (player == nullptr)
		return;

	ManagedReference<FishingSession*> fishingSession = getSession(player);

	if (fishingSession == nullptr)
		return;

	if ((state >= NOTFISHING) && (state <= REELGAME)) {
		fishingSession->setFishingState(state);
	}
}

SceneObject* FishingManagerImplementation::getFishMarker(CreatureObject* player) {
	if (player == nullptr)
		return nullptr;

	ManagedReference<FishingSession*> fishingSession = getSession(player);

	if (fishingSession != nullptr)
		return fishingSession->getMarker();
	else
		return nullptr; // Infinity: was returning player, causing wrong position calculations
}

void FishingManagerImplementation::setFishMarker(CreatureObject* player, SceneObject* marker) {
	if (player == nullptr)
		return;

	ManagedReference<FishingSession*> fishingSession = getSession(player);

	if (fishingSession == nullptr)
		return;

	fishingSession->setMarker(marker);
}

void FishingManagerImplementation::freeBait(CreatureObject* player) {
	if (player == nullptr)
		return;

	auto zoneServer = player->getZoneServer();
	if (zoneServer == nullptr)
		return;

	String bait = "object/tangible/fishing/bait/bait_worm.iff";
	ManagedReference<SceneObject*> baitObject = zoneServer->createObject(bait.hashCode(), 0);

	if (baitObject == nullptr)
		return;

	Locker locker(baitObject);

	ManagedReference<FishingPoleObject*> pole = getPole(player);

	if (pole != nullptr && !pole->isContainerFull()) {
		if (pole->transferObject(baitObject, -1, true)) {
			baitObject->sendTo(player, true);
		} else {
			baitObject->destroyObjectFromDatabase(true);
		}
	} else {
		baitObject->destroyObjectFromDatabase(true);
	}
}

void FishingManagerImplementation::fishingProceed(CreatureObject* player, int nextAction, SceneObject* marker, int fish, uint32 boxID, int newstate, bool sendPlayerMessage, String& moodString) {
	if (player == nullptr || marker == nullptr) {
		return;
	}

	int oldstate = getFishingState(player);

	ManagedReference<FishingSession*> fishingSession = getSession(player);

	if (fishingSession == nullptr) {
		return;
	}

	if (sendPlayerMessage) {
		switch (newstate) {
			case WAITING:
				if (oldstate == SNAGGED)
					player->sendSystemMessage("@fishing:line_free");
				break;
			case SNAGGED: {
				player->sendSystemMessage("@fishing:fs_snag");
				break;
			}
			case NIBBLE: {
				if (oldstate == SNAGGED) {
					player->sendSystemMessage("@fishing:line_free");
				} else {
					player->sendSystemMessage("@fishing:fs_nibble");
				}

				marker->showFlyText("fishing", "fly_nibble", 50, 200, 50);
				break;
			}
			case BITE: {
				marker->showFlyText("fishing", "fly_bite", 230, 150, 30);
				player->sendSystemMessage("@fishing:fs_bite");

				createSplash(marker->getPositionX(), marker->getPositionY(), marker->getPositionZ(), player->getZone(), player);
				break;
			}
			case CATCH: {
				marker->showFlyText("fishing", "fly_catch", 240, 240, 65);
				player->sendSystemMessage("@fishing:fs_caught");

				createSplash(marker->getPositionX(), marker->getPositionY(), marker->getPositionZ(), player->getZone(), player);
				break;
			}
			case REELING: {
				if (fishingSession != nullptr) {
					player->sendSystemMessage("@fishing:reel_in");
					FishingEvent* fishingEvent = createFishingEvent(player, REELGAME);

					if (fishingEvent != nullptr) {
						fishingSession->setEvent(fishingEvent);
						fishingSession->update(REEL, marker, fish, 0, REELGAME);
					}
				}
				break;
			}
			case REELGAME: {
				if (fishingSession != nullptr) {
					FishingEvent* fishingEvent = createFishingEvent(player, newstate);

					if (fishingEvent != nullptr) {
						fishingSession->setEvent(fishingEvent);
						fishingSession->update(REEL, marker, fish, 0, newstate);
					}
				}
				break;
			}
			default:
				break;
		}
	}

	if (nextAction >= DONOTHING && nextAction < REELING && newstate != REELING && newstate != REELGAME) {
		ManagedReference<FishingBaitObject*> bait = getBait(player);

		if (bait != nullptr && System::random(100) < FishingConstants::BAIT_DEGRADE_PERCENT) {
			bait->lessFresh();
		}

		if (player->isInRange(marker, FishingConstants::CATCH_RANGE)) {
			stopFishing(player, boxID, true);
			return;
		}
	}

	if ((newstate != REELING) && (newstate != REELGAME)) {
		if (fishingSession != nullptr) {
			FishingEvent* fishingEvent = createFishingEvent(player, newstate);

			if (fishingEvent != nullptr) {
				fishingSession->update(DONOTHING, marker, fish, boxID, newstate);
				fishingSession->setFishBoxID(createWindow(player, boxID));
				fishingSession->setEvent(fishingEvent);
			}
		}
	}
}

void FishingManagerImplementation::mishapEvent(const String& text, CreatureObject* player, SceneObject* marker, uint32 boxID, bool losebait, String& moodString) {
	if ((player != nullptr) && (!text.isEmpty())) {
		player->sendSystemMessage(text);

		if (losebait) {
			stopFishing(player, boxID, true);
			loseBait(player);
		} else {
			ManagedReference<FishingSession*> fishingSession = getSession(player);

			if (fishingSession != nullptr) {
				FishingEvent* fishingEvent = createFishingEvent(player, WAITING);

				if (fishingEvent != nullptr) {
					fishingSession->update(DONOTHING, getFishMarker(player), getFish(player, marker), boxID, WAITING);
					fishingSession->setFishBoxID(createWindow(player, boxID));
					fishingSession->setEvent(fishingEvent);
				}
			}
		}
	}
}

bool FishingManagerImplementation::loseBait(CreatureObject* player) {
	if (player == nullptr)
		return false;

	ManagedReference<SceneObject*> pole = player->getSlottedObject("hold_r");

	if (pole != nullptr) {
		if (pole->isFishingPoleObject() && pole->isContainerFull()) {
			if (pole->getContainerObject(0) != nullptr) {
				ManagedReference<SceneObject*> bait = pole->getContainerObject(0);

				if (!bait->isFishingBait())
					return false;

				ManagedReference<FishingBaitObject*> fishBait = cast<FishingBaitObject*>(bait.get());

				Locker fishBaitLocker(fishBait);

				if (fishBait->getUseCount() > 1) {
					fishBait->decreaseUseCount();
					fishBait->setFreshness(FRESH);
				} else {
					bait->destroyObjectFromWorld(true);

					if (bait->isPersistent())
						bait->destroyObjectFromDatabase(true);
				}

				return true;
			}
		}
	}

	return false;
}

void FishingManagerImplementation::animate(CreatureObject* player, int nextAction) {
	if (player == nullptr)
		return;

	switch (nextAction) {
		case TUGUP:
			player->doAnimation("fishing_tug_back");
			break;
		case TUGRIGHT:
			player->doAnimation("fishing_tug_right");
			break;
		case TUGLEFT:
			player->doAnimation("fishing_tug_left");
			break;
		case REEL:
			player->doAnimation("fishing_reel");
			break;
	}
}

SceneObject* FishingManagerImplementation::createMarker(Vector3& location, Zone* zone) {
	if (zone == nullptr) {
		return nullptr;
	}

	String marker = "object/tangible/fishing/marker.iff";
	ManagedReference<SceneObject*> markerObject = zone->getZoneServer()->createObject(marker.hashCode(), 0);

	if (markerObject == nullptr) {
		return nullptr;
	}

	Locker locker(markerObject);

	markerObject->initializePosition(location.getX(), location.getZ(), location.getY());

	if (!zone->transferObject(markerObject, -1, true)) {
		return nullptr;
	}

	markerObject->registerObserver(ObserverEventType::CLOSECONTAINER, _this.getReferenceUnsafeStaticCast());

	return markerObject;
}

void FishingManagerImplementation::createSplash(float x, float y, float z, Zone* zone, CreatureObject* player) {
	if (player == nullptr || zone == nullptr)
		return;

	auto zoneServer = player->getZoneServer();

	if (zoneServer == nullptr)
		return;

	String splash = "object/tangible/fishing/splash.iff";
	ManagedReference<SceneObject*> splashObject = zoneServer->createObject(splash.hashCode(), 0);

	if (splashObject == nullptr)
		return;

	Locker locker(splashObject);

	splashObject->initializePosition(x, z + 0.5, y);
	zone->transferObject(splashObject, -1, true);

	createFishingSplashEvent(splashObject);
}

bool FishingManagerImplementation::checkUpdateMarker(CreatureObject* player, Vector3& location) {
	bool beached = false;

	float newZ = location.getZ();

	if (!player->getZone()->getPlanetManager()->getTerrainManager()->getWaterHeight(location.getX(), location.getY(), newZ)) {
		beached = true;
	}

	newZ = player->getZone()->getHeight(location.getX(), location.getY());

	if (location.getZ() < newZ) {
		location.setZ(newZ);
		beached = true;
	}

	return beached;
}

SceneObject* FishingManagerImplementation::updateMarker(CreatureObject* player, SceneObject* marker, uint32 boxID, bool reelGame) {
	if (player == nullptr || marker == nullptr) {
		return nullptr;
	}

	auto zone = player->getZone();

	if (zone == nullptr) {
		return nullptr;
	}

	Locker markerLocker(marker);

	// Calculate new bobber position
	Vector3 newMarkerLoc;
	Vector3 markerLoc;
	float playerX = player->getPositionX();
	float playerY = player->getPositionY();

	markerLoc.setX(marker->getPositionX());
	markerLoc.setY(marker->getPositionY());

	newMarkerLoc.setX(playerX + ((markerLoc.getX() - playerX) * FishingConstants::MARKER_APPROACH_RATE)); // Closes X,Y by 25% of distance
	newMarkerLoc.setY(playerY + ((markerLoc.getY() - playerY) * FishingConstants::MARKER_APPROACH_RATE));
	newMarkerLoc.setZ(marker->getPositionZ());

	if (reelGame) {
		int event = System::random(15);
		switch (event) {
			case 1:
				// Fish Charge
				newMarkerLoc.setX(playerX + ((markerLoc.getX() - playerX) * FishingConstants::FISH_CHARGE_RATE)); // Closes X,Y by 35% of distance
				newMarkerLoc.setY(playerY + ((markerLoc.getY() - playerY) * FishingConstants::FISH_CHARGE_RATE));
				player->sendSystemMessage("@fishing:fish_charge");
				break;
			case 2:
				// Fight Easy
				player->sendSystemMessage("@fishing:fish_fight_easy");
				break;
			case 3:
				// Fight Hard
				newMarkerLoc.setX(markerLoc.getX() + (System::random(5) + 1) / 10.0f); // Infinity:  Fights hard, moves away between 0.1 and 0.5 meters
				newMarkerLoc.setY(markerLoc.getY() + (System::random(5) + 1) / 10.0f);
				player->sendSystemMessage("@fishing:fish_fight_hard");
				break;
			case 4:
				// Fight Away
				newMarkerLoc.setX(markerLoc.getX() + (System::random(10) + 1) / 10.0f); // Infinity:  Fights hard, moves away between 0.1 and 1.0 meters
				newMarkerLoc.setY(markerLoc.getY() + (System::random(10) + 1) / 10.0f);
				player->sendSystemMessage("@fishing:fish_fight_hard");
				break;
			case 5: {
				// Fish Run
				float angle = player->getDirectionAngle();

				if (angle > 360) {
					angle = angle - 360;
				}

				angle = 2 * M_PI * angle / 360;

				newMarkerLoc.setX(markerLoc.getX() + sin(angle) * (0.5f + (System::random(5) + 1) / 10.0f)); // Infinity:  Runs out minimum of 0.5 meters up to 1 meter
				newMarkerLoc.setY(markerLoc.getY() + cos(angle) * (0.5f + (System::random(5) + 1) / 10.0f));

				player->sendSystemMessage("@fishing:fish_run");
				break;
			}
			default:
				// succeed in drawing your catch closer
				player->sendSystemMessage("@fishing:fish_fight_closer");
				break;
		}
	}

	if (checkUpdateMarker(player, newMarkerLoc)) { // Beached
		if (reelGame) {
			player->sendSystemMessage("@fishing:loot_beached");
			newMarkerLoc = markerLoc;
		} else {
			player->sendSystemMessage("@fishing:loc_unfishable");
			stopFishing(player, boxID, true);
			return nullptr;
		}
	}

	removeMarker(player, marker);

	ManagedReference<SceneObject*> newMarker = createMarker(newMarkerLoc, zone);

	if (newMarker == nullptr)
		return nullptr;

	// Infinity: was Locker lock(player, marker) — but marker was just destroyed by removeMarker
	setFishMarker(player, newMarker);

	return newMarker;
}

// Marker and player are prelocked
void FishingManagerImplementation::removeMarker(CreatureObject* player, SceneObject* marker) {
	if (player == nullptr || marker == nullptr || marker->isPlayerCreature())
		return;

	// Clear fish marker for player fishing session
	setFishMarker(player, nullptr);

	for (int i = marker->getContainerObjectsSize() - 1; i >= 0; --i) {
		ManagedReference<SceneObject*> object = marker->getContainerObject(i);

		if (object == nullptr)
			continue;

		Locker clock(object, player);

		if (object->isPersistent()) {
			object->destroyObjectFromDatabase(true);
		}

		object->destroyObjectFromWorld(true);
	}

	// Destroy the marker
	marker->destroyObjectFromWorld(true);

	if (marker->isPersistent()) {
		marker->destroyObjectFromDatabase(true);
	}
}

void FishingManagerImplementation::removeSplash(SceneObject* splash) {
	if (splash == nullptr)
		return;

	splash->destroyObjectFromWorld(true);

	if (splash->isPersistent()) {
		splash->destroyObjectFromDatabase(true);
	}
}

void FishingManagerImplementation::createFishingSplashEvent(SceneObject* splash) {
	if (splash == nullptr)
		return;

	Reference<FishingSplashEvent*> splashEvent = new FishingSplashEvent(splash);

	if (splashEvent != nullptr)
		splashEvent->schedule(FishingConstants::SPLASH_DURATION_MS);
}

FishingEvent* FishingManagerImplementation::createFishingEvent(CreatureObject* player, int state) {
	if (player == nullptr)
		return nullptr;

	player->removePendingTask("fishing");

	Reference<FishingEvent*> fishingEvent = new FishingEvent(player, state);

	// Infinity: Variable timer — faster after catch state
	int timer = (state >= CATCH) ? (System::random(FishingConstants::FAST_TIMER_RANGE_MS) + FishingConstants::FAST_TIMER_MIN_MS) : FishingConstants::NORMAL_TIMER_MS;

	player->addPendingTask("fishing", fishingEvent, timer);

	return fishingEvent;
}

void FishingManagerImplementation::stopFishingEvent(CreatureObject* player) {
	if (player == nullptr) {
		return;
	}

	// removePendingTask cancels the task, no need for explicit cancel()
	player->removePendingTask("fishing");
}

FishingEvent* FishingManagerImplementation::getFishingEvent(CreatureObject* player) {
	if (player == nullptr)
		return nullptr;

	ManagedReference<FishingSession*> fishingSession = getSession(player);

	if (fishingSession == nullptr)
		return nullptr;

	FishingEvent* fishingEvent = fishingSession->getEvent();

	return fishingEvent;
}

bool FishingManagerImplementation::hasFishingSession(CreatureObject* player) {
	return getSession(player) != nullptr;
}

int FishingManagerImplementation::notifyCloseContainer(CreatureObject* player, SceneObject* container) {
	if (player == nullptr || container == nullptr)
		return 1;

	player->dropActiveSession(SessionFacadeType::FISHING);

	Reference<FishingManager*> managerRef = _this.getReferenceUnsafeStaticCast();
	Reference<SceneObject*> markerRef = container;
	Reference<CreatureObject*> playerRef = player;

	Core::getTaskManager()->scheduleTask([managerRef, markerRef, playerRef] () {
		if (managerRef == nullptr || markerRef == nullptr || playerRef == nullptr)
			return;

		Locker lock(playerRef);

		Locker clock(markerRef, playerRef);

		managerRef->removeMarker(playerRef, markerRef);
	}, "RemoveMarkerLambda", 100);

	return 1;
}

// Function is triggered by player movement
void FishingManagerImplementation::checkFishingOnPositionUpdate(CreatureObject* player) {
	if (player == nullptr) {
		return;
	}

	Locker lock(player);

	ManagedReference<FishingSession*> fishingSession = getSession(player);

	if (fishingSession == nullptr || player->isSwimming()) {
		stopFishing(player, 0, true);
		return;
	}

	int fishingState = fishingSession->getFishingState();

	if (fishingState == FishingManager::NOTFISHING) {
		stopFishing(player, 0, true);
		return;
	}

	ManagedReference<SceneObject*> marker = fishingSession->getMarker();

	if (marker == nullptr) {
		stopFishing(player, 0, true);
		return;
	}

	float distance = player->getDistanceTo(marker);

	// Player has moved and bobber is either too far or player got too close and scared the fish
	if ((distance < FishingConstants::CATCH_RANGE && fishingState < REELING) || distance > FishingConstants::MAX_LINE_RANGE) {
		stopFishing(player, 0, true);
		return;
	// Player has moved but is still in valid range but not close enough to catch the fish
	} else if (fishingState < REELING || distance > FishingConstants::CATCH_RANGE) {
		return;
	}

	success(player, fishingSession->getFish(), marker, 0);
}

String FishingManagerImplementation::getTime() {
	time_t seconds;
	time(&seconds);
	struct tm timeinfo;
	gmtime_r(&seconds, &timeinfo);
	char buffer[64];
	strftime(buffer, sizeof(buffer), "%a %b %d %H:%M:%S %Y UTC", &timeinfo);
	return String(buffer);
}

String FishingManagerImplementation::getPropertyString(int amount) {
	String msg = "";

	for (int i = 0; i < amount; ++i) {
		msg += "*";
	}

	return msg;
}