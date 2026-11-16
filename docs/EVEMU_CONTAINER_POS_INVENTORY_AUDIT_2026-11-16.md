# EVEmu Crucible — Container, POS and inventory integrity audit

Date: 16 November 2026  
Branch: `fix/crucible-station-agent-50px-http-and-inventory-guards-2026-11-16`  
Scope: inventory RPCs, containers, structures, POS initialization and control,
item movement, stack handling, item factory, SQL persistence and existing schema.

## Scope and caveats

The GitHub tree was enumerated (1,241 tracked files), and the full inventory,
container and POS source files plus relevant SQL queries and initialization
paths were examined. This is **not** an independently executed proof of every
code path in the repository. In particular, real-world access permissions,
orbital structures, corporate POS permissions, destruction/insurance, ship
fitting, and replication under load need separate integration tests.

The previous Windows client report confirmed that station entry now works but
assembled-container move and rename could terminate the server. Earlier
commits fixed a shared `StructureItem::mySE` dereference on both actions. This
follow-up audits adjacent failure modes and persistence safety.

## Changes made

### Inventory item integrity

- `InventoryItem::Merge` rejects self-merge, invalid source quantity and
  destination stack overflow **before** decrementing/deleting the source.
- `AlterQuantity` uses a 64-bit intermediate and checks the `int32` range;
  `SetQuantity` rejects negative counts.
- `InventoryItem::Delete` does not dereference a parent whose inventory
  cannot be loaded.
- `ToVirtual` removes the item from its parent inventory instead of
  mistakenly removing it from its own cargo contents.
- Rename notifications tolerate temporary absence of the owner's ship entity
  while docking and release undeliverable payloads.

### Container and item transport

- `Inventory::WouldCreateContainmentCycle` uses a bounded, visited ancestry
  walk to reject cached parent/child cycles. `InventoryBound::MoveItems`
  rejects such moves **before** changing stacks, flags or ownership.
- POS/corporate destination moves now check capacity before donating ownership.
- Customs-office inventory transfers now call `InventoryItem::Move`, which
  updates the item location in the database and invokes the destination
  inventory hook. The previous code added the item to the in-memory map only.
- `MultiMerge` rejects malformed payloads and invalid source quantities;
  ship destinations are verified before fitting/moving items.
- Transfer functions now use `ScopedInventoryClient` so exception and early
  return paths always clear ItemFactory's global `usingClient` context.

### POS system lifecycle

- POS RPC handlers use checked system-entity lookups, rather than chaining
  through possibly absent towers/structures.
- `StructureSE::InitData` reports failure if the required moon, gate,
  planet, tower, or system manager is unavailable. The caller avoids saving
  incomplete POS metadata.
- Tower initialization honours base initialization failures, and tower
  scoop/offline operations avoid dereferencing absent moon/shield entities.
- Sovereignty-dependent POS anchoring refuses a missing infrastructure hub
  or upgrade type instead of dereferencing it.
- POS anchor routines check required moon, gate and planet entities.
- Existing cargo-container `mySE` null checks and separate station rename
  override remain intact.

### SQL persistence

- Bulk saves in `ItemDB::SaveItems` now use the same SQL string-escaping
  helper as individual item saves, so `customInfo` containing quotes cannot
  break the whole batch.
- A read-only SQL integrity check is included:
  `sql/diagnostics/2026-11-16-container-pos-integrity.sql`.
  It lists direct/two-level/three-level containment cycles, suspicious
  parent locations, unmatched POS metadata and attribute rows, and existing
  entity indexes.
- **No migration** or destructive database alteration is included.
  Do not attempt to automatically delete or repair results: historical/static
  locations, virtual items and transient references can legitimately differ
  from normal dynamic `entity` items. Review any candidate IDs against your
  server log and backup before modifying records.

## Test instructions

1. Back up the live SQL database and the current executable/configuration.
2. Obtain the Windows artifact for the **final** PR head build, not an older
   green run. Confirm the commit SHA in the Actions build before replacing
   binaries. Keep the matching UCRT64 DLLs.
3. In Maurasi station (system 30000140), rename a player-owned assembled
   container, put a single low-value item into it, take it out, and verify
   the container is still present.
4. Restart the server; check persistence of the container name, location and
   child contents. Repeat for a modest stack split.
5. Try placing a container into itself or its own descendant; the move should
   be rejected without moving either item.
6. Test POS inventory transfer and return with a safe test structure and
   disposable cargo. Confirm capacity rejections do not change ownership.
7. Test a customs-office transfer and a restart; the item must have the
   destination location ID in the database.
8. If POS initialization reports a missing tower/moon/gate, review the system
   entity list and `posStructureData` before changing anything.
9. Run the read-only SQL audit **on a database backup first**; inspect any
   candidates and `SHOW INDEX FROM entity` output. Add a DB index only
   after measuring a real bottleneck and confirming no equivalent index exists.
10. If the process still terminates, attach its new log, faulting module,
    Windows exception code and a symbolized stack/minidump when available.

The existing `StationContainerTransfer: BEGIN` and `END` warnings should
bracket successful in-station moves. An earlier logged stack trace without a
faulting instruction is not sufficient to prove a particular crash cause.

## Unresolved / intentionally out of scope

- A complete authorization and roles model for every corporate POS division
  is not implemented by this patch. Do not deploy on a public/hostile server
  without reviewing roles and item-owner access paths.
- The cycle guard inspects cached parent items. It does not repair previously
  corrupted SQL graphs or prove the absence of cycles in unloaded ancestors.
- Several legacy POS and orbital services contain stubs. This patch preserves
  their existing behaviour rather than fabricating unsupported mechanics.
- No throughput or SQL-index improvement is claimed without workload metrics.
- CI checks are compile/unit/source checks, not a substitute for running
  Crucible 1.6.5 against this Windows build.
