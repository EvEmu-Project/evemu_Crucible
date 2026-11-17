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

-- 2. Detect defined security classes with no ore entries.
--    NULL is deliberately excluded here: it mixes known-space omissions,
--    wormhole space and special systems, and must be classified separately
--    by queries 10-13 before anyone changes static SQL.
SELECT mss.securityClass, COUNT(*) AS affected_systems
FROM mapSolarSystems AS mss
LEFT JOIN roidDistribution AS r ON r.systemSec = mss.securityClass
WHERE mss.securityClass IS NOT NULL AND r.roidID IS NULL
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

-- 10. IMPORTANT: break NULL securityClass into *system domains*, rather than
--     reporting the combined 2789 NULL systems as 2789 broken mining belts.
--     The 31xxxxxx range is traditional wormhole-space solarSystemIDs.
--     Wormhole mining/anomaly resources have different rules; do not assign
--     F/G/H/... null-sec class by security rating alone.
--     The groupID=9 filter is the source-verified Asteroid_Belt group.
SELECT
    CASE
        WHEN s.solarSystemID BETWEEN 31000000 AND 31999999 THEN 'wormhole-space IDs'
        WHEN s.solarSystemID BETWEEN 30000000 AND 30999999 THEN 'known-space IDs'
        ELSE 'other/special IDs'
    END AS system_domain,
    COUNT(*) AS null_security_class_systems,
    SUM(IFNULL(b.asteroid_belt_count, 0) > 0) AS systems_with_static_belts,
    SUM(IFNULL(b.asteroid_belt_count, 0)) AS total_static_belts,
    MIN(s.security) AS lowest_static_truesec,
    MAX(s.security) AS highest_static_truesec
FROM mapSolarSystems AS s
LEFT JOIN
    (SELECT solarSystemID, COUNT(*) AS asteroid_belt_count
     FROM mapDenormalize
     WHERE groupID = 9
     GROUP BY solarSystemID) AS b ON b.solarSystemID = s.solarSystemID
WHERE s.securityClass IS NULL
GROUP BY system_domain
ORDER BY system_domain;

-- 11. Locate missing known-space classes *that actually have static belts*.
--     These deserve investigation first; systems with NULL securityClass but
--     no belt might not be broken by the belt-ore class lookup at all.
SELECT s.regionID, r.regionName,
       COUNT(*) AS known_space_systems_with_missing_class,
       SUM(b.asteroid_belt_count) AS static_belts_affected,
       MIN(s.security) AS lowest_truesec,
       MAX(s.security) AS highest_truesec
FROM mapSolarSystems AS s
INNER JOIN
    (SELECT solarSystemID, COUNT(*) AS asteroid_belt_count
     FROM mapDenormalize
     WHERE groupID = 9
     GROUP BY solarSystemID) AS b ON b.solarSystemID = s.solarSystemID
LEFT JOIN mapRegions AS r ON r.regionID = s.regionID
WHERE s.securityClass IS NULL
  AND s.solarSystemID BETWEEN 30000000 AND 30999999
GROUP BY s.regionID, r.regionName
ORDER BY static_belts_affected DESC, s.regionID;

-- 12. Example known-space systems with missing classes and actual static belts.
--     The nearest class in the SAME CONSTELLATION is a diagnostic suggestion,
--     NOT evidence that the class is historically correct! Do not UPDATE from
--     this candidate without verifying the Crucible-era static data.
SELECT s.solarSystemID, s.solarSystemName,
       s.regionID, s.constellationID, s.factionID,
       s.security AS static_truesec,
       d.security AS runtime_denormalized_truesec,
       b.asteroid_belt_count,
       (SELECT neighbor.securityClass
        FROM mapSolarSystems AS neighbor
        WHERE neighbor.constellationID = s.constellationID
          AND neighbor.securityClass IS NOT NULL
        ORDER BY ABS(neighbor.security - s.security), neighbor.solarSystemID
        LIMIT 1) AS nearest_constellation_class_DIAGNOSTIC_ONLY
FROM mapSolarSystems AS s
INNER JOIN
    (SELECT solarSystemID, COUNT(*) AS asteroid_belt_count
     FROM mapDenormalize
     WHERE groupID = 9
     GROUP BY solarSystemID) AS b ON b.solarSystemID = s.solarSystemID
LEFT JOIN mapDenormalize AS d ON d.itemID = s.solarSystemID
WHERE s.securityClass IS NULL
  AND s.solarSystemID BETWEEN 30000000 AND 30999999
ORDER BY b.asteroid_belt_count DESC, s.regionID, s.solarSystemID
LIMIT 300;

-- 13. Compare the static security value used in the attached SQL summary
--     with the mapDenormalize security used by the original server query.
--     The patched loader now falls back to s.security only when d.security
--     is NULL. Do not overwrite differing non-NULL values without verifying
--     why the static and denormalized representations disagree.
SELECT s.solarSystemID, s.solarSystemName,
       s.security AS static_security,
       d.security AS denormalized_security,
       CASE WHEN d.itemID IS NULL THEN 'missing mapDenormalize system row'
            WHEN d.security IS NULL THEN 'missing denormalized security'
            ELSE 'nonmatching trueSec values' END AS diagnostic_reason
FROM mapSolarSystems AS s
LEFT JOIN mapDenormalize AS d ON d.itemID = s.solarSystemID
WHERE s.solarSystemID BETWEEN 30000000 AND 30999999
  AND (d.itemID IS NULL OR d.security IS NULL
       OR ABS(d.security - s.security) > 0.00001)
ORDER BY s.regionID, s.solarSystemID
LIMIT 300;
