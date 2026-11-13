# Live Patch Notes — xrizonx/evemu_Crucible

This file tracks fixes and maintenance changes made on this fork's `master` branch. Entries describe code changes, not a guarantee that every gameplay path has been manually tested.

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
