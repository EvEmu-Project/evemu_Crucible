# Instant Level 5 Military, Industry and Strategic sovereignty testing

**Fork:** xrizonx/evemu_Crucible • **Branch:** feat/sov-index-level5-gm-test-2026-11-18 • **Date:** 18 November 2026

## Audit

The original `/sovindex5` achieved Strategic Level 5 but left Military and Industry at Level 0: their `5` values were raw points, **not displayed levels**. The client calculates activity levels from raw point totals, and the original C++/SQL fields were too narrow for large totals. This follow-up patch uses 32-bit values and client packet fields. The existing `/sov` handler is a nonfunctional logging stub. `SovereigntyDB` returns hard-coded **5** for `militaryPoints` and `industrialPoints` in every claimed system; there is no NPC/mining activity progression implemented. Strategic index uses elapsed FILETIME claim age. The previous `GetAllDevelopmentIndices` service was a stub returning null. This patch implements that list response and the developer-only persistent test override, **not** a complete activity-driven Sovereignty Index feature.

Crucible/Dominion-era Strategic Level 5 requires **100 days** of unbroken claim ownership. Instead of modifying `mapSystemSovInfo.claimTime`, the command displays 100 simulated strategic days for testing, tied to the **current TCU itemID**. Real claims and upgrade inventories remain untouched.

## Install the SQL migration before using the new command

After making a backup of the `eve` database and ensuring no other migrations conflict, apply:

`sql/migrations/20261118070000-sov-development-index-overrides.sql` **Up** section, or let your normal migration runner apply it exactly once. It creates a new `mapSovDevelopmentIndexOverrides` table; no existing player / alliance / POS rows are altered. The new server can start without the table (it logs that test overrides are unavailable), but the new command will fail until the migration is applied. **If you previously applied that first migration, also apply the new `sql/migrations/20261118093000-sov-development-index-point-width.sql` migration via EVEDBTool**, then rebuild/restart. It widens the activity point columns and corrects old (5,5) overrides automatically. Never run the Down section just to reset a system.

## In-game usage

1. Update and restart `eve-server` with the build from this PR.
2. Log in with a **DEV** (`0x62f8000280c41000`) or **ADMIN** role. Normal corp members, CONTENT-only accounts, and VIPs cannot use this command.
3. Travel to an **alliance-claimed, player-claimable null-sec** solar system with an online, linked Territorial Claim Unit and registered Infrastructure Hub. The override affects only your **current system**, not a named system or entire region.
4. In chat, execute `/sovindex5`. The server writes **Military 1,000,000,000 raw points, Industry 30,000,000 raw points and Strategic 100 effective days**, targeting Level 5 in each development index to `mapSovDevelopmentIndexOverrides`, keyed to the current TCU. The real `claimTime` remains unchanged.
5. Reopen the Sovereignty information and IHub management panes to refresh cached index values. Test installing an upgrade that requires Level 5; note the actual client UI and any server errors.
6. Execute `/sovindex5 reset` to delete only the current TCU override, restoring the normal source values. If the system changes sov holder, the old TCU marker cannot activate for the new claim even if the stale table row remains. On ordinary claim removal, the server also attempts to delete the previous claim's test override so re-onlining the same TCU does not resurrect development test levels.

The override persists across server restarts and indexes are shared through `GetCurrentSovData`, `GetSystemSovereignty`, `GetDevelopmentIndicesForSystem` and implemented `GetAllDevelopmentIndices`.

**Limitations:** Military and Industry now receive artificial **raw point totals**, not the literal index level. Chosen values fit signed 32-bit PyInt and exceed historical Level 5 industry volume (24 million m3). These are GM-only testing values, not genuine NPC/mining progress. The precise military index threshold and final Crucible client Level 5 UI remain to be verified during gameplay. This command does **not** implement infrastructure upgrade installation, activation/fuel/upkeep checks, NPC anomaly spawning from installed upgrades, or full CCP sovereignty rules. A command success message means the override was stored, not that end-to-end IHub installation worked. Verify with the Crucible client and report any errors/screenshots. The GM override must never grant sovereignty in an unclaimed or NPC-faction system.

## Regression and operator checks

- CTest `SovIndexOverrideTest` covers Level 5 simulated points and age, claim transfer isolation, reset behavior via null override and real claim ages exceeding 100 days.
- Existing Windows/Linux CI runs `tools/verify_sov_pos_integrity.py`, which now asserts new GM role boundaries, migration path, no `claimTime` schema changes, and index-view wiring.
- Read-only SQL verification after using command:

```sql
SELECT s.solarSystemID, s.allianceID, s.claimStructureID, s.hubID,
       s.claimTime AS actualClaimTime, o.claimStructureID AS overrideTCU,
       o.militaryPoints, o.industrialPoints, o.strategicDays
FROM mapSystemSovInfo AS s
LEFT JOIN mapSovDevelopmentIndexOverrides AS o
    ON o.solarSystemID=s.solarSystemID
WHERE s.solarSystemID = YOUR_SOLAR_SYSTEM_ID;
```

Check that `actualClaimTime` is unchanged and the override TCU matches the claim TCU. After `/sovindex5 reset`, the joined override fields should be NULL.

## 18 November — Correcting Level 0 Military and Industry

The in-game test confirmed **Strategic Level 5 / Military Level 0 / Industry Level 0** after running the first version of `/sovindex5`. The fix uses `uint32` C++ fields, MariaDB `INT UNSIGNED` and `DBTYPE_I4` client rows instead of SMALLINT/I2. It stores **1,000,000,000 military raw points** and **30,000,000 industrial raw points** as artificial development. The separate migration `20261118093000-sov-development-index-point-width.sql` updates the schema and upgrades existing old test rows from `(5,5)` to these values.

**Operator steps:** Apply this follow-up migration in `eve` using EVEDBTool (do not delete the original first migration), rebuild the current PR branch, restart the server and run `/sovindex5` again. Reopen the Sovereignty/IHub view; confirm all three Level 5 indicators. Then try a Level 1 upgrade, followed by higher tiers up to Level 5 (each requires the earlier tier). Report any failure separately.

**Read-only SQL verification:**

```sql
SELECT solarSystemID, claimStructureID, militaryPoints, industrialPoints, strategicDays
FROM mapSovDevelopmentIndexOverrides;
```

Expected new values for the active TCU: `1000000000`, `30000000`, `100`. A matching row establishes the database is storing the correct test data, but only the live client verifies correct UI level calculation. The `/sovindex5 reset` command remains available to revert the GM override. Do not run the migration `Down` in normal operation.

## 18 November — Supercapital Construction Facilities no longer hard-rejected

The client error `Supercapital Construction Facilities is not currently supported.` originated from `Inventory::ValidateIHubUpgrade` in `src/eve-server/inventory/Inventory.cpp`. The old validator supported only Cynosural Navigation, Cynosural Suppression and Advanced Logistics Network, rejecting **all other** structure upgrades, including type `2009`. It also used the **real** TCU claim timestamp instead of the effective Strategic days exposed by `/sovindex5`.

Updated support explicitly accepts **Supercapital Construction Facilities (type 2009)** at **100 effective Strategic days (Level 5)**, without modifying `mapSystemSovInfo.claimTime`. It requires a registered online IHub in an actual sovereign alliance claim, restricts installation to that alliance or a DEV/ADMIN test account, and rejects a duplicate upgrade type rather than moving/converting it twice. `Inventory::AddItem` now applies the active structure flag to supported upgrades **before** indexing its inventory flag, preventing stale flag-map entries; unrelated IHub cargo is not activated.

**No additional SQL migration is required for this specific support fix.** Use the newer server binary from this PR, retain the previous sovereignty index migrations, relog/reopen the management window, then drag the Supercapital Construction Facilities upgrade item into the registered IHub inventory. The server should no longer emit `not currently supported` for type 2009. Check the upgrade item has `flagStructureActive` (and remains after server restart). A functioning capital ship assembly facility, construction job service, fuel/upkeep or client UI lock decoration is **not** proven by this validator change. If it remains visually locked, capture the exact window and logs: UI gating may have separate client-side prerequisites.

New `IHubUpgradeRulesTest` CTest checks 99 vs 100 strategic days, known supported upgrade types and an unsupported type. It does not simulate a full game client inventory transfer; test that manually.
