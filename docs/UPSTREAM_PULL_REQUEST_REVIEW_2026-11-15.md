# Complete upstream review dossier — EVEmu Crucible maintenance and gameplay fork
**Prepared:** 15 November 2026  
**Upstream target:** [EvEmu-Project/evemu_Crucible](https://github.com/EvEmu-Project/evemu_Crucible) `master` (`f55b488a006e3090470d3b481aee47a1d7a0dade`)  
**Contributing code:** [xrizonx/evemu_Crucible](https://github.com/xrizonx/evemu_Crucible) `master` (`96450674fae13954e52009f88517a006d1c68a3b`)  
**GitHub comparison:** 410 commits ahead, zero behind, **137 files changed**, **+9,316/-1,748** textual lines, before this documentation-only submission commit.

> **Review/approval requested, not an auto-merge.** This cumulative fork covers build systems, core behavior, missions, experimental weapon mechanics and database research. CI is green; production compatibility and complete in-game behavior are not proven. Please review by subsystem and request smaller cherry-picked PRs if preferable.

## 1. Build modernization, release artifacts and Windows 10/11 support

**Code and workflow:** `CMakeLists.txt`, `cmake/FindUTF8CPP.cmake`, `Dockerfile`, `.dockerignore`, `.github/workflows/build-release.yml`, `src/eve-core/utils/CallStack.cpp`, `src/eve-common/EVEVersion.h`, `src/eve-server/imageserver/ImageServerConnection.cpp`.

- Updated compiler/CMake/dependency integration for current Linux GCC and MSYS2 Windows UCRT64 GCC. Fixed modern Boost.Asio, Boost.DateTime, MariaDB/MySQL (`my_bool` to `bool`), TinyXML/utf8cpp and Windows linker API compatibility.
- Corrected Windows linking of `ws2_32`, `mswsock` and DbgHelp, replacing incompatible legacy MinGW BFD stack access. Fixed a source-level global/static initialization exception caused by an empty `GIT_COMMIT_HASH` and out-of-bounds `std::string::erase` before `main()`.
- Added modern Linux, Windows, Docker CI plus CTest, installation, packaging and artifact checks. `ccache` accelerates repeat builds without weakening compile coverage.
- Windows UCRT64 release archives are portable for Windows 10/11; recursive dependency collection places runtime DLLs beside the installed executable and explicitly checks `libmariadb.dll`, `libtinyxml-0.dll`, `libz-1.dll`. Fixed prior wrong `C:\\mingw64` path and DLL-in-staging-root issues.
- Windows and Linux packages include patched `EVEDBTool` and matching `sql/base`, `sql/migrations` and dungeon assets. Docker and local build instructions revised.
- Existing `README.md` explains fresh setup and upgrades, with MariaDB, SQL tool usage and troubleshooting.

**Verified CI for the exact source head:** [GitHub Actions run 43742655629](https://github.com/xrizonx/evemu_Crucible/actions/runs/43742655629) — Linux GCC + registered CTest + package, Windows UCRT64 GCC + CTest + portable ZIP, and Docker build **all successful**. Do not equate these tests with a full live-server/gameplay QA pass.

## 2. Database, Python protocol, session safety and general crash fixes

- Fixed 10 search-category SQL result projections so `DBResultToIntIntDict` receives the two expected columns; bounds checks on `DBResultRow::GetText` and missing columns prevent invalid reads.
- Improved `Buffer` iterator compatibility and PyPacket/PyVisitor/PyExceptions validation; guarded DB result conversion, static initialization and format-string handling in admin/debug commands.
- Fixed `EntityList` probe iterator invalidation when a probe removes itself during processing.
- Corrected null module repairs, planet-colony container launch failures, service `fcvt` null-pointer formatting and station-hangar reload/respawn null paths.
- `BubbleManager::Remove` now untracks an entity only in its owning bubble, avoiding duplicate remove-ball announcements and a previous infinite loop when encountering null entries.
- `SystemBubble` guards duplicate registration, stale ownership, null members and repeated untracking. Fixed a reversed `%p`/`%s` debug printf mismatch which could crash with trace logging enabled.
- `SystemManager` now avoids dereferencing `end()` on a missing entity ID. Fixed a wormhole **type ID vs group ID** comparison in system dynamics loading.
- `DynamicEntityFactory::BuildEntity` logs skipped entity item/type/group/category/system instead of producing repeated unsymbolized Windows stack traces. This is a plausible explanation for the user's Maurasi startup stacks, **not a symbolically proven root cause**.
- `ImageServer` handles absent directory config, category-folder creation, duplicate uploads, empty portrait buffers, failed writes, 50px legacy requests and fallback to locally available 512px images; expected redirects no longer log as fatal errors.

**Upstream references:** [#139 turret crash](https://github.com/EvEmu-Project/evemu_Crucible/issues/139), [#170 probe iteration](https://github.com/EvEmu-Project/evemu_Crucible/issues/170), [#154 pod respawn](https://github.com/EvEmu-Project/evemu_Crucible/issues/154), [#188 hangar reload](https://github.com/EvEmu-Project/evemu_Crucible/issues/188), [#275 NPC belt spawn](https://github.com/EvEmu-Project/evemu_Crucible/issues/275), [#276 dungeon editor](https://github.com/EvEmu-Project/evemu_Crucible/issues/276), [#319 MySQL reconnect](https://github.com/EvEmu-Project/evemu_Crucible/issues/319). Several fixes are defensive/partial; please do not auto-close open issues without reproducing their original reports.

## 3. Navigation, warp tunnel preservation, collision and docking

- Pending dock/stargate actions can survive an active warp and resume on arrival with existing range/session checks; manual movement commands cancel superseded pending actions.
- Updated Crucible WARP ball serialization, sector/distance/sentinel snapshots and dynamic entity state while preserving the warp tunnel. This is a **narrow adaptation** of the relevant part of [upstream PR #332](https://github.com/EvEmu-Project/evemu_Crucible/pull/332), not an import of its full physics rewrite.
- Heading vectors are normalized after turns; `acos` input is clamped. Direct station warp-ins include the station and ship collision radii plus clearance.
- `Follow()` now clamps surface separation and avoids negative floating-distance conversion to large unsigned ranges.
- `Orbit()` uses waypoints from the current position and avoids absolute timestamp teleports, guards invalid radius/speed and includes target hull size.
- Player-ship bump checks subtract both radii, track contact across neighbors and avoid stale/null ship pointers.
- **Unfinished:** full physical collision handling against structures, asteroids and every object class. Station/warp visual and high-speed bounce behavior require client tests.

## 4. Drones: engagement, control range, orbit, return and targeting

- Improved RPC decoding and in-system owner/drone-ID recovery for `CmdEngage`, auto-acquisition of NPC targets, target-cap checks, stale lock cleanup and target-manager callback handling.
- Corrected DroneAI idle/approach/engage/combat/return-state logic, target lifetime and guard behavior; rejected commands do not accidentally cancel a drone returning to bay.
- Historical Crucible drone command range uses 20 km base + 5 km per `Scout Drone Operation` level + 3 km per `Electronic Warfare Drone Interfacing` level, with already-computed/fitted bonuses where available. Targets are measured from the controller ship.
- Already-locked targets remain pursuable beyond the initial acquisition envelope; temporary approach transitions no longer reset the firing cooldown. Sustained shots use the specific locked target; invalid/stale IDs recover or clear safely.
- Focus-fire and aggressive/passive setting changes only alter supplied fields and are reused for newly launched drones. Ownership and state notification pathways hardened.
- Ship-side launch/scoop/abandon/owner cleanup includes return-to-ship and existing 2,500 m automatic bay scoop path, cargo/bandwidth accounting and bay-capacity protections.
- Optimized per-drone target scans by using a read-only `GetEntitiesView()` rather than allocating a whole solar-system map copy per scan.
- **Unfinished:** drone ore mining and hauling. `CmdMine` and `CmdMineRepeatedly` remain stubs, `DroneAI::State::Mining` contains no extraction cycle, and mining drones no longer masquerade as combat drones.
- **Evidence/limit:** user reported light combat drones *initially* fired on NPCs before final range/firing changes; range bonuses, sustained fire and scoop behavior require new in-game confirmation.

## 5. Weapon mechanics: hybrid/projectile/laser turrets, NPCs, sentries and ships

- Shared historical turret hit formula for player/NPC/drone/sentry: relative **angular** velocity, tracking speed, signature resolution versus target signature, optimal range and falloff. Fixed radial motion being counted as transversal.
- Fixed artificial signature/hit chance shortcuts and automatic minimum damage on missed drone attacks. Single roll controls hit/miss and quality; the fork's configurable default wrecking probability is 1%.
- Fixed stationary sentry tracking where the sentry has no DestinyManager, preserved fractional damage multipliers and guarded missing target/attack state.
- Missing shield/armor/structure resonance attributes yield 1.0 (no resistance), while explicit 0 resonance stays immune; invalid 0/NaN capacitor or missing turret multiplier is guarded.
- Hardened damage target lifetime and linked-gun/missile bonus handling; changed repeated active-module cycles to decrement the shot count once **per completed firing cycle**, not every processing tick.
- **Review caveats:** linked turret volleys still share an aggregate quality roll, and exact module/rig/ship bonus stacking across every Crucible hull has not been exhaustively validated.

## 6. Missiles, smartbombs, defenders and bombs

- Missile damage uses explosion radius and velocity, target signature and target speed via shared guarded `EVE_WeaponFormulas.h`. Invalid or out-of-range damage factors do not produce phantom damage.
- Crucible `WarheadUpgrades` corrected to +2% damage/level rather than +5%; `GuidedMissilePrecision` scoped to guided light/heavy/cruise variants rather than unguided rockets/HAMs/torpedoes. Incorrect specialization warhead-damage bonus removed; correct launcher rate-of-fire interaction still needs review.
- Missile launcher grouping increases volley damage, not projectile flight lifetime. Defender range uses milliseconds-to-seconds conversion; Defender skill increases flight velocity; partial defender damage reduces remaining missile HP.
- Smartbomb AoE may damage missiles using live HP, not static charge inventory fields; keep a stable target-ID snapshot where AoE damage can delete system entities mid-iteration.
- NPC-launched missiles validate target registration, velocity and destiny state, and apply missile-specific damage configuration.
- Latest homing work updates projectile target position each tick and caps movement by remaining distance; impact requires actual proximity (rather than launch-time-only arrival timer). The simulation remains **discrete 1-second**, not continuous realistic physics.
- **Experimental:** Bomb AoE path does not yet reproduce every official Crucible timed area detonation / flight behavior; doomsday/superweapon activation is not wired and remains a stub.

## 7. Agent missions, encounter progression, and synthetic Blockade test

- Added foundation for security-agent offer selection, starter tier, persistence and agent-specific active-offer filtering. Adjusted low-level agent access/standings display.
- Encounter acceptance creates a warpable mission pocket beacon, spawns NPCs, tracks kills through existing combat callbacks, reports objectives to the client, gates the agent Complete action and uses normal wallet reward/cleanup mechanisms.
- Prefers compatible first-room NPC data from existing dungeon templates, falling back to faction/group NPCs where needed. This is **not** a complete legacy EVE mission engine (no full multi-room trigger/wave/gate data).
- Added **synthetic** Angel Cartel Blockade test mission ID `65535`: 24 frigates; required 24 kills; 2-hour accepted timer; configured 150,000 ISK completion reward. Corrected reward assignment order, persistence, objective serialization, final-spawn cleanup and briefing text/line-break display.
- User confirmed 24 NPCs spawned and fired in-game; final `24/24` journal completion, single payout, pocket cleanup and persistence after restart remain unverified.
- Do **not** treat the synthetic mission as the official Blockade mission or import its fabricated IDs/content into production mission datasets.

## 8. Database staging, static data provenance, market and EVEDBTool

- Added additive, research-only `rmrMissionStage*` tables under `sql/mission_data/001` and `003`, source manifest, Blockade research placeholder and read-only staging validation. This does **not** update active `agtMissions`, inventory, wallets or user data; it is not a production migration.
- Mission importer guidance requires a version-pinned CCP SDE or historical source compatible with this Crucible build and explicit NPC/type/dungeon mappings, without guessing ids, triggers, loot or mission rewards. Levels 1–4 full mission catalogue not implemented.
- Added patched EVEDBTool 0.0.6 market seed command (`tools/evedbtool/command_seed.go`); select market regions, station saturation, preview/dry run and force-based reseeding guards. Updated SQL seeds, scripts, YAML example and database guidance for Windows/Linux.
- **Safety:** never run `install` on an existing populated MariaDB database; back it up, run `status` first, and apply approved additive migrations only to an appropriate test clone initially.

## 9. Documentation and visual updates

- Expanded `README.md` for Windows 10/11, MariaDB, EVEDBTool, local build, Docker and upgrades; updated `sql/README.md` and included optional Crucible-*inspired* original sci-fi README banner artwork (not official CCP media).
- Added chronological `PATCH_NOTES.md`, `STABILITY_WORKLOG.md` and `ISSUE_TRIAGE.md` with upstream issue mapping; historical status/commit snapshots are intentionally left as history. The present dossier supersedes old "pending CI" statements for the current source code.
- Detailed subsystem guides: [weapon audit](CRUCIBLE_WEAPONS_AUDIT_2026-11-15.md), [drone range/sustained fire](DRONE_RANGE_AND_SUSTAINED_COMBAT_2026-11-15.md), [drone and movement](DRONE_AND_MOTION_AUDIT.md), [Maurasi log triage](SERVER_LOG_TRIAGE_2026-11-15.md), [mission staging](missions/STAGING_MIGRATION_GUIDE.md), [mission source migration](missions/MIGRATION_GUIDE.md).

## 10. Exact CI evidence, reproduction and review gates

**Automated, completed successfully on source code head `96450674f`:** Windows UCRT64 GCC build + CTest + portable artifact; Linux GCC build + CTest + artifact; Docker image build — [run #43742655629](https://github.com/xrizonx/evemu_Crucible/actions/runs/43742655629).

Registered deterministic regression tests include `WeaponFormulaTest` (turret/missile chance, velocity, miss/quality, resist, defender HP, flight steps, impact range and repeat cycle), `DroneControlRangeTest` (20/45/60 km and skill/bonus invalid cases) and `MovementFormulaTest` (surface separation and orbit safety). These cannot replace gameplay/DB testing.

**In-game checks requested before accepting the gameplay sections:**
1. Clean Windows install and login/undock without an MSYS2 toolchain, correct DLL resolution.
2. Dock/stargate queue during warp and after tunnel; check jump cancellation, no warp-sector mismatch, station arrival clearance.
3. Belt NPC spawn class, warp-in, target lock, turret combat, bounty/wreck and despawn across security ratings.
4. One and five combat drones: range boundary 20/35/45/60 km by skill, sustained 60s combat, multiple targets/focus fire, aggression toggles, return/scoop with empty/full bay.
5. Turret hit rate at optimal/falloff vs transversal, zero radial tracking penalty, NPC/sentry/drone hits and damage resonance/HP.
6. Guided/unguided missiles vs small/large and moving targets; missile max flight range, linked launchers, defenders, smartbomb AoE and repeat-cycle guns.
7. Synthetic Blockade: 24/24 completion, exactly one 150,000 ISK wallet journal payout, completed status and pocket cleanup, server restart preservation.
8. Test-only staging SQL with a backed-up disposable MariaDB clone and read-only validation; do not auto-apply research schema to production.
9. Maurasi startup trace regression and character 50px portrait/cache behavior.
10. Historical Crucible ship, charge, skill and faction data compatibility across multiple hull sizes.

**Explicit limitations not represented as finished:** full mining drone extraction/ore haul; faithful bomb timer; doomsday class activation; real physics collision solver; all EVE mission variants/rooms/gates; all modern/static-data balance and module interactions. The user has **not** reported complete in-game validation of these systems.

## 11. Review and merge strategy

This contribution bundles **410 historical fork commits** in the code base and initially touches **137 paths** before adding this guide. It is intentionally submitted for maintainer approval as a **consolidated, highly reviewable inventory**, not as a demand for a one-click merge. Suggested acceptance sequence:
1. Build/CI/Windows portability and EVEDBTool packaging.
2. Crash, memory, DB row and image server guards.
3. Warp/bubble/ship movement with client regression checks.
4. NPC, drone/target manager and weapon formulas with Crucible static data checks.
5. Experimental mission workflow, synthetic Blockade and research SQL after separate maintainer sign-off.
6. Cosmetic README graphics, which can be dropped without impacting runtime behavior.

Upstream already has [PR #327](https://github.com/EvEmu-Project/evemu_Crucible/pull/327) for belt NPC spawn changes and historical [PR #332](https://github.com/EvEmu-Project/evemu_Crucible/pull/332) for broader warp work; avoid duplicate application without comparing diffs.

## 12. Full original changed-file inventory

Below are all **137** files in the direct upstream `master` -> fork `master` compare, before adding this review guide to the submission branch. Binary README images show zero textual lines, not zero bytes.

| File | Status | + | - |
|---|---|---:|---:|

| `.dockerignore` | modified | 25 | 9 |
| `.github/workflows/build-release.yml` | added | 301 | 0 |
| `.github/workflows/cmake.yml` | removed | 0 | 33 |
| `CMakeLists.txt` | modified | 29 | 9 |
| `Dockerfile` | modified | 22 | 11 |
| `ISSUE_TRIAGE.md` | added | 380 | 0 |
| `PATCH_NOTES.md` | added | 308 | 0 |
| `README.md` | modified | 383 | 67 |
| `STABILITY_WORKLOG.md` | added | 217 | 0 |
| `cmake/FindUTF8CPP.cmake` | modified | 1 | 0 |
| `docs/CRUCIBLE_WEAPONS_AUDIT_2026-11-15.md` | added | 125 | 0 |
| `docs/DRONE_AND_MOTION_AUDIT.md` | added | 76 | 0 |
| `docs/DRONE_RANGE_AND_SUSTAINED_COMBAT_2026-11-15.md` | added | 46 | 0 |
| `docs/SERVER_LOG_TRIAGE_2026-11-15.md` | added | 86 | 0 |
| `docs/assets/readme/database-tools-banner.jpg` | added | 0 | 0 |
| `docs/assets/readme/hero-banner.jpg` | added | 0 | 0 |
| `docs/assets/readme/windows-setup-banner.jpg` | added | 0 | 0 |
| `docs/missions/BLOCKADE_TEST_MODE.md` | added | 13 | 0 |
| `docs/missions/MIGRATION_GUIDE.md` | added | 68 | 0 |
| `docs/missions/STAGING_MIGRATION_GUIDE.md` | added | 77 | 0 |
| `docs/missions/STAGING_VALIDATION.sql` | added | 71 | 0 |
| `sql/README.md` | modified | 86 | 38 |
| `sql/evedb.yaml.example` | added | 14 | 0 |
| `sql/get_evedbtool.bat` | modified | 48 | 18 |
| `sql/get_evedbtool.sh` | modified | 31 | 29 |
| `sql/mission_data/001_create_staging_schema.sql` | added | 110 | 0 |
| `sql/mission_data/002_seed_research_case.sql` | added | 55 | 0 |
| `sql/mission_data/003_upgrade_existing_staging_schema.sql` | added | 756 | 0 |
| `sql/mission_data/research/the_blockade_level1_angel_cartel.json` | added | 189 | 0 |
| `sql/mission_data/source_manifest.json` | added | 70 | 0 |
| `sql/seed_and_clean/seed_market.sql` | modified | 44 | 4 |
| `sql/seed_and_clean/seed_special_modules.sql` | added | 85 | 0 |
| `sql/seed_and_clean/seed_station_market.sql` | modified | 4 | 1 |
| `src/eve-common/EVEVersion.h` | modified | 4 | 1 |
| `src/eve-common/EVE_DroneControlRange.h` | added | 27 | 0 |
| `src/eve-common/EVE_Missions.h` | modified | 12 | 0 |
| `src/eve-common/EVE_MovementFormulas.h` | added | 32 | 0 |
| `src/eve-common/EVE_WeaponFormulas.h` | added | 142 | 0 |
| `src/eve-common/database/EVEDBUtils.cpp` | modified | 4 | 0 |
| `src/eve-common/destiny/DestinyBinDump.cpp` | modified | 1 | 1 |
| `src/eve-common/destiny/DestinyStructs.h` | modified | 6 | 5 |
| `src/eve-common/network/EVETCPConnection.cpp` | modified | 13 | 7 |
| `src/eve-common/python/PyPacket.cpp` | modified | 3 | 3 |
| `src/eve-common/python/PyVisitor.cpp` | modified | 4 | 3 |
| `src/eve-common/python/classes/PyExceptions.cpp` | modified | 7 | 6 |
| `src/eve-core/database/dbcore.cpp` | modified | 2 | 2 |
| `src/eve-core/database/dbcore.h` | modified | 5 | 1 |
| `src/eve-core/utils/Buffer.h` | modified | 20 | 27 |
| `src/eve-core/utils/CallStack.cpp` | modified | 2 | 104 |
| `src/eve-core/utils/misc.cpp` | modified | 6 | 2 |
| `src/eve-core/utils/utils_string.cpp` | modified | 1 | 1 |
| `src/eve-core/utils/utils_time.cpp` | modified | 1 | 0 |
| `src/eve-server/Client.cpp` | modified | 336 | 36 |
| `src/eve-server/Client.h` | modified | 9 | 0 |
| `src/eve-server/ConsoleCommands.cpp` | modified | 16 | 4 |
| `src/eve-server/EVEServerConfig.cpp` | modified | 5 | 5 |
| `src/eve-server/EntityList.cpp` | modified | 32 | 10 |
| `src/eve-server/Profiler.cpp` | modified | 8 | 8 |
| `src/eve-server/ServiceDB.cpp` | modified | 10 | 2 |
| `src/eve-server/admin/DevToolsProviderService.cpp` | modified | 22 | 11 |
| `src/eve-server/admin/DungeonCommands.cpp` | modified | 1 | 1 |
| `src/eve-server/admin/GMCommands.cpp` | modified | 8 | 4 |
| `src/eve-server/admin/GMCommands.h` | modified | 1 | 1 |
| `src/eve-server/admin/SystemCommands.cpp` | modified | 1 | 1 |
| `src/eve-server/admin/debug_commands.cpp` | modified | 4 | 5 |
| `src/eve-server/agents/Agent.cpp` | modified | 132 | 78 |
| `src/eve-server/agents/Agent.h` | modified | 1 | 1 |
| `src/eve-server/agents/AgentBound.cpp` | modified | 301 | 45 |
| `src/eve-server/agents/AgentBound.h` | modified | 2 | 2 |
| `src/eve-server/character/Character.cpp` | modified | 16 | 4 |
| `src/eve-server/character/CharacterDB.cpp` | modified | 1 | 1 |
| `src/eve-server/chat/LSCService.cpp` | modified | 3 | 3 |
| `src/eve-server/dungeon/DungeonDB.cpp` | modified | 32 | 12 |
| `src/eve-server/dungeon/DungeonDB.h` | modified | 1 | 0 |
| `src/eve-server/dungeon/DungeonService.cpp` | modified | 2 | 1 |
| `src/eve-server/dungeon/DungeonService.h` | modified | 1 | 1 |
| `src/eve-server/effects/EffectsProcessor.cpp` | modified | 32 | 13 |
| `src/eve-server/imageserver/ImageServer.cpp` | modified | 64 | 60 |
| `src/eve-server/imageserver/ImageServerConnection.cpp` | modified | 7 | 5 |
| `src/eve-server/imageserver/ImageServerConnection.h` | modified | 4 | 4 |
| `src/eve-server/inventory/Inventory.cpp` | modified | 3 | 1 |
| `src/eve-server/market/MarketDB.cpp` | modified | 36 | 15 |
| `src/eve-server/missions/MissionDB.cpp` | modified | 61 | 3 |
| `src/eve-server/missions/MissionDB.h` | modified | 3 | 0 |
| `src/eve-server/missions/MissionDataMgr.cpp` | modified | 176 | 6 |
| `src/eve-server/missions/MissionDataMgr.h` | modified | 2 | 1 |
| `src/eve-server/npc/Drone.cpp` | modified | 229 | 175 |
| `src/eve-server/npc/Drone.h` | modified | 9 | 7 |
| `src/eve-server/npc/DroneAI.cpp` | modified | 535 | 66 |
| `src/eve-server/npc/DroneAI.h` | modified | 10 | 1 |
| `src/eve-server/npc/EntityService.cpp` | modified | 221 | 24 |
| `src/eve-server/npc/EntityService.h` | modified | 1 | 1 |
| `src/eve-server/npc/NPC.cpp` | modified | 24 | 18 |
| `src/eve-server/npc/NPCAI.cpp` | modified | 24 | 8 |
| `src/eve-server/npc/NPCAI.h` | modified | 2 | 2 |
| `src/eve-server/npc/SentryAI.cpp` | modified | 18 | 2 |
| `src/eve-server/npc/SentryAI.h` | modified | 1 | 1 |
| `src/eve-server/planet/Colony.cpp` | modified | 2 | 2 |
| `src/eve-server/search/SearchDB.cpp` | modified | 10 | 10 |
| `src/eve-server/services/BoundService.h` | modified | 1 | 1 |
| `src/eve-server/ship/BeyonceService.cpp` | modified | 39 | 19 |
| `src/eve-server/ship/Missile.cpp` | modified | 185 | 53 |
| `src/eve-server/ship/Missile.h` | modified | 8 | 1 |
| `src/eve-server/ship/Ship.cpp` | modified | 76 | 49 |
| `src/eve-server/ship/ShipService.cpp` | modified | 106 | 62 |
| `src/eve-server/ship/modules/ActiveModule.cpp` | modified | 213 | 41 |
| `src/eve-server/ship/modules/ActiveModule.h` | modified | 3 | 1 |
| `src/eve-server/ship/modules/GenericModule.cpp` | modified | 6 | 0 |
| `src/eve-server/ship/modules/MiningLaser.cpp` | modified | 25 | 2 |
| `src/eve-server/ship/modules/ModuleManager.cpp` | modified | 32 | 14 |
| `src/eve-server/ship/modules/TurretFormulas.cpp` | modified | 134 | 115 |
| `src/eve-server/ship/modules/TurretModule.cpp` | modified | 36 | 13 |
| `src/eve-server/system/BubbleManager.cpp` | modified | 13 | 33 |
| `src/eve-server/system/Damage.cpp` | modified | 35 | 15 |
| `src/eve-server/system/Damage.h` | modified | 6 | 4 |
| `src/eve-server/system/DestinyManager.cpp` | modified | 278 | 95 |
| `src/eve-server/system/DestinyManager.h` | modified | 5 | 1 |
| `src/eve-server/system/SystemBubble.cpp` | modified | 46 | 8 |
| `src/eve-server/system/SystemBubble.h` | modified | 32 | 0 |
| `src/eve-server/system/SystemEntity.cpp` | modified | 13 | 6 |
| `src/eve-server/system/SystemEntity.h` | modified | 1 | 0 |
| `src/eve-server/system/SystemManager.cpp` | modified | 53 | 32 |
| `src/eve-server/system/SystemManager.h` | modified | 8 | 2 |
| `src/eve-server/system/TargetManager.cpp` | modified | 307 | 72 |
| `src/eve-server/system/TargetManager.h` | modified | 8 | 4 |
| `src/eve-server/system/cosmicMgrs/AnomalyMgr.cpp` | modified | 5 | 0 |
| `src/eve-server/system/cosmicMgrs/DungeonMgr.cpp` | modified | 184 | 7 |
| `src/eve-server/system/cosmicMgrs/DungeonMgr.h` | modified | 2 | 0 |
| `src/eve-server/system/cosmicMgrs/SpawnMgr.cpp` | modified | 460 | 26 |
| `src/eve-server/system/cosmicMgrs/SpawnMgr.h` | modified | 5 | 3 |
| `src/eve-test/CMakeLists.txt` | modified | 17 | 4 |
| `src/eve-test/ProfilerTestStub.cpp` | added | 10 | 0 |
| `src/eve-test/utils/DroneControlRangeTest.cpp` | added | 33 | 0 |
| `src/eve-test/utils/MovementFormulaTest.cpp` | added | 36 | 0 |
| `src/eve-test/utils/WeaponFormulaTest.cpp` | added | 80 | 0 |
| `tools/evedbtool/command_seed.go` | added | 379 | 0 |
| `utils/config/eve-server.xml` | modified | 5 | 22 |
