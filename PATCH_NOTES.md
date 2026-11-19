## 19 November 2026 — Refresh upstream draft PR #335 and publish developer handover

- Ancestry-preserving synchronization of the upstream contribution branch with fork `master` commit `e2f8e14bca5b50bb1f8ff055f5f16a00d01b7c27`; merge commit `9f8929de3387c9ae471e768556eefc8657c62f5f` retains both original parents, previous contribution market work and new master changes without force pushing or deleting parent paths.
- Preserve the contribution-only **100-million-unit NPC ore/ice/mineral bids and asks** in EVEDBTool, Trader Joe, SQL seed templates and MariaDB regression fixtures. Resolve the only shared files (`.github/workflows/build-release.yml` and `PATCH_NOTES.md`) additively, keeping both 100M market and new ME/PE industry checks.
- Add a comprehensive [19 November upstream developer handover](docs/UPSTREAM_DEVELOPER_HANDOVER_2026-11-19.md), [cumulative patch notes](docs/UPSTREAM_PATCH_NOTES_2026-11-19.md), [exact 303-path pre-handover diff inventory](docs/UPSTREAM_PR335_FILE_INVENTORY_2026-11-19.md), and [ready-to-publish PR description](docs/UPSTREAM_PR_335_DESCRIPTION_2026-11-19.md). Preserved dated 15/17/18 November dossiers remain historical snapshots.
- Fork master [CI #44479434302](https://github.com/xrizonx/evemu_Crucible/actions/runs/44479434302) **passed Windows, Linux, Docker and MariaDB**. The combined contribution-only market changes plus latest industry tests **require an exact-branch CI run**, separate from that parent success. The upstream PR remains **draft**, not merged. No live database migrations, market seed force refresh, or deployment have been performed.

## 18 November 2026 — 100-million-unit mineral/ore NPC buy and sell orders

- New EVEDBTool regional and legacy station seeds and Trader Joe's dynamic NPC buy/sell orders now create **100,000,000 units per order per type/station** for minerals (group 18) and ore/ice (450–462, 465, 467–469). Both volume columns start at 100M. Other groups keep their existing amounts and **100% base-price** pricing.
- Guard against excessive pseudo-escrow: ore/mineral high-volume orders are allowed only up to 10,000,000 ISK/unit (1 quadrillion ISK notional), while other groups retain the existing order-value limits. No player wallets or orders are touched.
- Added Windows/Linux source invariants and MariaDB fixture checks covering mineral, Arkonor, Ice, existing charge volumes, player-order preservation, re-seed idempotency and high-price exclusions.
- **No existing orders automatically modified.** Backup and inspect `evedbtool seed -force -dryrun` before refreshing both buys/sells (`seed -buy-only -force` refreshes buys only). See [deployment guide](docs/NPC_ORE_MINERAL_100M_ORDERS_2026-11-18.md). This is committed on upstream draft PR #335's source branch, not fork `master`.

## 18 November 2026 — Update upstream draft PR #335 to current fork master

- Merge fork `master` at [`2d5cb4d2c7dbd0b33b70467e5caf3021fc52a713`](https://github.com/xrizonx/evemu_Crucible/commit/2d5cb4d2c7dbd0b33b70467e5caf3021fc52a713) into `contrib/upstream-crucible-maintenance-2026-11-15`, retaining earlier branch commits and 15/17 November upstream-review documents.
- Fork master adds **50 commits and 89 changed paths** since the previous contribution head `c876ce612173d15c71774d05393435d2ed4248ca`; see [18 November upstream reviewer dossier](docs/UPSTREAM_PULL_REQUEST_REVIEW_2026-11-18.md) for the complete file inventory, migration cautions, technical changes, and test gaps. The rest of this file contains the cumulative chronological notes from 13–18 November.
- Recent changes include `/sovindex5` test overrides and migrations, IHub upgrade-family support (including Supercapital Construction Facilities), POS force-field and ownership repairs, baseline upgraded military/industrial combat/ore sites, scanner fixes and `/dogma` permission checks.
- Latest integrated master [CI #44346201327](https://github.com/xrizonx/evemu_Crucible/actions/runs/44346201327) passed. Live Crucible client, full manufacturing and sovereignty lifecycle outcomes are not implied. No migrations executed.
- Excluded as not yet in master: [PR #41](https://github.com/xrizonx/evemu_Crucible/pull/41) Domination Grigori target filtering and [draft PR #42](https://github.com/xrizonx/evemu_Crucible/pull/42) mining-drone flight.

## 17 November 2026 — Upstream draft PR #335 refreshed with merged expedition + complete delta dossier

- Integrated fork [PR #36](https://github.com/xrizonx/evemu_Crucible/pull/36) into `master` at merge commit `97a98424d2ddcbbc885ec90ca753fa47cd1a5ca2`, preserving the original Blockade and the separate eight-NPC DED/Officer mission 65534 with 500M ISK turn-in, real faction/officer/deadspace loot and True Sansha type-name exclusion.
- Prepared upstream team handoff [`docs/UPSTREAM_PULL_REQUEST_REVIEW_2026-11-17.md`](docs/UPSTREAM_PULL_REQUEST_REVIEW_2026-11-17.md): 692 commits/241 changed paths versus upstream baseline; 280 commits/144 changed paths since the previous contribution baseline. Includes exact source file inventory, subsystem patch notes, verification matrix, runtime caveats and rollback guidance.
- Preserve the [15 November upstream review dossier](docs/UPSTREAM_PULL_REQUEST_REVIEW_2026-11-15.md) and [original contribution-branch patch note snapshot](docs/UPSTREAM_PATCH_NOTES_HISTORY_2026-11-15.md). PR #335 remains **draft**, with no upstream merge and no SQL execution. Published source contribution included **696 commits and 244 changed paths** at first handoff; later documentation-only commits may increment that count.
- Pre-merge CI [#575](https://github.com/xrizonx/evemu_Crucible/actions/runs/44142537989): Linux and MariaDB successful before merge; Windows UCRT64, Linux GCC, Docker and MariaDB insertion **all completed successfully** after the fork merge; release publication was skipped. Live confirmation of 500M payout and two consecutive crash-free elite encounters remains open.

## 19 November 2026 — Warp, stargate jumps and docking verified in game

- **Complete.** The operator confirmed in the Crucible client that warp animations play, stargate jumps complete and display correctly, and docking works with the updated build. [PR #43](https://github.com/xrizonx/evemu_Crucible/pull/43)
- Warp-to-zero targets 2,400 m between the ship and the gate or station collision surfaces, inside the 2,500 m jump and docking limits. The server counts each hull radius once, preserves longer selected ranges and the configured autopilot stand-off, and limits the last approach step along the ship's heading. Gate, station and bookmark coordinates use their corresponding destination geometry. [Landing and approach changes](https://github.com/xrizonx/evemu_Crucible/commit/a916ca02705330670c96773a5cfe1e8eca470c1e)
- Warp entry uses the ship's actual position after alignment. Heavy hulls can turn from a directly opposite heading, and Dock and Jump requests wait through alignment and active warp. The final tick publishes the landing position, stop, zero velocity and warp effect end together. Idle ships avoid repeated stop packets. [Warp geometry and verification record](docs/WARP_GATE_LANDING_2500M_2026-11-18.md)
- Stargate transfer sends origin-grid removals before the destination session. The existing movement-service bind can receive the new ship state, and the speed refresh follows the state snapshot. The first positive throttle input from rest supplies a valid course and releases gate cloak. A rejected jump returns without a success acknowledgement. [Gate transfer changes](https://github.com/xrizonx/evemu_Crucible/commit/e6ee9efbbe936b5c190a53ff3b64825d3125de2d)
- Warp sends full throttle before `WarpTo` and the warp effect, so the client can start alignment and the animation. Clearing invulnerability cancels the old timer, and a new gate arrival restarts the protection and cloak timers. The follow-up also handles duplicate bound-object releases and records inactive ballpark timers as diagnostics. [Warp throttle and timer changes](https://github.com/xrizonx/evemu_Crucible/commit/a0c9c75109beb6b4126f1c4620286280c8e2bdd5)
- Windows and Linux builds and CTest passed, along with Docker and MariaDB. Local regressions checked gate and station clearances across small and capital hull sizes, AU-scale coordinates, approach and packet ordering, and renewal with 4,525 ms left on the previous protection timer. [Verified gameplay build](https://github.com/xrizonx/evemu_Crucible/actions/runs/44473762678)

## 18 November 2026 — Mining drone flight and ore return

- Mining drones can lock and engage asteroids. They start mining within range, orbit at the configured surface clearance and carry ore back to their controller on cycle completion or recall. The server sends local positions, velocity and waypoints to the client during flight. Missing cruise speed uses the drone's own maximum velocity, and zero-mass mining types use a positive runtime fallback.
- Regression tests covered all four published movement speeds at AU-scale coordinates, including approach, a sustained orbit and return to a moving ship. Windows, Linux, Docker and MariaDB CI passed. Live client distance display and cargo delivery remain separate validation cases. [Merged PR #42](https://github.com/xrizonx/evemu_Crucible/pull/42) · [CI run](https://github.com/xrizonx/evemu_Crucible/actions/runs/44349122871)

## 18 November 2026 — DED & Officer Expedition target selection

- Excluded the reported untargetable Domination Grigori from the DED & Officer Expedition. The existing True Sansha exclusion and the eight-enemy fleet composition are preserved. These filters apply to this expedition's spawn pool. [Merged PR #41](https://github.com/xrizonx/evemu_Crucible/pull/41)

## 18 November 2026 — POS discovery, ship cargo and Crucible movement data

- Control towers resume an interrupted onlining countdown after restart. IHub ore-site creation reports success after the belt is populated. Online POS laboratories can advertise material/productivity research and copying slots, and assembly arrays can advertise manufacturing slots. Migration `20261118120000` supports stable line numbers, and discovery creates the facility's lines on demand. POS job installation and delivery still require their own implementation. [POS and sovereignty test guide](docs/SOV_INDEX_LEVEL5_GM_TEST_2026-11-18.md)
- For the reported Machariel cargo problem, ship inventory views refresh before the server lists or moves cargo. Single and bulk inventory transfers accept both Crucible integer ID encodings and refresh the ship's inventory after a transfer. The cargo-window behavior still needs its own live confirmation.
- Audited published ship, weapon, shield, mining, charge, drone and subsystem attributes against the Crucible 1.6 export. Guarded SQL repairs correct confirmed static data differences. Runtime fixes use fitted turret optimal and falloff, check mining range from live positions at harvest, correct Deep Core target selection and asteroid depletion, and send shield updates to remote-transfer recipients. Fitted effects and full module gameplay need their own live coverage. [Static attribute audit](docs/CRUCIBLE_1_6_STATIC_ATTRIBUTE_AUDIT.md)
- The capital warp audit confirmed base speeds of 0.75 AU/s for freighters and jump freighters, 1.50 AU/s for combat capitals, 1.20 AU/s for the Rorqual and 2.70 AU/s for the Orca. The guarded migration corrects the four freighter defaults, the Rorqual and the Orca. The server keeps fitted speed at full precision and encodes warp speed in tenths of an AU/s. [Range and warp audit](docs/CRUCIBLE_RANGE_AND_WARP_AUDIT_2026-11-17.md)
- Tractor movement applies live surface range and beam velocity, stops at the configured clearance, and updates the wreck's client movement state. Wreck and container access also use live ship and target positions. [Merged PR #40](https://github.com/xrizonx/evemu_Crucible/pull/40) · [Range and movement PR #31](https://github.com/xrizonx/evemu_Crucible/pull/31)

## 18 November 2026 — Repair post-build industry CI assertions for capped research skills

- [Run #44384199564](https://github.com/xrizonx/evemu_Crucible/actions/runs/44384199564) successfully compiled Windows/Linux, built Docker and passed MariaDB, but both compiler jobs stopped at `tools/verify_industry_workflows.py:47` before CTest. The obsolete assertion expected `(1.0f - (0.05f * MetallurgyLevel))`, while the actual, safer Crucible research formula caps the skill level with `std::min<uint8>(..., 5)`.
- Update **both** ME and PE source guards, scoped to their respective research functions. Assert actual 5%-per-level skill reductions (capped at level 5), linear research-level multiplier, and non-finite/overflow protection. Do not undo or weaken the C++ calculations.
- Reviewed all 28 source invariants against the branch. `IndustryFormulaTest` checks ME/PE level and skill effects on research time. Windows and Linux ran these checks successfully in the corrected run. [Corrected CI run](https://github.com/xrizonx/evemu_Crucible/actions/runs/44387410074)

## 18 November 2026 — Correct NPC orbit geometry and repeated DestinyError spam

- Analyzed repeated `Destiny::Orbit() - Invalid orbit geometry/speed` from Guristas Terrorist NPCs. The inherited high-order orbit calculation sometimes predicts a radius **below ship+target surface clearance** at normal 10–20km ranges (example: 10,143m vs 10,150m safe). Previously the server called `Stop()` and AI retried the same failed order each second.
- Clamp recoverable orbit radii to the minimum physically safe center distance; if the legacy equation overflows, use that safe clearance rather than feeding invalid geometry to the client. Truly impossible geometry (outside the bubble) still stops safely.
- Use valid `maxVelocity` when archived NPC static data omit `entityCruiseSpeed`; do not invent a speed for stationary NPCs. Immobile NPCs can still engage from a STOP state without reattempting a zero-speed orbit every tick.
- Successful repairs no longer emit a DestinyError; optional orbit trace explains clamping. Unrecoverable NPC errors are sampled once every 30 ticks as warnings, while player movement errors remain visible. Add C++ unit cases and source-invariant checks for radius underestimates, overflow, bubble limits and NPC speed fallback.
- No changes to warp targets, mining-drone movement, industry, market SQL, or server configuration. No SQL migration. Validate with live Guristas spawns on the patched server before declaring the behavior fixed.

## 18 November 2026 — Repair gameplay source-invariant CI failure (PR #43)

- The previous [Actions run #44380229744](https://github.com/xrizonx/evemu_Crucible/actions/runs/44380229744) **compiled Linux and Windows successfully and built Docker**, but Linux/Windows later failed `tools/verify_subsystems_audit.py` at line 67. Its invention-safety regression test asserted an obsolete comment string (`Do not consume datacores, runs or ISK`) instead of verifying code behavior.
- Replace the fragile text assertion with an explicit `ActivityCheck()` invention-branch guard: invention must throw the nonfunctional-invention error, and `InstallJob()` must call `ActivityCheck()` **before** `AccountService::TransferFunds()` and `FactoryDB::InstallJob()`. This preserves fail-closed invention without silently changing gameplay or charging players.
- The preceding compiler fix replaced the undefined `uint64` with `std::uint64_t`. The refreshed Windows and Linux CTest runs passed. [Corrected CI run](https://github.com/xrizonx/evemu_Crucible/actions/runs/44387410074)

## 18 November 2026 — Fix Windows/Linux/Docker industry build failure (PR #43)

- CI run [44378451842](https://github.com/xrizonx/evemu_Crucible/actions/runs/44378451842) failed all three C++ builds in `src/eve-server/manufacturing/RamMethods.cpp` because the new industry code used the undefined `uint64` alias; the follow-on `needed`, `available` and `factor` errors are cascading compiler diagnostics.
- Use portable C++17 `std::uint64_t` and explicit `<cstdint>` in the material stack accumulator and copy-job time arithmetic. Update CI source invariants to require the portable type and reject recurrence. No changes to production logic, SQL, warp, drone, sovereignty or other features; no migration required.

## 18 November 2026 — Restore Crucible-era linear ME/PE research levels

- Remove the post-2014 10-level exponential research-time table from Crucible's ME/PE research formula. Under pre-Crius rules each level used the same base time; the existing code had incorrectly limited both research activities to level 10. Restore linear per-level duration and allow research up to signed-byte level 127, with overflow-safe calculation and the existing 30-day per-job limit. Above 127 requires a future data-model/schema change.
- Extend industry CTest cases for level 20+, multi-level research and skill modifiers; preserve BPO-only restrictions, invention fail-closed, and copying logic. See [industry audit](docs/INDUSTRY_RESEARCH_COPY_INVENTION_AUDIT_2026-11-18.md).

## 18 November 2026 — Industry ME/PE research, blueprint copying and invention safety audit

- Tighten station industry line activity/category permissions, blueprint owner/location checks, completion owner/station authorization and personal job-list access. Prevent client-supplied negative/stale installation costs.
- Correct research ME/PE skill time reductions, floating-point blueprint PE and truncated 16-bit blueprint copy time. Copy jobs now account for both number of copies and licensed runs, with positive/max-run validation and 30-day submission checks.
- Persist the `copy` flag in `invBlueprints` on update, preserve original BPO infinite-run sentinels, and reconcile material quotes, multi-stack inventory verification and actual consumption with shared rounding. Extra material requirements no longer get an ME/Production Efficiency discount.
- Add `IndustryFormulaTest` CTest and Windows/Linux industry source-regression checks. **Invention remains deliberately disabled**: the previous output generator was an incomplete placeholder with no real T2 BPC or invention result. Do not consume resources for false success.
- No SQL migrations or live database changes. Full station client UI/job-delivery validation and transactional rollback protection remain necessary; POS/remote jobs are not functional. See [industry audit and test matrix](docs/INDUSTRY_RESEARCH_COPY_INVENTION_AUDIT_2026-11-18.md).

## 18 November 2026 — Movement drift, bump and micro-stutter investigation

- Synchronized `fix/gate-warp-landing-surface-clearance-2026-11-18` with latest fork `master` while preserving mining-drone PR #42's `ApplyMiningDroneStep` API and DED PR #41's NPC exclusions; retained all existing gate-warp source changes, with an ancestry-preserving merge.
- Removed idle `STOP` mode's repeated one-per-tick `CmdStop` emission; continue real deceleration and issue at most one stale-velocity correction. Guard malformed speed and heading values against propagating NaN positions.
- Extended final-tick collision-safe approach handling to both stargates and stations, with at least 2,400m surface clearance on manual Approach 0m. Consolidated the last position/stop/zero-velocity updates into one ordered Destiny packet batch, including at warp completion.
- Corrected movement speed reporting used by target/orbit prediction to measure current velocity rather than acceleration timer progress. Extended C++ and source-invariant regression coverage. See the [review and live-playtest plan](docs/WARP_GATE_LANDING_2500M_2026-11-18.md).
- `CheckBump()` reports player-to-player contact. The 19 November entry records the confirmed client behavior for warp, gate transit and docking.

## 18 November 2026 — Fix stargate warp landing, radius double counting and gate/station approach

- Manual `WarpToStuff` previously subtracted a gate's full radius, then added an arbitrary `radius / 3` and twice the ship radius. Result: variable landing distance and possible client bounce/overshoot. For gates and ordinary warpable objects, keep the original target-center coordinates and calculate the exact center stop distance by adding the ship and target collision radii **once** to the requested surface stand-off. "Warp to zero" gates now target **2,400m** from the surface (inside the 2,500m jump threshold); longer explicit ranges are retained.
- Autopilot previously used an unadjusted `apWarptoDistance` against an object's *center*, which could overlap large gates/stations. Add both hull radii to the configured surface gap; correct the autopilot initial `Follow()` command to receive a surface range.
- Fix `Client::StargateJump`, queued post-warp gate approaches and `DestinyManager::AttemptDockOperation` so `Follow()` receives a 2,400 m surface range. Gate and station interactions keep their 2,500 m limits. Subsequent changes use the station's actual center for warp and docking. The planet, moon and wormhole paths keep their own destination geometry, and raw bookmarks retain their exact coordinates.
- Clamp the final one-second movement step when a ship follows a stargate: move no farther than the remaining surface gap, send the exact corrected position and a zero-velocity/stop packet. This prevents full-tick overshoot even with fast or large hulls, including manual gate Approach.
- Added `WarpCenterStopRange` tests for small and capital hulls, selectable ranges, source guards and warp diagnostics. The 19 November follow-up passed CI and the operator verified the reported warp, jump and dock behavior in game. [Verification record](docs/WARP_GATE_LANDING_2500M_2026-11-18.md)

## 18 November 2026 — Baseline Military/Industrial upgraded IHub combat and ore sites

- On initialized sovereign solar systems, load active upgrades only from the matching TCU/IHub and schedule up to five additional combat anomalies for Military upgrades and five Gravimetric signatures for Industrial upgrades. Actual mineable asteroid objects are created through BeltMgr from existing system ore distribution.
- Fix DungeonMgr::MakeDungeon erroneously returning false after successfully constructing/registering a dungeon. Missing combat templates cause a clear failure instead of empty success; absent ore distribution causes a mining-site warning. Existing anomaly/site logic is reused.
- **Scope:** baseline site availability only. CCP-specific quality/rarity/tiers, complete NPC site content, manufacturing jobs, billing, activity-index progression/decay, immediate runtime refresh and site respawn policy still require further implementation and in-game tests.
- Extend source invariants and operator SQL diagnostics in the IHub test guide.

## 18 November 2026 — Full IHub-family install rules and control tower physical shield bubble

- Added category-39 IHub upgrade installation support for Strategic (group 1016), Industrial (1020) and Military (1021), with static upgrade prerequisite/blocking attributes, active flags, duplicate protection, and holding alliance Director/StarbaseConfig checks. Military/Industry installation remains restricted to simulated Level 5 testing while real activity progression is unimplemented.
- Expanded POS anchoring prerequisites to all POS with `anchoringRequiresSovUpgrade1`; require active installed upgrade, including Supercapital Construction Facilities for relevant capital arrays.
- Fixed tower force field spawn requiring nonempty password, missing harmonic persistence across restart, password clearing stopping the field, invalid zero radius (Large defaults to 30km), and force-field/missile packets incorrectly receiving damage state.
- Added test cases for all three IHub families and TowerShieldRules plus Python source guards. **Full military upgraded NPC/anomaly mechanics, ore cluster effects, actual activity index progression, manufacturing job service and sovereignty upkeep are not implemented by this commit.** Live gameplay testing required.

## 18 November 2026 — Support Supercapital Construction Facilities IHub upgrade installation

- Located exact server-side rejection of type `2009`: `Inventory::ValidateIHubUpgrade` supported only 3 strategic upgrade typeIDs, defaulting to `not currently supported` for Supercapital Construction Facilities.
- Updated type 2009 to require **100 effective Strategic days**, including `/sovindex5` TCU-bound testing days, rather than the actual TCU claim timestamp. Retain support for existing navigation/suppression/logistics upgrades, and reject other unsupported types explicitly.
- Require valid active alliance sovereignty and registered IHub, restrict installation to the holding alliance or DEV/ADMIN, prevent duplicate upgrades by type, and mark only supported upgrades `flagStructureActive` before inventory flag indexing. This improves persistence/listing consistency.
- Add pure `IHubUpgradeRulesTest` CTest and CI invariant checks. No SQL change required beyond previously merged index overrides; live client drag/drop, reload, and visual unlock need gameplay verification.

## 18 November 2026 — Resolve Linux CI source invariant failure for /sovindex5

- CI run 44259205124: Linux GCC **compiled**, but step `Verify sovereignty and POS integrity invariants` failed at line 71 because the checker expected the obsolete SQL string `strategicDays=100`.
- The upsert intentionally now uses `strategicDays=VALUES(strategicDays)` with an explicit bound C++ constant `EVESovIndex::LevelFiveStrategicDays = 100`. Update regression checker to assert the **current** parameterized query and the constant, keeping Strategic Level 5 unchanged.
- Docker and MariaDB jobs of the previous run passed; Windows compilation was still running when diagnosed. Repeat CI against this patch to confirm all platforms and regression guards.

## 18 November 2026 — Repair Level 0 Military/Industry after /sovindex5

- User's gameplay test: Strategic correctly shows Level 5, but Military and Industry still display Level 0. Root cause: old command wrote **5 raw points**, which is not equivalent to displayed Level 5, and 16-bit database/packet fields could not hold valid activity totals.
- Isolated DEV override now writes **1,000,000,000 military points** and **30,000,000 industrial points** (test-only values within signed 32-bit client serialization); Strategic remains 100 effective days. Change C++ override fields to uint32, widen client packed rows to DBTYPE_I4, SQL override columns to INT UNSIGNED.
- Add follow-up migration `20261118093000-sov-development-index-point-width.sql` that upgrades old (5,5) rows. Do not alter the original migration or any real claim timestamps, TCU, player assets, system ownership, or station settings.
- Expand CTest/Windows/Linux invariants for raw activity points and data widths. Must run new migration/rebuild/retest client before calling this resolved in gameplay.

## 18 November 2026 — GM Level 5 Infrastructure Hub development index test command

- Verified current `/sov` is only a stub and `SovereigntyDB` currently fakes military/industrial 5 points globally; Strategic is the genuine age from claimTime. The all-system index API previously returned null. Full upgrade installation functionality is not proven.
- Add `/sovindex5` for **DEV or ADMIN** only in a valid player-claimable null-sec system with real alliance TCU and registered IHub. It persists a 5/5 military/industry override and 100 strategic *effective* days; `/sovindex5 reset` removes the current claim's override. Original claimTime, TCU, inventory and sovereignty ownership are never changed.
- Add claim-bound table `mapSovDevelopmentIndexOverrides` via migration 20261118070000, loaded on server boot, with a TCU ID marker to prevent orphaned test indices carrying over to a new alliance. Claim removal also purges matching override rows to prevent onlining the same TCU from restoring test indices. The migration **must be applied before using the command**.
- Expose effective values via system, region, claim view, and completed all-system development indices; add new CTest and Windows/Linux invariants. Document exact command steps, restore action and iHub upgrade gameplay test checklist in [docs/SOV_INDEX_LEVEL5_GM_TEST_2026-11-18.md](docs/SOV_INDEX_LEVEL5_GM_TEST_2026-11-18.md).

## 18 November 2026 — Orphan reference recheck passed (manual cleanup eligible)

- Operator reconfirmed `posStructureData.itemID` **140003364** and **140003384** have zero contained inventory records, customs rows, incoming jump bridge links and sovereignty references. Earlier checks also showed absent base entities, and zero tower data, direct bridge records, dependent POS structures and dynamic attributes.
- Both rows remain unanchored/default-state metadata (`towerID=0`, `state=0`, `status=0`, `timestamp=0`) with anchorpoints 40177245 and 40177217 respectively. Recommend **manual archive and narrowly scoped deletion with explicit two-row transaction verification** after a full DB backup/server shutdown. Do not recreate items or delete unrelated records.
- No database modifications were made by this code review. The source lifecycle fix in PR #39 does not retrospectively clean these two records; the in-game deletion and restart path remains to be tested.

## 18 November 2026 — Confirmed POS orphan IDs and additional read-only reference audit

- Operator's five issue counts: invalid sovereignty claims 0; missing TCU entities 0; orphan towers 0; broken jump bridges 0; orphan structure metadata **2**.
- Two stale-looking `posStructureData` records: item `140003364` (anchorpoint `40177245`) and item `140003384` (anchorpoint `40177217`). Both have towerID/state/status/timestamp 0, no base `entity`, tower or direct bridge data, child structures or attributes.
- Added **POS-12 read-only reference check** for these IDs covering remaining inventory contents, customs rows, incoming jump bridges and sovereignty references. No repair SQL executes; back up and reconcile before any manual cleanup.
- Existing `StructureItem::Delete()` source fix prevents one future orphan pathway but does **not** automatically clean these historical entries.

## 18 November 2026 — Investigate 2 POS orphan rows and close deletion lifecycle gap

- Operator ran the read-only sovereignty/POS checks: four checks returned zero; two `posStructureData` rows have no `entity` record. Their IDs and relationships are not yet available. Do **not** run automatic cleanup.
- Source fix: `StructureItem::Delete()` now calls the existing POS metadata deletion routine before removing the inventory entity; previous scoop-only cleanup left metadata after permanent destruction. POS deletion also clears a reciprocal remote jump-bridge link first.
- Added POS-11 **read-only detailed orphan query** for item/tower/anchor/child counts and related POS attributes; updated regression source guards and the review dossier.
- Existing orphan rows remain unchanged until item IDs, dependencies and a backup establish whether manual reconciliation is safe. New source fix requires CI and live POS deletion/restart testing.

## 17 November 2026 — Upstream issue audit and scanner safety review (#128, #37, #170)

- Audited upstream issues against the merged fork before patching: `#319` (`bool` MariaDB reconnect) and core `#170` iterator advance are **already implemented**, so no duplicate replacement. `#35` has a 120-minute wreck timer but remains runtime-unverified.
- New scanner patch on `fix/upstream-issues-scanner-safety-2026-11-17`: **#128** client scan session now rebinds to the current system after jumps and refuses missing managers; **#37** directional cone calculation now normalizes vectors, checks invalid inputs, and supports full-circle scanning; **#170/#262** probe removal clears both live/active scan maps (partial mitigation only). **#269** post-NPC-death scan crash is not proven fixed.
- Added a scanning math unit test and CTest registration. The scanner changes are in `master` through PR #37. Live verification of the upstream scanner reports remains separate. [Merged PR #37](https://github.com/xrizonx/evemu_Crucible/pull/37)
- The audit records issue titles, code paths, reproduction tests and remaining limitations. [Complete upstream issue audit](docs/UPSTREAM_ISSUE_AUDIT_2026-11-17.md)

## 17 November 2026 — POS requested-position distance validation

- POS-10: SBU and orbital-platform anchor distance rules previously compared the stargate/planet to the structure old stored position, not the requested target. Use the target position for both checks; extend the Windows/Linux source regression guard.

## 17 November 2026 — Sovereignty and POS source integrity and access fixes

- SOV-01/02/03: write 64-bit FILETIME claimTime as BIGINT instead of mismatched %f, preserve cached sovereignty on failed DB insert/delete/query, and count unique online SBU-controlled gates to prevent false contested states.
- POS-01/02/03/04: lookup silo capacity by real item type; authorize tower settings, passwords, anchoring and jump bridge link/unlink by owner and StarbaseConfig/Director roles or server ADMIN; validate malformed POS access flag packets before writes.
- POS-05/06/07/08/09: persist tower sentry status and integer deploy flags, exclude packed cargo towers from deployed list, replace MariaDB self-referential bridge unlink with a reciprocal join, and reject wrong-system/occupied/cross-alliance bridge requests.
- Added CTest SovereigntyPosIntegrityTest and Windows/Linux CI source guard, read-only SQL diagnostics, full review dossier, risk notes and manual test matrix. Existing TCU/IHub and belt security fixes preserved. No migration, SQL mutation or account modification.
- Remaining work: live reinforcement/fuel, multi-system bridge atomicity, corporation roles and mission/structure playtests. Not merged until tested.

## 17 November 2026 — Fix /dogma access for DEV and ADMIN

- The observed `0x62f8000280c41000` DEV account includes CONTENT but not ADMIN (`0x0100000000000000`). Old `/dogma` required ADMIN only.
- Register `/dogma` with explicit **CONTENT OR ADMIN** permissions, preserving current ADMIN access. Ordinary/VIP players remain denied, and other ADMIN-only commands such as /kick, /ban and /givelp are unaffected. Do not modify account SQL or grant blanket privileges.
- Reuse bitmask authorization in `CommandDispatcher` and `/commandlist`, show CONTENT commands, log role masks as real 64-bit hex values instead of pointer `%p` formatting.
- Reject missing ship, missing item and unknown dogma attribute safely before modifying the item. Add CTest permission regression coverage for DEV, CONTENT, ADMIN, BOSS, VIP, STD and multi-bit roles. Live command check and CI needed.

## 17 November 2026 — PR #36 DED & Officer Expedition reward to 500M ISK

- Increase only synthetic Security Agent mission 65534 reward to **500,000,000 ISK** (from 50,000,000). Mission 65535 Blockade remains **50,000,000 ISK**.
- Keep the existing single payout at successful mission turn-in, 8-NPC target, premium faction/officer/deadspace loot and two-hour time limit.
- Update agent briefing, C++ lifecycle test, static regression script and operator playtest guide. Existing uncompleted DED offers get the reward through normal offer normalization; no SQL migration or player-record rewrite is required.

## 17 November 2026 — Exclude untargetable True Sansha NPCs from PR #36

- Operator reported that True Sansha NPCs are invalid player targets in elite Security Agent mission 65534. Filter NPC candidate type names containing "True Sansha" case-insensitively, only during the new mission's selection. Preserve other legitimate Sansha NPCs.
- Validate candidate NPCs within each category before selecting two distinct groups; skip groups with no eligible types, fail clearly if any category cannot fill its two slots, and keep the objective at eight kills.
- No change to the Blockade, global spawns, inventories, reward or rare loot. Add source regression checks and a two-run in-game verification checklist.

## 17 November 2026 — PR #36 repeated mission pocket cleanup safeguards

- After one eight-NPC expedition reached 8/8, operator reported server exit on a later run; the supplied FX and bound-object warnings do not include a native exception trace.
- Retire the matching mission bubble's spawn registrations and flags on completion/quit; preserve wrecks and remaining NPCs. Prevent accepting a beacon into an unexpectedly occupied or mismatched bubble. Keep empty-bubble manager system/ID lookups consistent.
- Add source checks and repeated acceptance playtest instructions. Mission reward, eight-NPC spawn mix, rare loot, Blockade, player data and database schema unchanged. CI and live confirmation required.

## 17 November 2026 — DED & Officer Expedition eight-NPC balance

- PR #36 operator confirms actual elite ship types spawn correctly. Reduce only mission 65534 from 18 to **8 ships**: two each from DED/deadspace, faction commander, pirate officer and DED overseer categories, randomized across the existing 18 candidate NPC groups.
- Align briefing, journal kill target, static assertions, lifecycle tests and source tests. Already accepted 18-NPC encounters keep their recorded target. Do not change original 12-NPC Blockade, 50-million-ISK reward, 120-minute limit or rare wreck loot.
- No database migration or modifications to player, item or market records. CI and real-client playtesting remain required.

## 17 November 2026 — Separate DED & Officer Security Agent Expedition (pending client acceptance)

- Added synthetic Security Agent mission `65534` **DED & Officer Expedition (TEST)** alongside unchanged `65535` Blockade. Both are randomly offered by the same Security Agent divisions and tiers.
- The new mission spawns up to 18 real Crucible NPC ships: five DED/deadspace, five faction commander, five pirate officer and three DED overseer types spanning frigates through battleships and all five pirate factions.
- Each elite mission NPC adds a matched-tier, market-listed **Faction (meta group 4), Officer (5) or Deadspace (6)** module to its wreck via the static type catalogue. Normal NPC loot, bounty, salvage and all non-elite missions are unchanged.
- Agent briefing, journal, mission-pocket kill count and two-hour turn-in timer reuse the established Blockade lifecycle; reward remains 50 million ISK.
- Added `MissionLifecycleTest` assertions, Windows/Linux source regression checks and [gameplay test guide](docs/missions/DED_OFFICER_EXPEDITION_TEST_MODE.md). Real player-client verification still required.

## 17 Nov 2026 — Repair NPC buy-order seed MariaDB Error 1264 (PR #35)

- Fixed `evedbtool seed -buy-only -force` failure `Out of range value for column 'orderRange'`: the seed wrote `-1`, which the deployed MariaDB column cannot represent. New static bids use **`orderRange=0`** and both refresh-deletion and duplicate detection use that value.
- `0` denotes a system-range marker to the client; immediate execution remains restricted by **stationID** in `MarketDB::FindBuyOrder`. This is a compatibility workaround, not implementation of remote sales.
- Expanded the source regression checks and a read-only post-failure audit. Previous interrupted `-force` executions may have partially changed NPC orders before the failed insert; compare with your backup before rerunning.
- Windows/Linux/Docker CI and a real MariaDB insert smoke test must pass before merging. No database migration or player-data changes are included in this fix.

## 17 Nov 2026 — Standard NPC buy seed at 100% static market reference (pending client validation)

- Added NPC **buy** orders to standard EVEDBTool market seed using independent `SEED_NPC_BUY_100PCT` migration so existing `SEED_MARKET` databases can add bids with `seed -buy-only` without rewriting sell/player orders. Bids cover every published, market-listed Crucible inventory type in configured regions.
- New bids use exactly `invTypes.basePrice` rounded to cents (100 ISK fallback when missing/nonpositive). Existing Trader Joe bot's future buy prices likewise no longer vary randomly from 80% to 110%. A narrowly scoped one-time normalization repairs old regional-range NPC bids under `ownerID=1000125`, which the server cannot execute, to supported `4000000` at 100% reference value and adjusted non-funded NPC pseudo-escrow.
- NPC buy orders are recognizable by `ownerID=4000000`, `bid=1`, database-compatible order range `0`, duration 250 days. Full refresh is explicit via `seed -buy-only -force`; default seed is once-only/idempotent. Orders limit notional escrow to 10 trillion ISK per order and cannot draw from player wallets.
- Market settlement uses stored NPC bid price and caps a sale to the requested inventory quantity. [Read-only SQL audit](sql/diagnostics/2026-11-17-npc-buy-orders-100pct-audit.sql) and [operator test plan](docs/NPC_BUY_MARKET_100_PERCENT_2026-11-17.md) document coverage, pricing, duplicates, rollback boundaries and live client tests. CI / real MariaDB / 2012 client outcomes must be recorded before merging to master.

## 17 Nov 2026 — A01 operator acceptance, merged validation and staging cleanup

- **A01 is RESOLVED (operator reported all A01 tests passed)** on the existing-data live test. Original [PR #32](https://github.com/xrizonx/evemu_Crucible/pull/32) was already merged into `master` at `7f600be1fa06191707f4352d46f2240bfe3d2159`.
- [A01 MariaDB 11.8 fixture CI 44072201705](https://github.com/xrizonx/evemu_Crucible/actions/runs/44072201705) and [post-merge master Windows/Linux/Docker CI 44073936776](https://github.com/xrizonx/evemu_Crucible/actions/runs/44073936776) passed. Real-client test acceptance is the operator's report; no private gameplay evidence was uploaded here, and the opt-in full-server GitHub Actions job was skipped.
- Retired only the A01-specific temporary fixture, isolated Docker harness, Python unit helpers and dedicated CI workflow; removed A01-only steps from the standard build. **Preserved `src/eve-test/`, CTest registration, normal Windows/Linux/Docker CI, existing game services, production Compose, SQL migrations and data.** Git history retains all A01 implementation files for reproducibility if needed.
- [A01 archived completion record](docs/A01_STAGING_HARNESS.md) and the [living repair status](docs/REPAIR_CHECKLIST.md) preserve the sign-off and next tasks.

## 16 Nov 2026 — PR #25 runtime-confirmed station container fix and 0.8.7-Rizonx server revision

- **Operator runtime-confirmed:** assembled Giant Secure Container **140002461** (named `Test`) can be renamed, loaded with items, and emptied into the station hangar without crashing the server or the Crucible client. Stack splitting works and contents persist over server restarts.
- **SQL evidence:** eight valid-looking mineral stacks with owner **90000000** and container location **140002461**; an indexed inventory lookup uses existing `idx_entity_location_item` (`ref` access), so no duplicate index or data repair is indicated.
- Set the human-readable `EVEMU_REVISION` display to **0.8.7-Rizonx**; EVE Crucible client version **7.31**, build **360229** and MachoNet **320** remain unchanged. Windows/Linux packages label their `BUILD-INFO.txt` accordingly.
- **Still pending:** POS tower/module lifecycle, inventory edge cases and nested-container rejections, customs-office persistence, multi-character permissions, end-to-end Windows packaging, and CI verification for the new version-bump head. Full operator-checkable list: [PR #25 in-game QA acceptance checklist](docs/PR25_INGAME_VALIDATION_CHECKLIST.md).
- Do not conflate a tested in-station container fix with blanket approval of all POS/ship/corp inventory systems. The earlier source/audit notes below are historical and were written before the user confirmed the in-game fix.

---

## 16 Nov 2026 — Maurasi station assembled container transfer crash triage

The operator confirms station entry now works with locally served character portraits, but moving items into an assembled container crashes the **server**. The 209-line attached log shows sixteen identical unsymbolized stack traces during initial station load (before subsequent normal services); the actual fatal instruction cannot be established from those traces alone.

Source audit and defensive fixes on PR #25:
- Fix `Mission_Container` group mismatch in `CargoContainer::_LoadItem` that could emit `traceStack()` when this group is loaded.
- Protect `Inventory::ValidateAddItem` from null source inventory references and missing in-space ship sessions; refuse self-insertion.
- Stop ignoring failed capacity checks in `InventoryBound::MoveItems`, reject missing source items/destination inventories, and skip null/empty multi-add sort entries.
- Prevent dereference of a missing parent cache entry in `ItemFactory::GetItemContainerRef` and `GetItemContainerInventory`.
- Fix undefined uninitialized `std::pair` use and duplicate flag entries in `Inventory::AddItem`.
- Reject `InventoryBound` creation for objects without an inventory; preserve assembled hangar containers when empty instead of auto-deleting them.
- Emit `StationContainerTransfer BEGIN/END` logs around the destination capacity check and item donation; extend CI source safety checks.
- Step-by-step QA and dump guidance: [Station assembled container transfer runbook](docs/CRUCIBLE_STATION_CONTAINER_TRANSFER_CRASH_TRIAGE_2026-11-16.md).

**Caution:** These are real source defects and defensive corrections, but without a symbolized crash dump and live reproduction we cannot claim a confirmed fix for the fatal exception. No schema changes or reseeding.

---

## 16 Nov 2026 update — file-backed 50px agent portraits (PR #25 follow-up)

- The initial PR #25 embedded 322-byte 50x50 JPEG did not prevent the user's Crucible client crash. It also intercepted every NPC agent 50px request **before filesystem access**.
- Replaced that forced placeholder with a **local-file-first** lookup: `image_cache/Character/<agentID>_50.jpg` then optional `image_cache/Character/agent_fallback_50.jpg`. No external redirects or 512px substitution for NPC agent 50px requests. Valid player portraits and other image categories continue to work as before.
- Kept the explicit HTTP Content-Length fix; added the Windows JPEG generation utility `tools/prepare-crucible-agent-portrait.ps1` and [file-backed station-agent A/B test](docs/CRUCIBLE_LOCAL_AGENT_PORTRAIT_TEST_2026-11-16.md).
- This latest item **supersedes the previous embedded-322-byte test description below**. There is still no client exception dump or evidence of a successful in-game station load.

# Live Patch Notes — xrizonx/evemu_Crucible

## 2026-11-16 — Exact 50x50 baseline agent HTTP image and crash-safety audit

- Operator confirms **HTTP 404 for `Character/3011918_50` immediately before Crucible client disconnect**, despite replacing newest .exe and DLLs. A preceding 301 and an oversized embedded fallback also coincided with crashes; no actual client minidump or exception has been supplied.
- The Crucible 2012 50px NPC portrait loader now receives **200 OK with an actual 50×50 baseline JPEG**, not 301/404 or the old incorrectly sized placeholder. Deterministic neutral 322-byte image is compiled into server for NPC IDs 3000000–3999999 at size 50 only; no file-system or external CDN dependency, no real NPC likenesses reintroduced.
- Image server 200 replies now include explicit `Content-Length`, `Content-Type`, `Connection: close` and `Cache-Control: no-store`, with a connection-owned async header buffer. The older split header/body response had no content length. Other Character IDs/sizes and non-Character image fallbacks retain prior behavior.
- CTest `ImageCompatibilityTest` verifies the actual compiled JPEG byte count, baseline SOF0 50×50 dimensions, JPEG markers and tight NPC request matching.
- Agent services: prevent null bind parameter and null mission-bookmark dereferences in agent manager and bound journal.
- Inventory: guard missing container `GetMyInventory()`; `Split` reserves the new item *before* decrementing the original, compensates if decrement fails, and refuses full/singleton splits; `AlterQuantity` no longer deletes whole stacks on quantity underflow.
- Added Linux/Windows source-safety invariants for these fixes. Maintains PR #20 container assembly, PR #23 station-office fix, mission/Blockade rewards, drones, weapons, market and warp.
- Reclassified the unconditional `TCPConnection` destructor warning as trace, formatted `std::thread::id` safely instead of passing it to `%X`, preserved true thread ID before detach, and corrected pointer formatting to `%p` in the thread manager. The operator's `[ThreadWarning]` line was normal teardown logging, not proof of a client crash.
- **Important:** automated tests confirm code safety only; the original station-load client crash has not been proven fixed. Validate HTTP 200 with exactly 322 bytes and 50×50 dimensions before loading the same station; if crash continues, obtain client minidump/fault module. [Runbook](docs/CRUCIBLE_STATION_AGENT_50PX_CLIENT_CRASH_TRIAGE_2026-11-16.md).

## 2026-11-16 — Prevent modern portrait redirects during Crucible station loading

- Operator verified that even after PR #22 fully removed experimental portraits and PR #23 fixed station offices, the 2012 Crucible client requested `Character/90000000_256` and `Character/3011918_50`; the latter **still received a 301 redirect** to `images.evetech.net` immediately before logout.
- Root-cause candidate verified in source: `ImageServerConnection::ProcessHeaders` retained pre-experiment `EvEAgent::ModernImageUrl` fallback for missing NPC character portraits. Legacy clients may not support the modern external HTTPS destination; the earlier rollback did not disable this pre-existing redirect.
- Missing **Character** pictures (player or NPC) now get HTTP **404**, with `Content-Length: 0`, `Connection: close` and `Cache-Control: no-store`; no external redirect and no newly bundled agent JPEG. Existing local player/agent JPEG files continue to load if present.
- Before returning `image/jpeg`, `ImageServer::GetImage` now rejects an empty image or an obviously invalid JPEG signature, preventing mislabeled PNG files in `image_cache/Character` from being returned as JPEG. This signature check is **not** a full JPEG decoder.
- Non-Character modern fallbacks, market container assembly, station office guard repairs, agent missions, Blockade reward, drones, weapons, and warp remain unchanged.
- Extended existing `AgentHelpersTest` and added `ImageCompatibilityTest` for no character HTTPS fallback, preserved logo/icon URLs and JPEG signature checks.
- **Crucible in-client crash fix not yet confirmed.** Earlier PR #21 404 responses did not stop the crash, so another cause or client-side cached redirects/images remain plausible. Clear only cached image data (not settings) and retest with the matching build; obtain client Event Viewer exception/minidump if it persists. See [Crucible portrait response diagnostic](docs/CRUCIBLE_IMAGE_REDIRECT_CRASH_TRIAGE_2026-11-16.md).

## 2026-11-16 — Fix corporation office selection and station-load null dereferences

- Investigated continuing Crucible station-load disconnects after full removal of experimental agent portraits. Supplied server excerpt contains 16 identical unsymbolized call stacks and station/faction-warfare RPCs; the actual client crash dump is not included, so the client-side root cause remains unconfirmed.
- Corrected `StationItem::GetOfficeID`: its previous `if (cur.second.corporationID = corpID)` assignment always selected the station's first office for any player corporation. The new shared `EvEStation::OfficeIDForCorporation` helper performs a read-only ID match and returns 0 when the corporation has no office.
- Prevented `StationItem::LoadStationOffice` from dereferencing a missing office or inventory, and only marks an office as loaded after `LoadContents` actually succeeds.
- Guarded missing station references while constructing system statics and cleaning up a disconnected/docked character; a partial/failed station load no longer causes an additional null dereference during logout.
- Guarded missing cached office PyRep before returning office data to the client.
- Corrected `StationDB::GetStationOfficeIDs` for office-folder locations: `officeFolderID` is already the encoded folder ID, so subtracting `STATION_OFFICE_OFFSET` produced an incorrect SQL filter and hid office rows.
- Added `StationOfficeIntegrityTest` CTest for multiple corporations, missing offices, empty office indexes and no cross-corporation mutation. No image, market, agents, weapon, drone, warp or container-assembly logic changed.
- See [station-load failure source audit and client testing](docs/STATION_LOGIN_OFFICE_CRASH_AUDIT_2026-11-16.md). CI success does **not** imply that the live client crash has been reproduced and resolved.

## 2026-11-16 — Completely remove experimental agent image feature and restore original HTTP handler

- The 2012 Crucible client still crashes on station load even after PR #21 blocked all NPC agent JPEGs. The attached 08:11 log shows **repeated unsymbolized server-side stacks** while initializing Maurasi, then station-service RPCs and client logout; it contains no Image Server request. **No image-related root cause is proven.**
- **Fully restored** `ImageServerConnection.cpp`, `ImageServer.cpp` and `ImageServer.h` to the pre-offline-agent-portrait implementation. Removed the agent-ID-specific helper and its tests, the external portrait import scripts and the generic JPEG fallback asset. No new 404 guard or other new image response behavior remains; the original image handler's cached images and external redirects are unchanged.
- **Preserved** PR #20 cargo container assembly, the existing agent lifecycle/missions, 50 million ISK Blockade test reward, market containers, drones, weapons and warp tunnel.
- CI asserts the absence of experimental portrait code and files. This is a **full rollback**, not an assertion that the client crash is fixed; the repeat `traceStack` pattern may indicate a stale executable, as a related `DynamicEntityFactory::BuildEntity` stack-print loop was removed in commit `9a3a317068cf` on 15 November.
- See [Windows deployment and server/client crash evidence checklist](docs/AGENT_PORTRAIT_FULL_REMOVAL_2026-11-16.md).

## 2026-11-16 — Disable crash-prone station NPC portrait delivery

- Reported **immediate Crucible client crash when entering a station** after PR #20 embedded a generic agent JPEG. The client-side decoder root cause is not yet proven; this is a conservative compatibility rollback.
- Image server now returns local **404** for every NPC agent portrait ID in **3,000,000–3,999,999** *before loading a cached, custom or generic JPEG*, so stale image_cache files cannot reactivate the fault. No HTTP redirect is sent for these IDs; agent pictures intentionally remain blank.
- Restored pre-offline-portrait `ImageServer.cpp` and `ImageServer.h`, dropped installation of fallback JPEG and removed its runtime usage; player portraits and unrelated image categories retain earlier behavior.
- **Preserved PR #20 cargo container assembly**, all agent station metadata and mission lifecycle fixes, Blockade reward/market seeding, drones, weapons and warp tunnel behavior.
- Added source-level regression for the early image block and new [station-load rollback guide](docs/AGENT_STATION_PORTRAIT_ROLLBACK_2026-11-16.md); live Crucible client confirmation still required.

## 2026-11-16 — Fix container assembly and working-directory-dependent agent portrait fallback

- Implemented the existing Crucible `invbroker.AssembleCargoContainer(itemID, None, 0.0)` RPC. Previously it only logged a warning and returned without assembling. Restricts operations to personally owned station containers in historical groups **12/340/448/649**, splitting **one** item from multi-item packaged stacks, setting the singleton state and updated volume, notifying inventory and explicitly saving to DB.
- Preserved client-owned items and station checks; non-container, already assembled, corp-owned and remote items are not mutated by the new path. Existing generic inventory splitting remains nontransactional on DB failure; run on a backed-up staging server first.
- Fixed image-server warnings such as agent **3011918** by embedding the exact neutral JPEG `assets/agent_portraits/agent_missing_512.jpg` directly into the executable as the last fallback for uncached NPC agent images, rather than relying on `../share/evemu` relative to the process working directory. Local per-agent portraits and operator-supplied placeholder files still take precedence, with no HTTP redirect.
- Added `ContainerAssemblyTest` to CTest and exact-byte embedded-JPEG Python regression coverage. Gameplay verification remains pending; see [implementation and operator verification](docs/CONTAINER_ASSEMBLY_AND_AGENT_FALLBACK_FIX_2026-11-16.md).

## 2026-11-15 — Offline Crucible station-agent portraits

- Removed external HTTPS redirection for missing NPC station-agent character portraits (legacy 3,000,000–3,999,999 IDs); the local image server now serves exact or alternative cached JPEG sizes, or a bundled original generic silhouette when no historic portrait exists.
- Added a portable placeholder at `share/evemu/agent_portraits/agent_missing_512.jpg` via CMake, with a custom override under `image_cache/Character/`. The packaged fallback remains accessible even if Docker mounts an existing `image_cache` volume.
- Added opt-in `tools/agent_portraits/sync_portraits.py` to import available genuine JPEGs as local files using `sql/base/agtAgents.sql.gz` without live MariaDB. It validates responses and preserves existing cached portraits; a `--local-import ... --offline-only` mode imports an existing historical JPEG folder/ZIP entirely without network. **No network access is required from the server during agent image requests**; the importer uses the official image service only when intentionally run.
- Added offline Python cache/JPEG tests and an expanded `AgentHelpersTest` NPC portrait-ID check; Windows/Linux CI verifies the importer against the actual packaged agent SQL dump.
- Documentation: [offline portrait setup and verification](docs/AGENT_LOCAL_PORTRAITS_2026-11-15.md). Complete historical NPC portrait coverage and a 2012 client visual test are not yet proven, and downloaded copyrighted CCP assets are not automatically added to the public GitHub source.

## 2026-11-15 — Blockade test reward and cargo containers in NPC market seed

- Increased the synthetic Level 0 Blockade test mission's ISK reward from **150,000 to 50,000,000 ISK**, without altering official Crucible-era mission templates or unrelated rewards.
- Updated five hard-coded Blockade briefing, conversation and journal text occurrences. Existing Blockade offers use the already-implemented normalization on retrieval; previously completed missions are not retroactively paid.
- Added published, market-group-assigned cargo containers to EVEDBTool's regional seed and force-reseed filters: inventory groups **12 (cargo), 340 (secure), 448 (audit-log secure), 649 (freight)**. This avoids wholesale seeding of the Celestial category.
- Applied the same container allowlist to both legacy region and single-station market seed SQL scripts.
- Expanded Windows and Linux EVEDBTool dry-run CI checks to verify both container insertion and force-cleanup SQL. Added a Blockade reward assertion to `AgentHelpersTest` and [rollout/audit instructions](docs/BLOCKADE_REWARD_AND_CONTAINER_MARKET_SEED_2026-11-15.md).
- **Verification:** build and live market/client gameplay tests must be checked on this branch. Run `evedbtool seed -force -dryrun` before applying a `-force` reseed; existing NPC orders may be replaced.

## 2026-11-15 — Mission offer lifecycle and offline-item integrity

- Fixed periodic expired-offer crashes when agent lookup fails; persistent state cleanup continues without client notifications.
- Fixed shared-agent cache invalidation by matching persistent offer IDs and character IDs rather than erasing an arbitrary entry for the same agent.
- Deferred (`Allocated`) offers now expire correctly; stale previous offers no longer remove a newly issued same-agent mission.
- Completed offers are immediately archived to mission history instead of lingering in active indexes. Mission cache update/remove paths now match offer IDs.
- A failed `agtOffers` insert no longer produces a non-persistent zero-ID mission.
- Offline mission item cleanup now preserves the unconsumed stack remainder, supports 32-bit quantities and stops on SQL errors.
- Added `MissionLifecycleTest` and [dedicated source audit](docs/MISSION_EXPIRY_AND_OFFLINE_STACK_AUDIT_2026-11-15.md).
- **Verification:** new branch CI and live client/DB verification pending. Earlier successful master CI does not verify this patch.

## 2026-11-15 — Agent data, station portraits and mission safety

- Agent station listing no longer relies exclusively on optional `chrNPCCharacters.stationID`: `ManagerDB::GetAgents` and `AgentDB::LoadAgentData` use `agtAgents.locationID` when it resolves to `staStations`, while preserving the legacy result columns.
- Individual agents no longer fail to load merely because the optional NPC bloodline record is missing. Genuine missing primary agent rows or query failures now fail cleanly; the `AgentData` structure is initialized to zero instead of exposing undefined values.
- The image server's missing-image redirects now use the supported `images.evetech.net` character-portrait/logo/icon/render APIs rather than the deprecated `image.eveonline.com` URL. Legacy 50px station agent portraits map to 64px remote requests while local `image_cache/Character/id_50.jpg` and `id_512.jpg` files continue to work. Historical NPC portraits absent from the remote CDN cannot be recreated by URL mapping; client support for HTTPS redirects must be tested.
- Local PNG images are now served with the correct `image/png` MIME type. Invalid zero image ID/size requests receive a 404 before attempting a redirect.
- Mission accept, complete, decline, defer and quit paths now validate that an agent actually has an offer in the required state before side effects; this prevents default-constructed missions and repeated completion payouts.
- Fixed time-bonus calculation: compare the current FILETIME with the acceptance timestamp plus the configured minutes, and show the actual bonus ISK rather than an estimated multiple of the base reward. Broken bonus check previously never paid eligible bonuses.
- Error handling now returns early if loading agent skill data from SQL fails.
- Added `AgentHelpersTest` for image URL mapping and bonus-time math and a dedicated [agent image/mission audit](docs/AGENT_STATION_IMAGE_AUDIT_2026-11-15.md). GitHub build and Crucible client gameplay verification pending for the new code.

This file tracks fixes and maintenance changes made on this fork's `master` branch. Entries describe code changes, not a guarantee that every gameplay path has been manually tested.


## Mission completion and briefing follow-up — 2026-11-14

- Strengthened the synthetic Blockade mission's final-hostile detection: the live NPC count in the dedicated mission pocket is now a fallback when stale spawn-registry entries remain.
- When the last live NPC dies, the remaining registry rows for that mission pocket are cleared and the accepted offer's kill objective is set to its required count, making it eligible for the normal agent turn-in path.
- Added visible spawn/NPC death diagnostics so mission kill callbacks, bubble IDs, live NPC counts, registry counts, and offer progress can be checked without enabling trace-only categories.
- Added custom plain-text Blockade briefing content to the agent's offer/view responses and mission briefing response data. This avoids relying solely on the missing synthetic localization IDs.
- Build and in-game verification are pending; the user's supplied log did not include mission death callback output.

## Upstream contribution summary — 2026-11-13

This PR proposes the current fork work against `EvEmu-Project/evemu_Crucible:master`. The fork's current integration head is 183 commits ahead of the upstream merge base in GitHub's comparison at the time of the 2026-11-14 review. This count includes the fork's accumulated maintenance history and should not be interpreted as 183 newly reviewed changes in the latest mission/drone slice. This is a grouped summary of the complete proposed change set, not a claim that all gameplay features are finished.

### Mission system — implemented foundation; combat encounter work in progress

- Added encounter mission offer loading, lookup and persistence support using the existing `agtMissions` data, including briefing and reward metadata.
- Added security-agent mission selection and Level 0 progression, including a global starter security-mission tier and checks that prevent repeating completed starter templates.
- Fixed a global active-offer lock that incorrectly blocked a character from requesting missions from every other security agent while holding an offer from just one agent. Offer checks now remain scoped to the current agent, matching its per-character offer tracking. Expired offers are ignored, and the failure dialogue no longer conflates an active starter mission with missing templates.
- Added Level 1 agent access/standing display adjustments.
- Added the first playable encounter mission lifecycle: accepting an encounter creates a warpable mission beacon in a reserved pocket, associates the pocket with the character/agent/offer, and starts NPC spawning.
- Added mission-pocket kill progress reporting back to the accepted offer and mission update notification to the client.
- Added encounter-site cleanup and mission completion handling in the existing client/agent flow.
- **Latest in-progress work:** encounter offers continue to be selected from the loaded `agtMissions` security templates. The selected mission's `dungeonID` is resolved; the dungeon-template faction is preferred, with agent/region faction fallback.
- When a mission dungeon template contains room objects, the first room's NPC ship/drone type IDs and relative positions are now used to create the mission pocket's initial NPC group. Templates without usable room NPC data retain the generic faction/class spawn fallback. Ordinary belt/anomaly faction selection is unchanged.
- This is the first template-driven encounter step, not the complete EVE mission engine: later rooms, acceleration-gate progression/unlocks, trigger-based reinforcements, mission-specific non-kill objectives, and per-mission reward/loot tuning remain in progress. It is **not yet** a complete library of EVE Level 1 mission-specific templates, named wave compositions, gate unlocks, objective triggers, or per-mission reward/loot tuning. Those are follow-up implementation tasks.
- Existing NPC death, bounty, wreck and loot behaviour remains the shared game-system path; this PR does not claim a newly implemented per-template loot/bounty table or verified Level 1 reward balance.

### Space movement and warp/docking

- Added server-side pending dock and stargate approach actions so dock/jump requests can remain active while a ship approaches the required range.
- Dock and stargate jump requests selected during an active warp are now queued until the warp tunnel completes, then resumed through the normal approach/perimeter checks.
- Normalized ship heading vectors after each turn interpolation and clamped dot products before `acos`, preventing vector-length drift and invalid angle calculations during alignment.
- Added cancellation/clearing of pending space actions when a new manual movement command supersedes them.
- Corrected Crucible client warp-state serialization for player ships, NPCs and drones, including warp sector encoding and dynamic entity snapshots.
- Added diagnostics and client-visible feedback for rejected stargate jump requests.

### NPCs, drones and combat stability

- Added drone damage-state encoding and defensive defaults for missing resistance attributes.
- Improved drone scoop, abandon and ship-destruction cleanup, including controller references and bandwidth accounting.
- Corrected NPC belt spawn security-class calculation and allowed off-grid NPCs to enter warp before normal culling can remove them.
- Added defensive validation to turret hit calculations to avoid invalid entity-state and zero-distance crash paths.

### Server stability and compatibility

- Fixed mutable iterator traits in the core Buffer implementation.
- Hardened DevTools loader file handling and temporary-buffer lifetime.
- Added defensive checks to database result conversion and row text access; corrected search queries that returned too few columns.
- Fixed probe-processing iterator invalidation when a probe removes itself during processing.
- Updated the MariaDB/MySQL reconnect option to use the current boolean type.
- Added guards around station inventory loading during pod respawn and corrected station inventory reload after system unload.
- Corrected the dungeon editor AddObject binding to accept the full client argument layout and nullable transform/radius values.

### Build, packaging and documentation

- Updated CMake/build configuration, Docker build inputs and ignore rules.
- Added a Windows/Linux/Docker build-and-release GitHub Actions workflow and updated compiler/runtime compatibility fixes.
- Added SQL setup documentation and example database configuration.
- Added issue triage and stability worklog documentation.

### Verification and known limitations

- The existing CI workflow has successful Windows, Linux and Docker build history for earlier commits in this change set. The latest mission-faction commit's workflow run was still in progress when this note was prepared; check the linked Actions run before treating it as green.
- Some earlier workflow runs were cancelled as newer commits superseded them; a previous full build run succeeded, but that does not substitute for a successful build of the final PR head.
- The repository's CTest configuration previously reported no tests found. Manual in-game verification is still needed for mission acceptance, warp to beacon, NPC combat, progress updates, mission completion/rewards, NPC bounties and loot.
- No database reset is part of this work. Existing MariaDB data must be preserved. Any future mission-template schema/data migration should be additive and separately reviewed.
- No full upstream merge has been performed by this patch-note update. The change set is submitted for upstream review through the PR.

## 2026-11-13

### Fixed

- **Search result column mismatch:** Updated all ten search-category queries in `src/eve-server/search/SearchDB.cpp` to return the two columns expected by `DBResultToIntIntDict`. This addresses the reported `DBCore::GetInt: Column index 1 exceeds number of columns in row (1)` failure in those search paths.
- **Defensive database result conversion:** Added a column-count check to `DBResultToIntIntDict` in `src/eve-common/database/EVEDBUtils.cpp`, preventing reads from a missing second column.
- **Defensive database row access:** Added null/result/column bounds checks to `DBResultRow::GetText()` in `src/eve-core/database/dbcore.h`.
- **Probe processing iterator safety:** Fixed probe iteration in `src/eve-server/EntityList.cpp`. A probe can remove itself while being processed; the loop now advances safely and only erases the expected map entry. Related upstream report: [#170](https://github.com/EvEmu-Project/evemu_Crucible/issues/170).
- **MySQL reconnect option compatibility:** Replaced the legacy `my_bool` reconnect variable with `bool` and passed its address directly in `src/eve-core/database/dbcore.cpp`. Related upstream report: [#319](https://github.com/EvEmu-Project/evemu_Crucible/issues/319).

- **Turret hit-calculation crash protection:** Added validation for missing ship/pilot/target state, missing target movement data, zero distance, and invalid tracking/signature/falloff inputs in `TurretFormulas::GetToHit()`. This hardens the crash path reported in upstream [#139](https://github.com/EvEmu-Project/evemu_Crucible/issues/139). It is a defensive source fix; asteroid weapon behaviour still needs an in-game regression test.
- **NPC belt spawn class and warp-in:** Corrected the security-rating scale used by `SpawnMgr::PrepSpawn()` to match the thresholds' expected -1.0..1.0 range. NPCs spawned in empty off-grid bubbles now enter warp immediately rather than being culled before warp-in. This implements the code approach from upstream [PR #327](https://github.com/EvEmu-Project/evemu_Crucible/pull/327) and targets the disappearance report [#275](https://github.com/EvEmu-Project/evemu_Crucible/issues/275). In-game spawn/combat testing is still required.

- **Dungeon editor AddObject binding:** Corrected the server binding to accept the full nine-argument client call and nullable yaw/pitch/roll/radius values. The existing `PyRep::FloatValue()` conversion maps `None` to zero, matching the client report in upstream [#276](https://github.com/EvEmu-Project/evemu_Crucible/issues/276). The actual dungeon-editor interaction still needs an in-game test.
- **Station inventory reload after system unload:** The client-side hangar-loaded marker could remain set after a solar system unloaded its station inventory. `Inventory::LoadContents()` now skips the load only when both the client marker and the in-memory inventory-loaded state are true. This addresses the stale-cache path reported in upstream [#188](https://github.com/EvEmu-Project/evemu_Crucible/issues/188). Verify by leaving a station until its system unloads, then returning and checking the hangar contents.

- **Pod-respawn station inventory crash guard:** `Client::LoadStationHangar()` now checks the active system, station lookup and station inventory before calling `LoadContents()`. It logs the missing object and reports a client-facing error instead of dereferencing a null inventory. This hardens the crash path in upstream [#154](https://github.com/EvEmu-Project/evemu_Crucible/issues/154); the reason the station/inventory can be missing is still under investigation, so this is not yet a complete gameplay fix.

### Under test

- **Stargate jump rejection diagnostics:** The server now logs the client movement state and sends a visible notification when `StargateJump()` rejects a request because another movement/session timer is still active. This addresses the previous silent no-op path; it does not force a jump while the ship is still warping. Test the Jump button after warp completion.
- **Warp state in client ball snapshots:** Corrected the Crucible WARP sector layout and changed dynamic entity snapshots to send `WARP` (including the alignment sentinel, warp distance, minimum range and warp factor) instead of always sending `STOP`. This is a narrow adaptation of the relevant serialization part of upstream [PR #332](https://github.com/EvEmu-Project/evemu_Crucible/pull/332); the PR itself is closed unmerged and its 75-file movement rewrite was not imported. Build and in-game warp-tunnel/bubble-transition testing are still required.

### Investigating / not yet fixed

- [#154 — Pod-death crash](https://github.com/EvEmu-Project/evemu_Crucible/issues/154): null station/inventory dereferences are now guarded, but the underlying reason the clone station lookup or inventory can be missing still needs tracing and in-game respawn testing.
- [#188 — Station inventory appears empty after a system unload](https://github.com/EvEmu-Project/evemu_Crucible/issues/188): reports items remain in the database but are not reloaded into the station inventory after returning to the system.
- [#269 — Scanner crash after destroying NPCs](https://github.com/EvEmu-Project/evemu_Crucible/issues/269): needs a current stack trace and reproduction against this fork.
- [#315 — Outpost construction crashes the server](https://github.com/EvEmu-Project/evemu_Crucible/issues/315): report contains no crash log; needs a reproducible test before changing construction logic.
- [#276 — Dungeon editor acceleration gate creation](https://github.com/EvEmu-Project/evemu_Crucible/issues/276): binding signature corrected to match the nine-argument call and accept nullable values; verify in the dungeon editor.
- [PR #326](https://github.com/EvEmu-Project/evemu_Crucible/pull/326) contains a large set of unrelated gameplay changes. It should be reviewed by subsystem and not merged wholesale without testing.
- [PR #323](https://github.com/EvEmu-Project/evemu_Crucible/pull/323) proposes saving precise ship position on logout; review persistence and login warp semantics before adopting it.

### Verification

- GitHub Actions build workflow: [run #107](https://github.com/xrizonx/evemu_Crucible/actions/runs/43454803533).
- The initial run #107 above covered the first five fixes. Additional commits later on 13 November triggered newer workflow runs; use the Actions page for the latest result.
- The code changes are not yet confirmed by a full in-game regression test. The repository's current CTest configuration previously reported no tests found.
- No database migration is required for these source fixes.

## Patch-note policy

- Add an entry when a code fix is committed to this fork.
- Link upstream issues where a relationship is confirmed.
- Distinguish **fixed**, **investigating**, and **not reproduced** items.
- Record build/CI status separately from in-game verification.
- Do not describe a report as fixed solely because a defensive guard was added; verify the underlying behavior where possible.


## 2026-11-14 — Blockade encounter, mission completion and drone combat integration

### Implemented on the integration branch

- **Blockade test fleet:** Expanded the synthetic Angel Cartel encounter to 24 frigates (NPC group 550). The user has confirmed that the fleet spawns, moves and engages the player in-game.
- **Mission objective:** Added an explicit 24-kill completion target for synthetic mission ID 65535. The target is initialized after generic Encounter offer setup because the generic setup clears the cargo/objective fields.
- **Mission timer:** The synthetic Blockade test mission now receives 120 minutes from acceptance. The special duration is scoped to mission 65535; other mission timers retain their existing level-scaled calculation.
- **Reward:** Configured a 150,000 ISK mission reward and retained the normal agent mission wallet-transfer path for turn-in.
- **Offer persistence:** Mission-offer updates now persist the reward as well as the courier/objective fields. Older open synthetic offers with missing reward or a zero kill target are normalized when loaded/queried; a nonzero existing target is preserved so partial encounter progress is not overwritten.
- **Objective metadata:** Encounter kill progress is no longer incorrectly serialized as an objective type ID. The Encounter objective type metadata is sent separately from the kill-progress/target fields.
- **Completion flow:** The existing Agent Complete action is gated by the kill objective. The existing completion path records completion, pays the configured reward and removes the encounter site/offer after successful turn-in.
- **Spawn target accuracy:** Encounter offer kill targets are taken from the actual mission-pocket required-kill value rather than a generic NPC count.
- **Drone damage and AI:** Removed duplicate global damage-rate scaling from drone attack construction; improved target-lock completion, approach/engage transitions, invalid/cross-system target cleanup, target-loss handling, and client-visible target/state synchronization. Fixed null-target reset and guarded state notifications when no system bubble is available.

### Root cause of the missing objective/reward regression

The synthetic mission's special reward and kill target were being assigned before the generic Encounter initialization block. That block subsequently reset the offer's courier/objective fields, so the mission journal could retain the 120-minute expiry while losing its reward and completion target. The special values are now assigned after the generic initialization. The database update query also previously omitted the reward column; that value is now persisted. A separate client payload issue passed the kill-progress field as the objective type ID; Encounter now sends the correct neutral objective type metadata.

### Validation status — final combined head

- Integration PR: [#11](https://github.com/xrizonx/evemu_Crucible/pull/11).
- Final code head before documentation refresh: `b527d20d17d7dff570ad0dd7b001042593f20589`.
- CI run: [#43593340656](https://github.com/xrizonx/evemu_Crucible/actions/runs/43593340656).
- Docker image build: passed.
- Linux GCC build, test invocation, install and packaging: passed.
- Windows 64-bit UCRT64 GCC build: was still running when this note was prepared; do not treat the combined run as fully green until that job completes successfully.
- In-game confirmation: 24 frigate spawns, movement and combat were confirmed by the user. The 24/24 journal counter, Complete action, 150,000 ISK wallet payout and encounter cleanup still require a live turn-in test. CI/compilation does not prove these gameplay outcomes.
- No database reset or destructive migration is part of this work.

### Integration and upstream review status

The Blockade encounter, two-hour timer, reward/completion and drone combat changes are consolidated in PR #11. The earlier fork PRs #8, #9 and #10 describe overlapping slices of this same work and should be closed as superseded only after the integration PR is merged. An upstream contribution PR is a separate review/merge path; merging into this fork's `master` does not itself merge anything into `EvEmu-Project/evemu_Crucible`.


## 2026-11-14 follow-up — final Blockade objective completion

A follow-up review found that the mission pocket's final-spawn cleanup and the mission-offer progress update were coupled only through the normal per-kill credit branch. If the final NPC death arrived without a resolvable player client/killer ID, the spawn entry was removed and the pocket became empty, but the offer could retain an incomplete kill counter. The final-spawn path now independently checks whether the mission spawn registry is empty and, for the matching accepted offer, persists progress at the required target. The Complete action and journal completion status use that persisted offer state. Normal per-kill progress remains capped at the required target.

The synthetic Blockade reward/kill target assignment in `Agent::MakeOffer()` has also been moved to after generic Encounter initialization, which resets courier fields. This ensures the offered mission itself carries its objective and reward rather than relying only on the acceptance-time pocket setup.

**Verification status:** source-level path corrected; automated build validation pending for the new commit. A live 24-kill run must still confirm 24/24 journal progress, Complete action availability, successful turn-in, 150,000 ISK wallet journal credit and encounter-site cleanup. Do not mark this as gameplay-verified until that test is performed.


## 2026-11-14 follow-up — Blockade objective accounting and mixed fleet

- Changed mission-pocket kill accounting so every NPC death reported by SpawnMgr advances the mission kill objective, even when the damage source cannot be resolved to an online player Client. The encounter pocket, rather than kill-credit ownership, is authoritative for this synthetic kill-all objective.
- Kept the objective bounded by the required-kill target; the pocket's final-spawn cleanup still forces persisted progress to the actual required count when no registered hostile spawns remain.
- Replaced the 24-frigate-only test fleet with a 24-ship composition selected from the installed static-data NPC groups: 12 Angel Cartel frigates, 4 destroyers, 4 cruisers and 4 battlecruisers. Types are selected dynamically from the static data pack within the expected Angel Cartel NPC groups rather than relying on a fixed list of type IDs.
- Fleet setup now refuses to start if any required ship class group or its NPC type list is missing, instead of silently substituting another class.
- Validation status: source changes pushed to the combat-mission integration branch. A fresh Windows/Linux/Docker CI run and in-game validation are still required. Confirm the log reports all four ship classes, then verify journal progress reaches the actual spawned count, Complete becomes available, turn-in pays once, and the pocket is cleaned up.


## 2026-11-14 follow-up — turret tracking and station warp-in clearance

- Corrected player and NPC turret transversal velocity: use relative velocity crossed with normalized line of sight, so only perpendicular motion contributes to angular velocity. Previously the full relative-speed magnitude incorrectly counted radial approach/recession as transversal and could cause excessive railgun/other turret misses.
- Added guards for zero-distance/invalid NPC turret targets, missing signature/tracking data and zero falloff when outside optimal range.
- Adjusted direct station warp-in points to stop beyond the station radius plus the ship's own collision radius and a 5 km clearance. Previously the point was offset by only the station radius, placing the ship at the collision surface and allowing hulls to overlap/bounce on arrival. Normal docking perimeter/approach logic remains unchanged.
- Validation pending: compile all CI targets, then test railguns against stationary and moving large targets at optimal/falloff, and warp ships of different hull sizes to stations before attempting docking.


## 2026-11-14 follow-up — Blockade fleet hull-tier correction

- Corrected the Angel Cartel NPC inventory group mapping after confirming that group 552 is the Battleship group, not Cruiser, and group 576 is the standard Angel Cartel BattleCruiser group.
- The synthetic 24-ship Blockade fleet now selects 12 frigates from group 550, 4 destroyers from 551, 4 cruisers from 553 and 4 battlecruisers from 576. Battleship group 552 is not part of the composition.
- Added exact expected group-name validation for each configured group and for each selected type. If static data maps an ID to an unexpected group/class, fleet setup fails rather than spawning that hull.
- Validation pending: compile/CI and a fresh in-game run. Confirm the fleet contains exactly 12/4/4/4 hulls, with no battleships or larger ships.


## 2026-11-14 follow-up — Corrected Angel Cartel destroyer/cruiser group IDs

- Runtime logs showed group 551 is not the Angel Cartel Destroyer group; it resolves to Asteroid Angel Cartel Cruiser. Group 553 resolves to Asteroid Angel Cartel Officer.
- Corrected the composition to use group 575 for Angel Cartel Destroyers and group 551 for Angel Cartel Cruisers. The other classes remain group 550 Frigate and group 576 BattleCruiser.
- This excludes the Officer group (553) as well as Battleships (552). Exact expected group-name checks remain in place to prevent another incorrect mapping from spawning.
- Validation pending: CI and a new mission run; expected composition is 12 frigates, 4 destroyers, 4 cruisers and 4 battlecruisers.


## 2026-11-14 follow-up — Blockade briefing presentation and live kill counter

- Refined the synthetic Blockade agent offer and active-mission dialogue into clearly separated sections for the situation, ship-class objectives, combat progress, completion instructions, reward and time limit.
- Added a live `Kills confirmed: current / required` counter to the agent's active-mission response and mission briefing/objective payload. The displayed value is capped at the required target and uses the persisted mission-offer kill progress.
- Added explicit `killsDestroyed` and `killsRequired` objective fields for clients that consume the extended mission objective dictionary.
- Kept the existing 24-ship objective, 120-minute duration, 150,000 ISK reward and turn-in/completion flow unchanged.
- User has confirmed in-game that the mission can now be turned in after destroying all ships, the reward is issued, and mission text is visible. The presentation/counter update is source-level only at this point; compile and a fresh in-game counter refresh test are still required.


## 2026-11-14 follow-up — render mission briefing line breaks correctly

- Fixed the Blockade briefing strings to use actual C++ newline escapes rather than double-escaped backslash-n text. The agent dialog and mission journal should now render paragraphs and headings on separate lines instead of displaying literal `\\n` sequences.
- Added leading spacing before the mission title so it is visually separated from the agent division heading.
- This corrects the text serialization/presentation issue; verify in-game after rebuilding the latest branch head.

## 2026-11-14 — Blockade journal response and drone combat follow-up

- Corrected the active Blockade agent conversation path: opening the agent with an accepted synthetic mission now returns the formatted mission text and live kill count instead of the numeric briefing ID `0`.
- Suppressed the nonexistent synthetic mission title/briefing localization IDs in mission briefing and journal responses. The client now receives the custom title and briefing strings rather than being asked to resolve message ID zero.
- Kept the kill objective in the normal encounter `kill` objective payload and exposed an explicit completion flag alongside current/required kills.
- Implemented `entity.CmdEngage` for player-selected drones: validates the active system, locked ship target, drone ownership/control, online state and shared bubble, then directs each selected drone to attack.
- Improved drone pursuit to use the configured chase-distance envelope rather than dropping targets at the shorter attack-range threshold. Aggressive drones can now retaliate against NPCs that target them; passive drones remain under explicit player control.
- Validation pending: compile current branch head and test journal display/progress updates, CmdEngage with light/medium/heavy drones against NPCs, pursuit at short and long distances, passive/aggressive behavior, target death, and drone return/scoop behavior.

## 2026-11-14 — Drone combat and recovery lifecycle

- Fixed a critical drone target-acquisition failure: combat drone types that omit `maxAttackTargets` were passing zero to the NPC target manager, which rejected every engage request. The AI now guarantees at least one target slot and a nonzero lock duration.
- Added safe fallback values for missing drone movement, orbit, pursuit, weapon-range and attack-cycle attributes so incomplete legacy static data does not leave drones with zero ranges or a disabled attack timer.
- Added a pending-target path: when an engage target is outside immediate drone lock range, the drone approaches and retries target acquisition once in range instead of abandoning the command.
- Reworked drone hit chance to use transversal velocity and guard missing tracking, signature, optimal-range and falloff attributes. Combat damage now validates base drone damage and multiplier values and emits per-shot diagnostics.
- Implemented `CmdReturnHome` (return and orbit), `CmdReturnBay` (return, then automatically scoop within 2,500 m) and `CmdAbandonDrone`.
- Added safe return-to-bay completion in the drone process loop, including drone-bay capacity validation, inventory transfer, bandwidth release, target-lock cleanup and solar-system entity removal.
- Fixed launch bandwidth validation to happen before moving the item or creating the space entity. Previously an over-bandwidth launch could leave an inert drone in space after the command failed.
- Pending engage targets are stored as entity IDs and re-resolved through the current solar-system manager each tick, avoiding stale entity pointers when an NPC dies or leaves the system. A new explicit engage order now clears a different stale/locked target before acquiring the new target. Added guards for invalid target state and a valid min/max orbit-speed interval.
- Validation pending on the latest branch head: Windows/Linux/Docker CI and in-game tests for light/medium/heavy drones, target lock, approach, orbit, damage, target death, return-home, return-to-bay, bay-full failure, abandon and bandwidth rejection.

## 2026-11-15 — Drone command decoding and automatic NPC engagement

- Fixed `entity.CmdEngage` argument decoding: the client can encode item IDs as Python `PyLong` values, while the handler previously required `PyInt` for the target and rejected non-`PyInt` drone IDs. The command now accepts both integer wire types and validates the server's 32-bit entity-ID range before lookup.
- Added automatic hostile-NPC acquisition for online combat drones. Idle drones scan their current bubble, select the nearest living NPC that is not owned by the pilot or of the same non-zero faction, engage it, and reacquire the next NPC after the current target is destroyed/lost.
- Explicit return orders suppress automatic reacquisition. Relaunching/online or issuing a new engage order enables combat acquisition again.
- Validation pending: CI and in-game tests of command decoding, automatic NPC acquisition, sequential target switching, faction filtering and return suppression.


## Drone engage ownership and client-ID recovery — 2026-11-15

- Explicitly bind the launching pilot to each newly created DroneSE instead of relying only on a second global client lookup by character ID.
- Validate drone command ownership using the drone's authoritative owner character ID, controlling ship ID, home-ship pointer, enabled state, and current bubble.
- If the client-supplied selected drone IDs do not resolve to any active drone entity, recover by scanning the current system registry for drones controlled by the calling character and ship, then issue the engage order.
- Added per-drone rejection diagnostics and fallback-order logs to distinguish stale client IDs from ownership, ship, enabled-state, or bubble mismatches.
- Requires Windows/Linux CI and in-game validation; no runtime success is claimed by this patch.


## 2026-11-15 follow-up — drone combat activity state and weapon gate

- Code review found the drone AI entered client activity state `Engaged` (10) for weapon combat, even though the source enum explicitly defines `Combat` (1) as the fighting state and describes `Engaged` as non-combat. This can produce an incorrect client activity label and makes the state machine inconsistent with the protocol's combat state.
- Changed the active attack state to `Combat`, retained processing of legacy `Engaged` states, and made combat-state transitions idempotent so the drone does not repeatedly reissue orbit commands while remaining on the same target.
- Aligned the weapon-fire gate with NPC AI: a drone now fires only after TargetManager reports that target locking has completed (`CanAttack()`), while the AI continues to select a fully locked target.
- Added a trace message for the lock-wait path. The existing firing log and zero-base-damage warning remain the evidence needed to distinguish a failed lock from missing static damage attributes.
- Validation: previous PR #13 head passed Linux GCC, Windows UCRT64 GCC and Docker builds. This follow-up commit requires a fresh CI run. In-game confirmation of the Combat label and actual NPC shield/armor/hull reduction is still required; no gameplay success is claimed yet.


## 2026-11-15 follow-up — drone range, combat envelope and stale target locks

**Reported symptoms**
- Combat drones oscillated between Approach and Fighting, did not visibly damage NPCs, and sometimes travelled far from the engagement.
- Player targeting reported the maximum of eight targets despite fewer visible active locks.

**Code findings and changes**
- Automatic drone acquisition scanned every NPC in the current system bubble without checking the pilot's drone control range. It could therefore select distant NPCs in another part of the system. Automatic acquisition is now restricted to NPCs within the effective ship/character `droneControlDistance`; a 20 km fallback is used only when old character data lacks that attribute. Explicit engage commands outside control range are rejected with a notification. A valid manual order can still be pursued after the target moves away, matching normal drone behaviour.
- The AI used the inner `EntityFlyRange + MaxRange` threshold to switch between Approach and Combat even though it already cached the outer `EntityAttackRange`. This created a gap where a drone could oscillate around the threshold. Combat now starts at the outer attack envelope, and approach uses the drone's orbit distance as its follow stop range.
- TargetManager counted every stored entry toward the player target cap, but did not proactively prune entries whose target had died, left the system, or moved to another bubble. Target entries now retain their entity IDs; stale entries are pruned safely before processing/new lock requests, lock state is recalculated, and the client receives target-lost updates. Drone target IDs are also cleared on loss.
- The player target limit calculation now uses the EVE base of two plus Targeting and Multitasking skill levels, still capped by the ship's `MaxLockedTargets` attribute. Cap diagnostics report tracked, locked, and currently-locking counts separately.
- Added per-shot drone damage-component and post-`ApplyDamage` shield/armor/hull delta diagnostics. These distinguish a failure to enter the weapon loop, missing static damage values, and a failure inside the common damage application path.

**Validation status**
- This is source-level implementation only. The new branch head requires Windows, Linux and Docker CI.
- In-game validation must confirm that the combat light drone's logged thermal/kinetic/etc. base damage is nonzero and that the post-ApplyDamage delta is positive; also verify no distant auto-acquisition and that the visible target list agrees with server lock counts. No in-game damage success is claimed yet.

- Follow-up hardening: null target-map keys/entries are now removed before capacity checks as well, so malformed empty map records cannot consume one of the player's target slots.


### 2026-11-15 — Drone lock callback and target-manager stability follow-up

- Fixed target-manager processing invalidation: the global entity list now iterates a snapshot and revalidates each manager after processing, so stale-target pruning cannot invalidate the active iterator when a manager unregisters itself.
- Target-lock completion now calls the owning entity's virtual `TargetAdded` callback. Drones therefore receive the actual lock-complete event, update their client-visible target ID, and re-evaluate approach/combat distance at the correct point in the lock lifecycle.
- Restart an invulnerability timer that is still marked enabled but has already reached 0 ms. The expired timer is replaced; a timer with positive remaining time is left intact.

**Log interpretation and validation**
- The attached log's stack trace is emitted by `EvE::traceStack()` in `Client::SetInvulTimer()` after the 0 ms remaining-timer warning. The log continues afterward, so that trace alone is not proof of a fatal server crash.
- The excerpt still does not contain a symbolized fatal crash report or drone fire/damage output. Runtime drone damage and any later process termination must be checked separately after this build.


## 2026-11-15 — combat crash, drone orbit and partial flight command hardening

- Hardened target-manager teardown against stale raw entity pointers. Targeter records now retain the targeter's entity ID; incoming targeter cleanup resolves that ID through the live solar-system registry before dereferencing the pointer. Outgoing target cleanup and target-list queries also use retained IDs rather than dereferencing potentially stale keys.
- Hardened target lookup used by drone AI: `GetFirstTarget()` and `GetTarget()` now verify that the retained ID still resolves to the same live entity before returning a pointer.
- Added null/dependency checks to target registration and lock completion. The entity death path now relies on `TargetManager::Destroyed()` / `ClearAllTargets()` to clear both outgoing and incoming target references before the manager is deleted.
- Changed drone combat orbit selection to use `AttrOrbitRange`, not `AttrMaxRange`. If old static data omits `orbitRange`, the weapon-range/falloff fallback is capped at 2,500 m (500 m if no useful value exists), preventing a missing orbit attribute from turning a weapon range into a very large idle/engagement orbit.
- Drone home-ship access now resolves the controller ID against the live system registry. Return, idle orbit, control-range checks and scoop no longer blindly trust a potentially stale non-owning ship pointer.
- Fixed partial `CmdEngage` selection recovery: if only some client-supplied drone IDs resolve, the handler now finds and commands the remaining eligible drones controlled by the current character and ship, without issuing duplicate orders to drones already handled.
- Validation: current head requires fresh Windows/Linux/Docker CI and in-game tests. The supplied log does not include a fatal crash stack; the turn-percent diagnostic is clamped in `DestinyManager::Turn()`, and the FX ID 0 warning is a missing weapon effect mapping rather than evidence of a server crash.

## 2026-11-15 — drone focus-fire and combat effect safety follow-up

- Drone auto-acquisition now respects the ship's Aggressive setting. Passive drones no longer independently acquire and attack nearby NPCs without an explicit engage order.
- When Focus Fire is enabled, drones controlled by the same ship share the first valid assigned target. The target assignment is recorded before lock completion, so other drones can join while the first drone is still locking.
- Drone attack processing now resolves the assigned target ID specifically instead of taking whichever locked target happens to be first in the target-manager pointer map. This avoids firing at an unintended target when stale or multiple target entries exist.
- Hardened FxProc::ApplyEffects() against missing source item references and missing fitted modules. A charge whose inventory flag is flagNone is skipped as an invalid/unfitted charge modifier; the processor no longer emits a full stack trace for every such data condition.
- The supplied 934-line log ends at 12:17:13 with no fatal exception, access violation, process-exit record or crash dump. The stack traces adjacent to FxProc::ApplyEffects() are generated by EvE::traceStack() in the flagNone diagnostic, not proof of the fatal crash site. The log also shows repeated destiny turn diagnostics, missing weapon effect mappings and unload attempts for unloaded railguns; these are separate issues and are not sufficient to identify the crash root cause.
- Runtime validation remains required. The log does not contain a drone firing/damage trace, nor a symbolized fatal crash stack. CI and in-game testing are required before calling the crash fixed.
