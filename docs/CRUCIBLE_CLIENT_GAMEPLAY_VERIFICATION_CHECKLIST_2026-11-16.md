# Crucible 2012 client: post-merge gameplay verification matrix

**Prepared:** 16 November 2026  
**Repository:** [xrizonx/evemu_Crucible](https://github.com/xrizonx/evemu_Crucible)  
**Merged source baseline:** `master` commit `b1122ea35f0ccb150997d58cad54e28e199e23f7` (PRs #15, #16, #17).  
**Final master CI run to inspect:** [43808049188](https://github.com/xrizonx/evemu_Crucible/actions/runs/43808049188).  
**Scope:** Crucible-era EVE client gameplay and MariaDB persistence. **All gameplay cases begin as NOT RUN.** CI success alone must not be recorded as a gameplay pass.

## A01 status — completed and signed off (17 November 2026)

- [x] **A01 — repeatable staging and client verification:** **RESOLVED / OPERATOR TESTED AND ACCEPTED.**
- Original implementation: [PR #32](https://github.com/xrizonx/evemu_Crucible/pull/32), merged at `7f600be1fa06191707f4352d46f2240bfe3d2159`.
- Automated evidence: [A01 MariaDB fixture CI 44072201705](https://github.com/xrizonx/evemu_Crucible/actions/runs/44072201705) passed; [merged-master Windows/Linux/Docker CI 44073936776](https://github.com/xrizonx/evemu_Crucible/actions/runs/44073936776) passed.
- **Operator sign-off:** On 17 November 2026 the maintainer explicitly reported all their A01 live tests passed and requested A01 be closed. No private client logs or SQL before/after exports were independently attached to this repository; the optional full-server GitHub Actions smoke was not used. These are evidence boundaries, not failed tests.
- A01's task-specific disposable fixture scripts, Docker Compose file and dedicated workflow were retired after sign-off to keep `master` free of completed temporary test scaffolding. Existing gameplay/C++ regression tests remain in place.
- **Do not treat this A01 sign-off as PASS for unrelated gameplay checklist rows below.** Pending agent, contract, drone, POS and industry tests remain independently tracked.

## 0. Test discipline and environment

- [ ] **P0-GATE-01** Record the actual installed `BUILD-INFO.txt` SHA, GitHub artifact name, EVE client build/version, platform (Windows 11 UCRT64/Linux/Docker), database schema version, config overrides and UTC start time. Do not test an older extracted ZIP by mistake.
- [ ] **P0-GATE-02** Back up the EVEmu MariaDB database plus `etc/eve-server.xml`, `sql/evedb.yaml`, `image_cache`, `server_cache` and available logs. Test on a private/staging server, not a public economy.
- [ ] **P0-GATE-03** Verify TCP game port 26000 and HTTP image port 26001 are client-reachable. When the client is on a second machine, `imageServer=127.0.0.1` is **not** an appropriate advertised address.
- [ ] **P0-GATE-04** Collect server startup logs, DB errors and any crash dump. Configure detailed logging only for relevant subsystems and restore normal logging after reproduction; sensitive login/authentication fields must not be posted publicly.
- [ ] **P0-GATE-05** Prepare two distinct characters (A and B), two clients where possible, two stations with agents, a high-/low-security system, a mission-capable combat ship, a second test target, 1 and 5 light drones, launchers/turrets/ammunition, and adequate ISK for market transactions. Keep ships, skill levels, fittings and target signatures recorded.
- [ ] **P0-GATE-06** Run the normal packaged test gates (where source build is available): `ctest --test-dir build --output-on-failure`; `python -m unittest discover -s tools/agent_portraits -p "test_*.py"`. Verify CTests include WeaponFormulaTest, MovementFormulaTest, DroneControlRangeTest, AgentHelpersTest and MissionLifecycleTest.

**Record each row** as `PASS / FAIL / BLOCKED / NOT RUN / KNOWN LIMITATION` with: client build, server SHA, UTC timestamp, character/agent/system/station IDs, actual versus expected, video/screenshot or packet/HTTP sample, relevant server log excerpts, and before/after DB or wallet state. If a crash occurs, stop testing that build until triaged.

**Suggested release gate:** All P0 cases that exercise supported features must pass before considering this build gameplay-validated. P1 features may be tracked as defects; documented unsupported systems must be marked KNOWN LIMITATION rather than silently counted as successful.

## 1. Station agents, portraits, journal and mission lifecycle

| ID / priority | Reproduction | Expected pass condition / evidence |
|---|---|---|
| AG-01 · P0 | Dock in a station with a valid agent; open the station Agents tab, agent conversation and journal. Repeat with an agent lacking optional `chrNPCCharacters` metadata. | Correct agent name, division/level/location, journal listing and interaction; no empty/broken row caused by missing optional portrait/bloodline row. Capture station and agent IDs. |
| AG-02 · P0 | With outward HTTPS disabled, request `http://<server>:26001/Character/<agentID>_50.jpg` using **GET**. First test no local JPEG, then place a valid `image_cache/Character/<agentID>_256.jpg` or `_50.jpg`. | Both respond HTTP **200**, `Content-Type: image/jpeg`, no external `Location` redirect. Missing genuine portraits use bundled neutral JPEG; per-ID local portraits override it. Restart and repeat. The image server does **not** support treating HEAD as a valid GET test. |
| AG-03 · P1 | Request an uncached *player character* portrait and unrelated icon/logo image; create/upload a player portrait if supported. | Agent placeholder never appears for player IDs; no unintended changes to other image categories or player portrait writes. |
| AG-04 · P0 | Character A requests and accepts a normal offer; inspect offered, accepted, deferred (when enabled), rejected/quit, completed and expired journal/dialog states. Repeat after server restart. | No phantom offers, valid state transitions and no action on a nonexistent/invalid state. Offer persistence survives restart and the journal reflects the server state. |
| AG-05 · P0 | A and B each request missions from the **same agent**; complete A's mission, decline/expire B's offer and immediately request another offer on A. | Agent/character/**offerID** identity is preserved; B's mission is not deleted or overwritten by A's cleanup. No stale completed offer blocks the next one. |
| AG-06 · P0 | Accept an encounter; attempt Complete before objectives; then complete legitimately, reopen dialogue repeatedly and reconnect/restart. | Early completion cannot pay or remove objectives; successful completion pays **once**, closes active offer and never pays again on repeated Complete calls. Cross-check wallet journal and `agtOffers` state. |
| AG-07 · P1 | Defer an offered mission and test expiry in a disposable DB at the boundary; repeat while the character is offline and restart around expiry. | Both offered/deferred states become expired once; the wrong character's mission is not removed and server never dereferences a missing agent. Do not modify expiry timestamps on the live DB. |
| AG-08 · P1 | Test courier mission item cleanup on an offline character with multiple stacks, including a partial stack and another unrelated stack of the same type. | Correct quantities and persisted inventory; no negative values or deletion of unrelated belongings. **Known remaining risk:** offline cleanup currently filters owner/type without strong mission provenance/location; treat unexpected deletion as a release blocker. |
| AG-09 · P1 | Complete a timed bonus mission once just inside its acceptance-based bonus window, once outside, on separate test offers. | Correct bonus payout exactly once only when eligible; base reward is correct in both cases. Capture wallet before/after and accepted/complete timestamps. |
| AG-10 · P1 | Open an ordinary research/storyline agent if present; inspect research section of agent journal. | Document actual behavior. Do **not** interpret missing/full research/storyline behavior as proof it has been implemented; those paths remain incomplete. |

**Example Windows 11 HTTP verification (use a real agent ID and the configured image host):**

```powershell
curl.exe -sS -D agent-headers.txt -o agent-portrait.jpg "http://127.0.0.1:26001/Character/3010819_50.jpg"
Get-Content .\agent-headers.txt
```

## 2. Synthetic Blockade mission, ISK reward and persistence

**This is the custom test encounter, not the canonical multi-wave Crucible Blockade.** Synthetic mission ID **65535**; **24 Angel Cartel frigates**; acceptance starts a **120-minute** completion timer; completion reward **50,000,000 ISK**. The offer itself has the existing one-day acceptance window.

| ID / priority | Reproduction | Expected pass condition / evidence |
|---|---|---|
| BL-01 · P0 | Request the test mission from an eligible security agent; inspect every offer/conversation/journal reward display. | All displays agree on **50,000,000 ISK**; objectives show **24/24** targets and the 120-minute accepted-mission window. |
| BL-02 · P0 | Accept, warp to the mission pocket and inspect NPC type/count and initial positioning. | One wave of **24 Angel frigates** exists, not a canonical reinforcement chain. NPCs are distributed, visible, targetable and not stacked at a single point. |
| BL-03 · P0 | Destroy 1, then 23 eligible mission ships using player guns and/or assigned combat drones. Check progress at 1/24, 23/24 and 24/24. | Count advances once per confirmed kill, never for unrelated targets; Complete remains unavailable before 24/24 and becomes available after the final eligible kill. |
| BL-04 · P0 | Complete once; record ISK balance immediately before/after; reopen agent dialogue, restart and retry. | Single **+50,000,000 ISK base mission payout** on completion, plus any separately documented eligible bonus; the same mission never pays twice, and the pocket is removed. Record wallet journal separately from NPC bounty income. |
| BL-05 · P1 | If an old accepted Blockade offer exists from before this build, inspect it and complete on staging. | Reward normalizes to the new central constant on offer lookup; prior **already completed** rewards are not backpaid. Record behavior rather than editing wallet history. |
| BL-06 · P1 | Abort/quit or let a disposable mission expire; restart between state transitions. | No success reward, no duplicate pocket/targets, correct final state and cleanup. |

## 3. Market seed and buyable containers

**Caution:** `evedbtool seed -force` replaces eligible **NPC corporation** sell orders in the configured regions. Preview and take a backup; do not reseed a production market solely for this test.

| ID / priority | Reproduction | Expected pass condition / evidence |
|---|---|---|
| MK-01 · P0 | Check `sql/evedb.yaml` for test region and saturation; use `evedbtool[.exe] seed -force -dryrun`. Apply `-force` only on disposable staging DB. | Generated insert + cleanup SQL permits published market-listed container groups **12 cargo, 340 secure, 448 audit-log secure, 649 freight**, without permitting every Celestial type. |
| MK-02 · P0 | Open the Crucible Market UI in a seeded region; search each available container family. Review best sell order and selected station. | Published, market-group-assigned items have buyable NPC sell orders in seeded stations. A particular station is **not guaranteed** when saturation is below 100%. |
| MK-03 · P0 | Purchase 1 of each available family with sufficient ISK; inspect station item hangar and wallet; undock/redock where appropriate. | Exactly one item of correct `typeID` arrives in purchaser inventory; ISK is debited once at the displayed price; order remaining volume updates consistently. |
| MK-04 · P1 | Place a separate player sell order in the test region, then reseed under backup/staging only. | Player-owned orders are preserved; appropriate NPC station-corporation orders refresh. |
| MK-05 · P1 | Search station/debris/wreck/other nonmarket Celestial items. | Unrelated celestial items are **not** added to the NPC sell seed by the container change. |

**Safe catalog query to identify candidates in MariaDB:**

```sql
SELECT it.typeID, it.typeName, ig.groupID, it.marketGroupID
FROM invTypes AS it
JOIN invGroups AS ig ON it.groupID = ig.groupID
WHERE it.published = 1
  AND it.marketGroupID IS NOT NULL
  AND ig.groupID IN (12, 340, 448, 649)
ORDER BY ig.groupID, it.typeName;
```

For seeded sell orders, cross-check `mktOrders` by `typeID`, `regionID`, `stationID`, `bid=0` and `volRemaining>0`, and compare station owner corporation with `staStations`. Do not assume the seed succeeded just because a market category exists.

## 4. Combat drones: range, repeated attacks, return and persistence

Control-range rule intended by this fork: **20 km base + 5 km per Scout Drone Operation level + 3 km per Electronic Warfare Drone Interfacing level**, adjusted by supported ship/fitting bonuses. Measure **new attack commands from the controller ship**, not from each drone.

| ID / priority | Reproduction | Expected pass condition / evidence |
|---|---|---|
| DR-01 · P0 | Without fitted range bonuses, test Scout/EW levels `0/0`, `3/0`, `5/0`, `5/5`. Order a new attack just inside/outside command range. | Command thresholds approximately **20/35/45/60 km**, allowing only rounding/distance presentation tolerances; outside commands are rejected without changing return orders. |
| DR-02 · P0 | Deploy 1 light combat drone and then 5; assign stationary and moving NPC targets for 60+ seconds. | Repeated attack effects **and repeated shield/armor/hull changes** over multiple cycles; no first-shot-only bug or frozen shot timer. |
| DR-03 · P0 | Issue attack while inside command range; let assigned NPC move outside that range while it remains locked, in the same bubble. Try a new command to a second distant NPC. | Existing assigned drone pursuit/fire continues while feasible; fresh out-of-range command is rejected. No accidental target reset to arbitrary nearby NPC. |
| DR-04 · P0 | Toggle Passive/Aggressive and Focus Fire independently; order Return to Orbit and Return to Drone Bay while drones are attacking; issue an invalid attack command mid-return. | Independent settings remain unchanged unless explicitly toggled. Return is not cancelled by invalid attack; drone scoops when within roughly **2,500 m** subject to capacity. |
| DR-05 · P1 | Destroy target mid-cycle, unlock/relock or remove the target from grid. | Drones stop firing on a dead/nonexistent target; where permitted, reacquire a valid locked target rather than hanging or crashing. |
| DR-06 · P1 | Test scoop with full bay and while home ship is moving. | No drone loss/duplication, consistent in-space count, inventory ownership and bay capacity checks. |
| DR-07 · Known limitation | Deploy mining drone, issue mining command against an asteroid. | **Do not expect ore extraction.** Mining-drone harvesting pipeline is currently not implemented; record its response separately from light-combat-drone regressions. |

Suggested evidence: 60–90 seconds of video of damage ticks, matching timestamps and **DRONE__AI_TRACE, DRONE__MESSAGE, TARGET__TRACE and DESTINY** traces when enabled.

## 5. Weapons, ship durability, missiles and AoE

Use the original Crucible-era data/attributes and configured server combat multipliers; **do not use current EVE Online tracking or damage values as the reference**. Prefer controlled target ships with measured signature radius, resistances, velocity and transversal rather than eyeballing damage.

| ID / priority | Reproduction | Expected pass condition / evidence |
|---|---|---|
| WP-01 · P0 | Fit one hybrid, projectile and energy turret. At fixed optimal and optimal+falloff, compare stationary versus orbiting target with varying signature. Fire 30+ cycles with identical fittings. | Hits/misses, tracking and falloff vary plausibly with angular velocity/signature, instead of guaranteed damage. Shield → armor → hull damage observes resists. |
| WP-02 · P0 | Fire repeated/autorepeat cycles, then grouped guns; inspect reload/ammunition, capacitor, RoF and stop action. | Weapon doesn't stop after first cycle, charge/cap consumption remains consistent, and server/client cycle state matches. **Known caveat:** grouped turret volley may still share a single hit-quality roll; record that separately. |
| WP-03 · P0 | Use light, heavy, cruise, rocket, heavy assault and torpedo missiles at short and near-max range, against stationary and moving small/large targets. | Flight speed/time and expiration plausibly follow Crucible attributes; damage arrives only when missile reaches target, not at an old fixed launch-time estimate. Moving targets are pursued, without immortal/zero-speed missiles or damage after expiry. |
| WP-04 · P1 | Vary Warhead Upgrades, Guided Missile Precision and eligible missile specialization skills; compare fitted/launch stats. | Correct category exclusions (GMP is for guided light/heavy/cruise family), **2% Warhead Upgrades** damage per level; don't apply unsupported skills to rockets/torpedoes. Launcher-side specialization RoF needs independent verification. |
| WP-05 · P1 | Compare linked launchers with ungrouped volleys, and NPC gun/missile fire with same target geometry. | Grouped missiles change volley output, not flight duration; NPC projectiles show sensible cadence/damage without null pointers. |
| WP-06 · P1 | Use Defender missiles against incoming small and higher-HP missiles; use smartbomb near a hostile ship and an incoming missile, with suitable legal/safety conditions. | Defender applies missile HP damage rather than unconditionally deleting every missile; smartbomb intercept checks remaining missile HP; ship resistances and affected targets behave consistently. Capture before/after HP and timers. |
| WP-07 · P0 | Take incoming damage on shields, armor and hull with neutral and high resistance targets; test cap exhaustion under weapon cycling. | No NaN/negative/nonfinite health, no 99%-resistance default for missing resonance attributes, and no crash when capacitor is zero. |
| WP-08 · Known limitation | Attempt bomb timed detonation or doomsday/superweapon on staging only. | **Do not claim fully supported.** Bomb flight/timed AoE and superweapon activation are known incomplete; record separately and do not conflate with ordinary missiles/turrets. |

## 6. Movement, warp tunnel and client/server synchronization

**High-priority regression:** Prior maintenance deliberately avoided changes to the warp acceleration/deceleration and packet state machine. Verify the **real Crucible warp tunnel effect**, authoritative ship position and correct destination/grid state. Do not accept a destination teleport as a full warp pass.

| ID / priority | Reproduction | Expected pass condition / evidence |
|---|---|---|
| WA-01 · P0 | At a safe in-system position, warp to station at range, then planet, then an owned bookmark; record start/end timestamps, client velocity and distance. | Align and acceleration occur, **warp tunnel appears for the entire travel interval**, then deceleration and a correct destination grid. No blink/teleport, premature tunnel loss, stuck controls or bounce back. |
| WA-02 · P0 | Perform multiple back-to-back warps and compare endpoints with server-side position; dock, undock and warp again. | Client and server agree on position and ball/entity state after each transition; ship isn't stuck in warp, invisible, or located on a stale grid. |
| WA-03 · P0 | Warp with drones launched/returning, NPC aggression, and a mission bookmark; check warp eligibility and transition. | No orphaned drones or duplicate system entities. Correctly permitted warp succeeds; prohibited warp is rejected consistently. |
| WA-04 · P1 | Initiate align then cancel/stop before warp entry, and attempt commands during actual warp. | Correct state-machine handling for interrupted **pre-warp** alignment and during-warp restrictions. Do not force interruption mechanics unsupported by the Crucible client. |
| WA-05 · P1 | After warp exit, approach, stop, orbit and follow a station/ship/asteroid; test low and high sub-warp speeds. | Smooth motion and stable orbit/stand-off; no large position teleport or unsigned-range wrapping. **Known limitation:** player-ship collision reporting is not a full object collision impulse solver. |
| WA-06 · P1 | Use 2 clients in the same system while one enters/exits warp, then jump to a neighboring system and reconnect. | Both clients see consistent ship presence, velocities and locations; no ghost balls, stale targets, hanging session change or duplicate entities. |
| WA-07 · P0 | Repeat WA-01 and WA-02 after the final merged build (not merely a prior branch build). | Regressions caught even though weapon/motion source changes previously left the warp code untouched. Attach actual captured warp-tunnel footage. |

## 7. Exit criteria and defect template

- [ ] **P0-CI** Final merged `master` CI at the actual deployed SHA is green for **Windows UCRT64, Linux GCC, CTest, EVEDBTool seed smoke checks, Docker**. Link the precise run; do not use earlier standalone PR runs as proof of combined master.
- [ ] **P0-CLIENT** Complete AG-01/02/04/05/06; BL-01/02/03/04; MK-01/02/03; DR-01/02/03/04; WP-01/02/03/07; WA-01/02/03/07 with two characters where applicable.
- [ ] **P0-INTEGRITY** Confirm no double payouts, negative wallet or inventory quantities, unintended player-market order removal, bad cross-character mission cache effects, or warp client/server desynchronization.
- [ ] **P1-REGRESSION** Run the remaining P1 cases as time permits; open separately scoped issues for non-blocking defects and mark unsupported systems as known limitations.
- [ ] **REPORT** Post a table containing every test ID, result, date, exact build SHA, setup/skills, reproduction steps, expected versus actual, logs/video and bug issue link. Keep credentials, tokens and private player identifiers redacted.

**Issue template:**

```text
Title: [Crucible QA][P0/P1] <subsystem> — <observed failure>
Test ID:
Deployed BUILD-INFO.txt SHA:
Client build/OS:
DB schema/config overrides:
Character/agent/system/station/test item IDs (non-sensitive only):
Start UTC / failure UTC:
Steps to reproduce:
Expected:
Actual:
Server log excerpt / CTest / SQL result:
Client video/screenshot / HTTP trace:
Frequency (e.g. 3 of 3):
Regression compared with which build:
```

## Reference audits in this repository

- [Agent image data](AGENT_STATION_IMAGE_AUDIT_2026-11-15.md) and [offline portraits](AGENT_LOCAL_PORTRAITS_2026-11-15.md)
- [Mission lifecycle and offline cleanup](MISSION_EXPIRY_AND_OFFLINE_STACK_AUDIT_2026-11-15.md) and [Blockade encounter](missions/BLOCKADE_TEST_MODE.md)
- [50m Blockade reward and container seed](BLOCKADE_REWARD_AND_CONTAINER_MARKET_SEED_2026-11-15.md)
- [Drone sustained combat/range](DRONE_RANGE_AND_SUSTAINED_COMBAT_2026-11-15.md) and [drone/movement audit](DRONE_AND_MOTION_AUDIT.md)
- [Crucible weapons review and known limitations](CRUCIBLE_WEAPONS_AUDIT_2026-11-15.md)
