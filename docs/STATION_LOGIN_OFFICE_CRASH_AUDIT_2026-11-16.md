# Crucible station-login failure audit — office identity and null dereferences

**Date:** 16 November 2026  
**Scope:** EVE Crucible 2012 client docking and existing station office loading.  
**Reported system:** Maurasi (`solarSystemID 30000140`).  
**Current server baseline before patch:** `1d46f4b5cda7d7fe9b14ff2372d7c904a401dabd` (all experimental agent portrait changes rolled back).  
**Status:** Source bugs fixed and automated tests added; live client station-load stability **not verified**.

## Log analysis (what the evidence proves)

The supplied 199-line server excerpt at approximately **08:36:37** contains exactly **16 repetitions of an identical 12-frame unsymbolized stack in `eve-server.exe`**, followed by `stationSvc.GetSolarSystem`, `facWarMgr.GetWarFactions`, `facWarMgr.GetFacWarSystems`, and `Client::Logout`. The upload does not show the first log message **before** those repeated frames and does **not** contain a decoded **client** crash dump.

Compared with the previous 08:11:12 stack, the application frames have a nearly constant address relocation. The same stack shape is being hit across launches. An actual faulting client address, register state and exception code **cannot** be inferred from those numbers.

An earlier commit `9a3a317068cf0948b950b4e44aadefea75a15524` removed repeated nonfatal `EvE::traceStack()` prints from `DynamicEntityFactory::BuildEntity`. The log resembles those historical prints. A stale process/binary is possible; **this is not proof** that the present crash is caused by that particular function.

## Confirmed station-login defects addressed

### 1. Wrong corporation office selected due to assignment in a condition

Before this patch, `StationItem::GetOfficeID(uint32 corpID)` contained:

```cpp
for (auto cur : m_officeMap)
    if (cur.second.corporationID = corpID)
        return cur.first;
```

That is an **assignment**, not a comparison. For a nonzero player corporation, it always returns the **first office** in the station (if any), not the character corporation's actual rented office. This can cause one corporation to load another corporation's office inventory and may also trigger a missing/wrong object load. The loop mutated only the copied `cur` pair, not stored map data, but still returned the wrong ID.

**Fix:** use a pure, read-only helper matching `office.second.corporationID == corpID`. Return 0 if there is no matching office. A CTest verifies correct selection for multiple corporations, missing IDs, empty offices and unchanged source data.

### 2. Unsafe dereference of missing corporate office item

`StationItem::LoadStationOffice`, invoked from `Client::MoveToLocation` on docking, used `oRef->GetMyInventory()` **without checking if `GetOfficeRef(officeID)` returned null**. Now missing office references and failed inventory loads are logged and skipped without marking the office as loaded, allowing a retry rather than a null-pointer crash.

### 3. Login/logout resilience with incomplete station data

- `Client::~Client` no longer dereferences `GetStationByID(...)` unconditionally while cleaning up a docked character. A failed/partial station cache can be logged without causing another server crash during logout.
- `SystemManager::LoadSystemStatics` checks missing station item references before constructing a `StationSE`.
- `StationItem::GetOffices` checks a null cached office response rather than incrementing a null PyRep reference.

These guards prevent specific **server-side** crashes; their interaction with any reported **client-side** exception requires gameplay testing.

### 4. Office folder SQL ID mismatch

`StationDB::GetStationOfficeIDs` accepted `locationID` for a station office folder but subtracted `STATION_OFFICE_OFFSET` before comparing it with `staOffices.officeFolderID`. Stored `officeFolderID` values are **already** the station ID plus that offset, so the query searched for the wrong value. The patch uses `locationID` unchanged. The return type and client wire contract are not modified.

## Preserved features

The patch does **not** modify:

- Agent portrait or image-server code; full experimental rollback from PR #22 remains intact
- Agent mission offers/completion, Blockade synthetic 50 million ISK payout
- Cargo container Assemble, multi-stack split/notifications, market group seeding
- Drones, turrets, missiles, damage or ship physics
- Warp tunnel, DestinyManager, protocol/serialization, solar system movement formulas

## Windows 11 clean deployment and reproduction

1. Back up MariaDB and configuration. Stop running `eve-server.exe` and the Crucible client.
2. Install a **new matching Windows UCRT64 CI artifact** into an empty folder. Verify `BUILD-INFO.txt` (the PR head SHA or later merged SHA) and the executable process path:

```powershell
Get-Content .\BUILD-INFO.txt
Get-CimInstance Win32_Process -Filter "Name='eve-server.exe'" |
  Select-Object ProcessId, ExecutablePath, CommandLine
```

3. Run the **same** Crucible client build and enter the previously crashing station in Maurasi.
4. If possible, compare characters with and without a rented office in that station; do not rent/impound offices in production to perform this test.
5. Verify the returned office ID matches the character's corporation. On a staging/backup MariaDB copy, a **read-only** audit query is:

```sql
SELECT stationID, itemID AS officeID, corporationID
FROM staOffices
WHERE stationID = <actual_station_id>
ORDER BY itemID;
```

6. Test undock/redock, relog and mission agent conversations. Ensure cargo container assembly and market buying still work.
7. If the client still crashes, collect the **Windows Application Error event for the client executable**, exception code/faulting module, any client minidump and 100 server-log lines **before** the repeated stack trace. The present server excerpt alone does not identify the client exception. Preserve the original database and cache for comparison.

## Test and confidence boundaries

- **Tested in CI:** C++ CTest matching different office IDs to different corp IDs, source compilation on Windows/Linux/Docker, existing combat/agent/container regression tests.
- **Not tested in CI:** a real populated MariaDB with the user's character/corporation/office, authentic Crucible client station login, the client exception, or whether clearing its local cache changes its behavior.
- **Conclusion:** real station-office lookup and null-dereference bugs have been corrected. A claim that the reported client crash is fully fixed must await a successful user-side station-load test.

## Additional audit findings, not changed here

- `StationSvc::GetSolarSystem` caches a system row using only the service/method name; if multiple different systems request it within the same process, the cache may reuse a prior system row. Changing object-cache keys may affect the old client wire contract and should be tested independently before modification.
- A generic session getter prints unsymbolized stacks for missing session keys if `CLIENT__SESSION_NOTFOUND` is enabled; these could explain repeated stack output, but **their caller is not established** by the uploaded log.
- The source-level station-office fixes should not be conflated with NPC agent portrait delivery, which was removed entirely in PR #22.
