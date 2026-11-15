# Live Patch Notes — xrizonx/evemu_Crucible

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
