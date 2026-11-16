# PR #25 — in-game QA acceptance checklist (0.8.7-Rizonx)

**Project:** EVEmu Crucible 1.6.5, client build 360229  
**PR:** https://github.com/xrizonx/evemu_Crucible/pull/25  
**Updated:** 2026-11-16  
**Status:** Assembled station-container crash fixed and operator-tested. The rest of
this PR has not been exhaustively verified in-game.

Only check a box after running it with the final PR-head server binary and
recording the result. A green GitHub Action is **not** an in-game pass.

## A. Already verified by operator — station assembled container

- [x] **A1.** Enter Maurasi station with the Crucible 1.6.5 client without
  the previously reported server/client crash.
- [x] **A2.** Rename an assembled **Giant Secure Container** in the station
  without a server/client crash. Test item **140002461**, named `Test`.
- [x] **A3.** Move mineral items **into** the assembled station container;
  the client and server stay running.
- [x] **A4.** Move items **out** of the container and back to the station
  hangar; the client and server stay running.
- [x] **A5.** Split an item stack successfully.
- [x] **A6.** Restart the server; verify container contents and item quantities
  persist. Operator confirms persistence after server reboots.
- [x] **A7.** SQL child-item inspection shows eight valid-looking mineral
  stacks, owner **90000000**, location **140002461**, positive quantities,
  `singleton=0`, and `flag=0`. No malformed rows among the eight.
- [x] **A8.** `EXPLAIN SELECT itemID FROM entity WHERE locationID=140000640
  ORDER BY itemID` uses `idx_entity_location_item` (access `ref`,
  `Using where; Using index`). No second equivalent index needed.

**Evidence note:** A1–A6 are direct operator-reported runtime checks; A7–A8
are operator-reported SQL query results. These tests verify that particular
workflows, not all possible container/POS paths, succeed.

## B. Outstanding station/ship/container inventory testing

- [ ] **B1.** Remove **the final item** from an assembled station container.
  It must remain in the hangar when empty, with its original item ID.
- [ ] **B2.** Merge compatible mineral stacks inside a container, including a
  partial merge; sums must be conserved and no item duplicated/lost.
- [ ] **B3.** Transfer **partial stacks** station → container → station →
  ship cargo hold (Ferox example: **140000640**). Verify quantities,
  ownership, location flags and persistence after restart.
- [ ] **B4.** Move assembled containers between station hangar and ship cargo
  where allowed by capacity; rejected moves must leave both items intact.
- [ ] **B5.** Attempt to put a container **inside itself**, then try creating
  a two- or three-level container cycle. The server must reject the move,
  preserve the original contents, and remain responsive.
- [ ] **B6.** Test a full-capacity container, oversized stack and invalid
  quantity/malformed move request using disposable items. No crash or
  duplicated/lost stacks. (Malformed RPC tests may require a test harness.)
- [ ] **B7.** Merge two extremely large test stacks near the `int32`
  quantity ceiling only on a disposable test character/DB. Overflow must
  be refused without deleting or decrementing the source.
- [ ] **B8.** Exercise jetcan, anchored secure-container and wreck loot:
  remove some/all items, close/reopen inventory, and check expected
  in-space lifetime, cleanup and item ownership after restarting.
- [ ] **B9.** Confirm split/merge/move between inventories after a previous
  validation error; no stale-client context, wrong-client capacity check,
  crash or permission leakage.

## C. Outstanding POS/starbase lifecycle

- [ ] **C1.** Anchor a control tower at a valid moon and confirm state
  updates, slim-item notifications, position and saved `posStructureData`.
- [ ] **C2.** Online and offline a tower with a test force field/password;
  check shield lifecycle, no stale shield entity and no crash.
- [ ] **C3.** Anchor, online, offline, unanchor and scoop a supported POS
  module; verify control tower links and state persistence on restart.
- [ ] **C4.** Add/remove disposable items to/from the tower fuel bay or
  corporate hangar array; verify proper flags, capacity and ownership.
- [ ] **C5.** Attempt capacity overflow and a move without required corporate
  permissions. Both must reject without mutating owner, location or quantity.
- [ ] **C6.** Test expected rejection of POS actions with missing tower,
  moon, gate, planet or sovereignty hub using a disposable test DB; check
  clear server logs and no null-pointer crash.
- [ ] **C7.** Test supported jump bridge, reactor and moon-miner initialization,
  especially after reboot, for base/derived `InitData` consistency.
- [ ] **C8.** Retry previously problematic POS RPCs (tower notifications,
  password, sentry settings, permissions, and anchoring) when the target
  entity is absent or unloading. Expect safe rejection without a crash.
- [ ] **C9.** Verify that repeated POS initialization does not create
  duplicate `posStructureData`/`posTowerData`/`posJumpBridgeData` rows.

## D. Outstanding customs offices / orbital cargo

- [ ] **D1.** Move cargo into and out of a supported customs office; verify
  `entity.locationID` matches the actual destination, including after restart.
- [ ] **D2.** Reject oversized customs-office transfers without moving items
  or changing their owner/flag.
- [ ] **D3.** Check relevant planetary export/import or launch containers:
  claim/remove contents and verify intended cleanup without deleting
  unrelated cargo; skip unsupported client mechanics with a recorded note.

## E. Outstanding database, regression and release checks

- [ ] **E1.** Run the **read-only** SQL integrity audit against a database
  backup: `sql/diagnostics/2026-11-16-container-pos-integrity.sql`.
  Investigate any returned containment cycles, suspect parent rows, orphan
  POS data or stale attributes. Do not auto-delete rows.
- [ ] **E2.** Confirm rename, inventory move and POS operations with a
  second character and a separate corporation where available. Check
  visibility, ownership and role restrictions.
- [ ] **E3.** Station services regressions: docking/undocking, agent/journal,
  NPC 50px portrait request and market/inventory windows; no client disconnect
  or server crash. The earlier station-entry pass does not cover every RPC.
- [ ] **E4.** Verify the **final PR-head** GitHub Actions run completes:
  Windows UCRT64, Linux GCC, Docker, safety source checks and CTest.
  An older green SHA does not count for the final changes.
- [ ] **E5.** Install the final Windows ZIP with the matching bundled UCRT64
  DLLs. Verify `BUILD-INFO.txt` commit SHA matches the installed executable.
- [ ] **E6.** Start the new server and confirm its startup banner shows
  **Server Revision: 0.8.7-Rizonx**. Supported client stays **Crucible
  1.6.5 / 7.31 / 360229 / MachoNet 320**.
- [ ] **E7.** No inventory changes were made through an unreviewed or
  destructive SQL migration. Record the DB backup and restore point used.
- [ ] **E8.** Review logs after repeated action cycles and one restart for
  `POS__ERROR`, `INV__ERROR`, exceptions or missing `StationContainerTransfer
  END` markers. Record any persistent errors with item IDs and a crash
  stack/minidump if applicable.

## Sign-off criteria / test evidence

PR #25 is **not ready for full POS/container QA sign-off** until B–E are
resolved or explicitly marked *unsupported/out of scope* in the PR review.
The original assembled station-container failure can be called
**runtime-confirmed fixed** based on A1–A6.

For each remaining test, record: date and client/server binary SHA, character
and item IDs, before/after SQL location/owner/quantity, what was clicked,
observed result, and a short log excerpt if a rejection or crash occurred.
Do not post credentials or private database dumps.

**Source version note:** `EVEMU_REVISION` is a server display version.
Changing it to `0.8.7-Rizonx` does not alter EVE client network protocol
values or imply that the game supports a newer CCP client.
