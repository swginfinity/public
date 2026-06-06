# BUG-2: Space Station Landing Not Working

## Problem
After merging 2.0.0 + 3.0.0 into 4.0.0, players could target space stations but could not initiate the conversation menu to land. The conversation system was incorrectly restricted to `CreatureObject` only, breaking conversations with `SpaceStationObject` (which extends `ShipAiAgent` -> `ShipObject` -> `TangibleObject` -> `SceneObject`, NOT `CreatureObject`).

## Root Cause
Multiple issues during the merge:

1. **ConversationObserver type mismatch**: The base `ConversationObserver` was changed to use `SceneObject` for the NPC parameter, but `LuaConversationObserver` still had `CreatureObject` parameters. This caused virtual method override to not match, so Lua conversation handlers were never called.

2. **Missing Lua bindings**: The Lua scripts call `SceneObject:isShipObject()` and `SceneObject:checkInConversationRange()` but these methods were never exposed to Lua in `LuaSceneObject.cpp`.

3. **ScreenHandler signature mismatch**: `ScreenHandler` and derived classes expected `CreatureObject*` for the NPC parameter instead of `SceneObject*`.

4. **LuaShipObject not registered**: The `ShipObject()` Lua wrapper class existed but was never registered in DirectorManager, making it unavailable to Lua scripts.

5. **Missing ship state methods**: `isPilotingShip()`, `isPobShipOperator()`, `isShipGunner()` were missing from CreatureObject.idl.

6. **Missing Lua globals**: Ship-related CreatureState, PlayerArrangement, ShipManager, and ShipFlag constants were not exposed to Lua.

## Files Modified

### C++ Core Files

**ConversationObserverImplementation.cpp** (restored from 3.0.0)
- Changed `conversingObject` from `CreatureObject*` to `SceneObject*` throughout
- All methods now accept `SceneObject*` for NPC parameter

**LuaConversationObserver.idl**
- Changed `conversingNPC` parameter from `CreatureObject` to `SceneObject` in:
  - `getNextConversationScreen()`
  - `runScreenHandlers()`

**DirectorManager.cpp**
- Updated method signatures to use `SceneObject*` for `conversingNPC`:
  - `getNextConversationScreen()`
  - `runScreenHandlers()`
- Added `#include "server/zone/objects/ship/LuaShipObject.h"`
- Added `Luna<LuaShipObject>::Register()` to register ShipObject Lua wrapper
- Added ship-related includes from 3.0.0:
  - `#include "server/zone/managers/ship/ShipManager.h"`
  - `#include "templates/params/ship/ShipFlags.h"`
  - `#include "templates/params/creature/PlayerArrangement.h"`
  - `#include "server/zone/objects/ship/components/ShipChassisComponent.h"`
- Added Lua globals for:
  - PlayerArrangement ship positions (SHIP_PILOT, SHIP_OPERATIONS, SHIP_GUNNER*, etc.)
  - CreatureState ship states (PILOTINGSHIP, SHIPOPERATIONS, SHIPGUNNER, SHIPINTERIOR, PILOTINGPOBSHIP)
  - ShipManager ship types (SHIP, FIGHTERSHIP, POBSHIP, SPACESTATION)
  - ShipFlag constants (SHIP_AI_ESCORT, SHIP_AI_FOLLOW)

**ScreenHandler.h**
- Changed `handleScreen()` signature from `CreatureObject* conversingNPC` to `SceneObject* conversingNPC`

**All ScreenHandler implementations** (added internal CreatureObject casts where needed):
- DeliverMissionScreenHandler.cpp
- PetTrainingScreenHandler.cpp
- (other handlers that only work with creatures)

**LuaSceneObject.h / LuaSceneObject.cpp** (NEW Lua bindings)
- Added `isShipObject()` - exposes SceneObject::isShipObject() to Lua
- Added `checkInConversationRange()` - exposes SceneObject::checkInConversationRange() to Lua

**CreatureObject.idl**
- Added `include server.zone.objects.ship.ShipObject;`
- Added ship state methods from 3.0.0:
  - `isPilotingShip()` - checks PILOTINGSHIP or PILOTINGPOBSHIP states
  - `isOnboardPobShip()` - checks SHIPINTERIOR, SHIPOPERATIONS, or SHIPGUNNER
  - `isInShipStation()` - checks PILOTINGPOBSHIP, SHIPOPERATIONS, or SHIPGUNNER
  - `isPobShipOperator()` - checks SHIPOPERATIONS state
  - `isShipGunner()` - checks SHIPGUNNER state

**LuaCreatureObject.h / LuaCreatureObject.cpp**
- Added `isPilotingShip()` Lua binding

**ConversationSession.h / ConversationSession.idl** (if modified)
- Changed `getNPC()` return type from `CreatureObject` to `SceneObject`

### Lua Files

**object_manager.lua**
- Added `_localLuaShipObject = LuaShipObject(nil)`
- Added `ShipObject()` wrapper function (was missing from merge)

### Debug Logging Added (temporary)

**SpaceStationObjectImplementation.cpp**
- `[SPACESTATION]` prefixed debug output in all conversation methods

**NpcConversationStartCommand.h**
- `[CONVO_START]` prefixed debug output

**spacestation_naboo_conv_handler.lua**
- `[NABOO_CONV]` debug print statements

**spacestation.lua**
- `[SPACESTATION_LUA]` debug print statements in landShip function

**RadialManagerImplementation.cpp**
- `[RADIAL]` debug print statements

**SpaceManagerImplementation.cpp**
- `logSpace()` function writing to log/space.log

## Testing Steps

1. Build the server: `cd infinity4.0.0/MMOCoreORB && make core3 -j24`
2. Launch server and client
3. Fly to Naboo space
4. Target space station (press C)
5. Press Shift+C to open comm menu
6. Select "Request Permission to Land"
7. Select a destination (e.g., Moenia)
8. Click "(Land)"
9. Verify player lands at the destination starport

## Cleanup (after verification)

Remove debug logging from:
- SpaceStationObjectImplementation.cpp
- NpcConversationStartCommand.h
- spacestation_naboo_conv_handler.lua
- spacestation.lua
- RadialManagerImplementation.cpp
- SpaceManagerImplementation.cpp

## Related Issues
- BUG-1: Space station targeting (fixed separately)

---

# BUG-3: Lua setState/clearState/hasState truncate 64-bit CreatureState values

## Problem
When using PilotSeatMenuComponent to enter the pilot seat of a POB ship, the player gets flooded with garbage creature states (combat, berserk, on fire, glowingJedi) instead of PILOTINGPOBSHIP. The player becomes stuck, unable to move or heal.

## Root Cause
`LuaCreatureObject.cpp` uses `uint32` to extract state values from Lua, but JTL space states require 64 bits:
- `SHIPINTERIOR = 0x100000000` (bit 32)
- `PILOTINGPOBSHIP = 0x200000000` (bit 33)

The `uint32` cast truncates these values, causing `setState(PILOTINGPOBSHIP)` to set wrong state bits. 3.0.0 correctly uses `uint64` - the 2.0.0 version (`uint32`) was kept during the merge.

## Fix
Changed `uint32` to `uint64` in three methods in `LuaCreatureObject.cpp`:
- `hasState()` (line 324)
- `setState()` (line 331)
- `clearState()` (line 2549)

## Files Modified
- LuaCreatureObject.cpp

---

# BUG-4: POB ship pilot seat re-entry not working

## Problem
After launching a POB ship and leaving the pilot seat via /leavestation, there was no way to re-enter the pilot seat from inside the ship.

## Root Cause
Five shipcontrol Lua templates had empty `{}` bodies in 4.0.0, missing their `objectMenuComponent` references that were present in 3.0.0. Without these, the Lua screenplay menu components (PilotSeatMenuComponent, ShipOperationsMenuComponent, ShipTurretMenuComponent) never activated.

## Fix
Added objectMenuComponent to each shipcontrol template:
- `shipcontrol_falcon.lua` → `PilotSeatMenuComponent`
- `shipcontrol_corvette.lua` → `PilotSeatMenuComponent`
- `shipoperations_falcon.lua` → `ShipOperationsMenuComponent`
- `turretcontrol_falcon.lua` → `ShipTurretMenuComponent`
- `turretcontrol_vt49.lua` → `ShipTurretMenuComponent`

The Lua screenplay components already existed in 4.0.0 and were already included in screenplays.lua.

## Files Modified
- shipcontrol_falcon.lua
- shipcontrol_corvette.lua
- shipoperations_falcon.lua
- turretcontrol_falcon.lua
- turretcontrol_vt49.lua

---

# BUG-5: Unknown command CRC 0xC47E6DCA on ship landing (TODO)

## Problem
When landing a ship and returning to ground, the console logs:
```
CommandQueue Invalid enqueueCommand call -- actionCRC: 3296619978 (0xC47E6DCA)
```

## Status
Not yet investigated. Likely a JTL command not registered in 4.0.0's command table.

---

# BUG-6: JTL Merge - Lua Loading Fixes (Unit Test Failures)

## Problem
After the JTL C++ merge compiled clean, Lua unit tests (`LuaMobileTest`, `BasicScreenPlayTest`) failed due to missing files, broken paths, orphaned screenplays, duplicate mobile definitions, and missing utility modules.

## Fixes Applied

### 1. Orphaned space/screenplays.lua (BasicScreenPlayTest)
`scripts/screenplays/space/screenplays.lua` was never included from the top-level `screenplays.lua`. It contains ~80 files: quest screenplays, squadron screenplays, conversation handlers, ship component menus, and spawning scripts.

**Fix:** Replaced 19 individual space includes in `screenplays.lua` (lines 710-728) with single `includeFile("space/screenplays.lua")`.

### 2. Broken screenplay include paths (BasicScreenPlayTest)
Three ship component menu files were included with wrong paths (`space/` instead of `space/ship_components/`).

**Fix:** Corrected in `space/screenplays.lua` which now loads them at the right paths.

### 3. Duplicate mobile definitions (LuaMobileTest - overwriting errors)
19 space mobiles existed in both planet directories (corellia/, dantooine/, tatooine/, etc.) AND `scripts/mobile/space/`. The planet versions load first, space overwrites them.

**Fix:** Commented out 19 duplicate includes in `scripts/mobile/space/serverobjects.lua`. Planet versions (2.0.0 originals) are kept.

Affected mobiles: adwan_turoldine, bek_rabor, dac_axfow, eryk_chinn, insurgent, kilnstrider, ta_667, willham_burke, zo_ssa, beissa, da_la_socuna, freelance_pilot, gunham, shamdon_kree, smuggler_pilot, talon_karrde, ty_vila, vrak, imperial_pilot

### 4. Missing conversation templates (LuaMobileTest - template not found)
18 space mobiles reference `conversationTemplate` values that aren't loaded. The conversation files exist in `scripts/mobile/conversations/space/` but `space_conversations.lua` is not included in the main conversation loader.

**Fix (temporary):** Set `conversationTemplate = ""` on all 18 mobiles with TODO comment.

Affected mobiles: rikkh, vincie_kalhoon, mooch_davoney, tarth_jaxx, guillo_parootchie, io_tsomcren, j_pai_brek, kreezo, zhanks, kulton_woodle, rhea, landau, kaydine, dulios, dinge, gil_burtin, ral_mundi, ramna

## Stubs That Need Real Implementations

### scripts/utils/space_helpers.lua (STUB)
53 stub methods returning safe defaults (false/nil/0/{}). Used by quest screenplays, squadron screenplays, and conversation handlers. Real implementation likely exists in swgemu unstable repo.

Key method groups:
- Quest management: activateSpaceQuest, completeSpaceQuest, failSpaceQuest, clearSpaceQuest
- Task management: activateSpaceQuestTask, completeSpaceQuestTask, clearSpaceQuestTask
- Quest status: isSpaceQuestActive, isSpaceQuestComplete, isSpaceQuestTaskActive, isSpaceQuestTaskComplete
- Pilot checks: isPilot, isRebelPilot, isImperialPilot, isNeutralPilot, isInYacht
- Squadron checks: isCorsecSquadron, isHavocSquadron, isRSFSquadron
- Skills: grantNovicePilot, grantSpaceSkill, surrenderPilot, hasPilotTierSkill
- Ship/faction: getPlayerShip, getPlayerShipFactionHash, getPlayerSpaceFactionString
- Waypoints: 12 add*Waypoint methods, clearQuestWaypoint(s)
- Rewards: spaceCreditReward, spaceItemReward, sendQuestReward
- Utility: getRandomPositionInSphere, sendQuestUpdate, sendQuestProgess

### scripts/screenplays/space/space_chassis_dealer_conv_handler.lua (STUB)
Empty conversation handler (returns nil). No NPC currently references it. May need real implementation from swgemu unstable.

### 18 Space Mobile conversationTemplate Fields (BLANKED)
Need to hook up `scripts/mobile/conversations/space/space_conversations.lua` in the conversation loader chain. The conversation template files all exist - they're just not loaded. Adding `includeFile("conversations/space/space_conversations.lua")` to `scripts/mobile/conversations.lua` and restoring the original conversationTemplate values will fix this.

## TODO
- [ ] Clone swgemu unstable repo to get real `space_helpers.lua`
- [ ] Hook up `space_conversations.lua` in conversation loader and restore 18 conversationTemplate values
- [ ] Verify `space_chassis_dealer_conv_handler` is needed or remove the include
