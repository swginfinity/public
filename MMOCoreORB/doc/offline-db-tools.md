# Offline Database Tools: `./core3 <tool>`

Standalone offline utility modes that run database maintenance operations
before the server fully initializes. The server must be **offline** — BDB
databases cannot be opened by two processes simultaneously.

These tools share a common pattern: parse arguments in `main.cpp`, initialize
only the subsystems needed, run the operation, then `_exit()`.

---

## `./core3 removeadk`

Scans `sceneobjects` for all AntiDecayKit objects — both standalone (in
inventory) and applied (referenced via `antiDecayKitObject` on weapons/wearables)
— logs each one, removes the reference from the parent item, and deletes them
from the database.

### Why offline?

Running this as a live `/database sceneobjects removeadk` command caused a
deadlock on TC. The DB iterator scan + object locking pattern does not mix
safely with the live object broker and task manager. Offline avoids all of that.

### Initialization requirements

Only two subsystems needed:

1. **ObjectDatabaseManager** — opens BDB databases directly.
2. **ObjectManager** (partially) — needed to resolve `ManagedReference<>` fields
   from raw DB data and to call `destroyObjectFromDatabase`.

No ZoneServer, no ZoneWorkers, no networking, no Lua templates.

### Command-line usage

```
./core3 removeadk              # scan and report only (dry-run)
./core3 removeadk delete       # scan and delete
```

### Implementation steps

#### 1. Add argument handler in `main.cpp`

In the existing argument dispatch chain (alongside `dumpnavmeshestofile`,
`purgeinvalidcrc`, etc.):

```cpp
} else if (arguments.contains("removeadk")) {
    ConfigManager::instance()->loadConfigData();
    ret = RemoveADK::run(arguments);
    _exit(ret);
}
```

#### 2. Create `src/tools/RemoveADK.h`

Static utility class. Header-only is fine.

```cpp
class RemoveADK {
public:
    static int run(const SortedVector<String>& arguments);
};
```

#### 3. Core algorithm

```
Phase 1: Initialize
  - ObjectDatabaseManager::instance()
  - loadObjectDatabase("sceneobjects", true)

Phase 2: DB scan (raw, no object instantiation)
  For each entry in sceneobjects:
    - Read _className: if "AntiDecayKit" -> add to standaloneADKs
    - Read TangibleObject.antiDecayKitObject (hash 0x2e41d02d):
        if non-null -> add parentOID to parentItemOIDs

Phase 3: Remove applied ADKs
  For each parentOID in parentItemOIDs:
    - Read parent entry raw data to get antiDecayKitObject OID
    - Log: parentOID, ADK OID, try to read _className of parent
    - If delete mode:
        - Zero out antiDecayKitObject field in parent DB entry (update)
        - database->deleteData(adkOID)
    - Add adkOID to handledADKs

Phase 4: Remove standalone ADKs
  For each adkOID in standaloneADKs not in handledADKs:
    - Read raw entry to get parent OID for logging
    - Log: ADK OID, parent OID
    - If delete mode: database->deleteData(adkOID)

Phase 5: Summary
  Print counts: applied removed, standalone removed, dry-run totals
```

#### 4. Dry-run vs delete mode

Default is **dry-run** — scan, log, report only. Require explicit `delete`
argument to actually remove entries.

```
./core3 removeadk              # scan and report
./core3 removeadk delete       # scan and delete
```

#### 5. Output format

```
[RemoveADK] Scanning sceneobjects database...
[RemoveADK] Scan complete. Standalone ADKs: 2, Items with applied ADKs: 1
[RemoveADK]   Applied ADK 281475279744243 on item 281475279402546 (object/weapon/melee/sword.iff)
[RemoveADK]   Standalone ADK 281475279744099 parent=281474993891614
[RemoveADK]   Standalone ADK 281475279658211 (no parent)
[RemoveADK] DRY RUN - no changes made. Run with 'delete' to remove.
  -- or --
[RemoveADK] Complete. Applied removed: 1, Standalone removed: 2, DB-only: 0
```

#### 6. Log to file

Write output to `log/removeadk.log` in addition to stdout so there's a
permanent record of what was removed.

#### 7. Files to create/modify

| File | Action |
|------|--------|
| `src/main.cpp` | Add `removeadk` argument handler |
| `src/tools/RemoveADK.h` | New file — removal logic |

#### 8. Safety

- **Server must be offline.** Print a warning if `core3.pid` exists.
- **Dry-run default.** Never delete without explicit `delete` argument.
- **Backup reminder.** Print warning to back up `databases/sceneobjects.db`
  before running with `delete`.
- **No object broker.** Read and write raw BDB entries only — do not call
  `getObject()` or any object broker methods.

---

## Future tools

Additional offline tools to be added here as implemented:

| Tool | Purpose |
|------|---------|
| `./core3 purgeinvalidcrc` | Remove objects with invalid template CRCs (see `purge-invalid-crc.md`) |
| `./core3 purgebuffs` | Remove orphaned buff objects from `buffs` database |
