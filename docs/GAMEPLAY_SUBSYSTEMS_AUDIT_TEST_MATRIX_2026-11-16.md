# EVEmu Crucible — gameplay subsystem source audit and in-game validation

**Date:** 16 November 2026  
**Baseline:** `fix/security-sovereignty-belt-rat-spawns-2026-11-16` (PR #26)  
**Changes:** `audit/agents-drones-combat-mail-contracts-industry-2026-11-16`  
**Audience:** private EVEmu Crucible 1.6.5 server operator.

This is a focused source-level audit of the requested subsystems, **not a
claim that every file in the repository has been exhaustively audited**.
Code fixes require CI compilation and independent in-game verification. Never
treat an unchecked game test as passed. This branch does not alter `master`
or merge PR #26.

## Findings and applied source fixes

### Agents / missions

**Fixed:** `AgentBound::GetMissionObjectiveInfo` loaded the current
`MissionOffer` only for zero-argument requests. With optional characterID
and contentID arguments, it could return empty/default mission objectives.
Both forms now load the caller's offer and reject another character's ID.

**Not changed / not verified:** Level 0 starter missions, Level 1–5
standing calculations, courier cargo spawning/delivery, kill-objective
updates, mission item rewards, persistent dungeon respawn and LP payouts.

### Mining drones

**Outstanding feature (NOT fixed):** `DroneAIMgr::Target` explicitly
ignores mining drones; `DroneAIMgr::Process` has no mining extraction
state. Combat drone target selection and AI must not be confused with an
ore-mining implementation. This audit **does not invent ore, award fake
yields or pretend drones mine**. A safe implementation needs mining target
orders, drone harvest volume/interval, approach, return to ship, cargo
capacity, valid asteroid depletion shared with player mining lasers,
persistence, safe cancellation and tests. Track separately.

**Related mining-laser risk:** The current `MiningLaser::ProcessCycle`
modifies rock quantity before some cargo-transfer failure paths and uses a
separate depletion-distribution callback. Do not refactor this in a broad
combat PR without a dedicated no-duplication test; investigate separately.

### Combat targeting and tracking

**Fixed:** `TargetManager::TimeToLock` now rejects missing source/target
and avoids dividing by zero when scan resolution is absent.
`TargetManager::RemoveTarget` no longer dereferences a potentially
deleted pointer solely to log a name. Targeter event/update loops now
re-resolve live entities by ID before delivering notifications.
Asteroid depletion now handles non-mining targeted modules and null
module entries, rather than treating every module as a `MiningLaser`.

**Existing, preserved:** Player/NPC/drone turret hit chance already uses
angular/transversal velocity and range/falloff. These formulas were not
arbitrarily rebalanced in this audit.

### Tractor beam

**Fixed:** Missing alliance or faction membership uses ID 0. Treating
two zeros as equal previously granted unintended tractor permissions.
Fleet/corporation/alliance/faction matches now require nonzero identity.
`TractorBeamStart` now occurs **after**, not before, the common
effective-range check. Null source/target movement-manager guards were
added, including tractor shutdown.

### Mail

**Fixed:** Recipient-specific read/unread, label and trash state belongs
to `mailStatus`, not `mailMessage`. Recipient IDs now scope batch updates,
delete and the mail-body RPC. Empty/invalid label masks are rejected;
mailing-list key names no longer include leading spaces or malformed
`util.KeyVal` type names. Mail title, label and mailing-list name inputs
are SQL-escaped. A failed `mailMessage` INSERT no longer attempts
`mailStatus` INSERT using an invalid message ID.

**Not implemented:** Most mailing-list membership and administration
RPCs remain stubs; corp/alliance bulk mail is incomplete. No claim of
end-to-end mail delivery, attachment support, or spam controls.

### Contracts

**Fixed:** Several `AcceptContract` SQL queries passed the
`PyInt* contractID` pointer rather than `contractID->value()`.
`CreateContract` now validates required named arguments, item IDs,
positive item quantities, nonnegative price/reward/collateral and at
least one item; checks courier plastic-wrap creation; SQL-escapes
title/description and validates response row exists.

**Additional permissions fixes:** A contract's delete operation previously
could reassign escrowed items to any character that supplied its contract ID.
The handler now restricts deletion to the issuing character while status=0,
and restricts the SQL status update by issuer. Private contracts are checked
against their designated character/corporation before acceptance.
`CompleteContract` now uses the numeric contract ID and requires the
contract be an **active courier** (type=3, status=1) whose acceptor is the
calling character. This rejects straightforward unauthorised and duplicate
completion attempts, though it does not provide transactional isolation.

**Major remaining risk:** Contract creation debits courier rewards and
mutates item ownership before all database writes complete, **without
a transactional rollback**. Contract acceptance also involves a
multi-operation ISK/item transfer without transaction isolation.
Do not use high-value contracts until transactional escrow and
failure recovery are implemented and tested. The present fixes harden
common errors; they do **not** make contract escrow fully safe.

### Industry / research

**Fixed:** `RamProxyService::CompleteJob` no longer marks an
industry job finished before checking the installed blueprint exists;
checks the DB completion result; skips a zero calculated material return
instead of manufacturing one free unit. Invention installation is now
explicitly rejected because its completion code reported guaranteed
success with `outputTypeID=0` and `outputItemID=0`.

**Not implemented:** Historical invention success, decryptor/skill
modifiers, reverse engineering, POS manufacturing arrays, some
corporation job paths. Existing invention jobs created by an earlier
build need dedicated backup-based recovery; do not manually deliver
invented items from unverified records.

## Compatibility and deployment notes

- No destructive SQL migrations or static-data replacements.
- This is a **stacked code branch** based on the sovereignty/belt branch
  (PR #26). **PR #28 temporarily targets `master` solely because GitHub
  Actions only runs pull-request workflows with `base: master`.**
  Consequently its diff currently includes both PR #26 and its own fixes.
  PR #26 does not yet contain merged PR #27's database and corporation-tax
  changes from `master`. Before a production deployment or PR #28 merge,
  land or reconcile PR #26, incorporate current `master` (including PR #27),
  and rerun Windows/Linux/Docker CI.
- Source-invariant checks: `tools/verify_subsystems_audit.py`, wired
  into Linux and Windows CI.
- No direct game-client tests have been run by the assistant. Record
  Windows package SHA from `BUILD-INFO.txt` for each test session.

## In-game test checklist (all unverified until performed)

### Agents and missions
- [ ] A1. Open mission briefing/journal with zero and two optional RPC arguments.
- [ ] A2. Verify same agent's mission objectives, location, reward, bookmark
      and progress are identical through both RPC variants.
- [ ] A3. Another character ID must not reveal the first character's
      mission objectives.
- [ ] A4. Accept, complete and decline one starter/security mission;
      verify objectives and NPC pocket cleanup.
- [ ] A5. Accept/complete a courier mission; verify cargo, ISK/LP rewards
      and journal after server restart.
- [ ] A6. Check agent standings thresholds, negative standings, faction,
      personal and corporation standings, and expired offers.

### Mining drones, mining lasers and belt depletion
- [ ] D1. Drone attack, mining and return-to-bay commands must not crash
      or falsely damage an asteroid.
- [ ] D2. **BLOCKED/UNIMPLEMENTED:** mining drone extraction cycle,
      actual ore delivery, capacity checks, asteroid depletion and
      concurrent mining laser ownership.
- [ ] D3. Mining lasers on the same rock should conserve ore when the
      final cycle depletes it; avoid duplicate/vanished yield.
- [ ] D4. Mining crystals, ice, gas and insufficient cargo space behave
      correctly after reconnect and server restart.

### Combat, tracking and tractor beams
- [ ] T1. Lock/lose targets rapidly while docking, warping, cloaking or
      destroying NPCs; no server crash or stale lock.
- [ ] T2. Compare stationary target vs radial travel vs high transverse
      velocity for turret accuracy; no reversed transversal penalty.
- [ ] T3. Lock with missing/zero scan-resolution test attributes;
      server must not calculate NaN/Infinity or hang.
- [ ] T4. Deplete an asteroid while a survey, tractor and mining module
      target it; all modules safely deactivate.
- [ ] T5. Tractor own wreck and permitted fleet/corporation wreck
      within range; inspect movement and beam start/stop.
- [ ] T6. Reject unrelated wreck with both players' alliance/faction
      IDs at zero; reject anchored can and out-of-range tractor action.
- [ ] T7. Combat drones maintain/clear locks on destroyed or warped
      targets; player turret firing remains stable.

### Mail and mailing lists
- [ ] M1. Send/receive mail with apostrophes, Unicode title and body,
      percent symbols and two recipients; restart and read.
- [ ] M2. Each recipient has independent read/unread state and labels.
- [ ] M3. Move one/all mail messages into/out of trash, empty trash
      and verify the other recipient is unaffected.
- [ ] M4. A recipient cannot read, delete or relabel an unowned messageID.
- [ ] M5. Create/edit/delete a label with special characters; validate
      invalid zero/multi-bit label requests do nothing.
- [ ] M6. Mailing list join/leave/admin and corp/alliance mail remain
      **unsupported** unless separately implemented and tested.

### Contracts and escrow
- [ ] C1. Search/browse contracts, create a low-value item exchange and
      reopen after server restart.
- [ ] C2. Verify accepted contract's `contractID` and status update,
      correct owner, item location, money transfer and receipt.
- [ ] C3. Reject missing named arguments, negative ISK and invalid
      quantity without charging funds or creating a contract.
- [ ] C4. Contract title and description containing apostrophes/percent
      signs persist intact.
- [ ] C5. Test courier plastic wrap and acceptance only with disposable
      items and minimal ISK.
- [ ] C6. **BLOCKED:** high-value contracts and concurrent accept/cancel
      until atomic escrow, rollback and balance/ownership checks exist.
- [ ] C7. Attempt to delete another character's outstanding contract ID:
      expect rejection and no ownership change.
- [ ] C8. Attempt to accept a private contract as the wrong character
      and unrelated corporation: expect rejection.
- [ ] C9. Attempt complete courier as a non-acceptor, then repeat after
      completion: no duplicated reward/ISK or delivered cargo.
- [ ] C10. Delete own outstanding contract, verify item owner/location
      restored and no unrelated item affected; no high-value assets.

### Industry
- [ ] I1. Manufacture a single-run blueprint, deliver once and confirm
      output, item owner/location, input material consumption and job status.
- [ ] I2. Material-efficiency and time-efficiency research updates survive
      a server restart.
- [ ] I3. Copy a blueprint, verify BPC run counts and stored ME/PE.
- [ ] I4. Cancel and deliver a job; zero returned material must produce
      exactly zero items, no duplicate delivery.
- [ ] I5. Missing blueprint and duplicate completion must not create
      output or change job state unexpectedly.
- [ ] I6. Invention attempts must be rejected safely, not claim a
      guaranteed success with typeID zero.
- [ ] I7. **BLOCKED:** POS arrays, invention, reverse engineering and
      unverified corporation job permissions.

### CI / database and rollback
- [ ] R1. Windows x86_64 UCRT64 GCC completes; `eve-test` CTest passes.
- [ ] R2. Linux GCC build, CTest, and Docker build all pass.
- [ ] R3. `verify_subsystems_audit.py` passes in Windows and Linux.
- [ ] R4. Record DB backup and final-head SHA; repeat a focused
      acceptance smoke test after rebase onto `master`.
- [ ] R5. Review mail/contract/ram row-count and ownership changes with
      read-only SQL; never use a generated `INSERT INTO UnknownTable`
      export as database repair statements.

**Release gate:** Compile green is necessary but not sufficient.
Do not merge or deploy high-risk mail/contract/industry changes to the
production server without a backed-up database and acceptance tests.
