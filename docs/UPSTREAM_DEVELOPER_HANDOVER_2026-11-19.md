# EVEmu Crucible — upstream PR #335: full developer handover (19 November 2026)

**Audience:** EvEmu-Project maintainers, code reviewers, CI maintainers, database operators, and Crucible client QA.
**Status:** DRAFT upstream contribution; **not** an upstream merge approval, migration execution, or declaration of full gameplay coverage.
**Target:** [EvEmu-Project/evemu_Crucible#335](https://github.com/EvEmu-Project/evemu_Crucible/pull/335), upstream `master` at inspection `f55b488a006e3090470d3b481aee47a1d7a0dade`.
**Contributor:** `xrizonx/evemu_Crucible`; source branch `contrib/upstream-crucible-maintenance-2026-11-15`.
**Source synchronization (19 Nov):** merge `9f8929de3387c9ae471e768556eefc8657c62f5f` with first parent `9b2aca90917fabdc78ae0526a220908b8464f2da` (previous upstream contribution) and second parent `e2f8e14bca5b50bb1f8ff055f5f16a00d01b7c27` (fork `master` after merged PR #43).
**Scope at that pre-documentation commit:** GitHub upstream draft PR #335 showed **811 commits, 303 changed paths, +22,836/-3,701 textual lines**. These are cumulative against upstream's older baseline and must **not** be mistaken for work authored on 19 November; the count increases when this handover is committed.
**Reference:** [Full pre-handover 303-path inventory](UPSTREAM_PR335_FILE_INVENTORY_2026-11-19.md) · [19 November cumulative patch notes](UPSTREAM_PATCH_NOTES_2026-11-19.md) · [fork patch notes](../PATCH_NOTES.md).

> **Read this first:** The contribution is intentionally broad. Upstream has previously reported an automated-review size limit. Request subsystem-specific reviews or select cherry-picks rather than equating a successful build with production acceptance. The user has authorized updating the draft *source branch* and PR description, not merging the PR into EvEmu-Project master or modifying any running database.

## 1. Executive engineering summary and integration boundary

The cumulative fork implements modern build/release plumbing for the older Crucible server, hardened Python/DB/inventory paths, mission and agent lifecycle work, warp/jump/docking and Destiny corrections, combat/drone improvements, NPC market pricing and order seeding, early sovereignty and POS upgrade paths, scanner improvements, and research/copying safeguards. The latest import brings three fork PRs already merged into fork `master`:

- **Fork #41:** Exclude untargetable Domination Grigori from synthetic DED/Officer mission 65534 while preserving the existing True Sansha exclusion and ordinary officer/belt NPCs.
- **Fork #42:** Mining-drone flight, approach/orbit, range, harvesting and return state integration. Keep `DestinyManager::ApplyMiningDroneStep` and matching drone state serialization. This path requires real ore-return and persistence QA.
- **Fork #43:** Client/server warp arrival and safe surface-distance approach, NPC orbit error recovery, Destiny idle/stop improvements, ME/PE research and blueprint copying safeguards, industry regression tests, and source-level CI repair. The operator reported the core warp animation, stargate jump, and station docking loop working in the Crucible client on the tested build; this **does not** certify all client interpolation or every collision mesh.

The contribution-only additions are **100-million-unit ore/ice/mineral NPC bids and asks**, plus historical upstream review documents and their CI/MariaDB tests. They deliberately remain outside fork `master` and are offered to upstream through PR #335.

### Integration proof and preservation

Before synchronization, contribution `9b2aca9` and fork master `e2f8e14` shared merge base `2d5cb4d`; the contribution was 21 commits ahead and 47 behind. The three-way comparison found only **two overlapping paths**: `.github/workflows/build-release.yml` and `PATCH_NOTES.md`. The merge retained the contribution-only 100M market steps, combined them with master’s research verifiers, and preserved both chronological histories. It is a **two-parent merge with no force push**.

The merged tree was checked against both parent Git trees: no path in either parent was deleted; only the two expected common files were reconciled. Source changes from master outside the contribution’s known 15-path delta were retained, and contribution market/seed files were deliberately overlaid where they differed. Git comparison against fork `master` now isolates that 15-path contribution-specific delta. This verifies **source preservation**, not the absence of runtime bugs.

## 2. Build system, compatibility and developer tooling

**Primary files:** `CMakeLists.txt`; `cmake/FindUTF8CPP.cmake`; `Dockerfile`; `.dockerignore`; `.github/workflows/build-release.yml`; `tools/evedbtool/command_seed.go`; appropriate `src/eve-core` and `src/eve-common` portability sources.

- Modernized GCC/CMake, MariaDB/MySQL client, Boost/Asio, TinyXML, UTF8CPP, Windows MinGW/UCRT64 linker and debug-stack support. Replaced old `my_bool` option handling with portable boolean handling. Protected program initialization against empty commit hash string manipulation and runtime misconfiguration.
- Linux builds use Ubuntu 22.04 and GCC/Ninja/CMake with installed runtime data. Windows builds use MSYS2 UCRT64 GCC on Windows 2022 runners and package portable Windows 10/11-compatible executables plus adjacent runtime DLLs. Recursive UCRT DLL staging and required DLL checks guard loader errors; this is **not** proof the whole EVE client is portable.
- Docker builds use the maintained build context, dependency and cache layout. Build artifacts include a Linux tarball and Windows x86_64 ZIP, checksums, SQL, tools, config and a matching patched EVEDBTool executable.
- Added registered CTest suites and Windows/Linux Python source-invariant checks. Source guards complement compiled tests, but text/string invariants must be updated when implementation semantics change: older audit scripts once rejected safe code because comments or formulas were rewritten.
- The EVEDBTool patch is applied to upstream `EvEmu-Project/EVEDBTool` tag `0.0.6` during CI and built for Windows/Linux; **deploy the patched tool with this tree’s SQL and migration definitions**, not an unrelated copy.

**Reproduction:** Fork `master` commit `e2f8e14` passed [GitHub Actions #44479434302](https://github.com/xrizonx/evemu_Crucible/actions/runs/44479434302): Linux GCC/CTest/package, Windows UCRT64 GCC/CTest/ZIP, Docker, and MariaDB integration. A previous branch run [#44473762678](https://github.com/xrizonx/evemu_Crucible/actions/runs/44473762678) also passed. **The combined 19 November upstream contribution merge has not yet been evidenced by a full CI pass**; its new market checks must run together with the master industry checks before accepting upstream.

## 3. Core stability, serialization, services and inventory

- Improved `DBResultToIntIntDict` and search projections to prevent one-column results being read as two, and bounds-checked database row text/column access. Hardened PyPacket and conversion paths, optional objects, format-string use and exception handling.
- Fixed probe map iteration when probes unregister themselves; guarded missing system entities, station inventory and clone respawn lookups, dynamic factory construction and NPC image handling.
- Corrected duplicate/empty bubble entity registration and removal, infinite-loop and double-remove-ball risks, and type-ID versus group-ID mistakes when loading wormhole/system dynamics. Added safe null checks instead of dereferencing inactive modules, entities or absent inventory containers.
- Hardened hangar/office/container inventory assembly, ownership, capacity, flag, move, stack and scoop semantics. The user's Giant Secure Container test was positive for a particular path but does not validate every POS/container lifecycle.
- Corrected Crucible-era image server category paths, MIME types, optional local portrait files and legacy NPC portrait fallback. Remote legacy portrait redirects and removed historical portraits remain client/network compatibility caveats.
- Addressed debugging and GM command permission checks, server logging for rejected actions, and null-state failures. Upstream issue cross-references and unresolved reproductions are documented in [historical issue review](UPSTREAM_PULL_REQUEST_REVIEW_2026-11-15.md) and [issue audit](UPSTREAM_ISSUE_AUDIT_2026-11-17.md).

**Reviewer check:** run search, inventory, journal and station transitions including relog and solar-system unload. Inspect DB writes on a staging database, verify no unexpected child items or duplicate ball removals, and check actual stack traces for unresolved crash reports.

## 4. Missions, agents and premium expedition

**Files:** `src/eve-server/agents/Agent.cpp`, `AgentBound.cpp`, `AgentDB.cpp`, `AgentMgrService.cpp`, `src/eve-server/system/cosmicMgrs/SpawnMgr.cpp`, related `DungeonMgr`/`ManagerDB` and NPC death/loot paths; `tools/verify_blockade_12_ship_fleet.py` and `tools/verify_elite_security_mission.py`.

- Normal security agents load existing SDE-backed mission templates, associate accepted offers with isolated encounter pockets/beacons and attempt template-based first-room NPC compositions; faction selection falls back to agent/region or compatible generic spawns. Mission offer progress, kill counts, cleanup, rewards, journal and agent state include explicit defensive checks. This is **not** a complete multi-room mission/dungeon scripting engine.
- The synthetic **Blockade mission ID 65535** uses **12 NPCs** and a **50 million ISK** reward. The separate **DED & Officer Expedition ID 65534** uses **8 NPCs** drawn from genuine deadspace/faction/officer/overseer SDE groups and awards **500 million ISK once** on successful turn-in; preserve the distinction, agent pool rules and one-time settlement. Do not inadvertently promote the Blockade reward to 500M.
- The expedition uses true SDE Module rarity metadata to select faction, officer and deadspace wreck drops; it does not invent item type IDs. Its candidate filter excludes name patterns **True Sansha** and **Domination Grigori** only in the new elite mission. Regular belts, ordinary agents and other Angel officers remain available.
- Fixes include dead-pocket cleanup, partial NPC spawn-registry inconsistency, stale offer ownership, bonus deadline and briefing response handling, crash-safe cleanup and fallback mission objective detection.

**Live acceptance matrix:** Agent A accepts, warp to beacon, NPC/ship damage and bounties, 8/8 or 12/12 kills, wrecks/loot, journal updates, once-only turn-in reward, standings and mission cleanup. Test declined/expired/restarted/relogged offers and concurrent players. Previous observed 8/8 completion is **not** evidence of every reward/cancel path or the cause of a later server exit; no native crash backtrace was available.

## 5. Destiny, warp, gates, docking, orbit, collisions and client state

**Files:** `src/eve-server/system/DestinyManager.cpp/.h`, `src/eve-server/ship/BeyonceService.cpp`, `src/eve-server/Client.cpp/.h`, `src/eve-common/EVE_MovementFormulas.h`, `src/eve-server/system/SystemBubble.cpp`, `src/eve-test/utils/MovementFormulaTest.cpp`, `tools/verify_crucible_range_and_warp.py`.

- Critical distinction: `CmdWarpTo` uses a **center-to-center** stopping range, while jump/docking/Follow checks use **surface separation** (subtract target and ship radii once). Explicit conversion fixes gate/target-radius fudge, duplicated ship radius, unsafe autopilot warps and inconsistent 0/10/100 km landings.
- Zero-distance gate/station warps aim for approximately **2,400 m surface clearance**, within the existing **2,500 m activation limit**. A safe terminal approach step cannot cross the collision surface. Selected longer ranges and autopilot defaults remain distinct; planets/moons/wormholes/bookmarks preserve specialized coordinate rules.
- Dock/jump requests remain queued across alignment and active warp, with pending-action cancellation on superseding movement commands. Destiny sends ordered final position, STOP, zero velocity and warp effect completion to reduce prediction drift. Normalize turns, protect against nonfinite headings/speed and prevent continuous idle stop-packet spam.
- Fixed an NPC orbit-radius formula that could predict inside hull clearance, producing one-per-second Guristas Terrorist invalid-orbit errors; valid radii now clamp to a safe geometric minimum. NPCs lacking `entityCruiseSpeed` can use genuine `maxVelocity`; immobile NPCs avoid endless orbit requests. Genuine unrecoverable NPC warnings are sampled rather than globally suppressed; pilot errors remain visible.
- Player/pilot orbit and turning changes include normalization and safe dot-product math. **Physical mesh collision, continuous sub-tick movement and client rendering frame-time spikes are not fully simulated or certified by these fixes.**
- **Reported client evidence:** On 19 Nov, operator confirmed warp animation, gate jumps/transit visuals and docking on the tested build. Refer to [navigation QA](WARP_GATE_LANDING_2500M_2026-11-18.md). Further test frigate/battleship/capital, gate variants, manual approach zero, autopilot, bookmarks, queued jumps, session changes, long idle, and high RTT.

## 6. Combat, missiles, turrets and drones

- Hardened turret hit probability against invalid state, missing ball data, zero distance, tracking/falloff and signature inputs; restored Crucible-appropriate weapon reach/ship warp attributes and guarded projectile, laser and hybrid math where touched.
- Improved missiles, defenders, smartbomb and other active-module interactions. Some bomb/doomsday content is still partial or requires an exact historical client/SDE verification; **do not claim complete weapon family parity**.
- Drone control/target permissions, bandwidth and controller references, attack/return/scoop/abandon cleanup and dynamic movement snapshots were corrected. Mining drone `DroneAI` now manages approach/orbit/harvest/return; `DestinyManager::ApplyMiningDroneStep` isolates its state changes while preserving the general Destiny command stream. Mining drones require playtests for distance, yield, ore split/merge, capacity, return and transfer, asteroid depletion and pilot disconnect.
- NPC belts use corrected security-range classification and off-grid warp-in. Engagement, pursuit, orbit and bounce behavior still need representative Guristas, Sansha and Angel NPC tests.

**Test coverage:** `MiningFormulaTest` and movement formulas plus `verify_subsystems_audit.py` and source guards exist; no C++ unit test substitutes for watching client packets, wreck contents and all live NPC ship classes.

## 7. Sovereignty, Infrastructure Hubs, POS and baseline upgraded sites

**Files:** `src/eve-server/system/sov/*`, `src/eve-server/pos/*`, `src/eve-server/inventory/Inventory.cpp`, `src/eve-server/system/cosmicMgrs/{DungeonMgr,BeltMgr,AnomalyMgr}.cpp`, `src/eve-common/EVE_Sov*.h`, `tools/verify_sov_pos_integrity.py` and migration files.

- Introduced a **DEV/ADMIN-only** `/sovindex5` and `/sovindex5 reset` for testing a real null-sec claim with TCU+IHub. Synthetic Military **1,000,000,000** points and Industrial **30,000,000** points plus **100 effective Strategic days** are tied to current claim/TCU. They are testing overrides, **not genuine activity progression**, and do not rewrite the TCU claim timestamp.
- Enlarged military/industrial columns and client data types so raw points do not overflow old 16-bit fields; adds lookup and list responses for development indices. Apply the override creation and widening migrations in timestamp order before relying on the command.
- The IHub upgrade installation validator supports Strategic (group **1016**), Industry (**1020**) and Military (**1021**) groups, including prerequisite/blocking fields, active flags, duplicates, ownership/Director permissions, and Supercapital Construction Facilities type **2009** with 100 effective strategic days.
- Enabled baseline Military-upgrade additional combat anomaly slots and Industry-upgrade gravimetric/mining sites using existing dungeon and ore distribution systems. **Not** full CCP tier-quality, spawn/decay or activity-index progression.
- POS work addresses force-field visibility for online large towers, default radius for missing static data, field persistence, harmonic/password behavior, sov-gated anchoring, bridge link cleanup and orphan metadata prevention. Source cleanup does **not** automatically delete historical orphan rows in an operator's database.
- POS research-lab slot provisioning/migration is present, but **remote/POS job installation and delivery are not yet functional**. Do not treat industry-line discovery as completed production/research gameplay.

**Staging QA:** test IHub all three upgrade families and prerequisites under correct sovereignty roles, tower bubble on/off and restart, bridge endpoints, destroyed/scooped structure metadata, Military/Industry anomaly spawning and minable ore; reset the GM override afterward. Read [sov index test plan](SOV_INDEX_LEVEL5_GM_TEST_2026-11-18.md) and [POS review](SOVEREIGNTY_POS_REVIEW_2026-11-17.md).

## 8. Industry: manufacturing, material research, time research and copies

**Files:** `src/eve-server/manufacturing/{Blueprint.h,FactoryDB.cpp,RamMethods.cpp,RamProxyService.cpp}`, `src/eve-common/EVE_RAM.h`, `src/eve-common/EVE_IndustryFormulas.h`, `src/eve-common/utils/EvEMath.cpp/.h` and `src/eve-test/utils/IndustryFormulaTest.cpp`.

- **ME research** (activity **4**) and **PE / Productivity-Time research** (activity **3**) implement station quote, activity/line/owner checks, timing, job installation, completion and persisted `invBlueprints.mLevel`/`pLevel` updates. This is legacy Crucible *level-based* research, not modern percentage TE.
- Removed an inappropriate post-2014 10-level exponential time model; research time increases linearly per old Crucible level, with bounded signed-byte blueprint level **127** and skill-level-based duration reductions. Validate against actual static-data base durations and client UI before balance sign-off.
- **Copying** (activity **5**) rejects BPC-to-BPC copying and invalid/zero/out-of-range licensed runs, charges production time for copy count multiplied by runs per copy, preserves parental ME/PE and sets BPC `copy` metadata. BPO unlimited-manufacturing-run sentinels are preserved.
- Quotes, material availability and inventory consumption use common total-quantity rounding and authorized BOM owner/location. Multiple stacks are considered; extra fixed materials (such as datacores in future activities) do not receive ME discounts. Client-provided authorized cost cannot override the server quote.
- Hardened personal vs corporation job listing, ownership, completion and output location; reject unrelated factory lines.
- **Invention activity 8 is intentionally blocked** before charges/consumption. No real T2 success roll, datacore/decryptor rules or T2 blueprint output is implemented. Do not remove the fail-closed guard solely to make the UI install button succeed.
- **Known design gap:** wallet payment, job scheduling, material deduction and output delivery are not one atomic transactional unit with verified rollback. Crash/restart and duplicate-delivery QA is mandatory. Corporate hangar roles and POS/remote installations need separate review.

**Acceptance:** personal station BPO ME 0→1→20, PE 0→1→20, two copies × five licensed runs, delivered BPC metadata after relog, denial of BPC re-copy and invalid materials, no unauthorized corp material consumption. Read [detailed audit](INDUSTRY_RESEARCH_COPY_INVENTION_AUDIT_2026-11-18.md).

## 9. NPC market, matching, pricing and contribution-only 100M orders

**Files:** `src/eve-server/market/{MarketBotMgr.cpp,MarketDB.cpp,MarketMgr.cpp}`, `tools/evedbtool/command_seed.go`, `sql/seed_and_clean/{seed_market.sql,seed_station_market.sql}`, `tools/tests/npc_buy_seed_unsigned_schema.sql`, `tools/verify_ore_mineral_100m.py` and `tools/verify_npc_buy_seed.py`.

- Existing fork work resolves NPC bid/sell reference pricing at **100% of `invTypes.basePrice`**, with an explicitly documented fallback for missing/nonpositive base price; improves settlement and seller ISK credit. The user previously confirmed player selling credited correctly, but all order-combination and market fairness cases are not verified.
- `EVEDBTool seed -buy-only` isolates NPC buy-order refresh; ordinary `seed -force` can refresh both sell and buy seed-managed orders. Unsigned MariaDB `orderRange` uses **0**, not negative sentinels; preserve station-local immediate matching and identifiable NPC owners. Ensure user/player orders are never deleted by seed refresh.
- **Contribution-only latest change:** new static regional/station and dynamic Trader Joe NPC mineral (group **18**), ore (**450–462, 467–469**) and ice (**465**) orders have `volEntered = volRemaining = 100,000,000` per type/station/order, for **both buys and sells**, at base reference prices. Other groups retain existing stock policies.
- The 100M bid exception applies only where unit price is at most **10,000,000 ISK**; the synthetic notional cap is up to **1 quadrillion ISK** (within tested `DECIMAL(20,2)`). Other items retain usual order cap/quantity. **This deliberately injects very large synthetic liquidity and is not neutral for a player-driven economy.**
- The new branch adds `verify_ore_mineral_100m.py` on both platforms, additional patched EVEDBTool smoke checks, and a MariaDB fixture validating Tritanium/Pyerite/Arkonor/Ice, overprice exclusion, order counts, idempotent refresh and player-order preservation. The merged workflow includes this alongside the newer industry checks.

**Operator warning:** no old NPC orders magically update on source deployment. Stop the market bot/server, back up MariaDB, run `evedbtool seed -force -dryrun` first, review selection, and use the exact staged procedure in [100M market runbook](NPC_ORE_MINERAL_100M_ORDERS_2026-11-18.md). `seed -force` replaces seed-managed sell and buy orders; `seed -buy-only -force` intentionally only refreshes identifiable NPC buys. Neither should be used against production before inspecting volume/ISK implications and player order counts.

## 10. Scanning and other client-facing mechanics

- Improved probe and directional/scanner math, result aggregation and safety guards around ranges, ship/celestial metadata and stale entities. The dedicated scanner audit and math tests are in the repository; live Crucible scan-result ordering, probe skill edge cases and map/client precision still need QA.
- Corrected agent journal response shape, mission briefing fallbacks, NPC portraits/media content type and inventory/office client interactions as described in archived subsystem reports.
- Review high-frequency Destiny packet behavior, market bot scheduling and NPC movement tick frequency in a release build under realistic entity counts. CI compilation is not a frame-pacing benchmark.

## 11. Schema and static-data migrations — staging operator sequence

**No migration has been run by this handover.** The contribution includes these five 18 November migrations; inspect each `Up` block and the production schema before execution:

| Order | Migration | Purpose and review gate |
| --- | --- | --- |
| 1 | `20261118070000-sov-development-index-overrides.sql` | TCU-bound DEV index override table; never rewrite real `claimTime` |
| 2 | `20261118093000-sov-development-index-point-width.sql` | Widen Military/Industry override points and normalize old test rows |
| 3 | `20261118120000-pos-industry-lines.sql` | POS industry line availability/provisioning; **does not** make remote/POS jobs fully functional |
| 4 | `20261118123000-crucible-static-attribute-corrections.sql` | Guarded static attribute corrections; compare installed SDE and existing custom data first |
| 5 | `20261118130000-crucible-capital-warp-speeds.sql` | Historical capital ship warp-speed corrections; check installed attributes and hulls |

**Deployment procedure:** (1) take an off-server full MariaDB backup and verify restore on staging; (2) record EVEDBTool build and migration ledger; (3) stop writes from server, Trader Joe and other workers; (4) run reviewed `Up` migrations in timestamp order **exactly once** with the supported migration runner; (5) inspect affected tables, column types, indexes, static attributes, active tower/IHub rows and sampled player assets; (6) deploy server/client-compatible artifacts and run scenario tests; (7) only if explicitly desired, audit/reseed NPC orders via documented dry-run. Migration files are not a substitute for reviewing a production database with custom changes.

**Never run `Down` automatically as a recovery step** on live sovereignty, industry, POS or static-data tables. A safe rollback may require restoring a known backup and corresponding binaries; the one-way seed refresh changes market order state and is not reversed by a Git revert.

## 12. CI and automated verification matrix

| Verification | Implemented coverage | Status boundary |
| --- | --- | --- |
| Linux GCC/Ninja + CTest + package | C++ compile, registered unit tests, regression guards, Linux artifact | Fork `master` passed [#44479434302](https://github.com/xrizonx/evemu_Crucible/actions/runs/44479434302) |
| Windows MSYS2 UCRT64 GCC + CTest + ZIP | Portable server/tools, DLL packaging, checksum, source guards | Same `master` run passed |
| Docker | Reproducible image build | Same `master` run passed |
| MariaDB integration | Unsigned `orderRange` seed, POS industry migration, static attributes, seeded market checks | Same `master` run passed |
| Contribution-specific 100M ore/mineral | Source guard on Linux/Windows; extended seeded MariaDB order tests | **Present on combined contribution branch; full combined CI unverified at handover time** |
| Gameplay source guards | Mission, Blockade, DED/Officer, drones, movement, industry, POS/IHub, scanner | Catches known regressions; comment-based checks previously yielded false positives |
| Direct Crucible client | Core warp animation, jump and docking reportedly verified | Broad combat, industrial, market, POS, drone, sovereignty and visual tests still outstanding |

**CI workflow location:** `.github/workflows/build-release.yml`. It dispatches on push to `master`, PRs targeting fork `master` and manual workflow dispatch, but **a push to the contribution branch alone does not automatically establish a new fork workflow success**; upstream PR workflow checks depend on the upstream repository. Reviewers should run the **combined contribution head**, not cite older `master` green tests as a substitute.

**Known CI history:** early industry compile failures from undefined `uint64` were fixed via `std::uint64_t`; source guards were updated to inspect actual fail-closed invention and capped ME/PE formulas rather than obsolete comments. Prior reruns passed. Do not reintroduce fragile comment-string tests.

## 13. Reviewer-specific acceptance matrix (suggested PR slicing)

| Slice | Key files / subsystem | Mandatory acceptance before merge |
| --- | --- | --- |
| A: Toolchain/core | CMake, Docker, workflows, DB/protocol, image server | Windows/Linux/Docker clean from contribution SHA, smoke server boot, packaging |
| B: Movement/NPCs | Destiny, Client, Beyonce, NPC AI, SystemBubble | Gate 0/10/100km, jump/dock, ship sizes, orbit stability, no movement spam |
| C: Missions | agents, SpawnMgr, DungeonMgr, mission and loot helpers | Offer/accept/kill/turn-in, 12/8 NPC counts, 50M/500M exactly once, no targetless spawn |
| D: Drones/combat | DroneAI, Drone, modules, turret/missile, TargetManager | Target ranges, harvesting & transfer, return/scoop, invalid combat inputs |
| E: Sov/POS | sovereignty, IHub, TCU, tower, bridge, migrations | Genuine TCU prerequisites, DEV override reset, upgrade requirements, tower field, rollback |
| F: Industry | RAM/FactoryDB/Blueprint, formulas | BPO ME/PE/copy, owner and BOM isolation, no duplicate output, invention denied |
| G: Market/seed | MarketMgr/MarketBot, EVEDBTool, 100M SQL + MariaDB | Player order preserved, ISK credited, exact volumes, price cap and reseed idempotency |

PR #335 is much larger than a typical reviewable patch. It may be better for upstream maintainers to request separate PRs or cherry-picked commits per slice, retaining this draft as the reference integration branch. Do not independently merge only parts of closely coupled Destiny/drone updates without preserving their public API and tests.

## 14. Runtime QA checklist

1. **Boot/restart:** staging database backup, schema migration ledger, clean boot, error log diff, image server/local SQL data loading.
2. **Navigation:** 0/10/100km gate warp, station docking, autopilot, queued Jump/Dock during warp, correct client warp effect, 60-second idle without packet spam.
3. **NPC combat:** Guristas Terrorist (the original 1Hz invalid-orbit reproduction), several NPC sizes/classes, actual orbit and damage; no silent target loss.
4. **Missions:** Blockade 12 and elite 8 NPCs, unique mission persistence, 50M vs 500M payouts, officer loot tags, no untargetable Domination Grigori/True Sansha.
5. **Drones:** mining target, approach, orbit, harvest, return, cargo transfer and relog; combat drone targeting/bandwidth, recall/scoop/destroy.
6. **Industry:** ME & PE BPO research, BPC copies × licensed runs, job cost, delivered copies, owner/roles; invention denied without debit.
7. **Market:** before/after NPC and player order count, 100%-base prices, 100M volume for qualified ore/ice/minerals, cap rejection, seller payout, force-dryrun preview.
8. **Sov/POS:** admin level-5 override and reset, IHub prerequisites, supercapital facility eligibility, tower force field and persisted harmonic/password, anomaly/ore sites.
9. **Client resilience:** disconnect while warping, docking, harvesting and mission combat; retry/duplicate delivery; crash logs and player inventory reconciliation.
10. **Performance:** median/p95 frame-time, entity count, movement update rate, CPU and database query rates; do not infer eliminated micro-stutters from compiler tests.

Collect reproducible client and server timestamps, character/ship/type/solar system, entity ID, packet trace if available, SQL query logs, and precise steps for every remaining failure.

## 15. Remaining defects, gaps and risk classification

**Blocking for full feature claims:**
- Blueprint invention cannot generate genuine T2 BPCs or probabilistic outcomes; it is correctly disabled.
- Industry jobs do not yet have proven atomic install/payment/material/output transactions and rollback across crashes.
- POS/remote industry jobs and complete sovereignty military/industry index progression are unfinished.
- Several mission-room scripting and trigger/gate flows, specialty weapons, complete collision mesh physics and frame-time micro-stutter elimination remain incomplete or not thoroughly playtested.

**Potentially high operational impact:**
- Re-seeding creates 100M synthetic NPC market liquidity and can replace seed-managed orders; backup, dry-run and explicit economy-policy approval are required.
- SOV/POS and static-attribute migrations can affect existing installation data; inspect deployed SDE and user-owned structures first.
- Very large upstream PR diff creates review/merge and regression risk. Green CI is necessary but not sufficient for gameplay parity.

**Known reproducibility evidence:** operator explicitly confirmed core warp/jump/docking client workflow and player ISK credit on sale to NPC buy order. Other game behavior entries describe committed source and unit/source-check intent, not independently observed gameplay completion.

## 16. Ownership, rollout, rollback and next maintainer actions

**Code reviewers:** review source slices A–G, request changes to unsafe or untested paths, and require CI against the exact contribution head; check file inventory and method interfaces against upstream master.

**Database operator:** validate backup restore, migration state, market dry-run and player assets using staging; authorize any seed refresh separately. Never treat documentation publication as permission to mutate a live DB.

**Crucible client QA:** perform the runtime QA checklist in section 14 as written, record reproducible failures, especially mission settlement, BPO research delivery, mining drone ore returns, and generated NPC order volumes.

**Suggested promotion:** maintain PR #335 as **draft** until upstream review gates and combined CI are green. Optionally split into smaller reviewable PRs. Preserve existing commit lineage and historical documentation; do not force-reset the contribution branch onto master.

**Rollback strategy:** keep prior executable + SQL asset package, verified database backup and migration ledger; redeploy previous binaries for code-only regression where schema is compatible. Reverse individual staging-only upgrades only after reviewing migration `Down` and live data ownership. Market reseeds and schema migrations **cannot** be safely reverted by git reset alone.

### Documentation cross-links

- [Full 19 Nov cumulative subsystem patch notes](UPSTREAM_PATCH_NOTES_2026-11-19.md)
- [Exact upstream changed-file inventory](UPSTREAM_PR335_FILE_INVENTORY_2026-11-19.md)
- [Original 15 Nov handover](UPSTREAM_PULL_REQUEST_REVIEW_2026-11-15.md)
- [17 Nov expedition and market handover](UPSTREAM_PULL_REQUEST_REVIEW_2026-11-17.md)
- [18 Nov sovereignty/POS dossier](UPSTREAM_PULL_REQUEST_REVIEW_2026-11-18.md)
- [19 Nov warp landing/client QA](WARP_GATE_LANDING_2500M_2026-11-18.md)
- [Industry research and invention audit](INDUSTRY_RESEARCH_COPY_INVENTION_AUDIT_2026-11-18.md)
- [100M market seed deployment](NPC_ORE_MINERAL_100M_ORDERS_2026-11-18.md)
- [SOV/Infrastructure Hub test](SOV_INDEX_LEVEL5_GM_TEST_2026-11-18.md)
- [Fork chronological patch log](../PATCH_NOTES.md)
