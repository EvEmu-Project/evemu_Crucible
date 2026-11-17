-- EVEmu Crucible — non-destructive container, inventory and POS consistency audit.
-- 2026-11-16
-- Run manually against a BACKUP of the EvE database first, or on the live
-- database in a read-only SQL session. This file NEVER updates or deletes data.
-- Investigate candidates individually; static map locations, virtual items,
-- courier/PI structures and temporary IDs can legitimately lack entity rows.
--
-- 1. Items referring to themselves. These can recurse during inventory load.
SELECT itemID, itemName, typeID, ownerID, locationID, flag, quantity, singleton
FROM entity
WHERE itemID = locationID
LIMIT 100;

-- 2. Two-item containment cycles.
SELECT a.itemID AS itemA, a.itemName AS nameA,
       b.itemID AS itemB, b.itemName AS nameB
FROM entity AS a
JOIN entity AS b ON b.itemID = a.locationID
WHERE b.locationID = a.itemID AND a.itemID < b.itemID
LIMIT 100;

-- 3. Three-item containment cycles.
SELECT a.itemID AS itemA, b.itemID AS itemB, c.itemID AS itemC
FROM entity AS a
JOIN entity AS b ON b.itemID = a.locationID
JOIN entity AS c ON c.itemID = b.locationID
WHERE c.locationID = a.itemID
  AND a.itemID < b.itemID AND a.itemID < c.itemID
LIMIT 100;

-- 4. Heuristic only: items at high-valued nonstatic locations with no matching
--    entity/station/character/office/system. Values outside the represented
--    location tables may still be valid; DO NOT delete these automatically.
SELECT child.itemID, child.itemName, child.typeID, child.ownerID,
       child.locationID, child.flag, child.quantity
FROM entity AS child
LEFT JOIN entity AS parent ON parent.itemID = child.locationID
LEFT JOIN staStations AS station ON station.stationID = child.locationID
LEFT JOIN chrCharacters AS character_row ON character_row.characterID = child.locationID
LEFT JOIN staOffices AS office_row ON office_row.itemID = child.locationID
LEFT JOIN mapSolarSystems AS solarsystem ON solarsystem.solarSystemID = child.locationID
WHERE child.locationID >= 100000000
  AND parent.itemID IS NULL
  AND station.stationID IS NULL
  AND character_row.characterID IS NULL
  AND office_row.itemID IS NULL
  AND solarsystem.solarSystemID IS NULL
LIMIT 200;

-- 5. POS metadata with no dynamic item; investigate before modifying it.
SELECT p.itemID, p.towerID, p.anchorpointID, p.state, p.status
FROM posStructureData AS p
LEFT JOIN entity AS e ON e.itemID = p.itemID
WHERE e.itemID IS NULL
LIMIT 100;

-- 6. POS modules whose tower is missing from both dynamic item and POS metadata.
--    Tower references of 0 and older migrations are not necessarily an error.
SELECT p.itemID, p.towerID, p.anchorpointID, p.state
FROM posStructureData AS p
LEFT JOIN entity AS tower_item ON tower_item.itemID = p.towerID
LEFT JOIN posStructureData AS tower_pos ON tower_pos.itemID = p.towerID
WHERE p.towerID > 0 AND tower_item.itemID IS NULL
  AND tower_pos.itemID IS NULL
LIMIT 100;

-- 7. Possible stale dynamic attribute rows for deleted items.
SELECT a.itemID, COUNT(*) AS attribute_count
FROM entity_attributes AS a
LEFT JOIN entity AS e ON e.itemID = a.itemID
WHERE e.itemID IS NULL
GROUP BY a.itemID
ORDER BY attribute_count DESC
LIMIT 100;

-- 8. Measure the existing index strategy before considering a migration.
--    InventoryDB::GetItemContents filters on locationID and orders by itemID.
SHOW INDEX FROM entity;
EXPLAIN SELECT itemID FROM entity WHERE locationID = 0 ORDER BY itemID;
-- Replace 0 with the affected container itemID to get representative estimates.
-- A composite (locationID, itemID) index may help large inventories, but
-- DO NOT add a duplicate index or alter schema without verifying the actual
-- table definition, indexes, and EXPLAIN plan first.
