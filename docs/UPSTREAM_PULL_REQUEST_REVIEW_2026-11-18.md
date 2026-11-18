# EVEmu Crucible upstream draft #335 — 18 November 2026 update

## Addendum — 18 November 2026: 100M NPC ore/mineral stock

**New work after the documented fork-master import:** NPC buy and sell orders for minerals (group 18) and ore/ice groups 450–462, 465, 467–469 now start at **100,000,000 units per type and station** when created by EVEDBTool or Trader Joe. All other quantity categories and 100%-reference pricing remain unchanged; ore/mineral bids have a conservative 10M ISK/unit ceiling to prevent unsafe pseudo-escrow. See [100M stock deployment guide](NPC_ORE_MINERAL_100M_ORDERS_2026-11-18.md) for exact groups, refresh commands and MariaDB CI coverage.

This additional code is on PR #335's contribution branch **after** the 50-master-commit/89-path snapshot described below. It is not in fork `master`, does not automatically overwrite existing market orders, and still requires CI and client QA.


> **Maintainer review only.** This extends the archived [15 November](UPSTREAM_PULL_REQUEST_REVIEW_2026-11-15.md) and [17 November](UPSTREAM_PULL_REQUEST_REVIEW_2026-11-17.md) handoffs; the original [15 November notes](UPSTREAM_PATCH_NOTES_HISTORY_2026-11-15.md) remain preserved.

## Source and preservation

- Source: `xrizonx/evemu_Crucible:master` [`2d5cb4d2c7dbd0b33b70467e5caf3021fc52a713`](https://github.com/xrizonx/evemu_Crucible/commit/2d5cb4d2c7dbd0b33b70467e5caf3021fc52a713).
- Destination: `xrizonx:contrib/upstream-crucible-maintenance-2026-11-15`, source of [upstream draft PR #335](https://github.com/EvEmu-Project/evemu_Crucible/pull/335), targeting `EvEmu-Project/evemu_Crucible:master`.
- Before sync, branch head `c876ce612173d15c71774d05393435d2ed4248ca` had **five branch-only commits** while fork `master` had **50 newer commits / 89 changed files**. Merge preserves both ancestries, the three historical branch-only docs and their unique patch-note paragraph.
- The 89 paths listed below are the **new fork master delta since the previous contribution branch snapshot**, **not** the full upstream PR scope. For complete dated changes through this snapshot consult [PATCH_NOTES.md](../PATCH_NOTES.md). Upstream maintainers should review the *live PR diff* for authoritative cumulative statistics.

## Patch notes — most recent source changes

### Infrastructure Hub development and Sovereignty (18 November)
- `/sovindex5` and `reset`: restricted DEV/ADMIN testing with TCU/IHub-claim-bound Military/Industry points and 100 effective Strategic days, without rewriting claimTime. Migrations widen activity point storage from legacy 16-bit values to signed 32-bit client-compatible values (test values: **1,000,000,000 Military** and **30,000,000 Industrial**).
- IHub installation now covers the Strategic (1016), Military (1021) and Industrial (1020) upgrade families with prerequisite/blocking validation, active flag, duplicate and holding-alliance permission checks. **Supercapital Construction Facilities (type 2009)** checks 100 effective Strategic days. Real manufacturing operation remains incomplete.
- Upgraded systems gain initial Military combat anomaly and Industrial gravimetric site scheduling (up to five of each) using existing dungeon, anomaly and ore generation. This is baseline site availability, **not** full CCP-tier encounter rules, upgrade billing, real activity progression/decay, instant refresh or manufacturing jobs.
- Relevant migrations: `20261118070000-sov-development-index-overrides.sql`, `20261118093000-sov-development-index-point-width.sql`, `20261118120000-pos-industry-lines.sql`, `20261118123000-crucible-static-attribute-corrections.sql`, `20261118130000-crucible-capital-warp-speeds.sql`. Stage in order, back up first; this Git sync does not run SQL.

### POS, towers and admin safety (17–18 November)
- Control-tower force-field bubble now addresses password/harmonic persistence, valid radius (Large Control Tower fallback) and visible field packets. Strengthened POS structure teardown, reciprocal jump-bridge links, SBU target anchoring distance, sovereignty claimTime encoding, SBU gate counting and alliance authorization.
- `/dogma` grants only CONTENT/DEV and ADMIN access while keeping unrelated administrator commands restricted. Corrected permission-mask logging and unsafe missing-attribute paths.
- Orphaned POS structure rows were identified via **read-only** diagnostics; they require independent backup/reference review and must **not** be deleted automatically.

### Scanning, combat, missions and market (15–18 November)
- Scanner sessions rebind after system jumps; directional scan cone handling and probe bookkeeping are hardened. A possible post-NPC-death scanner crash remains unproven resolved.
- Preserve earlier navigation, Warp/Orbit/Follow, weapon range, warp speed, NPC AI, drone targeting and server crash-safety fixes. Client physics/movement behavior still needs end-to-end testing.
- Distinct Security Agent **Blockade #65535** (12 NPCs / **50M ISK**) and **DED & Officer Expedition #65534** (8 NPCs / **500M ISK** once at turn-in) remain separate. Elite wrecks use SDE faction/officer/deadspace module categories; **True Sansha** types are filtered from the elite mission only.
- Preserve 100% `invTypes.basePrice` NPC market sell/buy seed policy, MariaDB unsigned `orderRange=0` workaround, inventory/container protections, station agent/office hardening and Crucible-compatible image-server behavior.
- **Not included:** open fork [#41](https://github.com/xrizonx/evemu_Crucible/pull/41) Domination Grigori filter and draft fork [#42](https://github.com/xrizonx/evemu_Crucible/pull/42) mining-drone flight — both unmerged at the captured master SHA.

### CI, reproducibility and operational boundaries
- [Fork master Actions #44346201327](https://github.com/xrizonx/evemu_Crucible/actions/runs/44346201327) **passed at the exact source SHA above**, including the configured platform/test jobs. Earlier expedition pipeline [#44142537989](https://github.com/xrizonx/evemu_Crucible/actions/runs/44142537989) also passed Windows/Linux/Docker/MariaDB.
- Green CI is **not** proof of live 2012-client behavior or compatibility of every upstream service. New contribution-branch/PR CI, upgrade installation, shield visualization, missions, scanning, POS, manufacturing and migration staging remain reviewer/operator checks.
- No production SQL, EVEDBTool `seed -force`, player-data changes, or upstream merges are part of the source sync. Large cumulative upstream PR may benefit from subsystem-specific cherry-pick reviews.

## Source file inventory: 50 newer fork commits, 89 changed paths

### Build, tests and documents (20)
- `.github/workflows/build-release.yml`
- `PATCH_NOTES.md`
- `docs/CRUCIBLE_1_6_STATIC_ATTRIBUTE_AUDIT.md`
- `docs/CRUCIBLE_RANGE_AND_WARP_AUDIT_2026-11-17.md`
- `docs/SOVEREIGNTY_POS_REVIEW_2026-11-17.md`
- `docs/SOV_INDEX_LEVEL5_GM_TEST_2026-11-18.md`
- `docs/UPSTREAM_ISSUE_AUDIT_2026-11-17.md`
- `src/eve-test/CMakeLists.txt`
- `src/eve-test/utils/CommandPermissionsTest.cpp`
- `src/eve-test/utils/IHubUpgradeRulesTest.cpp`
- `src/eve-test/utils/MiningFormulaTest.cpp`
- `src/eve-test/utils/MovementFormulaTest.cpp`
- `src/eve-test/utils/ScanDirectionTest.cpp`
- `src/eve-test/utils/SovIndexOverrideTest.cpp`
- `src/eve-test/utils/SovereigntyPosIntegrityTest.cpp`
- `src/eve-test/utils/TowerShieldRulesTest.cpp`
- `tools/tests/verify_pos_industry_lines.py`
- `tools/verify_crucible_range_and_warp.py`
- `tools/verify_sov_pos_integrity.py`
- `tools/verify_subsystems_audit.py`

### SQL and migrations (6)
- `sql/diagnostics/2026-11-17-sov-pos-readonly-audit.sql`
- `sql/migrations/20261118070000-sov-development-index-overrides.sql`
- `sql/migrations/20261118093000-sov-development-index-point-width.sql`
- `sql/migrations/20261118120000-pos-industry-lines.sql`
- `sql/migrations/20261118123000-crucible-static-attribute-corrections.sql`
- `sql/migrations/20261118130000-crucible-capital-warp-speeds.sql`

### Shared definitions and math (11)
- `src/eve-common/EVE_CommandPermissions.h`
- `src/eve-common/EVE_IHubUpgradeRules.h`
- `src/eve-common/EVE_MiningFormulas.h`
- `src/eve-common/EVE_MovementFormulas.h`
- `src/eve-common/EVE_RAM.h`
- `src/eve-common/EVE_ScanningMath.h`
- `src/eve-common/EVE_SovIndexOverride.h`
- `src/eve-common/EVE_SovPosIntegrity.h`
- `src/eve-common/EVE_TowerShieldRules.h`
- `src/eve-common/destiny/DestinyBinDump.cpp`
- `src/eve-common/destiny/DestinyStructs.h`

### Admin and scan services (13)
- `src/eve-server/admin/AllCommands.cpp`
- `src/eve-server/admin/AllCommands.h`
- `src/eve-server/admin/AllCommandsList.h`
- `src/eve-server/admin/CommandDispatcher.cpp`
- `src/eve-server/admin/CommandDispatcher.h`
- `src/eve-server/admin/GMCommands.cpp`
- `src/eve-server/admin/GMCommands.h`
- `src/eve-server/admin/SystemCommands.cpp`
- `src/eve-server/admin/SystemCommands.h`
- `src/eve-server/admin/debug_commands.cpp`
- `src/eve-server/exploration/Scan.cpp`
- `src/eve-server/exploration/Scan.h`
- `src/eve-server/exploration/ScanMgrService.cpp`

### Inventory and manufacturing (8)
- `src/eve-server/inventory/Inventory.cpp`
- `src/eve-server/inventory/InventoryBound.cpp`
- `src/eve-server/inventory/InventoryBound.h`
- `src/eve-server/inventory/InventoryItem.cpp`
- `src/eve-server/manufacturing/FactoryDB.cpp`
- `src/eve-server/manufacturing/FactoryDB.h`
- `src/eve-server/manufacturing/RamMethods.cpp`
- `src/eve-server/manufacturing/RamProxyService.cpp`

### NPC, drones and weapons (9)
- `src/eve-server/npc/Drone.cpp`
- `src/eve-server/npc/DroneAI.cpp`
- `src/eve-server/npc/DroneAI.h`
- `src/eve-server/npc/EntityService.cpp`
- `src/eve-server/npc/EntityService.h`
- `src/eve-server/ship/modules/ActiveModule.cpp`
- `src/eve-server/ship/modules/ActiveModule.h`
- `src/eve-server/ship/modules/MiningLaser.cpp`
- `src/eve-server/ship/modules/MiningLaser.h`

### POS, sovereignty and cosmic systems (22)
- `src/eve-server/pos/PosMgr.cpp`
- `src/eve-server/pos/PosMgrDB.cpp`
- `src/eve-server/pos/Structure.cpp`
- `src/eve-server/pos/Tower.cpp`
- `src/eve-server/pos/sovStructures/SBU.cpp`
- `src/eve-server/system/Asteroid.cpp`
- `src/eve-server/system/Asteroid.h`
- `src/eve-server/system/BubbleManager.cpp`
- `src/eve-server/system/Container.cpp`
- `src/eve-server/system/DestinyManager.cpp`
- `src/eve-server/system/DestinyManager.h`
- `src/eve-server/system/IndexManager.cpp`
- `src/eve-server/system/SystemBubble.cpp`
- `src/eve-server/system/SystemManager.cpp`
- `src/eve-server/system/TargetManager.cpp`
- `src/eve-server/system/cosmicMgrs/AnomalyMgr.cpp`
- `src/eve-server/system/cosmicMgrs/BeltMgr.cpp`
- `src/eve-server/system/cosmicMgrs/DungeonMgr.cpp`
- `src/eve-server/system/sov/SovereigntyDB.cpp`
- `src/eve-server/system/sov/SovereigntyDB.h`
- `src/eve-server/system/sov/SovereigntyDataMgr.cpp`
- `src/eve-server/system/sov/SovereigntyDataMgr.h`

