# Mission staging and test migration guide

## Safety boundary

This migration adds only the isolated `rmrMissionStage*` tables. It does not modify or truncate `agtMissions`, `agtOffers`, `dun*`, `invTypes`, character/account tables, wallet/inventory data, or migration history. Do not apply this to the live database initially. Install it on a disposable clone or test database made from the same Crucible build.

No SQL is executed by this project. The operator runs the reviewed file manually on the test copy.

## Files

- `sql/mission_data/001_create_staging_schema.sql`: additive staging DDL; safe to re-run for tables that already exist.
- `sql/mission_data/003_upgrade_existing_staging_schema.sql`: additive column upgrade for staging tables created by an earlier version; adds only missing columns and skips columns already present.
- `sql/mission_data/002_seed_research_case.sql`: optional, non-executable The Blockade research placeholder with source links; it contains no guessed NPC/type/room/reward rows.
- `docs/missions/STAGING_VALIDATION.sql`: read-only compatibility and completeness checks.
- `sql/mission_data/source_manifest.json`: source policy and provenance.
- `sql/mission_data/research/`: research observations; not runtime import data.

## Why staging comes first

The existing Crucible database has 2,673 `agtMissions` rows whose `rewardISK` and `dungeonID` fields were reported as zero. The runtime loader filters on `rewardISK > 0`. Removing that filter would expose incomplete records, not restore valid security missions.

The repository's dungeon runtime path also depends on this fork's own dungeon/template and room-object structures. A modern EVE mission guide or current SDE type ID is not proof that the NPC, item, room or spawn is available in this Crucible database.

## Installation or upgrade on a test copy

1. Back up the disposable test database.
2. Confirm the test database was created from the same Crucible source revision and base SQL as the server binary.
3. Apply `001_create_staging_schema.sql` first. Its `CREATE TABLE IF NOT EXISTS` statements create missing staging tables and leave existing tables/data intact.
4. If these staging tables already existed from an earlier schema revision, apply `003_upgrade_existing_staging_schema.sql` next. It checks `information_schema.COLUMNS`, adds only missing non-key columns and skips existing columns. It does not drop or rebuild tables. Existing placeholder/default values are retained and must be reviewed; the migration does not infer real mission data.
5. Verify all five staging tables exist and inspect their columns using `SHOW TABLES LIKE 'rmrMissionStage%';` and `SHOW COLUMNS FROM <table_name>;`.
6. Do not run any production data migration at this stage. Load research records only after each field has a source URL, source revision/date, confidence and compatibility status.

**Important:** `003` assumes the five staging tables and their primary auto-increment ID columns were created by `001`. It upgrades missing data columns, not arbitrary third-party schemas. If a table has a different primary key or incompatible column type, stop and inspect it before applying `003`; do not force an automatic conversion.

## Data entry rules

- Keep CCP/SDE source IDs separate from Crucible target IDs. Use `sourceMissionID` and `targetMissionID` separately.
- Never rewrite `invTypes` or create guessed item/NPC types to satisfy a mission. If a source NPC/item has no Crucible match, leave it unmapped and mark the mission `blocked`.
- Spawn rows require a real Crucible `typeID` and positive quantity. Spawn types must resolve to `invTypes` and `invGroups` category 6 (ship) or 18 (drone).
- Room/template references must resolve to the Crucible `dunTemplates` data. Coordinates and triggers remain NULL/unverified until supported by a compatible source or a controlled in-game test.
- Keep NPC bounty separate from mission reward. Keep mission reward items separate from NPC drops and salvage.
- A guide's possible drop is not a guaranteed drop. Leave chance NULL unless a reliable source provides it.
- Keep faction variants separate when NPC types, waves, triggers or room layouts differ.
- A zero reward is valid only when the source semantics establish it. Unknown reward values remain NULL, not zero.

## Validation and promotion gate

Run the read-only queries in `STAGING_VALIDATION.sql` on the test copy. Any missing type ID, invalid spawn category, missing room template, zero quantity or approved record with unresolved references blocks promotion. Also manually review mission level, faction, agent eligibility, mission completion conditions, wave triggers, reward math, LP/standing behavior, loot and salvage.

Only after all checks pass should a separate generator emit a target-specific migration into the runtime schema. That migration must:
- be additive and idempotent;
- preserve existing mission/offer IDs unless an explicit collision-safe mapping is reviewed;
- reference only target-build IDs;
- include a dry-run row-count/diff report;
- include rollback limited to the newly inserted mission data;
- never delete or reset player/account data.

Do not promote the research sample The Blockade until its NPC type IDs, compatible room/template, spawn quantities, triggers, reward semantics and item drops have been mapped and validated against this exact Crucible build.

## Test procedure after promotion

1. Start the test server and verify mission loader counts and warnings.
2. Verify a security agent offers the intended mission at the correct level/faction.
3. Accept the mission and verify the correct dungeon/room loads.
4. Verify every NPC type and quantity, then test each wave trigger and completion condition.
5. Verify NPC bounties, wrecks, loot, salvage and mission reward separately.
6. Complete and turn in the mission; verify ISK, LP, standings and offer cleanup.
7. Restart the server and confirm mission/offer persistence.
8. Keep logs, SQL diff, source manifest and test result with the dataset revision.

## Version compatibility

CCP's 2025 SDE redesign is explicitly not backwards-compatible with older SDE consumers. For this Crucible fork, use the legacy static-data generation that matches the bundled Crucible item/faction/dungeon identifiers. The archived Crucible 1.6 static-data release from March 2012 is a useful historical reference, but the fork's current bundled database is the final compatibility authority. Do not import 2012 or 2026 IDs blindly.

## Current status

This is the first SQL-level implementation: isolated staging DDL and read-only validation queries. It is not a populated runtime mission pack. The first populated encounter pack will be built only after the Crucible type/room cross-reference is complete. No SQL has been run against the live database.
