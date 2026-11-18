# Instant Level 5 Military, Industry and Strategic sovereignty testing

**Fork:** xrizonx/evemu_Crucible • **Branch:** feat/sov-index-level5-gm-test-2026-11-18 • **Date:** 18 November 2026

## Audit

The existing `/sov` handler is a nonfunctional logging stub. `SovereigntyDB` returns hard-coded **5** for `militaryPoints` and `industrialPoints` in every claimed system; there is no NPC/mining activity progression implemented. Strategic index uses elapsed FILETIME claim age. The previous `GetAllDevelopmentIndices` service was a stub returning null. This patch implements that list response and the developer-only persistent test override, **not** a complete activity-driven Sovereignty Index feature.

Crucible/Dominion-era Strategic Level 5 requires **100 days** of unbroken claim ownership. Instead of modifying `mapSystemSovInfo.claimTime`, the command displays 100 simulated strategic days for testing, tied to the **current TCU itemID**. Real claims and upgrade inventories remain untouched.

## Install the SQL migration before using the new command

After making a backup of the `eve` database and ensuring no other migrations conflict, apply:

`sql/migrations/20261118070000-sov-development-index-overrides.sql` **Up** section, or let your normal migration runner apply it exactly once. It creates a new `mapSovDevelopmentIndexOverrides` table; no existing player / alliance / POS rows are altered. The new server can start without the table (it logs that test overrides are unavailable), but the new command will fail until the migration is applied. Never run the Down section just to reset a system.

## In-game usage

1. Update and restart `eve-server` with the build from this PR.
2. Log in with a **DEV** (`0x62f8000280c41000`) or **ADMIN** role. Normal corp members, CONTENT-only accounts, and VIPs cannot use this command.
3. Travel to an **alliance-claimed, player-claimable null-sec** solar system with an online, linked Territorial Claim Unit and registered Infrastructure Hub. The override affects only your **current system**, not a named system or entire region.
4. In chat, execute `/sovindex5`. The server writes **Military 5, Industry 5 and Strategic 100 effective days** to `mapSovDevelopmentIndexOverrides`, keyed to the current TCU. The real `claimTime` remains unchanged.
5. Reopen the Sovereignty information and IHub management panes to refresh cached index values. Test installing an upgrade that requires Level 5; note the actual client UI and any server errors.
6. Execute `/sovindex5 reset` to delete only the current TCU override, restoring the normal source values. If the system changes sov holder, the old TCU marker cannot activate for the new claim even if the stale table row remains. On ordinary claim removal, the server also attempts to delete the previous claim\x27s test override so re-onlining the same TCU does not resurrect development test levels.

The override persists across server restarts and indexes are shared through `GetCurrentSovData`, `GetSystemSovereignty`, `GetDevelopmentIndicesForSystem` and implemented `GetAllDevelopmentIndices`.

**Limitations:** Military/Industrial `5` are existing emulator point values and have not been validated against all 2012 client UI index thresholds. This command does **not** implement infrastructure upgrade installation, activation/fuel/upkeep checks, NPC anomaly spawning from installed upgrades, or full CCP sovereignty rules. A command success message means the override was stored, not that end-to-end IHub installation worked. Verify with the Crucible client and report any errors/screenshots. The GM override must never grant sovereignty in an unclaimed or NPC-faction system.

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
