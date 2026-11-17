## 17 November 2026 — POS requested-position distance validation

- POS-10: SBU and orbital-platform anchor distance rules previously compared the stargate/planet to the structure old stored position, not the requested target. Use the target position for both checks; extend the Windows/Linux source regression guard.

## 17 November 2026 — Sovereignty and POS source integrity and access fixes

- SOV-01/02/03: write 64-bit FILETIME claimTime as BIGINT instead of mismatched %f, preserve cached sovereignty on failed DB insert/delete/query, and count unique online SBU-controlled gates to prevent false contested states.
- POS-01/02/03/04: lookup silo capacity by real item type; authorize tower settings, passwords, anchoring and jump bridge link/unlink by owner and StarbaseConfig/Director roles or server ADMIN; validate malformed POS access flag packets before writes.
- POS-05/06/07/08/09: persist tower sentry status and integer deploy flags, exclude packed cargo towers from deployed list, replace MariaDB self-referential bridge unlink with a reciprocal join, and reject wrong-system/occupied/cross-alliance bridge requests.
- Added CTest SovereigntyPosIntegrityTest and Windows/Linux CI source guard, read-only SQL diagnostics, full review dossier, risk notes and manual test matrix. Existing TCU/IHub and belt security fixes preserved. No migration, SQL mutation or account modification.
- Remaining work: live reinforcement/fuel, multi-system bridge atomicity, corporation roles and mission/structure playtests. Not merged until tested.

## 17 November 2026 — Fix /dogma access for DEV and ADMIN

- The observed `0x62f8000280c41000` DEV account includes CONTENT but not ADMIN (`0x0100000000000000`). Old `/dogma` required ADMIN only.
- Register `/dogma` with explicit **CONTENT OR ADMIN** permissions, preserving current ADMIN access. Ordinary/VIP players remain denied, and other ADMIN-only commands such as /kick, /ban and /givelp are unaffected. Do not modify account SQL or grant blanket privileges.
- Reuse bitmask authorization in `CommandDispatcher` and `/commandlist`, show CONTENT commands, log role masks as real 64-bit hex values instead of pointer `%p` formatting.
- Reject missing ship, missing item and unknown dogma attribute safely before modifying the item. Add CTest permission regression coverage for DEV, CONTENT, ADMIN, BOSS, VIP, STD and multi-bit roles. Live command check and CI needed.

## 17 November 2026 — PR #36 DED & Officer Expedition reward to 500M ISK

- Increase only synthetic Security Agent mission 65534 reward to **500,000,000 ISK** (from 50,000,000). Mission 65535 Blockade remains **50,000,000 ISK**.
- Keep the existing single payout at successful mission turn-in, 8-NPC target, premium faction/officer/deadspace loot and two-hour time limit.
- Update agent briefing, C++ lifecycle test, static regression script and operator playtest guide. Existing uncompleted DED offers get the reward through normal offer normalization; no SQL migration or player-record rewrite is required.

## 17 November 2026 — Exclude untargetable True Sansha NPCs from PR #36

- Operator reported that True Sansha NPCs are invalid player targets in elite Security Agent mission 65534. Filter NPC candidate type names containing "True Sansha" case-insensitively, only during the new mission's selection. Preserve other legitimate Sansha NPCs.
- Validate candidate NPCs within each category before selecting two distinct groups; skip groups with no eligible types, fail clearly if any category cannot fill its two slots, and keep the objective at eight kills.
- No change to the Blockade, global spawns, inventories, reward or rare loot. Add source regression checks and a two-run in-game verification checklist.

## 17 November 2026 — PR #36 repeated mission pocket cleanup safeguards

- After one eight-NPC expedition reached 8/8, operator reported server exit on a later run; the supplied FX and bound-object warnings do not include a native exception trace.
- Retire the matching mission bubble's spawn registrations and flags on completion/quit; preserve wrecks and remaining NPCs. Prevent accepting a beacon into an unexpectedly occupied or mismatched bubble. Keep empty-bubble manager system/ID lookups consistent.
- Add source checks and repeated acceptance playtest instructions. Mission reward, eight-NPC spawn mix, rare loot, Blockade, player data and database schema unchanged. CI and live confirmation required.

## 17 November 2026 — DED & Officer Expedition eight-NPC balance

- PR #36 operator confirms actual elite ship types spawn correctly. Reduce only mission 65534 from 18 to **8 ships**: two each from DED/deadspace, faction commander, pirate officer and DED overseer categories, randomized across the existing 18 candidate NPC groups.
- Align briefing, journal kill target, static assertions, lifecycle tests and source tests. Already accepted 18-NPC encounters keep their recorded target. Do not change original 12-NPC Blockade, 50-million-ISK reward, 120-minute limit or rare wreck loot.
- No database migration or modifications to player, item or market records. CI and real-client playtesting remain required.

## 17 November 2026 — Separate DED & Officer Security Agent Expedition (pending client acceptance)

- Added synthetic Security Agent mission `65534` **DED & Officer Expedition (TEST)** alongside unchanged `65535` Blockade. Both are randomly offered by the same Security Agent divisions and tiers.
- The new mission spawns up to 18 real Crucible NPC ships: five DED/deadspace, five faction commander, five pirate officer and three DED overseer types spanning frigates through battleships and all five pirate factions.
- Each elite mission NPC adds a matched-tier, market-listed **Faction (meta group 4), Officer (5) or Deadspace (6)** module to its wreck via the static type catalogue. Normal NPC loot, bounty, salvage and all non-elite missions are unchanged.
- Agent briefing, journal, mission-pocket kill count and two-hour turn-in timer reuse the established Blockade lifecycle; reward remains 50 million ISK.
- Added `MissionLifecycleTest` assertions, Windows/Linux source regression checks and [gameplay test guide](docs/missions/DED_OFFICER_EXPEDITION_TEST_MODE.md). Real player-client verification still required.

## 17 Nov 2026 — Repair NPC buy-order seed MariaDB Error 1264 (PR #35)

- Fixed `evedbtool seed -buy-only -force` failure `Out of range value for column 'orderRange'`: the seed wrote `-1`, which the deployed MariaDB column cannot represent. New static bids use **`orderRange=0`** and both refresh-deletion and duplicate detection use that value.
- `0` denotes a system-range marker to the client; immediate execution remains restricted by **stationID** in `MarketDB::FindBuyOrder`. This is a compatibility workaround, not implementation of remote sales.
- Expanded the source regression checks and a read-only post-failure audit. Previous interrupted `-force` executions may have partially changed NPC orders before the failed insert; compare with your backup before rerunning.
- Windows/Linux/Docker CI and a real MariaDB insert smoke test must pass before merging. No database migration or player-data changes are included in this fix.

## 17 Nov 2026 — Standard NPC buy seed at 100% static market reference (pending client validation)

- Added NPC **buy** orders to standard EVEDBTool market seed using independent `SEED_NPC_BUY_100PCT` migration so existing `SEED_MARKET` databases can add bids with `seed -buy-only` without rewriting sell/player orders. Bids cover every published, market-listed Crucible inventory type in configured regions.
- New bids use exactly `invTypes.basePrice` rounded to cents (100 ISK fallback when missing/nonpositive). Existing Trader Joe bot's future buy prices likewise no longer vary randomly from 80% to 110%. A narrowly scoped one-time normalization repairs old regional-range NPC bids under `ownerID=1000125`, which the server cannot execute, to supported `4000000` at 100% reference value and adjusted non-funded NPC pseudo-escrow.
- NPC buy orders are recognizable by `ownerID=4000000`, `bid=1`, database-compatible order range `0`, duration 250 days. Full refresh is explicit via `seed -buy-only -force`; default seed is once-only/idempotent. Orders limit notional escrow to 10 trillion ISK per order and cannot draw from player wallets.
- Market settlement uses stored NPC bid price and caps a sale to the requested inventory quantity. [Read-only SQL audit](sql/diagnostics/2026-11-17-npc-buy-orders-100pct-audit.sql) and [operator test plan](docs/NPC_BUY_MARKET_100_PERCENT_2026-11-17.md) document coverage, pricing, duplicates, rollback boundaries and live client tests. CI / real MariaDB / 2012 client outcomes must be recorded before merging to master.

## 17 Nov 2026 — A01 operator acceptance, merged validation and staging cleanup

- **A01 is RESOLVED (operator reported all A01 tests passed)** on the existing-data live test. Original [PR #32](https://github.com/xrizonx/evemu_Crucible/pull/32) was already merged into `master` at `7f600be1fa06191707f4352d46f2240bfe3d2159`.
- [A01 MariaDB 11.8 fixture CI 44072201705](https://github.com/xrizonx/evemu_Crucible/actions/runs/44072201705) and [post-merge master Windows/Linux/Docker CI 44073936776](https://github.com/xrizonx/evemu_Crucible/actions/runs/44073936776) passed. Real-client test acceptance is the operator's report; no private gameplay evidence was uploaded here, and the opt-in full-server GitHub Actions job was skipped.
- Retired only the A01-specific temporary fixture, isolated Docker harness, Python unit helpers and dedicated CI workflow; removed A01-only steps from the standard build. **Preserved `src/eve-test/`, CTest registration, normal Windows/Linux/Docker CI, existing game services, production Compose, SQL migrations and data.** Git history retains all A01 implementation files for reproducibility if needed.
- [A01 archived completion record](docs/A01_STAGING_HARNESS.md) and the [living repair status](docs/REPAIR_CHECKLIST.md) preserve the sign-off and next tasks.

## 16 Nov 2026 — PR #25 runtime-confirmed station container fix and 0.8.7-Rizonx server revision

- **Operator runtime-confirmed:** assembled Giant Secure Container **140002461** (named `Test`) can be renamed, loaded with items, and emptied into the station hangar without crashing the server or the Crucible client. Stack splitting works and contents persist over server restarts.
- **SQL evidence:** eight valid-looking mineral stacks with owner **90000000** and container location **140002461**; an indexed inventory lookup uses existing `idx_entity_location_item` (`ref` access), so no duplicate index or data repair is indicated.
- Set the human-readable `EVEMU_REVISION` display to **0.8.7-Rizonx**; EVE Crucible client version **7.31**, build **360229** and MachoNet **320** remain unchanged. Windows/Linux packages label their `BUILD-INFO.txt` accordingly.
- **Still pending:** POS tower/module lifecycle, inventory edge cases and nested-container rejections, customs-office persistence, multi-character permissions, end-to-end Windows packaging, and CI verification for the new version-bump head. Full operator-checkable list: [PR #25 in-game QA acceptance checklist](docs/PR25_INGAME_VALIDATION_CHECKLIST.md).
- Do not conflate a tested in-station container fix with blanket approval of all POS/ship/corp inventory systems. The earlier source/audit notes below are historical and were written before the user confirmed the in-game fix.

---

## 16 Nov 2026 — Maurasi station assembled container transfer crash triage

The operator confirms station entry now works with locally served character portraits, but moving items into an assembled container crashes the **server**. The 209-line attached log shows sixteen identical unsymbolized stack traces during initial station load (before subsequent normal services); the actual fatal instruction cannot be established from those traces alone.

Source audit and defensive fixes on PR #25:
- Fix `Mission_Container` group mismatch in `CargoContainer::_LoadItem` that could emit `traceStack()` when this group is loaded.
- Protect `Inventory::ValidateAddItem` from null source inventory references and missing in-space ship sessions; refuse self-insertion.
- Stop ignoring failed capacity checks in `InventoryBound::MoveItems`, reject missing source items/destination inventories, and skip null/empty multi-add sort entries.
- Prevent dereference of a missing parent cache entry in `ItemFactory::GetItemContainerRef` and `GetItemContainerInventory`.
- Fix undefined uninitialized `std::pair` use and duplicate flag entries in `Inventory::AddItem`.
- Reject `InventoryBound` creation for objects without an inventory; preserve assembled hangar containers when empty instead of auto-deleting them.
- Emit `StationContainerTransfer BEGIN/END` logs around the destination capacity check and item donation; extend CI source safety checks.
- Step-by-step QA and dump guidance: [Station assembled container transfer runbook](docs/CRUCIBLE_STATION_CONTAINER_TRANSFER_CRASH_TRIAGE_2026-11-16.md).

**Caution:** These are real source defects and defensive corrections, but without a symbolized crash dump and live reproduction we cannot claim a confirmed fix for the fatal exception. No schema changes or reseeding.

---

## 16 Nov 2026 update — file-backed 50px agent portraits (PR #25 follow-up)

- The initial PR #25 embedded 322-byte 50x50 JPEG did not prevent the user's Crucible client crash. It also intercepted every NPC agent 50px request **before filesystem access**.
- Replaced that forced placeholder with a **local-file-first** lookup: `image_cache/Character/<agentID>_50.jpg` then optional `image_cache/Character/agent_fallback_50.jpg`. No external redirects or 512px substitution for NPC agent 50px requests. Valid player portraits and other image categories continue to work as before.
- Kept the explicit HTTP Content-Length fix; added the Windows JPEG generation utility `tools/prepare-crucible-agent-portrait.ps1` and [file-backed station-agent A/B test](docs/CRUCIBLE_LOCAL_AGENT_PORTRAIT_TEST_2026-11-16.md).
- This latest item **supersedes the previous embedded-322-byte test description below**. There is still no client exception dump or evidence of a successful in-game station load.

# Live Patch Notes — xrizonx/evemu_Crucible

## 2026-11-16 — Exact 50x50 baseline agent HTTP image and crash-safety audit

- Operator confirms **HTTP 404 for `Character/3011918_50` immediately before Crucible client disconnect**, despite replacing newest .exe and DLLs. A preceding 301 and an oversized embedded fallback also coincided with crashes; no actual client minidump or exception has been supplied.
- The Crucible 2012 50px NPC portrait loader now receives **200 OK with an actual 50×50 baseline JPEG**, not 301/404 or the old incorrectly sized placeholder. Deterministic neutral 322-byte image is compiled into server for NPC IDs 3000000–3999999 at size 50 only; no file-system or external CDN dependency, no real NPC likenesses reintroduced.
- Image server 200 replies now include explicit `Content-Length`, `Content-Type`, `Connection: close` and `Cache-Control: no-store`, with a connection-owned async header buffer. The older split header/body response had no content length. Other Character IDs/sizes and non-Character image fallbacks retain prior behavior.
- CTest `ImageCompatibilityTest` verifies the actual compiled JPEG byte count, baseline SOF0 50×50 dimensions, JPEG markers and tight NPC request matching.
- Agent services: prevent null bind parameter and null mission-bookmark dereferences in agent manager and bound journal.
- Inventory: guard missing container `GetMyInventory()`; `Split` reserves the new item *before* decrementing the original, compensates if decrement fails, and refuses full/singleton splits; `AlterQuantity` no longer deletes whole stacks on quantity underflow.
- Added Linux/Windows source-safety invariants for these fixes. Maintains PR #20 container assembly, PR #23 station-office fix, mission/Blockade rewards, drones, weapons, market and warp.
- Reclassified the unconditional `TCPConnection` destructor warning as trace, formatted `std::thread::id` safely instead of passing it to `%X`, preserved true thread ID before detach, and corrected pointer formatting to `%p` in the thread manager. The operator's `[ThreadWarning]` line was normal teardown logging, not proof of a client crash.
- **Important:** automated tests confirm code safety only; the original station-load client crash has not been proven fixed. Validate HTTP 200 with exactly 322 bytes and 50×50 dimensions before loading the same station; if crash continues, obtain client minidump/fault module. [Runbook](docs/CRUCIBLE_STATION_AGENT_50PX_CLIENT_CRASH_TRIAGE_2026-11-16.md).

## 2026-11-16 — Prevent modern portrait redirects during Crucible station loading

- Operator verified that even after PR #22 fully removed experimental portraits and PR #23 fixed station offices, the 2012 Crucible client requested `Character/90000000_256` and `Character/3011918_50`; the latter **still received a 301 redirect** to `images.evetech.net` immediately before logout.
- Root-cause candidate verified in source: `ImageServerConnection::ProcessHeaders` retained pre-experiment `EvEAgent::ModernImageUrl` fallback for missing NPC character portraits. Legacy clients may not support the modern external HTTPS destination; the earlier rollback did not disable this pre-existing redirect.
- Missing **Character** pictures (player or NPC) now get HTTP **404**, with `Content-Length: 0`, `Connection: close` and `Cache-Control: no-store`; no external redirect and no newly bundled agent JPEG. Existing local player/agent JPEG files continue to load if present.
- Before returning `image/jpeg`, `ImageServer::GetImage` now rejects an empty image or an obviously invalid JPEG signature, preventing mislabeled PNG files in `image_cache/Character` from being returned as JPEG. This signature check is **not** a full JPEG decoder.
- Non-Character modern fallbacks, market container assembly, station office guard repairs, agent missions, Blockade reward, drones, weapons, and warp remain unchanged.
- Extended existing `AgentHelpersTest` and added `ImageCompatibilityTest` for no character HTTPS fallback, preserved logo/icon URLs and JPEG signature checks.
- **Crucible in-client crash fix not yet confirmed.** Earlier PR #21 404 responses did not stop the crash, so another cause or client-side cached redirects/images remain plausible. Clear only cached image data (not settings) and retest with the matching build; obtain client Event Viewer exception/minidump if it persists. See [Crucible portrait response diagnostic](docs/CRUCIBLE_IMAGE_REDIRECT_CRASH_TRIAGE_2026-11-16.md).

## 2026-11-16 — Fix corporation office selection and station-load null dereferences

- Investigated continuing Crucible station-load disconnects after full removal of experimental agent portraits. Supplied server excerpt contains 16 identical unsymbolized call stacks and station/faction-warfare RPCs; the actual client crash dump is not included, so the client-side root cause remains unconfirmed.
- Corrected `StationItem::GetOfficeID`: its previous `if (cur.second.corporationID = corpID)` assignment always selected the station's first office for any player corporation. The new shared `EvEStation::OfficeIDForCorporation` helper performs a read-only ID match and returns 0 when the corporation has no office.
- Prevented `StationItem::LoadStationOffice` from dereferencing a missing office or inventory, and only marks an office as loaded after `LoadContents` actually succeeds.
- Guarded missing station references while constructing system statics and cleaning up a disconnected/docked character; a partial/failed station load no longer causes an additional null dereference during logout.
- Guarded missing cached office PyRep before returning office data to the client.
- Corrected `StationDB::GetStationOfficeIDs` for office-folder locations: `officeFolderID` is already the encoded folder ID, so subtracting `STATION_OFFICE_OFFSET` produced an incorrect SQL filter and hid office rows.
- Added `StationOfficeIntegrityTest` CTest for multiple corporations, missing offices, empty office indexes and no cross-corporation mutation. No image, market, agents, weapon, drone, warp or container-assembly logic changed.
- See [station-load failure source audit and client testing](docs/STATION_LOGIN_OFFICE_CRASH_AUDIT_2026-11-16.md). CI success does **not** imply that the live client crash has been reproduced and resolved.

## 2026-11-16 — Completely remove experimental agent image feature and restore original HTTP handler

- The 2012 Crucible client still crashes on station load even after PR #21 blocked all NPC agent JPEGs. The attached 08:11 log shows **repeated unsymbolized server-side stacks** while initializing Maurasi, then station-service RPCs and client logout; it contains no Image Server request. **No image-related root cause is proven.**
- **Fully restored** `ImageServerConnection.cpp`, `ImageServer.cpp` and `ImageServer.h` to the pre-offline-agent-portrait implementation. Removed the agent-ID-specific helper and its tests, the external portrait import scripts and the generic JPEG fallback asset. No new 404 guard or other new image response behavior remains; the original image handler's cached images and external redirects are unchanged.
- **Preserved** PR #20 cargo container assembly, the existing agent lifecycle/missions, 50 million ISK Blockade test reward, market containers, drones, weapons and warp tunnel.
- CI asserts the absence of experimental portrait code and files. This is a **full rollback**, not an assertion that the client crash is fixed; the repeat `traceStack` pattern may indicate a stale executable, as a related `DynamicEntityFactory::BuildEntity` stack-print loop was removed in commit `9a3a317068cf` on 15 November.
- See [Windows deployment and server/client crash evidence checklist](docs/AGENT_PORTRAIT_FULL_REMOVAL_2026-11-16.md).

## 2026-11-16 — Disable crash-prone station NPC portrait delivery

- Reported **immediate Crucible client crash when entering a station** after PR #20 embedded a generic agent JPEG. The client-side decoder root cause is not yet proven; this is a conservative compatibility rollback.
- Image server now returns local **404** for every NPC agent portrait ID in **3,000,000–3,999,999** *before loading a cached, custom or generic JPEG*, so stale image_cache files cannot reactivate the fault. No HTTP redirect is sent for these IDs; agent pictures intentionally remain blank.
- Restored pre-offline-portrait `ImageServer.cpp` and `ImageServer.h`, dropped installation of fallback JPEG and removed its runtime usage; player portraits and unrelated image categories retain earlier behavior.
- **Preserved PR #20 cargo container assembly**, all agent station metadata and mission lifecycle fixes, Blockade reward/market seeding, drones, weapons and warp tunnel behavior.
- Added source-level regression for the early image block and new [station-load rollback guide](docs/AGENT_STATION_PORTRAIT_ROLLBACK_2026-11-16.md); live Crucible client confirmation still required.

## 2026-11-16 — Fix container assembly and working-directory-dependent agent portrait fallback

- Implemented the existing Crucible `invbroker.AssembleCargoContainer(itemID, None, 0.0)` RPC. Previously it only logged a warning and returned without assembling. Restricts operations to personally owned station containers in historical groups **12/340/448/649**, splitting **one** item from multi-item packaged stacks, setting the singleton state and updated volume, notifying inventory and explicitly saving to DB.
- Preserved client-owned items and station checks; non-container, already assembled, corp-owned and remote items are not mutated by the new path. Existing generic inventory splitting remains nontransactional on DB failure; run on a backed-up staging server first.
- Fixed image-server warnings such as agent **3011918** by embedding the exact neutral JPEG `assets/agent_portraits/agent_missing_512.jpg` directly into the executable as the last fallback for uncached NPC agent images, rather than relying on `../share/evemu` relative to the process working directory. Local per-agent portraits and operator-supplied placeholder files still take precedence, with no HTTP redirect.
- Added `ContainerAssemblyTest` to CTest and exact-byte embedded-JPEG Python regression coverage. Gameplay verification remains pending; see [implementation and operator verification](docs/CONTAINER_ASSEMBLY_AND_AGENT_FALLBACK_FIX_2026-11-16.md).

## 2026-11-15 — Offline Crucible station-agent portraits

- Removed external HTTPS redirection for missing NPC station-agent character portraits (legacy 3,000,000–3,999,999 IDs); the local image server now serves exact or alternative cached JPEG sizes, or a bundled original generic silhouette when no historic portrait exists.
- Added a portable placeholder at `share/evemu/agent_portraits/agent_missing_512.jpg` via CMake, with a custom override under `image_cache/Character/`. The packaged fallback remains accessible even if Docker mounts an existing `image_cache` volume.
- Added opt-in `tools/agent_portraits/sync_portraits.py` to import available genuine JPEGs as local files using `sql/base/agtAgents.sql.gz` without live MariaDB. It validates responses and preserves existing cached portraits; a `--local-import ... --offline-only` mode imports an existing historical JPEG folder/ZIP entirely without network. **No network access is required from the server during agent image requests**; the importer uses the official image service only when intentionally run.
- Added offline Python cache/JPEG tests and an expanded `AgentHelpersTest` NPC portrait-ID check; Windows/Linux CI verifies the importer against the actual packaged agent SQL dump.
- Documentation: [offline portrait setup and verification](docs/AGENT_LOCAL_PORTRAITS_2026-11-15.md). Complete historical NPC portrait coverage and a 2012 client visual test are not yet proven, and downloaded copyrighted CCP assets are not automatically added to the public GitHub source.

## 2026-11-15 — Blockade test reward and cargo containers in NPC market seed

- Increased the synthetic Level 0 Blockade test mission's ISK reward from **150,000 to 50,000,000 ISK**, without altering official Crucible-era mission templates or unrelated rewards.
- Updated five hard-coded Blockade briefing, conversation and journal text occurrences. Existing Blockade offers use the already-implemented normalization on retrieval; previously completed missions are not retroactively paid.
- Added published, market-group-assigned cargo containers to EVEDBTool's regional seed and force-reseed filters: inventory groups **12 (cargo), 340 (secure), 448 (audit-log secure), 649 (freight)**. This avoids wholesale seeding of the Celestial category.
- Applied the same container allowlist to both legacy region and single-station market seed SQL scripts.
- Expanded Windows and Linux EVEDBTool dry-run CI checks to verify both container insertion and force-cleanup SQL. Added a Blockade reward assertion to `AgentHelpersTest` and [rollout/audit instructions](docs/BLOCKADE_REWARD_AND_CONTAINER_MARKET_SEED_2026-11-15.md).
- **Verification:** build and live market/client gameplay tests must be checked on this branch. Run `evedbtool seed -force -dryrun` before applying a `-force` reseed; existing NPC orders may be replaced.

## 2026-11-15 — Mission offer lifecycle and offline-item integrity

- Fixed periodic expired-offer crashes when agent lookup fails; persistent state cleanup continues without client notifications.
- Fixed shared-agent cache invalidation by matching persistent offer IDs and character IDs rather than erasing an arbitrary entry for the same agent.
- Deferred (`Allocated`) offers now expire correctly; stale previous offers no longer remove a newly issued same-agent mission.
- Completed offers are immediately archived to mission history instead of lingering in active indexes. Mission cache update/remove paths now match offer IDs.
- A failed `agtOffers` insert no longer produces a non-persistent zero-ID mission.
- Offline mission item cleanup now preserves the unconsumed stack remainder, supports 32-bit quantities and stops on SQL errors.
- Added `MissionLifecycleTest` and [dedicated source audit](docs/MISSION_EXPIRY_AND_OFFLINE_STACK_AUDIT_2026-11-15.md).
- **Verification:** new branch CI and live client/DB verification pending. Earlier successful master CI does not verify this patch.

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


## Mission completion and briefing follow-up — 2026-11-14

- Strengthened the synthetic Blockade mission's final-hostile detection: the live NPC count in the dedicated mission pocket is now a fallback when stale spawn-registry entries remain.
- When the last live NPC dies, the remaining registry rows for that mission pocket are cleared and the accepted offer's kill objective is set to its required count, making it eligible for the normal agent turn-in path.
- Added visible spawn/NPC death diagnostics so mission kill callbacks, bubble IDs, live NPC counts, registry counts, and offer progress can be checked without enabling trace-only categories.
- Added custom plain-text Blockade briefing content to the agent's offer/view responses and mission briefing response data. This avoids relying solely on the missing synthetic localization IDs.
- Build and in-game verification are pending; the user's supplied log did not include mission death callback output.

## Upstream contribution summary — 2026-11-13

This PR proposes the current fork work against `EvEmu-Project/evemu_Crucible:master`. The fork's current integration head is 183 commits ahead of the upstream merge base in GitHub's comparison at the time of the 2026-11-14 review. This count includes the fork's accumulated maintenance history and should not be interpreted as 183 newly reviewed changes in the latest mission/drone slice. This is a grouped summary of the complete proposed change set, not a claim that all gameplay features are finished.

### Mission system — implemented foundation; combat encounter work in progress

- Added encounter mission offer loading, lookup and persistence support using the existing `agtMissions` data, including briefing and reward metadata.
- Added security-agent mission selection and Level 0 progression, including a global starter security-mission tier and checks that prevent repeating completed starter templates.
- Fixed a global active-offer lock that incorrectly blocked a character from requesting missions from every other security agent while holding an offer from just one agent. Offer checks now remain scoped to the current agent, matching its per-character offer tracking. Expired offers are ignored, and the failure dialogue no longer conflates an active starter mission with missing templates.
- Added Level 1 agent access/standing display adjustments.
- Added the first playable encounter mission lifecycle: accepting an encounter creates a warpable mission beacon in a reserved pocket, associates the pocket with the character/agent/offer, and starts NPC spawning.
- Added mission-pocket kill progress reporting back to the accepted offer and mission update notification to the client.
- Added encounter-site cleanup and mission completion handling in the existing client/agent flow.
- **Latest in-progress work:** encounter offers continue to be selected from the loaded `agtMissions` security templates. The selected mission's `dungeonID` is resolved; the dungeon-template faction is preferred, with agent/region faction fallback.
- When a mission dungeon template contains room objects, the first room's NPC ship/drone type IDs and relative positions are now used to create the mission pocket's initial NPC group. Templates without usable room NPC data retain the generic faction/class spawn fallback. Ordinary belt/anomaly faction selection is unchanged.
- This is the first template-driven encounter step, not the complete EVE mission engine: later rooms, acceleration-gate progression/unlocks, trigger-based reinforcements, mission-specific non-kill objectives, and per-mission reward/loot tuning remain in progress. It is **not yet** a complete library of EVE Level 1 mission-specific templates, named wave compositions, gate unlocks, objective triggers, or per-mission reward/loot tuning. Those are follow-up implementation tasks.
- Existing NPC death, bounty, wreck and loot behaviour remains the shared game-system path; this PR does not claim a newly implemented per-template loot/bounty table or verified Level 1 reward balance.

### Space movement and warp/docking

- Added server-side pending dock and stargate approach actions so dock/jump requests can remain active while a ship approaches the required range.
- Dock and stargate jump requests selected during an active warp are now queued until the warp tunnel completes, then resumed through the normal approach/perimeter checks.
- Normalized ship heading vectors after each turn interpolation and clamped dot products before `acos`, preventing vector-length drift and invalid angle calculations during alignment.
- Added cancellation/clearing of pending space actions when a new manual movement command supersedes them.
- Corrected Crucible client warp-state serialization for player ships, NPCs and drones, including warp sector encoding and dynamic entity snapshots.
- Added diagnostics and client-visible feedback for rejected stargate jump requests.

### NPCs, drones and combat stability

- Added drone damage-state encoding and defensive defaults for missing resistance attributes.
- Improved drone scoop, abandon and ship-destruction cleanup, including controller references and bandwidth accounting.
- Corrected NPC belt spawn security-class calculation and allowed off-grid NPCs to enter warp before normal culling can remove them.
- Added defensive validation to turret hit calculations to avoid invalid entity-state and zero-distance crash paths.

### Server stability and compatibility

- Fixed mutable iterator traits in the core Buffer implementation.
- Hardened DevTools loader file handling and temporary-buffer lifetime.
- Added defensive checks to database result conversion and row text access; corrected search queries that returned too few columns.
- Fixed probe-processing iterator invalidation when a probe removes itself during processing.
- Updated the MariaDB/MySQL reconnect option to use the current boolean type.
- Added guards around station inventory loading during pod respawn and corrected station inventory reload after system unload.
- Corrected the dungeon editor AddObject binding to accept the full client argument layout and nullable transform/radius values.

### Build, packaging and documentation

- Updated CMake/build configuration, Docker build inputs and ignore rules.
- Added a Windows/Linux/Docker build-and-release GitHub Actions workflow and updated compiler/runtime compatibility fixes.
- Added SQL setup documentation and example database configuration.
- Added issue triage and stability worklog documentation.

### Verification and known limitations

- The existing CI workflow has successful Windows, Linux and Docker build history for earlier commits in this change set. The latest mission-faction commit's workflow run was still in progress when this note was prepared; check the linked Actions run before treating it as green.
- Some earlier workflow runs were cancelled as newer commits superseded them; a previous full build run succeeded, but that does not substitute for a successful build of the final PR head.
- The repository's CTest configuration previously reported no tests found. Manual in-game verification is still needed for mission acceptance, warp to beacon, NPC combat, progress updates, mission completion/rewards, NPC bounties and loot.
- No database reset is part of this work. Existing MariaDB data must be preserved. Any future mission-template schema/data migration should be additive and separately reviewed.
- No full upstream merge has been performed by this patch-note update. The change set is submitted for upstream review through the PR.

## 2026-11-13

### Fixed

- **Search result column mismatch:** Updated all ten search-category queries in `src/eve-server/search/SearchDB.cpp` to return the two columns expected by `DBResultToIntIntDict`. This addresses the reported `DBCore::GetInt: Column index 1 exceeds number of columns in row (1)` failure in those search paths.
- **Defensive database result conversion:** Added a column-count check to `DBResultToIntIntDict` in `src/eve-common/database/EVEDBUtils.cpp`, preventing reads from a missing second column.
- **Defensive database row access:** Added null/result/column bounds checks to `DBResultRow::GetText()` in `src/eve-core/database/dbcore.h`.
- **Probe processing iterator safety:** Fixed probe iteration in `src/eve-server/EntityList.cpp`. A probe can remove itself while being processed; the loop now advances safely and only erases the expected map entry. Related upstream report: [#170](https://github.com/EvEmu-Project/evemu_Crucible/issues/170).
- **MySQL reconnect option compatibility:** Replaced the legacy `my_bool` reconnect variable with `bool` and passed its address directly in `src/eve-core/database/dbcore.cpp`. Related upstream report: [#319](https://github.com/EvEmu-Project/evemu_Crucible/issues/319).

- **Turret hit-calculation crash protection:** Added validation for missing ship/pilot/target state, missing target movement data, zero distance, and invalid tracking/signature/falloff inputs in `TurretFormulas::GetToHit()`. This hardens the crash path reported in upstream [#139](https://github.com/EvEmu-Project/evemu_Crucible/issues/139). It is a defensive source fix; asteroid weapon behaviour still needs an in-game regression test.
- **NPC belt spawn class and warp-in:** Corrected the security-rating scale used by `SpawnMgr::PrepSpawn()` to match the thresholds' expected -1.0..1.0 range. NPCs spawned in empty off-grid bubbles now enter warp immediately rather than being culled before warp-in. This implements the code approach from upstream [PR #327](https://github.com/EvEmu-Project/evemu_Crucible/pull/327) and targets the disappearance report [#275](https://github.com/EvEmu-Project/evemu_Crucible/issues/275). In-game spawn/combat testing is still required.

- **Dungeon editor AddObject binding:** Corrected the server binding to accept the full nine-argument client call and nullable yaw/pitch/roll/radius values. The existing `PyRep::FloatValue()` conversion maps `None` to zero, matching the client report in upstream [#276](https://github.com/EvEmu-Project/evemu_Crucible/issues/276). The actual dungeon-editor interaction still needs an in-game test.
- **Station inventory reload after system unload:** The client-side hangar-loaded marker could remain set after a solar system unloaded its station inventory. `Inventory::LoadContents()` now skips the load only when both the client marker and the in-memory inventory-loaded state are true. This addresses the stale-cache path reported in upstream [#188](https://github.com/EvEmu-Project/evemu_Crucible/issues/188). Verify by leaving a station until its system unloads, then returning and checking the hangar contents.

- **Pod-respawn station inventory crash guard:** `Client::LoadStationHangar()` now checks the active system, station lookup and station inventory before calling `LoadContents()`. It logs the missing object and reports a client-facing error instead of dereferencing a null inventory. This hardens the crash path in upstream [#154](https://github.com/EvEmu-Project/evemu_Crucible/issues/154); the reason the station/inventory can be missing is still under investigation, so this is not yet a complete gameplay fix.

### Under test

- **Stargate jump rejection diagnostics:** The server now logs the client movement state and sends a visible notification when `StargateJump()` rejects a request because another movement/session timer is still active. This addresses the previous silent no-op path; it does not force a jump while the ship is still warping. Test the Jump button after warp completion.
- **Warp state in client ball snapshots:** Corrected the Crucible WARP sector layout and changed dynamic entity snapshots to send `WARP` (including the alignment sentinel, warp distance, minimum range and warp factor) instead of always sending `STOP`. This is a narrow adaptation of the relevant serialization part of upstream [PR #332](https://github.com/EvEmu-Project/evemu_Crucible/pull/332); the PR itself is closed unmerged and its 75-file movement rewrite was not imported. Build and in-game warp-tunnel/bubble-transition testing are still required.

### Investigating / not yet fixed

- [#154 — Pod-death crash](https://github.com/EvEmu-Project/evemu_Crucible/issues/154): null station/inventory dereferences are now guarded, but the underlying reason the clone station lookup or inventory can be missing still needs tracing and in-game respawn testing.
- [#188 — Station inventory appears empty after a system unload](https://github.com/EvEmu-Project/evemu_Crucible/issues/188): reports items remain in the database but are not reloaded into the station inventory after returning to the system.
- [#269 — Scanner crash after destroying NPCs](https://github.com/EvEmu-Project/evemu_Crucible/issues/269): needs a current stack trace and reproduction against this fork.
- [#315 — Outpost construction crashes the server](https://github.com/EvEmu-Project/evemu_Crucible/issues/315): report contains no crash log; needs a reproducible test before changing construction logic.
- [#276 — Dungeon editor acceleration gate creation](https://github.com/EvEmu-Project/evemu_Crucible/issues/276): binding signature corrected to match the nine-argument call and accept nullable values; verify in the dungeon editor.
- [PR #326](https://github.com/EvEmu-Project/evemu_Crucible/pull/326) contains a large set of unrelated gameplay changes. It should be reviewed by subsystem and not merged wholesale without testing.
- [PR #323](https://github.com/EvEmu-Project/evemu_Crucible/pull/323) proposes saving precise ship position on logout; review persistence and login warp semantics before adopting it.

### Verification

- GitHub Actions build workflow: [run #107](https://github.com/xrizonx/evemu_Crucible/actions/runs/43454803533).
- The initial run #107 above covered the first five fixes. Additional commits later on 13 November triggered newer workflow runs; use the Actions page for the latest result.
- The code changes are not yet confirmed by a full in-game regression test. The repository's current CTest configuration previously reported no tests found.
- No database migration is required for these source fixes.

## Patch-note policy

- Add an entry when a code fix is committed to this fork.
- Link upstream issues where a relationship is confirmed.
- Distinguish **fixed**, **investigating**, and **not reproduced** items.
- Record build/CI status separately from in-game verification.
- Do not describe a report as fixed solely because a defensive guard was added; verify the underlying behavior where possible.


## 2026-11-14 — Blockade encounter, mission completion and drone combat integration

### Implemented on the integration branch

- **Blockade test fleet:** Expanded the synthetic Angel Cartel encounter to 24 frigates (NPC group 550). The user has confirmed that the fleet spawns, moves and engages the player in-game.
- **Mission objective:** Added an explicit 24-kill completion target for synthetic mission ID 65535. The target is initialized after generic Encounter offer setup because the generic setup clears the cargo/objective fields.
- **Mission timer:** The synthetic Blockade test mission now receives 120 minutes from acceptance. The special duration is scoped to mission 65535; other mission timers retain their existing level-scaled calculation.
- **Reward:** Configured a 150,000 ISK mission reward and retained the normal agent mission wallet-transfer path for turn-in.
- **Offer persistence:** Mission-offer updates now persist the reward as well as the courier/objective fields. Older open synthetic offers with missing reward or a zero kill target are normalized when loaded/queried; a nonzero existing target is preserved so partial encounter progress is not overwritten.
- **Objective metadata:** Encounter kill progress is no longer incorrectly serialized as an objective type ID. The Encounter objective type metadata is sent separately from the kill-progress/target fields.
- **Completion flow:** The existing Agent Complete action is gated by the kill objective. The existing completion path records completion, pays the configured reward and removes the encounter site/offer after successful turn-in.
- **Spawn target accuracy:** Encounter offer kill targets are taken from the actual mission-pocket required-kill value rather than a generic NPC count.
- **Drone damage and AI:** Removed duplicate global damage-rate scaling from drone attack construction; improved target-lock completion, approach/engage transitions, invalid/cross-system target cleanup, target-loss handling, and client-visible target/state synchronization. Fixed null-target reset and guarded state notifications when no system bubble is available.

### Root cause of the missing objective/reward regression

The synthetic mission's special reward and kill target were being assigned before the generic Encounter initialization block. That block subsequently reset the offer's courier/objective fields, so the mission journal could retain the 120-minute expiry while losing its reward and completion target. The special values are now assigned after the generic initialization. The database update query also previously omitted the reward column; that value is now persisted. A separate client payload issue passed the kill-progress field as the objective type ID; Encounter now sends the correct neutral objective type metadata.

### Validation status — final combined head

- Integration PR: [#11](https://github.com/xrizonx/evemu_Crucible/pull/11).
- Final code head before documentation refresh: `b527d20d17d7dff570ad0dd7b001042593f20589`.
- CI run: [#43593340656](https://github.com/xrizonx/evemu_Crucible/actions/runs/43593340656).
- Docker image build: passed.
- Linux GCC build, test invocation, install and packaging: passed.
- Windows 64-bit UCRT64 GCC build: was still running when this note was prepared; do not treat the combined run as fully green until that job completes successfully.
- In-game confirmation: 24 frigate spawns, movement and combat were confirmed by the user. The 24/24 journal counter, Complete action, 150,000 ISK wallet payout and encounter cleanup still require a live turn-in test. CI/compilation does not prove these gameplay outcomes.
- No database reset or destructive migration is part of this work.

### Integration and upstream review status

The Blockade encounter, two-hour timer, reward/completion and drone combat changes are consolidated in PR #11. The earlier fork PRs #8, #9 and #10 describe overlapping slices of this same work and should be closed as superseded only after the integration PR is merged. An upstream contribution PR is a separate review/merge path; merging into this fork's `master` does not itself merge anything into `EvEmu-Project/evemu_Crucible`.


## 2026-11-14 follow-up — final Blockade objective completion

A follow-up review found that the mission pocket's final-spawn cleanup and the mission-offer progress update were coupled only through the normal per-kill credit branch. If the final NPC death arrived without a resolvable player client/killer ID, the spawn entry was removed and the pocket became empty, but the offer could retain an incomplete kill counter. The final-spawn path now independently checks whether the mission spawn registry is empty and, for the matching accepted offer, persists progress at the required target. The Complete action and journal completion status use that persisted offer state. Normal per-kill progress remains capped at the required target.

The synthetic Blockade reward/kill target assignment in `Agent::MakeOffer()` has also been moved to after generic Encounter initialization, which resets courier fields. This ensures the offered mission itself carries its objective and reward rather than relying only on the acceptance-time pocket setup.

**Verification status:** source-level path corrected; automated build validation pending for the new commit. A live 24-kill run must still confirm 24/24 journal progress, Complete action availability, successful turn-in, 150,000 ISK wallet journal credit and encounter-site cleanup. Do not mark this as gameplay-verified until that test is performed.


## 2026-11-14 follow-up — Blockade objective accounting and mixed fleet

- Changed mission-pocket kill accounting so every NPC death reported by SpawnMgr advances the mission kill objective, even when the damage source cannot be resolved to an online player Client. The encounter pocket, rather than kill-credit ownership, is authoritative for this synthetic kill-all objective.
- Kept the objective bounded by the required-kill target; the pocket's final-spawn cleanup still forces persisted progress to the actual required count when no registered hostile spawns remain.
- Replaced the 24-frigate-only test fleet with a 24-ship composition selected from the installed static-data NPC groups: 12 Angel Cartel frigates, 4 destroyers, 4 cruisers and 4 battlecruisers. Types are selected dynamically from the static data pack within the expected Angel Cartel NPC groups rather than relying on a fixed list of type IDs.
- Fleet setup now refuses to start if any required ship class group or its NPC type list is missing, instead of silently substituting another class.
- Validation status: source changes pushed to the combat-mission integration branch. A fresh Windows/Linux/Docker CI run and in-game validation are still required. Confirm the log reports all four ship classes, then verify journal progress reaches the actual spawned count, Complete becomes available, turn-in pays once, and the pocket is cleaned up.


## 2026-11-14 follow-up — turret tracking and station warp-in clearance

- Corrected player and NPC turret transversal velocity: use relative velocity crossed with normalized line of sight, so only perpendicular motion contributes to angular velocity. Previously the full relative-speed magnitude incorrectly counted radial approach/recession as transversal and could cause excessive railgun/other turret misses.
- Added guards for zero-distance/invalid NPC turret targets, missing signature/tracking data and zero falloff when outside optimal range.
- Adjusted direct station warp-in points to stop beyond the station radius plus the ship's own collision radius and a 5 km clearance. Previously the point was offset by only the station radius, placing the ship at the collision surface and allowing hulls to overlap/bounce on arrival. Normal docking perimeter/approach logic remains unchanged.
- Validation pending: compile all CI targets, then test railguns against stationary and moving large targets at optimal/falloff, and warp ships of different hull sizes to stations before attempting docking.


## 2026-11-14 follow-up — Blockade fleet hull-tier correction

- Corrected the Angel Cartel NPC inventory group mapping after confirming that group 552 is the Battleship group, not Cruiser, and group 576 is the standard Angel Cartel BattleCruiser group.
- The synthetic 24-ship Blockade fleet now selects 12 frigates from group 550, 4 destroyers from 551, 4 cruisers from 553 and 4 battlecruisers from 576. Battleship group 552 is not part of the composition.
- Added exact expected group-name validation for each configured group and for each selected type. If static data maps an ID to an unexpected group/class, fleet setup fails rather than spawning that hull.
- Validation pending: compile/CI and a fresh in-game run. Confirm the fleet contains exactly 12/4/4/4 hulls, with no battleships or larger ships.


## 2026-11-14 follow-up — Corrected Angel Cartel destroyer/cruiser group IDs

- Runtime logs showed group 551 is not the Angel Cartel Destroyer group; it resolves to Asteroid Angel Cartel Cruiser. Group 553 resolves to Asteroid Angel Cartel Officer.
- Corrected the composition to use group 575 for Angel Cartel Destroyers and group 551 for Angel Cartel Cruisers. The other classes remain group 550 Frigate and group 576 BattleCruiser.
- This excludes the Officer group (553) as well as Battleships (552). Exact expected group-name checks remain in place to prevent another incorrect mapping from spawning.
- Validation pending: CI and a new mission run; expected composition is 12 frigates, 4 destroyers, 4 cruisers and 4 battlecruisers.


## 2026-11-14 follow-up — Blockade briefing presentation and live kill counter

- Refined the synthetic Blockade agent offer and active-mission dialogue into clearly separated sections for the situation, ship-class objectives, combat progress, completion instructions, reward and time limit.
- Added a live `Kills confirmed: current / required` counter to the agent's active-mission response and mission briefing/objective payload. The displayed value is capped at the required target and uses the persisted mission-offer kill progress.
- Added explicit `killsDestroyed` and `killsRequired` objective fields for clients that consume the extended mission objective dictionary.
- Kept the existing 24-ship objective, 120-minute duration, 150,000 ISK reward and turn-in/completion flow unchanged.
- User has confirmed in-game that the mission can now be turned in after destroying all ships, the reward is issued, and mission text is visible. The presentation/counter update is source-level only at this point; compile and a fresh in-game counter refresh test are still required.


## 2026-11-14 follow-up — render mission briefing line breaks correctly

- Fixed the Blockade briefing strings to use actual C++ newline escapes rather than double-escaped backslash-n text. The agent dialog and mission journal should now render paragraphs and headings on separate lines instead of displaying literal `\\n` sequences.
- Added leading spacing before the mission title so it is visually separated from the agent division heading.
- This corrects the text serialization/presentation issue; verify in-game after rebuilding the latest branch head.

## 2026-11-14 — Blockade journal response and drone combat follow-up

- Corrected the active Blockade agent conversation path: opening the agent with an accepted synthetic mission now returns the formatted mission text and live kill count instead of the numeric briefing ID `0`.
- Suppressed the nonexistent synthetic mission title/briefing localization IDs in mission briefing and journal responses. The client now receives the custom title and briefing strings rather than being asked to resolve message ID zero.
- Kept the kill objective in the normal encounter `kill` objective payload and exposed an explicit completion flag alongside current/required kills.
- Implemented `entity.CmdEngage` for player-selected drones: validates the active system, locked ship target, drone ownership/control, online state and shared bubble, then directs each selected drone to attack.
- Improved drone pursuit to use the configured chase-distance envelope rather than dropping targets at the shorter attack-range threshold. Aggressive drones can now retaliate against NPCs that target them; passive drones remain under explicit player control.
- Validation pending: compile current branch head and test journal display/progress updates, CmdEngage with light/medium/heavy drones against NPCs, pursuit at short and long distances, passive/aggressive behavior, target death, and drone return/scoop behavior.

## 2026-11-14 — Drone combat and recovery lifecycle

- Fixed a critical drone target-acquisition failure: combat drone types that omit `maxAttackTargets` were passing zero to the NPC target manager, which rejected every engage request. The AI now guarantees at least one target slot and a nonzero lock duration.
- Added safe fallback values for missing drone movement, orbit, pursuit, weapon-range and attack-cycle attributes so incomplete legacy static data does not leave drones with zero ranges or a disabled attack timer.
- Added a pending-target path: when an engage target is outside immediate drone lock range, the drone approaches and retries target acquisition once in range instead of abandoning the command.
- Reworked drone hit chance to use transversal velocity and guard missing tracking, signature, optimal-range and falloff attributes. Combat damage now validates base drone damage and multiplier values and emits per-shot diagnostics.
- Implemented `CmdReturnHome` (return and orbit), `CmdReturnBay` (return, then automatically scoop within 2,500 m) and `CmdAbandonDrone`.
- Added safe return-to-bay completion in the drone process loop, including drone-bay capacity validation, inventory transfer, bandwidth release, target-lock cleanup and solar-system entity removal.
- Fixed launch bandwidth validation to happen before moving the item or creating the space entity. Previously an over-bandwidth launch could leave an inert drone in space after the command failed.
- Pending engage targets are stored as entity IDs and re-resolved through the current solar-system manager each tick, avoiding stale entity pointers when an NPC dies or leaves the system. A new explicit engage order now clears a different stale/locked target before acquiring the new target. Added guards for invalid target state and a valid min/max orbit-speed interval.
- Validation pending on the latest branch head: Windows/Linux/Docker CI and in-game tests for light/medium/heavy drones, target lock, approach, orbit, damage, target death, return-home, return-to-bay, bay-full failure, abandon and bandwidth rejection.

## 2026-11-15 — Drone command decoding and automatic NPC engagement

- Fixed `entity.CmdEngage` argument decoding: the client can encode item IDs as Python `PyLong` values, while the handler previously required `PyInt` for the target and rejected non-`PyInt` drone IDs. The command now accepts both integer wire types and validates the server's 32-bit entity-ID range before lookup.
- Added automatic hostile-NPC acquisition for online combat drones. Idle drones scan their current bubble, select the nearest living NPC that is not owned by the pilot or of the same non-zero faction, engage it, and reacquire the next NPC after the current target is destroyed/lost.
- Explicit return orders suppress automatic reacquisition. Relaunching/online or issuing a new engage order enables combat acquisition again.
- Validation pending: CI and in-game tests of command decoding, automatic NPC acquisition, sequential target switching, faction filtering and return suppression.


## Drone engage ownership and client-ID recovery — 2026-11-15

- Explicitly bind the launching pilot to each newly created DroneSE instead of relying only on a second global client lookup by character ID.
- Validate drone command ownership using the drone's authoritative owner character ID, controlling ship ID, home-ship pointer, enabled state, and current bubble.
- If the client-supplied selected drone IDs do not resolve to any active drone entity, recover by scanning the current system registry for drones controlled by the calling character and ship, then issue the engage order.
- Added per-drone rejection diagnostics and fallback-order logs to distinguish stale client IDs from ownership, ship, enabled-state, or bubble mismatches.
- Requires Windows/Linux CI and in-game validation; no runtime success is claimed by this patch.


## 2026-11-15 follow-up — drone combat activity state and weapon gate

- Code review found the drone AI entered client activity state `Engaged` (10) for weapon combat, even though the source enum explicitly defines `Combat` (1) as the fighting state and describes `Engaged` as non-combat. This can produce an incorrect client activity label and makes the state machine inconsistent with the protocol's combat state.
- Changed the active attack state to `Combat`, retained processing of legacy `Engaged` states, and made combat-state transitions idempotent so the drone does not repeatedly reissue orbit commands while remaining on the same target.
- Aligned the weapon-fire gate with NPC AI: a drone now fires only after TargetManager reports that target locking has completed (`CanAttack()`), while the AI continues to select a fully locked target.
- Added a trace message for the lock-wait path. The existing firing log and zero-base-damage warning remain the evidence needed to distinguish a failed lock from missing static damage attributes.
- Validation: previous PR #13 head passed Linux GCC, Windows UCRT64 GCC and Docker builds. This follow-up commit requires a fresh CI run. In-game confirmation of the Combat label and actual NPC shield/armor/hull reduction is still required; no gameplay success is claimed yet.


## 2026-11-15 follow-up — drone range, combat envelope and stale target locks

**Reported symptoms**
- Combat drones oscillated between Approach and Fighting, did not visibly damage NPCs, and sometimes travelled far from the engagement.
- Player targeting reported the maximum of eight targets despite fewer visible active locks.

**Code findings and changes**
- Automatic drone acquisition scanned every NPC in the current system bubble without checking the pilot's drone control range. It could therefore select distant NPCs in another part of the system. Automatic acquisition is now restricted to NPCs within the effective ship/character `droneControlDistance`; a 20 km fallback is used only when old character data lacks that attribute. Explicit engage commands outside control range are rejected with a notification. A valid manual order can still be pursued after the target moves away, matching normal drone behaviour.
- The AI used the inner `EntityFlyRange + MaxRange` threshold to switch between Approach and Combat even though it already cached the outer `EntityAttackRange`. This created a gap where a drone could oscillate around the threshold. Combat now starts at the outer attack envelope, and approach uses the drone's orbit distance as its follow stop range.
- TargetManager counted every stored entry toward the player target cap, but did not proactively prune entries whose target had died, left the system, or moved to another bubble. Target entries now retain their entity IDs; stale entries are pruned safely before processing/new lock requests, lock state is recalculated, and the client receives target-lost updates. Drone target IDs are also cleared on loss.
- The player target limit calculation now uses the EVE base of two plus Targeting and Multitasking skill levels, still capped by the ship's `MaxLockedTargets` attribute. Cap diagnostics report tracked, locked, and currently-locking counts separately.
- Added per-shot drone damage-component and post-`ApplyDamage` shield/armor/hull delta diagnostics. These distinguish a failure to enter the weapon loop, missing static damage values, and a failure inside the common damage application path.

**Validation status**
- This is source-level implementation only. The new branch head requires Windows, Linux and Docker CI.
- In-game validation must confirm that the combat light drone's logged thermal/kinetic/etc. base damage is nonzero and that the post-ApplyDamage delta is positive; also verify no distant auto-acquisition and that the visible target list agrees with server lock counts. No in-game damage success is claimed yet.

- Follow-up hardening: null target-map keys/entries are now removed before capacity checks as well, so malformed empty map records cannot consume one of the player's target slots.


### 2026-11-15 — Drone lock callback and target-manager stability follow-up

- Fixed target-manager processing invalidation: the global entity list now iterates a snapshot and revalidates each manager after processing, so stale-target pruning cannot invalidate the active iterator when a manager unregisters itself.
- Target-lock completion now calls the owning entity's virtual `TargetAdded` callback. Drones therefore receive the actual lock-complete event, update their client-visible target ID, and re-evaluate approach/combat distance at the correct point in the lock lifecycle.
- Restart an invulnerability timer that is still marked enabled but has already reached 0 ms. The expired timer is replaced; a timer with positive remaining time is left intact.

**Log interpretation and validation**
- The attached log's stack trace is emitted by `EvE::traceStack()` in `Client::SetInvulTimer()` after the 0 ms remaining-timer warning. The log continues afterward, so that trace alone is not proof of a fatal server crash.
- The excerpt still does not contain a symbolized fatal crash report or drone fire/damage output. Runtime drone damage and any later process termination must be checked separately after this build.


## 2026-11-15 — combat crash, drone orbit and partial flight command hardening

- Hardened target-manager teardown against stale raw entity pointers. Targeter records now retain the targeter's entity ID; incoming targeter cleanup resolves that ID through the live solar-system registry before dereferencing the pointer. Outgoing target cleanup and target-list queries also use retained IDs rather than dereferencing potentially stale keys.
- Hardened target lookup used by drone AI: `GetFirstTarget()` and `GetTarget()` now verify that the retained ID still resolves to the same live entity before returning a pointer.
- Added null/dependency checks to target registration and lock completion. The entity death path now relies on `TargetManager::Destroyed()` / `ClearAllTargets()` to clear both outgoing and incoming target references before the manager is deleted.
- Changed drone combat orbit selection to use `AttrOrbitRange`, not `AttrMaxRange`. If old static data omits `orbitRange`, the weapon-range/falloff fallback is capped at 2,500 m (500 m if no useful value exists), preventing a missing orbit attribute from turning a weapon range into a very large idle/engagement orbit.
- Drone home-ship access now resolves the controller ID against the live system registry. Return, idle orbit, control-range checks and scoop no longer blindly trust a potentially stale non-owning ship pointer.
- Fixed partial `CmdEngage` selection recovery: if only some client-supplied drone IDs resolve, the handler now finds and commands the remaining eligible drones controlled by the current character and ship, without issuing duplicate orders to drones already handled.
- Validation: current head requires fresh Windows/Linux/Docker CI and in-game tests. The supplied log does not include a fatal crash stack; the turn-percent diagnostic is clamped in `DestinyManager::Turn()`, and the FX ID 0 warning is a missing weapon effect mapping rather than evidence of a server crash.

## 2026-11-15 — drone focus-fire and combat effect safety follow-up

- Drone auto-acquisition now respects the ship's Aggressive setting. Passive drones no longer independently acquire and attack nearby NPCs without an explicit engage order.
- When Focus Fire is enabled, drones controlled by the same ship share the first valid assigned target. The target assignment is recorded before lock completion, so other drones can join while the first drone is still locking.
- Drone attack processing now resolves the assigned target ID specifically instead of taking whichever locked target happens to be first in the target-manager pointer map. This avoids firing at an unintended target when stale or multiple target entries exist.
- Hardened FxProc::ApplyEffects() against missing source item references and missing fitted modules. A charge whose inventory flag is flagNone is skipped as an invalid/unfitted charge modifier; the processor no longer emits a full stack trace for every such data condition.
- The supplied 934-line log ends at 12:17:13 with no fatal exception, access violation, process-exit record or crash dump. The stack traces adjacent to FxProc::ApplyEffects() are generated by EvE::traceStack() in the flagNone diagnostic, not proof of the fatal crash site. The log also shows repeated destiny turn diagnostics, missing weapon effect mappings and unload attempts for unloaded railguns; these are separate issues and are not sufficient to identify the crash root cause.
- Runtime validation remains required. The log does not contain a drone firing/damage trace, nor a symbolized fatal crash stack. CI and in-game testing are required before calling the crash fixed.