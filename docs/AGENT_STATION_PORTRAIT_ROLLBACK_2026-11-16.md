# Crucible station-load crash: NPC agent portrait rollback (16 November 2026)

**User report:** the 2012 Crucible client crashes immediately when loading into a station after the local station-agent portrait fallback was enabled by PR #20. The exact client image decoder failure is not confirmed; this is a conservative safety rollback.

## What was disabled

NPC agent image requests with paths `/Character/{agentID}_{size}.jpg` and agent IDs **3,000,000–3,999,999** now return **HTTP 404 before the server attempts to load any local agent image**. The guard prevents both the packaged fallback and stale user-imported agent JPEGs from being served. There is no outbound redirect for these agent IDs.

The image server sources were restored to their pre-offline-agent-portrait state (pre-PR #17 baseline commit `298aacbd6c60732eb3f68ec7a17315d5e8fcda91`) while retaining the early HTTP 404 guard. The local JPEG installation step was removed. The optional historical image importer remains only in the source tree as a dormant tool; **do not use it while agent images are disabled**.

**Expected limitation:** station NPC agent pictures are blank. Agent metadata, interaction, journal and mission lifecycle remain unchanged; PR #20 cargo-container assembly, market seeding, weapon/drone and warp tunnel changes are retained.

## Windows 11 rollout

1. Stop `eve-server.exe` and close the EVE client. Back up MariaDB, `etc/` and `image_cache/`. Do not reinstall the database or reseed the market.
2. Use the new Windows UCRT64 ZIP produced from the rollback PR/merged master. Check `BUILD-INFO.txt` for the corresponding commit SHA. The previous PR #20 executable is **not** safe for this regression test.
3. Start the server as usual. Do not delete existing user portraits; the new early 404 guard blocks NPC agent images regardless of their cache state.
4. Verify the previously affected agent 3011918 with a real HTTP **GET**:

```powershell
curl.exe -sS -D agent-headers.txt -o NUL "http://127.0.0.1:26001/Character/3011918_50.jpg"
Get-Content .\agent-headers.txt
```

Expect **404 Not Found**, **no Location redirect**, and no JPEG response. A blank portrait is intentional for now.
5. Log in with the Crucible 2012 client, dock in a station containing NPC agents, open station Agents and a mission conversation. It should no longer crash **if the portrait payload was the cause**. If it still crashes, collect a client crash dump, Windows Event Viewer application error and the server log with UTC timestamps.
6. Confirm previously purchased station cargo containers can still be assembled, including a stack of three resulting in one singleton and two packaged items. Verify player portraits, market buying and warp are still usable.

**CI limitation:** Linux/Windows/Docker builds and the guard regression can only establish source/build behavior. They cannot prove the live client no longer crashes; verify with the same client build and station that reproduced the problem.

## Historical documentation

The earlier [offline portrait guide](AGENT_LOCAL_PORTRAITS_2026-11-15.md), [agent UI audit](AGENT_STATION_IMAGE_AUDIT_2026-11-15.md) and [PR #20 fix notes](CONTAINER_ASSEMBLY_AND_AGENT_FALLBACK_FIX_2026-11-16.md) describe superseded code; they are not current portrait-deployment instructions.
