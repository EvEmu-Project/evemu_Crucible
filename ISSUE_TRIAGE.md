# Upstream Issue Triage — xrizonx/evemu_Crucible

Last reviewed: 2026-11-13  
Working branch: `master`  
Source tracker: [EvEmu-Project/evemu_Crucible/issues](https://github.com/EvEmu-Project/evemu_Crucible/issues)

## Scope

The upstream tracker currently exposes **128 open issue/PR records** across two API pages (100 + 28). The API's issue listing also includes pull requests. This is an inventory of the open records, not a claim that all 128 have received a full source-level audit. The items below are the subset inspected in detail or selected for follow-up.

## Implemented on this fork

| Upstream item | Area | Fork status | Verification still needed |
|---|---|---|---|
| [#139](https://github.com/EvEmu-Project/evemu_Crucible/issues/139) | Turret fire at asteroids can crash | Added null/state/numeric guards to `TurretFormulas::GetToHit()` | In-game asteroid targeting and turret fire |
| [#170](https://github.com/EvEmu-Project/evemu_Crucible/issues/170) | Probe processing iterator invalidation | Fixed safe iteration/removal in `EntityList::Process()` | Repeated probe recovery/removal in-game |
| [#275](https://github.com/EvEmu-Project/evemu_Crucible/issues/275) | NPC belt spawns disappear before warp-in | Implemented the two-part spawn fix from [PR #327](https://github.com/EvEmu-Project/evemu_Crucible/pull/327): security-rating scale correction and immediate warp for NPCs in empty bubbles | Spawn belts across security classes; observe NPC warp-in and combat |
| [#276](https://github.com/EvEmu-Project/evemu_Crucible/issues/276) | Dungeon editor acceleration gate creation | Corrected `AddObject` binding to accept the full nine arguments and nullable optional values | Test adding acceleration gates in the dungeon editor |
| [#319](https://github.com/EvEmu-Project/evemu_Crucible/issues/319) | MySQL reconnect option build compatibility | Replaced legacy `my_bool` usage with `bool` | Confirm all CI platforms pass |

The search-result projection mismatch, defensive DB result conversion, and DB row text bounds checks are additional fork fixes documented in [PATCH_NOTES.md](PATCH_NOTES.md).

## Priority investigation queue

| Priority | Upstream item | Finding so far | Next engineering step |
|---|---|---|---|
| High | [#154](https://github.com/EvEmu-Project/evemu_Crucible/issues/154) Pod-death crash | Historical stack trace reaches `Inventory::LoadContents(this=0x0)` from `Client::LoadStationHangar()`. Added null checks and a client-facing error path; underlying station/inventory lookup failure still needs tracing. | Reproduce clone respawn to the medical station and trace why the station or inventory lookup can fail; verify the guard prevents the crash. |
| [#188](https://github.com/EvEmu-Project/evemu_Crucible/issues/188) | Station inventory appears empty after system unload | Fixed stale client hangar marker handling: reload when the inventory itself is no longer loaded | Test by letting a station's system unload, then returning and verifying DB-backed contents reload |
| High | [#269](https://github.com/EvEmu-Project/evemu_Crucible/issues/269) Scanner crash after NPC kills | Reproduction sequence is specific, but no stack trace is supplied. | Reproduce with wrecks and multiple NPC kills; capture stack trace and inspect scanner result entity lifetimes. |
| Medium | [#39](https://github.com/EvEmu-Project/evemu_Crucible/issues/39) Docking while target-painted causes segfault | Old report; comments mention several historical target-manager fixes, but also later reports of continued crashes. | Re-test current branch with offensive modules active during docking, warp and session changes; inspect target/module cleanup ordering. |
| Medium | [#315](https://github.com/EvEmu-Project/evemu_Crucible/issues/315) Outpost construction crashes | Repro steps exist but reporter supplied no crash log. | Reproduce after anchoring and build timer; capture server stack before modifying construction flow. |
| Medium | [#276](https://github.com/EvEmu-Project/evemu_Crucible/issues/276) Dungeon editor acceleration gate creation | Binding signature has been updated to accept the client’s nine arguments and nullable optional values. | Test the editor interaction and check whether other object types need the same protocol handling. |

## Upstream pull requests worth reviewing

- [PR #327 — belt rat spawn class and warp-in](https://github.com/EvEmu-Project/evemu_Crucible/pull/327): the relevant code changes have been adapted to this fork and are recorded above.
- [PR #326 — broad gameplay fixes](https://github.com/EvEmu-Project/evemu_Crucible/pull/326): touches many unrelated systems. Review individual subsystems and test each change; do not merge the entire PR blindly.
- [PR #323 — persist ship position on logout](https://github.com/EvEmu-Project/evemu_Crucible/pull/323): review save timing, session changes and login warp semantics before adoption.
- [Issue #328 — ship physics claim](https://github.com/EvEmu-Project/evemu_Crucible/issues/328): reporter claims a fix but has not supplied a patch in the issue; no code has been adopted from it.

## Review rules

1. Confirm that the report still applies to this fork before changing code.
2. Prefer a reproducer, stack trace, failing test, or clearly demonstrated source defect.
3. Keep fixes narrowly scoped; avoid importing large unrelated upstream patches.
4. Build on Windows GCC, Linux GCC and Docker CI.
5. Test gameplay separately; compilation alone does not close a gameplay bug.
6. Never run destructive migrations or reset the user's live database as part of issue triage.
7. Update [PATCH_NOTES.md](PATCH_NOTES.md) after each committed fix and record its verification status here.
