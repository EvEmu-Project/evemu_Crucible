# Full removal of experimental station-agent image support

**16 November 2026 — compatibility rollback for EVE Crucible (2012)**

The user reports that the Crucible client still exits/crashes upon station load after PR #21 suppressed NPC agent images. The attached 08:11:12–08:11:17 `eve-server.exe` log shows around 16 repeated *unsymbolized server-side* call stacks during Maurasi (30000140) initialization, followed by station/faction-war RPCs and a logout. It does **not** show an Image Server request or a symbolized client crash. Agent portraits are a suspected temporal correlate, not a proven cause.

## Changes in this rollback

- `src/eve-server/imageserver/ImageServerConnection.cpp` was restored **byte-for-byte** from pre-agent-feature commit `298aacbd6c60732eb3f68ec7a17315d5e8fcda91`, matching already-restored `ImageServer.cpp` and `ImageServer.h`. There are **no extra agent-specific checks, hardcoded JPEG bytes or temporary HTTP guards**.
- The `IsStationAgentPortraitRequest` helper and its unit tests have been removed.
- The optional `tools/agent_portraits/` scripts, their CI tests and the generic `assets/agent_portraits/agent_missing_512.jpg` were deleted.
- Original image behavior now applies: the image server may read existing cached portraits or return its previous HTTP redirect for missing image types. This matches the behavior before the experimental new agent images were introduced. **Existing locally cached experimental JPEG files in your deployment remain your responsibility; do not copy them to the clean server while testing.**
- Agent offers, station agent metadata and conversations, PR #20 cargo-container assembly, Blockade rewards, market seeding, weapons, drones and warp code were not modified.

## Clean Windows deployment (important)

1. Stop all `eve-server.exe` processes and close the EVE client. Back up MariaDB, `etc` configuration and persistent caches. **Do not reinstall or reseed SQL.**
2. Extract the new Windows UCRT64 artifact into a **new empty directory**. Do not overwrite an old `bin` directory or mix DLLs from a different version. Read its `BUILD-INFO.txt` commit; it must match this PR's head commit or the final merged master SHA.
3. Copy only the necessary XML/database configuration and *known-good* image cache contents. For a clean image-path test, do **not** copy imported `image_cache/Character/<agentID>_<size>.jpg` files or the custom `image_cache/Character/agent_missing_512.jpg`; leave user/player character pictures intact if you need them.
4. In PowerShell verify the executable actually running:

```powershell
Get-CimInstance Win32_Process -Filter "name='eve-server.exe'" |
    Select-Object ProcessId, ExecutablePath, CommandLine
Get-Content "F:\PS1\2025 Download Brave\evemu-crucible-windows-x86_64\BUILD-INFO.txt"
```

The BUILD-INFO path is an example taken from the supplied log. Use your **new** installation path.
5. Log into the **same Maurasi station**. Compare before/after. Unlike the temporary PR #21 safety guard, this complete rollback intentionally restores normal image HTTP response behavior rather than guaranteeing 404.
6. Confirm agent conversations still work; assemble a purchased container to verify PR #20 was preserved. No DB migration is needed.

## What to capture if the client still crashes

- A Windows Event Viewer → Windows Logs → Application **client** crash event: exception code, faulting module, instruction offset.
- Any actual EVE client crash dump/log (not `eve-server.exe` stack frames); redact credentials.
- Server log starting before `AnomalyMgr Initialized for Maurasi(30000140)`, including the `DynamicEntityFactory::BuildEntity` warnings and logout.
- Exact `BUILD-INFO.txt` commit and **actual** `eve-server.exe` process executable path.

**Important diagnostic:** On 15 November commit [9a3a317068cf](https://github.com/xrizonx/evemu_Crucible/commit/9a3a317068cf0948b950b4e44aadefea75a15524) removed a call to `EvE::traceStack()` from `DynamicEntityFactory::BuildEntity` because unsupported entity rows spammed unsymbolized stacks during system loading. The supplied repeated trace looks similar; this **suggests a stale executable is possible**, but cannot establish the stack source without symbols. A new clean extraction is the most reliable test.

**Status:** Full feature rollback in source; runtime client crash remains unresolved pending a clean deployment and actual client results.
