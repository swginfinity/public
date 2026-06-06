# w100 Branch — Clang 19 Build Instructions

This branch is a copy of `infinity_jtl` with clang-19 compatibility fixes applied.
It targets the `infinity2.0.0` repo, branch `w100`.

## What Changed (vs infinity_jtl)

All changes are clang-19 compatibility fixes. No game logic changed.

### Build System
- `CMakeLists.txt` — Quoted `${CMAKE_BUILD_TYPE}` in 4 if-statements (cmake correctness)
- `CMakeLists.txt` — Changed `CMAKE_CXX_STANDARD` from 14 to 17 (fixes AVX-512 alignment crash, see below)
- `src/CMakeLists.txt` — Added `-Wno-ambiguous-reversed-operator` for IDL-generated code

### Engine3 (3 lines)
- `String.h` — `operator ""_hashCode` → `operator""_hashCode` (remove space)
- `json.hpp` — Same fix for `_json` and `_json_pointer` literals

### Source Code
- `ObjectManager.h` — Added `override` keyword to 4 virtual method declarations
- `SquadLeaderBuff.idl` — `if (player != null)` → `if (player)` (2 locations)
- `AuctionManagerImplementation.cpp` — `(int)MAXVENDORPRICE` casts (2 locations)
- `DwDoorBusterCommand.h` — `(int)MIN_COOLDOWN_MS` cast
- `DetourNavMesh.cpp` — `(void*)` cast on memset arguments
- `dbconvert.cpp` — Fixed `decompress()` return: `LocalDatabase::uncompress()` returns `void`, not `bool`

### AVX-512 Alignment Crash (C++14 → C++17)

The most critical fix is the C++17 upgrade. On AVX-512 capable CPUs, clang-19 with
`-march=native` generates `vmovaps %zmm0,(%rax)` (64-byte aligned store) when zeroing
boost lockfree queue nodes, which have `BOOST_ALIGNMENT(64)`. In C++14 mode,
`std::allocator::allocate()` uses plain `operator new` which only guarantees 16-byte
alignment. The misaligned AVX-512 store causes SIGSEGV during zone startup
(`PlanetManager::buildRegionNavAreas` → `PendingTasksMap` constructor).

C++17 fixes this because `std::allocator::allocate()` properly uses aligned `operator new`
for over-aligned types. GCC is not affected because it generates `rep stosq` or unaligned
stores for the same memset.

---

## Step-by-Step Build

### Prerequisites
- Ubuntu 24.04 (or compatible)
- clang 19 installed (`clang++-19` in PATH, or symlinked to `clang++`)
- All standard SWGEmu dependencies (libdb5.3-dev, liblua5.3-dev, libmysqlclient-dev, etc.)

### 1. Clone or update the repo

```bash
cd /home/swgemu/workspace
git clone git@github.com:swginfinity/infinity2.0.0.git infinity2.0.0_w100
cd infinity2.0.0_w100
git checkout w100
git pull origin w100
```

Or if the directory already exists:
```bash
cd /home/swgemu/workspace/infinity2.0.0_w100
git checkout w100
git pull origin w100
```

### 2. Initialize engine3 submodule

```bash
cd MMOCoreORB
git submodule update --init --recursive
```

**NOTE:** The engine3 submodule has 3 literal spacing fixes applied directly to its files.
If `git submodule update` overwrites them, re-apply:
```bash
cd utils/engine3
# Check if the fix is present:
grep 'operator""_hashCode' MMOEngine/src/system/lang/String.h
# If it shows 'operator ""_hashCode' (with a space), the fixes were overwritten.
# In that case, check out the w100 versions:
cd ../..
git checkout -- utils/engine3/
```

### 3. Create the build directory

```bash
mkdir -p build
cd build
```

### 4. Run cmake

```bash
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo \
      -DCMAKE_C_COMPILER=clang \
      -DCMAKE_CXX_COMPILER=clang++ \
      -DENABLE_ERROR_ON_WARNINGS=OFF \
      ..
```

**Important flags:**
- `ENABLE_ERROR_ON_WARNINGS=OFF` — Recommended for first build. Once it compiles clean,
  you can re-enable to catch new warnings.
- If clang 19 is installed as `clang-19`/`clang++-19`, use those paths explicitly.

### 5. Build

```bash
make -j24
```

The binary lands in `../bin/core3`.

### 6. Build dbconvert (optional — for database migration)

```bash
make dbconvert -j24
```

The binary lands in `../bin/dbconvert`.

---

## Testing dbconvert with 2.0.0 Databases

### 1. Set up database folders

```bash
cd /home/swgemu/workspace/infinity2.0.0_w100/MMOCoreORB/bin

# Copy clean 2.0.0 databases as the pristine source
cp -r /path/to/2.0.0/databases databases_clean

# Create empty target (or copy clean set to start fresh)
cp -r databases_clean databases
```

### 2. Set up config

Make sure `conf/config.lua` and `conf/config-local.lua` exist and have correct
BDB settings. Key settings for large databases:

```lua
-- conf/config-local.lua
Core3.BerkeleyDB = {
    envMaxLocks = 10000000,
    envMaxLockers = 10000000,
    envMaxLockObjects = 10000000,
}
```

### 3. Run conversion

```bash
cd /home/swgemu/workspace/infinity2.0.0_w100/MMOCoreORB/bin

# Test on a small database first
./dbconvert sample sceneobjects

# If that works, convert all
./dbconvert all
```

### 4. If conversion fails

```bash
# Wipe and restore from clean copy
rm -rf databases/
cp -r databases_clean databases

# Fix the issue, rebuild, try again
cd ../build && make dbconvert -j24
cd ../bin && ./dbconvert all
```

---

## Creating the PR

After confirming the build succeeds:

```bash
cd /home/swgemu/workspace/infinity2.0.0_w100
git add -A
git commit -m "w100: jtl codebase with clang-19 compatibility fixes"
git push origin w100
```

Then create PR from `w100` → `test` on GitHub.

---

## Summary of Changes (for PR description)

```
Replaced w100 branch content with current infinity_jtl codebase,
then applied minimal clang-19 compatibility fixes:

Build:
- Quote CMAKE_BUILD_TYPE variables (cmake correctness)
- Add -Wno-ambiguous-reversed-operator for IDL-generated code
- C++14 → C++17 (fixes AVX-512 aligned store crash in boost lockfree)

Engine3:
- Remove space in user-defined literal operators (clang 19/20 requirement)

Source:
- Add override keywords to ObjectManager.h virtual methods
- Fix null comparison in SquadLeaderBuff.idl (null → boolean check)
- Add (int) casts for static const int in Math::min/max calls
- Add (void*) cast for memset in DetourNavMesh.cpp
- Fix decompress() void/bool return type in dbconvert.cpp
```
