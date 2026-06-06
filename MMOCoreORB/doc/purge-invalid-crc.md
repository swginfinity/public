# Implementation Plan: `./core3 purgeinvalidcrc`

Standalone offline utility mode that scans object databases for entries whose
`serverObjectCRC` no longer maps to a valid template and removes them.

## Why standalone?

- `sceneobjects` is ~80 GB on production — iterating it on a live server would
  block the command queue thread for minutes and thrash memory.
- No object instantiation needed — we only read two fields per entry
  (`serverObjectCRC` and `_className`) from raw serialized data.
- No ZoneServer, no networking, no object broker required.
- Server must be **offline** (BDB databases can't be opened by two processes).

## Databases to scan

All databases that store SceneObject-derived objects with a `serverObjectCRC`
field.  These are loaded by `ObjectManager::ObjectManager()` in
`src/server/zone/managers/object/ObjectManager.cpp` lines 128-151:

| Database           | Contents                              | Scan? |
|--------------------|---------------------------------------|-------|
| `sceneobjects`    | All player items, NPCs, world objects | Yes   |
| `clientobjects`   | Static/client-side objects            | No    |  - Can be deleted and regenerated on boot
| `playerstructures`| Player-placed structures              | Yes   |
| `draftschematics`  | Draft schematics                     | Yes   |
| `cityregions`     | City region active areas              | Yes   |
| `spawnareas`      | Spawn area active areas               | No    |   - Can be deleted and regenerated on boot
| `navareas`        | Nav mesh areas                        | No    |	- Can be deleted and regenerated on boot
| `guilds`          | Guild objects                         | Yes   |
| `resourcespawns`  | Resource spawn objects                | Yes   |
| `buffs`           | Buff objects (ManagedObject, no CRC)  | No    |
| `mail`            | Mail (ManagedObject)                  | No    |
| `chatrooms`       | Chat rooms (ManagedObject)            | No    |
| `accounts`        | Accounts (ManagedObject)              | No    |

## Initialization requirements

The tool needs exactly two subsystems:

1. **ConfigManager** — to read `conf/config.lua` for TRE path and DB path.
   Already a Singleton, init via `ConfigManager::instance()->loadConfigData()`.

2. **TemplateManager** — loads TRE archive and lua object templates so we can
   validate CRCs. Singleton, auto-inits on `instance()`. But templates are
   loaded separately by calling `loadLuaTemplates()`.
   Call chain: `TemplateManager::instance()` (loads TRE, slots, etc.) then
   `templateManager->loadLuaTemplates()` (runs `scripts/object/main.lua`).

3. **ObjectDatabaseManager** — to open BDB databases.
   `ObjectDatabaseManager::instance()->loadObjectDatabase(name, true)`.

No ServerCore, no ZoneServer, no ObjectBroker, no networking.

## Implementation steps

### 1. Add command-line argument in `main.cpp`

In the existing argument dispatch chain (after `dumpnavmeshestofile`, before
the `convert` block), add:

```cpp
} else if (arguments.contains("purgeinvalidcrc")) {
    ConfigManager::instance()->loadConfigData();

    ret = purgeInvalidCRC(arguments);
    _exit(ret);
}
```

### 2. Create `src/tools/PurgeInvalidCRC.h`

Static utility class with the purge logic.  Keep it as a header-only
implementation (like the existing command headers) or a .cpp if preferred.

```
PurgeInvalidCRC::run(const SortedVector<String>& arguments) -> int
```

### 3. Core algorithm

```
Phase 1: Initialize
  - ConfigManager::instance()->loadConfigData()
  - TemplateManager::instance()  // loads TRE
  - templateManager->loadLuaTemplates()  // loads all CRC->template mappings
  - ObjectDatabaseManager::instance()

Phase 2: Scan each database
  For each database name in the scan list:
    - loadObjectDatabase(name, true)
    - ObjectDatabaseIterator to walk all entries
    - For each entry, read serverObjectCRC from raw ObjectInputStream:
        Serializable::getVariable<uint32>(
            STRING_HASHCODE("SceneObject.serverObjectCRC"),
            &crc, &objectData)
    - If crc != 0 && templateManager->getTemplate(crc) == nullptr:
        - Read _className for logging
        - Add to invalid list (or delete immediately)
    - Track: total scanned, invalid found per database

Phase 3: Delete or report
  Two modes:
    --dry-run (default): scan and report only, no deletions
    --delete: actually call database->deleteData(objectID)

Phase 4: Summary
  Print per-database stats and total.
```

### 4. Dry-run vs delete mode

Default behavior should be **dry-run** (report only). Require an explicit
`--delete` or `delete` argument to actually remove entries. This prevents
accidental data loss.

```
./core3 purgeinvalidcrc              # scan and report
./core3 purgeinvalidcrc delete       # scan and delete
```

### 5. Output format

```
[PurgeInvalidCRC] Initializing templates...
[PurgeInvalidCRC] Loaded 12345 templates.
[PurgeInvalidCRC] Scanning database: sceneobjects
[PurgeInvalidCRC]   Scanned 5000000 objects, found 42 invalid.
[PurgeInvalidCRC]   OID=281475279658211 CRC=0xDEADBEEF class=TangibleObject
[PurgeInvalidCRC]   OID=281475279744099 CRC=0x12345678 class=WeaponObject
[PurgeInvalidCRC]   ...
[PurgeInvalidCRC] Scanning database: playerstructures
[PurgeInvalidCRC]   Scanned 50000 objects, found 0 invalid.
[PurgeInvalidCRC] Scanning database: draftschematics
[PurgeInvalidCRC]   Scanned 100000 objects, found 15 invalid.
[PurgeInvalidCRC]
[PurgeInvalidCRC] Summary:
[PurgeInvalidCRC]   sceneobjects:     5000000 scanned, 42 invalid
[PurgeInvalidCRC]   playerstructures: 50000 scanned, 0 invalid
[PurgeInvalidCRC]   draftschematics:  100000 scanned, 15 invalid
[PurgeInvalidCRC]   Total: 5150000 scanned, 57 invalid, 57 deleted (or: 0 deleted - dry run)
```

### 6. Log to file

Write detailed per-object results to `log/purge_invalid_crc.log` so there's a
permanent record of what was removed, including OID, CRC, and className for
each entry.

### 7. Progress reporting

For large databases (sceneobjects), print progress every 100,000 or 1,000,000
entries so the operator knows it's still running:

```
[PurgeInvalidCRC]   Progress: 1000000 / ? scanned, 3 invalid so far...
```

### 8. Files to create/modify

| File | Action |
|------|--------|
| `src/main.cpp` | Add `purgeinvalidcrc` argument handler |
| `src/tools/PurgeInvalidCRC.h` | New file — purge logic |
| `src/CMakeLists.txt` | No change needed (compiles into core3 binary) |

### 9. Safety considerations

- **Server must be offline.** BDB does not support concurrent access from
  multiple processes. The tool should check for a lock file or at minimum
  print a warning.
- **Dry-run default.** Never delete without explicit confirmation.
- **Backup reminder.** Print a warning to back up databases before running
  with `delete`.
- **No object instantiation.** We never call `getObject()` or load objects
  into the object broker. We only read raw bytes from BDB and check the CRC
  field. This keeps memory usage minimal even for 80 GB databases.
- **Transaction safety.** BDB deletes are transactional. Consider committing
  in batches (every 1000 deletes) to avoid huge transaction logs.

### 10. Future extensions

- Support targeting a single database: `./core3 purgeinvalidcrc sceneobjects`
- Export invalid entries to JSON before deletion (reuse the POD/JSON dump
  pattern from `ObjectManager::loadPersistentObject`)
- Integrate with `failed_objects.log` — cross-reference what was already
  flagged at runtime
