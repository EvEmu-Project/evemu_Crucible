# Upstream EVEmu Crucible issue audit and scanner remediation — 17 November 2026

> **Status: review branch; not an upstream issue closure.** The current fork is `xrizonx/evemu_Crucible`, base `master` merge `97a98424d2dd`; proposed fixes are on `fix/upstream-issues-scanner-safety-2026-11-17` at source commit `b1e35f3e5e5eab8cb0dfbf22fe6d59600e1f02bf`. The upstream issue tracker is read-only to this integration. The audit deliberately separates **existing source patches**, **new code changes**, **partial mitigations**, and **untested reports**.

## Scope, methodology and guardrails

- Queried the [upstream open issues](https://github.com/EvEmu-Project/evemu_Crucible/issues) directly from GitHub REST, two pages, excluding pull requests. **123 open issues** were present when indexed. The table at the end is the **complete open-issue title inventory from that snapshot**, not a claim of 123 deep code reviews.
- Cross-checked selected reports against the latest fork master source before patching. Relevant files: `src/eve-core/database/dbcore.cpp`, `src/eve-server/EntityList.cpp`, `src/eve-server/exploration/Scan.cpp`, `ScanMgrService.cpp`, `src/eve-server/system/Container.cpp`, `src/eve-server/market/MarketMgr.cpp`, `utils/config/eve-server.xml`, and existing fork maintenance/audit documents.
- Do **not** conflate a source patch, successful build or reporter observation with verified resolution of intermittent runtime crashes. Upstream GitHub Issues must remain open unless upstream maintainers decide otherwise. No SQL changes or migrations were necessary for the scanner patch.

## Issue-by-issue decisions

| Upstream issue | Symptom | Fork finding / source change | Verification status |
| --- | --- | --- | --- |
| [#319](https://github.com/EvEmu-Project/evemu_Crucible/issues/319) | MySQL/MariaDB compilation with `my_bool` | **Already patched in fork**: `dbcore.cpp:112-113` uses `bool reconnect` and `mysql_options(...,&reconnect)`; no duplicate code change | Validated by current fork Windows/Linux CI [#575](https://github.com/xrizonx/evemu_Crucible/actions/runs/44142537989) |
| [#170](https://github.com/EvEmu-Project/evemu_Crucible/issues/170) | Probe recovery iterator invalidation | **Core fix already in fork**: `EntityList::Process()` advances/revalidates probe-map iterator before callback; **new source fix** clears `m_activeProbeMap` on removal | Requires live recovery timer/probe expiry tests |
| [#128](https://github.com/EvEmu-Project/evemu_Crucible/issues/128) | Segfault when scanning after travel | **New source fix**: `Scan::RefreshSystem()` gets current `Client::SystemMgr()`, resets stale probe pointers and guards missing managers at scan entry | New CI / multi-system scanning required |
| [#37](https://github.com/EvEmu-Project/evemu_Crucible/issues/37) | Directional scan cone slider ignored | **New source fix**: normalize camera and target vectors; reject NaN, zero geometry; 360° handling; add `ScanDirectionTest` CTest | New CI / real Crucible client slider testing required |
| [#262](https://github.com/EvEmu-Project/evemu_Crucible/issues/262) | Probe recovery crash | **Partial mitigation**: invalidate active probe references and guard null system; no proof that the crash's exact call stack was this path | Live scan/probe pickup + Windows dump if crash repeats |
| [#269](https://github.com/EvEmu-Project/evemu_Crucible/issues/269) | Scan after NPC destruction crashes | **Partial mitigation**: scanner rebinding and null guards; no direct NPC-death scan stack trace provided | Reproduce with multiple dead NPC wrecks and dscan/system scan |
| [#35](https://github.com/EvEmu-Project/evemu_Crucible/issues/35) | Wrecks survive beyond 2 hours | **Already implemented mechanism, not confirmed resolved**: `WreckSE::Process()` checks `m_deleteTimer`; `WorldDecay=120` in `utils/config/eve-server.xml`. Potential persistence/process issues remain | Live 2h+ wreck expiration test needed |
| [#95](https://github.com/EvEmu-Project/evemu_Crucible/issues/95) | Intermittent market freeze on item sale | **Overlaps existing market changes**, including NPC buy order settlement; no proof of intermittent MariaDB hang resolved | Concurrency/load and market DB performance tests needed |
| [#97](https://github.com/EvEmu-Project/evemu_Crucible/issues/97) | Agents not resetting after completion | **Overlaps existing fork mission lifecycle changes**, not verified as fully resolved | Repeated missions/relog/restart |
| [#138](https://github.com/EvEmu-Project/evemu_Crucible/issues/138) | Virtual market seeding | **Substantial fork implementation exists** (EVEDBTool/100% reference NPC buy/sell); do not reimplement | Database audit + client price/volume testing still needed |
| [#263](https://github.com/EvEmu-Project/evemu_Crucible/issues/263) | Warp-to-gate without automatic jump | Not patched in this review; requires separate Destiny/autopilot flow inspection | Client repro needed |

## Source patch, review branch and tests

| File | Change | Links |
| --- | --- | --- |
| `src/eve-server/exploration/Scan.cpp` | Current-system refresh before scan and scan callbacks; clear cached probe maps on system change; retire recovered probes from active map; directional scan uses robust normalized geometry | [#128](https://github.com/EvEmu-Project/evemu_Crucible/issues/128), [#37](https://github.com/EvEmu-Project/evemu_Crucible/issues/37), [#170](https://github.com/EvEmu-Project/evemu_Crucible/issues/170) |
| `src/eve-server/exploration/Scan.h` | New current-system ID and `RefreshSystem()` declaration | #128 |
| `src/eve-server/exploration/ScanMgrService.cpp` | Reject missing ship/system before calling Destiny or probing stale maps | #128, #262 |
| `src/eve-common/EVE_ScanningMath.h` | Pure, independent cone-inclusion math for normalized vectors; radians, finite validation and full-circle behavior | #37 |
| `src/eve-test/utils/ScanDirectionTest.cpp` | Deterministic CTest: unnormalized axis, 30° and 180° cones, 360° full coverage, opposite direction, zero vectors, NaN and boundary | #37 |
| `src/eve-test/CMakeLists.txt` | Register new standalone CTest in standard Windows/Linux CI | #37 |

### Reproduction and acceptance matrix

1. Start the patched Windows or Linux server; run complete GitHub CI (Windows UCRT64, Linux GCC + CTest, Docker and MariaDB seeding). Confirm `ScanDirectionTest` actually executes and passes in CTest logs.
2. For #128, undock, perform a system scan, **jump to two or more systems**, perform ship, probe and directional scans; return to origin and repeat. Confirm returned signatures always identify the current system, no old-system/wreck signals and no segmentation fault.
3. For #37, use the same cluster of test objects with directional angle slider at **5°, 30°, 90°, 180° and 360°** and verify narrower angles exclude off-axis objects, while 360° includes the full range. Test with camera direction vectors of varying lengths.
4. For #170/#262, launch multiple probes, start scan, recover one or several **during the scan timer** and after scanning, warp or jump systems, and let leftover probes expire. Verify no stale active probe entries, iterator errors or double frees.
5. For #269, destroy multiple belt rats, scan their wrecks in the same system, jump and scan again. If it crashes, capture symbolized stack/Windows Event Viewer faulting module; the scanner changes are not proven to solve unrelated entity lifecycle defects.
6. Retest DED & Officer Expedition and the original Blockade in the same region to confirm scanner changes do not break mission beacons.

## Current source status and known limitations

- `#319` already has the requested `bool` change; patching it again would be redundant.
- `#170` already has a conservative map-iterator repair in `EntityList::Process()`; the new change only closes the outstanding active-scan map gap, not every possible probe recovery crash.
- `#35` has a two-hour decay timer, but persistence across unloads and timer scheduling were **not** reproduced; do not call it resolved yet.
- `#95` has numerous fork market updates but its historical MariaDB freeze requires independent timed/concurrent testing.
- `#128` and `#37` are **code-level fix candidates pending CI and client verification**. They are not officially resolved upstream.
- `#262` and `#269` are **partially mitigated**, not closed.
- Upstream issue [#270](https://github.com/EvEmu-Project/evemu_Crucible/issues/270) was previously **closed**; the cache defect found here also explains a plausible path for stale previous-system scan results, but this was not independently reproduced.

## Complete upstream open-issue inventory (123 issues; titles only for unreviewed reports)

The following catalog supports future prioritization. **"Catalogued" means NOT fixed or source-reviewed in this pass.** Categories are keyword triage labels, not canonical GitHub labels.

- Other / infrastructure / feature: **40** open issues
- Combat / movement / ships: **35** open issues
- Inventory / POS / stations: **24** open issues
- Missions / agents / character: **9** open issues
- Scan / exploration / cosmic: **9** open issues
- Market / economy: **6** open issues

| Issue | Title | Triage area | Source review status |
| --- | --- | --- | --- |
| [#328](https://github.com/EvEmu-Project/evemu_Crucible/issues/328) | Fixed ship physics in Crucible. | Combat / movement / ships | Reporter claims physics fixed; independent confirmation NOT established |
| [#324](https://github.com/EvEmu-Project/evemu_Crucible/issues/324) | May be not a bug , just stupid me. Cant login to client | Other / infrastructure / feature | Catalogued; source-level root cause not reviewed yet |
| [#319](https://github.com/EvEmu-Project/evemu_Crucible/issues/319) | Simple fix for dbcore.cpp... | Inventory / POS / stations | Already patched in fork — bool reconnect and MariaDB connector interface |
| [#315](https://github.com/EvEmu-Project/evemu_Crucible/issues/315) | When clicking "build" on a station outpost egg, server closes/crashes | Inventory / POS / stations | Catalogued; source-level root cause not reviewed yet |
| [#276](https://github.com/EvEmu-Project/evemu_Crucible/issues/276) | Adding Acceleration Gate to a Dungeon Does not seem to work | Missions / agents / character | Catalogued; source-level root cause not reviewed yet |
| [#275](https://github.com/EvEmu-Project/evemu_Crucible/issues/275) | Belt Rats and Spawned NPCs disappear during combat if orbiting? | Combat / movement / ships | Catalogued; source-level root cause not reviewed yet |
| [#274](https://github.com/EvEmu-Project/evemu_Crucible/issues/274) | Deploying a control tower disappears but shows on directional scan for about five minutes. | Scan / exploration / cosmic | Catalogued; source-level root cause not reviewed yet |
| [#269](https://github.com/EvEmu-Project/evemu_Crucible/issues/269) | Destroying a few enemy NPC ships and using dscan or system scan can cause a server and client crash. | Scan / exploration / cosmic | Partial mitigation: scanner null/context guards; exact post-kill crash NOT proven resolved |
| [#268](https://github.com/EvEmu-Project/evemu_Crucible/issues/268) | Fitting 1600 Reinforced Steel Plates will sometimes cause ship to appear damaged | Combat / movement / ships | Catalogued; source-level root cause not reviewed yet |
| [#267](https://github.com/EvEmu-Project/evemu_Crucible/issues/267) | When weapons are grouped only the first weapon gets loaded with ammo | Combat / movement / ships | Catalogued; source-level root cause not reviewed yet |
| [#266](https://github.com/EvEmu-Project/evemu_Crucible/issues/266) | Large control Tower Won’t load into Standard Industrial or Advanced Industrial cargo | Inventory / POS / stations | Catalogued; source-level root cause not reviewed yet |
| [#263](https://github.com/EvEmu-Project/evemu_Crucible/issues/263) | Clicking on the Jump button warps you to the gate but does not jump the player. | Combat / movement / ships | Not yet source-audited: warp-to-jump completion |
| [#261](https://github.com/EvEmu-Project/evemu_Crucible/issues/261) | Can't warp directly to cosmic signature (Wormhole) -Also wormhole does not appear | Scan / exploration / cosmic | Catalogued; source-level root cause not reviewed yet |
| [#260](https://github.com/EvEmu-Project/evemu_Crucible/issues/260) | Covert Ops Cloak not working correctly with Anathema | Other / infrastructure / feature | Catalogued; source-level root cause not reviewed yet |
| [#256](https://github.com/EvEmu-Project/evemu_Crucible/issues/256) | Server Crash when reviewing the journal for assignements | Missions / agents / character | Catalogued; source-level root cause not reviewed yet |
| [#255](https://github.com/EvEmu-Project/evemu_Crucible/issues/255) | Fedora:33-x86_64 Error not found. | Other / infrastructure / feature | Catalogued; source-level root cause not reviewed yet |
| [#251](https://github.com/EvEmu-Project/evemu_Crucible/issues/251) | Asteroids not visible for 2nd Account | Other / infrastructure / feature | Catalogued; source-level root cause not reviewed yet |
| [#242](https://github.com/EvEmu-Project/evemu_Crucible/issues/242) | Staging - Character creation crashes server with 2 errors and caught signal 6 | Missions / agents / character | Catalogued; source-level root cause not reviewed yet |
| [#224](https://github.com/EvEmu-Project/evemu_Crucible/issues/224) | Adding Server config for wormhole spawns | Scan / exploration / cosmic | Catalogued; source-level root cause not reviewed yet |
| [#222](https://github.com/EvEmu-Project/evemu_Crucible/issues/222) | Adding server config who can dock in stations | Combat / movement / ships | Catalogued; source-level root cause not reviewed yet |
| [#220](https://github.com/EvEmu-Project/evemu_Crucible/issues/220) | Adding server config to disable chat window in high, low, null sec.  | Other / infrastructure / feature | Catalogued; source-level root cause not reviewed yet |
| [#213](https://github.com/EvEmu-Project/evemu_Crucible/issues/213) | using titan to kill an npc causes a segfault + you can't board a titan | Other / infrastructure / feature | Catalogued; source-level root cause not reviewed yet |
| [#212](https://github.com/EvEmu-Project/evemu_Crucible/issues/212) | segfault on board when you're surrounded by hostile ships | Combat / movement / ships | Catalogued; source-level root cause not reviewed yet |
| [#211](https://github.com/EvEmu-Project/evemu_Crucible/issues/211) | Wormholes do not spawn in system | Scan / exploration / cosmic | Catalogued; source-level root cause not reviewed yet |
| [#208](https://github.com/EvEmu-Project/evemu_Crucible/issues/208) | [Feature] Station Services / Management | Inventory / POS / stations | Catalogued; source-level root cause not reviewed yet |
| [#205](https://github.com/EvEmu-Project/evemu_Crucible/issues/205) | crash anchoring Moon Harvesting Arrays | Other / infrastructure / feature | Catalogued; source-level root cause not reviewed yet |
| [#203](https://github.com/EvEmu-Project/evemu_Crucible/issues/203) | getting jetcans weird loading behaviour  | Inventory / POS / stations | Catalogued; source-level root cause not reviewed yet |
| [#202](https://github.com/EvEmu-Project/evemu_Crucible/issues/202) | learnin skill "salvaging" make server halt without error | Missions / agents / character | Catalogued; source-level root cause not reviewed yet |
| [#200](https://github.com/EvEmu-Project/evemu_Crucible/issues/200) | Boarding spawn then re-boarding prior ship = STACKTRACE, no ammo and ServerError 35221 | Combat / movement / ships | Catalogued; source-level root cause not reviewed yet |
| [#199](https://github.com/EvEmu-Project/evemu_Crucible/issues/199) | contenair deploying when jettison and making server crash + unable to deploy any other way | Other / infrastructure / feature | Catalogued; source-level root cause not reviewed yet |
| [#197](https://github.com/EvEmu-Project/evemu_Crucible/issues/197) | Dragging charges to reload part-filled weapon in station results in exception | Combat / movement / ships | Catalogued; source-level root cause not reviewed yet |
| [#194](https://github.com/EvEmu-Project/evemu_Crucible/issues/194) | Cannot destroy wrecks | Inventory / POS / stations | Catalogued; source-level root cause not reviewed yet |
| [#190](https://github.com/EvEmu-Project/evemu_Crucible/issues/190) | Cannot remove Event - Advisory mission offer from Journal | Missions / agents / character | Catalogued; source-level root cause not reviewed yet |
| [#188](https://github.com/EvEmu-Project/evemu_Crucible/issues/188) | Station inventory appears empty after a system unload | Inventory / POS / stations | Catalogued; source-level root cause not reviewed yet |
| [#178](https://github.com/EvEmu-Project/evemu_Crucible/issues/178) | Migrate database subsystem to ODB | Other / infrastructure / feature | Catalogued; source-level root cause not reviewed yet |
| [#170](https://github.com/EvEmu-Project/evemu_Crucible/issues/170) | Recovering Probes causes Segfault | Scan / exploration / cosmic | Core iterator repair already in fork; active-probe cleanup added in this PR; LIVE RETEST REQUIRED |
| [#169](https://github.com/EvEmu-Project/evemu_Crucible/issues/169) | EvilNumber bullshit | Other / infrastructure / feature | Catalogued; source-level root cause not reviewed yet |
| [#165](https://github.com/EvEmu-Project/evemu_Crucible/issues/165) | [Feature] Implement contracts | Other / infrastructure / feature | Catalogued; source-level root cause not reviewed yet |
| [#164](https://github.com/EvEmu-Project/evemu_Crucible/issues/164) | Mission Move the goods, secure containers that were left on station vanished | Missions / agents / character | Catalogued; source-level root cause not reviewed yet |
| [#155](https://github.com/EvEmu-Project/evemu_Crucible/issues/155) | Strip Miner can not dig the ore, at least Veldspar and Scordite | Inventory / POS / stations | Catalogued; source-level root cause not reviewed yet |
| [#154](https://github.com/EvEmu-Project/evemu_Crucible/issues/154) | Killing a pod crashes the server | Other / infrastructure / feature | Catalogued; source-level root cause not reviewed yet |
| [#152](https://github.com/EvEmu-Project/evemu_Crucible/issues/152) | Polaris solar system stations are owned by System | Inventory / POS / stations | Catalogued; source-level root cause not reviewed yet |
| [#150](https://github.com/EvEmu-Project/evemu_Crucible/issues/150) | Wrong warping distance exit to a station or gate when you have seleced exact distance | Combat / movement / ships | Catalogued; source-level root cause not reviewed yet |
| [#147](https://github.com/EvEmu-Project/evemu_Crucible/issues/147) | Cant warp to member when in fleet | Combat / movement / ships | Catalogued; source-level root cause not reviewed yet |
| [#144](https://github.com/EvEmu-Project/evemu_Crucible/issues/144) | `.tr` needs completion (mostly working, but still incomplete)  | Other / infrastructure / feature | Catalogued; source-level root cause not reviewed yet |
| [#143](https://github.com/EvEmu-Project/evemu_Crucible/issues/143) | Destiny updates | Other / infrastructure / feature | Catalogued; source-level root cause not reviewed yet |
| [#140](https://github.com/EvEmu-Project/evemu_Crucible/issues/140) | Implement new TargetMgr | Combat / movement / ships | Catalogued; source-level root cause not reviewed yet |
| [#139](https://github.com/EvEmu-Project/evemu_Crucible/issues/139) | attacking asteroid causes server crash | Other / infrastructure / feature | Catalogued; source-level root cause not reviewed yet |
| [#138](https://github.com/EvEmu-Project/evemu_Crucible/issues/138) | Virtual Market Seeding | Market / economy | NPC market seeding exists in fork; live client coverage remains pending |
| [#136](https://github.com/EvEmu-Project/evemu_Crucible/issues/136) | Potencial pos exploit | Inventory / POS / stations | Catalogued; source-level root cause not reviewed yet |
| [#129](https://github.com/EvEmu-Project/evemu_Crucible/issues/129) | Pirate anomalies are buggy | Scan / exploration / cosmic | Catalogued; source-level root cause not reviewed yet |
| [#128](https://github.com/EvEmu-Project/evemu_Crucible/issues/128) | System scan causes segfault | Scan / exploration / cosmic | Proposed source fix: rebind cached system on every scan; CI and LIVE RETEST REQUIRED |
| [#122](https://github.com/EvEmu-Project/evemu_Crucible/issues/122) | Concord kiling pirates | Other / infrastructure / feature | Catalogued; source-level root cause not reviewed yet |
| [#111](https://github.com/EvEmu-Project/evemu_Crucible/issues/111) | Unable to Pilot "Cockroach" | Other / infrastructure / feature | Catalogued; source-level root cause not reviewed yet |
| [#106](https://github.com/EvEmu-Project/evemu_Crucible/issues/106) | [Future] Implement Billing | Other / infrastructure / feature | Catalogued; source-level root cause not reviewed yet |
| [#97](https://github.com/EvEmu-Project/evemu_Crucible/issues/97) | Sometimes mission agents do not reset after mission is completed | Missions / agents / character | Mission offer lifecycle changed in fork; repeated reset behavior NOT proven resolved |
| [#95](https://github.com/EvEmu-Project/evemu_Crucible/issues/95) | Market sometimes does not remove items and freezes | Market / economy | Market source work exists in fork; intermittent freeze and DB load NOT proven resolved |
| [#94](https://github.com/EvEmu-Project/evemu_Crucible/issues/94) | Station background vanishing after logoff | Inventory / POS / stations | Catalogued; source-level root cause not reviewed yet |
| [#92](https://github.com/EvEmu-Project/evemu_Crucible/issues/92) | Objects not visible to the second pilot to show up at an asteroid field | Other / infrastructure / feature | Catalogued; source-level root cause not reviewed yet |
| [#91](https://github.com/EvEmu-Project/evemu_Crucible/issues/91) | Adding Skills (via GM command) which are in Skill Queue causing SegFault | Missions / agents / character | Catalogued; source-level root cause not reviewed yet |
| [#78](https://github.com/EvEmu-Project/evemu_Crucible/issues/78) | Dragging multiple stacks from cargo to a jetcan only moves one of each | Inventory / POS / stations | Catalogued; source-level root cause not reviewed yet |
| [#77](https://github.com/EvEmu-Project/evemu_Crucible/issues/77) | Firing Missiles causes multiple client exceptions | Combat / movement / ships | Catalogued; source-level root cause not reviewed yet |
| [#76](https://github.com/EvEmu-Project/evemu_Crucible/issues/76) | Character re-customization can be completed now, but the data is not persisted | Other / infrastructure / feature | Catalogued; source-level root cause not reviewed yet |
| [#75](https://github.com/EvEmu-Project/evemu_Crucible/issues/75) | They've gone plaid!  MicroWarp drives can sometimes achieve LUDICROUS SPEED | Combat / movement / ships | Catalogued; source-level root cause not reviewed yet |
| [#74](https://github.com/EvEmu-Project/evemu_Crucible/issues/74) | Going nowhere fast: MicroWarp Drives are sometimes boat anchors | Combat / movement / ships | Catalogued; source-level root cause not reviewed yet |
| [#73](https://github.com/EvEmu-Project/evemu_Crucible/issues/73) | Jetcans are quite busted | Inventory / POS / stations | Catalogued; source-level root cause not reviewed yet |
| [#72](https://github.com/EvEmu-Project/evemu_Crucible/issues/72) | The Corporate Hanger button in an orca won't stay checked | Other / infrastructure / feature | Catalogued; source-level root cause not reviewed yet |
| [#71](https://github.com/EvEmu-Project/evemu_Crucible/issues/71) | Orca: "Scoop Ship to Maintenance Bay" doesn't work | Combat / movement / ships | Catalogued; source-level root cause not reviewed yet |
| [#70](https://github.com/EvEmu-Project/evemu_Crucible/issues/70) | Orca: Launch Ship from Bay doesn't work | Combat / movement / ships | Catalogued; source-level root cause not reviewed yet |
| [#69](https://github.com/EvEmu-Project/evemu_Crucible/issues/69) | Moving large items to cargo hold won't let you move the right number of units | Inventory / POS / stations | Catalogued; source-level root cause not reviewed yet |
| [#67](https://github.com/EvEmu-Project/evemu_Crucible/issues/67) | Sometimes: when undocking or logging back in, I have no speedometer | Combat / movement / ships | Catalogued; source-level root cause not reviewed yet |
| [#66](https://github.com/EvEmu-Project/evemu_Crucible/issues/66) | Can't join the Caldari Militia | Other / infrastructure / feature | Catalogued; source-level root cause not reviewed yet |
| [#65](https://github.com/EvEmu-Project/evemu_Crucible/issues/65) | Giving ISK to a corporation doesn't update its Balance | Market / economy | Catalogued; source-level root cause not reviewed yet |
| [#64](https://github.com/EvEmu-Project/evemu_Crucible/issues/64) | Cannot add Corp Contacts | Other / infrastructure / feature | Catalogued; source-level root cause not reviewed yet |
| [#63](https://github.com/EvEmu-Project/evemu_Crucible/issues/63) | Corporate Standing trumps personal Contact Standing | Other / infrastructure / feature | Catalogued; source-level root cause not reviewed yet |
| [#62](https://github.com/EvEmu-Project/evemu_Crucible/issues/62) | Agent availability is not consistent | Missions / agents / character | Catalogued; source-level root cause not reviewed yet |
| [#61](https://github.com/EvEmu-Project/evemu_Crucible/issues/61) | Can't move large stacks between Corp Hanger division Holds | Other / infrastructure / feature | Catalogued; source-level root cause not reviewed yet |
| [#60](https://github.com/EvEmu-Project/evemu_Crucible/issues/60) | Blatantly violate cargohold sizes | Inventory / POS / stations | Catalogued; source-level root cause not reviewed yet |
| [#59](https://github.com/EvEmu-Project/evemu_Crucible/issues/59) | Decorations show a "elapsed time" instead of date | Other / infrastructure / feature | Catalogued; source-level root cause not reviewed yet |
| [#58](https://github.com/EvEmu-Project/evemu_Crucible/issues/58) | Decorations can count Recipients, but doesn't list them | Other / infrastructure / feature | Catalogued; source-level root cause not reviewed yet |
| [#57](https://github.com/EvEmu-Project/evemu_Crucible/issues/57) | After recieving a medal, Medals tab break the UI. | Other / infrastructure / feature | Catalogued; source-level root cause not reviewed yet |
| [#56](https://github.com/EvEmu-Project/evemu_Crucible/issues/56) | POS bays have no limits | Inventory / POS / stations | Catalogued; source-level root cause not reviewed yet |
| [#54](https://github.com/EvEmu-Project/evemu_Crucible/issues/54) | Enemy positions "need a syncloc" | Inventory / POS / stations | Catalogued; source-level root cause not reviewed yet |
| [#53](https://github.com/EvEmu-Project/evemu_Crucible/issues/53) | When targeting out of range wrecks with a Salvager, the reported distance is quite off | Combat / movement / ships | Catalogued; source-level root cause not reviewed yet |
| [#52](https://github.com/EvEmu-Project/evemu_Crucible/issues/52) | Propulsion mods are not behaving | Other / infrastructure / feature | Catalogued; source-level root cause not reviewed yet |
| [#51](https://github.com/EvEmu-Project/evemu_Crucible/issues/51) | Grouped Weapons cause havoc | Combat / movement / ships | Catalogued; source-level root cause not reviewed yet |
| [#50](https://github.com/EvEmu-Project/evemu_Crucible/issues/50) | Unloading charges while in space to a full cargohold tells you that you have negative space | Combat / movement / ships | Catalogued; source-level root cause not reviewed yet |
| [#49](https://github.com/EvEmu-Project/evemu_Crucible/issues/49) | lightskipping: exiting warp with CRAZY speed | Combat / movement / ships | Catalogued; source-level root cause not reviewed yet |
| [#48](https://github.com/EvEmu-Project/evemu_Crucible/issues/48) | Aura doesn't tell me that my asteroid is depleted | Other / infrastructure / feature | Catalogued; source-level root cause not reviewed yet |
| [#47](https://github.com/EvEmu-Project/evemu_Crucible/issues/47) | Looting jetcans breaks cargoholds | Inventory / POS / stations | Catalogued; source-level root cause not reviewed yet |
| [#46](https://github.com/EvEmu-Project/evemu_Crucible/issues/46) | Can warp sideways | Combat / movement / ships | Catalogued; source-level root cause not reviewed yet |
| [#45](https://github.com/EvEmu-Project/evemu_Crucible/issues/45) | The "Groups" tab in the Market window is on siesta | Market / economy | Catalogued; source-level root cause not reviewed yet |
| [#44](https://github.com/EvEmu-Project/evemu_Crucible/issues/44) | The Personal Assets window spins forever | Inventory / POS / stations | Catalogued; source-level root cause not reviewed yet |
| [#43](https://github.com/EvEmu-Project/evemu_Crucible/issues/43) | Corporate Wallet has no Journal | Market / economy | Catalogued; source-level root cause not reviewed yet |
| [#42](https://github.com/EvEmu-Project/evemu_Crucible/issues/42) | Some modules can't handle the removal of their target | Combat / movement / ships | Catalogued; source-level root cause not reviewed yet |
| [#40](https://github.com/EvEmu-Project/evemu_Crucible/issues/40) | The market order window is not updated after a purchase | Market / economy | Not yet source-audited: market window refresh |
| [#39](https://github.com/EvEmu-Project/evemu_Crucible/issues/39) | Docking up while being target painted causes a segfault | Combat / movement / ships | Catalogued; source-level root cause not reviewed yet |
| [#38](https://github.com/EvEmu-Project/evemu_Crucible/issues/38) | Large Officer Neutron Blasters won't accept Ammo | Combat / movement / ships | Catalogued; source-level root cause not reviewed yet |
| [#37](https://github.com/EvEmu-Project/evemu_Crucible/issues/37) | Angle slider on Directional Scanner doesn't filter results | Scan / exploration / cosmic | Proposed source fix: normalize D-scan cone geometry; CTest added; LIVE RETEST REQUIRED |
| [#36](https://github.com/EvEmu-Project/evemu_Crucible/issues/36) | Lack of space lasers | Other / infrastructure / feature | Catalogued; source-level root cause not reviewed yet |
| [#35](https://github.com/EvEmu-Project/evemu_Crucible/issues/35) | Wrecks not removed after 2 hours | Inventory / POS / stations | Existing 120-minute wreck timer; world-decay failure still UNVERIFIED |
| [#34](https://github.com/EvEmu-Project/evemu_Crucible/issues/34) | Shield Extenders are not operational in space | Other / infrastructure / feature | Catalogued; source-level root cause not reviewed yet |
| [#33](https://github.com/EvEmu-Project/evemu_Crucible/issues/33) | Salvaging violates cargohold sizes | Inventory / POS / stations | Catalogued; source-level root cause not reviewed yet |
| [#32](https://github.com/EvEmu-Project/evemu_Crucible/issues/32) | New ore collected while mining doesn't automatically stack | Inventory / POS / stations | Catalogued; source-level root cause not reviewed yet |
| [#31](https://github.com/EvEmu-Project/evemu_Crucible/issues/31) | UX: Changing the name of another player's items. | Other / infrastructure / feature | Catalogued; source-level root cause not reviewed yet |
| [#30](https://github.com/EvEmu-Project/evemu_Crucible/issues/30) | Fitting window breaks with a Covert Ops Cloak | Other / infrastructure / feature | Catalogued; source-level root cause not reviewed yet |
| [#27](https://github.com/EvEmu-Project/evemu_Crucible/issues/27) | Notepad not saving notes | Other / infrastructure / feature | Catalogued; source-level root cause not reviewed yet |
| [#26](https://github.com/EvEmu-Project/evemu_Crucible/issues/26) | calendar not working | Other / infrastructure / feature | Catalogued; source-level root cause not reviewed yet |
| [#25](https://github.com/EvEmu-Project/evemu_Crucible/issues/25) | Hyperinflation: Repairshop does funky math, gives terrible prices | Other / infrastructure / feature | Catalogued; source-level root cause not reviewed yet |
| [#24](https://github.com/EvEmu-Project/evemu_Crucible/issues/24) | I can see objects at my destination when entering warp | Combat / movement / ships | Catalogued; source-level root cause not reviewed yet |
| [#23](https://github.com/EvEmu-Project/evemu_Crucible/issues/23) | When undocking, I suddenly loose all my cap | Combat / movement / ships | Catalogued; source-level root cause not reviewed yet |
| [#22](https://github.com/EvEmu-Project/evemu_Crucible/issues/22) | Sensors boosters are active while docked, can't be removed? | Combat / movement / ships | Catalogued; source-level root cause not reviewed yet |
| [#21](https://github.com/EvEmu-Project/evemu_Crucible/issues/21) | AutoTargeting Systems reduce your lock range to 0 | Combat / movement / ships | Catalogued; source-level root cause not reviewed yet |
| [#20](https://github.com/EvEmu-Project/evemu_Crucible/issues/20) | Pods wont jump to 0 | Combat / movement / ships | Catalogued; source-level root cause not reviewed yet |
| [#19](https://github.com/EvEmu-Project/evemu_Crucible/issues/19) | ship insurance broken | Combat / movement / ships | Catalogued; source-level root cause not reviewed yet |
| [#17](https://github.com/EvEmu-Project/evemu_Crucible/issues/17) | Even the IRS lets you round up? | Other / infrastructure / feature | Catalogued; source-level root cause not reviewed yet |
| [#16](https://github.com/EvEmu-Project/evemu_Crucible/issues/16) | jetcans are removed upon docking | Combat / movement / ships | Catalogued; source-level root cause not reviewed yet |
| [#14](https://github.com/EvEmu-Project/evemu_Crucible/issues/14) | repairwindow changes % dmg after undocking/docking | Combat / movement / ships | Catalogued; source-level root cause not reviewed yet |
| [#13](https://github.com/EvEmu-Project/evemu_Crucible/issues/13) | Can activate modules during warp deceleration, violate range limits | Combat / movement / ships | Catalogued; source-level root cause not reviewed yet |
| [#12](https://github.com/EvEmu-Project/evemu_Crucible/issues/12) | Once open, no limit on range for accessing jetcan | Inventory / POS / stations | Catalogued; source-level root cause not reviewed yet |
| [#8](https://github.com/EvEmu-Project/evemu_Crucible/issues/8) | Creating Titans inside Station using .create GM command break | Inventory / POS / stations | Catalogued; source-level root cause not reviewed yet |
| [#3](https://github.com/EvEmu-Project/evemu_Crucible/issues/3) | Autopilot disables itself | Other / infrastructure / feature | Catalogued; source-level root cause not reviewed yet |
| [#1](https://github.com/EvEmu-Project/evemu_Crucible/issues/1) | Auto lock from spawn wont reset.  Spawn disappears. | Other / infrastructure / feature | Catalogued; source-level root cause not reviewed yet |

---

**Next maintainer action:** review the scanner PR, obtain green CI, then validate scan, probe recovery and cross-system behavior in the 2012 Crucible client before identifying `#128`/`#37` as resolved. Do not close upstream issues based solely on static analysis.
