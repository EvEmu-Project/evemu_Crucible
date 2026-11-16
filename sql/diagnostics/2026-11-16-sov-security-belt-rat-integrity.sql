-- EVEmu Crucible — READ-ONLY security, belt-ore, NPC-faction and sovereignty diagnostics.
-- Run on a backup or in a read-only database session.
-- No UPDATE/INSERT/DELETE, migrations, table changes or automatic repair.
--
-- BeltMgr selects ore with roidDistribution.systemSec matching
-- mapSolarSystems.securityClass, not by a hand-built security-number cutoff.
-- "percent" may be fractional or percentage-scale; ratios matter, not
-- whether individual class totals are precisely 1 or precisely 100.

-- 1. All security classes in use and the weight-list size for each class.
SELECT mss.securityClass,
       COUNT(*) AS system_count,
       MIN(mss.security) AS min_truesec,
       MAX(mss.security) AS max_truesec,
       (SELECT COUNT(*) FROM roidDistribution AS r
        WHERE r.systemSec = mss.securityClass) AS ore_type_rows
FROM mapSolarSystems AS mss
GROUP BY mss.securityClass
ORDER BY mss.securityClass;

-- 2. Detect any security class with no ore entries. These are belts that can
--    fail to spawn ore even when BeltEnabled=true.
SELECT mss.securityClass, COUNT(*) AS affected_systems
FROM mapSolarSystems AS mss
LEFT JOIN roidDistribution AS r ON r.systemSec = mss.securityClass
WHERE r.roidID IS NULL
GROUP BY mss.securityClass
ORDER BY affected_systems DESC;

-- 3. Check weights and suspicious values, without assuming sum=1 or 100.
SELECT systemSec, COUNT(*) AS row_count,
       SUM(percent) AS total_weight,
       MIN(percent) AS smallest_weight,
       MAX(percent) AS largest_weight,
       SUM(CASE WHEN percent <= 0 OR percent IS NULL THEN 1 ELSE 0 END) AS bad_weights
FROM roidDistribution
GROUP BY systemSec
ORDER BY systemSec;

-- 4. Find missing ore types, non-asteroid type mappings, and invalid weights.
--    EVEDB category 25 is Asteroid. Investigate before changing seed SQL.
SELECT r.systemSec, r.roidID, r.percent,
       t.typeName, g.groupName, g.categoryID
FROM roidDistribution AS r
LEFT JOIN invTypes AS t ON t.typeID = r.roidID
LEFT JOIN invGroups AS g ON g.groupID = t.groupID
WHERE t.typeID IS NULL OR g.categoryID IS NULL OR g.categoryID <> 25
   OR r.percent <= 0 OR r.percent IS NULL
ORDER BY r.systemSec, r.roidID;

-- 5. Actual ore pool per security class. Compare these rows against a
--    Crucible 1.6.5 era ore distribution reference, not modern post-2020 EVE.
SELECT r.systemSec, r.roidID, t.typeName, r.percent
FROM roidDistribution AS r
LEFT JOIN invTypes AS t ON t.typeID = r.roidID
ORDER BY r.systemSec, r.percent DESC, t.typeName;

-- 6. Security and NPC faction overrides by region, to diagnose missing rats.
SELECT mss.regionID, COUNT(*) AS systems_in_region,
       MIN(mss.security) AS min_truesec, MAX(mss.security) AS max_truesec,
       mr.regionName, mr.ratFactionID
FROM mapSolarSystems AS mss
LEFT JOIN mapRegions AS mr ON mr.regionID = mss.regionID
GROUP BY mss.regionID, mr.regionName, mr.ratFactionID
ORDER BY mss.regionID;

-- 7. Region rat-faction mappings not present in npcClassGroup.
--    Rogue drone fallback is intentional for unknown/zero factions.
SELECT mr.regionID, mr.regionName, mr.ratFactionID
FROM mapRegions AS mr
LEFT JOIN npcClassGroup AS ncg ON ncg.factionID = mr.ratFactionID
WHERE mr.ratFactionID > 0
GROUP BY mr.regionID, mr.regionName, mr.ratFactionID
HAVING COUNT(ncg.groupID) = 0
ORDER BY mr.regionID;

-- 8. Claims in NPC-faction systems or without a real alliance/TCU.
--    Factional warfare occupancy is a separate system and not player sov.
SELECT sov.solarSystemID, mss.solarSystemName, mss.factionID,
       mss.securityClass, sov.allianceID, sov.corporationID,
       sov.claimStructureID, sov.claimTime, sov.hubID, sov.contested
FROM mapSystemSovInfo AS sov
LEFT JOIN mapSolarSystems AS mss ON mss.solarSystemID = sov.solarSystemID
WHERE mss.solarSystemID IS NULL OR mss.factionID > 0
   OR sov.allianceID = 0 OR sov.claimStructureID = 0
ORDER BY sov.solarSystemID
LIMIT 200;

-- 9. Summary of dynamic asteroid rows, if sysAsteroids has been initialized.
--    A zero-row result does NOT prove belts will be empty; belts spawn on entry.
SELECT systemID, beltID, COUNT(*) AS stored_asteroids,
       COUNT(DISTINCT typeID) AS distinct_types,
       MIN(quantity) AS smallest_quantity,
       MAX(quantity) AS largest_quantity
FROM sysAsteroids
GROUP BY systemID, beltID
ORDER BY stored_asteroids DESC
LIMIT 100;
