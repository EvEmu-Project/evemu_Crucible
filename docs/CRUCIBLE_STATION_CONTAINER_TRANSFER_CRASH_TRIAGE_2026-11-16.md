## Confirmed underlying source bug — assembled containers inherited POS-only operations

A subsequent live test reported that **both** placing items into assembled
containers and **renaming** the container terminated the Windows server, even
though run #43844019748 built cleanly on all platforms. Inspection of
`src/eve-server/pos/Structure.cpp` established the shared failure mechanism:

- `CargoContainer` derives from `StructureItem`. Its `AddItem()` inherited
  `StructureItem::AddItem()`, which unconditionally executed
  `mySE->IsCOSE()` after the inventory insertion.
- Its inherited `Rename()` unconditionally executed
  `mySE->GetPOSSE()->GetState()` before applying any name change.
- `StructureItem::StructureItem()` had not initialized its `mySE` pointer.
  Station-hangar containers have no in-space system entity, so *both* calls
  dereferenced an uninitialized/absent pointer (undefined behaviour, likely
  access violation), exactly matching the two reproducible user actions.
- The derived `CargoContainer` constructor redundantly allocated
  `pInventory` a second time despite `StructureItem` already allocating it,
  leaking the first `Inventory` reference.

**Fixes:** initialize the base POS entity pointer to `nullptr`; require
a valid customs-office entity for the POS-specific verification in
`StructureItem::AddItem()`; reject missing entities gracefully in
`StructureItem::Rename()`; override `CargoContainer::Rename()` to use
`InventoryItem::Rename()` for normal assembled cargo containers (while
retaining the unanchored-in-space restriction); eliminate the redundant
inventory allocation. Extend the CI source invariants to check these conditions.

This is a **directly verifiable source cause**, not a runtime-proven resolution:
Windows CI and manual tests of drag-in, drag-out, rename, and persistence after
restart are still required. The previous source hardening below remains useful.

---

# Crucible assembled container crash: local station transfer hardening
Date: 16 November 2026

## Operator evidence
The player can now enter Maurasi station without client exit. Placing any item
into an assembled cargo container reportedly terminates the server process.
The uploaded 209-line server log shows login at 11:40:13, system initialization
at 11:40:24, sixteen repeated 12-frame native traces of unsymbolized addresses,
then normal station RPCs and JPEG image serving through 11:40:28. No crash
exception code or symbolized container-transfer stack is present. The traces
precede the last visible normal log entries; do not assume they are the fatal
exception site.

## Source defects found and corrected
- CargoContainer::_LoadItem rejected the Mission_Container group even though
  InventoryItem::_LoadItem dispatched it there. Rejections invoked the configured
  stack trace and failed item loading; the dispatch and acceptance rules now match.
- Inventory::ValidateAddItem dereferenced the source container returned by
  GetItemRef without checking null. Moving a stack into a station container
  may encounter an unloaded/missing source location; the code now safely skips
  that dereference. In-space range checks refuse requests lacking an active ship.
  Direct self-insertion is rejected.
- InventoryBound::Add accessed an absent source item; MoveItems assumed an
  existing destination inventory and ignored ValidateAddItem's false result.
  Guard those cases and refuse rejected moves.
- ItemFactory::GetItemContainerRef / GetItemContainerInventory dereferenced a
  map end iterator if the parent item was not cached. Both use a checked parent
  lookup respecting the existing load flag.
- Inventory::AddItem examined an uninitialized std::pair when an item was
  already present, and could duplicate entries in m_contentsByFlag. Emplace
  status now initialized and duplicates skipped.
- InventoryBound::CatSortItems indexed itemVec.at(0) for an empty list or
  dereferenced null item refs. Skip nulls and return an empty list safely.
- InvBrokerBound inventory binding now refuses targets that do not own an
  Inventory instead of storing a null inventory in an InventoryBound.
- CargoContainer::RemoveItem could delete an assembled container in the
  station/ship hangar when its last contained item was removed. Keep the
  existing in-space empty-container behavior, but don't auto-delete storage
  containers outside a solar-system location.

The original server logic for CCP/Crucible item content, other station operations
and persistence remains intact. No SQL migration, reseed, item deletion or
changes to images/agents/combat are included in this focused transfer update.

## Targeted logging
When a Celestial-category inventory in a station receives a transfer,
the server emits:
```
StationContainerTransfer: BEGIN item=<id> source=<id> destination=<id> flag=<n> qty=<n>.
StationContainerTransfer: END item=<id> nowIn=<id> expected=<id>.
```
BEGIN without END narrows the execution window; this is not a symbolized
exception trace. If the process still exits, capture the Windows Application
Error exception code, faulting module, a dump with thread stacks and relevant
new server log lines. Do not assume the old 16 repeated traces prove the
container-transfer cause.

## Acceptance test
1. Back up SQL database and old Windows executable/DLLs. Install the matching
   new PR #25 Windows build to a clean folder; confirm the running exe path.
2. Login to the same Maurasi station. Do not delete or replace existing containers.
3. Open a player-owned, assembled and empty container in a personal hangar.
   Move exactly one low-value test item into it.
4. Confirm the server stays running, the UI shows the item in the container,
   and the BEGIN / END log reflects the same destination ID.
5. Restart the server and verify the item remains in that container.
6. Move the test item out. Confirm the assembled empty container remains in
   the hangar; restart again to check persistence.
7. Verify moving a stacked item (split), an invalid item ID, and a request
   that exceeds capacity are handled without a process crash. Do not attempt
   unknown corrupt/duplicate inventory records on production data.
8. If a crash persists, attach the updated full log and Windows WER fault
   module + exception code or a minidump. The screenshot/log alone cannot
   identify the unsymbolized instruction address.

## CI
The existing `tools/verify_station_crash_safety.py` has new source guards
for these non-null and file-backed inventory safety properties. The CI build
and live-client test remain necessary; source guards cannot prove runtime safety.
