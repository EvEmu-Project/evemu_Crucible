# Sovereignty and POS source review — 17 November 2026

Repository: xrizonx/evemu_Crucible. Branch: fix/sovereignty-pos-integrity-2026-11-17. Based on the fork master after PR #38. This is a source patch in a review branch, not a completed playtest or upstream issue closure. No live SQL was modified.

## Source-level fixes and issue IDs

| Internal ID | Subsystem | Root defect and correction | Evidence and verification |
| --- | --- | --- | --- |
| SOV-01 | Claim creation | SovereigntyDB inserted FILETIME as %f (double) even though the 2021 migration declares claimTime BIGINT. This can read the wrong varargs type, corrupt timestamps and reset claim age. It now writes the actual 64-bit supplied claim timestamp with %lli, falling back to current FILETIME only if none is given. | SovereigntyDB.cpp; SovereigntyPosIntegrityTest; SQL query 1 |
| SOV-02 | Claim cache | Failed INSERT, DELETE or subsequent SELECT could erase valid in-memory claims. DB results now report success; cache replacement is delayed until read succeeds. | SovereigntyDB.cpp and SovereigntyDataMgr.cpp; simulate disconnected DB |
| SOV-03 | SBU blockade | Counting every online SBU let multiple units at one gate count as blockade of different gates. Count distinct valid gate IDs instead. | SBU.cpp; CTest duplicates and zero IDs |
| POS-01 | Silo capacity | GetSiloCapacityByItemID queried type 0 for all structures. Resolve in-system POS entity and use its actual type. | PosMgr.cpp; deployed silo UI |
| POS-02 | Tower settings | Config/password/sentry RPCs looked up tower ID but did not verify owning corporation, StarbaseConfig/Director roles, current system or explicit server ADMIN. Central guard now protects five tower mutation paths. | PosMgr.cpp; two-corporation negative tests |
| POS-03 | Anchor | POS anchor accepted foreign-owned in-system structures. Require configured corporation role or personal item ownership, valid system and finite coordinates. | PosMgr.cpp; unauthorized anchor tests |
| POS-04 | Permission payload | Missing or wrongly typed deploy/use flag rows could crash and partially apply tower settings. Validate both complete payloads before mutation. | PosMgr.cpp; malformed packet test |
| POS-05 | Sentry SQL | UpdateSentry persisted standing but not status, though GetTowerData reads status on reboot. Persist both. | PosMgrDB.cpp; reboot persistence |
| POS-06 | Deploy flags SQL | Integer anchor flag was passed to %f formatter. Use %i. | PosMgrDB.cpp; reboot persistence |
| POS-07 | Deployed towers | GetCorpControlTowers used LEFT JOIN for posTowerData so towers held in cargo could appear as deployed. Require existing deployed tower data row. | PosMgrDB.cpp; corporation UI |
| POS-08 | Bridge unlink | MariaDB prohibits UPDATE with scalar subquery from the same target table in this form. Use a joined update, updating only a reciprocal remote endpoint. | PosMgrDB.cpp; link/unlink in MariaDB |
| POS-09 | Bridge link/unlink | Callers could specify arbitrary bridge IDs and two databases rows would change without owner, alliance and occupied-link validation. Check current system, local permission, both persisted endpoints, alliance match, reciprocal/unlinked status before editing. | PosMgr.cpp; cross-corp link tests |

| POS-10 | Structure placement limits | SBU and orbital platform checked maximum/minimum anchor distance using the structure original stored position rather than the new client-requested target, allowing an out-of-range placement. They now validate distance to the requested position. | Structure.cpp; source invariant and in-game boundary check |

## Existing fixes preserved

The fork already had TCU claim ownership validation, NPC sovereign system rejection, SBU zero-gate protection, infrastructure hub claim checks, missing POS initialization guards and security-specific belt/NPC fixes. No duplicate code changes or rollback of those existing fixes were made. See docs/CRUCIBLE_SOV_SECURITY_BELTS_RATS_AUDIT_2026-11-16.md.

## Tests and SQL review

The new C++ SovereigntyPosIntegrityTest checks FILETIME preservation, permitted and denied POS owner/role combinations, and distinct online SBU gate counting. Windows and Linux CI run the source invariant script tools/verify_sov_pos_integrity.py before CTest. Use sql/diagnostics/2026-11-17-sov-pos-readonly-audit.sql against a backup or read-only replica; no cleanup statements are supplied.

Required in-game checks: create an alliance-owned TCU in test null-sec, restart twice and verify stable claimID and claimTime; use SBUs on distinct versus duplicate stargates and confirm contested threshold and invulnerability transitions; test tower password, notifications, sentry settings and access permissions with Director, StarbaseConfig, ordinary member and foreign-corporation accounts; verify personal container anchoring and true silo capacity. Deploy/link/unlink same-alliance jump bridges and reject cross-alliance, occupied or wrong-system links. After restarting, verify sentry status and deploy flags persisted and corporation tower list contains only deployed towers.

## Follow-up: two POS metadata rows with no inventory entity (18 November 2026)

The operator's read-only query reports zero invalid sovereignty claims, missing TCU inventory entities, orphaned tower rows and broken jump bridge links, but **two posStructureData rows with no corresponding entity row**. This does not prove the rows can be deleted safely: their tower references, subordinate modules and historical state must first be inspected.

Source audit found a reproducible lifecycle gap: `StructureItem::Delete()` called `InventoryItem::Delete()` without the `PosMgrDB::DeleteData()` cleanup that `StructureSE::Scoop()` already uses. Permanent destruction now removes own POS metadata, and `PosMgrDB::DeleteData()` first clears a reciprocal remote jump bridge reference. This prevents one plausible future orphan source but **does not alter the two existing database rows**.

Run the new **POS-11 detailed orphan inspection** in `sql/diagnostics/2026-11-17-sov-pos-readonly-audit.sql` to obtain each itemID, towerID, anchorpoint, state, subordinate structure count and related configuration rows. Preserve a backup; reconcile any dependent structures before deciding whether manual cleanup is justified. No automatic UPDATE or DELETE is supplied.

## Confirmed operator database evidence — 18 November 2026

The operator checked the `eve` MariaDB database. Four issue counts were zero: invalid sovereignty claims, missing TCU entities, orphan control towers and broken bridge links. The POS metadata diagnostic reported **two orphaned `posStructureData` entries**:

| itemID | anchorpointID | towerID | state | status | timestamp | tower/bridge rows | child structures | attribute rows |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `140003364` | `40177245` | 0 | 0 | 0 | 0 | 0 / 0 | 0 | 0 |
| `140003384` | `40177217` | 0 | 0 | 0 | 0 | 0 / 0 | 0 | 0 |

The operator subsequently ran the POS-12 checks and confirmed **zero contained inventory items, zero customs office rows, zero incoming jump bridge links, and zero sovereignty references for each itemID**. Combined with the previous two zero-dependent-structure, zero-attribute, zero-tower and zero-outgoing-link checks, these exact two unanchored metadata rows are strong candidates for **manual cleanup after a verified full database backup and a separate two-row snapshot**. The original deletion event has not been proven. **No live SQL mutation or automatic deletion was performed.** Only the two verified IDs should be considered for targeted archival/deletion while the server is stopped, with restrictive state/anchorpoint predicates and transaction row-count verification. The source-level deletion fix is preventative and does not remove historical records.

## Known unresolved issues and rollout cautions

The bridge connection still performs two SQL UPDATE operations without a transaction. If the second fails, a partial link can remain; use the read-only diagnostics before an operator repairs it. Comprehensive tower fuel simulation, moon mining/reactors, reinforcement cycle/strontium consumption, POS weapons, sovereignty notifications and outpost construction were not proven correct by these edits and remain separate investigations. The source role check intentionally restricts POS mutation to Director/StarbaseConfig or server ADMIN; test legitimate role grants and corp sessions before shipping. Compiling successfully is not proof of persistence, protection against all crafted network packets or gameplay compatibility. PR remains draft until CI and live validation.
