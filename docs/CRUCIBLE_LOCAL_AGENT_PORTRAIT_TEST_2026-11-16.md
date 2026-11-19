# Crucible station crash: disk-backed NPC portrait A/B test
16 November 2026 — follow-up to PR #25

## What changed
Crucible still uses the HTTP image URL advertised during handshake. Replacing
that URL with a direct Windows file path would require modifying the 2012 client.
The server now **uses the local HTTP endpoint solely as a transport** for
NPC character portraits; it reads the JPEG bytes from the server image folder.

For `GET /Character/3011918_50.jpg`, ImageServer reads, in order:
1. `<imageDir>/Character/3011918_50.jpg` (real or operator-supplied agent portrait).
2. `<imageDir>/Character/agent_fallback_50.jpg` (optional generic local fallback).
3. If neither file exists or the selected file has an invalid JPEG signature,
   respond with 404 (no external Character CDN redirect).

No embedded 322-byte JPEG can intercept that request. For legacy agent 50px
requests, the server **does not substitute a 512px portrait** under the 50px URL.
Other Character sizes and unrelated icon categories retain their previous
behavior. The explicit Content-Length and connection-owned HTTP headers from
PR #25 remain unchanged.

## Windows 11 test
1. Build/download the updated **PR #25 branch** and replace the corresponding
   `eve-server.exe` and DLLs together. The original PR #25 binary always
   served the embedded 322-byte asset regardless of local JPEG files.
2. Find the exact `Image Server path:` line at server startup. Default config
   is `../image_cache/` relative to **the process working directory**, not
   necessarily the directory containing `eve-server.exe`.
3. Run the included PowerShell script. Example from a folder containing your
   server distribution (replace the path to match the startup log):
   ```powershell
   powershell.exe -ExecutionPolicy Bypass -File .\tools\prepare-crucible-agent-portrait.ps1 -ImageDir 'F:\path\to\evemu-crucible-windows-x86_64\image_cache' -AgentId 3011918
   ```
   This creates a freshly encoded generic 50×50 JPEG in
   `Character/3011918_50.jpg`. Use `-SourceJpeg path\portrait.jpg` to resize
   an existing legitimate JPEG instead. `-GenericFallback` creates
   `agent_fallback_50.jpg` for missing NPC agents. Existing files are not
   overwritten without `-Overwrite`; replacements are backed up.
4. With the updated server running:
   ```powershell
   curl.exe -sS -D agent-headers.txt -o agent-response.jpg "http://127.0.0.1:26001/Character/3011918_50.jpg"
   Get-Content .\agent-headers.txt
   Add-Type -AssemblyName System.Drawing
   $image = [System.Drawing.Image]::FromFile((Resolve-Path .\agent-response.jpg).Path)
   try { "Dimensions: $($image.Width)x$($image.Height)" } finally { $image.Dispose() }
   ```
   Expect `HTTP/1.0 200 OK`, `Content-Type: image/jpeg`, a
   `Content-Length` matching the **local file's byte length**, and
   `Dimensions: 50x50`. The response should not be the old 322-byte payload.
   Look for `Serving disk JPEG for agent 3011918 ... from ...` in the log.
5. With the client closed, clear **only its HTTP/image cache** (do not delete
   the user's settings). Run the same station load and record whether the
   client stays active.
6. If it still exits with a verified 50×50 file-based HTTP 200 response,
   capture the Windows **faulting module, exception code, and client crash
   dump**. A server-side `Client::Logout()` is a disconnect event, not proof
   of a JPEG decoder crash.

## Rollback
Restore the original Windows build (and matching DLLs), and remove the test
`Character/3011918_50.jpg` or restore its automatically created backup.
No SQL schema changes, character inventories, missions, containers, drones,
weapons, or station data are modified by this test.

**Validation status:** the source change is in the PR; successful Crucible
station loading has not been demonstrated. Review CI and verify on the actual
client before merging.
