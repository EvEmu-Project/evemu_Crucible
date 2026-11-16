# Crucible station crash triage: exact 50px agent image + safe agent/inventory paths

**Date:** 16 November 2026  
**Affected client:** EVE Crucible 1.6.5, build 360229  
**Operator report:** 09:36:27 local HTTP 404 for `/Character/3011918_50.jpg`, followed 2 seconds later by character logout and `[ThreadWarning] Destroying TCPConnection for thread 0x1`. The user confirms the newest .exe/.dll were installed.  
**Status:** automated source/build regressions are possible, but an actual client exception and live station-load success have **not** been observed yet.

## Evidence and diagnosis

The latest log establishes that the server accepted the agent picture request and sent an intentional **HTTP 404**, not an image. Two earlier attempts gave a modern HTTPS **301 redirect** or an incorrectly sized placeholder. The client quit after those responses as well. While there is a strong temporal association, the server's `Client::Logout` and `ThreadWarning` describe a disconnect; they **do not identify the faulting client instruction or prove image HTTP is the sole cause**.

The previous generic agent placeholder was a file named 512px but actually contained a 128px JPEG; it was served at the legacy 50px URL without resizing. The earlier 404 guards prevented that file from reaching the client but still gave the portrait loader no usable JPEG.

## Compatibility response in this branch

For **`Character` category, 3,000,000–3,999,999 NPC IDs, size exactly 50**:

- Return a deterministic, compiled-in, **322-byte baseline RGB JFIF JPEG, exactly 50 × 50 pixels**. This image is a neutral gray square, not a CCP portrait or a likeness of the agent. No runtime image files, online CDN, TLS, working directory or stale `image_cache` can override it.
- Send `HTTP/1.0 200 OK` with `Content-Type: image/jpeg`, exact `Content-Length: 322`, `Connection: close`, and `Cache-Control: no-store`. Response header is owned by the connection until asynchronous completion. This removes ambiguity from the older two-stage 200 response, which lacked `Content-Length`.
- **Other Character image IDs/sizes** keep their existing safe behavior: valid local JPEG if present, otherwise 404 with no modern HTTPS redirect. Corporation, alliance, inventory icon and render images retain their earlier routing; all local 200 images gain explicit HTTP body length and close semantics.
- The test uses the **actual embedded bytes**, verifying length, JFIF SOI/EOI and baseline SOF0 dimensions of 50 × 50. The original opt-in agent portrait importer remains deleted, and no external agent portrait is restored.

## Independent agent and inventory bugs corrected

The audit of agent mission, inventory and container code found these issues. The provided crash log does *not* show any corresponding agent RPC or AssembleCargoContainer call, so these are preventive reliability repairs, not claimed client crash root causes.

1. **Agent journal null bookmarks:** both `AgentMgrService::GetMyJournalDetails` and `AgentBound::GetMyJournalDetails` dereferenced `offer.bookmarks` with `->Clone()`. If a loaded offer lacks that pointer, return an empty `PyList` rather than crash the server.
2. **Malformed agent bind:** `AgentMgrService::BindObject` dereferenced a null `bindParameters` before checking its type; null and non-int binds now fail cleanly.
3. **Unloaded container inventory:** `InvBrokerBound::GetContainerContents` dereferenced `item->GetMyInventory()` even when null; now logs and refuses the request.
4. **Nontransactional stack split:** `InventoryItem::Split` previously decremented/deleted the original stack **before** trying to create the new item; if spawning failed, goods could disappear. The new implementation creates the temporary split first, only then decrements the original and removes the temporary item if the decrement fails. It refuses full-stack or singleton splits, avoiding source-object deletion/use-after-free.
5. **Quantity underflow destroys stack:** `InventoryItem::AlterQuantity` previously called `Delete()` on an insufficient quantity request. It now rejects the request without deleting the source item.
6. The above behaviors are guarded by the new `tools/verify_station_crash_safety.py` source-invariant CI step, in addition to existing CTest and Windows/Linux compilation.

**Not changed:** current station office/corporation fix from PR #23, assembled container groups and singleton/notification handling from PR #20, mission rewards (including 50m test Blockade), agent offers, market seed data, combat/drone/weapon math, Destiny/warp and client version protocol. No database schema migration or reseed.

## Verify on Windows 11 — decisive observations

1. Stop `eve-server.exe` and EVE client; back up MariaDB and configuration. Install the *new* matching Windows UCRT64 ZIP in a **new empty folder**, with its own DLLs, not on top of prior builds. Confirm `BUILD-INFO.txt` and actual running `eve-server.exe` executable path.
2. **Before running the client**, issue a direct GET without `-L`:

```powershell
curl.exe -sS -D agent-headers.txt -o agent-3011918.jpg "http://127.0.0.1:26001/Character/3011918_50.jpg"
Get-Content .\agent-headers.txt
Add-Type -AssemblyName System.Drawing
$img = [System.Drawing.Image]::FromFile((Resolve-Path ".\agent-3011918.jpg").Path)
try { "JPEG dimensions: $($img.Width)x$($img.Height)" } finally { $img.Dispose() }
```

Expected: `200 OK`, `Content-Type: image/jpeg`, `Content-Length: 322`, no redirect, `JPEG dimensions: 50x50`. The server should log `Serving exact 50x50 baseline JPEG placeholder for agent 3011918` and `Serving JPEG Character/3011918_50 (322 bytes, Content-Length specified)`.

3. With the client closed, use its cache-clearing option to remove *only image/HTTP cache*, **not** user settings. Old permanent HTTP 301 and 404 entries may be cached.
4. Launch the same Crucible build, enter the same station and verify whether it now stays open. Test the Agents tab, agent conversation/journal, market purchase, and assembled cargo container. The synthetic gray thumbnail is intentional during compatibility testing.
5. If it STILL crashes after a **verified 200/322-byte 50px** image response, the agent HTTP failure response can no longer explain it. Capture a client minidump or Windows Application Error event including the **faulting module and exception code**, plus 100+ server log lines before the disconnect. The thread warning is not itself a client stack trace.

The value of this patch is a controlled A/B image protocol test plus independently important data-safety fixes. CI checks do not claim a successful real-client station session.
