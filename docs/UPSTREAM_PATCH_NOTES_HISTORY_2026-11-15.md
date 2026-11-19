# Historical contribution-branch patch notes (15 November 2026)

Preserved from the original upstream PR #335 source branch before integrating current fork `master`. This is historical and its CI/commit counts are superseded by the 17 November dossier.

---

# Live Patch Notes — xrizonx/evemu_Crucible

## 2026-11-15 — Agent data, station portraits and mission safety

- Agent station listing no longer relies exclusively on optional `chrNPCCharacters.stationID`: `ManagerDB::GetAgents` and `AgentDB::LoadAgentData` use `agtAgents.locationID` when it resolves to `staStations`, while preserving the legacy result columns.
- Individual agents no longer fail to load merely because the optional NPC bloodline record is missing. Genuine missing primary agent rows or query failures now fail cleanly; the `AgentData` structure is initialized to zero instead of exposing undefined values.
- The image server's missing-image redirects now use the supported `images.evetech.net` character-portrait/logo/icon/render APIs rather than the deprecated `image.eveonline.com` URL. Legacy 50px station agent portraits map to 64px remote requests while local `image_cache/Character/id_50.jpg` and `id_512.jpg` files continue to work. Historical NPC portraits absent from the remote CDN cannot be recreated by URL mapping; client support for HTTPS redirects must be tested.
- Local PNG images are now served with the correct `image/png` MIME type. Invalid zero image ID/size requests receive a 404 before attempting a redirect.
- Mission accept, complete, decline, defer and quit paths now validate that an agent actually has an offer in the required state before side effects; this prevents default-constructed missions and repeated completion payouts.
- Fixed time-bonus calculation: compare the current FILETIME with the acceptance timestamp plus the configured minutes, and show the actual bonus ISK rather than an estimated multiple of the base reward. Broken bonus check previously never paid eligible bonuses.
- Error handling now returns early if loading agent skill data from SQL fails.
- Added `AgentHelpersTest` for image URL mapping and bonus-time math and a dedicated [agent image/mission audit](docs/AGENT_STATION_IMAGE_AUDIT_2026-11-15.md). GitHub build and Crucible client gameplay verification pending for the new code.

This file tracks fixes and maintenance changes made on this fork's `master` branch. Entries describe code changes, not a guarantee that every gameplay path has been manually tested.


## 2026-11-15 — Complete upstream submission review snapshot

- Source: `xrizonx/evemu_Crucible:master`, commit `96450674fae13954e52009f88517a006d1c68a3b`, versus `EvEmu-Project/evemu_Crucible:master` `f55b488a006e3090470d3b481aee47a1d7a0dade`.
- **410 commits ahead, 0 behind, 137 changed paths, +9,316/−1,748 textual lines.** Earlier patch-note counts (e.g. 183) and pending CI notes were historical snapshots.
- [CI run 43742655629](https://github.com/xrizonx/evemu_Crucible/actions/runs/43742655629): **all passed** — Linux GCC + CTest + packaging, Windows UCRT64 GCC + CTest + portable ZIP, Docker image.
- New upstream-review inventory: [`docs/UPSTREAM_PULL_REQUEST_REVIEW_2026-11-15.md`](docs/UPSTREAM_PULL_REQUEST_REVIEW_2026-11-15.md). This contains detailed changes by subsystem, current test evidence, full file inventory and a reviewer acceptance matrix.
- Updates through the source head: missions/Blockade, NPC spawning, ship warp/approach and orbit/collision fixes, drone targeting/control range/return, turret and missile Crucible-era mechanics, defender/smartbomb, image server, null/data safeguards, CI packaging and EVEDBTool market seeding.
- **Still unverified or incomplete:** in-game 24/24 mission completion and payout; mining drone harvesting; full bomb and doomsday systems; continuous physical collision resolution; individual ship/module historical balance. Reviewers should not treat a green CI result as full gameplay verification.
- This is an upstream submission **for maintainer review** only; it does not merge the upstream branch or execute/approve database changes.

This file tracks fixes and maintenance changes made on this fork's `master` branch. Entries describe code changes, not a guarantee that every gameplay path has been manually tested.
