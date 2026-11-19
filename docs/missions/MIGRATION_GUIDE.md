# Mission data restoration and migration guide

## Purpose

Restore the missing EVEmu mission catalogue from traceable EVE Online data, with priority on Level 1-4 security/encounter missions. This project must not fabricate mission definitions or mutate a live server database during data acquisition.

The current database investigation found 2,673 `agtMissions` rows with zero `rewardISK` and zero `dungeonID`; the 16 encounter-type rows are named `Cargo Delivery`. The current loader in `src/eve-server/missions/MissionDB.cpp` also filters `agtMissions` on `rewardISK > 0`. Removing that filter alone would load incomplete records and is not a valid repair.

## Version compatibility gate

The current CCP SDE is not automatically a safe import target for this older Crucible codebase. Before importing any current SDE mission or NPC type, compare the SDE build/era against the repository's bundled static data and the target database's existing type/faction IDs. Prefer an archived SDE matching the static-data era of this fork. Current 2026 mission guides are useful for discovering variants and mechanics, but must be version-tagged and must not silently overwrite legacy-compatible data. Any type ID absent from this fork's `invTypes` must be rejected or explicitly mapped; never create guessed type rows.

## Data-source hierarchy

1. **CCP Static Data Export (SDE)** — canonical source for mission IDs, mission kind, localized briefing/objective text, static reward fields, item/type IDs, NPC ship types, groups, factions and dungeon metadata where present. Official documentation: https://developers.eveonline.com/docs/services/static-data/
2. **EVE Info SDE Missions** — searchable index of official SDE mission briefings and rewards; useful for cross-checking parsed SDE output: https://eveinfo.com/sde-missions/
3. **EVE University Mission Reports** — community-maintained mission level, enemy faction and mission variant index: https://wiki.eveuniversity.org/Mission_reports
4. **EVE University individual mission reports and guides** — objective details, enemy composition, damage types and practical mission notes. Use page-specific links and retain attribution.
5. **EVE-Survival Mission Reports** — additional historical wave/trigger walkthroughs and faction variants: https://eve-survival.org/?wakka=MissionReports
6. **NPC reference databases** — map NPC names to actual type IDs and combat attributes; for example https://gamingwithdaopa.ellatha.com/eveonline/npc-list/
7. **Existing EVEmu static data and code** — final compatibility authority for this fork's schema, type IDs, spawn classes, dungeon structures and mission runtime behavior.

CCP's SDE is a snapshot of static data. It does not provide every live runtime detail. Community guides are corroborating evidence, not canonical authority for CCP's current behavior. Historical loot reports must not be presented as guaranteed drop tables.

## Data model and provenance requirements

Every imported mission/variant must retain:
- stable internal key and source mission ID(s)
- mission display name and variant name
- mission type, level, important/storyline flags, faction/race, and agent eligibility where known
- briefing/objective source reference and language
- reward ISK, bonus ISK, bonus time, reward item and quantity, LP/standing fields where available
- dungeon/template ID and room mapping
- ordered stages/waves, trigger conditions, spawn group, NPC type ID, quantity and 3D placement
- completion conditions, required item/cargo and destination where applicable
- loot/salvage observations separately from guaranteed mission rewards
- source URL, retrieval date, source revision/build, confidence and notes for each non-SDE field

Do not collapse faction variants into one record when their NPC types or spawn layouts differ. Do not invent exact NPC type IDs, coordinates, triggers, reward amounts, or loot probabilities. Unknown values remain NULL/unknown, not zero unless zero is confirmed by source semantics.

## Import and migration workflow

1. Download and pin one CCP SDE build. Record its build number and archive checksum in the dataset manifest. Prefer JSONL for automated ingestion; CCP documents both JSONL and YAML.
2. Extract the SDE mission definitions and related type/faction/dungeon references into a staging format. Keep source identifiers intact.
3. Build a cross-reference against the fork's existing `agtMissions`, `dunTemplates`, `dunRoomObjects`, `invTypes`, `invGroups`, `facFactions` and existing quest tables. Treat name matches as candidates only; IDs and semantics must be verified.
4. Enrich mission variants from community mission indexes and individual reports. Store each wave/trigger/loot observation with its own provenance and confidence.
5. Validate all foreign keys, type/category consistency, reward ranges, wave references, room references, and duplicate mission variants before generating SQL.
6. Generate a **new, additive, idempotent migration** for the target EVEmu schema. Use explicit stable keys and guarded inserts/upserts. Do not truncate, drop, reset, or replace live character, account, wallet, inventory, standings, offer, or migration-history data.
7. Generate a dry-run report showing row counts and differences before any SQL is applied. The operator must review and approve the migration.
8. Test on a disposable copy of the same schema first. Verify server loading logs and mission-offer eligibility, then accept/complete a mission and verify wave progression, NPC death, rewards and cleanup.
9. Only after tests pass, provide the migration and rollback plan for the live server. No SQL is run against the user's live database by this project without explicit approval.

## Compatibility rules for this EVEmu fork

- Keep existing mission runtime fields compatible with `MissionData` and `MissionOffer`.
- The current loader's `rewardISK > 0` condition must not be used as the only completeness test. A mission can have a valid zero/variable base reward or a reward calculated at runtime; eligibility should instead require a valid mission definition and supported runtime fields.
- Encounter mission templates must reference real compatible dungeon/template data. Never assign an arbitrary dungeon ID to make a row load.
- NPC spawn types must resolve to valid ship/drone categories in this fork's `invGroups` and `invTypes`.
- Bounties, mission payout, bonus payout, LP, loot drops and salvage are distinct systems. Do not duplicate bounty/loot into mission reward columns.
- The current dungeon-template C++ loader is a runtime bridge for NPC room objects; it does not create missing mission definitions or validate the complete mission catalogue.

## Licensing and attribution

Use official SDE data in accordance with the current CCP Developer License Agreement and retain the required EVE/CCP attribution. For third-party guides, store factual observations and source links/attribution; do not bulk-copy copyrighted walkthrough prose or images into the repository. Confirm each source's terms before redistributing any extracted dataset.

## Current scope and status

This is the data-engineering foundation only. No SQL migration in this folder is intended to be applied yet. The next deliverable is a reproducible SDE importer plus a normalized staging catalogue, followed by Level 1 security mission enrichment and validation. Level 2-4 expansion follows the same pipeline.
