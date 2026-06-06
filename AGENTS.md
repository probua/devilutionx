# AGENTS.md

## Project

DevilutionX — a source port of Diablo and Hellfire. C++ game engine with CMake build system.

This is a **fork with a mod**: reduces Diablo from 16 to 7 dungeon levels, XP x10, shared XP, plus spell changes (Healing+HealOther merge, Telekinesis rework, new Raise Skeleton spell). Diablo only (no Hellfire). See `docs/mod-overview.md` for the full mod specification.

## Build

```sh
# Debug (convenience script)
./build-debug.sh

# Release (convenience script, builds Linux + Windows via Docker)
./build-release.sh

# Manual build
cmake -S. -Bbuild -DCMAKE_BUILD_TYPE=Release
cmake --build build -j $(nproc)
```

Out-of-source build is required — never build in the source root.

Key CMake options:
- `-DBUILD_TESTING=OFF` — disable tests (default ON)
- `-DNONET=ON` — disable networking (also disables exceptions and RTTI)
- `-DASAN=ON` / `-DUBSAN=ON` — enable sanitizers (debug only)
- `-DUSE_SDL1=ON` — build against SDL 1.2 instead of SDL 2

## Test

```sh
# Build with tests enabled (default)
cmake -S. -Bbuild
cmake --build build -j $(nproc)
cd build && ctest --output-on-failure

# Run a single test binary directly
./build/drlg_l1_test
```

Tests use Google Test. Each `test/*_test.cpp` compiles to its own executable linked against `test_main` (static) and `libdevilutionx_so` (shared). Test fixtures live in `test/fixtures/`.

Some tests require `spawn.mpq` in the build directory. CI downloads it from `https://github.com/diasurgical/devilutionx-assets/releases/download/v2/spawn.mpq`.

Tests run in headless mode — `devilution::HeadlessMode = true` is set in `test/main.cpp`.

## Formatting and Linting

```sh
# Check formatting (CI uses clang-format 18)
clang-format --dry-run --Werror -i Source/*.cpp Source/*.h Source/**/*.cpp Source/**/*.h
clang-format --dry-run --Werror -i test/*.cpp
```

- Style: WebKit-based, tabs for indentation, 4-column tab width
- `.clang-format` files in both `Source/` and `test/`
- CI workflow: `clang-format-check.yml` uses clang-format **18**

```sh
# clang-tidy
run-clang-tidy -p build 'Source.*'
```

Configured in `Source/.clang-tidy`.

## Code Style Conventions

C++ standard is set to 20 but only C++17 features are used (oldest supported compiler: GCC 6.5).

Naming (from `.clang-tidy`):
- Classes/Structs/Enums/Template params: `CamelCase`
- Free functions: `CamelCase`
- Methods/Parameters/Variables: `camelBack`
- Member variables (class/struct): `lower_case` with `_` suffix (e.g. `health_`)
- Global variables/functions: `aNy_CasE` (legacy)
- Constants (`constexpr`, `static const`): `CamelCase`

Tabs for C/C++ indentation (not spaces). CRLF line endings for C/C++ files. See `.editorconfig`.

## Mod Documentation (docs/)

This fork has extensive mod-specific docs. Essential reading before changing game logic:

- `docs/mod-overview.md` — full mod spec: 7-level layout, quest mapping, virtual level system, all hotfixes applied
- `docs/mod-session-reference.md` — **complete reference for new sessions**: level map, modified files, quest flow (Leoric/Lazarus set levels), `_qvar1`/`_qvar2` states, spell changes, pending work
- `docs/mod-experience-changes.md` — XP x10 multiplier and shared XP implementation
- `docs/mod-fix-flash-spell.md` — Flash spell bug fix (upstream devilutionX bug)
- `docs/spells/` — detailed spell change specs (healing merge, telekinesis rework, raise skeleton, spell tiers)
- `docs/mod-etapa-*.md` — per-stage implementation notes (levels, warps, monsters, quests, items, dungeons, polish)

## Mod Architecture Notes

The mod uses a **virtual level mapping** system — this is the central concept:

- `LevelMap = { 0, 1, 3, 5, 7, 9, 13, 16 }` maps mod levels 1–7 to original Diablo levels
- `GetVirtualLevel()` in `Source/levels/gendung.h` — used everywhere that needs original-level scaling (damage, items, traps, shops)
- `GetVirtualLevelForShop()` — variant for shop item generation
- `NUMLEVELS` stays at 25 (Hellfire buffer safety), `giNumberOfLevels = 8` (7 + town)
- Hellfire code is **not removed**, just never activated (`gbIsHellfire == false`)

Key mod-modified files (see `docs/mod-session-reference.md` for the full table):
- `Source/levels/gendung.h` / `.cpp` — virtual level mapping, `GetLevelType()`
- `Source/diablo.cpp` — `giNumberOfLevels = 8`, cutscenes
- `Source/quests.cpp` — 7 active quests, 9 forced to `QUEST_NOTAVAIL`, set level logic for Leoric/Lazarus
- `Source/levels/drlg_l*.cpp` — dungeon generation adapted for 7 levels
- `Source/player.cpp` — XP x10 (`ExperienceMultiplier`), shared XP (`gbActivePlayers`), `MaxSpellLevel = 4`
- `Source/missiles.cpp` — 15 damage formulas using `GetVirtualLevel()`, new `AddSkeleton()`
- `Source/items.cpp` — `ItemsGetCurrlevel()` with virtual mapping, spell book/staff fixes
- `Source/monster.cpp` — 93 monsters remapped in `monstdat.cpp`, skeleton summon system, minion leash AI
- `Source/automap.cpp` — minion (golem/skeleton) visible on automap as green arrow
- `Source/spelldat.cpp` — new `SpellID::Skeleton=37`, spell tier data, `MAX_SPELLS=53`

## Architecture

- **Entry point**: `Source/main.cpp` → `devilution::DiabloMain()` in `Source/diablo.cpp`
- **Main library**: `libdevilutionx` (OBJECT library), defined in `Source/CMakeLists.txt`
- **Executable**: `devilutionx`, links `libdevilutionx`
- **Namespace**: `devilution`
- **Key subdirectories under `Source/`**:
  - `engine/` — rendering, audio, assets, random, events
  - `levels/` — dungeon generation (`drlg_l1`–`drlg_l4`), town, crypt
  - `controls/` — input handling (keyboard, gamepad, touch)
  - `DiabloUI/` — menus, hero selection, dialogs
  - `dvlnet/` — networking (TCP, ZeroTier, loopback)
  - `qol/` — quality-of-life features (stash, autopickup, item labels)
  - `mpq/` — MPQ archive reader/writer (conditionally compiled)
  - `storm/` — Storm* compatibility layer
  - `utils/` — string helpers, file I/O, SDL utilities
- **3rdParty/** — vendored dependencies (SDL2, fmt, libsodium, etc.), can use system versions via `DEVILUTIONX_SYSTEM_*` flags
- **CMake/** — build system modules, platform toolchains in `CMake/platforms/`
- **Translations/** — `.po` files, compiled to `.gmo` at build time
- **Packaging/** — platform-specific packaging scripts and resources
