# EVEmu Crucible — upstream PR #335 handoff and patch review (17 November 2026)

> This document updates, not replaces, [the 15 November complete upstream dossier](UPSTREAM_PULL_REQUEST_REVIEW_2026-11-15.md) and the full fork [PATCH_NOTES.md](../PATCH_NOTES.md). The historical snapshot is intentionally preserved. It is for maintainers reviewing **draft** [upstream PR #335](https://github.com/EvEmu-Project/evemu_Crucible/pull/335), not an approval or an upstream merge.

## Current upstream PR #335 publication state

- Existing [draft PR #335](https://github.com/EvEmu-Project/evemu_Crucible/pull/335) was verified **open**, **draft**, and mergeable at the Git comparison level, not merged. Its source is the original `xrizonx:contrib/upstream-crucible-maintenance-2026-11-15`, now incorporating fork `master` as a second-parent Git merge while preserving its previous three branch-only commits and review documents.
- GitHub's published contribution-branch comparison immediately after that merge showed **696 commits ahead, 0 behind and 244 changed paths** relative to the upstream baseline. Later documentation-only commits may increment the commit count, so use the live GitHub PR count as the authoritative current value.
- **Full latest CI result:** [fork Actions #575](https://github.com/xrizonx/evemu_Crucible/actions/runs/44142537989) completed **successfully** on the PR #36 head. **Windows UCRT64 GCC, Linux GCC, Docker image and MariaDB seed insertion all passed**; the optional release publish job was skipped. This build does not establish that the later server-crash scenario or 500M wallet transaction has been playtested.

## 1. Reproducible scope and provenance
- Contributor: [xrizonx/evemu_Crucible](https://github.com/xrizonx/evemu_Crucible), merged `master` **`97a98424d2ddcbbc885ec90ca753fa47cd1a5ca2`** (fork PR [#36](https://github.com/xrizonx/evemu_Crucible/pull/36) merged on 17 November 2026).
- Upstream target baseline: [EvEmu-Project/evemu_Crucible](https://github.com/EvEmu-Project/evemu_Crucible), `master` **`f55b488a006e3090470d3b481aee47a1d7a0dade`**, as checked for this submission.
- **Master vs upstream baseline:** **692 commits ahead**, **0 behind**, **241 changed paths** before the additional draft-review documentation/merge commits. These are Git history/diff statistics, **not newly authored changes in one PR**.
- **Since 15 November contribution merge base** `6de04d1df5640122b691365029eb5e5c26f197c5`: **280 commits ahead**, **144 changed paths** (7,158 additions / 1,018 deletions in GitHub's textual diff; merge commits and historical changes are included). All 144 paths are inventoried below. Historical branch-only review notes are archived separately; the updated submission is a genuine merge, not a branch reset.
- Local fork merge commit: [`97a98424d2dd`](https://github.com/xrizonx/evemu_Crucible/commit/97a98424d2ddcbbc885ec90ca753fa47cd1a5ca2). Source changes are not merged into upstream `master` until maintainers decide.

## 2. Team patch notes — new work since the previous upstream dossier

### Missions, agent state and expedition
- Kept Security Agent **The Blockade** (synthetic mission ID **65535**) at **12 NPCs and 50,000,000 ISK**. Existing mission state, reward-once semantics and agent standing rules remain.
- Added distinct **DED & Officer Expedition** (ID **65534**) to normal and important Security Agent pools. **8 NPCs** per new site: **2 real deadspace/DED**, **2 faction commanders**, **2 pirate officers**, **2 DED overseers**. Random selection chooses distinct validated SDE inventory groups from 18 legacy candidate groups across repeated runs, not invented type IDs.
- Exclude type names containing **"True Sansha"** case-insensitively from this new mission's NPC candidate set, following operator targetability feedback. Other valid Sansha types remain eligible; category validation requires at least two groups with valid candidates. The exclusion does **not** change normal belt, anomaly or Blockade spawn tables.
- Elite mission reward now **500,000,000 ISK once at successful agent turn-in**, **120-minute** acceptance deadline. Agent offer normalization, briefing/journal, C++ lifecycle guards and Python source checks enforce this; newly offered missions obtain the value without an SQL migration. **Do not change Blockade to 500M**.
- Premium wreck drop logic selects genuine `invMetaTypes.metaGroupID` **4 Faction**, **5 Officer**, **6 Deadspace** Module-category items from the installed Crucible SDE. Prefer published, market-group items; fall back to real rarity-tagged Module types if their old SDE `published`/`marketGroupID` fields are unset. Applies **only** to 65534 NPC wrecks; ordinary loot is additive and unmodified. Operator observed **605 faction, 490 officer, 346 deadspace** eligible module types in startup logs.
- First eight-NPC live run reported **8/8 kills** and ready-for-turn-in. Operator reported a later server exit; no native exception stack was provided. Defensive mission cleanup releases only matching offer spawn references/flags without deleting wrecks; new beacon placement rejects occupied or mismatched bubbles; bubble indexes and anomaly signal null-bubble handling hardened. These are source-level fixes, **not a confirmed resolution of that server exit**.
- Regression: `src/eve-test/utils/MissionLifecycleTest.cpp`, `tools/verify_elite_security_mission.py`, updated build workflow and [mission client test guide](missions/DED_OFFICER_EXPEDITION_TEST_MODE.md).

### Market pricing, sell settlement and EVEDBTool buy orders
- Added and repaired 100%-reference NPC **sell** pricing and **buy** market seeding with explicit audit/rollback notes in [market guide](NPC_BUY_MARKET_100_PERCENT_2026-11-17.md), [price guidance](NPC_MARKET_100_PERCENT_REFERENCE_PRICE_2026-11-16.md) and `sql/diagnostics`. The user previously confirmed player ISK credit after selling to NPC buy orders; this is not blanket evidence of all market flows.
- `evedbtool seed -buy-only` creates NPC bids at `invTypes.basePrice` with recorded fallback for missing base prices; `-force` performs an intentional refresh. MariaDB unsigned `orderRange` compatibility requires **0**, not **-1**; stationID constrains immediate sell matching. Scoped NPC owner/funds and aggregate caps are documented. Avoid executing `-force` against production without backup/inspection.
- Preserve player-market orders, inventory state and ordinary transactions; no migration is required merely to **merge source code**.

### Client, inventory, agents, station portraits and POS
- Improved station container assembly, inventory validation, item moves, null container references, duplicate insertion handling and station/office interactions; user reported one assembled Giant Secure Container path working with contents across restarts, not a complete POS approval. See [PR25 game checklist](PR25_INGAME_VALIDATION_CHECKLIST.md) and [container audit](EVEMU_CONTAINER_POS_INVENTORY_AUDIT_2026-11-16.md).
- Station agent image requests, local-vs-remote portraits and Crucible 50px image behavior were examined with rollback/client-compatibility notes in the image/portrait documents; do not assume deprecated NPC portraits are available from remote CDN.
- Sovereignty/structure and POS modules, corporation/mail/contract/manufacturing guardrails, asteroid security/rat spawn tables and database cleanup audits form part of the current fork delta. These areas need subsystem-specific regression, not a single blanket acceptance.

### Navigation, combat and infrastructure
- Updated warp/approach, orbit and weapon-range diagnostics, movement formulas, tower/faction spawn handling, target lifecycle defenses, drone behavior and FX safeguards. See [range and warp audit](CRUCIBLE_RANGE_AND_WARP_AUDIT_2026-11-17.md), [belt security audit](CRUCIBLE_SOV_SECURITY_BELTS_RATS_AUDIT_2026-11-16.md), and [subsystem test matrix](GAMEPLAY_SUBSYSTEMS_AUDIT_TEST_MATRIX_2026-11-16.md).
- Updated Windows UCRT64, Linux GCC, Docker and CTest automation, packaging and EVEDBTool seed smoke tests; no one CI success proves all 2012-client interactions, live markets, mining drones, or POS functionality. The base 15 November dossier covers earlier foundational fixes in full.

## 3. Fork PR #36 — complete changed-file responsibilities

| Changed path | Team review focus |
| --- | --- |
| `.github/workflows/build-release.yml` | Windows/Linux regression and package checks |
| `PATCH_NOTES.md` | Chronological code changes, failures and verification boundaries |
| `docs/missions/DED_OFFICER_EXPEDITION_TEST_MODE.md` | SQL audit and end-to-end operator playtest plan |
| `src/eve-common/EVE_Missions.h` | Mission IDs, reward/kill constants, rarity mapping and random selection helper |
| `src/eve-server/Client.cpp` | Encounter site placement, journaling and safe teardown |
| `src/eve-server/StaticDataMgr.cpp` | Load and select actual SDE faction/officer/deadspace Module types |
| `src/eve-server/StaticDataMgr.h` | Rare module cache/interface |
| `src/eve-server/agents/Agent.cpp` | Offer normalization and persistent reward/kill state |
| `src/eve-server/agents/AgentBound.cpp` | Acceptance, briefing, completion and single reward settlement |
| `src/eve-server/missions/MissionDataMgr.cpp` | Register elite encounter in Security Agent pools |
| `src/eve-server/npc/NPC.cpp` | Scope premium wreck drops to elite mission NPCs |
| `src/eve-server/system/BubbleManager.cpp` | Repair bubble lookup index cleanup |
| `src/eve-server/system/SystemBubble.cpp` | Bubble baseline lifecycle and mission bookkeeping |
| `src/eve-server/system/SystemBubble.h` | Mission identity and clear/reset lifecycle |
| `src/eve-server/system/cosmicMgrs/AnomalyMgr.cpp` | Null-bubble protection for new mission signals |
| `src/eve-server/system/cosmicMgrs/SpawnMgr.cpp` | Eight-NPC selection, True Sansha filter and kill events |
| `src/eve-server/system/cosmicMgrs/SpawnMgr.h` | Mission bubble teardown interface |
| `src/eve-test/utils/MissionLifecycleTest.cpp` | Deterministic mission and reward regression checks |
| `tools/verify_elite_security_mission.py` | CI source invariants for agent, spawn, loot and cleanup code |

## 4. CI evidence and verification boundaries
- Pre-merge fork [GitHub Actions #575](https://github.com/xrizonx/evemu_Crucible/actions/runs/44142537989) on expedition head `256cf374bb1a04635573861465fbf73dc6faafdc`: **completed** / **success** (all four build/seed jobs passed). Linux GCC and MariaDB had already passed before fork PR #36 merged; Windows and Docker passed shortly afterward. This is compile/static CI confirmation only, not a complete Crucible client playtest.
- [Fork PR #36](https://github.com/xrizonx/evemu_Crucible/pull/36) merged into `master` via `97a98424d2ddcbbc885ec90ca753fa47cd1a5ca2`; an independent master post-merge CI run may be triggered and must be checked separately.
- Older [15 Nov CI baseline #43747566528](https://github.com/xrizonx/evemu_Crucible/actions/runs/43747566528) passed Windows/Linux/Docker for an older baseline, **not** this current head.
- Operator-reported evidence: valid module cache **605/490/346**, eight real NPC types, progress **8/8**; wallet confirmation for the updated **500M** payout, consecutive no-crash missions and all True Sansha targetability require new gameplay validation.

| Area | CI/static coverage | Required maintainer/live acceptance |
| --- | --- | --- |
| DED 65534 | Mission lifecycle/static SDE/source checks and compilation | Accept twice, kill exactly 8 valid NPCs each, no True Sansha; verify distinct groups per category and targetability |
| Loot | 3-tier item cache/source asserts | Inspect 4/5/6 rarity item in wrecks; confirm no changes to Blockade, belts or anomalies |
| Reward | Separate 500M/50M constants and one-time turn-in path | Wallet: exactly one **500M** reward on DED completion; no duplicate on repeat interaction; Blockade **50M** |
| Mission cleanup | Bubble/registry and null-pointer guards | Warp out, abandon, finish, reaccept in same system, test wreck persistence; capture Windows dump if server exits |
| Market | Go seed smoke/MariaDB unsigned range checks and price scripts | Backup before `seed -buy-only -force`; verify matching, price and ISK credit with no player-order damage |
| Inventory/stations | Null/capacity/container and service tests | Player item moves, container relog, agent portraits, cargo and hangar station edge cases |
| Combat/drones/POS | Compile and isolated behavior checks | Sustained PvE, drone aggression/retreat, warp/weapon mechanics, POS/sovereignty data with actual Crucible client |
| Packaging | Windows/Linux build, Docker, CTest | Windows 10/11 client/server deployment and real SQL schema compatibility |

## 5. Operational cautions, scope and rollback
- This is a **draft upstream review**, not permission to merge upstream or run database commands automatically.
- The fork's source merge **did not run SQL migrations**; SQL diagnostics are primarily read-only, while explicitly documented seed/normalization commands can modify live market rows.
- Prefer maintainer reviews **by subsystem**, since the cumulative PR is too large for one thorough pass. Keep any backports/cherry-picks tied to the 17 November snapshot.
- Distinguish *compiled*, *operator-reported working*, and *not yet verified* paths; particularly the unknown fatal server-exit stack and unverified new wallet credit.
- Rollback strategy: a code-level revert or targeted cherry-pick if rejected; restore database from backups only if an operator separately executed a mutating seeding/migration tool. The review branch preserves both `master` history and earlier contribution-branch commits.

## 6. Complete changed-file inventory since the 15 November contribution baseline

This is the **complete 144-file list reported by GitHub** for `6de04d1` → `97a98424d2dd`. It is a **delta since 15 November**, not a replacement for the earlier exhaustive upstream inventory; together both documents cover the overall submission. The reported status/line counts are textual source diff statistics, not measured gameplay impact.

| Path | GitHub diff status | Added / deleted lines |
| --- | --- | --- |
| `.github/workflows/build-release.yml` | modified | +151 / -6 |
| `.gitignore` | modified | +7 / -1 |
| `PATCH_NOTES.md` | modified | +172 / -0 |
| `README.md` | modified | +5 / -2 |
| `docs/A01_STAGING_HARNESS.md` | added | +21 / -0 |
| `docs/AGENT_LOCAL_PORTRAITS_2026-11-15.md` | added | +76 / -0 |
| `docs/AGENT_PORTRAIT_FULL_REMOVAL_2026-11-16.md` | added | +41 / -0 |
| `docs/AGENT_STATION_IMAGE_AUDIT_2026-11-15.md` | modified | +6 / -0 |
| `docs/AGENT_STATION_PORTRAIT_ROLLBACK_2026-11-16.md` | added | +33 / -0 |
| `docs/BLOCKADE_REWARD_AND_CONTAINER_MARKET_SEED_2026-11-15.md` | added | +48 / -0 |
| `docs/CONTAINER_ASSEMBLY_AND_AGENT_FALLBACK_FIX_2026-11-16.md` | added | +65 / -0 |
| `docs/CRUCIBLE_CLIENT_GAMEPLAY_VERIFICATION_CHECKLIST_2026-11-16.md` | added | +170 / -0 |
| `docs/CRUCIBLE_IMAGE_REDIRECT_CRASH_TRIAGE_2026-11-16.md` | added | +47 / -0 |
| `docs/CRUCIBLE_ITEM_SQL_CORP_TAX_FIX_2026-11-16.md` | added | +161 / -0 |
| `docs/CRUCIBLE_LOCAL_AGENT_PORTRAIT_TEST_2026-11-16.md` | added | +67 / -0 |
| `docs/CRUCIBLE_RANGE_AND_WARP_AUDIT_2026-11-17.md` | added | +54 / -0 |
| `docs/CRUCIBLE_SOV_SECURITY_BELTS_RATS_AUDIT_2026-11-16.md` | added | +173 / -0 |
| `docs/CRUCIBLE_STATION_AGENT_50PX_CLIENT_CRASH_TRIAGE_2026-11-16.md` | added | +63 / -0 |
| `docs/CRUCIBLE_STATION_CONTAINER_TRANSFER_CRASH_TRIAGE_2026-11-16.md` | added | +113 / -0 |
| `docs/EVEMU_CONTAINER_POS_INVENTORY_AUDIT_2026-11-16.md` | added | +122 / -0 |
| `docs/GAMEPLAY_SUBSYSTEMS_AUDIT_TEST_MATRIX_2026-11-16.md` | added | +241 / -0 |
| `docs/MISSION_EXPIRY_AND_OFFLINE_STACK_AUDIT_2026-11-15.md` | added | +32 / -0 |
| `docs/NPC_BUY_MARKET_100_PERCENT_2026-11-17.md` | added | +66 / -0 |
| `docs/NPC_MARKET_100_PERCENT_REFERENCE_PRICE_2026-11-16.md` | added | +93 / -0 |
| `docs/PR25_INGAME_VALIDATION_CHECKLIST.md` | added | +153 / -0 |
| `docs/REPAIR_CHECKLIST.md` | added | +25 / -0 |
| `docs/STATION_LOGIN_OFFICE_CRASH_AUDIT_2026-11-16.md` | added | +94 / -0 |
| `docs/missions/BLOCKADE_TEST_MODE.md` | modified | +5 / -3 |
| `docs/missions/DED_OFFICER_EXPEDITION_TEST_MODE.md` | added | +106 / -0 |
| `sql/README.md` | modified | +14 / -2 |
| `sql/diagnostics/2026-11-16-container-pos-integrity.sql` | added | +83 / -0 |
| `sql/diagnostics/2026-11-16-normalize-npc-sell-orders-100-percent.sql` | added | +85 / -0 |
| `sql/diagnostics/2026-11-16-sov-security-belt-rat-integrity.sql` | added | +193 / -0 |
| `sql/diagnostics/2026-11-17-crucible-ship-weapon-range-warp-audit.sql` | added | +106 / -0 |
| `sql/diagnostics/2026-11-17-crucible-warp-speed-remaining-14-repair.sql` | added | +72 / -0 |
| `sql/diagnostics/2026-11-17-crucible-warp-speed-verified-repair.sql` | added | +173 / -0 |
| `sql/diagnostics/2026-11-17-npc-buy-orders-100pct-audit.sql` | added | +106 / -0 |
| `sql/seed_and_clean/seed_market.sql` | modified | +13 / -5 |
| `sql/seed_and_clean/seed_station_market.sql` | modified | +9 / -3 |
| `src/eve-common/EVEVersion.h` | modified | +4 / -1 |
| `src/eve-common/EVE_AgentHelpers.h` | modified | +6 / -3 |
| `src/eve-common/EVE_AsteroidSpawnHelpers.h` | added | +51 / -0 |
| `src/eve-common/EVE_ContainerHelpers.h` | added | +35 / -0 |
| `src/eve-common/EVE_ImageHelpers.h` | added | +19 / -0 |
| `src/eve-common/EVE_LegacyAgentThumbnail.h` | added | +18 / -0 |
| `src/eve-common/EVE_Market.h` | modified | +4 / -4 |
| `src/eve-common/EVE_Missions.h` | modified | +60 / -2 |
| `src/eve-common/EVE_MovementFormulas.h` | modified | +11 / -0 |
| `src/eve-common/EVE_StationOfficeHelpers.h` | added | +27 / -0 |
| `src/eve-core/database/dbcore.cpp` | modified | +9 / -4 |
| `src/eve-core/network/TCPConnection.cpp` | modified | +6 / -1 |
| `src/eve-core/threading/Threading.cpp` | modified | +11 / -6 |
| `src/eve-server/CMakeLists.txt` | modified | +1 / -1 |
| `src/eve-server/Client.cpp` | modified | +49 / -5 |
| `src/eve-server/ServiceDB.cpp` | modified | +4 / -1 |
| `src/eve-server/StaticDataMgr.cpp` | modified | +61 / -1 |
| `src/eve-server/StaticDataMgr.h` | modified | +5 / -0 |
| `src/eve-server/agents/Agent.cpp` | modified | +37 / -5 |
| `src/eve-server/agents/AgentBound.cpp` | modified | +107 / -39 |
| `src/eve-server/agents/AgentMgrService.cpp` | modified | +4 / -3 |
| `src/eve-server/character/Character.h` | modified | +3 / -0 |
| `src/eve-server/contract/ContractProxy.cpp` | modified | +95 / -17 |
| `src/eve-server/corporation/CorpRegistryBound.cpp` | modified | +57 / -12 |
| `src/eve-server/corporation/CorpRegistryBound.h` | modified | +1 / -1 |
| `src/eve-server/corporation/CorporationDB.cpp` | modified | +12 / -6 |
| `src/eve-server/corporation/CorporationDB.h` | modified | +1 / -1 |
| `src/eve-server/imageserver/ImageServer.cpp` | modified | +36 / -5 |
| `src/eve-server/imageserver/ImageServerConnection.cpp` | modified | +35 / -7 |
| `src/eve-server/imageserver/ImageServerConnection.h` | modified | +1 / -0 |
| `src/eve-server/inventory/InvBrokerService.cpp` | modified | +106 / -35 |
| `src/eve-server/inventory/Inventory.cpp` | modified | +52 / -6 |
| `src/eve-server/inventory/Inventory.h` | modified | +2 / -0 |
| `src/eve-server/inventory/InventoryBound.cpp` | modified | +97 / -11 |
| `src/eve-server/inventory/InventoryItem.cpp` | modified | +59 / -24 |
| `src/eve-server/inventory/ItemDB.cpp` | modified | +16 / -7 |
| `src/eve-server/inventory/ItemFactory.cpp` | modified | +11 / -37 |
| `src/eve-server/mail/MailDB.cpp` | modified | +162 / -238 |
| `src/eve-server/mail/MailDB.h` | modified | +10 / -9 |
| `src/eve-server/mail/MailMgrService.cpp` | modified | +11 / -7 |
| `src/eve-server/manufacturing/RamMethods.cpp` | modified | +4 / -10 |
| `src/eve-server/manufacturing/RamProxyService.cpp` | modified | +11 / -6 |
| `src/eve-server/market/MarketBotMgr.cpp` | modified | +20 / -10 |
| `src/eve-server/market/MarketDB.cpp` | modified | +1 / -1 |
| `src/eve-server/market/MarketMgr.cpp` | modified | +60 / -101 |
| `src/eve-server/missions/MissionDB.cpp` | modified | +29 / -17 |
| `src/eve-server/missions/MissionDataMgr.cpp` | modified | +80 / -46 |
| `src/eve-server/missions/MissionDataMgr.h` | modified | +1 / -0 |
| `src/eve-server/npc/DroneAI.cpp` | modified | +7 / -3 |
| `src/eve-server/npc/NPC.cpp` | modified | +30 / -0 |
| `src/eve-server/pos/JumpBridge.cpp` | modified | +14 / -4 |
| `src/eve-server/pos/JumpBridge.h` | modified | +1 / -1 |
| `src/eve-server/pos/Module.cpp` | modified | +8 / -2 |
| `src/eve-server/pos/Module.h` | modified | +1 / -1 |
| `src/eve-server/pos/PosMgr.cpp` | modified | +36 / -12 |
| `src/eve-server/pos/Structure.cpp` | modified | +92 / -19 |
| `src/eve-server/pos/Structure.h` | modified | +5 / -1 |
| `src/eve-server/pos/Tower.cpp` | modified | +26 / -10 |
| `src/eve-server/pos/Tower.h` | modified | +1 / -1 |
| `src/eve-server/pos/sovStructures/IHub.cpp` | modified | +15 / -5 |
| `src/eve-server/pos/sovStructures/SBU.cpp` | modified | +30 / -10 |
| `src/eve-server/pos/sovStructures/TCU.cpp` | modified | +31 / -3 |
| `src/eve-server/ship/Ship.cpp` | modified | +1 / -1 |
| `src/eve-server/ship/modules/ActiveModule.cpp` | modified | +38 / -16 |
| `src/eve-server/ship/modules/MiningLaser.cpp` | modified | +23 / -0 |
| `src/eve-server/ship/modules/TurretFormulas.cpp` | modified | +8 / -5 |
| `src/eve-server/station/Station.cpp` | modified | +20 / -7 |
| `src/eve-server/station/Station.h` | modified | +6 / -1 |
| `src/eve-server/station/StationDB.cpp` | modified | +4 / -1 |
| `src/eve-server/system/BubbleManager.cpp` | modified | +13 / -5 |
| `src/eve-server/system/Container.cpp` | modified | +18 / -1 |
| `src/eve-server/system/Container.h` | modified | +3 / -1 |
| `src/eve-server/system/DestinyManager.cpp` | modified | +39 / -49 |
| `src/eve-server/system/IndexManager.cpp` | modified | +8 / -2 |
| `src/eve-server/system/SystemBubble.cpp` | modified | +1 / -0 |
| `src/eve-server/system/SystemBubble.h` | modified | +17 / -0 |
| `src/eve-server/system/SystemManager.cpp` | modified | +14 / -8 |
| `src/eve-server/system/TargetManager.cpp` | modified | +43 / -12 |
| `src/eve-server/system/cosmicMgrs/AnomalyMgr.cpp` | modified | +14 / -2 |
| `src/eve-server/system/cosmicMgrs/BeltMgr.cpp` | modified | +68 / -34 |
| `src/eve-server/system/cosmicMgrs/BeltMgr.h` | modified | +1 / -1 |
| `src/eve-server/system/cosmicMgrs/ManagerDB.cpp` | modified | +6 / -1 |
| `src/eve-server/system/cosmicMgrs/SpawnMgr.cpp` | modified | +331 / -19 |
| `src/eve-server/system/cosmicMgrs/SpawnMgr.h` | modified | +2 / -0 |
| `src/eve-server/system/sov/SovereigntyDataMgr.cpp` | modified | +29 / -11 |
| `src/eve-test/CMakeLists.txt` | modified | +16 / -1 |
| `src/eve-test/utils/AgentHelpersTest.cpp` | modified | +4 / -2 |
| `src/eve-test/utils/AsteroidSpawnTest.cpp` | added | +56 / -0 |
| `src/eve-test/utils/ContainerAssemblyTest.cpp` | added | +49 / -0 |
| `src/eve-test/utils/ImageCompatibilityTest.cpp` | added | +46 / -0 |
| `src/eve-test/utils/MissionLifecycleTest.cpp` | added | +83 / -0 |
| `src/eve-test/utils/MovementFormulaTest.cpp` | modified | +17 / -2 |
| `src/eve-test/utils/StationOfficeIntegrityTest.cpp` | added | +39 / -0 |
| `src/eve-test/utils/WeaponFormulaTest.cpp` | modified | +4 / -0 |
| `tools/evedbtool/command_seed.go` | modified | +255 / -52 |
| `tools/prepare-crucible-agent-portrait.ps1` | added | +65 / -0 |
| `tools/tests/npc_buy_seed_unsigned_schema.sql` | added | +81 / -0 |
| `tools/verify_agent_portraits_removed.py` | added | +33 / -0 |
| `tools/verify_blockade_12_ship_fleet.py` | added | +31 / -0 |
| `tools/verify_crucible_range_and_warp.py` | added | +54 / -0 |
| `tools/verify_elite_security_mission.py` | added | +168 / -0 |
| `tools/verify_npc_buy_seed.py` | added | +82 / -0 |
| `tools/verify_npc_market_baseprice.py` | added | +43 / -0 |
| `tools/verify_station_crash_safety.py` | added | +187 / -0 |
| `tools/verify_subsystems_audit.py` | added | +75 / -0 |
