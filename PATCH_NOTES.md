# Live Patch Notes — xrizonx/evemu_Crucible

This file tracks fixes and maintenance changes made on this fork's `master` branch. Entries describe code changes, not a guarantee that every gameplay path has been manually tested.


## Upstream contribution summary — 2026-11-13

This PR proposes the current fork work against `EvEmu-Project/evemu_Crucible:master`. The fork is 58 commits ahead of the upstream merge base at preparation time. This is a grouped summary of the complete proposed change set, not a claim that all gameplay features are finished.

### Mission system — implemented foundation; combat encounter work in progress

- Added encounter mission offer loading, lookup and persistence support using the existing `agtMissions` data, including briefing and reward metadata.
- Added security-agent mission selection and Level 0 progression, including a global starter security-mission tier and checks that prevent duplicate active starter offers.
- Added expiry-aware active-offer checks so expired Level 0 offers do not block new starter missions.
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
