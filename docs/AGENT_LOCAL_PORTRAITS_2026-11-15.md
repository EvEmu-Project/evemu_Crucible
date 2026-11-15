# Offline station-agent portraits — implementation and rollout (15 November 2026)

## Goal

A Crucible client requesting `GET /Character/{agentID}_50.jpg` from the **local** EVEmu image server must never need to follow an external HTTP or HTTPS redirect for NPC agents. The server must work with Windows 11, Linux GCC and Docker while preserving existing player portraits and non-agent image behavior.

## Source findings

- The server accepts legacy 50px agent portrait URLs and maps them to local files in `<imageDir>/Character/`.
- Previously, missing NPC agent portraits redirected to `https://images.evetech.net/characters/{id}/portrait?size=64`. This requires network access and does not reliably provide every historical NPC portrait.
- No agent JPEGs were committed to this fork as of the 15 November 2026 base commit. NPC agent IDs are in the **3,000,000–3,999,999** range (see `EVE_Defines.h`).
- Existing station/agent database metadata fixes remain intact. This patch does not synthesize true historical faces.

## What changed

1. `ImageServer::GetImage` serves an exact local JPEG or reuses available local `512/256/128/64/50/40/32/1024` variants. The 2012 client can request 50px even when the downloaded JPEG is 256px.
2. If a requested `Character` ID belongs to the legacy agent range but no portrait exists, the server uses a **locally bundled, original, intentionally generic silhouette**. Its file is `share/evemu/agent_portraits/agent_missing_512.jpg`, installed by CMake (so existing Docker `image_cache` volumes cannot mask it). Operators can override it at `image_cache/Character/agent_missing_512.jpg`.
3. NPC-agent image requests no longer redirect externally, even when the fallback file is missing: they return HTTP 404 locally instead.
4. The one-time optional utility `tools/agent_portraits/sync_portraits.py` reads `sql/base/agtAgents.sql.gz` without MariaDB, requests official JPEG portraits for known NPC agent IDs, validates JPEG headers and content types, ignores a generic CCP fallback image when it matches the default, skips existing local portraits, and writes `image_cache/Character/{agentID}_256.jpg` atomically. This utility needs Internet access **only when explicitly invoked**, not when the server or client runs.
5. JPEG fallback asset, Python parser tests and CI cache verification are packaged into the Windows/Linux/Docker build pipeline. **Authentic external portraits are not automatically downloaded or committed to the public repository**, because their availability and rights are separate from the original fallback graphic.

## Windows 11 — populate portrait cache once

In a **new unpacked release directory**, first ensure the packaged `share/evemu/sql/base/agtAgents.sql.gz` archive exists. Python 3.10+ is needed **only to run the optional importer**. No Python installation is needed to run EVEmu.

From PowerShell in the release root (the directory containing `bin`, `share` and `image_cache`):

```powershell
py -3 .\share\evemu\tools\agent_portraits\sync_portraits.py --verify-only --limit 10
py -3 .\share\evemu\tools\agent_portraits\sync_portraits.py --limit 50
```

The first command makes **no network requests**. The second tries the first 50 agent IDs and writes valid available JPEGs. Once satisfied, populate the full cache:

```powershell
py -3 .\share\evemu\tools\agent_portraits\sync_portraits.py
```

To use a custom image root (when `etc/eve-server.xml` overrides `<imageDir>`), specify:

```powershell
py -3 .\share\evemu\tools\agent_portraits\sync_portraits.py --output "D:\EVE\image_cache\Character"
```

Keep any original 2012 portraits you already own as `<agentID>_50.jpg` or `<agentID>_512.jpg`; the importer will not overwrite them. In the public image service, some historic agents will return HTTP 404 or a default portrait. These agents intentionally display the locally shipped silhouette instead.

## Linux source checkout

```bash
python3 tools/agent_portraits/sync_portraits.py --verify-only --limit 10
python3 tools/agent_portraits/sync_portraits.py --limit 50
# On a completed install use --output pointing to the configured imageDir/Character.
```

Docker: the fallback stays in `/app/share/evemu/agent_portraits/` even when `/app/image_cache` is a persistent volume. Populate the named `image_cache` volume from the host or a one-time maintenance container if genuine portraits are wanted. **The running EVEmu image server never calls CCP.**

## Validation checklist

- Confirm `share/evemu/agent_portraits/agent_missing_512.jpg` exists in the release and has JPEG contents.
- Confirm that a local portrait `image_cache/Character/3010819_256.jpg` (if available) is served for `GET http://127.0.0.1:26001/Character/3010819_50.jpg` with HTTP **200**, `Content-Type: image/jpeg` and **no `Location` header**. If the matching portrait is absent, verify that the fallback JPEG is returned instead.
- Try an agent with missing `chrNPCCharacters` portrait metadata: agent list population and images are independent.
- Restart the server with outgoing network blocked and ensure station-agent portraits still load. Verify a genuinely uncached player portrait is not replaced with the generic NPC silhouette.
- Test a second 2012 client on a LAN using the configured reachable image-server address and port 26001.
- Validate that player portrait uploads and optional external **non-agent** logos/icons continue to work.

## Rights and limitations

The bundled fallback silhouette is original placeholder artwork, **not** a historical CCP portrait. Genuine downloaded CCP images remain CCP property. Per [CCP's current Content Creation Terms](https://support.eveonline.com/hc/en-us/articles/8563917741084-EVE-Online-Content-Creation-Terms-of-Use), noncommercial community uses may be allowed subject to conditions; check the terms before redistribution. The importer is opt-in and stores available portraits on the operator's local filesystem. No canonical full archive of authentic Crucible agent portraits has been located or included in this PR. No in-game verification has yet been performed.
