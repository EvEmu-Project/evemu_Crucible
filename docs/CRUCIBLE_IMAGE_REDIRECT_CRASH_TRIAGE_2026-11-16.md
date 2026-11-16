# Crucible 2012 station-login crash: missing Character image redirect

**2026-11-16 — narrow compatibility patch, not a confirmed full crash repair**

## Latest operator evidence

The operator reports using Windows Actions run #43820377960 (PR #23, SHA `f5327f4fb1cd383f423d629f4b8c1db5bfca2852`). The final server lines were:

```text
09:09:49 C ImageServer: GetImage() called. Cat: Character, id: 90000000, size:256
09:09:50 C ImageServer: GetImage() called. Cat: Character, id: 3011918, size:50
09:09:50 D Image Server: Redirecting missing image Character/3011918_50 to fallback.
09:09:52 G Client::Logout(): ... logging out.
```

These lines establish that an **HTTP 301 redirect** was initiated for agent `3011918` just before logout. They do **not** demonstrate a client exception instruction, and are not proof the redirect caused the crash. The prior PR #21 tried 404 for NPC agents without resolving the user-reported crash.

## Source diagnosis

After PR #22 removed experimental agent JPEG assets, the original image-server response code still called `EvEAgent::ModernImageUrl("Character", 3011918, 50)`, which generated `https://images.evetech.net/characters/3011918/portrait?size=64`. A Crucible v1.6.5 (build 360229) client requesting a legacy 50px portrait may not support the external modern CDN, TLS handshake or response encoding.

This patch **does not reintroduce agent portraits**. Missing local `Character` images return 404 without redirect. Valid existing local JPEGs continue to be served; non-Character icons/logos/render fallback behavior stays unchanged. The 404 has an empty body and explicit close/no-store headers.

A second possible image-path fault is a non-JPEG upload or corrupt cached file being served with `Content-Type: image/jpeg`. `ImageServer::GetImage` now rejects empty files and files without the JPEG SOI signature (FF D8 FF). This is only a file signature check, not complete JPEG validation.

## Verify HTTP behavior using Windows PowerShell

1. Back up MariaDB and the full server `etc` config and caches. Stop all server processes. Unzip the new matching Windows build into an **empty directory**, keeping its matching runtime DLLs; retain PR #23's station fixes.
2. Check the package `BUILD-INFO.txt`, then verify the *running* executable path with `Get-CimInstance Win32_Process -Filter "Name='eve-server.exe'"`.
3. Run without `-L` (so a redirect cannot be silently followed):

```powershell
curl.exe -sS -D agent-headers.txt -o NUL "http://127.0.0.1:26001/Character/3011918_50.jpg"
Get-Content .\agent-headers.txt
```

Expected **404 Not Found** with `Content-Length: 0`, `Connection: close` and **no Location header**. The server must NOT log `Redirecting missing image Character/3011918_50`. This proves the new response path is running, but not client stability.

4. Test the player image URL using the same `id` and size as the log: `http://127.0.0.1:26001/Character/90000000_256.jpg`. It may return 200 for a known-good cached JPEG or 404 if the image is missing or invalid, **but never 301**.
5. Close the EVE client. Clear only image/cache files in the client cache (or use the in-client clear-cache control), not character settings. A previously received **301 permanent redirect may remain cached** until removed. Do not delete persistent server images without first backing them up.
6. Retest the same character/station. Record whether the agent picture is blank and whether the client remains open. Also test Assemble Container, missions, drones, weapons and warp after login.

If the 2012 client **still crashes**, stop treating image redirects as a proven root cause: collect a **Windows Application Error event (client process, faulting module and exception code)**, minidump if available, and the full server log starting before station load. This is necessary to distinguish client protocol, cached art and station state faults.

## Automated verification limits

CI CTests check the no-Character-redirect rule and JPEG signature helper, and full Windows/Linux/Docker builds exercise compilation. CI does **not** launch the actual 2012 Crucible client, query the user's MariaDB, inspect actual `90000000_512.jpg`, or prove the client crash fixed.
