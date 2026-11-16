# PR #25 — in-game QA acceptance checklist (0.8.7-Rizonx)

**Project:** EVEmu Crucible 1.6.5, client build 360229  
**PR:** https://github.com/xrizonx/evemu_Crucible/pull/25  
**Updated:** 2026-11-16  
**Status: MERGE-READY CANDIDATE.** Assembled-container workflows and all B1–B9 station/ship/container checks were reported working by the operator. Remaining POS, customs-office and broader manual QA are **deferred, non-blocking post-merge verification** by operator decision (16 Nov 2026). These untested items are NOT marked as passed.

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

## B. Operator-reported working — station/ship/container inventory (9/9)

**Operator update (16 November 2026):** All nine B-series checks marked working in-game. This is an operator-reported **section-level pass**; item-by-item logs, build SHA and boundary-value evidence were not independently provided. Record a more detailed test report if one of these flows later regresses.

- [x] **B1.** Remove **the final item** from an assembled station container.
  It must remain in the hangar when empty, with its original item ID.
- [x] **B2.** Merge compatible mineral stacks inside a container, including a
  partial merge; sums must be conserved and no item duplicated/lost.
- [x] **B3.** Transfer **partial stacks** station → container → station →
  ship cargo hold (Ferox example: **140000640**). Verify quantities,
  ownership, location flags and persistence after restart.
- [x] **B4.** Move assembled containers between station hangar and ship cargo
  where allowed by capacity; rejected moves must leave both items intact.
- [x] **B5.** Attempt to put a container **inside itself**, then try creating
  a two- or three-level container cycle. The server must reject the move,
  preserve the original contents, and remain responsive.
- [x] **B6.** Test a full-capacity container, oversized stack and invalid
  quantity/malformed move request using disposable items. No crash or
  duplicated/lost stacks. (Malformed RPC tests may require a test harness.)
- [x] **B7.** Merge two extremely large test stacks near the `int32`
  quantity ceiling only on a disposable test character/DB. Overflow must
  be refused without deleting or decrementing the source.
- [x] **B8.** Exercise jetcan, anchored secure-container and wreck loot:
  remove some/all items, close/reopen inventory, and check expected
  in-space lifetime, cleanup and item ownership after restarting.
- [x] **B9.** Confirm split/merge/move between inventories after a previous
  validation error; no stale-client context, wrong-client capacity check,
  crash or permission leakage.

## Merge decision and risk acceptance — 16 November 2026

- **Accepted for merge:** A1–A8 and B1–B9: 17 of 37 items supported by operator runtime reports and supplied SQL findings.
- **Manual QA waived as a merge prerequisite:** C1–C9 (9 POS tests), D1–D3 (3 customs tests), and E1–E3/E5–E8 (7 database/operational tests) are **19 deferred items**, not passed.
- **Automated CI remains required:** E4 covers Windows UCRT64, Linux GCC, Docker and registered safety/CTest checks. On the pre-documentation head `047221efc3f78dc5cf150cfc0e94422f898c3ef1`, [run #43859501069](https://github.com/xrizonx/evemu_Crucible/actions/runs/43859501069) succeeded. Confirm the **final** head's CI before merging.
- **GitHub PR state at preparation:** PR #25 was reported **clean/mergeable**, not a draft, with no unresolved recorded reviews. Repository branch protection/ruleset permissions were not available to this integration; GitHub's actual merge button remains authoritative.
- **Known risk:** POS tower/module initialization, corporate permissions, customs-office persistence and multi-character regressions have not been demonstrated in-game. These paths may have bugs after merging. Keep a working database backup and preserve a rollback SHA.
- No untested item is represented as working. This is a decision to **defer verification**, not remove tests, turn off CI, or waive repository protection rules.

### Required before merge (automated, not manual gameplay testing)

Verify the latest PR-head GitHub Actions run passes for Windows, Linux and Docker. Do not merge a failing build solely to bypass manual checks.

## C. Deferred post-merge (non-blocking) — POS/starbase lifecycle

- **C1.** Anchor a control tower at a valid moon and confirm state
  updates, slim-item notifications, position and saved `posStructureData`.
- **C2.** Online and offline a tower with a test force field/password;
  check shield lifecycle, no stale shield entity and no crash.
- **C3.** Anchor, online, offline, unanchor and scoop a supported POS
  module; verify control tower links and state persistence on restart.
- **C4.** Add/remove disposable items to/from the tower fuel bay or
  corporate hangar array; verify proper flags, capacity and ownership.
- **C5.** Attempt capacity overflow and a move without required corporate
  permissions. Both must reject without mutating owner, location or quantity.
- **C6.** Test expected rejection of POS actions with missing tower,
  moon, gate, planet or sovereignty hub using a disposable test DB; check
  clear server logs and no null-pointer crash.
- **C7.** Test supported jump bridge, reactor and moon-miner initialization,
  especially after reboot, for base/derived `InitData` consistency.
- **C8.** Retry previously problematic POS RPCs (tower notifications,
  password, sentry settings, permissions, and anchoring) when the target
  entity is absent or unloading. Expect safe rejection without a crash.
- **C9.** Verify that repeated POS initialization does not create
  duplicate `posStructureData`/`posTowerData`/`posJumpBridgeData` rows.

## D. Deferred post-merge (non-blocking) — customs offices / orbital cargo

- **D1.** Move cargo into and out of a supported customs office; verify
  `entity.locationID` matches the actual destination, including after restart.
- **D2.** Reject oversized customs-office transfers without moving items
  or changing their owner/flag.
- **D3.** Check relevant planetary export/import or launch containers:
  claim/remove contents and verify intended cleanup without deleting
  unrelated cargo; skip unsupported client mechanics with a recorded note.

## E. Deferred post-merge (non-blocking) — database, regression and release checks

- **E1.** Run the **read-only** SQL integrity audit against a database
  backup: `sql/diagnostics/2026-11-16-container-pos-integrity.sql`.
  Investigate any returned containment cycles, suspect parent rows, orphan
  POS data or stale attributes. Do not auto-delete rows.
- **E2.** Confirm rename, inventory move and POS operations with a
  second character and a separate corporation where available. Check
  visibility, ownership and role restrictions.
- **E3.** Station services regressions: docking/undocking, agent/journal,
  NPC 50px portrait request and market/inventory windows; no client disconnect
  or server crash. The earlier station-entry pass does not cover every RPC.
- **E4.** Verify the **final PR-head** GitHub Actions run completes:
  Windows UCRT64, Linux GCC, Docker, safety source checks and CTest.
  An older green SHA does not count for the final changes.
- **E5.** Install the final Windows ZIP with the matching bundled UCRT64
  DLLs. Verify `BUILD-INFO.txt` commit SHA matches the installed executable.
- **E6.** Start the new server and confirm its startup banner shows
  **Server Revision: 0.8.7-Rizonx**. Supported client stays **Crucible
  1.6.5 / 7.31 / 360229 / MachoNet 320**.
- **E7.** No inventory changes were made through an unreviewed or
  destructive SQL migration. Record the DB backup and restore point used.
- **E8.** Review logs after repeated action cycles and one restart for
  `POS__ERROR`, `INV__ERROR`, exceptions or missing `StationContainerTransfer
  END` markers. Record any persistent errors with item IDs and a crash
  stack/minidump if applicable.

## Sign-off criteria / test evidence

**Merge-ready with deferred manual QA** by operator decision. 17 tests are
reported complete, 19 manual tests are deferred to post-merge, and E4 is the
automated final-head CI gate. POS/customs-office functional verification is
not a required pre-merge condition for this PR. Keep deferred tests in the
backlog, and require new runtime evidence before declaring those paths
validated in release notes.

For each remaining test, record: date and client/server binary SHA, character
and item IDs, before/after SQL location/owner/quantity, what was clicked,
observed result, and a short log excerpt if a rejection or crash occurred.
Do not post credentials or private database dumps.

**Source version note:** `EVEMU_REVISION` is a server display version.
Changing it to `0.8.7-Rizonx` does not alter EVE client network protocol
values or imply that the game supports a newer CCP client.
