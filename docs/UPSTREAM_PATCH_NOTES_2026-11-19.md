# Upstream draft PR #335 — cumulative patch notes, updated 19 November 2026

**Review target:** [EvEmu-Project/evemu_Crucible#335](https://github.com/EvEmu-Project/evemu_Crucible/pull/335), currently **draft**, not merged upstream.
**Fork source:** `xrizonx/evemu_Crucible:contrib/upstream-crucible-maintenance-2026-11-15`.
**Integration point:** source commit `9f8929de3387c9ae471e768556eefc8657c62f5f` combines fork `master` `e2f8e14bca5b50bb1f8ff055f5f16a00d01b7c27` and the previous contribution branch `9b2aca90917fabdc78ae0526a220908b8464f2da`. Cumulative upstream scope at this pre-documentation snapshot: **811 commits / 303 changed paths / +22,836/-3,701 textual lines**.
**Companion review:** [full developer handover](UPSTREAM_DEVELOPER_HANDOVER_2026-11-19.md) and [exact changed-file inventory](UPSTREAM_PR335_FILE_INVENTORY_2026-11-19.md).

> These patch notes summarize cumulative fork development **13–19 November**, not a single atomic release. Build success, source regression checks, operator observations, incomplete features and risky database actions are identified separately. The earlier dated notes in `PATCH_NOTES.md` and review dossiers are retained as historical snapshots; superseding status is below.

## 19 November — Integration and client navigation sign-off

**Integrated:** fork PRs [#41](https://github.com/xrizonx/evemu_Crucible/pull/41), [#42](https://github.com/xrizonx/evemu_Crucible/pull/42), and [#43](https://github.com/xrizonx/evemu_Crucible/pull/43) are **already merged into fork master** and are now inherited by the contribution branch. In upstream PR #335 they remain **proposed changes**.

- Restored original branch-specific 100M ore/mineral seeding, original upstream handovers and prior contribution commits while importing 47 previously missing fork-main commits; branch is now 0 behind fork `master` at the integration point.
- Resolved the only two simultaneously modified files, `.github/workflows/build-release.yml` and `PATCH_NOTES.md`, preserving both market and industry verification and both patch histories. No code-only parent file deleted, and a two-parent merge retains the complete Git ancestry.
- Fork master [CI #44479434302](https://github.com/xrizonx/evemu_Crucible/actions/runs/44479434302) passed Linux GCC/CTest, Windows UCRT64 GCC/CTest/portable packaging, Docker and MariaDB. **CI for the combined upstream contribution head has not been independently demonstrated**.
- Operator reported **working Crucible warp animation, stargate jumps and docking** on the tested fork build; not a blanket guarantee that every ship and geometry is jitter-free.

## Navigation, Destiny and ships

- Corrected client/server distinction between **center-based warp stop distance** and **surface separation for Follow, Jump and Dock**. Target and ship radii are each included exactly once; remove legacy gate `radius/3` and duplicate ship-radius offsets.
- Warp at zero aims for **2,400m surface gap**, within the **2,500m** gate/docking interaction threshold. Preserve selected 10/30/100km distances, bookmarks and original planet, moon, wormhole special cases; autopilot uses its configured range without unsafe center/surface mixing.
- Implemented queued gate-jump/dock approaches through turn and warp alignment; cancel when the player orders a superseding maneuver. Correct heading normalization and stalled initial opposite-heading alignment.
- Corrected Crucible Destiny WARP ball serialization and sector state for ships, NPCs and drones. Update final landing position, movement STOP, zero velocity and warp completion together; restart arrival protection and cloak state appropriately.
- Prevent full-tick gate/station penetration and repeated stationary CmdStop broadcasts; guard invalid speed/headings against NaN drift. Actual velocity replaces acceleration-time progress in reported speed for orbit/target prediction.
- Repaired underestimating NPC orbit math that could predict inside physical hull clearance at 10–20km, including Guristas Terrorist repeated invalid orbit errors. Fallback to configured NPC maxVelocity only if cruise speed absent, avoid stationary NPC retry loops, and sample unrecoverable NPC warnings without suppressing genuine player errors.
- Added `MovementFormulaTest`, source-level warp/ship-range assertions and a navigation client test checklist. **Remaining:** generic mesh collision physics, every capital-hull interaction, latency/packet interpolation and true frame pacing.

## Combat, NPC AI and weapons

- Fixed NPC spawn security-class scaling, belt rat faction selection and off-grid spawn warp-in; safer bubble lifetime and NPC target state reduce disappearing/untargetable spawns.
- Added or corrected turret to-hit validation for zero distance, falloff, tracking, signatures, empty pilot/ship state and missing Destiny data; improved Crucible-era hybrid/projectile/laser, missile, defender and smartbomb behavior in the touched modules.
- Hardened missile/drone/weapon damage-state handling, module range checks and sentry/NPC target sequencing. Avoid invalid physics inputs and null damage/owner references.
- Specialty weapons (including bombs and doomsdays) may remain partial; source changes must not be advertised as full CCP parity. Test each module on authentic Crucible SDE and client with damage numbers and actual ammo usage.

## Mining and combat drones

- Corrected range/ownership and controller checks for drone commands, control distance, target queue, scoop/abandon, ship destruction cleanup and bandwidth use. Preserve drone model/control identity when the owner docks, disconnects or loses the ship.
- Repaired mining drone approach-to-asteroid flight, orbit state, laser activation range, returned mined-unit tracking, ore capacity and transfer/merge in the home ship. Updated `DroneAI`, `Drone` and `DestinyManager::ApplyMiningDroneStep` plus Destiny snapshot fields.
- Added source regression protections and mining math CTests. **Needs live QA** on multiple drone types, asteroid depletion, full cargo, split stacks, recall mid-cycle, relog and concurrent drones. Fork PR #42 is merged but not a proof that all client drone animation or yield behavior is correct.

## Missions and agents

- Added/strengthened security-agent mission selection and progression, handling active offers per agent and rejecting expired/invalid offers. Improved standing, journal and mission-briefing responses, bonus deadline handling and agent records missing optional character/station metadata.
- Implemented initial mission encounter pockets/beacons with NPC spawn/group metadata, objective kill progress, cleanup and first-room dungeon-template spawning where usable; generic faction fallback otherwise.
- Synthetic **The Blockade** (`missionID 65535`): **12 NPCs**, **50,000,000 ISK** base mission payout, separately tracked, with final-hostile fallback and objective lifecycle checks.
- Synthetic **DED & Officer Expedition** (`missionID 65534`): **8 NPCs** drawn from real SDE faction, DED, overseer and officer classes; **500,000,000 ISK** one-time successful turn-in, **120-minute** offer deadline. Preserve faction/officer/deadspace Module wreck drops based on actual SDE meta groups rather than fake item IDs.
- Exclude name-matched **True Sansha** and **Domination Grigori** from the elite mission's potential untargetable NPCs. These filters **must not** remove ordinary Angel/Domination officers, other Sansha NPCs or NPCs spawned outside mission 65534.
- Hardened beacon registration, offer-owned cleanup, partial dead-spawn tracking, packet/state transitions, concurrent character access, duplicate pay protection and null bubble handling.
- Regression checks cover mission counts, group eligibility, one-time payout source and compatibility. **Incomplete:** all mission-specific triggers, multi-room gates/unlocks, non-kill mission objectives and complete reward balance. Reproduce previously observed post-8/8 server exit with native symbolized trace before claiming it resolved.

## Market pricing, NPC bids/asks and seller settlement

- Changed NPC market asks and scripted NPC buys to **100% `invTypes.basePrice`** (with documented fallback on unavailable/nonpositive static price); preserved player orders and normal settlement. Fixes in matching/buy and seller credit paths were operator-tested for at least one sale.
- Added patched EVEDBTool `seed -buy-only` isolation, visible dry-run SQL, correct unsigned MariaDB orderRange=0 and station-local matching. Guard against excessive bid pseudo-escrow and unrelated player-wallet updates.
- New contribution-specific feature: **100,000,000 units per NPC buy or sell order per type/station** for minerals `groupID 18`, ores `450–462, 467–469` and ice `465`. Both `volEntered` and `volRemaining` are initialized to 100M; other item groups keep their existing quantities and market policy.
- Only ore/mineral/ice bids with unit reference price ≤ **10,000,000 ISK** get 100M size, capping synthetic notional value at **1 quadrillion ISK**. Other products use the existing ≤10-trillion-ISK policy and normal quantity rules; high-price exceptions are skipped, not truncated silently.
- Updated five generation paths: patched regional EVEDBTool seed, `seed_market.sql`, `seed_station_market.sql`, `MarketBotMgr.cpp` dynamic Trader Joe bids and asks. Updated MariaDB fixture and `verify_ore_mineral_100m.py` to cover prices, amounts, idempotency, player order isolation and risky priced-out ore.
- **Important deployment effect:** source import does not rewrite previously placed NPC orders. `seed -force` deliberately refreshes seed-managed bids **and** asks, while `seed -buy-only -force` refreshes only identifiable NPC bids. **Never issue force operations without a database backup, dry-run, item-volume review and operator approval.**

## Industry, blueprint research, copying and manufacturing

- Validate blueprint owner/location, permitted factory or laboratory category/group and selected activity (Manufacturing 1, PE/Time 3, ME 4, Copying 5). Tighten personal/corporation job and output ownership; enforce legitimate BOM inventory location and hangar flag.
- Correct ME/PE research-skill time modifiers (skills decrease, not increase time), replace erroneous 2014 exponential 10-level formula with **linear pre-Crius research time**, bound existing blueprint signed-byte levels at **127**, and guard calculated time overflow/nonfinite values.
- Fix `Blueprint::GetPE` truncation, large copy-time base data, job rent time for **copy count × licensed runs**, zero/negative/out-of-bounds run counts and original BPO unlimited run sentinel preservation.
- Persist BPC `copy` bit with `mLevel`/`pLevel`/remaining runs; ensure quote materials, available multi-stack inventory and consumed amounts use compatible shared rounding. Do not discount fixed extra materials; prevent client-side installation price override.
- Correct personal vs corporate job-list exposure, completion owner/station authorization, and output destination checks.
- **Invention activity 8 remains disabled on purpose.** The pre-existing completion code did not create a genuine T2 BPC or calculate production outcome. Do not enable it until complete success/failure, datacore/decryptor use, item generation, persistence and rollback are implemented.
- Add `IndustryFormulaTest`, `EVE_IndustryFormulas.h` and Windows/Linux `verify_industry_workflows.py`. Test personal station BPO ME, PE, copies and relog; **non-transactional wallet/material/job/output transitions remain a concern** and are not fixed by source guards. POS/remote industry installation is not ready.

## Sovereignty, infrastructure upgrades and POS

- Added **DEV/ADMIN-only** `/sovindex5` and `reset` tied to an existing alliance TCU/IHub, without altering real `claimTime`: Military 1,000,000,000 points, Industrial 30,000,000, Strategic 100 effective days, to simulate development Level 5. The feature is a test override, not real usage-based level progression.
- Widened stored point fields and response widths; provisioned indexed lookup/list services and claim-bound override invalidation and removal.
- IHub upgrade validation now handles Strategic (group 1016), Military (1021) and Industrial (1020) family installation, prerequisites, blocking, active flags and corporate/alliance roles. Supercapital Construction Facilities type 2009 requires effective Strategic 100 days.
- Added baseline upgraded military combat anomalies and industrial gravimetric/mineable ore sites. Not a full CCP rarity/quality, automatic activity decay or upgraded wave-content system.
- Fixed POS tower force-field missing bubble, password/harmonic persistence, default radius where static data invalid, shield packet compatibility, and structure deletion metadata and jump-bridge relationship cleanup. Old orphan records are only diagnosed, not automatically dropped.
- POS industry slots now have a migration and basic provisioning but **no complete POS or remote research/production job loop**. Module/IHub activity still requires client and restart testing.

## Scanning, inventory, station and image server

- Scanning and probe corrections include safe geometry, result sorting/range, probe iteration and scanner math test coverage.
- Hardened station/office, inventory moves, container assembly and item ownership/capacity; avoid stale loaded state after system unload, and station null dereferences during clone/pod respawn.
- Improved image server fallback for missing legacy NPC portraits and corrected local image MIME/directory behavior; removed unverified experimental portraits. Legacy Crucible redirects remain client-specific.

## Reliability and error handling

- Fixed search DB column mismatch, database result bounds, iterator invalidation, nullable packets, unsafe object lookups, static compiler/runtime compatibility and inventory/service exceptions.
- Strengthened bubble/session lifetime and ship move state handling and introduced targeted diagnostics for incomplete NPC/docking/research conditions. Avoid globally muting critical errors; sample unavoidable NPC warning spam only after recovery/stop logic is exhausted.
- Updated test scripts to check **semantics rather than stale comments** after the C++ implementation evolves; corrected an invalid C++ `uint64` alias to portable `std::uint64_t`.

## Files, migrations and CI

New or updated repository sections include `.github/workflows/build-release.yml`; `src/eve-common`, `src/eve-core`, `src/eve-server`, `src/eve-test`, `tools`, `sql`, `docs`. The complete **303-path pre-handover inventory** is available as an exact per-file appendix. Archive review snapshots for 15, 17 and 18 Nov are preserved, but many of their source SHA/count/status statements are historical.

Five SQL migrations require an explicit staging schema review, backup and application in chronological order when the corresponding features are needed:

1. `20261118070000-sov-development-index-overrides.sql`
2. `20261118093000-sov-development-index-point-width.sql`
3. `20261118120000-pos-industry-lines.sql`
4. `20261118123000-crucible-static-attribute-corrections.sql`
5. `20261118130000-crucible-capital-warp-speeds.sql`

**Verified parent evidence:** fork `master` Windows + Linux builds and CTest, Docker and MariaDB integration all passed [#44479434302](https://github.com/xrizonx/evemu_Crucible/actions/runs/44479434302). **Required:** independently test the *merged contribution head* with both the new 100M market checks and the merged industry/movement checks. No database migrations or seed refreshes were executed as part of creating these patch notes.

## Outstanding review / production acceptance

- **Must verify:** complete contribution branch CI including native CTest, MariaDB seed and packaging; real market economy values; movement across hull types and latency; mission 65534/65535 payouts and respawn behavior; mining drone ore return; research/blueprint delivery; POS shield and IHub upgrade lifecycle.
- **Known incomplete:** T2 invention, POS/remote research jobs, transaction-atomic industry install/delivery, genuine sovereign activity accumulation and decay, full mission template scripting, complete physical collision simulation, certain historic special weapon families.
- **Production caution:** do not combine upgrading C++ binaries, applying migrations and refreshing 100M orders in one irreversible step. Stage and verify each separately with backups and rollback notes.
- **Upstream PR status:** remain draft; request subsystem slices for review due cumulative diff size, with changes and limitations recorded in [handover](UPSTREAM_DEVELOPER_HANDOVER_2026-11-19.md).

## Archived notes

- [Original 15 November dossier](UPSTREAM_PULL_REQUEST_REVIEW_2026-11-15.md)
- [17 November integration dossier](UPSTREAM_PULL_REQUEST_REVIEW_2026-11-17.md)
- [18 November infrastructure/POS dossier](UPSTREAM_PULL_REQUEST_REVIEW_2026-11-18.md)
- [Historical baseline patch notes](UPSTREAM_PATCH_NOTES_HISTORY_2026-11-15.md)
- [Fork chronological PATCH_NOTES.md](../PATCH_NOTES.md)
