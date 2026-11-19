# EVEmu Crucible maintenance and gameplay improvements — updated developer handover (19 November 2026)

> **DRAFT / Maintainer review requested, not ready for automatic upstream merge.** This is the accumulated fork development proposed against `EvEmu-Project/evemu_Crucible:master`. It is a broad integration PR with experimental and incomplete gameplay paths. A green build is not a guarantee of bug-free client behavior or production data safety. Please review by subsystem; smaller cherry-picked PRs may be more manageable.

## Current source, history and preservation

- **Source:** `xrizonx/evemu_Crucible:contrib/upstream-crucible-maintenance-2026-11-15`; **upstream base:** `EvEmu-Project/evemu_Crucible:master`.
- **19 November integration merge:** `9f8929de3387c9ae471e768556eefc8657c62f5f`, preserving original contribution parent `9b2aca90917fabdc78ae0526a220908b8464f2da` and latest fork master `e2f8e14bca5b50bb1f8ff055f5f16a00d01b7c27` (merged fork PR #43).
- **Scope at integration (pre-handover documentation):** **811 commits, 303 files changed, +22,836/−3,701 lines** compared with upstream's older baseline. Live GitHub counts can increase after these documentation commits; all commits are cumulative, not new as of 19 November.
- **Preservation audit:** zero deleted paths from either parent, all 13 unique contribution-only paths preserved, six contribution market files deliberately overlaid on fork master, and both overlapping files (`.github/workflows/build-release.yml`, `PATCH_NOTES.md`) reconciled to retain both CI/test families and dated notes. **No force push.**

**Developer documentation (recommended order):**

1. [19 November **complete developer handover**](https://github.com/xrizonx/evemu_Crucible/blob/contrib/upstream-crucible-maintenance-2026-11-15/docs/UPSTREAM_DEVELOPER_HANDOVER_2026-11-19.md) — 16 sections covering architecture, source ownership, subsystem behavior, CI, live QA, migrations, risks, rollback and acceptance.
2. [19 November **cumulative patch notes**](https://github.com/xrizonx/evemu_Crucible/blob/contrib/upstream-crucible-maintenance-2026-11-15/docs/UPSTREAM_PATCH_NOTES_2026-11-19.md) — detailed changes across movement, NPCs, missions, drones, combat, market, sovereignty/POS, industry and tools.
3. [Complete **303-path changed-file inventory** at the integration snapshot](https://github.com/xrizonx/evemu_Crucible/blob/contrib/upstream-crucible-maintenance-2026-11-15/docs/UPSTREAM_PR335_FILE_INVENTORY_2026-11-19.md) — every file grouped with diff status and line counts.
4. [Prior 18 November upstream PR description and chronological record](https://github.com/xrizonx/evemu_Crucible/blob/contrib/upstream-crucible-maintenance-2026-11-15/docs/UPSTREAM_PR_335_DESCRIPTION_2026-11-18.md) — retained so the original review history is not lost.

## Subsystem review summary

| Area | Implemented work to review | Key risk / verification boundary |
| --- | --- | --- |
| Build/release | CMake/GCC/Boost/MariaDB portability, Windows UCRT64+DLL ZIP, Linux tarball, Docker, patched EVEDBTool, CTest, MariaDB fixture | Run **combined PR-head CI** rather than relying on older master CI |
| Core stability | Python/DB bounds, null/entity/iterator protections, station/inventory reloads, image server and legacy agent portraits | Live station/logout/relog, native crashes, protocol compatibility |
| Movement | Center vs surface-distance warps, 2,400m gate/station clearance (2,500m activation), queued jump/dock, Destiny packet state, idle/jitter guards, NPC safe orbit | User verified warp/jump/dock for a tested build; broad hull, collision and frame pacing still require QA |
| Combat/NPCs | NPC security-class spawns, belt warp-in, turret/missile/defender/smartbomb and target safety | Historical balance and specialized weapons not completely verified |
| Drones | Controller, bandwidth, scoop/recall, targeting and mining drone flight/harvest/return via `ApplyMiningDroneStep` | Real ore transfer, capacity, asteroid depletion, disconnect/ship-loss QA |
| Missions | Agents, pockets/beacons, initial template NPCs, Blockade `65535` **12 NPCs/50M ISK** and DED/Officer `65534` **8 NPCs/500M ISK** | Confirm one-time rewards, cleanup and client mission objectives. Exclude True Sansha and Domination Grigori only from elite mission |
| NPC market | 100%-basePrice bid/ask, verified seller credit path, patched buy-only seed, preserved player orders | High-liquidity seed behavior, historical seed cleanup and production DB safety |
| NEW 100M liquidity | Minerals group 18, ore 450–462/467–469, ice 465: **100,000,000 units on each NPC buy/sell order per type/station** | Bids eligible only with reference price ≤10M ISK/unit; at most 1 quadrillion ISK synthetic notional per order; requires opt-in reseed |
| Sovereignty/POS | DEV-only `/sovindex5` test overrides, full IHub upgrade-family validation incl supercapital facility, baseline military/industrial sites, tower force-field fixes | No complete activity progression; POS remote industry and full site economics incomplete; staged migrations |
| Industry | Crucible ME research (4), PE/time (3), copying (5), material/owner/price isolation and persisted BPCs, CTest | **Invention (8) deliberately disabled**; no proven job transaction atomicity; needs client QA |
| Scanning and ancillary | Scanner/probe math, agent journal, office/inventory, privilege checks and safety logs | Client-specific scanner behavior and visuals require live tests |

### Market-order behavior needing explicit upstream approval

The contribution-only market delta modifies `tools/evedbtool/command_seed.go`, `src/eve-server/market/MarketBotMgr.cpp`, `sql/seed_and_clean/{seed_market,seed_station_market}.sql`, MariaDB fixture and Python guard. New ore/ice/mineral NPC bids and asks have **100M initial volume** at **100% basePrice**, with 10M ISK per-unit upper bound on high-volume bids. Other groups keep their existing behavior.

**Deployment does not mutate existing orders.** Operators must separately decide whether to refresh: `evedbtool seed -force` refreshes seed-managed buy **and sell** orders; `seed -buy-only -force` refreshes identifiable NPC bids only. Take a full DB backup, inspect `seed -force -dryrun`, and reconcile player orders and balances. [Deployment runbook](https://github.com/xrizonx/evemu_Crucible/blob/contrib/upstream-crucible-maintenance-2026-11-15/docs/NPC_ORE_MINERAL_100M_ORDERS_2026-11-18.md).

## 19 November — Docker market-load performance

The Crucible UI uses nested best-ask queries and separate regional item sell/buy queries on `mktOrders`. Four additive composite indexes now target their region/station/system filters, along with the bot's owner/issued expiry sweep. Trader Joe now performs one expiry pass per refresh and batches guarded DELETEs rather than issuing one per order; Docker Compose waits for healthy MariaDB/InnoDB initialization. The [performance runbook](https://github.com/xrizonx/evemu_Crucible/blob/contrib/upstream-crucible-maintenance-2026-11-15/docs/DOCKER_MARKET_PERFORMANCE_2026-11-19.md) provides read-only EXPLAIN/slow-query diagnostics and rollback steps. MariaDB CI confirms idempotent index DDL, preserved player orders, and actual index selection for representative queries; **real user latency has not been measured**. [Branch CI #44498635401](https://github.com/xrizonx/evemu_Crucible/actions/runs/44498635401).

## Database changes — review before any execution

Five 18 November migrations and one optional 19 November market-index migration are present; **none was applied to a production database by updating this PR**:

| Sequence | SQL migration | Intent |
| --- | --- | --- |
| 1 | `20261118070000-sov-development-index-overrides.sql` | Claim/TCU-bound DEV index overrides |
| 2 | `20261118093000-sov-development-index-point-width.sql` | Correct Military/Industry point widths and old test values |
| 3 | `20261118120000-pos-industry-lines.sql` | Lab line provisioning, **not** completed remote/POS jobs |
| 4 | `20261118123000-crucible-static-attribute-corrections.sql` | Guarded historical static-data corrections |
| 5 | `20261118130000-crucible-capital-warp-speeds.sql` | Capital-hull warp attributes |
| 6 | `20261119113000-market-order-lookup-indexes.sql` | Four additive market query/Trader Joe expiry indexes; review and apply off-peak |

Use staging + verified MariaDB backup, inspect the actual `Up` SQL and your SDE, migrate in timestamp order once, validate before server rollout, and **never treat automatic `Down` or a Git revert as a DB restoration plan**. `/sovindex5` uses raw test-only points and must be reset after testing. POS orphan records require a separate manual reference audit.

## Verification evidence and remaining gates

- **Confirmed on fork master `e2f8e14`:** [Actions #44479434302](https://github.com/xrizonx/evemu_Crucible/actions/runs/44479434302) succeeded on **Linux GCC + CTest + packaging, Windows UCRT64 GCC + CTest + ZIP, Docker, and MariaDB integration**.
- **Historical successful fork PR CI:** [#44473762678](https://github.com/xrizonx/evemu_Crucible/actions/runs/44473762678) for the immediate earlier fork head. The final PR #43 source was merged into master with identical code tree.
- **Verified:** the combined contribution branch including 100M market changes, new MariaDB market indexes and newer industry guards passed [Actions #44498635401](https://github.com/xrizonx/evemu_Crucible/actions/runs/44498635401) on Linux/Windows with CTest, Docker and MariaDB. The fork's workflow now also triggers on pushes to this long-running contribution branch.
- **Observed gameplay:** the operator reported warp animation, gate jump/visuals and docking functioning; seller ISK credit for a tested NPC buy-order sale. Other described features remain code-level/CI-backed until individually playtested.
- **Open blockers:** invention output, fully atomic industry transactions, POS/remote lab work, real sovereignty activity points/decay, full mission room triggers, some specialty weapons, continuous collision physics and frame-time stutters.

## Suggested upstream review order

1. Toolchain and crash-safety foundation, with exact-head Windows/Linux/Docker CTest.
2. Destiny + NPC movement + drones as a dependency-aware slice.
3. Security agent missions, Blockade and DED/Officer loot/settlement.
4. Market and EVEDBTool including 100M opt-in liquidity decision.
5. Sovereignty/POS migrations and client state behavior.
6. Industry ME/PE/copying with explicit invention and POS lab limitations.
7. Scanner, inventory and image server regressions.

Please keep this PR in **draft** until subsystem reviewers approve and the combined CI and game-client regression matrix are complete. The historical review documents are preserved; the full [handover](https://github.com/xrizonx/evemu_Crucible/blob/contrib/upstream-crucible-maintenance-2026-11-15/docs/UPSTREAM_DEVELOPER_HANDOVER_2026-11-19.md) contains owner/test/rollback detail. **No upstream merge, database migration, market reseed or live deployment was performed by preparing this contribution.**
