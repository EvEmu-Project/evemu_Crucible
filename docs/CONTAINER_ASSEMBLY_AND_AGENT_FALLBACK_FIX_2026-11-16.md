# Crucible regression repair: cargo container assembly and station-agent portrait fallback

> **DISABLED after Crucible station-load crash (16 November 2026):** Serving NPC station-agent JPEGs was rolled back for client compatibility. Current server builds return **HTTP 404** for all agent portrait requests before reading the image cache. **Do not follow historical portrait-import instructions below** until a client-verified fix is available. See [safety rollback](AGENT_STATION_PORTRAIT_ROLLBACK_2026-11-16.md).

**Date:** 16 November 2026  
**Reports from running Crucible client:**
- `07:19:18 W Image Server: Local agent portrait and fallback missing for agent 3011918.`
- Repeated `InvBrokerBound::Handle_AssembleCargoContainer(): size=3` at 07:20:03/06/08, with no visible change.

## Findings

1. `InvBrokerBound::AssembleCargoContainer` in the merged server was a logging-only stub returning `nullptr`. The client correctly sent three positional RPC arguments (item ID, None, 0.0), but the server never changed the purchased item's packed/assembled state. The new market seed in PR #16 only enables purchasing types; it cannot assemble them.
2. `ImageServer::GetImage` tried the per-agent cached JPEG, a custom `image_cache/Character/agent_missing_512.jpg`, and finally the path `../share/evemu/agent_portraits/agent_missing_512.jpg`. The last path was relative to the process working directory, not the location of `eve-server.exe` or `eve-server`. A correctly packaged release can therefore still return 404 if launched from another directory. Docker mounts and pre-existing installations can also change filesystem visibility. Agent ID 3011918 is in the expected Crucible 3-million range.

## Fix: assemble one player-owned station container

- Restrict the action to `Cargo Container` (group **12**), `Secure Cargo Container` (**340**), `Audit Log Secure Container` (**448**) and `Freight Container` (**649**) from the historical Crucible SDE.
- The caller must be **docked**, own the item personally, and have the item located in their **current station**. The server rejects unrelated/corporate items instead of allowing an unsafe inventory mutation.
- An unassembled quantity-one item changes from a packaged stack to a singleton using the existing `InventoryItem::ChangeSingleton(true, true)`, which also recalculates volume and dispatches `OnItemChange`.
- A stack with quantity above one first splits exactly one unit using the existing inventory split/notify path, then assembles **only that unit**. Other containers remain packaged and are not lost or converted as a group.
- Both newly assembled item and the remaining source stack are explicitly saved so assembly persists even if `saveOnUpdate` is false.
- Already assembled items, unsupported groups, missing items, wrong ownership and inappropriate locations are ignored safely with a specific warning, not silently mutated. Corp-owned container assembly is **not** enabled by this fix because role/delegation validation is not implemented.
- `ContainerAssemblyTest` checks all four allowed Crucible groups, owner/station restrictions, already-assembled state and invalid/empty stack quantities. This does not replace a live database and client validation.

**Important existing limitation:** the generic `InventoryItem::Split` path is not a SQL transaction. If a DB failure occurs between stack reduction and new item creation, it can still require reconciliation. Do not test on a populated production economy before a database backup.

## Fix: reliable local-only station-agent portraits

- Preserve operator images under `image_cache/Character/<agentID>_<size>.jpg` (and custom `agent_missing_512.jpg`) as first priority.
- Embed the exact **original generic fallback JPEG** already in `assets/agent_portraits/agent_missing_512.jpg` into `eve-server` as a static, decoded-once image. On missing local artwork, NPC agent requests return HTTP 200 JPEG directly from the executable even if the process runs from an unexpected working directory or without a `share/evemu` folder.
- Existing `ImageServerConnection` already serves these bytes with `Content-Type: image/jpeg`. No redirects, Internet requests, HTTP client, new CDN dependency, or runtime filesystem asset are required for missing NPC agents.
- The packaged `share/evemu/agent_portraits/agent_missing_512.jpg` remains an operator-editable asset; however, overriding the **embedded** default requires the explicit custom file in `image_cache/Character/agent_missing_512.jpg`.
- Python cache regression tests decode the exact in-source base64 JPEG and compare its bytes to the packaged JPG, preventing accidental broken placeholder data.
- **The fallback is not an authentic historical CCP agent portrait.** Add genuine agent portraits separately with the existing importer.

## Windows 11 validation

1. Take a MariaDB backup. Use a *new build directory* when extracting the fixed Windows x86_64 UCRT64 CI artifact, and keep the matching `bin` DLLs with the server executable.
2. Check `BUILD-INFO.txt` matches the new PR head or subsequent merge. An older server that contains the logging-only assembly handler will behave exactly as reported.
3. Keep your existing `etc/eve-server.xml`, `sql/evedb.yaml`, database and `image_cache` intact; do not rerun fresh database installation or destructive reseeding for this test. Start the new server and open a station where you own a market-purchased container.
4. With one packaged container: **Assemble Container**; verify the item's `singleton` state and volume change in the client, then open its cargo/inventory view if supported. Move an item into/out of the assembled container to confirm contents permissions.
5. Buy a stack of three containers: assemble once, confirm one assembled singleton and a remaining stack of two packaged units. Reconnect/restart and confirm no loss, duplication or reversion.
6. Attempt to assemble another player's item, a corporate-owned item, an existing singleton and a non-container item. No item mutation is permitted.
7. Request the failed agent portrait directly (send a real **GET**, not HEAD):

```powershell
curl.exe -sS -D agent-3011918-headers.txt -o agent-3011918.jpg "http://127.0.0.1:26001/Character/3011918_50.jpg"
Get-Content .\agent-3011918-headers.txt
```

Expected: **HTTP 200**, `Content-Type: image/jpeg`, no `Location` redirect, nonempty image bytes; station Agents tab renders the generic image when a unique portrait is unavailable. Test with outgoing HTTPS blocked. If connecting from a different machine, replace 127.0.0.1 with your configured server image address. Verify the old warning for agent 3011918 is absent.

## Linux / Docker

Run the new build containing the change, not just the updated documentation. `Dockerfile` still packages the JPG for reference, but the built-in fallback works even if an `image_cache` volume masks files. For a Docker client on another host, configure the advertised `imageServer` to a reachable IP and expose TCP port 26001. Run container assembly tests only against a disposable MariaDB test copy.

## Acceptance and unresolved work

- [ ] Windows, Linux, Docker build and CTests pass at the new head SHA.
- [ ] One container assembles with persistent singleton/volume, remains usable as an inventory; a multi-item stack loses only one packaged unit.
- [ ] Unauthorized, unsupported and already-assembled items do not mutate.
- [ ] Agent `3011918` returns local HTTP 200 JPEG without relying on the `../share` path; station Agents tab shows fallback/real local JPEG without external network.
- [ ] Market orders, NPC sell seeding, previous mission payout fixes, drones, weapons and warp tunnel behavior stay unchanged.

These are **source-level fixes** until live client and MariaDB validation completes. Do not label them verified in-game merely because CI succeeds.
