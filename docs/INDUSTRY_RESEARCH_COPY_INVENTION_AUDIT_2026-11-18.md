# EVEmu Crucible — industry research, copying and invention review

**Review date:** 18 November 2026  
**Source branch:** `fix/gate-warp-landing-surface-clearance-2026-11-18` (ongoing fork PR #43)  
**Client version:** EVE Crucible-era science and industry (pre-Phoebe terminology: material **ME** and productivity/time **PE**)

## Audit results by activity

| Activity | Source state after this patch | Limitations |
| --- | --- | --- |
| Material Efficiency research (activity 4) | Research job quote, location and activity permission checks, skill/time calculation, station job insertion, completion and persisted `invBlueprints.mLevel` update are implemented | Station path requires live UI/database testing; current source only allows cumulative ME up to level **10**, reflecting its existing research time table |
| Productivity / Time Efficiency research (activity 3) | Same flow for `invBlueprints.pLevel`; skill now reduces rather than increases time, and the PE float calculation no longer truncates to zero | Same 10-level code cap; old PE system is not modern percentage-based TE |
| Blueprint copying (activity 5) | BPO-only check; rejects zero, negative, over-limit and int16-overflow runs; total time = base copy time × BPC count × licensed runs; new BPC retains parent ME/PE and explicit copy metadata | Need game client test confirming every copy is delivered and shown in hangar; job completion is not fully transactional |
| Invention (activity 8) | **Not functional; explicitly blocked before costs/material consumption** | No persisted output blueprint selection, complete datacore/decryptor mechanics, safe success roll, failure state or authentic Tech II BPC generation |
| Manufacturing (activity 1) | Existing manufacturing retained; original BPO infinite-run sentinel no longer decremented | Full production economics and material-return behavior need separate live verification |
| POS/remote research | Line discoverability/migration present | `LinePermissionCheck` deliberately blocks POS jobs; remote installation and job delivery are unfinished |

**This is not a claim that all client workflows are verified.** The C++ test suite checks formulas and permissions source invariants; only a real client connected to a seeded industry station can prove the UI/DB/job-delivery path end to end.

## Bugs corrected in this patch

1. **Wrong research line accepted:** `FactoryDB::GetMultipliers` previously returned success even when no allowed category/group matched. `LinePermissionCheck` also did not compare the selected activity against the line's activity. Both issues are addressed.
2. **Invalid copying jobs:** reject zero and negative BPC licensed runs, excess runs and job-count overflow. A BPC cannot be researched/copied, and a BPO remains infinite for manufacturing.
3. **Time and costs:** total blueprint copy time includes each licensed run of each copy. Billable line usage is calculated only after the final total time; research skill levels reduce ME/PE duration. `CopyTime` accepts the full uint32 static-data duration, not a truncated 16-bit duration.
4. **Blueprint metadata:** the duplicate-key `invBlueprints` write now persists the `copy` flag as well as ME/PE/runs. `GetPE()` uses floating-point division.
5. **Materials:** shared total-quantity rounding for quote, missing materials, inventory verification and consumption; availability aggregates multiple stacks of the same type and checks ownership; `extra` items (e.g. datacores) are not discounted by blueprint ME or character PE.
6. **Installation security:** blueprint owner and actual location are validated; personal material source now decodes the BOM location rather than the blueprint location; a client cannot override or submit negative installation cost.
7. **Job completion/access:** an owner cannot query another player's personal jobs. A player cannot deliver/cancel someone else's personal industry job, or redirect the output to a different container ID.

## Known remaining risks

- **Invention is disabled, not implemented.** Its old `CompleteJob()` placeholder constructs a fake success response with zero output item/type IDs. Keep installation blocked until a persisted invention job model and T2 blueprint creation/skills/decryptors are implemented and tested.
- **Non-atomic payment/job/delivery flow:** wallet transfer, `ramJobs` insertion, line scheduling, blueprint move, material deductions and output spawning are not wrapped in one database transaction. A mid-process DB/inventory failure can leave a partially installed/delivered job. Do not claim full financial/inventory crash safety until transactions and rollback tests exist.
- **Corporate hangar roles and remote/POS labs:** the source contains explicit TODOs and disabled paths. Test personal station research first; avoid announcing corp/POS research as complete.
- **ME/PE research level cap:** `RamMethods::ActivityCheck` limits `mLevel/pLevel + runs` to 10 and `EvEMath::RAM::Research_LevelModifier` supports that range. Do not silently lift it without implementing higher-level timings and checking the legacy client protocol.

## Automated checks

- `IndustryFormulaTest` (CTest): material rounding, extra-material rules, copy-run validity, skill-based ME/PE research speed and >65535s copy durations.
- `tools/verify_industry_workflows.py`: station/research/copy access checks, owner and location validation, metadata persistence, quote/time consistency, invention disabled by design.
- Windows UCRT64 and Linux CI must both compile and execute the tests. Docker and MariaDB seed checks must not regress.

## Manual in-game validation

1. In a station with a material research line, put a personal **BPO in the hangar** and quote ME 0 → 1, then install, wait, deliver, relog and verify ME remains 1.
2. Repeat with a **PE/Time Efficiency** line and verify research skills decrease the quoted time. Test 0 → 1 → 2 and rejection of a request exceeding level 10.
3. Copy the BPO using 2 copies × 5 licensed runs. Ensure quote time reflects 10 run-equivalents, both new BPCs have `copy=1`, `runs=5` and preserve BPO ME/PE; verify originals remain `copy=0`, `runs=-1`.
4. Try copying an existing BPC, zero licensed runs and a wrong activity/lab line: each must reject **before payment and item movement**.
5. Use two separate stacks of required material; quote and submit must match and consume only the owner/corp's authorized stacks.
6. Test cancellation, early delivery, duplicate delivery, denied output redirection and another character's job ID. Monitor wallet, BPO, BPC, `ramJobs`, item location and line scheduling.
7. Attempt invention on a T1 BPC: expect an explicit not-implemented error, with **no loss of ISK, datacores or BPC runs**.

### Read-only HeidiSQL verification

```sql
-- Set a real blueprint ID from your own database.
SET @bpID = 140000000;
SELECT e.itemID, e.typeID, e.ownerID, e.locationID, e.flag,
       b.copy, b.mLevel, b.pLevel, b.runs
FROM entity AS e
JOIN invBlueprints AS b ON b.itemID = e.itemID
WHERE e.itemID = @bpID;

SELECT jobID, ownerID, installerID, assemblyLineID, installedItemID,
       runs, licensedProductionRuns, completedStatusID,
       beginProductionTime, endProductionTime
FROM ramJobs WHERE installedItemID = @bpID
ORDER BY jobID DESC LIMIT 20;
```

No SQL migration is needed for this patch and no live database has been modified by this PR.
