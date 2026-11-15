# Upstream Issue Triage — xrizonx/evemu_Crucible

Last reviewed: 2026-11-13  
Working branch: `master`  
Source tracker: [EvEmu-Project/evemu_Crucible/issues](https://github.com/EvEmu-Project/evemu_Crucible/issues)

## 2026-11-14 — Blockade mission completion / text

- **Open / needs runtime verification:** the user reports the mission remains incomplete after clearing all hostiles. The mission kill handler previously considered the objective complete only when the per-bubble spawn registry became empty. A live-NPC-count fallback now detects the final death and clears stale registry rows for the dedicated mission pocket.
- **Open / needs runtime verification:** the synthetic mission used `briefingID = 0` and a synthetic title ID, leaving the client without localized offer text. Added explicit plain-text briefing and objective descriptions to the agent offer/view and briefing response paths.
- Added visible NPC-death and spawn-manager diagnostic messages. Use the next runtime log to confirm the final NPC callback, bubble count, and persisted offer progress.
- CI compilation and a fresh in-game test are still required; do not mark this fixed until the journal reaches 24/24 and the agent offers Complete.

## Scope

The upstream tracker currently exposes **128 open issue/PR records** across two API pages (100 + 28). The API's issue listing also includes pull requests. This is an inventory of the open records, not a claim that all 128 have received a full source-level audit. The items below are the subset inspected in detail or selected for follow-up.

## Implemented on this fork

| Upstream item | Area | Fork status | Verification still needed |
|---|---|---|---|
| [#139](https://github.com/EvEmu-Project/evemu_Crucible/issues/139) | Turret fire at asteroids can crash | Added null/state/numeric guards to `TurretFormulas::GetToHit()` | In-game asteroid targeting and turret fire |
| [#170](https://github.com/EvEmu-Project/evemu_Crucible/issues/170) | Probe processing iterator invalidation | Fixed safe iteration/removal in `EntityList::Process()` | Repeated probe recovery/removal in-game |
| [#154](https://github.com/EvEmu-Project/evemu_Crucible/issues/154) | Pod-death crash | Added null checks and a client-facing error path for missing station/inventory objects; root cause remains under investigation | Reproduce clone respawn and confirm the server remains stable |
| [#188](https://github.com/EvEmu-Project/evemu_Crucible/issues/188) | Station inventory appears empty after system unload | Reloads inventory when the client marker is stale but the in-memory inventory has been unloaded | Test system unload/return and verify DB-backed hangar contents |
| [#275](https://github.com/EvEmu-Project/evemu_Crucible/issues/275) | NPC belt spawns disappear before warp-in | Implemented the two-part spawn fix from [PR #327](https://github.com/EvEmu-Project/evemu_Crucible/pull/327): security-rating scale correction and immediate warp for NPCs in empty bubbles | Spawn belts across security classes; observe NPC warp-in and combat |
| [#276](https://github.com/EvEmu-Project/evemu_Crucible/issues/276) | Dungeon editor acceleration gate creation | Corrected `AddObject` binding to accept the full nine arguments and nullable optional values | Test adding acceleration gates in the dungeon editor |
| [#319](https://github.com/EvEmu-Project/evemu_Crucible/issues/319) | MySQL reconnect option build compatibility | Replaced legacy `my_bool` usage with `bool` | Confirm all CI platforms pass |

The search-result projection mismatch, defensive DB result conversion, and DB row text bounds checks are additional fork fixes documented in [PATCH_NOTES.md](PATCH_NOTES.md).

## Priority investigation queue

| Priority | Upstream item | Finding so far | Next engineering step |
|---|---|---|---|
| High | [#154](https://github.com/EvEmu-Project/evemu_Crucible/issues/154) Pod-death crash | Historical stack trace reaches `Inventory::LoadContents(this=0x0)` from `Client::LoadStationHangar()`. Added null checks and a client-facing error path; underlying station/inventory lookup failure still needs tracing. | Reproduce clone respawn to the medical station and trace why the station or inventory lookup can fail; verify the guard prevents the crash. |
| High | [#269](https://github.com/EvEmu-Project/evemu_Crucible/issues/269) Scanner crash after NPC kills | Reproduction sequence is specific, but no stack trace is supplied. | Reproduce with wrecks and multiple NPC kills; capture stack trace and inspect scanner result entity lifetimes. |
| Medium | [#39](https://github.com/EvEmu-Project/evemu_Crucible/issues/39) Docking while target-painted causes segfault | Old report; comments mention several historical target-manager fixes, but also later reports of continued crashes. | Re-test current branch with offensive modules active during docking, warp and session changes; inspect target/module cleanup ordering. |
| Medium | [#315](https://github.com/EvEmu-Project/evemu_Crucible/issues/315) Outpost construction crashes | Repro steps exist but reporter supplied no crash log. | Reproduce after anchoring and build timer; capture server stack before modifying construction flow. |
| Medium | [#276](https://github.com/EvEmu-Project/evemu_Crucible/issues/276) Dungeon editor acceleration gate creation | Binding signature has been updated to accept the client’s nine arguments and nullable optional values. | Test the editor interaction and check whether other object types need the same protocol handling. |

## Upstream pull requests worth reviewing

- [PR #327 — belt rat spawn class and warp-in](https://github.com/EvEmu-Project/evemu_Crucible/pull/327): the relevant code changes have been adapted to this fork and are recorded above.
- [PR #332 — client-aligned warp/travel movement](https://github.com/EvEmu-Project/evemu_Crucible/pull/332): closed without merging and changes 75 files. A narrow WARP ball-state serialization correction has been adapted to this fork; the full Destiny physics rewrite remains unported pending review and gameplay tests.
- [PR #326 — broad gameplay fixes](https://github.com/EvEmu-Project/evemu_Crucible/pull/326): touches many unrelated systems. Review individual subsystems and test each change; do not merge the entire PR blindly.
- [PR #323 — persist ship position on logout](https://github.com/EvEmu-Project/evemu_Crucible/pull/323): review save timing, session changes and login warp semantics before adoption.
- [Issue #328 — ship physics claim](https://github.com/EvEmu-Project/evemu_Crucible/issues/328): reporter claims a fix but has not supplied a patch in the issue; no code has been adopted from it.

## Review rules

1. Confirm that the report still applies to this fork before changing code.
2. Prefer a reproducer, stack trace, failing test, or clearly demonstrated source defect.
3. Keep fixes narrowly scoped; avoid importing large unrelated upstream patches.
4. Build on Windows GCC, Linux GCC and Docker CI.
5. Test gameplay separately; compilation alone does not close a gameplay bug.
6. Never run destructive migrations or reset the user's live database as part of issue triage.
7. Update [PATCH_NOTES.md](PATCH_NOTES.md) after each committed fix and record its verification status here.


## 2026-11-14 progress update — Blockade encounter and drone combat

The following work is implemented on `combat-mission-drone-integration` and consolidated in [fork PR #11](https://github.com/xrizonx/evemu_Crucible/pull/11). This is a progress record for this fork; it does not change or close upstream issue records.

| Work item | Implementation/progress | Evidence and remaining verification |
|---|---|---|
| Synthetic Blockade encounter fleet | Expanded the Angel Cartel group 550 test pocket to 24 frigates. | User confirmed 24 ships spawn, move and fire in-game. |
| Kill objective | Explicit 24-kill target; initialized after generic Encounter setup; objective progress remains separate from objective type metadata. | Source path and payload corrected. Still verify the journal counter increments to 24/24 in a live client session. |
| 120-minute mission duration | Special 120-minute accepted duration for synthetic mission 65535 only. | User confirmed the journal now reflects 120 minutes. Other mission timers remain unchanged. |
| Reward persistence | 150,000 ISK reward stored through mission offer updates; old open synthetic offers with missing reward are normalized. | Source/SQL path corrected. Wallet credit still needs an end-to-end live turn-in check. |
| Completion and turn-in | Complete action is gated by required kills; existing agent completion flow pays reward and removes the encounter site/offer. | Source-level path reviewed. Verify Complete unlock, successful turn-in, wallet journal entry and site cleanup in-game. |
| Encounter kill target source | Offer target is derived from the mission pocket's required-kill value, not a generic NPC count. | Source-level correction; verify target against the spawned encounter. |
| Drone combat | Removed duplicate global damage scaling; improved lock-complete activation, approach/engage transitions, invalid target cleanup, target-loss handling and client state/target synchronization. | CI build is the current automated check; dedicated server-side drone AI unit coverage is not present. Gameplay regression testing remains required. |

### CI snapshot for the integrated code

Workflow [#43593340656](https://github.com/xrizonx/evemu_Crucible/actions/runs/43593340656), head `b527d20d17d7dff570ad0dd7b001042593f20589`:

- Docker image build: passed.
- Linux GCC build/test invocation/package: passed.
- Windows 64-bit UCRT64 GCC: still running at the time of this update; the final matrix is not yet fully green.

The run must be checked again after the documentation commit, because a new branch head triggers a new validation run. A green compile is not a substitute for the live mission turn-in and drone gameplay checks above.

### Fork PR housekeeping

- PR #11 is the consolidated integration PR and is the merge candidate after all required CI jobs pass.
- PR #8 (24-frigate encounter/reward), PR #9 (drone combat), and PR #10 (120-minute timer) overlap with the consolidated PR #11. Keep them open until #11 is successfully merged, then close them as superseded with a link to #11; do not independently merge duplicate branches.
- The fork's issue tracker API is disabled in repository metadata (`has_issues=false`), so this Markdown triage file is the maintained progress tracker for this fork. The upstream issue tracker is separate and is not being marked closed by these changes.


## 2026-11-14 follow-up — mission fails to complete after last NPC

**Reported symptom:** Player destroys all hostile NPCs in the Blockade pocket, but the mission remains incomplete / Complete is not offered.

**Code-path finding:** `SpawnMgr::SpawnKilled()` removed the final mission spawn entry and cleaned up the pocket, while offer progress was only persisted inside `RecordMissionKill(killerID)` success. A final NPC death without a resolvable player Client/killer ID could therefore leave the offer's persisted kill progress below its required count even though the mission spawn registry was empty.

**Fix committed on integration branch:**
- Determine whether the final mission spawn entry has been removed independently of per-kill credit.
- When the mission pocket has no remaining spawn entries, find the matching accepted offer by character, agent and offer ID and persist its kill progress to the required objective count.
- Keep ordinary kill progress capped at the target and send the normal mission update to the online character.
- Move the synthetic Blockade reward/24-kill objective initialization after the generic Encounter reset in `Agent::MakeOffer()`.

**Validation still required:** CI for the new head, then live acceptance and 24 NPC kills. Confirm the journal reaches 24/24, the agent's Complete action appears, turn-in marks the offer completed, exactly 150,000 ISK is credited, and the site is removed. Source inspection is not a substitute for this end-to-end test.

**PR overlap:** PR #10 contains the older timer-only implementation and conflicts/overlaps with the consolidated PR #11 on `EVE_Missions.h`, `AgentBound.cpp`, and `MissionDataMgr.cpp`. Keep #10 open only until #11 is merged; then close #10 as superseded rather than merging the divergent branch independently.


## 2026-11-14 follow-up — Blockade still not completing after pocket clear

**Reported symptom**
- The player reports that clearing every hostile ship still does not complete the combat mission.

**Revised diagnosis and fix**
- The prior completion fallback still depended on SystemBubble::RecordMissionKill(killerID) accepting a player character ID. NPC::Killed() deliberately passes zero when the damage source cannot be resolved to an online player Client, which can occur with drone/assist damage.
- Mission kill objectives are pocket-owned kill-all objectives. SystemBubble::RecordMissionKill() now counts every NPC death reported by the mission SpawnMgr, independent of kill-credit resolution, and remains capped at the required count.
- The final spawn-registry-empty fallback remains in place to persist the objective at the required count after the final hostile is removed.
- The 24-ship test fleet is now mixed: 12 frigates, 4 destroyers, 4 cruisers and 4 battlecruisers, with ship types resolved dynamically from the installed Angel Cartel NPC static-data groups.

**Still to validate**
- CI on the updated head must pass.
- In-game test must confirm all four classes spawn, every kill advances progress, the journal reaches the actual spawn count, the agent exposes Complete, turn-in credits 150,000 ISK once, and the mission pocket is cleaned up.
- If the test still fails, capture the SpawnMgr::SpawnKilled::Mission, Mission offer progress, and NPC::Killed log lines from a fresh mission run; these will distinguish missing death callbacks from offer lookup/persistence failures.


## 2026-11-14 follow-up — railgun misses and station warp-in bumping

**Railgun/turret diagnosis**
- Both player and NPC turret formulas used the magnitude of full relative velocity as transversal speed. This incorrectly included radial velocity along the line of sight; EVE turret tracking should use only perpendicular relative velocity.
- Updated both formulas to use `|relativeVelocity × normalizedLineOfSight|`, then angular velocity = transversal / range. Added safe handling for zero range, invalid NPC target state, missing signature/tracking values and zero falloff outside optimal.

**Station warp-in diagnosis**
- `BeyonceBound::CmdWarpToStuff` previously offset a station warp point by exactly the station radius. It omitted the player's ship radius and a safety margin, leaving the hull at the station collision surface.
- Station warp-in now adds the ship collision radius plus 5 km beyond the station radius. This affects the warp-in point only; docking range and normal approach behavior are unchanged.

**Validation pending**
- CI must pass before merge.
- Test railguns at optimal and falloff against stationary/approaching/receding targets and targets with transverse motion.
- Warp frigates, cruisers and battleships to station and verify no collision bounce; separately confirm docking approach still completes within the normal perimeter.


## 2026-11-14 follow-up — Blockade fleet spawning battleships

**Reported symptom**
- User observed battleships in the synthetic Blockade fleet. The highest permitted class is battlecruiser.

**Root cause**
- The configured NPC group IDs were incorrectly labelled: Angel Cartel group 552 is Battleships, while group 553 is Cruisers and group 576 is BattleCruisers. The old table therefore selected battleships for the four intended cruiser slots and did not use the correct battlecruiser group.

**Fix pushed to integration branch**
- Corrected the fleet group mapping to 550 Frigate (12), 551 Destroyer (4), 553 Cruiser (4), and 576 BattleCruiser (4).
- Group 552 (Battleship) is explicitly excluded.
- Added exact group-name validation both when loading candidate group types and immediately before creating each NPC. An unexpected group mapping now aborts fleet setup rather than admitting battleships or another unintended class.

**Validation pending**
- CI/compile must pass on the new head.
- Start a fresh mission and inspect every spawned hull / spawn log. Expected composition: 12 frigates, 4 destroyers, 4 cruisers, 4 battlecruisers; zero battleships, capitals or other higher-tier hulls.


## 2026-11-14 follow-up — Corrected spawn IDs from live log

**Observed runtime errors**
- Group 551 was rejected as the configured Angel Cartel Destroyer group.
- Group 553 was rejected as the configured Angel Cartel Cruiser group.
- The encounter consequently selected only 16/24 ships and aborted.

**Verified static-data mapping**
- 550 = Asteroid Angel Cartel Frigate
- 575 = Asteroid Angel Cartel Destroyer
- 551 = Asteroid Angel Cartel Cruiser
- 576 = Asteroid Angel Cartel BattleCruiser
- 552 = Asteroid Angel Cartel Battleship (excluded)
- 553 = Asteroid Angel Cartel Officer (excluded)

**Fix**
- Updated the mixed fleet table to use 575 for Destroyers and 551 for Cruisers. Kept the exact group-name/type validation and the battlecruiser ceiling.

**Validation pending**
- Rebuild and start a fresh mission. Confirm the spawn log reports 24/24 and the fleet contains exactly 12 frigates, 4 destroyers, 4 cruisers and 4 battlecruisers, with no officers, battleships or larger ships.


## 2026-11-14 — Blockade mission text and kill-counter polish

- [x] User-confirmed gameplay: all hostile ships destroyed, mission turn-in succeeds, and the reward is issued.
- [x] User-confirmed agent mission text is now visible.
- [x] Refined Blockade offer/active-mission wording with structured sections and ship-class objective breakdown.
- [x] Added persisted kill progress as a live `current / required` counter in the agent response and mission briefing/objective data.
- [ ] Build the latest branch head on Windows, Linux and Docker CI.
- [ ] In-game verify that the counter updates after each kill (for example 0/24, 1/24, 12/24, 24/24), and that the mission remains turn-in ready at 24/24.


## 2026-11-14 — Blockade briefing newline rendering

- [x] Root cause identified: the briefing source used double-escaped newline sequences, so the client displayed literal `\\n` characters.
- [x] Replaced double-escaped sequences with proper C++ newline escapes throughout the custom Blockade briefing strings.
- [x] Added spacing before the mission title to separate it from the agent division label.
- [ ] Rebuild and confirm that headings, paragraphs and kill counter render with real line breaks in the EVE client.

## 2026-11-14 follow-up — Blockade journal objective and drone combat

**Blockade symptoms**
- The accepted mission's first agent-conversation response showed a literal message ID `0` because the `actionID == 0` path still returned `offer.briefingID` as an integer.
- The synthetic mission has no localized title or briefing message IDs. The journal and briefing response still exposed those missing IDs, including briefing ID zero, even though custom text was supplied through the objective dictionary.
- The journal kill objective uses the normal encounter payload. The live counter is persisted in the encounter offer's `courierTypeID` field and the required count in `courierAmount`; the mission pocket kill callback updates the offer and sends a modified-mission notification. The objective response now also supplies an explicit objective-completed flag.

**Fix pushed**
- Return the formatted, dynamic Blockade briefing from the initial active-agent conversation as well as the View Mission action.
- Return `None` for synthetic title/briefing localization IDs in the journal and briefing response, avoiding attempts to resolve nonexistent message IDs.
- Preserve the standard encounter `kill` objective structure and expose completion state in its progress dictionary.

**Drone combat root cause**
- `entity.CmdEngage` was a stub that always sent “Drone Control is not implemented yet”; it never invoked the existing `DroneAIMgr::Target` implementation.
- The drone AI also used the shorter attack-range value as its outer target/pursuit limit, despite already caching a separate chase-distance attribute. This caused otherwise valid commands to be dropped before the drone could approach.
- `DroneAIMgr::Targeted` did not react to incoming NPC aggression, even when the drone's aggressive setting was enabled.

**Fix pushed**
- Implemented CmdEngage with validation for current system, player-locked target, valid ship target, selected drone ownership, home ship, online status and bubble membership.
- Route each valid selected drone through its existing targeting/AI path.
- Use the chase-distance envelope for target acquisition and pursuit, retaining the weapon envelope for orbit/attack.
- Enable NPC retaliation only for drones configured as aggressive; passive drones still require an explicit engage command.

**Validation pending**
- Fresh compile/CI is required; no successful build is claimed yet.
- In game, confirm journal shows the mission title and `0/24` initially, increments after each NPC death, displays the objective, and reaches `24/24`.
- Test CmdEngage with multiple owned drones against a locked NPC target; verify they approach, orbit and apply damage. Also test invalid/unlocked targets, drones owned by another pilot, offline drones, aggressive retaliation, and passive behavior.

## 2026-11-14 follow-up — drone combat and recovery lifecycle

**Confirmed code-level blockers**
- `DroneAIMgr::Target()` passed the drone's `maxAttackTargets` attribute directly to the NPC target manager. When static data omitted the attribute, the value was zero; the target manager rejects a target whenever current target count is greater than or equal to the limit. This prevented the drone from acquiring any target.
- The AI treated a target outside immediate lock range as a hard failure, despite the target manager indicating that the drone should approach. There was no retained target ID/retry path.
- Missing/zero static attributes could produce zero attack, orbit, chase or fly ranges and a zero weapon cycle.
- The old drone hit-chance calculation used total relative velocity instead of transversal velocity and divided by unguarded tracking/signature/falloff values.
- `CmdReturnHome`, `CmdReturnBay` and `CmdAbandonDrone` were still stubs that always reported drone control was not implemented.
- `ShipSE::LaunchDrone()` moved and spawned a drone before checking bandwidth. A failed bandwidth check could leave an inert drone entity in space.

**Fix pushed**
- Clamp drone target capacity to at least one and provide a safe nonzero target-lock duration.
- Supply conservative fallback values for incomplete drone static attributes.
- Retain distant engage targets by entity ID, approach them, resolve them safely through the current system manager and retry target locking when they enter range.
- Use transversal velocity in the drone tracking formula and guard invalid/missing tracking, signature, optimal-range and falloff values.
- Validate damage attributes and damage multiplier before applying damage; emit a combat log entry for each drone firing cycle and a warning if the drone has no base damage configured.
- Implement return-home/orbit, return-to-bay/automatic scoop within 2,500 m, and abandon commands.
- Complete bay recovery after the AI tick, validate bay capacity, transfer the drone item, release bandwidth, clear target locks and remove the drone entity.
- Validate bandwidth before launch side effects, preventing failed launches from leaving inert entities.

**Validation still required**
- Latest branch head must pass Windows, Linux and Docker CI.
- In-game test light, medium and heavy combat drones separately against NPCs. Confirm target lock, approach from outside weapon range, orbit, firing effects, shield/armor/hull damage and target death.
- Test multiple drones on one target and target switching after death.
- Test return-home and verify drones orbit the ship without disappearing.
- Test return-to-bay at close and long range; verify automatic travel, scoop, inventory transfer, drone count and bandwidth reduction.
- Test a full drone bay, bandwidth overrun, abandoned drones and offline/foreign drones. No runtime success is claimed until these tests are performed.

## 2026-11-15 follow-up — drone engage command and auto-acquisition

**New command blocker identified**
- The bound-service dispatcher validates argument types exactly. `CmdEngage` was registered with `PyInt*` for the target ID, but EVE can send item IDs as `PyLong`/`PyIntegerVar`. That caused the method signature not to match, so the handler could fail before any drone logic ran. Within the handler, drone IDs were also checked with `IsInt()` only, silently rejecting long-form IDs.

**Fix pushed to a new branch**
- Accept the target argument as `PyRep*` and safely decode either `PyInt` or `PyLong`, rejecting invalid/out-of-range IDs instead of truncating them.
- Decode drone IDs using the same helper, accepting both wire integer forms and logging invalid values.
- Add autonomous acquisition: idle online drones scan their current bubble and engage the nearest living NPC, excluding NPCs owned by the pilot and same-faction NPCs where both faction IDs are known. Drones then reacquire another NPC after the current target is lost/destroyed.
- Manual return disables automatic reacquisition until the drone is launched/online again or receives a new engage order.

**Validation still required**
- Compile the new branch on Windows, Linux and Docker CI.
- In game, confirm the CmdEngage handler is reached and accepts the client's actual target/drone ID encodings; then verify drones acquire NPCs without a manual order, apply damage, switch targets after kills and remain idle after a return command.


## 2026-11-15 — Drone engage reports no active drones

- **Root-cause candidates:** the engage handler could reject every selected drone when client-supplied IDs do not resolve to the server's 32-bit system entity IDs; it also depended on a cached DroneSE Client pointer being identical to the current session pointer.
- Explicitly assign the live pilot to DroneSE at launch.
- Use owner character ID plus controlling ship/home-ship identity for authorization; do not rely solely on cached Client pointer identity.
- When no supplied drone ID resolves and passes validation, enumerate current-system entities and issue the engage command to all active drones owned and controlled by the calling character/ship.
- Added rejection/fallback logs. Verify actual client command arguments and in-game drone damage after CI passes.


## 2026-11-15 follow-up — combat drones show Approach/Repair and deal no damage

**Code review finding**
- The drone AI used activity state `Engaged` (10) when it entered weapon range. The local enum describes `Combat` (1) as the fighting state and `Engaged` (10) as non-combat. This is a protocol/state-reporting mismatch and can explain an incorrect client activity label.
- The drone weapon loop used a per-target lookup as its fire gate, whereas the NPC AI uses TargetManager's explicit `CanAttack()` flag, set only after lock completion.

**Fix pushed to PR #13 branch**
- Enter `Combat` while actively attacking, keep `Combat`/legacy `Engaged` in the active AI processing path, and avoid reissuing orbit every process tick for an unchanged target.
- Gate weapon fire on TargetManager's completed-lock flag and log when the drone is still waiting for lock.

**What remains to verify**
- Fresh CI for this commit must pass.
- In game, check that combat light drones show Combat rather than Repair/Engaged, then confirm NPC shield/armor/hull values decrease.
- If there is still no damage, capture the new drone trace plus `DRONE__MESSAGE` / `DRONE__WARNING` output. The firing log confirms whether the attack loop is reached; the zero-base-damage warning identifies missing static damage attributes; the TargetManager lock trace identifies lock completion. This code inspection does not establish which runtime branch the user's current server is taking.


## 2026-11-15 follow-up — drones chase distant NPCs, no visible damage, and target cap appears stale

**Confirmed code findings**
- Idle auto-engage scanned the entire current system bubble and selected the nearest hostile NPC without considering the pilot's drone control distance. This could send drones across the system to NPCs that were not in the pilot's control area.
- The combat transition used the inner fly-range threshold while the AI also cached a larger attack envelope. The gap could cause repeated Approach/Combat transitions around the threshold.
- TargetManager's player cap used `m_targets.size()` without first pruning target entries whose entity had died, left the system, or changed bubble. The stored list could therefore disagree with the client's visible target list until the normal loss path ran.
- TargetManager's target-loss callback handled NPCs but not DroneSE, leaving the drone's client-visible target ID stale after a loss.

**Fix pushed to PR #13 branch**
- Restrict automatic NPC acquisition to the character's `droneControlDistance`; use a conservative 20 km fallback only when that character attribute is missing/zero. Reject explicit engage commands when the target is outside that control range. Once a valid order is issued, drones may continue pursuing if the target later moves away.
- Use the outer drone attack envelope to enter Combat and orbit at the configured drone orbit distance rather than repeatedly crossing the inner approach threshold.
- Store target IDs alongside target pointers, prune stale locks safely before target processing and new lock requests, recalculate the lock-complete flag, and notify the client when an orphaned lock is removed.
- Clear DroneSE's target ID on target loss and invoke the drone target-loss callback from TargetManager.
- Correct player target capacity to base two plus Targeting/Multitasking levels, capped by the ship's maximum; log tracked/locked/locking counts when the cap is reached.
- Add logs for raw drone damage attributes and before/after shield, armor and hull values around the common ApplyDamage call.

**Validation required**
- CI must pass on Windows UCRT64, Linux GCC and Docker.
- Test a light combat drone at close range and at a target within control range. Confirm the drone's damage-component log is nonzero and the post-ApplyDamage delta is positive. If components are zero, the remaining fault is the imported static attributes; if components are nonzero but deltas remain zero, inspect the common damage application logs/config.
- Test auto-engage with NPCs both nearby and far away in the same system. Drones should select only targets inside control range; a manually designated target may continue to be pursued after leaving control range.
- Lock/unlock/destroy/warp several targets repeatedly. Confirm the cap is based on live server locks and the target list stays synchronized; check the new cap diagnostic if the limit is still reported incorrectly.
