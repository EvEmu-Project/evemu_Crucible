# Crucible station-load crash: complete agent image removal and Character isolation

**16 November 2026** — the user reports that the station-load crash persists after PR #21 prevented all NPC agent JPEGs. The attached excerpt from 08:11:12–08:11:17 shows repeated **server** stack traces in `eve-server.exe` while initializing Maurasi (solarSystemID 30000140), followed by `stationSvc.GetSolarSystem`, faction war queries and character logout. The stack traces lack symbols and the excerpt includes **no Image Server request**. This evidence does **not** establish portrait delivery as the cause.

## Source changes

- Deleted `assets/agent_portraits/agent_missing_512.jpg` and the entire optional `tools/agent_portraits/` importer/test feature, plus its Windows/Linux CI invocation.
- Removed `EvEAgent::IsStationAgentPortraitRequest` and its dedicated CTest assertions; existing agent-mission deadline and reward tests remain.
- The historical `ImageServer.cpp` and `ImageServer.h` implementations were already restored by PR #21 and remain unchanged.
- As a **temporary isolation measure**, `ImageServerConnection::ProcessHeaders` now immediately returns **HTTP 404** for **every** image with category `Character`, including **players and NPCs**. The guard executes before *any* local file lookup, embedded/fallback JPEG, or CDN redirect.
- Corporation/alliance logos, type icons and rendered-type images continue through the previous image service path.
- The cargo container `AssembleCargoContainer` fix from PR #20, station agent data, conversations, mission lifecycle and payouts, drones, weapons, market seeding and warp tunnel source are unmodified.

**Impact:** All character portraits (including the user's own character image) are temporarily blank. This change is for **diagnostic isolation**, not a final solution. If the client still crashes, the uploaded logs and this isolation result point strongly toward inspecting game data/protocol or server stack traces rather than attempting more JPEG changes.

## Operator verification — Windows 11

1. Stop the server and close the Crucible client. Back up your MariaDB instance and configuration, and preserve the original `image_cache` untouched. Do **not** reseed the market or reinstall the database.
2. Extract the freshly built **Windows UCRT64 artifact of this branch or its merged master commit** into a **new empty directory**, not over an existing server's DLLs. Copy only your config/SQL connection settings and required persistent directories. Check `BUILD-INFO.txt` to ensure the running server executable belongs to the new SHA.
3. Start this server and confirm that the image server is available. Verify that the Character category is entirely suppressed:

```powershell
curl.exe -sS -D agent-headers.txt -o NUL "http://127.0.0.1:26001/Character/3011918_50.jpg"
curl.exe -sS -D player-headers.txt -o NUL "http://127.0.0.1:26001/Character/90000001_512.jpg"
Get-Content .\agent-headers.txt
Get-Content .\player-headers.txt
```

Expected for **both** requests: **404 Not Found**, with no `Location:` header and no JPEG body. The player ID shown is illustrative; the HTTP guard blocks all IDs in this category.
4. Open a compatible 2012 Crucible client and log into **the same Maurasi location and station** that consistently crashed. If this now succeeds, the failure is related to an image request or side effects of its delivery. If it **still crashes**, portrait delivery is no longer a plausible immediate trigger.
5. Confirm mission agents remain listed and conversations work (pictures will be blank). Assemble a station container to ensure PR #20's fix remains intact. Check market buying, weapons and warp in separate safe tests.

## If crashing persists

The supplied server excerpt contains roughly 160 unsymbolized `[unknown function]` stack frames, repeating the same stack ~16 times during solar-system initialization. This is **not a decoded client crash backtrace**. Do not infer the crashing instruction from those raw addresses.

Capture:

- The Windows Event Viewer → Windows Logs → Application event for the **client** (exception code, faulting module, offset).
- Any client `*.dmp`/crash log from the EVE client install/user profile, if generated. Avoid sharing passwords or auth tokens.
- `eve-server` log **50–100 lines before** the first repeated stack and through character logout, including its log category, plus server commit SHA and client build.
- Whether the server process itself remains running after client logout, and whether login outside this solar system works.
- The exact `BUILD-INFO.txt` `Commit:` line from the new installed package. On Windows, check Task Manager's `eve-server.exe` file location to avoid an older process.

To symbolicate server frames, preserve the **exact matching binary and debug symbols** for the reported build, and resolve addresses with its load base (ASLR relocation). Guessing functions from the raw `0xd...` addresses is unreliable.

## Regression status

Windows/Linux CTest and the repository's build/packaging gates can verify the new source compiles and the all-Character HTTP guard is present. Only a real 2012 client loading into Maurasi proves whether the crash persists. Do not label the issue fixed until then.
