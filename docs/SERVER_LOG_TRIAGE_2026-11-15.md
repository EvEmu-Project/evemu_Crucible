# EVEmu Windows server log triage — 2026-11-15

## Log examined

User-supplied 207-line excerpt recorded at approximately 16:26:30–16:26:36
while a character/system session loads Maurasi (30000140).

- System BeltMgr, SpawnMgr, DungeonMgr and AnomalyMgr initialization messages
  appear before **16 identical 12-frame native call stacks**.
- Each stack contains the same unsymbolized eve-server.exe addresses (starting
  at `0x233cf377`) with Windows thread-start frames. **There is no named
  exception, access-violation code, assertion, or final process termination in
  this excerpt**. The log continues with cached object requests afterward.
- At 16:26:36 the image server emits `RedirectLocation() called` at ERROR
  severity for `Character/3011918_50`. The source uses a 301 fallback when
  local artwork is absent; this is not by itself a fatal error.
- `WarRegistryBound: Called GetWars stub` is an unimplemented placeholder; it
  is not a confirmed crash source.

The exact 12-frame call stack cannot be located reliably in source without
the **matching executable and symbols**, module load base, or a symbolized
minidump. The duplicated trace may be a debug stack dump rather than an actual
crash; attributing it to one function would be speculative.

## Source-level defects fixed during this pass

1. `BubbleManager::Remove()` previously called `Untrack()` on **all** bubbles,
   even when they did not own the entity, and called `Remove()` again on its
   actual bubble. That could trigger duplicate ball removals, stale bookkeeping
   and repeated debug traces. The list loop also had a null-entry `continue`
   without advancing its iterator (infinite loop).
2. `SystemBubble::Untrack()` now ignores unregistered/double-untracked entities
   and logs a specific warning if a bubble contains a stale registration for
   an entity owned elsewhere. It cleans that stale registration rather than
   emitting repeated unlabelled stack dumps. `Remove()` no longer clears the
   owning pointer when called on the wrong bubble.
3. `SystemBubble::Add()` now rejects a duplicate dynamic entity before
   overwriting `m_bubble`. A debug log had reversed `%p` and `%s` arguments,
   which could dereference an object pointer as a C string when enabled.
4. `SystemManager::LoadSystemDynamics()` compared a *type ID* to the
   `EVEDB::invGroups::Wormhole` group constant; it now compares the group ID.
5. `ImageServerConnection` no longer treats an expected redirect as ERROR.
   `ImageServer` accepts legacy 50px character requests, reads exact-size
   local images when available and falls back to a stored 512px portrait.
6. `ImageServer::ImageServer()` creates category subdirectories regardless
   of whether the base directory was newly created or already existed,
   and handles an empty configured base directory.
7. `ImageServer::ReportNewImage()` replaces stale uploads for repeat account
   uploads. `ReportNewCharacter()` guards missing/empty image data, fopen
   failures and short writes instead of dereferencing an empty vector or null
   FILE pointer; it retains an upload if saving fails.

These are **code fixes**, not proof that the repetitive stack in the provided
excerpt was caused by any one of these paths.

## Remaining diagnostics and tests

- Run a **fresh Windows build from the passing master CI run**. Check
  `BUILD-INFO.txt` and preserve the same `eve-server.xml` and MariaDB backup.
- Reproduce system loading, system changes, launch/scoop/return drone, and
  portrait requests. Enable DESTINY bubble tracing only while reproducing.
  Verify that no repeated unsymbolized stack spam and no missing bubble
  entities remain.
- Test the image server with existing 512px portraits requested at 50px,
  missing artwork receiving HTTP 301, and a newly uploaded portrait.
- Obtain a **matching Windows EXE with debug symbols** and a crash dump if
  a fatal crash remains. Native absolute addresses may move across builds.
- The WarRegistry `GetWars` handler is still a stub; implementing the full
  game protocol and database responses is separate feature work.
- A complete line-by-line audit of 1,312 repository tree entries, all SQL
  migrations and live game behavior is outside the scope of this pass.
  Investigate remaining errors using full logs and isolated reproductions,
  rather than hiding warnings or adding speculative fixes.

Prior audits: [Drone and Motion](DRONE_AND_MOTION_AUDIT.md),
[Patch Notes](../PATCH_NOTES.md).
