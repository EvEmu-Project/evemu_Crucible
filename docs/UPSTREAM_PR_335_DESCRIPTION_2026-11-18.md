## 18 November 2026 — NPC ore/mineral stock set to 100 million

New NPC ore/ice/mineral **buy and sell** orders now start at **100,000,000 units each** per type/station through EVEDBTool and Trader Joe. Existing NPC and all player orders are unchanged until an intentional operator refresh; other item amounts and 100% `invTypes.basePrice` pricing are preserved. The ore/mineral order-value safety ceiling is 10M ISK per unit. See the [deployment/test guide](https://github.com/xrizonx/evemu_Crucible/blob/contrib/upstream-crucible-maintenance-2026-11-15/docs/NPC_ORE_MINERAL_100M_ORDERS_2026-11-18.md). Linux/Windows and MariaDB checks were added; a successful live build and client QA are still required.

# Upstream PR #335 — patch-note update for 18 November 2026

> **Ready to copy into the top of [upstream draft PR #335](https://github.com/EvEmu-Project/evemu_Crucible/pull/335).** The contributor's GitHub integration does not have permission to edit or comment on this upstream-owned PR; this text is committed to the contribution branch for the PR author to paste. Preserve the prior PR history underneath.

## Current contribution snapshot

| Field | Value |
| --- | --- |
| Source branch | `xrizonx:contrib/upstream-crucible-maintenance-2026-11-15` |
| Historical master-import merge commit | `32eb2e5cd0e6b8c327268ea0b6cd3f80348332e6` (PR's live head will be newer) |
| Integrated fork `master` | `2d5cb4d2c7dbd0b33b70467e5caf3021fc52a713` |
| Upstream `master` snapshot | `f55b488a006e3090470d3b481aee47a1d7a0dade` |
| GitHub PR comparison before the 100M market patch | 748 commits; 291 changed paths; +21,109 / −3,405 lines (historical baseline; see live PR for current totals) |
| Master delta imported since 17 Nov handoff | 50 newer master commits; 89 changed paths |

**Complete [patch notes](https://github.com/xrizonx/evemu_Crucible/blob/contrib/upstream-crucible-maintenance-2026-11-15/PATCH_NOTES.md)** (13–18 Nov) and **[new 18 November reviewer dossier](https://github.com/xrizonx/evemu_Crucible/blob/contrib/upstream-crucible-maintenance-2026-11-15/docs/UPSTREAM_PULL_REQUEST_REVIEW_2026-11-18.md)** (all 89 paths, migrations, scope, CI and live-test gaps). Earlier [17 November](https://github.com/xrizonx/evemu_Crucible/blob/contrib/upstream-crucible-maintenance-2026-11-15/docs/UPSTREAM_PULL_REQUEST_REVIEW_2026-11-17.md) and [15 November](https://github.com/xrizonx/evemu_Crucible/blob/contrib/upstream-crucible-maintenance-2026-11-15/docs/UPSTREAM_PULL_REQUEST_REVIEW_2026-11-15.md) dossiers are retained.

## Key changes included in this updated draft

- **Sovereignty and IHub:** TCU/IHub-bound DEV/ADMIN `/sovindex5` testing and reset, corrected 32-bit Military/Industrial points, 100 effective Strategic days, installation checks for Strategic/Industrial/Military families including **Supercapital Construction Facilities** (type 2009), alliance/active/duplicate/prerequisite validation, and baseline upgraded military combat anomalies and gravimetric ore sites.
- **POS and force fields:** Tower bubble appearance and radius defaults, password/harmonic persistence, SBU and anchoring geometry, ownership authorization, sovereignty claim integrity and jump-bridge reciprocal link safety; read-only orphaned POS record investigation.
- **Scanning and commands:** Directional scan math, post-jump scan session rebinding, probe state guards and scoped `/dogma` CONTENT/DEV or ADMIN access with permission-mask fixes.
- **Missions and NPCs:** Existing **Blockade 65535** (12 NPCs / 50M ISK) and independent **DED & Officer Expedition 65534** (8 NPCs / 500M ISK once at turn-in), SDE-backed faction/officer/deadspace wreck loot, mission-only True Sansha exclusion, encounter pocket cleanup and test coverage.
- **Economy and inventory:** 100%-base NPC sell/buy pricing, MariaDB-compatible NPC buy orderRange=0, EVEDBTool seed guards, station/office/container hardening and legacy-compatible image-server changes.
- **Combat, navigation, tests:** Existing drone target/attack, missile/turret and warp/orbit/approach fixes; modern Windows UCRT64/Linux/Docker CI, CTest guards and diagnostics.

**Not included:** [fork PR #41](https://github.com/xrizonx/evemu_Crucible/pull/41) for untargetable Domination Grigori and [draft PR #42](https://github.com/xrizonx/evemu_Crucible/pull/42) for mining-drone flight, as neither was merged into fork `master` at the captured snapshot.

**CI:** Integrated fork [master run #44346201327](https://github.com/xrizonx/evemu_Crucible/actions/runs/44346201327) passed for the exact source snapshot. The contribution branch and upstream PR require independent CI and code review. Client gameplay, POS field visuals, IHub upgrades, supercapital manufacturing, site progression, market transactions and repeated mission/reward flows are **not** proven by CI alone.

**Migration caution:** Added sovereign development index, point-width, POS industry, static attribute and capital-warp-speed SQL migrations are source files only; no migrations have been executed through this PR. Operators must back up their data and apply/review them in staging before deployment. Avoid force-seeding NPC market orders without inspection.

This PR remains **draft**, not accepted for upstream merge. Given its broad cumulative scope, upstream maintainers may request smaller subsystem-specific reviews.
